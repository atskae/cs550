#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 256
#define MAX_COMMANDS 32
#define MAX_ARGS 8

// indices to the metadata array ; info about each command
#define METADATA_SIZE 6
#define METADATA_NUM_ARGS 0
#define METADATA_BG_MODE 1
#define METADATA_REDIR_R 2
#define METADATA_REDIR_L 3
#define METADATA_FILER_IDX 4
#define METADATA_FILEL_IDX 5

#define NO_REDIR 0
#define REDIR_R 1
#define REDIR_L 2

char** all_commands[MAX_COMMANDS] = {NULL}; // MAX_COMMANDS = maximum number of filters per command line
int metadata[MAX_COMMANDS][METADATA_SIZE] = {0}; // for each command [number of arguments, background mode, redirection, file index]
pid_t pgroup = -1;
pid_t kill_pid = -1;

void bg_process(int signal) {
	int status;
	waitpid(-1, &status, WNOHANG);
	//printf("%i process exited.\n", pid);
}

void kill_fp(int signal) {
	if(pgroup != -1) {
		printf("Killing process group %i\n", pgroup);
		killpg(pgroup, SIGKILL);		
		pgroup = -1;
	} else printf("No processes to kill.\n");
}

// recursive filters based on https://gist.github.com/zed/7835043
void execute_commands(int cmd, int in_fd) {
	
	// debug
	printf("%i commands: \n", cmd+1);
	for(int i=0; i<=cmd; i++) {
		int j = 0;
		while(all_commands[i][j]) {
			printf("%s ", all_commands[i][j]);
			j++;
		}
		printf("\n");
	}

	int status, error;
	int fd[2]; // pipe for filters "|"
	pid_t pid;

	// there are more commands, so a pipe between this command and next command must be created
	if(all_commands[cmd + 1]) { 
		if(pipe(fd) < 0) { // create pipe
			perror("Error creating pipe.\n");
			return;
		} else printf("pipe created.\n");	
	}

	// Create a child process which runs the command
	if( (pid = fork()) < 0) {
		perror("Failed to create a child process.\n");
		return;
	} else if(pid == 0) { // child process
		
		printf("%i) My process group %i\n", getpid(), getpgid(getpid()));
		
		// check if redirection
		int redirr = metadata[cmd][METADATA_REDIR_R];
		int redirl = metadata[cmd][METADATA_REDIR_L];
		int file, file_idx;

		if(all_commands[cmd + 1]) { // filter
			close(fd[0]); // close the read end of the pipe
			dup2(in_fd, STDIN_FILENO); // set the previous stdin to input
			dup2(fd[1], 1); // set stdout to point to the write end of the pipe
		}
		else if(redirr) { 
			printf("Redirection right\n");
			file_idx = metadata[cmd][METADATA_FILER_IDX];
			file = open( all_commands[cmd][file_idx], O_CREAT | O_RDWR, 0644);
			if(file < 0) {
				perror("Failed to open file for redirection.\n");
				return;
			}
			dup2(file, STDOUT_FILENO); // sets file to stdout
			all_commands[cmd][file_idx] = NULL; // don't pass the file name to exec
		}
		
		if(redirl) {
			printf("Redirection left\n");
		        file_idx = metadata[cmd][METADATA_FILEL_IDX];
		        file = open(all_commands[cmd][file_idx], O_RDONLY, 0);
		        if(file < 0) {
				perror("Failed to open file for redirection.\n");	
		        	return;
			}
			dup2(file, STDIN_FILENO); // sets file to stdin;	
			all_commands[cmd][file_idx] = NULL; // don't pass the file name to exec

		} 
		
		if(metadata[cmd][METADATA_BG_MODE]) printf("Process %i in background mode.\n", getpid());

		printf("%i) Child process. My parent %i\n", getpid(), getppid());
		printf("Child will execute: ");
		int j=0;
		while(all_commands[cmd][j]) {
			printf("%s ", all_commands[cmd][j]);
			j++;
		}
		printf("\n");

		// execute command
		error = execvp(all_commands[cmd][0], all_commands[cmd]);
		if(error < 0) { // only reaches here if error occurred
			printf("Failed to run %s\n", all_commands[cmd][0]);	
			return;
		}
	
	}  else { // parent process
		//if(cmd == 0) {
		//	pgroup = pid; // parents sets first child as the process group id ; children must set to this number
		//}
		//if(setpgid(pid, pgroup) != 0) {
		//	perror("Failed to assign process group.\n");
		//}	
		printf("%i) Parent process. My child %i. My process group %i\n", getpid(), pid, getpgid( getpid())  );
		if(!metadata[cmd][METADATA_BG_MODE]) {
			error = waitpid(pid, &status, WUNTRACED); // block wait for child to complete, if not in background mode	
			if(error < 0) printf("%i) waitpid() for child process %i failed.\n", getpid(), pid);	
			if(!WIFEXITED(status)) printf("%i) Child %i exited abnormally...\n", getpid(), pid);
		} // if child in bg mode, sig handler will take take of it
	
		close(fd[1]); // close the write end ; parent not writing anything 
		if(in_fd != STDIN_FILENO) close(in_fd); // child used this, not the parent ; no need		
		execute_commands(cmd + 1, fd[0]); // execute next command	
	}	

}

