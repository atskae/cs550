#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_BUFFER_SIZE 256
#define MAX_ARGS 8

#define NO_REDIR 0
#define REDIR_R 1
#define REDIR_L 2

void bg_process(int signal) {
	int status;
	pid_t pid = waitpid(-1, &status, WNOHANG);
//	printf("%i process exited.\n", pid);
}

int main(int argc, char** argv) {

	// set up signal handler for child
	signal(SIGCHLD, bg_process);

	// set up pipe for parent-child communication
	int fd[2];
	pipe(fd); // creates pipe

	char input[MAX_BUFFER_SIZE];
	while(1) {
		printf("minish> ");
	//	pid = waitpid(-1, &status, WNOHANG); // wait for background process
	////	if(WIFEXITED(status)) printf("%i) Child %i exited normally.\n", getpid(), pid);	
		
		if(fgets(input, MAX_BUFFER_SIZE, stdin) != NULL) {
			// remove the newline so that strcmp() works as desired
			input[ strcspn(input, "\r\n") ] = 0; // strcpn(str1, str2) counts # of chars in str1 that is NOT is str2
			
			if(strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) break;
			else { // obtain the command line arguments
				int bg_mode = 0;
				int redir = NO_REDIR; // redirection
				int redir_file_index = 0; // index in args[] where file is located
	
				char** args = (char**) malloc(MAX_BUFFER_SIZE * sizeof(char*));
				int num_args = 0;
				
				// get the first token (the command)
				args[0] = strtok(input, " "); // strtk(string, delimiter)		
				// obtain the rest of arguments
				while( args[num_args] != NULL && num_args < MAX_ARGS ) {	
			
					if( strcmp(">", args[num_args]) == 0) {
					//	printf("Redirection right on\n");
						redir = REDIR_R;	
						redir_file_index = num_args + 1;
					} else if(strcmp("<", args[num_args]) == 0) {
					//	printf("Redirection left on\n");
						redir = REDIR_L;
						redir_file_index = num_args + 1;
					}

					num_args++;
					args[num_args] = strtok(NULL, " ");
				}
				args[num_args] = NULL; // must terminate arguments array with NULL pointer when exec()
				
				// debug
			//	printf("%i) %i args\n", getpid(), num_args);
			//	for(int i=0; i<num_args; i++) {
			//		printf("arg %i: %s\n", i, args[i]);
			//	}
				
				// check if running in background mode, if so, do not wait for the child.
				if( strcmp(args[num_args-1], "&") == 0) {
					bg_mode = 1;
					// ignore the ampersand in the command to be forked
					args[num_args-1] = NULL;
					num_args--; 
				} 

				// Create a child process which runs the command
				int status, error;
				pid_t pid = fork();
				if(pid == 0) { // child process
		
					// check if redirection
					if(redir == REDIR_R || redir == REDIR_L ) { 
					//	printf("%i) Redirecting using %s\n", getpid(), args[redir_file_index]);	
						int file;
						if(redir == REDIR_R) {
							file = open(args[redir_file_index], O_CREAT | O_RDWR, 0644);
							dup2(file, STDOUT_FILENO); // sets file to stdout
						} else {
					//		printf("Redir left %s\n", args[redir_file_index]);	
							file = open(args[redir_file_index], O_RDONLY, 0);
							if(file < 0) {
								printf("Failed to open file %s for redirection.\n", args[redir_file_index]);
								continue;
							}
							dup2(file, STDIN_FILENO); // sets file to stdin;	
						}

						close(file);
						args[redir_file_index - 1] = NULL; // arguments after ">" invalidated
					} 
					
					if(bg_mode) {
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
					error = execvp(args[0], args);
					if(error < 0) { // only reaches here if error occurred
						printf("Failed to run %s\n", args[0]);	
						exit(1);
					}	
				} else if (pid < 0) {
					printf("%i) Failed to create child.\n", getpid());
					exit(1);
				} else { // parent process
					// printf("%i) Parent process. My child %i\n", getpid(), pid);
					if(!bg_mode) {
						error = waitpid(pid, &status, WUNTRACED); // wait for child to complete, if not in background mode	
						if(error < 0) {
							printf("%i) waitpid() for child process %i failed.\n", getpid(), pid);
						}
						
						if(WIFEXITED(status)) printf("%i) Child %i exited normally.\n", getpid(), pid);
					} else continue;  		
				}	
				
				free(args);	
			} // obtain the command line arguments ; end
			
		} // read command and arguments ; end	
	} // while(1) end	

	printf("minish exiting.\n");	
	return 0;

}
