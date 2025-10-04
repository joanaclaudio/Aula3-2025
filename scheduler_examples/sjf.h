//
// Created by jlccl on 03/10/2025.
//

#ifndef SJF_H
#define SJF_H
#include <stdint.h>  // Para tipos como uint32_t
#include "queue.h"   // Para podermos usar queue_t e pcb_t

/**
 * @brief Shortest Job First scheduler
 *
 * Esta função implementa o algoritmo SJF:
 * - Sempre que a CPU está livre, escolhe o processo com menor tempo total (time_ms)
 *   da fila.
 * - Controla o tempo decorrido do processo em execução e envia mensagem quando termina.
 *
 */
void sjf_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);
#endif //SJF_H
