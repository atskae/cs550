#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "minish.h"

pid_t shell_pid;
pid_t fg_pid = -1;

int bg_prog = 0;

void error(char* msg) {
	perror(msg);
	exit(1);
}

void bg_process(int signal) {
	int status;
	pid_t pid = waitpid(-1, &status, WNOHANG);
	 fprintf(stderr, "%i process exited.\n", pid);
	if(pid > 0) bg_prog--;
}

void kill_fp(int signal) {
	if(fg_pid != -1) {
		int err = tcsetpgrp(STDIN_FILENO, shell_pid); // give terminal control back to shell
		if(err < 0) error("Failed to return terminal control in signal handler.\n");
		fprintf(stderr, "Killing foreground process group %i\n", fg_pid);
		killpg(fg_pid, SIGKILL);	
		fg_pid = -1;
	} else fprintf(stderr, "No processes to kill.\n");
}

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
	free(c);
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

void execute_commands_r(command_t* c) {

	int fd_in = STDIN_FILENO;
	
	while(c) {

		int fd[2];
		// check if pipes are needed
		if(c->next) {
			if(pipe(fd) < 0) error("Failed to create pipe.\n");
		}	
		
		int status, err;
		pid_t pid;
		if( (pid = fork()) < 0) error("Fork failed.\n");
		else if (pid == 0) { // child process
			
			// fprintf(stderr, "%i) Child, my process group %i, will execute %s\n", getpid(), getpgid(0), c->argv[0]);
	
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
			if(c->bg_mode) fprintf(stderr, "Process %i in background mode.\n", getpid()); 			
		
			// fprintf(stderr, "%i) executing %s\n", getpid(), c->argv[0]);	
			err = execvp(c->argv[0], c->argv);
			if(err < 0) error("execvp() failed.\n");

		} else { // parent process
			//fprintf(stderr, "%i) Parent, my pgid: %i\n", getpid(), getpgid(getpid()));	
			
			// parent does not use pipe ; close
			if(c->next) { // pipe was created
				fd_in = fd[0]; // set read end of the pipe to next child
				close(fd[1]); // parent does not write to pipe ; close
			}

			if(!c->bg_mode) {
				err = waitpid(pid, &status, WUNTRACED); // blocking wait for child to complete
				if(err < 0) fprintf(stderr, "%i) waitpid() for child process %i failed.\n", getpid(), pid);	
				if(!WIFEXITED(status)) fprintf(stderr, "Child %i exited abnormally...\n", pid);		
			} // signal handler will take care of background mode processes 
		}

		c = c->next; // go to next command
	} // while(c) ; end	
}

void execute_commands(command_t* c) {

	// set shell pid in case Control+C is sent
	shell_pid = getpid();

	if(c->bg_mode) {
		bg_prog++;
		execute_commands_r(c); // do not fork an intermediate child
	}
	else {
		// the first child will fork each command
		int status, err;
		pid_t pid;
		if( (pid = fork()) < 0) error("Fork failed.\n");
		else if(pid == 0) {
			fg_pid = getpid();
			err = setpgid(getpid(), fg_pid); // set process group ; the rest of the children will inherit this process group
			if(err < 0) error("Failed to set process group.\n");
			while(tcgetpgrp(STDIN_FILENO) != getpid()); // wait until parent gives terminal control to child
			
			execute_commands_r(c);
		
			err = tcsetpgrp(STDIN_FILENO, getppid()); // give terminal control back to parent
			if(err < 0) error("Failed to return terminal control.\n");
			exit(0);
		} else {
			fg_pid = pid;	
			err = tcsetpgrp(STDIN_FILENO, pid); // give child process control of the terminal
			if(err < 0) error("Failed to assign terminal control to child.\n");	
		
			err = waitpid(pid, &status, WUNTRACED); // blocking wait for child to complete
			if(err < 0) fprintf(stderr, "%i) waitpid() for child process %i failed.\n", getpid(), pid);	
			if(!WIFEXITED(status)) fprintf(stderr, "First child %i exited abnormally...\n", pid);	
			fg_pid = -1; 
		}
	} // not background process ; end	
}
