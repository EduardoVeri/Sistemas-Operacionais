#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/wait.h>
#include <unistd.h>

char buf[4096];


int conta_op(char **argv) {
  int i = 0;
  int c = 0;
  while (argv[i]) {
    if (argv[i][0] == '|' || argv[i][0] == '&' || argv[i][0] == '<' ||
        argv[i][0] == '>')
      c++;
    i++;
  }
  return c;
}

void posicoes_op(char **argv, int *pos) {
  int i = 0, j = 0;
  while (argv[i]) {
    if (argv[i][0] == '|' || argv[i][0] == '&' || argv[i][0] == '<' ||
        argv[i][0] == '>') {
      pos[j] = i;
      j++;
    }
    i++;
  }
}

void id_op(char **argv, int *pos_op, int *vet_op, int qtd_op) {

  int aux;

  for (int i = 0; i < qtd_op; i++) {

    aux = pos_op[i];

    if (argv[aux][0] == '|') {

      if (argv[aux][1] == '|') {
        vet_op[i] = 2;
      }

      else {
        vet_op[i] = 1;
      }
    }

    else if (argv[aux][0] == '&') {
      vet_op[i] = 3;
    }

    else if (argv[aux][0] == '<') {
      if (argv[aux][1] == '<') {
        vet_op[i] = 6;
      }

      else {
        vet_op[i] = 4;
      }
    }

    else if (argv[aux][0] == '>') {
      if (argv[aux][1] == '>') {
        vet_op[i] = 7;
      }

      else {
        vet_op[i] = 5;
      }
    }
  }
}

void imprime_argv(char **argv) {
  int i = 0;
  if (argv[i])
    printf("===\n");
  while (argv[i]) {
    printf("%d: '%s'\n", i, argv[i]);
    i++;
  }
  return;
}

int executar_or(char ***comandos, int qtd_pipe){

  //se o primeiro comando for sucesso, retorna para main, caso contrário, executar o segundo comando e dai retorna;

  return 1;
}

int executar_and(char ***comandos, int qtd_pipe){

  //se o primeiro comando for sucesso, executa o segundo, caso não so retorna para main;

  return 1;
}

int executar_pipe(char ***comandos, int qtd_pipe) {

  int status, i;
  pid_t pid;

  for (i = 0; i <= qtd_pipe; i++) {
    int fd[2];
    if (pipe(fd) == -1) {
      perror("pipe()");
      return -1;
    }
    pid = fork();

    if (pid == 0) {
      printf("filho %d \n", (int)getpid());
      
	  write(STDIN_FILENO, buf, strlen(buf));
	  
	  //close(STDIN_FILENO);
	  printf("Comando %s Argumentos %s\n", comandos[i][0], comandos[i][1]);
	  //printf("Passou o write\n");
	  close(fd[0]);
      dup2(fd[1], STDOUT_FILENO);
      close(fd[1]);
      execvp(comandos[i][0], comandos[i]);
      break;
    }
    printf("Pai (%d) esperando filho (%d) terminar.\n", (int)getpid(), pid);
    waitpid(-1, NULL, 0);
    
	close(fd[1]);
    read(fd[0], buf, 4096);
    close(fd[0]);
	//printf("%s", buf);
	
    /* close(fd[1]);
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]); */
  }
}

int main(int argc, char **argv) {
  
  bzero(buf, 4096);

  if (argc == 1) {
    printf("Uso: %s <cmd> <p1> ... <pn> \n", argv[0]);
    return 0;
  }

  int qtd_op = conta_op(argv);
  char **comandos[qtd_op + 1];
  int operadores[qtd_op + 1];
  int vet_op[qtd_op];
  char **exec[2];

  // 1 = pipe
  // 2 = or
  // 3 = and
  // 4 = <
  // 5 = >
  // 6 = <<
  // 7 = >>

  // imprime_argv(argv);
  posicoes_op(argv, vet_op);
  id_op(argv, vet_op, operadores, qtd_op);
  

  int pos = 0;
  for (int i = 0; i < qtd_op; i++) {
    pos = vet_op[i];
    argv[pos] = NULL;
    comandos[i + 1] = &argv[pos + 1];
  }
  comandos[0] = &argv[1];

  operadores[qtd_op] = 0;

  for (int i = 0; i <= qtd_op; i++) {
    imprime_argv(comandos[i]);
    printf("\nOPERADOR ENTRE OS COMANDOS: %d\n", operadores[i]);
  }

  
  int i = 0;

  while (i <= qtd_op) {

    if (operadores[i] == 0){
	  


      exec[0] = comandos[i];
	  //printf("Entrou no primeiro!\n");
      executar_pipe(exec, 0);
	  //printf("Saiu do primeiro! %s\n", buf);
      i++;
    }

    else if (operadores[i] == 1) {

      exec[0] = comandos[i];
      exec[1] = comandos[i + 1];
	  //printf("Entrou no segundo!\n");
      executar_pipe(exec, 0);
	  //printf("Saiu do segundo! %s\n", buf);
      i++;
    }

    else if (operadores[i] == 2){

      exec[0] = comandos[i];
      exec[1] = comandos[i + 1];

      executar_or(exec, 0);

      i++;
    }

    else if (operadores[i] == 3){

      i++;
    }

    else if (operadores[i] == 4){

      i++;
    }

    else if (operadores[i] == 5){

      i++;
    }

    else if (operadores[i] == 6){

      i++;
    }

    else if (operadores[i] == 7){

      i++;
    }

  }

  // executar_pipe(comandos, qtd_op);
  return 0;
}