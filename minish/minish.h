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

// signal handlers
void bg_process(int signal);
void kill_fp(int signal);

command_t* new_command();
void free_command(command_t* c);
void print_commands(command_t* c);
void error(char* msg);
void execute_commands(command_t* c);

#endif
