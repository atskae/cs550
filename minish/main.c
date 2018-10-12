#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#include "minish.h"

extern int bg_prog;

int main(int argc, char** argv) {

	// set up signal handlers 
	signal(SIGCHLD, bg_process); // checks if background process exited
	signal(SIGINT, kill_fp); // control+C triggers this signal ; kill foreground process
	
	while(1) {
		printf("minish> ");
		
		char* line = NULL;
		size_t n = 0;
		
		if(getline(&line, &n, stdin) > 0) {
			line[strcspn(line, "\r\n")] = 0;  // remove newline char for strcmp() to work
			if(strcmp(line, "exit") == 0) {
				free(line);
				break;	
			}

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
				} else if(strcmp(c->argv[c->argc], "&") == 0) {
					c->bg_mode = 1;
					c->argv[c->argc] = strtok(NULL, " ");
					continue;
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
			//print_commands(commands);
			
			if(strcmp(commands->argv[0], "kill") == 0) {
				pid_t kill_pid = atoi(commands->argv[1]);
				fprintf(stderr, "%i) Killing process %i\n", getpid(), kill_pid);
				int err = kill(kill_pid, SIGKILL);
				if(err) error("Failed to kill process.\n");
			} else execute_commands(commands);	
					
			free_command(commands);	
		} // getline() ; end

		free(line);
			
	} // while(1) ; end

	// "exit" was issued ; reap the background child processes
	if(bg_prog > 0) {
		printf("Must wait for %i background processes before exiting.\n", bg_prog);
		for(int i=0; i<bg_prog; i++) {
			wait(NULL);	
		}
	}

	printf("minish exiting.\n");	
	return 0;
}
