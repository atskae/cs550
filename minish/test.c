#include <stdio.h>
#include <unistd.h>

/* tests redirection from file */
int main(int argc, char* argv[]) {
	sleep(10);

	char* name = "Welt";
	if(argv[1]) name = argv[1];	
	printf("Hallo %s!\n", name);
	char ch;
	while(read(STDIN_FILENO, &ch, 1) > 0) {
		printf("%c\n", ch); 
		break;
	}		

	return 0;
}
