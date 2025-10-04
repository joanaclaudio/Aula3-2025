//
// Created by jlccl on 03/10/2025.
//
#include "rr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "msg.h"

#define TIME_SLICE_MS 500

void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {


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
            // Application finished and can be removed (this is FIFO after all)
            /*
                 *Libera a memória do processo terminado.
                 *CPU fica livre (cpu_task = NULL).
             */
            free(*cpu_task);
            *cpu_task = NULL;
        }

        /*
         *Se o processo não terminou, mas já usou
         todo o seu time slice (TIME_SLICE_MS), ele é preemptado.
         *enqueue_pcb(rq, *cpu_task) coloca o processo no final da fila de prontos,
         garantindo que todos os processos tenham chance de rodar.
         *CPU fica livre (*cpu_task = NULL) para pegar o próximo processo da fila.
         */
        else if ((*cpu_task)->ellapsed_time_ms % TIME_SLICE_MS == 0) {
            // reinserir no final da fila
            enqueue_pcb(rq, *cpu_task);
            *cpu_task = NULL;
        }
    }
    /*
     *Se a CPU está livre e há processos na fila de prontos,
     remove o próximo processo da fila (dequeue_pcb) e o coloca na CPU.
     *Mantém a ordem FIFO dentro da fila,
     garantindo que processos antigos não fiquem “presos”.

     */
    if (*cpu_task == NULL && rq->head != NULL) {
        *cpu_task = dequeue_pcb(rq);
    }
}