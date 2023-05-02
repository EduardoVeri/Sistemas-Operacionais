#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>


int N, NT;
int *buffer;
int off_w;

sem_t mutex; // 1

void init_vars(void){
	off_w = 0;
	buffer = (int*)malloc(sizeof(int)*N);
	sem_init(&mutex, 0, 1);
}

void *produtor(void *args) {
	int id = *(int*)args;

	int aux = floor(N/NT);

    int inicio = aux*id;
    int fim = aux;

	if (id == NT-1) fim = N;
	else fim = fim * (id+1);

	int item = 0;

	for(int i = inicio; i < fim; i++) {
        if (i == 0) continue;
		if(N % i == 0){
			// Entra na regiao critica
            sem_wait(&mutex);
            buffer[off_w] = i;
            off_w++;
            sem_post(&mutex);
			// Sai da regiao critica
        }
	}
	pthread_exit(NULL);
}

void cria_threads(pthread_t *t, int *t_id, int n, void *(*func)(void*)){
	int i;
	for (i=0; i<n; i++) {
		t_id[i] = i;
		pthread_create(&t[i], NULL, func, &t_id[i]);
	}
	return;
}

void espera_threads(pthread_t *t, int n) {
	int i;
	for (i=0; i<n; i++) {
		pthread_join(t[i], NULL);
	}
}
int main(int argc, char **argv) {
	if (argc != 3) {
		printf("./numero_perfeito <N> <NT>\n");
		return 1;
	}
	N = atoi(argv[1]);
	NT = atoi(argv[2]);
	
	pthread_t tp[NT];
	int id_p[NT];
	init_vars();
	cria_threads(tp, id_p, NT, produtor);
	espera_threads(tp, NT);

	int soma = 0;
	for (int i = 0; i < off_w; i++) {
		soma += buffer[i];
	}

	if (soma == N) {
		printf("Numero perfeito\n");
	} else {
		printf("Numero nao perfeito\n");
	}

	return 0;
}