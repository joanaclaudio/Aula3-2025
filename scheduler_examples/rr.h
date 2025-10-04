//
// Created by jlccl on 03/10/2025.
//

#ifndef RR_H
#define RR_H
#include <stdint.h>
#include "queue.h"   // Para pcb_t e queue_t

/**
 * @brief Round-Robin (RR) scheduling algorithm
 *
 * Executa cada processo por um time slice fixo de 500ms.
 *
 */
void rr_scheduler(uint32_t current_time_ms, queue_t *rq, pcb_t **cpu_task);
#endif //RR_H
