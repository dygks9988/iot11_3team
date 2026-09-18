/*
 * os_common.h
 *
 *  Created on: Sep 18, 2026
 *      Author: pc
 */

#ifndef PRJ_LIB_RTOS_OS_COMMON_H_
#define PRJ_LIB_RTOS_OS_COMMON_H_

//FreeRTOS API
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

extern SemaphoreHandle_t adcSemaphoreHandle;
extern QueueHandle_t insQueueHandle;

void rtos_init();
void actuator_task();
void instruction_task();



#endif /* PRJ_LIB_RTOS_OS_COMMON_H_ */
