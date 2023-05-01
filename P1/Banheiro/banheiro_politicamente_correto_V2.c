#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t women, men, mutex; // Semáforos inicializados com 1
int num_women = 0, num_men = 0;

void woman_wants_to_enter() {
    sem_wait(&women); // Se o semáforo for 1, mulher pode entrar
    sem_wait(&mutex); // Entra na região crítica
    num_women++;
    // Se for a primeira mulher, deve impedir que homens entrem
    if (num_women == 1) {
        sem_wait(&men); // Decrementa o semáforo dos homens
    }
    sem_post(&mutex); // Sai da região crítica
    sem_post(&women);
}

void woman_leaves() {
    sem_wait(&women);
    sem_wait(&mutex); // Entra na região crítica 
    num_women--;
    if (num_women == 0) {
        sem_post(&men); // Se for a última mulher, libera os homens
    }
    sem_post(&mutex); // Sai da região crítica
    sem_post(&women);
}

void man_wants_to_enter() {
    sem_wait(&men); // Se o semáforo for 1, homem pode entrar
    sem_wait(&mutex); // Entra na região crítica
    num_men++;
    if (num_men == 1) {
        // Se for o primeiro homem, deve impedir que as mulheres entrem
        sem_wait(&women); 
    }
    sem_post(&mutex); // Sai da região crítica
    sem_post(&men);
}

void man_leaves() {
    sem_wait(&men);
    sem_wait(&mutex); // Entra na região crítica
    num_men--;
    if (num_men == 0) {
        sem_post(&women); // Se for o último homem, libera as mulheres
    }
    sem_post(&mutex); // Sai da região crítica
    sem_post(&men);
}

int main() {
    sem_init(&women, 0, 1);
    sem_init(&men, 0, 1);

    // Run some example code to test the bathroom
    for (int i = 0; i < 10; i++) {
        pthread_t woman_thread, man_thread;
        pthread_create(&woman_thread, NULL, (void *)woman_wants_to_enter, NULL);
        pthread_create(&man_thread, NULL, (void *)man_wants_to_enter, NULL);
        pthread_join(woman_thread, NULL);
        pthread_join(man_thread, NULL);
    }

    sem_destroy(&women);
    sem_destroy(&men);

    return 0;
}