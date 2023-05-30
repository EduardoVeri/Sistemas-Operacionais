/* Eh mais facil do que eu pensei, basta criar dois vetores, um para o swap e outro para a memoria real. Com isso, criar indices entre os valores de MS + MR. */

/* Inicializar esses vetores com valores iniciais, que deve ser mantido para todos os demais testes, ate para algoritmos distintos. Esses dados de inicializacao podem ser gerados em um arquivo externo
inicializa_vetor.txt */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define RAM_SIZE 4096
#define SWAP_SIZE 4096
#define MAX_CONT 1000000

typedef unsigned long long int tempo_t; 

typedef struct page_t {

    unsigned char referenciada : 1; // Bit de referencia
    unsigned char modificada : 1; // Bit de modificacao
    unsigned char pres_aus : 1; // Bit de presenca/ausencia
    unsigned char pad : 5; // Padding
    unsigned int moldura_pag; // Indice da moldura da pagina
    unsigned char MRU_count; // Contador para o algoritmo MRU
    unsigned short WS_tempo_virtual; // Tempo virtual para o algoritmo WS
    /* Adicionar mais detalhes  */

    struct page_t* proximo_fila; // Proxima pagina
    struct page_t* anterior_fila; // Pagina anterior

} page_t;

// ===== Estruturas para a Fila utilizado no algoritmo FIFO ===== //
typedef struct fila{
    int tamanho;    
    page_t* inicio;
    page_t* fim;
} fila_t;

fila_t* inicia_fila(){
    fila_t* fila =  (fila_t*)malloc(sizeof(fila_t));
    fila->fim = NULL;
    fila->inicio = NULL;
    fila->tamanho = 0;

    return fila;
}

void inserir_fila(fila_t* fila, page_t* page){
    if(fila->tamanho == 0){
        fila->inicio = page;
        fila->fim = page;
        fila->tamanho++;

        page->anterior_fila = NULL;
        page->proximo_fila = NULL;
        
        return;
    }

    page_t* final_fila = fila->fim;

    final_fila->proximo_fila = page;
    page->anterior_fila = final_fila;
    page->proximo_fila = NULL;

    fila->fim = page;
    fila->tamanho++;

    return;
}

page_t* remover_fila(fila_t* fila){
    if(fila->tamanho == 0){
        return NULL;
    }

    page_t* pagina_removida = fila->inicio;
    fila->tamanho--;
    
    if(fila->tamanho == 1){
        fila->inicio = NULL;
        fila->fim = NULL;

        return pagina_removida;
    }

    fila->inicio = pagina_removida->proximo_fila;
    return pagina_removida;
}

// ===== Estruturas para acessos a memoria ===== //

int gerar_acesso_memoria(FILE* arquivo, int *moldura, int* modificado){

    if (!arquivo){
        printf("Arquivo nao encontrado!\n");
        return -1;
    }

    if(fscanf(arquivo, "%d %d", modificado, moldura) == EOF){
        printf("Fim do arquivo!\n");
        return -1;
    }

    return 0;
}

void inicializa_vetor(page_t* vetor[], int tamanho){
	for(int i = 0; i < tamanho; i++){
		vetor[i] = NULL;
	}
}

void liberar_vetor(page_t* vetor[], int tamanho){
    for(int i = 0; i < tamanho; i++){
        if(vetor[i] != NULL){
            free(vetor[i]);
            vetor[i] = NULL;
        }
    }
}

int encontrar_espaco_livre_RAM(page_t* MR[]){
	for(int i = 0; i < RAM_SIZE; i++){
		if(MR[i] == NULL){
			return i;
		}
	}
	return -1;
}

int encontrar_indice(page_t* vetorMR[], page_t* vetorMS[], int indice){
    for(int i = 0; i < RAM_SIZE; i++){
        if(vetorMR[i] == NULL){
            printf("Erro: Indice null encontrado na memoria real (%d)!\n", i);
        }
		else if(vetorMR[i]->moldura_pag == indice){
			return i;
		}
	}

    for(int i = 0; i < SWAP_SIZE; i++){
        if(vetorMS[i] == NULL){
            printf("Erro: Indice null encontrado na memoria real (%d)!\n", i);
        }
        else if(vetorMS[i]->moldura_pag == indice){
            return i + RAM_SIZE;
        }
    }

    // Ele tem que estar obrigatoriamente em algum dos dois vetores
    printf("Erro! Pagina '%d' nao encontrada!\n", indice);
	return -1;
}

