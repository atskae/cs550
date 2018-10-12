#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#include "minish.h"

void bg_process(int signal) {
	int status;
	waitpid(-1, &status, WNOHANG);
	//printf("%i process exited.\n", pid);
}

int main(int argc, char** argv) {

	// set up signal handlers 
	signal(SIGCHLD, bg_process); // checks if background process exited
	
	while(1) {
		printf("minish> ");
		
		char* line = NULL;
		size_t n = 0;
		
		if(getline(&line, &n, stdin) > 0) {
			line[strcspn(line, "\r\n")] = 0;  // remove newline char for strcmp() to work
			if(strcmp(line, "exit") == 0) break;	
			
			command_t* commands = new_command();
			command_t* c = commands;	
			c->argv[c->argc] = strtok(line, " "); // get first token
			
			while(c->argv[c->argc] != NULL && c->argc < MAX_ARGS) {	
				if(strcmp(c->argv[c->argc], ">") == 0) {
					c->file_out = strtok(NULL, " "); // next token is the file name for right redirection
					c->argc--; // exclude file name in argv
				} else if(strcmp(c->argv[c->argc], "<") == 0) {
					c->file_in = strtok(NULL, " "); // next token is the file name for left redirection
					c->argc--; // exclude file name in argv
				} else if(strcmp(c->argv[c->argc], "|") == 0) {
					c->argv[c->argc] = NULL; // overwrite "|"	
					c->next = new_command();
					c = c->next;
					c->argv[c->argc] = strtok(NULL, " ");
					continue;
				}	
				c->argc++;
				c->argv[c->argc] = strtok(NULL, " ");
			}	
			print_commands(commands);
			
			free(commands);	
		} // getline() ; end

		free(line);
			
	} // while(1) ; end	
	
	return 0;
}
