/*A SP-125 (Oswaldo Cruz), que leva a Ubatuba, é uma rodovia de pista única com
diversas curvas sinuosas na Serra do Mar. Supondo que na descida da serra, houve um
acidente em uma das curvas, impedindo o tráfego em uma via da pista. Para evitar mais
acidentes nesse trecho e possibilitar que o tráfego flua nos dois sentidos, é necessário
sincronizar o uso da única via disponível entre os veículos que estão descendo (sentido
serra-litoral) e veículos que estão subindo (sentido litoral-serra). Sem uma sincronização
nesse trecho, dois veículos em sentidos opostos poderão usar, ao mesmo tempo, a única
via da pista. O resultado disso é uma situação de deadlock, em que um veículo é impedido
pelo outro de continuar o percurso no seu sentido (no pior caso,os dois veículos irão se
colidir frontalmente). Portanto, um veículo no sentido serra-litoral só poderá atravessar esse
trecho se não houver um veículo vindo no sentido litoral-serra, e vice-versa.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 10 // Número de carros

sem_t mutex;
sem_t sinalSubida;
sem_t sinalDescida;
int cntSubida = 0;
int cntDescida = 0;


void subida(){
    sem_wait(&mutex);
    cntSubida = cntSubida + 1;
    if(cntSubida == 1){
        sem_wait(&sinalDescida);
    }
    sem_post(&mutex);

    sem_wait(&sinalSubida);
    subir_serra();
    sem_post(&sinalSubida)

    sem_wait(&mutex);
    cntSubida = cntSubida - 1;
    if(cntSubida == 0){
        sem_post(&sinalSubida);
    }
    sem_post(&mutex);
}

void descida(){
    sem_wait(&mutex);
    cntDescida = cntDescida + 1;
    if(cntDescida == 1){
        sem_wait(&sinalSubida);
    }

    sem_wait(&sinalDescida);
    descer_serra();
    sem_post(&sinalDescida);

    sem_wait(&mutex);
    cntDescida = cntDescida - 1;
    if(cntDescida == 0){
        sem_post(&sinalSubida);
    }
    sem_post(&mutex);

}