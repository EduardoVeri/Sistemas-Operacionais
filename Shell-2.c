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
		i++;
	}

	noAux->proximo = novoNo;
	tamanho++;
	//printf("No adicionado com sucesso!\n");
}

void formatarArgv(char **argv) {
	int inicio = 1;
	int i = 1;

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
	//printf("Comando adicionado a lista!\n");
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
	executar o comando anterior ao do que sera realizado pelo pai */
	/* Exemplo: Em "ls -la | grep Shell" o "ls -la" sera executado pelo
	filho, enquanto o pai aguarda para executar o "grep Shell" */
	pid_t p_id;
	p_id = fork();

	if (p_id == 0 ) {
		// Processo Filho
		/* Realiza a conexao de escrita do pipe entre os processos antes 
		de chamar a funcao novamente */
		dup2(fd_proximo[1], STDOUT_FILENO);
		realizaOperacaoPipe(i-1);
	} 
	else {
		// Processo Pai
		/* Espera o processo filho finalizar, trazendo o 
		resultado de sua execucao */
		waitpid(-1, NULL, 0);
		close(fd_proximo[1]); // Fecha o lado de escrita do pipe, ja que nao sera mais utilizado
		
		// Retira o nome e os argumentos do proximo comando da lista
		char **cmd;
		cmd = retirarNomeComando(i);

		/* Realiza a conexao de leitura do pipe antes de realizar a execucao do comando */
		dup2(fd_proximo[0], STDIN_FILENO);
		execvp(cmd[0], cmd);
	} 

	close(fd_proximo[0]);
}
	
int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
		return 0;
	}

	formatarArgv(argv);

	//mostrarLista();

	pid_t p_id;
	int i = tamanho;
	
	/* Cria um processo identico ao pai */
	p_id = fork();

	if (p_id == 0 ) {
		realizaOperacaoPipe(i);
	} 
	else {
		printf("Esperando os comandos serem executados!\n");
		waitpid(-1, NULL, 0);
		printf("Comandos finalizados!\n");
	} 	

	liberarLista(listaCMD);

	return 0;
}