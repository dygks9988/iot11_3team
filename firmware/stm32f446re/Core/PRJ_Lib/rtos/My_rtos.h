/*
 * My_rtos.h
 *
 *  Created on: Sep 18, 2026
 *      Author: pc
 */

#ifndef PRJ_LIB_RTOS_MY_RTOS_H_
#define PRJ_LIB_RTOS_MY_RTOS_H_

//FreeRTOS API
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

extern SemaphoreHandle_t adcSemaphoreHandle;
extern QueueHandle_t insQueueHandle;

void actuator_task();
void motor_instruction_task();

#endif /* PRJ_LIB_RTOS_MY_RTOS_H_ */
