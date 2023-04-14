/* Projeto para a disciplina de Sistemas Operacionais
 * Interpretador SHELL
 * Docente: Bruno Kimura
 * 
 * Itegrantes do Grupo:
 * Caio Bonani Carvalho - 156313
 * Eduardo Verissimo Faccio - 148859 
 * Marco Antonio Coral dos Santos - 158467
 * Raphael Damasceno Rocha de Moraes - 156380
 *
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>

#define MAX_BUFFER 1024
#define MAX_CMD 200
#define TRUE 1

// Struct para rodar 
typedef struct comando{
	int background; // Deve ser rodado em background
	int pipe; // Possui um encadeamento de pipe
	int indice; // Marca quantos argumentos tem o comando
	int opLogico; // -1 = N/d, 0 = &&, 1 = ||
	int modoAbertura; // 0 = Sobrescrever, 1 = Adicionar
	char* entrada; // Arquivo de entrada
	char* saida; // Arquivo de saida
	char** cmd; // Comando cmd com os seus argumentos
} COMANDO;

COMANDO ** vetorCMD = NULL; // Vetor de comandos
int indiceVetor = 0; // Indice atual do vetor de comandos
int maxTam = 0; // Tamanho maximo do vetor de comandos

// Instrucoes para criar, pegar e organizar os comandos
void inicializaVetor(); // Inicializa o vetor de comandos
COMANDO* criaNo(); // Cria um novo no para o vetor de comandos
void retirarQuebra(char* str); // Retira uma quebra de linha caso ela tenha sido pega pelo fgets
void pegarCMD(); // Pega os comandos fornecidos pelo usuario
void mostrarCMD(); // Mostra os comandos pegos na tela
int temAspas(char* token); // Verifica se a string possui aspas
char* juntaToken(char* token); // Retira as aspas da string e junta os tokens
void liberarLista(); // Libera a lista de comandos

// Instrucoes para executar os comandos
void realizaOperacaoPipe(int i, int min); // Realiza comandos encadeados por pipe
int realizaComando(int i); // Realiza um comando unitario
int verificaPipe(int i); // Realiza uma verificaçao para saber se o comando possui pipe
void realizaComandoLogico(int i, int *status); // Realiza um comando com operador logico && ou ||
int verificaOp(int i); // Verifica se o comando possui operador logico && ou ||

/* Funcao MAIN para o SHELL */
int main(int argc, char **argv) {
	int incremento = 0, status = 0;
	int fd_arquivo, flag = 0;
	
	pid_t p_id;

	while(TRUE){
		liberarLista();
		printf("> ");
		pegarCMD();
		//mostrarCMD();
		
		status = 0;
		for(int i = 0; i < maxTam; i = i + incremento){	
			// Realiza a execucao dos comandos da lista obitida anteriormente
			
			flag = 0;
			incremento = 0;

			if((incremento = verificaPipe(i)) > 0){
				flag = 1;
			}
			else if(verificaOp(i) == 1){
				flag = 2;
			}

			incremento++;

			int fd_status[2];
			if (pipe(fd_status) == -1) {
				perror("pipe()");
				exit(1);
			}
			
			p_id = fork();

			if (p_id == 0) {
				// ** Processo Filho **
				if(flag == 1){		
					/* Cria mais um processo filho para poder executar os comandos pipe 
					e conseguir recuperar o seu valor de retorno */		
					pid_t p_id2;
					p_id2 = fork();

					// Processo filho 2
					if(p_id2 == 0) {
						if(vetorCMD[indiceVetor-1]->modoAbertura == 0) {
							fd_arquivo = open(vetorCMD[indiceVetor-1]->saida, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
							dup2(fd_arquivo, STDOUT_FILENO); 
							close(fd_arquivo);
						}
						else if (vetorCMD[indiceVetor-1]->modoAbertura == 1) {
							fd_arquivo = open(vetorCMD[indiceVetor-1]->saida, O_CREAT | O_APPEND | O_WRONLY, 0600); 
							dup2(fd_arquivo, STDOUT_FILENO); 
							close(fd_arquivo);
						}
						
						realizaOperacaoPipe(indiceVetor-1, incremento);
					}
					else{
						int statusAux;
						waitpid(-1, &statusAux, 0);
						if (WIFEXITED(statusAux)) {
							status = WEXITSTATUS(statusAux);
						}
					}
				}
				else if(flag == 2){
					//printf("Executando comando logico! %s %s\n", vetorCMD[i]->cmd[0], vetorCMD[i]->cmd[1]);
					realizaComandoLogico(i, &status);
				}
				else{
					status = realizaComando(i);
				}

				// Envia o status para o processo pai
				close(fd_status[0]);
				write(fd_status[1], &status, sizeof(status));
				close(fd_status[1]);

				//printf("Status: %d\n", status);
				// Finaliza o processo filho para nao ficar em loop
				return 0;
			} 
			else {
				// ** Processo Pai **
				// Aguarda a finalizacao da execucao dos comandos para poder liberar a lista ao final da execucao
				waitpid(-1, NULL, 0);
				
				// Recebe o status do processo filho
				close(fd_status[1]);
				read(fd_status[0], &status, sizeof(int));
				close(fd_status[0]);
			} 	
		}
	}
	return 0;
}

void realizaOperacaoPipe(int i, int min){
	int fd_arquivo;

	// Caso base da recursao
	if(i == min)
		return;
	
	// Cria o Pipe que sera utilizado para a comunicacao entre os processos pai e filho
	int fd_proximo[2];
	if (pipe(fd_proximo) == -1) {
		perror("pipe()");
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
		
		close(fd_proximo[0]);

		if(vetorCMD[i-1]->modoAbertura == 0){
			fd_arquivo = open(vetorCMD[i-1]->saida, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
			dup2(fd_arquivo, STDOUT_FILENO); 
			close(fd_arquivo);
		}
		else if (vetorCMD[i-1]->modoAbertura == 1){
			fd_arquivo = open(vetorCMD[i-1]->saida, O_CREAT | O_APPEND | O_WRONLY, 0600); 
			dup2(fd_arquivo, STDOUT_FILENO); 
			close(fd_arquivo);
		}
		else{ 
			dup2(fd_proximo[1], STDOUT_FILENO);
		}
		realizaOperacaoPipe(i-1, min);

		close(fd_proximo[1]);
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
		cmd = vetorCMD[i]->cmd;

		/* Realiza a conexao de leitura do pipe antes de realizar a execucao do comando */
		
		if(vetorCMD[i]->entrada != NULL){			
			fd_arquivo = open(vetorCMD[i]->entrada, O_RDONLY, 0600);  
			dup2(fd_arquivo, STDIN_FILENO);
			close(fd_arquivo);
		}
		else if(i != 0){
			dup2(fd_proximo[0], STDIN_FILENO);
		}

		close(fd_proximo[0]);
		
		execvp(cmd[0], cmd); // Executa o comando desejado
	} 
}

int realizaComando(int i){
	int fileDescriptor;
	int status;
	int returnStatus;

	char **cmd;
	cmd = vetorCMD[i]->cmd;

	pid_t p_id;
	p_id = fork();

	if (p_id == 0){
		if(vetorCMD[i]->entrada != NULL){
			int fileDescriptor = open(vetorCMD[i]->entrada, O_RDONLY, 0600);  
			dup2(fileDescriptor, STDIN_FILENO);
			close(fileDescriptor);
		}
		if(vetorCMD[i]->modoAbertura == 0){
			fileDescriptor = open(vetorCMD[i]->saida, O_CREAT | O_TRUNC | O_WRONLY, 0600); 
			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);
		}
		else if (vetorCMD[i]->modoAbertura == 1){
			fileDescriptor = open(vetorCMD[i]->saida, O_CREAT | O_APPEND | O_WRONLY, 0600); 
			dup2(fileDescriptor, STDOUT_FILENO); 
			close(fileDescriptor);
		}
		
		execvp(cmd[0], cmd);
	}
	else{
		// Caso o comando nao seja executado em background
		if(vetorCMD[i]->background == 0){
			waitpid(-1, &status, 0);
			if (WIFEXITED(status)) {
				returnStatus = WEXITSTATUS(status);
				return returnStatus;
			}
		}
		else{
			/* Caso o comando deva ser efetuado em background
			o parametro WNOHANG faz com que o pai nao fique preso
			esperando o filho terminar. Da mesma forma, o filho nao vira
			zumbi, ja que a conexao entre pai e filho se mantem */
			waitpid(-1, NULL, WNOHANG);
			printf("Processo %d esta executando em background\n", p_id);
		}
	}	
	
	return 0;
}


void realizaComandoLogico(int i, int *status){
	
	// Caso seja o primeiro comando a ser executado
	if(vetorCMD[i]->opLogico == 0){
		if(*status == 0){
			*status = realizaComando(i);
		}
	}
	else if(vetorCMD[i]->opLogico == 1){
		if(*status != 0){
			*status = realizaComando(i);
		}
	}
}


/* Essa instrucao verifica caso exista um encadeamento de pipes,
retornando a quantidade de comandos que devem ser executados em sequencia */
int verificaPipe(int i){
	int contador = 0;

	while(vetorCMD[i]->pipe == 1 && i < maxTam){
		i++;
		contador++;
	}

	return contador;
}

int verificaOp(int i){
	if(vetorCMD[i]->opLogico != -1)
		return 1;
	else
		return 0;
}

// =============================================================================
// =============== Instrucoes para pegar e organizar os comandos ===============
// =============================================================================

void inicializaVetor(){
	vetorCMD = (COMANDO**)malloc(sizeof(COMANDO*)*MAX_CMD);
	
	for(int i = 0; i < MAX_CMD; i++){
		vetorCMD[i] = NULL;
	}
}

COMANDO* criaNo(){
	COMANDO* novoNo = (COMANDO*)malloc(sizeof(COMANDO));
	novoNo->background = 0;
	novoNo->pipe = -1;
	novoNo->indice = 0;
	novoNo->modoAbertura = -1;
	novoNo->entrada = NULL;
	novoNo->saida = NULL;
	novoNo->opLogico = -1;
	novoNo->cmd = (char**)malloc(sizeof(char*)*10);

	for(int i = 0; i < 10; i++){
		novoNo->cmd[i] = NULL;
	}

	return novoNo;
}

void retirarQuebra(char* str){
	int tam = strlen(str);
	for(int i = 0; i < tam; i++){
		if(str[i] == '\n'){
			str[i] = '\0';
			return;
		}
	}
}

int temAspas(char* token){
	int i, flag = 0;
	for(i = 0; i < strlen(token); i++){
		if(token[i] == '\"'){
			flag++;
		}
	}
	return flag;
}


char * juntaToken(char* token){
	int tamanho, primeiro = 1;
	char * palavra = NULL;
	//bzero(palavra, MAX_BUFFER);
	int i;
	int flag = temAspas(token);

	if(flag == 1){
		tamanho = strlen(token);
		token[tamanho] = ' ';
		token[tamanho + 1] = '\0';

		palavra = strtok(NULL, "\"");
		strcat(token, palavra);
	}
	else if (flag == 2){
		for(i = 0; i < strlen(token); i++){
			if(token[i] == '\"'){
				int j, tam;
				tam = strlen(token);
				for(j = i; j < tam; j++){
					token[j] = token[j+1];
				}
				flag++;
			}
		}
	}
	else{
		printf("Erro Semantico, comando nao reconhecido\n");
		return NULL;
	}

	return token;
}

void pegarCMD(){
	COMANDO* noCMD = NULL;
	int flag = 0;
	int op = -1;
	char cmd[MAX_BUFFER];
    fgets(cmd, MAX_BUFFER, stdin);
    inicializaVetor();

	// Busca os valores dos tokens, que serao os comandos CMD
    char *token = strtok(cmd, " \t");
	while(token != NULL){
		retirarQuebra(token); // Retira a ultima quebra de linha
		
		if(strcmp(token, "&") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			noCMD->background = 1;
			flag = 0;
		}
		else if(strcmp(token, "&&") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			flag = 0;
			op = 0;
		}
		else if(strcmp(token, "||") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			flag = 0;
			op = 1;
		}
		else if(strcmp(token, "\"|\"") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			noCMD->pipe = 1;
			flag = 0;
		}
		else if(strcmp(token, "<") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			token = strtok(NULL, " \t");
			retirarQuebra(token);

			if(temAspas(token) > 0){
				token = juntaToken(token);
			}

			noCMD->entrada = token;
		}
		else if(strcmp(token, ">") == 0){
			if(noCMD == NULL){
				printf("Erro sintatico, operacao abortada\n");
				return;
			}
			
			token = strtok(NULL, " \t");
			retirarQuebra(token);

			if(temAspas(token) > 0){
				token = juntaToken(token);
			}

			noCMD->saida = token;
			noCMD->modoAbertura = 0;
		}
		else if(strcmp(token, ">>") == 0){
			if(noCMD == NULL){
				printf("Erro semantico, operacao abortada\n");
				return;
			}

			token = strtok(NULL, " \t");
			retirarQuebra(token);

			if(temAspas(token) > 0){
				token = juntaToken(token);
			}

			noCMD->saida = token;
			noCMD->modoAbertura = 1;
		}
		else if((token[0] == '-' || flag == 1) && strlen(token) > 0 ){
			if(temAspas(token) > 0){
				token = juntaToken(token);
			}
            noCMD->cmd[noCMD->indice++] = token;
		}
		else if(strlen(token) > 0){ // Evita adicionar uma instrucao so com o \n
			noCMD = criaNo();
			noCMD->cmd[0] = token;
			noCMD->indice = 1;
			noCMD->opLogico = op;
			vetorCMD[indiceVetor++] = noCMD;
			maxTam++;
			flag = 1;
			op = -1;
		}
		token = strtok(NULL, " \t");
	}
}

void liberarLista(){
	for(int i = 0; i < indiceVetor; i++){
		free(vetorCMD[i]->cmd);
		free(vetorCMD[i]);
	}
	indiceVetor = 0;
	maxTam = 0;
}


void mostrarCMD(){
	for(int i = 0; i < maxTam; i++){
		printf("Comando: %s ", vetorCMD[i]->cmd[0]);
		for(int j = 1; j < vetorCMD[i]->indice; j++){
			printf("%s ", vetorCMD[i]->cmd[j]);
		}
		printf("Op: %d \n", vetorCMD[i]->opLogico);
	}
	printf("\n");
}
