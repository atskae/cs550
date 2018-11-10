/* 
	https://www.tldp.org/LDP/lkmpg/2.6/html/hello2.html
	example of renaming init_module() and cleanup_module()
*/

#include <linux/module.h> // needed by all modules
#include <linux/kernel.h> // KERN_INFO
#include <linux/init.h> // needed in order to rename init_module() and cleanup_module()

static int __init hello2_init(void) { // cannot leave out void parameter
	printk(KERN_INFO "Hallo Welt 2\n");
	return 0; // success must return 0
}

static void __exit hello2_exit(void) {
	printk(KERN_INFO "Tschuss, Welt 2\n");	
}

module_init(hello2_init);
module_exit(hello2_exit);
