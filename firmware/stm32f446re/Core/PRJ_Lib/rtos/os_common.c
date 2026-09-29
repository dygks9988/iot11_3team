#include "main.h"

//FreeRTOS API
#include "os_common.h"

#include "msg.h"
#include "robot_mode.h"


SemaphoreHandle_t adcSemaphoreHandle;
QueueHandle_t insQueueHandle;
QueueHandle_t modeQueueHande;



void rtos_init(void)
{
	adcSemaphoreHandle = xSemaphoreCreateBinary();
	if(adcSemaphoreHandle == NULL){
		Error_Handler();
	}
	insQueueHandle = xQueueCreate(10,sizeof(Motor_Instruction_MsgTypeDef));
	if(insQueueHandle == NULL){
		Error_Handler();
	}
	modeQueueHande = xQueueCreate(10,sizeof(Robot_CmdTypeDef));
	if(modeQueueHande == NULL){
		Error_Handler();
	}

}
