/* Projeto para a disciplina de Sistemas Operacionais
 * Interpretador SHELL
 * Docente: Bruno Kimura
 * 
 * Itengrantes do Grupo:
 * Eduardo Verissimo Faccio - 148859 
 * 
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

// Lista dinamica de ponteiros para armazenar os comandos
typedef struct no {
	char **ponteiroArgv;
	struct no *proximo;
} NO;

typedef NO* PONTEIRONO;

PONTEIRONO listaCMD = NULL;
int tamanho = -1;

PONTEIRONO criarNo(char **argv){
	PONTEIRONO novoNo = (PONTEIRONO)malloc(sizeof(NO));
	novoNo->ponteiroArgv = argv;
	novoNo->proximo = NULL;
	//printf("No criado com sucesso!\n");
	return novoNo;
}	

void adicionarCMDLista(char **argv){
	int i = 0;
	PONTEIRONO novoNo, noAux;
	novoNo = criarNo(argv);
	noAux = listaCMD;

	if(listaCMD == NULL){
		listaCMD = novoNo;
		tamanho++;
		//printf("Lista vazia, primeiro no adicionado!\n");
		return;
	}

	while(i < tamanho){
		noAux = noAux->proximo;
	}

	noAux->proximo = novoNo;
	tamanho++;
	//printf("No adicionado com sucesso!\n");
}

void formatarArgv(char **argv) {
	//char **aux = argv;
	int inicio = 1;
	int i = 1;
	//printf("Formatando argv...\n");
	while(argv[i] != NULL) {
		//printf("argv[%d] = %s\n", i, argv[i]);
		if(strcmp(argv[i], "|") == 0){
			argv[i] = NULL; 
			adicionarCMDLista(&argv[inicio]);
			inicio = i+1;
			//printf("Comando adicionado a lista!\n");
		}
		i++;
	}
	adicionarCMDLista(&argv[inicio]);
}

void mostrarLista(){
	PONTEIRONO noAux = listaCMD;
	int i = 0;
	printf("Mostrando lista...\n");
	while(i <= tamanho){
		printf("No %d: ", i);
		while(*noAux->ponteiroArgv != NULL){
			printf("%s ", *noAux->ponteiroArgv);
			noAux->ponteiroArgv++;
		}
		noAux = noAux->proximo;
		i++;
		printf("\n");
	}
}

void liberarLista(PONTEIRONO no){
	if(no == NULL){
		return;
	}
	liberarLista(no->proximo);
	free(no);
}

char** retirarNomeComando(int indice){
	PONTEIRONO aux = listaCMD;
	int i = 0;

	if(indice > tamanho){
		printf("Indice fora do intervalo!");
		liberarLista(listaCMD);
		exit(1);
	}

	while(i != indice){
		aux = aux->proximo;
		i++;
	}

	return aux->ponteiroArgv;
}

void realizaOperacaoPipe(int i, int fd_anterior[2]){
	
	if(i == -1){
		//close(fd_anterior[1]);
		//close(fd_anterior[0]);
		return;
	}
	
	int fd_proximo[2];
	if (pipe(fd_proximo) == -1) {
		perror("pipe()");
		liberarLista(listaCMD);
		exit(1);
	}

	pid_t p_id;
	p_id = fork();

	if (p_id == 0 ) {
		//char **cmd;
		//cmd = retirarNomeComando(*i);
		dup2(fd_proximo[1], STDOUT_FILENO);
		realizaOperacaoPipe(i-1, fd_proximo);
		//execvp(cmd[0], cmd);
	} 
	else {
		// pai
		// ================
		char **cmd;

		cmd = retirarNomeComando(i);
		
		waitpid(-1, NULL, 0);
		dup2(fd_proximo[0], STDIN_FILENO);
		execvp(cmd[0], cmd);
		
		// ================


		//close(fd[1]);
		//printf("Pai (%d) esperando filho (%d) terminar.\n", (int)getpid(), p_id);
		//waitpid(-1, NULL, 0);
		//printf("Filho acabou.\n");

		
		//cmd = retirarNomeComando(i+1);
		//dup2(fd[0], STDIN_FILENO);
		//execvp(cmd[0], cmd);
	} 
	//close(fd[0]);
	//close(fd[1]);
}
	
int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
		return 0;
	}

	//printf("Primeiro\n");
	//printf("%d", tamanho);
	formatarArgv(argv);
	//printf("%d", tamanho);
	//printf("Indice: %d\n", tamanho);
	//mostrarLista();
	//liberarLista(listaCMD);
	//PONTEIRONO no = retirarNomeComando(0);
	//printf("%s\n", *no->ponteiroArgv);
	//printf("%s\n", *cmd);

	if(tamanho == 0){
		liberarLista(listaCMD);
		return -1;
	}
	pid_t p_id;
	int i = tamanho;
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe()");
		liberarLista(listaCMD);
		exit(1);
	}
	
	
	p_id = fork();
	//printf("%d\n", p_id);
	if (p_id == 0 ) {
		printf("Filho %d\n", (int)getpid());
		realizaOperacaoPipe(i, fd);
	} 
	else {
		printf("Esperando os comandos serem executados!\n");
		printf("Pai %d\n", (int)getpid());
		waitpid(-1, NULL, 0);
		printf("Comandos finalizados!\n");
	} 	


	return 0;
}