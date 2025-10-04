//
// Created by jlccl on 03/10/2025.
//
// mlfq.c - Escalonador Multi-Level Feedback Queue (estilo FIFO/SJF)
//

#include "mlfq.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"

// ---------------------------------------------------------------
// Definições do MLFQ
// ---------------------------------------------------------------

#define MLFQ_LEVELS 3        // Número de níveis de prioridade
#define MLFQ_TIME_SLICE 500  // 500ms por time slice

/**
 * @brief Escalonador MLFQ simplificado (estilo FIFO/SJF)
 *
 * @param current_time_ms Tempo atual em milissegundos
 * @param rq Fila ativa (nível atual)
 * @param cpu_task Ponteiro para o ponteiro do processo atualmente na CPU
 * @param current_level Ponteiro para o nível da fila do processo atual
 */
void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq[], pcb_t **cpu_task, int *current_level) {

    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;

        /*
             *Se a CPU está ocupada (*cpu_task não é NULL):
             *Incrementa o tempo que o processo já executou (ellapsed_time_ms) em cada “tick” do simulador.
             *
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


            /*
                 *Libera a memória do processo terminado.
                 *CPU fica livre (cpu_task = NULL).
             */
            free(*cpu_task);
            *cpu_task = NULL;
        }

        //Se o time slice acabou, rebaixa o processo

        /*
         *Verifica se o processo atual usou todo o seu time slice (MLFQ_TIME_SLICE).
         *ellapsed_time_ms % MLFQ_TIME_SLICE == 0 indica que o processo completou
         exatamente um múltiplo do time slice, então precisa ser rebaixado de prioridade.

         */
        else if ((*cpu_task)->ellapsed_time_ms > 0 &&
         (*cpu_task)->ellapsed_time_ms % MLFQ_TIME_SLICE == 0){
            /*
             *Determina o próximo nível de prioridade do processo.
             *Se o processo já está no nível mais baixo (MLFQ_LEVELS - 1),
             não pode descer mais, então mantém no último nível.
            */
            int next_level = *current_level + 1;
            if (next_level >= MLFQ_LEVELS)
                next_level = MLFQ_LEVELS - 1;

            /*
             *Adiciona o processo no final da fila do nível de prioridade inferior.
             *Isso garante que processos que usam muito tempo
             sejam penalizados e tenham menos acesso imediato à CPU.
             */
            enqueue_pcb(rq[next_level], *cpu_task);

            // Marca que a CPU está livre, permitindo
            // que o escalonador selecione outro processo da fila mais alta no próximo ciclo.
            *cpu_task = NULL;
        }
    }


    //Se CPU estiver livre, pega o processo da fila de maior prioridade
    /*Verifica se não há processo a rodar na CPU.*/
    if (*cpu_task == NULL) {

        /*Percorre os níveis de prioridade do mais alto ao mais baixo.*/
        for (int lvl = 0; lvl < MLFQ_LEVELS; lvl++) {
            // Confirma se a fila do nível lvl tem processos prontos.
            if (rq[lvl]->head != NULL) {
                // Retira o primeiro processo da fila e coloca para rodar na CPU.
                *cpu_task = dequeue_pcb(rq[lvl]);

                // Atualiza o nível atual do processo na CPU, necessário para decidir rebaixamentos futuros.
                *current_level = lvl;

                // Para de percorrer as filas, pois já encontrou o processo de maior prioridade disponível.
                break;
            }
        }
    }
}