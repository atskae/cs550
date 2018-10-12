#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#include "minish.h"

command_t* new_command() {

	command_t* c = (command_t*) malloc( sizeof(command_t) );
	c->argv	= (char**) malloc(MAX_ARGS * sizeof(char*));
	c->argc = 0;
	c->bg_mode = 0;
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
		printf("argc %i, bg_mode %i, file_in %s, file_out %s\n", ptr->argc, ptr->bg_mode, ptr->file_in, ptr->file_out);
		ptr = ptr->next;
		n++;
	}
}

void error(char* msg) {
	perror(msg);
	exit(1);
}

void execute_commands(command_t* c) {

	int fd_in = STDIN_FILENO;
	
	while(c) {
	
		int fd[2];
		// check if pipes are needed
		if(c->next) {
			if(pipe(fd) < 0) error("Failed to create pipe.\n");
			//else printf("%i) Pipe created.\n", getpid());
		}	
		
		int status, err;
		pid_t pid;
		if( (pid = fork()) < 0) error("Fork failed.\n");
		else if (pid == 0) { // child process

			// if previous child wrote to write end of the previous pipe, this child should obtain stdin from read end of that pipe
			if(fd_in != STDIN_FILENO) {
				dup2(fd_in, STDIN_FILENO);
				close(fd_in);
			}
			if(c->next) {
				dup2(fd[1], STDOUT_FILENO); // redirect stdout to write end of the pipe	
				close(fd[1]);
				close(fd[0]); // close the read end of the pipe
			}

			// check for left or right redirection
			if(c->file_in) {
				//fprintf(stderr, "%i) Will redirect from file %s\n", getpid(), c->file_in);
				int file = open(c->file_in, O_RDONLY, 0);
				if(file < 0) error("Failed to open file for left redirection.\n"); 
				dup2(file, STDIN_FILENO); // set stdin to file
			}
			if(c->file_out) {
				//fprintf(stderr, "%i) Will redirect to file %s\n", getpid(), c->file_out);
				int file = open(c->file_out, O_CREAT | O_RDWR, 0644);
				if(file < 0) error("Failed to open file for right redirection.\n"); 
				dup2(file, STDOUT_FILENO); // set stdout to file
			}			
			if(c->bg_mode) printf("Process %i in background mode.\n", getpid()); 			
		
			// fprintf(stderr, "%i) executing %s\n", getpid(), c->argv[0]);	
			err = execvp(c->argv[0], c->argv);
			if(err < 0) error("execvp() failed.\n");

		} else { // parent process
			
			// parent does not use pipe ; close
			fd_in = fd[0]; // set read end of the pipe to next child
			close(fd[1]); // parent does not write to pipe ; close
			
			if(!c->bg_mode) {
				err = waitpid(pid, &status, WUNTRACED); // blocking wait for child to complete
				if(err < 0) printf("%i) waitpid() for child process %i failed.\n", getpid(), pid);	
				if(!WIFEXITED(status)) printf("Child %i exited abnormally...\n", pid);		
			} // signal handler will take care of background mode processes 
		}

		c = c->next; // go to next command
	}	
}
