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
#define MAX_BUFFER 1024
#define MAX_CMD 200

// Struct para rodar 
typedef struct comando{
	int background; // Deve ser rodado em background
	int pipe; // Possui um encadeamento de pipe
	int indice; // Marca quantos argumentos tem o comando
	int modoAbertura; // 0 = Sobrescrever, 1 = Adicionar
	char* entrada; // Arquivo de entrada
	char* saida; // Arquivo de saida
	char** cmd; // Comando cmd com os seus argumentos
} COMANDO;

COMANDO ** vetorCMD = NULL;
int indiceVetor = 0;
int maxTam = 0;

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

void pegarCMD(){
	COMANDO* noCMD = NULL;
	int flag = 0;
	char cmd[MAX_BUFFER];
    fgets(cmd, MAX_BUFFER, stdin);
    inicializaVetor();


	// Busca os valores dos tokens, que serao os comandos CMD
    char *token = strtok(cmd, " ");
	while(token != NULL){
		retirarQuebra(token); // Retira a ultima quebra de linha


		if(strcmp(token, "&") == 0){
			noCMD->background = 1;
			flag = 0;
		}
		else if(strcmp(token, "&&") == 0){
			flag = 0;
		}
		else if(strcmp(token, "||") == 0){
			flag = 0;
		}
		else if(strcmp(token, "\"|\"") == 0){
			noCMD->pipe = 1;
			flag = 0;
		}
		else if(strcmp(token, "<") == 0){
			token = strtok(NULL, " ");
			noCMD->entrada = token;
		}
		else if(strcmp(token, ">") == 0){
			token = strtok(NULL, " ");
			noCMD->saida = token;
			noCMD->modoAbertura = 0;
		}
		else if(strcmp(token, ">>") == 0){
			token = strtok(NULL, " ");
			noCMD->saida = token;
			noCMD->modoAbertura = 1;
		}
		else if(token[0] == '-' || flag == 1){
			// A string pega eh parametro
            noCMD->cmd[noCMD->indice++] = token;
			//printf("Parametro: %s\n", token);
		}
		else if(strlen(token) > 0){ // Evita adicionar uma instrucao so com o \n
			noCMD = criaNo();
			noCMD->cmd[0] = token;
			noCMD->indice = 1;
			vetorCMD[indiceVetor++] = noCMD;
			maxTam++;
			flag = 1;
		}
		token = strtok(NULL, " ");
	}
}

void mostrarCMD(){
	for(int i = 0; i < maxTam; i++){
		printf("Comando: %s ", vetorCMD[i]->cmd[0]);
		for(int j = 1; j < vetorCMD[i]->indice; j++){
			printf("%s ", vetorCMD[i]->cmd[j]);
		}
		printf("\n");
	}
	printf("\n");
}

void realizaOperacaoPipe();
	
int main(int argc, char **argv) {
	pegarCMD();
	mostrarCMD();
	pid_t p_id;
	
	/* Cria um processo identico ao pai */
	p_id = fork();

	if (p_id == 0) {
		// ** Processo Filho **
		// Realiza a execucao dos comandos da lista obitida anteriormente
		realizaOperacaoPipe();
		return 0;
	} 
	else {
		// ** Processo Pai **
		// Aguarda a finalizacao da execucao dos comandos para poder liberar a lista ao final da execucao
		printf("Esperando os comandos serem executados!\n");
		waitpid(-1, NULL, 0);
		printf("Comandos finalizados!\n");
	} 	

	return 0;
}

void realizaOperacaoPipe(){
	/* int *fd_aux1 = NULL;
	int *fd_aux2 = NULL; */

	int fd_1[2];
	int fd_2[2];

	pid_t p_id;
	pid_t p_id2;
	
/* 	fd_aux1 = fd_1;
	fd_aux2 = fd_1; */

	for(int i = 0; i < maxTam; i = i + 2){	
		p_id = fork();
		
		// Cria o Pipe que sera utilizado para a comunicacao entre os processos pai e filho
		if(p_id == 0){
			if (pipe(fd_1) == -1) {
				perror("pipe()");
				exit(1);
			}

			p_id2 = fork();

			if(p_id2 == 0){
				char **cmd;
				cmd = vetorCMD[i]->cmd;
				
				if(i != 0) dup2(fd_2[0], STDIN_FILENO);
				if(i+1 < maxTam) dup2(fd_1[1], STDOUT_FILENO);

				execvp(cmd[0], cmd); // Executa o comando desejado
				return;
			}
			else{
				waitpid(-1, NULL, 0);
		
				if(i+2 >= maxTam){
					if(i%2 == 1)
						return;
					//printf("Esperando o comando ser executado\n");
					
					char **cmd;
					cmd = vetorCMD[i+1]->cmd;
					close(fd_1[1]);
					dup2(fd_1[0], STDIN_FILENO);
					//close(STDIN_FILENO);
					execvp(cmd[0], cmd);
				}
				else{
					if (pipe(fd_2) == -1) {
						perror("pipe()");
						exit(1);
					}
					
					dup2(fd_1[0], STDIN_FILENO);
					dup2(fd_2[1], STDOUT_FILENO);
					close(fd_2[0]);
					close(fd_1[1]);

					char **cmd;
					cmd = vetorCMD[i+1]->cmd;
					
					execvp(cmd[0], cmd);
				}				
			}
		}
		else{
			waitpid(-1, NULL, 0);
		}	
	} 
}