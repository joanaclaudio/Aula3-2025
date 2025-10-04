//
// Created by jlccl on 03/10/2025.
//
#include "sjf.h"     // Header do SJF, onde declaramos a função sjf_scheduler

#include <stdio.h>   // Para perror()
#include <stdlib.h>  // Para malloc/free
#include <unistd.h>  // Para write()

#include "msg.h"     // Estruturas de mensagens usadas para comunicar com as aplicações

/**
 * @brief Shortest Job First (SJF) scheduling algorithm.
 *
 * Seleciona sempre o processo com menor tempo de execução (time_ms)
 * da fila e coloca-o a correr quando a CPU está livre.
 */
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {


    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;
        /*
            *Se a CPU está ocupada (*cpu_task não é NULL):
            *Incrementa o tempo que o processo já executou (ellapsed_time_ms) em cada “tick” do simulador.
        */

        if ((*cpu_task)->ellapsed_time_ms >= (*cpu_task)->time_ms) {
            /*
                *Verifica se o processo já terminou:
                *time_ms é o tempo total que o processo precisa para completar.
                *Se ellapsed_time_ms >= time_ms, o processo terminou.
                *
            */
            // Task finished
            // Send msg to application
            msg_t msg = {
                .pid = (*cpu_task)->pid,
                .request = PROCESS_REQUEST_DONE,
                .time_ms = current_time_ms
            };

            /*
                 *Envia uma mensagem para informar que o processo terminou.
                 *msg contém: PID, tipo de requisição (PROCESS_REQUEST_DONE) e tempo atual.
                 *write envia a mensagem via socket para o processo/aplicação.
                 *
             */
            if (write((*cpu_task)->sockfd, &msg, sizeof(msg_t)) != sizeof(msg_t)) {
                perror("write");
            }
            // Application finished and can be removed (this is FIFO after all)
            /*
                 *Libera a memória do processo terminado.
                 *CPU fica livre (cpu_task = NULL).
             */

            free(*cpu_task);
            *cpu_task = NULL;
        }
    }


    // Se a CPU está livre e a fila de prontos não está vazia, vamos selecionar o próximo processo.

    if (*cpu_task == NULL && rq->head != NULL) {
        /*
         *Inicializa dois ponteiros para percorrer a fila
         *curr percorre todos os elementos da fila.
         *shortest_elem mantém o ponteiro para o processo com menor tempo de execução encontrado até agora.
        */
        queue_elem_t *curr = rq->head; // Nó atual da fila
        queue_elem_t *shortest_elem = rq->head; // Começamos por assumir que o 1º é o mais curto

        /*
         * Percorre toda a fila (while) procurando o processo com menor time_ms.
         * Cada vez que encontra um processo mais curto, atualiza shortest_elem.
        */
        while (curr != NULL) {
            if (curr->pcb->time_ms < shortest_elem->pcb->time_ms) {
                shortest_elem = curr; // Atualizar se encontrarmos um processo mais curto
            }
            curr = curr->next; // Avançar para o próximo nó da fila
        }

        // Agora temos o nó mais curto → remover da fila
        /*
         *Remove o processo mais curto da fila de prontos.
         *remove_queue_elem retorna o nó da fila que foi removido (não libera o PCB).
         */
        queue_elem_t *removed = remove_queue_elem(rq, shortest_elem);

        /*
         *Passa o processo mais curto para a CPU (*cpu_task).
         *Libera apenas o nó da fila, mas não o PCB (porque ele agora está em execução).
         */
        if (removed) {
            *cpu_task = removed->pcb; // Passar o processo mais curto para a CPU
            free(removed);            // Libertar apenas o nó da fila (não o processo!)
        }
    }
}