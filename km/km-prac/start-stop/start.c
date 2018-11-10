/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x351.html
	Example of multi-file kernel module
*/

#include <linux/module.h>
#include <linux/kernel.h>

int init_module() {
	printk(KERN_INFO "Inside start.c\n");
	return 0;
}
