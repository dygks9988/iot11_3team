/*
 * actuator_task.c
 *
 *  Created on: Sep 18, 2026
 *      Author: pc
 */

#include <c_stdlib.h>
// FreeRTOS API
#include <os_common.h>

// Prj_Lib
#include "actuator.h"
#include "msg.h"

void actuator_task(){
	Motor_Instruction_MsgTypeDef receive_msg = {0,0,0};
	for(;;){
		if (xQueueReceive(insQueueHandle,&receive_msg, portMAX_DELAY) == pdTRUE){
			actuator_process(&receive_msg);
		}
	}
}
