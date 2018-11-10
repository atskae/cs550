/* http://tldp.org/LDP/lkmpg/2.4/html/x149.html */

#include <linux/module.h> // needed by all kernel modules
#include <linux/kernel.h> // needed for KERN_ALERT

/*
	All kernel modules must have:
	- init_module(): what happens when module is loaded in by insmod ; "start" function
	- cleanup_module(): what happens when module is removed by rmmod ; "end" function
	- can rename above functions
*/

int init_module() {
	printk("<1> Hello world! 1.\n"); // <1> indicates priority ; macros in kernel.h ; <1> = "action must be taken immediately"
	// low prioriy messages are logged to /var/log/messages ; may not make it to console
	// <1> ensures the message is printed to console
	// use appropriate priorities in real modules
	
	// must always return 0 on success ; or else module cannot be loaded
	return 0;
}

// unload module safely
void cleanup_module() {
	printk(KERN_ALERT "Goodbye world. 1.\n");
}
