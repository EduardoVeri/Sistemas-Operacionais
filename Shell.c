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
int indice = -1;

PONTEIRONO criarNo(char **argv){
	PONTEIRONO novoNo = (PONTEIRONO)malloc(sizeof(NO));
	novoNo->ponteiroArgv = argv;
	novoNo->proximo = NULL;
	printf("No criado com sucesso!\n");
	return novoNo;
}	

void adicionarCMDLista(char **argv){
	int i = 0;
	PONTEIRONO novoNo, noAux;
	novoNo = criarNo(argv);
	noAux = listaCMD;

	if(listaCMD == NULL){
		listaCMD = novoNo;
		indice++;
		printf("Lista vazia, primeiro no adicionado!\n");
		return;
	}

	while(i < indice){
		noAux = noAux->proximo;
	}

	noAux->proximo = novoNo;
	indice++;
	printf("No adicionado com sucesso!\n");
}

void formatarArgv(char **argv) {
	//char **aux = argv;
	int inicio = 1;
	int i = 1;
	printf("Formatando argv...\n");
	while(argv[i] != NULL) {
		printf("argv[%d] = %s\n", i, argv[i]);
		if(strcmp(argv[i], "|") == 0){
			argv[i] = NULL; 
			adicionarCMDLista(&argv[inicio]);
			inicio = i+1;
			printf("Comando adicionado a lista!\n");
		}
		i++;
	}
	adicionarCMDLista(&argv[inicio]);
}

void mostrarLista(){
	PONTEIRONO noAux = listaCMD;
	int i = 0;
	printf("Mostrando lista...\n");
	while(i <= indice){
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


int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
		return 0;
	}

	formatarArgv(argv);
	printf("Indice: %d\n", indice);
	mostrarLista();
	liberarLista(listaCMD);
	
	/* 
	pid_t p_id;
	p_id = fork();

	if (p_id == 0) {
		// filho
		char *cmd, **cmd_argv;
		cmd = argv[1];
		cmd_argv = argv+1;
		execvp(cmd, cmd_argv);
	} else {
		// pai
		printf("Pai (%d) esperando filho (%d) terminar.\n",
			(int)getpid(), p_id);
		waitpid(-1, NULL, 0);
		printf("Filho acabou.\n");
	} */

	return 0;
}