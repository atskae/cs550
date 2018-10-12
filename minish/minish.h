#ifndef MINISH_H
#define MINISH_H

#define MAX_ARGS 8

typedef struct command_t command_t; // forward declaration

typedef struct command_t {

	int argc;
	char** argv;	
	
	int fd_in;
	int fd_out;
	char bg_mode;

	char* file_in;
	char* file_out;
	
	command_t* next;
	
} command_t;

command_t* new_command();
void print_commands(command_t* c);

#endif
