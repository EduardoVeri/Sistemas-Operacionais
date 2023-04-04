/* Projeto para a disciplina de Sistemas Operacionais
 * Interpretador SHELL
 * Docente: Bruno Kimura
 * 
 * Itegrantes do Grupo:
 * Caio Bonani Carvalho - 156313
 * Eduardo Verissimo Faccio - 148859 
 * Marco Antonio Coral dos Santos - 156314
 * Raphael Damasceno Rocha de Moraes - 156380
 *
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

// Ativa os prints das funcoes da lista e impede a realizacao dos comandos
#define DEBUGER 0 // 0 = Sem Debug, 1 = Com Debug

// Lista dinamica de ponteiros para armazenar os comandos
typedef struct no {
	char **ponteiroArgv;
	struct no *proximo;
} NO;

typedef NO* PONTEIRONO;

// Variaveis globais, uma para a raiz da lista de comandos e outra para o tamanho da lista
PONTEIRONO listaCMD = NULL;
int tamanho = -1;

// Funcoes para o gerenciamento da lista de comandos
PONTEIRONO criarNo(char **argv);
void adicionarCMDLista(char **argv);
void formatarArgv(char **argv);
void mostrarLista();
void liberarLista(PONTEIRONO no);
char** retirarNomeComando(int indice);

// Funcao que ira realizar a execucao dos comandos
void realizaOperacaoPipe(int i);
	
int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Uso: %s <cmd> <p1> ... <pn> [\"|\" <cmd> ...]\n", argv[0]);
		return 0;
	}

	// Organiza os comandos obtidos em uma lista dinamica
	formatarArgv(argv);

	if(DEBUGER == 1){
		mostrarLista();
		liberarLista(listaCMD);
		return 0;
	}

	pid_t p_id;
	int i = tamanho;
	
	/* Cria um processo identico ao pai */
	p_id = fork();

	if (p_id == 0) {
		// ** Processo Filho **
		// Realiza a execucao dos comandos da lista obitida anteriormente
		realizaOperacaoPipe(i);
	} 
	else {
		// ** Processo Pai **
		// Aguarda a finalizacao da execucao dos comandos para poder liberar a lista ao final da execucao
		printf("Esperando os comandos serem executados!\n");
		waitpid(-1, NULL, 0);
		printf("Comandos finalizados!\n");
	} 	

	liberarLista(listaCMD);

	return 0;
}

void realizaOperacaoPipe(int i){
	// Caso base da recursao
	if(i == -1)
		return;
	
	// Cria o Pipe que sera utilizado para a comunicacao entre os processos pai e filho
	int fd_proximo[2];
	if (pipe(fd_proximo) == -1) {
		perror("pipe()");
		liberarLista(listaCMD);
		exit(1);
	}

	/* Cria um processo identico ao pai, que sera utilizado para 
	executar o comando anterior ao do que sera realizado pelo pai
	
	Exemplo: Em "ls -la | grep Shell" o "ls -la" sera executado pelo
	filho, enquanto o pai aguarda para executar o "grep Shell" */
	pid_t p_id;
	p_id = fork();

	if (p_id == 0 ) {
		// ** Processo Filho **
		/* Realiza a conexao de escrita do pipe entre os processos antes 
		de chamar a funcao novamente */
		dup2(fd_proximo[1], STDOUT_FILENO);
		realizaOperacaoPipe(i-1);
	} 
	else {
		// ** Processo Pai **
		/* Espera o processo filho finalizar, trazendo o 
		resultado de sua execucao */
		waitpid(-1, NULL, 0);

		// Fecha o lado de escrita do pipe, ja que nao sera mais utilizado
		close(fd_proximo[1]); 
		
		// Retira o nome e os argumentos do proximo comando da lista
		char **cmd;
		cmd = retirarNomeComando(i);

		/* Realiza a conexao de leitura do pipe antes de realizar a execucao do comando */
		dup2(fd_proximo[0], STDIN_FILENO);
		execvp(cmd[0], cmd); // Executa o comando desejado
		
	} 
}

// ========== Funcoes Lista ==========

PONTEIRONO criarNo(char **argv){
	PONTEIRONO novoNo = (PONTEIRONO)malloc(sizeof(NO));
	novoNo->ponteiroArgv = argv;
	novoNo->proximo = NULL;
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

		if(DEBUGER == 1)
			printf("Lista vazia, primeiro no adicionado!\n");

		return;
	}

	while(i < tamanho){
		noAux = noAux->proximo;
		i++;
	}

	noAux->proximo = novoNo;
	tamanho++;

	if(DEBUGER == 1)
		printf("No adicionado com sucesso!\n");
}

void formatarArgv(char **argv) {
	int inicio = 1;
	int i = 1;

	while(argv[i] != NULL) {
		if(DEBUGER == 1)
			printf("argv[%d] = %s\n", i, argv[i]);

		if(strcmp(argv[i], "|") == 0){
			argv[i] = NULL; 
			adicionarCMDLista(&argv[inicio]);
			inicio = i+1;

			if(DEBUGER == 1)
				printf("Comando adicionado a lista!\n");
		}
		i++;
	}
	adicionarCMDLista(&argv[inicio]);
	
	if(DEBUGER == 1)
		printf("Comando adicionado a lista!\n");
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
