#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define TRUE 1
#define N 10
#define NP 5
#define NC 2

int buffer[N];
int off_w;

sem_t mutex; // 1

void init_prod_cons(void){
	off_w = 0;
	sem_init(&mutex, 0, 1);
}

void *consumidor(void *args) {
	int id = *(int*)args;
    int inicio = (N/NP);
    inicio = inicio * id;
    int fim = (N/NP);
    fim = fim * (id+1);

	for(int i = inicio; i < fim; i++) {
        if(N % i == 0){
            sem_wait(&mutex);
            printf("Divisel por: %d\n", i);
            item = buffer[off_w];
            off_w++;
            sem_post(&mutex);
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
	pthread_t tp[NP];
	int id_p[NP];
	init_prod_cons();
	cria_threads(tp, id_p, NP, produtor);
	espera_threads(tp, NP);
	return 0;
}