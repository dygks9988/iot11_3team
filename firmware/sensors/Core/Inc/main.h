/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

// Sensors Data structure

#define DMA_Rx_Lens	32

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

typedef struct
{
	float temp;
	float humid;
	uint16_t co2;
	float dust_density;
	uint8_t zone;
} sensorData;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Fire_Check_Pin GPIO_PIN_14
#define Fire_Check_GPIO_Port GPIOC
#define Dust_LED_Pin GPIO_PIN_1
#define Dust_LED_GPIO_Port GPIOC
#define Dust_ADC_Pin GPIO_PIN_2
#define Dust_ADC_GPIO_Port GPIOC
#define DHT11_OUT_Pin GPIO_PIN_1
#define DHT11_OUT_GPIO_Port GPIOA
#define Hall_Digital_Pin GPIO_PIN_4
#define Hall_Digital_GPIO_Port GPIOA
#define Hall_Digital_EXTI_IRQn EXTI4_IRQn
#define Hall_Analog_Pin GPIO_PIN_5
#define Hall_Analog_GPIO_Port GPIOA
#define CM1106_RX_Pin GPIO_PIN_5
#define CM1106_RX_GPIO_Port GPIOC
#define CM1106_TX_Pin GPIO_PIN_10
#define CM1106_TX_GPIO_Port GPIOB
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