int incluir_paginas_iniciais(page_t* MR[], page_t* MS[], fila_t* fila){

    /* Aqui deve ser lido o arquivo inicializa_vetor.txt e incluir os valores
    no vetor MR e MS. Os vetores devem estar totalmente preenchidos. */

    /* Ao incluir os valores na lista, eh importante verificar se sera preciso
    modificar alguma coisa com base nos demais algoritmos presentes aqui. Por exemplo
    o algoritmo fifo precisa que os valores adicionados na RAM seja guardados em uma fila. */

    FILE* arquivo = fopen("valores_iniciais.txt", "r");

    if(arquivo == NULL){
        printf("Erro: Nao foi possivel abrir o arquivo de valores iniciais!\n");
        return -1;
    }

    unsigned int modificado = 0;
    unsigned int moldura = 0;

    page_t* pagina = NULL;

    for(int i = 0; i < (RAM_SIZE + SWAP_SIZE); i++){
        if(!fscanf(arquivo, "%d", &moldura)){
            printf("Erro: Leitura do dado inicial %d\n", i);
            return -1;
        }

        pagina = (page_t*)malloc(sizeof(page_t));
        pagina->MRU_count = 0;
        pagina->WS_tempo_virtual = 0;
        pagina->modificada = 0;
        pagina->referenciada = 0;
        pagina->pres_aus = 0;
        pagina->modificada = 0;
        pagina->moldura_pag = moldura;
        pagina->proximo_fila = NULL;
        pagina->anterior_fila = NULL;

        if(i < RAM_SIZE){
            MR[i] = pagina; // Os primeiros serao adicionados na memoria RAM
            inserir_fila(fila, pagina); // Adiciona todos as paginas que foram colocadas na RAM
        }
        else{
            MS[i - RAM_SIZE] = pagina; // Os demais serao adicionados na memoria SWAP
        }
    }

    printf("Valores iniciais adicionados com sucesso!\n");

    fclose(arquivo);

    return 0;
}

// ======= Estruturas e Funcoes - Algoritmo NUR ======= //

void NUR (page_t** MR, page_t** MS, int indice) {

    int c0 = 0, c1 = 0, c2 = 0, c3 = 0;

    //aq foi pra ram foi referenciada (1)
    //aq foi pro swap bota 0 no referenciada
    
    int* classe0 = NULL;   
    int* classe1 = NULL;
    int* classe2 = NULL;
    int* classe3 = NULL;

    page_t* aux;

    //percorre todas pag da mem real
    for (int i = 0; i < RAM_SIZE; i++) {

        if ((MR[i]->modificada == 0) && (MR[i]->referenciada == 0)) { //classe 0

            if (classe0 == NULL)
                classe0 = (int*) malloc(++c0 * sizeof(int));

            else
                classe0 = (int*) realloc(classe0,  sizeof(int) * (++c0));

            classe0[c0 -1] = i;
            
        } else if ((MR[i]->modificada == 1) && (MR[i]->referenciada == 0)) { //classe 1

            if (classe1 == NULL)
                classe1 = (int*) malloc(++c1 * sizeof(int));

            else
                classe1 = (int*) realloc(classe1,  sizeof(int) * (++c1));

            classe1[c1 -1] = i;

        } else if ((MR[i]->modificada == 0) && (MR[i]->referenciada == 1)) { //classe 2

            if (classe2 == NULL)
                classe2 = (int*) malloc(++c2 * sizeof(int));

            else
                classe2 = (int*) realloc(classe2,  sizeof(int) * (++c2));

            classe2[c2 -1] = i;

        } else if ((MR[i]->modificada == 1) && (MR[i]->referenciada == 1)) { //classe 3

            if (classe3 == NULL)
                classe3 = (int*) malloc(++c3 * sizeof(int));

            else
                classe3 = (int*) realloc(classe3,  sizeof(int) * (++c3));

            classe3[c3 -1] = i;
        }
    }

    if (classe0 != NULL) {

        //escolhe um valor rand dentro do tamanho do vetor para remover
        int i = (rand() % c0);
        
        aux = MR[classe0[i]];
        MR[classe0[i]] = MS[indice - RAM_SIZE];
        MS[indice - RAM_SIZE] = aux;

        MR[classe0[i]]->referenciada = 1;
        MS[indice - RAM_SIZE]->referenciada = 0;
        
    } else if (classe1 != NULL) {

        //escolhe um valor rand dentro do tamanho do vetor para remover
        int i = (rand() % c1);
        
        aux = MR[classe1[i]];
        MR[classe1[i]] = MS[indice - RAM_SIZE];
        MS[indice - RAM_SIZE] = aux;

        MR[classe1[i]]->referenciada = 1;
        MS[indice - RAM_SIZE]->referenciada = 0;
        
    } else if (classe2 != NULL) {

        //escolhe um valor rand dentro do tamanho do vetor para remover
        int i = (rand() % c2);
        
        aux = MR[classe2[i]];
        MR[classe2[i]] = MS[indice - RAM_SIZE];
        MS[indice - RAM_SIZE] = aux;

        MR[classe2[i]]->referenciada = 1;
        MS[indice - RAM_SIZE]->referenciada = 0;

    } else {

        //escolhe um valor rand dentro do tamanho do vetor para remover
        int i = (rand() % c3);
        
        aux = MR[classe3[i]];
        MR[classe3[i]] = MS[indice - RAM_SIZE];
        MS[indice - RAM_SIZE] = aux;

        MR[classe3[i]]->referenciada = 1;
        MS[indice - RAM_SIZE]->referenciada = 0;
    }   

    free(classe0);
    free(classe1);
    free(classe2);
    free(classe3);
    
    return;
}

