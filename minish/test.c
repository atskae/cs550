#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
	sleep(1);

	char* name = "Welt";
	if(argv[1]) name = argv[1];	
	printf("Hallo %s!\n", name);
//	char ch;
//	while(read(STDIN_FILENO, &ch, 1) > 0) {
//		printf("%c", ch); 
//		break;
//	}		
//
	return 0;
}
