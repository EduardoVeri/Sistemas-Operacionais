/*Um estudante especializando-se em antropologia e interessado em ciência da computação embarcou em 
um projeto de pesquisa para ver se os babuínos africanos podem ser ensinados a respeito de impasses. 
Ele localiza um cânion profundo e amarra uma corda de um lado ao outro, de maneira que os babuínos podem 
atravessá-lo agarrando-se com as mãos. Vários babuínos podem atravessar ao mesmo tempo, desde que todos sigam 
na mesma direção. Se babuínos seguindo na direção leste e outros seguindo na direção oeste se encontrarem 
na corda ao mesmo tempo, ocorrerá um impasse (eles ficarão presos no meio do caminho), pois é impossível 
que um passe sobre o outro. Se um babuíno quiser atravessar o cânion, ele tem de conferir para ver se não 
há nenhum outro babuíno cruzando o cânion no momento na direção oposta. Escreva um programa usando semáforos 
que evite o impasse. Não se preocupe com uma série de babuínos movendo-se na direção leste impedindo indefinidamente 
a passagem de babuínos movendo-se na direção oeste.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex;
int cntLeste = 0;
int cntOeste = 0;
int sinal = -1;

void corda_oeste(){
    while(TRUE){
        sem_wait(&mutex);

        // Se não houver babuinos do lado leste, então o babuino do lado oeste pode atravessar
        if(cntLeste == 0 && (sinal == 0 || sinal == -1)){
            cntOeste = cntOeste + 1; 
            sem_post(&mutex); 

            usar_corda();  // Atravessa a corda

            sem_wait(&mutex); 

            cntOeste = cntOeste - 1; 

            if (cntOeste == 0) 
                sinal = 1; // Sinaliza que os babuinos do lado leste podem atravessar
        }  
        sem_post(&mutex); 
    }
}

void corda_leste(){
    while(TRUE){
        sem_wait(&mutex);

        // Se não houver babuinos do lado oeste, então o babuino do lado leste pode atravessar
        if(cntOeste == 0 && (sinal == 1 || sinal == -1)){
            cntLeste = cntLeste + 1;
            sem_post(&mutex); 

            usar_corda(); // Atravessa a corda

            sem_wait(&mutex); 
            cntLeste = cntLeste - 1; 

            if(cntLeste == 0) 
                sinal = 0;
        }
        sem_post(&mutex); 
    }
}