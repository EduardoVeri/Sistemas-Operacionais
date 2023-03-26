#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv){
	execvp(argv[1], &argv[1]);
	
	printf("Hello world");
	return 0;

}