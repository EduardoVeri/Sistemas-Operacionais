/* Suponha que uma universidade queira mostrar o quão politicamente correta ela é, aplicando a doutrina “Sepa- rado mas igual é inerentemente desigual” da Suprema Corte dos EUA para o gênero, assim como a raça, termi- nando sua prática de longa data de banheiros segregados por gênero no campus. No entanto, como uma concessão para a tradição, ela decreta que se uma mulher está em um banheiro, outras mulheres podem entrar, mas nenhum homem, e vice-versa. Um sinal com uma placa móvel na porta de cada banheiro indica em quais dos três estados possíveis ele se encontra atualmente: Vazio; Mulheres presentes; Homens presentes. 
Implemente as seguintes rotinas (use as técnicas de sincronização e contadores que quiser): */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

sem_t women, men;
sem_t mutex;
int num_women = 0, num_men = 0;

/* Função unicamente utilizada para manter o semáforo binário */
int get_sem(sem_t *sem) {
    int value;
    sem_getvalue(sem, &value);
    if (value == 0)
        return sem_post(sem);
    return -1;
}

void woman_wants_to_enter() {
    sem_wait(&mutex);
    while (num_men > 0) {
        // Existem Homens no banheiro
        sem_post(&mutex); // Libera o mutex para que os homens possam sair
        sem_wait(&women); // Espera os homens sairem
        sem_wait(&mutex); // Entra na região crítica novamente
    } 

    sem_wait(&men); // Bloqueia os homens de entrar

    // Libera todas as mulheres que estão esperando
    while(get_sem(&women) != -1); 
    
    num_women++;
    sem_post(&mutex);
}

void man_wants_to_enter() {
    sem_wait(&mutex);
    while (num_women > 0) {
        // Existem mulheres no banheiro
        sem_post(&mutex);
        sem_wait(&men); // Espera as mulheres sairem
        sem_wait(&mutex);
    }
    
    sem_wait(&women); // Bloqueia as mulheres de entrar 

    // Libera todos os homens que estão esperando
    while(get_sem(&men) != -1); 
    
    num_men++;
    sem_post(&mutex);
}

void woman_leaves() {
    sem_wait(&mutex);
    num_women--;
    if (num_women == 0) {
        sem_post(&men); // Libera os homens
    }
    sem_post(&mutex);
}

void man_leaves() {
    sem_wait(&mutex);
    num_men--;
    if (num_men == 0) {
        sem_post(&women); // Libera as mulheres
    }
    sem_post(&mutex);
}