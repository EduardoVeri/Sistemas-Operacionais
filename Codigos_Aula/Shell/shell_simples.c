#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char** argv){
	if(argc == 1){
		printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
		return 0;
	}
	int x;
	
	pid_t p_id;
	p_id = fork();
	
	if(p_id == 0){
		//filho
		char *cmd, **cmd_argv;
		cmd =  argv[1];
		cmd_argv = &argv[1];
		execvp(cmd, cmd_argv);
	}
	else{
		//Pai
		printf("Pai (%d) esperando filho (%d) terminar.\n", (int)getpid(), p_id);
		waitpid(-1, NULL, 0); //Espera o filho acabar
		printf("Filho acabou");
	}
	return 0;
}