// ============ Algoritmo FIFO ============ //

void FIFO(page_t* MR[], page_t* MS[], fila_t* fila, int indice, int modificado){
    /* Esse algoritmo armazena todos os processos da RAM em uma fila. 
    Dessa forma, essa fila tem que ser criada na main e passada como argumento
    aqui nessa funcao! */
    
    page_t* pagina_removida = remover_fila(fila); // Remove a cabeca da fila
    page_t* pagina_escolhida = MS[indice - RAM_SIZE];

    int indice_pag_removida = encontrar_indice(MR, MS, pagina_removida->moldura_pag);
    
    // Indice tem que ser na Mem RAM
    if (indice_pag_removida >= RAM_SIZE){
        printf("Erro: Indice incorreto no algoritmo FIFO!\n");
        return;
    }

    MR[indice_pag_removida] = pagina_escolhida;
    MS[indice - RAM_SIZE] = pagina_removida;

    inserir_fila(fila, pagina_escolhida); // Insere a pagina que estava no SWAP na fila
    
    pagina_escolhida->referenciada = 1;
    pagina_escolhida->modificada = modificado;
    pagina_removida->referenciada = 0;
    pagina_removida->modificada = 0;
}

// ======= Estruturas e Funcoes - Algoritmo MRU ======= //

void MRU(page_t* MR[], page_t* MS[], int indice, int modificado){
    int menor_contador = MAX_CONT;
    int localizacao_menor_contador = -1;
    
    for(int i = 0; i < RAM_SIZE; i++){
        if(MR[i]->MRU_count < menor_contador){
            menor_contador = MR[i]->MRU_count;
            localizacao_menor_contador = i;
        }
    }

    page_t* aux = MR[localizacao_menor_contador];

    aux->MRU_count = 0;
    aux->modificada = 0;
    aux->referenciada = 0;
    
    MR[localizacao_menor_contador] = MS[indice-RAM_SIZE];
    MS[indice - RAM_SIZE] = aux;

    aux = MR[localizacao_menor_contador];
    aux->referenciada = 1;
    aux->modificada = modificado;
    aux->MRU_count = 1;

}

/* Funcao inspirada no arduino, que pega um valor de um contador 
interno e devolve para quem o chamou. Assim podemos fazer certas 
coisas quando o tempo for ultrapassado */
tempo_t millis(){
    clock_t now = clock();
    return now * 1000 / CLOCKS_PER_SEC;
}


