/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x279.html
	Demonstrates module documentation conventions
	
	modinfo <module name>.ko
*/

#include <linux/module.h> // needed by all modules
#include <linux/kernel.h> // printk macros
#include <linux/init.h>

#define AUTHOR "Atsuko Shimizu"
#define DESC "A super simple module."

static int __init hello4_init(void) {
	printk(KERN_INFO "Hallo vier\n");
	return 0;
}

static void __exit hello4_cleanup(void) {
	printk(KERN_INFO "Aufwiedersehen, 4\n");	
}

module_init(hello4_init);
module_exit(hello4_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESC);
