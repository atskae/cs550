#include <stdlib.h>
#include <stdio.h>

#include "minish.h"

command_t* new_command() {

	command_t* c = (command_t*) malloc( sizeof(command_t) );
	c->argv	= (char**) malloc(MAX_ARGS * sizeof(char*));
	c->argc = 0;
	c->file_in = NULL;
	c->file_out = NULL;
	c->next = NULL;
	
	return c;	
}

void free_command_r(command_t* c) {
	if(!c->next) {
		free(c->argv);
		return;
	}
	free_command_r(c->next);
	free(c->next);
	free(c->argv);
}
void free_command(command_t* c) {
	free_command_r(c);	
}

void print_commands(command_t* c) {

	command_t* ptr = c;
	int n = 0;
	while(ptr) {
		printf("%i) ", n);
		for(int i=0; i<ptr->argc; i++) {
			printf("%s ", ptr->argv[i]);
		}
		printf("; ");
		printf("argc %i, file_in %s, file_out %s\n", ptr->argc, ptr->file_in, ptr->file_out);
		ptr = ptr->next;
		n++;
	}
}