int main(int argc, char** argv) {

	// set up signal handlers 
	signal(SIGCHLD, bg_process); // checks if background process exited
	signal(SIGINT, kill_fp); // control+C triggers this signal ; kill foreground process

	while(1) {
		printf("minish> ");
	
		char input[MAX_BUFFER_SIZE]; // command line as as string	
		int num_commands = 0; // each command is separated by a filter "|"
	
		if(fgets(input, MAX_BUFFER_SIZE, stdin) != NULL) {

			// remove the newline so that strcmp() works as desired
			input[ strcspn(input, "\r\n") ] = 0; 
			
			if(strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) break;

			pgroup = -1;
			char get_kill_pid = 0; // "1" kill was requested						
			kill_pid = -1; // reset ; the process to kill
			int cmd = 0;	
			int arg = 0;

			all_commands[cmd] = (char**) malloc(MAX_ARGS * sizeof(char*)); // allocate for first command
	
			// get the first token (the command)
			all_commands[cmd][arg] = strtok(input, " "); // strtk(string, delimiter)		
			// obtain the rest of arguments
			while( all_commands[cmd][arg] != NULL && arg < MAX_ARGS ) {	
							
				if(strcmp(">", all_commands[cmd][arg]) == 0) {
					metadata[cmd][METADATA_REDIR_R] = 1; 	
					metadata[cmd][METADATA_FILER_IDX] = arg;	
					arg--; // ovewrites the ">" in the next iteration
				} else if(strcmp("<", all_commands[cmd][arg]) == 0) {
					metadata[cmd][METADATA_REDIR_L] = 1;
					metadata[cmd][METADATA_FILEL_IDX] = arg;
					arg--; // overwites the "<" in the next iteration
				} else if(strcmp("|", all_commands[cmd][arg]) == 0) {
					all_commands[cmd][arg] = NULL; // overwrites "|"
					metadata[cmd][METADATA_NUM_ARGS] = arg;
					cmd++; // move to next command
				} else if(strcmp("&", all_commands[cmd][arg]) == 0) {
					metadata[cmd][METADATA_BG_MODE] = 1;
					all_commands[cmd][arg] = NULL; // overwrites "&"
					metadata[cmd][METADATA_NUM_ARGS] = arg;
					break;		
				} 
				else if(strcmp("kill", all_commands[cmd][arg]) == 0) {
					get_kill_pid = 1; // get the pid to kill in the next token	
				} else if(get_kill_pid) {
					kill_pid = atoi(all_commands[cmd][arg]);
					break;
				}

				if(!all_commands[cmd]) {
					all_commands[cmd] = (char**) malloc(MAX_ARGS * sizeof(char*));	
					arg = 0;
				} else arg++;

				all_commands[cmd][arg] = strtok(NULL, " "); // get next token
			}

			if(kill_pid > 0) {
				kill(kill_pid, SIGKILL);
			} else {
				metadata[cmd][METADATA_NUM_ARGS] = arg;
				num_commands = cmd + 1;
				execute_commands(0, STDIN_FILENO);	
			}
						
			// reset command values			
			for(int i=0; i<num_commands; i++) {
				free(all_commands[i]);	
				all_commands[i] = NULL;
				for(int j=0; j<METADATA_SIZE; j++) {
					metadata[i][j] = 0;
				}
			}	
						
		} // read command and arguments ; end	
		
	} // while(1) end	

	printf("minish exiting.\n");
		
	return 0;

}
