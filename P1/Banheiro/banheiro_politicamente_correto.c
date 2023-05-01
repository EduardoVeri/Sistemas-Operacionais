/* Suponha que uma universidade queira mostrar o quão politicamente correta ela é, aplicando a doutrina “Sepa- rado mas igual é inerentemente desigual” da Suprema Corte dos EUA para o gênero, assim como a raça, termi- nando sua prática de longa data de banheiros segregados por gênero no campus. No entanto, como uma concessão para a tradição, ela decreta que se uma mulher está em um banheiro, outras mulheres podem entrar, mas nenhum homem, e vice-versa. Um sinal com uma placa móvel na porta de cada banheiro indica em quais dos três estados possíveis ele se encontra atualmente: Vazio; Mulheres presentes; Homens presentes. 
Implemente as seguintes rotinas (use as técnicas de sincronização e contadores que quiser): */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

sem_t women, men;
sem_t mutex;
int num_women = 0, num_men = 0;

void woman_wants_to_enter() {
    sem_wait(&mutex);
    if (num_men > 0) {
        printf("Wait for men to leave\n");
        sem_post(&mutex); // Libera o mutex para que os homens possam sair
        sem_wait(&women); // Espera os homens sairem
        sem_wait(&mutex); // Entra na região crítica novamente
    } else {
        sem_post(&women); // Aumenta o contador de mulheres
    }
    num_women++;
    printf("Women present\n");
    sem_post(&mutex);
}

void man_wants_to_enter() {
    sem_wait(&mutex);
    if (num_women > 0) {
        printf("Wait for women to leave\n");
        sem_post(&mutex);
        sem_wait(&men);
        sem_wait(&mutex);
    } else {
        sem_post(&men);
    }
    num_men++;
    printf("Men present\n");
    sem_post(&mutex);
}

void woman_leaves() {
    sem_wait(&mutex);
    num_women--;
    if (num_women == 0) {
        sem_post(&men);
        printf("Men can enter\n");
    }
    printf("Women left\n");
    sem_post(&mutex);
}

void man_leaves() {
    sem_wait(&mutex);
    num_men--;
    if (num_men == 0) {
        sem_post(&women);
        printf("Women can enter\n");
    }
    printf("Men left\n");
    sem_post(&mutex);
}