#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 256
#define MAX_ARGS 8

#define BACKGROUND 0 // execute command with an ampersand suffix
#define FOREGROUND 1 // execute the request command, then return to the prompt "minish>"

int mode = 0; // foreground or background mode

int main(int argc, char** argv) {

	char input[MAX_BUFFER_SIZE];
	while(1) {
		printf("minish> ");
		if(fgets(input, MAX_BUFFER_SIZE, stdin) != NULL) {
			// remove the newline so that strcmp() works as desired
			input[ strcspn(input, "\r\n") ] = 0; // strcpn(str1, str2) counts # of chars in str1 that is NOT is str2
			
			if(strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) break;
			else { // obtain the command line arguments
				
				char** args = (char**) malloc(MAX_ARGS * sizeof(char*));
				int num_args = 0;
				
				// get the first token (the command)
				args[0] = strtok(input, " "); // strtk(string, delimiter)		
				// obtain the rest of arguments
				while( args[num_args] != NULL && num_args < MAX_ARGS ) {	
					num_args++;
					args[num_args] = strtok(NULL, " ");
				}
				args[num_args] = NULL; // must terminate arguments array with NULL pointer when exec()
				
				// debug
				printf("%i) %i args\n", getpid(), num_args);
				for(int i=0; i<num_args; i++) {
					printf("arg %i: %s\n", i, args[i]);
				}

				// Create a child process which runs the command
				int status, error;
				pid_t pid = fork();
				if(pid == 0) { // child process
					printf("%i) Child process. My parent %i\n", getpid(), getppid());
					// execute command
					error = execvp(args[0], args);
					if(error < 0) { // only reaches here if error occurred
						printf("Failed to run %s\n", args[0]);	
						exit(1);
					}
					
				} else { // parent process
					printf("%i) Parent process. My child %i\n", getpid(), pid);
					error = waitpid(pid, &status, WUNTRACED); // WUNTRACED: wait until child has stopped
					if(error < 0) {
						printf("%i) waitpid() for child process %i failed.\n", getpid(), pid);
					}
					if(WIFEXITED(status)) printf("%i) Child %i exited normally.\n", getpid(), pid);	
				}	
				
				
			} // obtain the command line arguments ; end
			
		} // read command and arguments ; end	
	}	

	printf("minish exiting.\n");	
	return 0;

}