int main(int argc, char** argv) { 

    if(argc < 2 || argc > 5){
        printf("%s <Num Alg> <Temp Interr> <Nome Arq Teste>\n", argv[0]);
        printf("1) Nao Usada Recentemente (NUR)\n");
        printf("2) First in First out (FIFO)\n");
        printf("3) Menos Recentemente Usada (MRU)\n");
        return 0;
    }

    int num_iteracao;

    FILE* arquivo = fopen(argv[4], "r");
    if(arquivo == NULL){
        printf("Erro ao abrir o arquivo!\n");
        return 0;
    }
    if(fscanf(arquivo, "%d", &num_iteracao) == 0){
        printf("Erro: Arquivo Vazio!\n");
        return 0;
    }

    // Qual algoritmo vai ser usado. 1 = NUR, 2 = FIFO, 3 = MRU
    int algoritmo = atoi(argv[1]);
    if(algoritmo > 3 || algoritmo < 1){
        printf("Numero do algoritmo fora do itervalo!\n");
        return 0;
    }

    int selec_tempo = atoi(argv[2]);
    if(selec_tempo < 0 || selec_tempo > 1){
        printf("0) Tempo continuo\n1) Tempo Discreto!\n");
        return 0;
    }

    int tempo_interrupcao = atoi(argv[3]);
    if(tempo_interrupcao < 0){
        printf("Tempo de interrupcao nao pode ser negativo!\n");
        return 0;
    }
    
    int moldura;
    int modificado;
	int indice;

    int page_miss_count = 0;
    int page_hit_count = 0;
    int interrupt_count = 0;

    tempo_t tempo_anterior = millis();    
	page_t* MR[RAM_SIZE];
	page_t* MS[SWAP_SIZE];

    fila_t* fila = inicia_fila();

	inicializa_vetor(MR, RAM_SIZE);
	inicializa_vetor(MS, SWAP_SIZE);

	if(incluir_paginas_iniciais(MR, MS, fila) == -1){
        liberar_vetor(MR, RAM_SIZE);
        liberar_vetor(MS, SWAP_SIZE);
        free(fila);
        return 0;
    }

	for(int i=0; i<num_iteracao; i++){
        /* Criar o verificador de contador aqui */
        
        switch(selec_tempo){
            case 0:
                if ((millis() - tempo_anterior) > tempo_interrupcao){
                    // Zerar bits de referencia e os contadores
                    for(int i = 0; i < RAM_SIZE; i++){
                        MR[i]->referenciada = 0;
                        MR[i]->MRU_count = 0;
                    }
                    interrupt_count++;
                    tempo_anterior = millis();
                }
                break;
            
            case 1:
                if(i%tempo_interrupcao == 0){
                    // Zerar bits de referencia e os contadores
                    for(int i = 0; i < RAM_SIZE; i++){
                        MR[i]->referenciada = 0;
                        MR[i]->MRU_count = 0;
                    }
                    interrupt_count++;
                    
                }
        }


        /* Fazer um laco com gerando os numeros aleatorios
        para poder realizar o acesso na memoria */
        /* Alem de gerar um numero da memoria, gerar tbm se ele
        vai modificar ou nao ele */
        if(gerar_acesso_memoria(arquivo, &moldura, &modificado) == -1){ 
            continue; // Continua caso algum erro tenha acontecido
        }

		/* Verificar se a moldura esta na memoria real */
        if((indice = encontrar_indice(MR, MS, moldura)) == -1){
            continue; // Continua caso algum erro tenha ocorrido
        } 

        if(indice >= RAM_SIZE){
            /* Page Miss */
            page_miss_count++;
            /* Se a moldura estiver na memoria secundaria, entao
            deve ser movida para a memoria real */
            int indice_livre = encontrar_espaco_livre_RAM(MR);
            if(indice_livre == -1){
                /* Se nao houver espaco livre na memoria real, entao
                deve ser feita a substituicao de pagina */
                switch(algoritmo){
                    case 1:
                        // Colocar o BIT modificado, pois faltou fazer isso
                        NUR(MR, MS, indice);    
                        break;
                    
                    case 2:
                        FIFO(MR, MS, fila, indice, modificado);
                        break;

                    case 3:
                        MRU(MR, MS, indice, modificado);
                        break;
                }
            }
            else{
                /* Se houver espaco livre na memoria real, entao
                deve ser movida a pagina da memoria secundaria para a memoria real */
                MR[indice_livre] = MS[indice - RAM_SIZE];
                MS[indice - RAM_SIZE] = NULL;
                /* Teoricamente em nossos testes isso nunca vai acontecer!!! */
            }
        }
        else{
            /* Page Hit */
             switch(algoritmo){      
                    case 1:
                        /* Pensar o que colocar aqui */
                        break;
                    
                    case 2:
                        /* Pensar o que colocar aqui */
                        break;

                    case 3:
                        MR[indice]->MRU_count++; // Aumenta o contador
                        break;
            }

            /* Se a moldura estiver na memoria real, entao alterar seu bit de referencia */
            MR[indice]->referenciada = 1;
            MR[indice]->modificada = modificado;
        }
    }

    // Liberar as estruturas criadas
    liberar_vetor(MR, RAM_SIZE);
    liberar_vetor(MS, SWAP_SIZE);
    free(fila);
    
    printf("Page Miss: %d\n", page_miss_count);
    printf("Interrupcoes: %d\n", interrupt_count);

    return 0; 
}