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

// 세마포어
extern SemaphoreHandle_t adcSemaphoreHandle;

// 큐
extern QueueHandle_t insQueueHandle;
extern QueueHandle_t modeQueueHande;

// 이벤트 그룹
// 모터 드라이빙 비트
#define ROBOT_DRIVING_BIT (1 << 0)

void rtos_init();
void actuator_task();
void instruction_task();
void mode_task();
// 현재 모드 상태를 리턴



#endif /* PRJ_LIB_RTOS_OS_COMMON_H_ */
