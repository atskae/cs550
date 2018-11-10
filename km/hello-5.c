/*
	https://www.tldp.org/LDP/lkmpg/2.6/html/x323.html
	Obtaining command line arguments
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/stat.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atsuko Shimizu");

/* Define command line arguments here */
static short int myshort = 1;
static char* name = "Pajama Sam";
static int intArray[2] = {5, 25};
static int array_size = 0;

module_param(myshort, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(myshort, "A short integer");

module_param(name, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(name, "A dream boat");

// arrays must be set up differently
module_param_array(intArray, int, &array_size, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(intArray, "An array containing some interesting numbers");

static int __init hello5_init(void) {
	
	int i;
	printk(KERN_INFO "Wilkommen zu fifth world...\n");
	
	printk(KERN_INFO "myshort %hd\n", myshort);
	printk(KERN_INFO "%s is on his way!!\n", name);
	for(i=0; i<sizeof(intArray)/sizeof(int); i++) {
		printk(KERN_INFO "intArray[%i] = %i\n", i, intArray[i]);
	}
	printk("array_size = %i\n", array_size);
	
	return 0;
}

static void __exit hello5_exit(void) {
	printk(KERN_INFO "Adios! Von Welt funf.\n");
}

module_init(hello5_init);
module_exit(hello5_exit);
