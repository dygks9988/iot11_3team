/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "usart.h"
#include "adc.h"
#include "queue.h"

#include <stdio.h>
#include <My_MCU_Printf_Lib_V2_8.h>
#include "modbus_protocol.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

extern uint8_t uart_buf[DMA_Rx_Lens];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern sensorData s_data;
/* USER CODE END Variables */
/* Definitions for RS485Task */
osThreadId_t RS485TaskHandle;
const osThreadAttr_t RS485Task_attributes = {
  .name = "RS485Task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for DHT11Task */
osThreadId_t DHT11TaskHandle;
const osThreadAttr_t DHT11Task_attributes = {
  .name = "DHT11Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for FastSensorTask */
osThreadId_t FastSensorTaskHandle;
const osThreadAttr_t FastSensorTask_attributes = {
  .name = "FastSensorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MagnetTask */
osThreadId_t MagnetTaskHandle;
const osThreadAttr_t MagnetTask_attributes = {
  .name = "MagnetTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for modbusQueue */
osMessageQueueId_t modbusQueueHandle;
const osMessageQueueAttr_t modbusQueue_attributes = {
  .name = "modbusQueue"
};
/* Definitions for SensorMutex */
osMutexId_t SensorMutexHandle;
const osMutexAttr_t SensorMutex_attributes = {
  .name = "SensorMutex"
};
/* Definitions for rs485Sem */
osSemaphoreId_t rs485SemHandle;
const osSemaphoreAttr_t rs485Sem_attributes = {
  .name = "rs485Sem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartRS485Task(void *argument);
void StartDht11Task(void *argument);
void StartFastSensorTask(void *argument);
void StartMagnetTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of SensorMutex */
  SensorMutexHandle = osMutexNew(&SensorMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of rs485Sem */
  rs485SemHandle = osSemaphoreNew(1, 1, &rs485Sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of modbusQueue */
  modbusQueueHandle = osMessageQueueNew (4, sizeof(uint16_t), &modbusQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of RS485Task */
  RS485TaskHandle = osThreadNew(StartRS485Task, NULL, &RS485Task_attributes);

  /* creation of DHT11Task */
  DHT11TaskHandle = osThreadNew(StartDht11Task, NULL, &DHT11Task_attributes);

  /* creation of FastSensorTask */
  FastSensorTaskHandle = osThreadNew(StartFastSensorTask, NULL, &FastSensorTask_attributes);

  /* creation of MagnetTask */
  MagnetTaskHandle = osThreadNew(StartMagnetTask, NULL, &MagnetTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartRS485Task */
/**
  * @brief  Function implementing the RS485Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartRS485Task */
void StartRS485Task(void *argument)
{
  /* USER CODE BEGIN StartRS485Task */

  ModbusRxMsgTypeDef rx_msg;
  Modbus_ParserTypedef modbus_parser;
  ModbusResponse_FrameTypeDef modbus_packer;
  ModbusRTU_FrameTypeDef rtu_frame;

  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_buf, sizeof(uart_buf));

  /* Infinite loop */
  for(;;)
  {
	if(xQueueReceive((QueueHandle_t)modbusQueueHandle, &rx_msg, portMAX_DELAY))
	{
		memset(&modbus_parser, 0, sizeof(modbus_parser));
		memset(&modbus_packer, 0, sizeof(modbus_packer));
		memset(&rtu_frame, 0, sizeof(rtu_frame));

		if (modbus_parsing(rx_msg.data, &modbus_parser, &rtu_frame))
		{
			HAL_UART_Transmit(&huart2, "Parsing\r\n", 9, 100);
			uint8_t index = 0;
			uint8_t rs485_tx_len = 5;

			switch(modbus_parser.cmd)
			{
				case Read_Holding_Registers:

					if(modbus_parser.flags & (1 << REG_Temp_Addr))
					{
						modbus_packer.data[index++] = s_data.temp;
						modbus_parser.flags &= ~(1 << REG_Temp_Addr);
						rs485_tx_len += 2;
					}

					if(modbus_parser.flags & (1 << REG_Humid_Addr))
					{
						modbus_packer.data[index++] = s_data.humid;
						modbus_parser.flags &= ~(1 << REG_Humid_Addr);
						rs485_tx_len += 2;
					}

					if(modbus_parser.flags & (1 << REG_Co2_Addr))
					{
						modbus_packer.data[index++] = s_data.co2;
						modbus_parser.flags &= ~(1 << REG_Co2_Addr);
						rs485_tx_len += 2;
					}

					if(modbus_parser.flags & (1 << REG_Dust_Addr))
					{
						modbus_packer.data[index++] = s_data.dust_density;
						modbus_parser.flags &= ~(1 << REG_Dust_Addr);
						rs485_tx_len += 2;
					}

					if(modbus_parser.flags & (1 << REG_Zone_Addr))
					{
						modbus_packer.data[index++] = s_data.zone;
						modbus_parser.flags &= ~(1 << REG_Zone_Addr);
						rs485_tx_len += 2;
					}

					uint8_t rs485_tx_buf[rs485_tx_len];
					modbus_packing(rs485_tx_buf, &modbus_packer, &rtu_frame);

					HAL_UART_Transmit(&huart1, (uint8_t*)rs485_tx_buf, rs485_tx_len, 100);
			}
		}
		else ;
	}
	// HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_buf, sizeof(uart_buf));
  }
  /* USER CODE END StartRS485Task */
}

/* USER CODE BEGIN Header_StartDht11Task */
/**
* @brief Function implementing the DHT11Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDht11Task */
void StartDht11Task(void *argument)
{
  /* USER CODE BEGIN StartDht11Task */
  /* Infinite loop */
	for(;;)
	{
	    uint16_t duration[42] = {0};
	    uint8_t bits[40] = {0};
	    uint16_t temph = 0, humidh = 0;
	    uint16_t templ = 0, humidl = 0;

	    // Output 모드 전환
	    GPIO_InitTypeDef GPIO_InitStruct = {0};
	    GPIO_InitStruct.Pin = DHT11_OUT_Pin;
	    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	    GPIO_InitStruct.Pull = GPIO_NOPULL;
	    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	    HAL_GPIO_Init(DHT11_OUT_GPIO_Port, &GPIO_InitStruct);

	    // DHT11 Start Signal (LOW 20ms -> HIGH)
	    HAL_GPIO_WritePin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin, GPIO_PIN_RESET);
	    osDelay(20);
	    HAL_GPIO_WritePin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin, GPIO_PIN_SET);

	    // Input 모드 전환
	    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	    HAL_GPIO_Init(DHT11_OUT_GPIO_Port, &GPIO_InitStruct);


	    HAL_TIM_Base_Start(&htim7);

	    // 타이밍 정밀 측정 구간 (스케줄러/인터럽트 잠금)
	    taskENTER_CRITICAL();

	    for(int i = 0; i < 42; i++)
	    {
	        // LOW 구간 측정
	        __HAL_TIM_SET_COUNTER(&htim7, 0);
	        while(HAL_GPIO_ReadPin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin) == GPIO_PIN_RESET)
	        {
	            if(__HAL_TIM_GET_COUNTER(&htim7) > 100) break;
	        }

	        // HIGH 구간 측정
	        __HAL_TIM_SET_COUNTER(&htim7, 0);
	        while(HAL_GPIO_ReadPin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin) == GPIO_PIN_SET)
	        {
	            if(__HAL_TIM_GET_COUNTER(&htim7) > 100) break;
	        }
	        duration[i] = __HAL_TIM_GET_COUNTER(&htim7); // HIGH 신호 유지 시간(us)
	    }

	    taskEXIT_CRITICAL();

	    // 비트 판별 (HIGH 신호 길이 기준: ~26us=0, ~70us=1)
	    for(int i = 0; i < 40; i++)
	    {
	        if((duration[i+2] >= 15) && (duration[i+2] <= 35)) {
	            bits[i] = 0;
	        }
	        else if ((duration[i+2] >= 55) && (duration[i+2] <= 85)) {
	            bits[i] = 1;
	        }
	    }

	    // 데이터 비트 복원
	    for(int i = 0; i < 8; i++) {
	        humidh |= (bits[i] << (7-i));
	        humidl |= (bits[i+8]    << (7-i));
	        temph  |= (bits[i+16] << (7-i));
	        templ  |= (bits[i+24]   << (7-i));
	    }

	    // Mutex를 통한 안전한 전역 구조체 업데이트
	    if(osMutexAcquire(SensorMutexHandle, osWaitForever) == osOK)
	    {
	        s_data.temp = (float)temph  + ((float)templ  * 0.1f);;
	        s_data.humid = (float)humidh + ((float)humidl * 0.1f);;
	        osMutexRelease(SensorMutexHandle);
	    }

	    char tx_buf[64];
	    sprintf(tx_buf, "[zone %d] temp : %.1f,   humid : %.1f,   co2 : %u ppm,  dust : %.1f\r\n", s_data.zone, s_data.temp, s_data.humid, s_data.co2, s_data.dust_density);
	    HAL_UART_Transmit(&huart2, (uint8_t*)tx_buf, strlen(tx_buf), 100);

	    osDelay(2000);
	}
  /* USER CODE END StartDht11Task */
}

/* USER CODE BEGIN Header_StartFastSensorTask */
/**
* @brief Function implementing the FastSensorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFastSensorTask */
void StartFastSensorTask(void *argument)
{
  /* USER CODE BEGIN StartFastSensorTask */

	uint8_t cm1106_rx_buf[8];
	uint8_t cm1106_cmd[4] = {0x11, 0x01, 0x01, 0xED};
	uint16_t co2_ppm = 0;

	uint16_t dust_adc = 0;
	float dust_voltage = 0;
	float dust_value = 0;
	static const float dust_clean = 0.28;
  /* Infinite loop */
  for(;;)
  {
	memset(cm1106_rx_buf, 0, sizeof(cm1106_rx_buf));
	HAL_UART_Receive_DMA(&huart3, cm1106_rx_buf, sizeof(cm1106_rx_buf));
	HAL_UART_Transmit(&huart3, cm1106_cmd, sizeof(cm1106_cmd), 100);

	uint32_t flags = osThreadFlagsWait(0x02, osFlagsWaitAny, 300);

	if ((flags & 0x02) != 0)
	{
		co2_ppm = cm1106_rx_buf[3] * 256 + cm1106_rx_buf[4];
	}

	if(osMutexAcquire(SensorMutexHandle, osWaitForever) == osOK)
	{
		s_data.co2 = co2_ppm;
		osMutexRelease(SensorMutexHandle);
	}

	if(s_data.co2 > 1500) HAL_GPIO_WritePin(Fire_Check_GPIO_Port, Fire_Check_Pin, 1);
	else HAL_GPIO_WritePin(Fire_Check_GPIO_Port, Fire_Check_Pin, 0);

//	char co2_buf[64];
//	sprintf(co2_buf, "co2 : %u ppm\r\n",s_data.co2);
//	HAL_UART_Transmit(&huart2, (uint8_t*)co2_buf, strlen(co2_buf), 100);

    osDelay(200);


    // 미세먼지 sensor
	HAL_GPIO_WritePin(Dust_LED_GPIO_Port, Dust_LED_Pin, 0);

	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while(__HAL_TIM_GET_COUNTER(&htim6) < 280);

	HAL_ADCEx_InjectedStart(&hadc1);

	if(HAL_ADCEx_InjectedPollForConversion(&hadc1, 10) == HAL_OK)
	{
		dust_adc = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
	}

	__HAL_TIM_SET_COUNTER(&htim6, 0);
	while(__HAL_TIM_GET_COUNTER(&htim6) < 40);

	HAL_GPIO_WritePin(Dust_LED_GPIO_Port, Dust_LED_Pin, 1);

	dust_voltage = dust_adc * 5.0 / 4095.0;
	dust_value = (dust_voltage - dust_clean) / 0.005;
	s_data.dust_density = dust_value;

//	char dust_buf[64];
//	sprintf(dust_buf, "dust : %f \r\n", s_data.dust_density);
//	HAL_UART_Transmit(&huart2, (uint8_t*)dust_buf, strlen(dust_buf), 100);

    osDelay(200);

  }
  /* USER CODE END StartFastSensorTask */
}

/* USER CODE BEGIN Header_StartMagnetTask */
/**
* @brief Function implementing the MagnetTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMagnetTask */
void StartMagnetTask(void *argument)
{
  /* USER CODE BEGIN StartMagnetTask */

  uint8_t current_zone = 1;
  /* Infinite loop */
  for(;;)
  {
	osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

	current_zone = (current_zone % 3) + 1;

	if(osMutexAcquire(SensorMutexHandle, osWaitForever) == osOK)
	{
		s_data.zone = current_zone;
		osMutexRelease(SensorMutexHandle);
	}

    osDelay(1000);

    osThreadFlagsClear(0x01);
  }
  /* USER CODE END StartMagnetTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

