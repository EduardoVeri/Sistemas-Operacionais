#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h> 
#include <stdlib.h>
#include <unistd.h>
//Biblioteca para numeros aleatorios


#define N 10

int buffer[N];
int off_w, off_r;
sem_t mutex, vazio, cheio;


void init_prod_cons() {
	//off_w = off_r = 0;
	off_w = 0;
	off_r = 0;
	sem_init(&mutex, 0, 1);
	sem_init(&vazio, 0, N);
	sem_init(&cheio, 0, 0);
	srand(time(NULL));
}

int gera_item() {
	return(rand()%N);
}

void *produtor(void *args) {
	int id = *(int*)args;
	int item;
	while(1){
		sleep(1);
		item = gera_item();
		sem_wait(&vazio);
		sem_wait(&mutex);

		//insere_item(item);
		buffer[off_w] = item;
		printf("Produtor %d buffer[%d] = %d\n", id, off_w, item);
		off_w = (off_w + 1) % N;

		sem_post(&mutex);
		sem_post(&cheio);
	}
	pthread_exit(NULL);
}


void *produtor(void *args) {
	int id = *(int*)args;
	int item;
	while(1){
		sem_wait(&cheio);
		sem_wait(&mutex);

		//item = remove_item();
		item = buffer[off_r];
		printf("Consumidor %d item %d = buffer[%d]\n", id, item, off_r);


		sem_post(&mutex);
		sem_post(&vazio);
		//consome_item(item);
	}
	pthread_exit(NULL);
}

void cria_threads(pthread_t *t, int *t_id, int n, void *(*func)(void *)) {
	int i;
	for(i = 0; i < n; i++) {
		t_id[i] = i;
		pthread_create(&t[i], NULL, func, &t_id[i]);
	}
	return;
}

void espera_threads(pthread_t *t, int n) {
	int i;
	for(i = 0; i < n; i++) {
		pthread_join(t[i], NULL);
	}
	return;
}

#define NP 5
#define NC 2

int main(int argc, char *argv[]) {
	pthread_t tp[NP], tc[NC];
	int id_p[NP], id_c[NC];
	
	init_prod_cons();

	cria_threads(tp, id_p, NP, produtor);
	cria_threads(tc, id_c, NC, produtor);

	espera_threads(tp, NP);
	espera_threads(tc, NC);

	return 0;
}