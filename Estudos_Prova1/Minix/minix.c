/* registro parcialmente apresentado */
struct proc {
    char p_priority; /* prioridade do processo */
    u64_t p_cpu_time_left; /* tempo restante de CPU */
    unsigned p_quantum_size_ms; /* Quantum atribuído em ms */
    struct proc *p_nextready; /* próximo processo */
};

void sched(struct proc * rp, int *q, int *front) {
    /* Determina a fila e a posição de inserção. */

    /* Se o processo não tem tempo de CPU restante, ele é colocado no fim da fila. */
    if (rp->p_cpu_time_left == 0) {
        rp->p_cpu_time_left = rp->p_quantum_size_ms; // resetar o tempo de CPU
        *front = 0; // adiciona no fim da fila

        if(rp->p_priority != rp->min_priority)
            rp->p_priority++;
    }
    /* Se o processo tem tempo de CPU restante, ele é colocado no início da fila. */
    else {
        if(rp->p_priority != rp->max_priority)
            rp->p_priority--;

        *front = 1;
    }

    *q = rp->p_priority;

}

void enqueue(struct proc *rp) {
    int q; /* fila a ser usada */
    int front; /* adiciona na frente ou atrás */
    /* Determina q e front para inserir rp. */
    sched(rp, &q, &front);
    /* Agora adiciona o processo na fila. */
    if (rdy_head[q] == NIL_PROC) { /* fila vazia */
        rdy_head[q] = rdy_tail[q] = rp;
        rp->p_nextready = NIL_PROC;
    }
    else if (front) { /* adiciona no início da fila */
        rp->p_nextready = rdy_head[q];
        rdy_head[q] = rp;
    }
    else { /* adiciona no fim da fila */
        rdy_tail[q]->p_nextready = rp;
        rdy_tail[q] = rp;
        rp->p_nextready = NIL_PROC;
    }
    /* Agora seleciona o próximo processo para rodar. */
    pick_proc(); /* tipicamente rdy_head do menor q */
}
