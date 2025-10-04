//
// Created by jlccl on 03/10/2025.
//

#ifndef MLFQ_H
#define MLFQ_H
#include <stdint.h>
#include "queue.h"


// Número de níveis de prioridade no MLFQ
#define MLFQ_LEVELS 3

// Time slice em milissegundos
#define MLFQ_TIME_SLICE 500

/**
 * @brief Função que implementa o escalonador Multi-Level Feedback Queue.
 *
 * @param current_time_ms Tempo atual em milissegundos.
 * @param rq Array de ponteiros para filas de prontos, uma para cada nível de prioridade.
 * @param cpu_task Ponteiro duplo para o processo atualmente em execução na CPU.
 * @param current_level Ponteiro para armazenar o nível atual de prioridade do processo em CPU.
 */
void mlfq_scheduler(uint32_t current_time_ms, queue_t *rq[], pcb_t **cpu_task, int *current_level);

#endif //MLFQ_H
