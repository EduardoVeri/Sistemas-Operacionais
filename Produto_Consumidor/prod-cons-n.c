#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define N 10
#define NP 5
#define NC 2

int buffer[N];
int off_w, off_r;

sem_t mutex; // 1
sem_t vazio; // N
sem_t cheio; // 0

void init_prod_cons(void){
	off_w = 0;
	off_r = 0;
	sem_init(&mutex, 0, 1);
	sem_init(&vazio, 0, N);
	sem_init(&cheio, 0, 0);
	srand(time(NULL));
}

int gera_item (void) {
	return(rand()%N);
}

void *produtor(void *args) {
	int id = *(int*)args;
	int item;
	while(1) {
		sleep(1);
		item = gera_item();
		sem_wait(&vazio);
		sem_wait(&mutex);
		//insere_item(item);
		buffer[off_w] = item;
		printf("Prod %d buffer[%d] = %d \n",
				id, off_w, item);
		off_w = (off_w + 1) % N;
		sem_post(&mutex);
		sem_post(&cheio);
	}
	pthread_exit(NULL);
}

void *consumidor(void *args) {
	int id = *(int*)args;
	int item;
	while (1) {
		sem_wait(&cheio);
		sem_wait(&mutex);
		//item = remove_item();
		item = buffer[off_r];
		printf("Cons %d item %d = buffer[%d]\n",
				id, item, off_r);
		off_r = (off_r + 1) % N;
		sem_post(&mutex);
		sem_post(&vazio);
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
	pthread_t tp[NP], tc[NC];
	int id_p[NP], id_c[NC];
	init_prod_cons();
	cria_threads(tp, id_p, NP, produtor);
	cria_threads(tc, id_c, NC, consumidor);
	espera_threads(tp, NP);
	espera_threads(tc, NC);
	return 0;
}