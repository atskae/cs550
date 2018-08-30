#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
 
void indent(int num) {
	for(int i=0; i<num; i++) {
		printf("\t");
	}
}

int main(int argc, char** argv) {
	int H, C;
	if(argc < 3) {
		printf("H: height of the tree; C: number of children per inner node\n");
		exit(1);
	}

	H = atoi(argv[1]);
	C = atoi(argv[2]);	
//	printf("H: %i, C: %i\n", H, C);
	
	indent(H);
	printf("(%i): Process starting\n", getpid());
	indent(H);
	printf("(%i): Parent's id = (%i)\n", getpid(), getppid());
	indent(H);
	printf("(%i): Height of the tree = %i\n", getpid(), H);
	indent(H);
	printf("(%i): Creating %i children from height %i\n---\n", getpid(), C, H-1);

	if(H == 1) { // base case
		indent(H); 
		printf("(%i) Terminating at height %i\n", getpid(), H);
		exit(0);			
	} else {
	
		pid_t parent = getpid();
		pid_t result, w;
		int status = 0;
		pid_t children[C];

		for(int i=0; i<C; i++) {
			if(getpid() == parent) children[i] = fork();
			if(children[i] == 0) {
				result = children[i];
				break; // child		
			}
		}

		if(getpid() == parent) {
			for(int i=0; i<C; i++) { // wait for all children
				waitpid(children[i], &status, WUNTRACED);
			}
			indent(H); 
			printf("(%i) Terminating at height %i\n", getpid(), H);
			exit(0);
		}


		if(result == 0) {
			char arg1[100];
			char arg2[100];
			sprintf(arg1, "%i", H-1);
			sprintf(arg2, "%i", C);
			int error = execl(argv[0], argv[0], arg1, arg2, NULL);
			
			perror("(%i) execl() failed.\n");
		}
	}

	return 0;
}
