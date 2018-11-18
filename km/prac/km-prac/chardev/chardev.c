/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x569.html#FTN.AEN630
	Simple character devices that tells user how many times the device file has been read
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h> // definition of file_operations struct (function ptrs of device functions)
#include <linux/sched.h>
#include <asm/uaccess.h> // put_user() ; for kernel to write to a user-provided buffer

#include "chardev.h"

/* Global variables defined as static */
static int Major; // Major number assigned to device driver ; indicates which driver handles which device file
static int Device_Open = 0;
static char message[BUF_LEN]; // message that devices gives when user asks
static char* message_ptr;

// defined in linux/fs.h
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release	
};

// when device is first loaded into the kernel
int init_module(void) {
	// adding a driver to the system
	Major = register_chrdev(0, DEVICE_NAME, &fops); // 0: kernel assigns a free major number for this device
	if(Major < 0) {
		printk(KERN_ALERT "Failed to register device with %i\n", Major);
		return Major; // non-zero return value prevents loading this module
	}

	printk(KERN_INFO "Device %s was loaded with major %i\n", DEVICE_NAME, Major);
	return SUCCESS; // 0	
}

// right before device is unloaded from kernel
void cleanup_module(void) {
	unregister_chrdev(Major, DEVICE_NAME); // no return value...
}

/*
	Character driver methods
*/

// called when process opens the device file ; ex) cat /dev/mycharfile
static int device_open(struct inode* inode, struct file* file) {
	static int counter = 0;
	if(Device_Open) {
		return -EBUSY;
	}
	Device_Open++;
	sprintf(message, "You opened this device file %i times.\n", counter++);
	message_ptr = message;
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
static ssize_t device_read(struct file* filep, char* buffer, size_t length, loff_t* offset) {
	// number of bytes written to buffer
	int bytes_read = 0;
	
	if(message_ptr == 0) return 0;  // indicates end of message
	
	// put data to be read into a buffer
	while(length && *message_ptr) {
		// buffer is in user data segment, not in kernel (so cannot dereference buffer to assign value to buffer)
		put_user(*(message_ptr++), buffer++);
		
		length--;
		bytes_read++;
	}

	// convention to return the number of bytes successfully read
	return bytes_read;
}

// called when user attempts to write to device file ; ex) echo "hi" > /dev/hello
static ssize_t device_write(struct file* filep, const char* buffer, size_t length, loff_t* offset) {
	printk(KERN_ALERT "Sorry, you can't write to this device... Lo siento.\n");
	return 0;
} 

