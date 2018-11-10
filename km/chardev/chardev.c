/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x569.html#FTN.AEN630
	Counts the number of times the character device file was read	
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h> // put_user()

/*
	Function prototypes ; these usually go into a .h file
*/

// required enter and exit functions
int init_module(void);
void cleanup_module(void);

static int device_open(struct inode*, struct file);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 128 // max message length from device

/*
	Global variables ; must declare as static so they remain in this file (?)
*/
static int Major; // each device comes with a Major and Minor number for identification
static int Device_Open = 0;
static char message[BUF_LEN];
static char* message_ptr;

static struct file_operations fops {
	.read = device_read;
	.write = device_write;
	.open = device_open;
	.release = device_release;
};
