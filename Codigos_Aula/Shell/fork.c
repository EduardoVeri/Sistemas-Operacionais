#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	if(argc != 2){
		printf("Uso: %s\n", argv[0]);
		return 0;
	}
	pid_t p_id;
	p_id = fork();
	
	if(p_id == 0){
		//filho
		int s = atoi(argv[1]);
		printf("filho %d dormindo %d s.\n", (int)getpid(), s);
		sleep(s);
	}
	else{
		//Pai
		printf("Pai (%d) esperando filho (%d) terminar.\n", (int)getpid(), p_id);
		waitpid(-1, NULL, 0); //Espera o filho acabar
	}
	return 0;
}
