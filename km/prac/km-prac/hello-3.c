/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x245.html
	demonstrating initializing varibles
*/

#include <linux/module.h> // every module needs this
#include <linux/kernel.h> // printk macros
#include <linux/init.h> // init macros

static int data __initdata = 5;

static int __init hello3_init(void) {
	printk(KERN_INFO "Initalized data %d\n", data);
	return 0; // must return 0 on success
}

static void __exit hello3_exit(void) {
	printk(KERN_INFO "See ya, world 3\n");
}

module_init(hello3_init);
module_exit(hello3_exit);
