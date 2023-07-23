#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <strings.h>
#include <string.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Uso: %s <msg_p_filho>\n", argv[0]);
		return 0;
	}
	pid_t p_id;
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe()");
		return -1;
	}

	p_id = fork();

	if (p_id == 0) {
		// filho
		#define PIPE_SIZE 4096
		char msg[PIPE_SIZE];
		int nr;
		bzero(msg, PIPE_SIZE);
		nr = read(fd[0], msg, PIPE_SIZE);
		if (nr < 0) {
			perror("read()");
			return -1;
		}
		printf("Filho (%d) leu do pipe fd[0](%d) total de %d bytes: '%s'\n",
			(int)getpid(), fd[0], nr, msg);
		return 0;
	} else {
		// pai
		char *msg_p_filho = argv[1];
		int nw;
		sleep(10);
		nw = write(fd[1], msg_p_filho, strlen(msg_p_filho));
		printf("Pai (%d) escreveu no pipe fd[1](%d) para filho (%d) total de %d bytes: '%s'\n",
			(int)getpid(), fd[1], p_id, nw, msg_p_filho);
		waitpid(-1, NULL, 0);
	}

	return 0;
}