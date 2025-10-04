#include "fifo.h"

#include <stdio.h>
#include <stdlib.h>

#include "msg.h"
#include <unistd.h>

/**
 * @brief First-In-First-Out (FIFO) scheduling algorithm.
 *
 * This function implements the FIFO scheduling algorithm. If the CPU is not idle it
 * checks if the application is ready and frees the CPU.
 * If the CPU is idle, it selects the next task to run based on the order they were added
 * to the ready queue. The task that has been in the queue the longest is selected to run next.
 *
 * @param current_time_ms The current time in milliseconds.
 * @param rq Pointer to the ready queue containing tasks that are ready to run.
 * @param cpu_task Double pointer to the currently running task. This will be updated
 *                 to point to the next task to run.
 */

/*
     *Função que implementa o escalonamento FIFO.
     *current_time_ms → tempo atual do simulador.
     *rq → ponteiro para a fila de prontos.
     *cpu_task → ponteiro duplo para o processo atualmente em execução na CPU.
*/
void fifo_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task) {


    if (*cpu_task) {
        (*cpu_task)->ellapsed_time_ms += TICKS_MS;      // Add to the running time of the application/task
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
            free((*cpu_task));
            (*cpu_task) = NULL;
        }
    }
    /*
     *cpu_task é um ponteiro para o processo que está rodando na CPU.
     *Se *cpu_task == NULL, significa que a CPU não está ocupada e pode receber um novo processo.
     */
    if (*cpu_task == NULL) {            // If CPU is idle
        /*
         *A função dequeue_pcb retorna o processo que está na cabeça da fila,
         *seguindo a ordem FIFO (primeiro a entrar, primeiro a sair).
         *
         */
        *cpu_task = dequeue_pcb(rq);   // Get next task from ready queue (dequeue from head)
    }
}