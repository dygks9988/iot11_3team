/*
 * motor_instruction_task.c
 *
 *  Created on: Sep 18, 2026
 *      Author: pc
 */

#include <c_stdlib.h>
//FreeRTOS API
#include <os_common.h>

//My_Lib
#include "My_ARM_RTOS_ADC_Lib_V3_2.h"

//Prj_Lib
#include "motor_instruction.h"
#include "msg.h"

#define debug 1

void instruction_task(){
	Motor_Instruction_MsgTypeDef instruction_msg = {0,0,0};
	for(;;){
		if (xSemaphoreTake(adcSemaphoreHandle, portMAX_DELAY) == pdTRUE)
		 {
#if debug
			for(int i = 0; i<5;i++){
			printf("ad_buf%d = %u\r\n",i,adc->ad_buf[i]);
			}
#endif
		    motor_instruction_create(&instruction_msg,adc->ad_buf);
		    xQueueSend(insQueueHandle,&instruction_msg,10);
		 }
	}
}
