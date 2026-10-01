/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "queue.h"
#include <stdio.h>
#include "modbus_protocol.h"
//#include <My_MCU_Printf_Lib_V2_8.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */



/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

extern osThreadId_t FastSensorTaskHandle;
extern osThreadId_t MagnetTaskHandle;
extern osSemaphoreId_t rs485SemHandle;
extern osMessageQueueId_t modbusQueueHandle;

uint8_t uart_buf[DMA_Rx_Lens];
uint16_t dma_old_pos = 0;

sensorData s_data = {
		.zone = 1
};

/*
uint8_t cm1106_rx_buf[8];
uint8_t cm1106_cmd[4] = {0x11, 0x01, 0x01, 0xED};
uint16_t co2_ppm = 0;


uint16_t dust_adc = 0;
float dust_voltage = 0;
float dust_value = 0;
static const float dust_clean = 0.28;


uint16_t hall_adc;	// Hall sensor

uint16_t tempdht11, humidht11;
*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
void dht11(uint16_t *temperature, uint16_t *humid)
{
    uint16_t tempc, humidc;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DHT11_OUT_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin, 0);

    HAL_Delay(20);

    HAL_GPIO_WritePin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin, 1);

    GPIO_InitStruct.Pin = DHT11_OUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DHT11_OUT_GPIO_Port, &GPIO_InitStruct);

    // try to get counter's start
    __HAL_TIM_SET_COUNTER(&htim7, 0);

    // pick the time of start GPIO_PIN_SET / RESET for each ler[0], ler[1]
    uint16_t ler[2];
    // save the time between start and end of signal
    uint16_t duration[42];
    uint8_t bits[40];
    uint16_t temph = 0;
    uint16_t humidh = 0;

    for(int i = 0; i < 42; i++) {
        while(HAL_GPIO_ReadPin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin) == 0);
        ler[0] = __HAL_TIM_GET_COUNTER(&htim7);
        while(HAL_GPIO_ReadPin(DHT11_OUT_GPIO_Port, DHT11_OUT_Pin) == 1);
        ler[1] = __HAL_TIM_GET_COUNTER(&htim7);
        duration[i] = ler[1] - ler[0];
    }

    for(int i = 0; i < 40; i++) {
        if((duration[i+2] >= 20) && (duration[i+2] <= 32)) {
            bits[i] = 0;
        }
        else if ((duration[i+2] >= 65) && (duration[i+2] <= 75)) {
            bits[i] = 1;
        }
    }

    for(int i = 0; i < 8; i++) {
        temph += bits[i+16] << (7-i);
        humidh += bits[i] << (7-i);
    }

    tempc = temph;
    humidc = humidh;

    *temperature = tempc;
    *humid = humidc;
}
*/

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim6);
  HAL_TIM_Base_Start(&htim7);

  //HAL_UART_Receive_DMA(&huart3, cm1106_rx_buf, sizeof(cm1106_rx_buf));
  //HAL_ADC_Start_DMA(&hadc1, &hall_adc, 1);


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*	// CO2 sensor
	  HAL_UART_Transmit(&huart3, cm1106_cmd, sizeof(cm1106_cmd), 100);
	  HAL_Delay(200);

	  co2_ppm = cm1106_rx_buf[3] * 256 + cm1106_rx_buf[4];
	  printf("CO2 ppm = %d\r\n", co2_ppm);

	  HAL_Delay(800);
	  */

	  /*	// 미세먼지 sensor
	  HAL_GPIO_WritePin(Dust_LED_GPIO_Port, Dust_LED_Pin, 0);

	  __HAL_TIM_SET_COUNTER(&htim6, 0);
	  while(__HAL_TIM_GET_COUNTER(&htim6) < 280);

	  HAL_ADCEx_InjectedStart(&hadc1);
	  //HAL_ADC_Start_DMA(&hadc1, &dust_adc, 1);

	  if(HAL_ADCEx_InjectedPollForConversion(&hadc1, 10) == HAL_OK)
	  {
		  dust_adc = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
	  }

	  __HAL_TIM_SET_COUNTER(&htim6, 0);
	  while(__HAL_TIM_GET_COUNTER(&htim6) < 40);

	  HAL_GPIO_WritePin(Dust_LED_GPIO_Port, Dust_LED_Pin, 1);

	  printf("Dust_adc : %u\r\n", dust_adc);

	  dust_voltage = dust_adc * 5.0 / 4095.0;
	  printf("Dust_voltage : %f\r\n", dust_voltage);

	  dust_value = (dust_voltage - dust_clean) / 0.005;
	  printf("Dust_value : %f\r\n", dust_value);

	  HAL_Delay(1000);
	  */

	  /*	// A3144E sensor
	  printf("Hall: %u\r\n", hall_adc);
	  HAL_Delay(500);
	  */

	  /*	// DHT-11 sensor
      dht11(&tempdht11, &humidht11);
      printf("temp : %d, hum : %d\r\n", tempdht11, humidht11);
      HAL_Delay(2000);  // 2초마다 읽기
	  */

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* EXTI4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	if (huart->Instance == USART1)
//	{
//		osSemaphoreRelease(rs485SemHandle);
//		HAL_UART_Transmit(&huart2, "Call BACK\r\n", 11, 100);
//	}

    if (huart->Instance == USART3)
    {
        osThreadFlagsSet(FastSensorTaskHandle, 0x02);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	// DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
	      uint32_t event = HAL_UARTEx_GetRxEventType(huart);
	      uint16_t dma_pos = Size;
	      uint16_t old_pos = dma_old_pos;
	      uint16_t length = 0;

	      static uint8_t Rx_Buff[DMA_Rx_Lens] = {0};	// rx_d->Rx_Buff
	      static uint8_t Rx_Buff_Index = 0;	// rx_cnt->rx_cnt_2
	      // uint8_t Rx_data[20] = {0};	// rx_d->Rx_data_2

	      // 1. 순환 버퍼(Circular) 구조에서 정확한 데이터 길이 계산
	      if(dma_pos >= old_pos) length = dma_pos - old_pos;
	      else length = DMA_Rx_Lens - old_pos + dma_pos;

	      // 2. 새로 들어온 길이만큼 복사 (인덱스 유지하면서 Rx_Buff에 누적)
	       for(uint16_t i = 0; i < length; i++)
	        {
	    	  Rx_Buff[Rx_Buff_Index] = uart_buf[(old_pos + i) % DMA_Rx_Lens];
	    	  Rx_Buff_Index++;

	          if(Rx_Buff_Index >= DMA_Rx_Lens)
	           {
	        	 Rx_Buff_Index = 0; // 버퍼 오버플로우 방지 (처음으로 롤백)
	           }
	        }

	       // 현재 위치를 다음 번 처리를 위해 저장
	       dma_old_pos = dma_pos;

	       #if debugging
	         printf("SIZE=%u OLD=%u LEN=%u EVENT=%lu\r\n", Size, old_pos, length, event);
	       #endif

	       // 3. 오직 IDLE(EVENT=2) 이벤트가 발생했을 때만 data 복사.
	       if(event == HAL_UART_RXEVENT_IDLE)
	        {
	    	  if(Rx_Buff_Index == 0) return;

	    	  ModbusRxMsgTypeDef msg = {0};

	          msg.len = Rx_Buff_Index;

	          for(uint8_t i = 0; i < msg.len; i++)
	          {
	            msg.data[i] = Rx_Buff[i];
	          }

	          // Buff Index 초기화
	          Rx_Buff_Index = 0;

	          BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	          xQueueSendFromISR((QueueHandle_t)modbusQueueHandle, &msg, &xHigherPriorityTaskWoken);
	          portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	        }
}

// 자기 sensor
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == Hall_Digital_Pin)
	{
		osThreadFlagsSet(MagnetTaskHandle, 0x01);
	}
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
