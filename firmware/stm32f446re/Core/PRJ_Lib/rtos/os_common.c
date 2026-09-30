#include "main.h"

//FreeRTOS API
#include "os_common.h"

#include "msg.h"
#include "robot_mode.h"
#include "My_ARM_RTOS_UART_Lib_V5_2.h"
#include "modbus_protocol.h"

SemaphoreHandle_t adcSemaphoreHandle;
QueueHandle_t insQueueHandle;
QueueHandle_t modeQueueHandle;
QueueHandle_t modbQueueHandle;


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
	modeQueueHandle = xQueueCreate(10,sizeof(Robot_CmdTypeDef));
	if(modeQueueHandle == NULL){
		Error_Handler();
	}
	modbQueueHandle = xQueueCreate(10,sizeof(ModbusRxMsgTypeDef));
	if(modbQueueHandle == NULL){
		Error_Handler();
	}
}
