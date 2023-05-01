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
int cntSubida = 0;
int cntDescida = 0;

void subida(){
    while(TRUE){
        sem_wait(&mutex);

        // Se não há carros descendo, pode subir
        if(cntDescida == 0){
            cntSubida = cntSubida + 1; // Incrementa o contador de carros subindo
            sem_post(&mutex); // Libera para que outros carros possam subir

            subir_serra(); // Carro sobe a serra

            sem_wait(&mutex); // Entra na região crítica
            cntSubida = cntSubida - 1; // Carrro terminou de subir, decrementa o contador de carros subindo
        }  
        sem_post(&mutex); // Sai da região crítica
    }
}

void descida(){
    while(TRUE){
        sem_wait(&mutex);

        // Se não há carros subindo, pode descer
        if(cntSubida == 0){
            cntDescida = cntDescida + 1;
            sem_post(&mutex); // Libera para que outros carros possam descer

            descer_serra(); // Carro desce a serra

            sem_wait(&mutex); // Entra na região crítica
            cntDescida = cntDescida - 1; 
        }
        sem_post(&mutex); // Sai da região crítica
    }
}