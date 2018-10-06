#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 256
#define MAX_COMMANDS 256
#define MAX_ARGS 8

// indices to the metadata array ; info about each command
#define METADATA_SIZE 4
#define METADATA_NUM_ARGS 0
#define METADATA_BG_MODE 1
#define METADATA_REDIR 2
#define METADATA_FILE_IDX 3

#define NO_REDIR 0
#define REDIR_R 1
#define REDIR_L 2

void bg_process(int signal) {
	int status;
	waitpid(-1, &status, WNOHANG);
//	printf("%i process exited.\n", pid);
}

char** all_commands[MAX_COMMANDS] = {NULL}; // MAX_COMMANDS = maximum number of filters per command line
int metadata[MAX_COMMANDS][4] = {0}; // for each command [number of arguments, background mode, redirection, file index]

void execute_commands(int pos) {
	
	int args_pos = metadata[pos][METADATA_NUM_ARGS];
	// debug
	printf("%i commands: \n", pos+1);
	for(int i=0; i<=pos; i++) {
		int j = 0;
		while(all_commands[i][j]) {
			printf("%s ", all_commands[i][j]);
			j++;
		}
		printf("\n");
	}
	
	// check if running in background mode, if so, do not wait for the child.
	if( strcmp(all_commands[pos][args_pos-1], "&") == 0) {
		metadata[pos][METADATA_BG_MODE] = 1;
		// ignore the ampersand in the command to be forked
		all_commands[pos][args_pos-1] = NULL;
		args_pos--; 
	} 

	// Create a child process which runs the command
	int status, error;
	pid_t pid;
	if( (pid = fork()) < 0) {
		perror("Failed to create a child process.\n");
		exit(1);
	} else if(pid == 0) { // child process	
		// check if redirection
		int redir = metadata[pos][METADATA_REDIR];
		if(redir == REDIR_R || redir == REDIR_L) { 
		//	printf("%i) Redirecting using %s\n", getpid(), args[redir_file_index]);	
			int file;
			int file_idx = metadata[pos][METADATA_FILE_IDX];
			if(metadata[pos][METADATA_REDIR] == REDIR_R) {
				file = open( all_commands[pos][file_idx], O_CREAT | O_RDWR, 0644);
				dup2(file, STDOUT_FILENO); // sets file to stdout
			} else {
		//		printf("Redir left %s\n", args[redir_file_index]);	
				file = open(all_commands[pos][file_idx], O_RDONLY, 0);
				if(file < 0) printf("Failed to open file for redirection.\n");	
				dup2(file, STDIN_FILENO); // sets file to stdin;	
			}

			close(file);
			all_commands[pos][file_idx - 1] = NULL; // arguments after ">" invalidated
		} 
		
		if(metadata[pos][METADATA_BG_MODE]) {
			sleep(1);
			printf("Process %i in background mode.\n", getpid());
		}	
	//	printf("%i) Child process. My parent %i\n", getpid(), getppid());
	//	printf("Child will execute: ");
	//	int j=0;
	//	while(args[j]) {
	//		printf("%s ", args[j]);
	//		j++;
	//	}
	//	printf("\n");

		// execute command
		error = execvp(all_commands[pos][0], all_commands[pos]);
		if(error < 0) { // only reaches here if error occurred
			printf("Failed to run %s\n", all_commands[pos][0]);	
			exit(1);
		}	
	}  else { // parent process
		// printf("%i) Parent process. My child %i\n", getpid(), pid);
		if(!metadata[pos][METADATA_BG_MODE]) {
			error = waitpid(pid, &status, WUNTRACED); // wait for child to complete, if not in background mode	
			if(error < 0) printf("%i) waitpid() for child process %i failed.\n", getpid(), pid);
				
			if(!WIFEXITED(status)) printf("%i) Child %i exited abnormally...\n", getpid(), pid);
		} 
	}	

}

int main(int argc, char** argv) {

	// set up signal handler for child
	signal(SIGCHLD, bg_process);

	while(1) {
		printf("minish> ");
	
		char input[MAX_BUFFER_SIZE]; // command line as as string	
		int num_commands = 0;
	
		if(fgets(input, MAX_BUFFER_SIZE, stdin) != NULL) {

			// remove the newline so that strcmp() works as desired
			input[ strcspn(input, "\r\n") ] = 0; 
			
			if(strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) break;
				
			int pos = 0;	
			int args_pos = 0;
			all_commands[pos] = (char**) malloc(MAX_ARGS * sizeof(char*)); // allocate for first command
	
			// get the first token (the command)
			all_commands[pos][args_pos] = strtok(input, " "); // strtk(string, delimiter)		
			// obtain the rest of arguments
			while( all_commands[pos][args_pos] != NULL && args_pos < MAX_ARGS ) {	
			
				if( strcmp(">", all_commands[pos][args_pos]) == 0) {
				//	printf("Redirection right on\n");
					metadata[pos][METADATA_REDIR] = REDIR_R;	
					metadata[pos][METADATA_FILE_IDX] = args_pos + 1;
				} else if(strcmp("<", all_commands[pos][args_pos]) == 0) {
				//	printf("Redirection left on\n");
					metadata[pos][METADATA_REDIR] = REDIR_L;
					metadata[pos][METADATA_FILE_IDX] = args_pos + 1;
				} else if(strcmp("|", all_commands[pos][args_pos]) == 0) {
					printf("Filter detected\n");	
					all_commands[pos][args_pos] = NULL;
					metadata[pos][METADATA_NUM_ARGS] = args_pos;
					pos++;
				}

				if(!all_commands[pos]) {
					all_commands[pos] = (char**) malloc(MAX_ARGS * sizeof(char*));	
					args_pos = 0;
				} else args_pos++;

				all_commands[pos][args_pos] = strtok(NULL, " ");
			}		
			metadata[pos][METADATA_NUM_ARGS] = args_pos;
			num_commands = pos + 1;
	
			execute_commands(0);
			
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
