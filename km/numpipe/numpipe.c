#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // definition of file_operations struct (function ptrs of device functions)
#include <linux/sched.h>
#include <asm/uaccess.h> // put_user() ; for kernel to write to a user-provided buffer
#include <linux/moduleparam.h>
#include <linux/vmalloc.h> // allocates virtually contiguous memory, not necessarily physically contiguous
// ; (kmalloc() allocates phyiscally contiguous)
#include <linux/uaccess.h> // copy_to_user() ; copy_from_user()

#include "numpipe.h"

#define SUCCESS 0
#define DEVICE_NAME "numpipe"
#define QUEUE_DEFAULT_SIZE 20

/* Device Information */
static int Major; // Major number assigned to device driver ; indicates which driver handles which device file
static int Device_Open = 0; // number of users who currently opened device file

/* Share FIFO queue */
static int* queue = NULL; // shared FIFO queue
static int head_ptr = 0; // points to the next item to be read; must be updated with modulus
static int tail_ptr = 0; // points to the next free spot to add an item; must be updated with modulus
static int max_size = QUEUE_DEFAULT_SIZE; // maximum size of queue 
module_param(max_size, int, 0); // module_param(name, type, permissions)
//MODULE_PARAM_DESC(max_size, "Maximum size of the shared FIFO queue");

int num_items = 0; // debug ; do not need

/* Synchronization */
static DEFINE_MUTEX(lock); // gives mutual exclusion when accessing shared FIFO queue
static DEFINE_SEMAPHORE(full_slots_n); // keeps track of full slots ; if full_slots == max_size, block
static DEFINE_SEMAPHORE(empty_slots_n); // keeps tracks of empty slots ; if empty_slots == max_size, block
//static wait_queue_head_t producer_wq; // producers that blocked because the queue was full
//static wait_queue_head_t consumer_wq; // consuemrs that blocked because the queue was empty

// defined in linux/fs.h
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release	
};

// when device is first loaded into the kernel
int init_module(void) {
	// adding driver to the system
	Major = register_chrdev(0, DEVICE_NAME, &fops); // 0: kernel assigns a free major number for this device
	if(Major < 0) {
		printk(KERN_ALERT "Failed to register device with %i\n", Major);
		return Major; // non-zero return value prevents loading this module
	}

	// allocate memory for shared FIFO queue
	queue = (int*) vmalloc(max_size);
	if(!queue) {
		printk(KERN_ALERT "Failed to allocate FIFO queue memory.\n");
		return -ENOMEM; 
	}
	memset(queue, 0, max_size * sizeof(int));
	head_ptr = tail_ptr = 0; // reset pointers
	
	// initialize synchronization primitives
	mutex_init(&lock);
	sema_init(&full_slots_n, 0);
	sema_init(&empty_slots_n, max_size);
	//init_waitqueue_head(&producer_wq);
	//init_waitqueue_head(&consumer_wq);

	printk(KERN_INFO "Device %s was loaded with major %i\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Shared FIFO queue max_size: %i\n", max_size);
	return SUCCESS; // 0
}

// right before device is unloaded from kernel
void cleanup_module(void) {
	unregister_chrdev(Major, DEVICE_NAME);
	
	// free memory of shared FIFO queue
	vfree(queue);
}

/*
	Character driver methods
*/

// called when process opens the device file ; ex) cat /dev/mycharfile
static int device_open(struct inode* inode, struct file* file) {
	Device_Open++;
	try_module_get(THIS_MODULE); // increments usage count ; ensures that module cannot be removed if users are currently using it
	// must call module_put() when closing device to decrement usage count
	
	return SUCCESS;
}

// called when user closes device file
static int device_release(struct inode* inode, struct file* file) {
	Device_Open--;
	module_put(THIS_MODULE); // decrements current usage count ; if this never reaches 0, the module can never be removed	
	
	return SUCCESS;
}

// called when user attempts to read device file
// consumer removes an item from the shared FIFO queue
static ssize_t device_read(struct file* filep, char* buffer, size_t length, loff_t* offset) {

	int item; // item to give to consumer
	int err;
	unsigned long ret;

	err = down_interruptible(&full_slots_n); // decreases the # of full slots ; if already 0, user-process blocks
	if(err != 0) { // may have been interrupted
		printk(KERN_ALERT "Consumer failed to acquire full_slots_n semaphore.\n");
		return err;
	}
	err = mutex_lock_interruptible(&lock); // obtains the lock if available
	if(err != 0) {
		printk(KERN_ALERT "Consumer failed to acquire lock.\n");
		return err;
	}
	item = queue[head_ptr];
	num_items--; // debug
	printk(KERN_INFO "%i items in queue.\n", num_items);	
	head_ptr = (head_ptr + 1) % max_size;
	mutex_unlock(&lock); // release lock
	up(&empty_slots_n); // just removed item from queue ; increase the number of empty slots 

	printk(KERN_INFO "Sending item %i to user.\n", item);
	ret = copy_to_user(buffer, &item, sizeof(item)); // copy_to_user(to, from, number of bytes)
	if(ret != 0) { // success = 0
		printk(KERN_ALERT "Failed to send %li bytes to user.\n", ret);
		return sizeof(item) - ret;
	}

	return sizeof(item);
}

// called when user attempts to write to device file ; ex) echo "hi" > /dev/hello
// producer adds an item to the shared FIFO queue
static ssize_t device_write(struct file* filep, const char* buffer, size_t length, loff_t* offset) {

	// item to place into the shared FIFO queue
	int item;
	int err;
	unsigned long ret = copy_from_user(&item, buffer, sizeof(item)); // obtain item from user
	if(ret != 0) {
		printk(KERN_ALERT "Failed to obtain %li bytes from user.\n", ret);
		return sizeof(item) - ret;
	}	

	err = down_interruptible(&empty_slots_n); // decrease the number of empty slots
	if(err != 0) { // may have been interrupted
		printk(KERN_ALERT "Producer failed to acquire empty_slots_n semaphore.\n");
		return err;
	}
	err = mutex_lock_interruptible(&lock); // obtain lock
	if(err != 0) { // may have been interrupted
		printk(KERN_ALERT "Producer failed to acquire lock.\n");
		return err;
	}
	queue[tail_ptr] = item; // place into queue
	num_items++; // debug
	printk(KERN_INFO "%i items in queue.\n", num_items);	
	tail_ptr = (tail_ptr + 1) % max_size;
	mutex_unlock(&lock); // release lock
	up(&full_slots_n); // just added an item to queue ; increase the number of full slots	

	printk(KERN_INFO "Obtained item %i from user.\n", item);	
	return sizeof(item);
} 
