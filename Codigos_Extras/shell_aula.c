#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

int pega_pipe_posicao(char **argv) {
	int i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "|") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}


void imprime_argv(char **argv){
	int i = 0;
	if (argv == NULL) 
		printf("===\n");
	
	while(argv[i] != NULL){
		printf("%d: %s ",i, argv[i]);
		i++;
	}
}

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
		return 0;
	}

	int pp;
	//imprime_argv(argv);
	pp = pega_pipe_posicao(argv);
	//printf("pp = %d", pp);
	
	argv[pp] = NULL;

	char **cmd1, **cmd2;
	cmd1 = &argv[1];
	cmd2 = &argv[pp+1];

	int fd[2];
	pid_t p_id;

	if(pipe(fd) == -1){
		perror("pipe");
		exit(1);
	}

	p_id = fork();

	if(p_id == 0){
		//filho
		printf("Filho (%d).\n", (int)getpid());
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		//close(STDOUT_FILENO);
		execvp(cmd1[0], cmd1);
	}
	else if (p_id > 0) {
		printf("Pai (%d) esperando filho (%d) terminar.\n",
			(int)getpid(), p_id);
		waitpid(-1, NULL, 0);

		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		//close(STDIN_FILENO);
		execvp(cmd2[0], cmd2);

	} else {
		// pai

		perror("fork");
	}

	return 0;
}