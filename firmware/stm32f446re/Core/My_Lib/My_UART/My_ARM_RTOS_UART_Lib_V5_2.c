/*
 * uart.c
 *
 *  Created on: Nov 12, 2025
 *      Author: Administrator
 */

// My Uart lib
// 송 명 규
// 010-2402-4398
// End Ver = V5.1

// Ver1.0 = 2018, 07, 09
// Ver2.0 = 2019, 05, 19
// Ver3.0 = 2020, 11, 06
// ver3.2 = 2022, 11, 28
// ver3.4 = 2022, 12, 23 = LIB 정리
// Ver3.5 = 2023, 06, 16 = 조건식 빌드항목 추가
// Ver4.0 = 2023, 06, 20 = tx func, 콜백함수 추가
// Ver4.1 = 2023, 07, 01 = 콘솔 체널 자동설정 추가, DMA/IRQ 자동설정 추가
// Ver4.2 = 2023, 07, 07 = 변수추가, 함수 수정
// Ver4.3 = 2023, 07, 19 = __PUTCHAR 추가, IRQ Func 정리
// Ver4.4 = 2023, 09. 12 = my_printf함수 변경 = UART채널 선택 멤버변수 추가
// Ver4.5 = 2023, 10. 05 = rx_ok_flag, rx_ready_flag 변수 추가
// Ver4.6 = 2024, 01. 01 = 파일명 변경, RX IRQ Callback 함수 수정
// Ver4.7 = 2024, 03, 10 = RS-485용 tx_str_485 함수 추가, 구조체 포인터 추가
// Ver4.8 = 2025, 04, 16 = FREERTOS 추가 및 IDLE Interrupt CallBack 함수 추가
// Ver4.9 = 2025, 11, 04 = FREERTOS 버퍼 삭제, UART TX time out 조정
// Ver5.0 = 2025, 11, 12 = FREERTOS용으로 분리작성 = AIOT 7기
// Ver5.1 = 2026, 05, 08 = 보강, 동작모드 분리 FREERTOS용으로 작성 = AIOT 9기
// Ver5.2 = 2026, 08, 13 = IDLE DMA Ring Buff 방식 보강 = AIOT 10기


// System Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "main.h"

// My Lib
#include <My_ARM_RTOS_UART_Lib_V5_2.h>


// 구조체 변수 초기화
// rx flag
_RX_FLAG_  Rxd_Sts = {
  	                   .rx_end_flag_1 = 0,
  	                   .rx_end_flag_2 = 0,
  	                   .rx_end_flag_3 = 0,
  	                   .rx_end_flag_4 = 0,
  	                   .rx_end_flag_5 = 0,
  	                   .rx_end_flag_6 = 0
                     };
_RX_FLAG_ *rx_flag = &Rxd_Sts;


// rx data save buffer
_RX_DATA_  rx_buf = {
		              .Rx_data_1 = {0},
					  .Rx_data_2 = {0},
					  .Rx_data_3 = {0},
					  .Rx_data_4 = {0,},
					  .Rx_data_5 = {0,},
					  .Rx_data_6 = {0,},
					  .Rx_Buff = {0},
					  .buff_size = 0,
					  .RxDataLen = 0
                    };
_RX_DATA_  *rx_d = &rx_buf;


// rx byte counter
_RX_CNT_  Rx_Cnt = {
                     .rx_cnt_1 = 0, // uart 1 수신 Data Counter
					 .rx_cnt_2 = 0, // uart 2 수신 Data Counter
					 .rx_cnt_3 = 0, // uart 3 수신 Data Counter
					 .rx_cnt_4 = 0, // uart 4 수신 Data Counter
					 .rx_cnt_5 = 0, // uart 5 수신 Data Counter
					 .rx_cnt_6 = 0, // uart 6 수신 Data Counter
                   };
_RX_CNT_  *rx_cnt = &Rx_Cnt;

// IDLE DMA 방식에서 사용 2026, 08, 13 추가 == AioT 10기
volatile uint16_t dma_old_pos = 0;
volatile uint16_t ring_head = 0;
volatile uint16_t ring_tail = 0;
volatile uint16_t length = 0;

extern DMA_HandleTypeDef hdma_usart2_rx;

uint8_t ring_buf[50];


// 2022, 11, 06 추가 = 교육생들이 좀 더 쉽게하기 위해서 추가 함 = 송신 함수
// 2023, 07, 01 추가 = 콘솔채널 자동설정 추가
extern int __io_putchar(int ch)
{
#if Console_Ch1
    //UART_HandleTypeDef huart1;
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch2
    //UART_HandleTypeDef huart2;
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch3
    //UART_HandleTypeDef huart3;
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch4
    //UART_HandleTypeDef huart4;
    HAL_UART_Transmit(&huart4, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch5
    //UART_HandleTypeDef huart3;
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch6
    //UART_HandleTypeDef huart3;
    HAL_UART_Transmit(&huart6, (uint8_t *)&ch, 1, 0xFFFF);
#endif


    __NOP();
  return ch;
}

// printf 사용하기 위해서 = 2023, 06, 23 위치이동
// 2023, 07, 01 추가 = 콘솔채널 자동설정 추가
extern int fputc(int ch, FILE *f)
 {
#if Console_Ch1
    //UART_HandleTypeDef huart1;
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch2
    //UART_HandleTypeDef huart2;
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

#elif Console_Ch3
    //UART_HandleTypeDef huart3;
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
#endif

    __NOP();

   return(ch);
 }


// 2022, 11, 06 추가 = 교육생들이 좀 더 쉽게하기 위해서 추가 함 = 송신
extern int _write(int file, char *ptr, int len)
{
    int DataIdx;
    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
      // 콘솔에 출력할 수 있도록 하기 위해서 수정 함
      __io_putchar(*ptr++);// == org

      asm("nop");
    }
    return len;
}

// 2022, 11, 06 추가 = 교육생들이 좀 더 쉽게하기 위해서 추가 함 = 송신
extern int __io_getchar(void)
{
    char data[4];
    uint8_t ch, len = 1;

    // 2023, 07, 01 추가 = 콘솔채널 자동설정 추가
#if Console_Ch1
    //UART_HandleTypeDef huart1;
    while(HAL_UART_Receive(&huart1, &ch, 1, 10) != HAL_OK){ }

#elif Console_Ch2
    //UART_HandleTypeDef huart2;
    while(HAL_UART_Receive(&huart2, &ch, 1, 10) != HAL_OK){ }

#elif Console_Ch3
    //UART_HandleTypeDef huart3;
    while(HAL_UART_Receive(&huart3, &ch, 1, 10) != HAL_OK){ }
#endif

    memset(data, 0x00, 4);
    switch(ch)
    {
        case '\r':
        case '\n':
            len = 2;
            sprintf(data, "\r\n");
            break;

        case '\b':
        case 0x7F:
            len = 3;
            sprintf(data, "\b \b");
            break;

        default:
            data[0] = ch;
            break;
    }

    asm("nop");
    return ch;
}

// 2023, 7, 19 추가
#ifdef __GNUC_C__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#elif __GNU_F__
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
    return ch;
}
#endif /* __GNUC__ */


// 2026, 08, 14 = AIOT 10기 추가
// Uart init
void Uart_Init(UART_HandleTypeDef *huart)
{
 if(huart->Instance == USART2)
 {
  memset(rx_d -> Rx_Buff, 0, sizeof(rx_d -> Rx_Buff));
  #if IDLE_IRQ_Mode_Ch2
     HAL_UART_Receive_IT(&huart2, &rx_d -> Rx_data_2, 1, 100);
  #elif IDLE_DMA_Normal_Mode_ch2
     HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rx_d -> Rx_data_2, DMA_Rx_Lens);
  #elif IDLE_DMA_Circular_Mode_ch2
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rx_d -> Rx_data_2, DMA_Rx_Lens);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    huart2.hdmarx->Instance->CR &= ~DMA_SxCR_HTIE; // 최초 호출 시에도 HT 인터럽트 차단!
  #endif
 }
}


// my usart LIB func == 2023, 06, 20 추가
void tx_send(uint8_t tx_data, uint8_t ch)
{
 switch(ch)
  {
   case 1: // uart 1
           #ifdef USART_Ch_1_EN
	          HAL_UART_Transmit(&huart1, (uint8_t *)&tx_data, 1, 100);
            #endif
	     break;

   case 2: // uart 2
           #ifdef USART_Ch_2_EN
             HAL_UART_Transmit(&huart2, (uint8_t *)&tx_data, 1, 100);
           #endif
         break;

   case 3: // uart 3
           #ifdef USART_Ch_3_EN
            HAL_UART_Transmit(&huart3, (uint8_t *)&tx_data, 1, 100);
           #endif
         break;

   case 4: // uart 4
          #ifdef UART_Ch_4_EN
           HAL_UART_Transmit(&huart4, (uint8_t *)&tx_data, 1, 100);
          #endif
     break;

   case 5: // uart 5
             #ifdef UART_Ch_5_EN
              HAL_UART_Transmit(&huart5, (uint8_t *)&tx_data, 1, 100);
             #endif
        break;

   case 6: // uart 6
                #ifdef USART_Ch_6_EN
                 HAL_UART_Transmit(&huart6, (uint8_t *)&tx_data, 1, 100);
                #endif
           break;

  }
}

// v3.5 == 2023, 06, 18 수정
void tx_str(uint8_t  *tx_d, char ch)
{
  while(*tx_d != '\0')
  {
    tx_send(*tx_d, ch);
    tx_d++;
  }
}

// v3.5 == 2023, 06, 18 추가
// low data tx
void tx(uint8_t *tx_d, char ch, char lans)
{
	 do{
	      tx_send(*tx_d, ch);
	      tx_d++;
	      lans--;
	     }while(lans != 0);
}


// 2023, 06, 20 수정
// 2023, 9, 12 수정 = 멤버변수 추가 = Ver4.4
 void vprint(uint8_t ch, const char *fmt, va_list argp)
  {
    char string[200];
    if(0 < vsprintf(string,fmt,argp)) // build string
    {
      // 2022, 11, 28수정 = Ver 3.2
     #ifdef USART_Ch_1_EN
     	if(ch == Usart_Ch_1) HAL_UART_Transmit(&huart1, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif

     #ifdef USART_Ch_2_EN
     	if(ch == Usart_Ch_2) HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif

     #ifdef USART_Ch_3_EN
     	if(ch == Usart_Ch_3) HAL_UART_Transmit(&huart3, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif

     #ifdef UART_Ch_4_EN
     	if(ch == Uart_Ch_4) HAL_UART_Transmit(&huart4, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif

     #ifdef UART_Ch_5_EN
   	    if(ch == Uart_Ch_5) HAL_UART_Transmit(&huart5, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif

     #ifdef USART_Ch_6_EN
  	    if(ch == Usart_Ch_6) HAL_UART_Transmit(&huart6, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
     #endif
    }
}

// 2023, 9, 12 수정 = 멤버변수 추가 = Ver4.4
void my_printf(uint8_t ch, const char *fmt, ...) // custom printf() function
{
  va_list argp;
  va_start(argp, fmt);
  vprint(ch, fmt, argp); // 2023, 9, 12 수정 = 멤버변수 추가 = Ver4.4
  va_end(argp);
}


// 1Byte 수신하기 = AIOT6기 추가 = 2015, 11, 11
void rx_1byte(uint8_t *ch)
{
 // 1 Byte 수신 처리
 if(rx_d -> Rx_data_2[0] == '\n')
   {
    rx_flag -> rx_end_flag_2 = 1;
    rx_cnt -> rx_cnt_2 = 0;
   }
 else
   {
  	d_buf[rx_cnt -> rx_cnt_2] = rx_d -> Rx_data_2[0];
   	rx_cnt -> rx_cnt_2++;
   }
}

//2026, 08, 08 추가
void uart_tx(UART_HandleTypeDef *huart, uint8_t *tx_buf, uint16_t Lans)
{
 HAL_UART_Transmit(huart, (uint8_t *)tx_buf, Lans, 1000);
 while(__HAL_UART_GET_FLAG(huart,UART_FLAG_TC)==RESET);
}


//=======  interrupt call back func  ======================
// 2023, 06, 20 추가
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if USART_Ch_1_EN
  // uart1 IRQ
  if(huart->Instance == USART1)
   {
     #if RTOS_Queue_Mode
	   // ISR에서 Queue로 데이터 전송
	   // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	   //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	   xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_1, &xHigherPriorityTaskWoken);

	   // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   rx_flag -> rx_end_flag_1 = 1;

     #elif RTOS_Sema_Mode // 세마포어바이러니 모드
       __NOP();
       rx_flag -> rx_end_flag_1 = 1;

     #elif IRQ_Mode_ch1
	   rx_flag -> rx_end_flag_1 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_IT(&huart1, (uint8_t *)rx_d -> Rx_data_1, 10);

	   // Test
	   //printf("IRQ Rx ok\r\n");
    #elif DMA_Normal_Mode_ch1
	   rx_flag -> rx_end_flag_1 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_DMA(&huart1, (uint8_t *)rx_d -> Rx_data_1, 10); //rx_d -> buff_size);

 	   // Test
	   //printf("Normal_DMA Rx ok\r\n");
    #elif DMA_Circular_Mode_ch1
	  //tx_str(2, rx_d -> Rx_data_1);
	  //memset(rx_d -> Rx_data_1, 0, 20 *sizeof(uint8_t));
	  rx_flag -> rx_end_flag_1 = 1;

	  // Test
  	  //printf("Circular DMA Rx ok\r\n");
    #endif
  	  __HAL_UART_CLEAR_PEFLAG(&huart1);
	}
#endif
//---------------------------------------------------------------------------------


#if USART_Ch_2_EN  // uart 2
  // uart2 IRQ
if(huart->Instance == USART2)
{
  #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_2, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_2 = 1;

  #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_2 = 1;

  #elif IRQ_Mode_ch2
	  rx_flag -> rx_end_flag_2 = 1;
	  // 다음 바이트 수신 준비
	  HAL_UART_Receive_IT(&huart2, (uint8_t *)rx_d -> Rx_data_2, 5);

	  // Test
	  //printf("IRQ Rx ok\r\n");
  #elif DMA_Normal_Mode_ch2
	  rx_flag -> rx_end_flag_2 = 1;
	  // 다음 바이트 수신 준비
	  HAL_UART_Receive_DMA(&huart2, (uint8_t *)rx_d -> Rx_data_2, 5); //rx_d -> buff_size);

	  // Test
	  //printf("Normal_DMA Rx ok\r\n");
  #elif DMA_Circular_Mode_ch2
	  //tx_str(2, rx_d -> Rx_data_2);
	  //memset(rx_d -> Rx_data_2, 0, 20 *sizeof(uint8_t));
	  rx_flag -> rx_end_flag_2 = 1;

	  // Test
  	  //printf("Circular DMA Rx ok\r\n");
  #endif
  	__HAL_UART_CLEAR_PEFLAG(&huart2);
   }
#endif
//---------------------------------------------------------------------------


#if USART_Ch_3_EN
 // uart3 IRQ
 if(huart->Instance == USART3)
  {
   #if RTOS_Queue_Mode
     // ISR에서 Queue로 데이터 전송
     // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
     BaseType_t xHigherPriorityTaskWoken = pdFALSE;

     //xHigherPriorityTaskWoken = 전환 필요 여부 표시
     xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_3, &xHigherPriorityTaskWoken);

     // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
     rx_flag -> rx_end_flag_3 = 1;

   #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_3 = 1;

   #elif IRQ_Mode_ch3
	  rx_flag -> rx_end_flag_3 = 1;
	  // 다음 바이트 수신 준비
	  HAL_UART_Receive_IT(&huart3, (uint8_t *)rx_d -> Rx_data_3, 5);

	  // Test
	  //printf("IRQ Rx ok\r\n");
   #elif DMA_Normal_Mode_ch3
	  rx_flag -> rx_end_flag_3 = 1;
	  // 다음 바이트 수신 준비
	  HAL_UART_Receive_DMA(&huart3, (uint8_t *)rx_d -> Rx_data_3, 10); //rx_d -> buff_size);

	  // Test
	  //printf("Normal_DMA Rx ok\r\n");
   #elif DMA_Circular_Mode_ch3
	  rx_flag -> rx_end_flag_3 = 1;

	  // Test
  	  //printf("Circular DMA Rx ok\r\n");
   #endif
  	  __HAL_UART_CLEAR_PEFLAG(&huart3);
   }
#endif



#if UART_Ch_4_EN
  // uart4 IRQ
   if(huart->Instance == UART4)
    {
     #if RTOS_Queue_Mode
	   // ISR에서 Queue로 데이터 전송
	   // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	   //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	   xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_4, &xHigherPriorityTaskWoken);

	   // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   rx_flag -> rx_end_flag_4 = 1;

     #elif RTOS_Sema_Mode // 세마포어바이러니 모드
       __NOP();
       rx_flag -> rx_end_flag_4 = 1;

     #elif IRQ_Mode_ch4
	   rx_flag -> rx_end_flag_4 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_IT(&huart4, (uint8_t *)rx_d -> Rx_data_4, 8);

	   // Test
	   //printf("IRQ Rx ok\r\n");
     #elif DMA_Normal_Mode_ch4
	   rx_flag -> rx_end_flag_4 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_DMA(&huart4, (uint8_t *)rx_d -> Rx_data_4, 5); //rx_d -> buff_size);

	   // Test
	   //printf("Normal_DMA Rx ok\r\n");
     #elif DMA_Circular_Mode_ch4
	  rx_flag -> rx_end_flag_4 = 1;


	   // Test
 	   //printf("Circular DMA Rx ok\r\n");
     #endif
 	__HAL_UART_CLEAR_PEFLAG(&huart4);
    }
#endif


#if UART_Ch_5_EN
   // uart5 IRQ
   if(huart->Instance == UART5)
    {
      #if RTOS_Queue_Mode
	   // ISR에서 Queue로 데이터 전송
	   // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	   //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	   xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_5, &xHigherPriorityTaskWoken);

	   // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   rx_flag -> rx_end_flag_5 = 1;

     #elif RTOS_Sema_Mode // 세마포어바이러니 모드
       __NOP();
       rx_flag -> rx_end_flag_5 = 1;

     #elif IRQ_Mode_ch5
	   rx_flag -> rx_end_flag_5 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_IT(&huart5, (uint8_t *)rx_d -> Rx_data_5, 8);

	   // Test
	   //printf("IRQ Rx ok\r\n");
     #elif DMA_Normal_Mode_ch5
	   rx_flag -> rx_end_flag_5 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_DMA(&huart5, (uint8_t *)rx_d -> Rx_data_5, 5); //rx_d -> buff_size);

	   // Test
	   //printf("Normal_DMA Rx ok\r\n");
     #elif DMA_Circular_Mode_ch5
	   rx_flag -> rx_end_flag_5 = 1;


	   // Test
 	   //printf("Circular DMA Rx ok\r\n");
     #endif
 	 __HAL_UART_CLEAR_PEFLAG(&huart5);
    }
#endif


#if USART_Ch_6_EN
   // uart6 IRQ
   if(huart->Instance == USART6)
    {
     #if RTOS_Queue_Mode
	   // ISR에서 Queue로 데이터 전송
	   // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	   //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	   xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_6, &xHigherPriorityTaskWoken);

	   // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	   portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	   rx_flag -> rx_end_flag_6 = 1;

     #elif RTOS_Sema_Mode // 세마포어바이러니 모드
       __NOP();
       rx_flag -> rx_end_flag_6 = 1;

     #elif IRQ_Mode_ch6
	   rx_flag -> rx_end_flag_6 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_IT(&huart6, (uint8_t *)rx_d -> Rx_data_6, 8);

	   // Test
	   //printf("IRQ Rx ok\r\n");
     #elif DMA_Normal_Mode_ch6
	   rx_flag -> rx_end_flag_6 = 1;
	   // 다음 바이트 수신 준비
	   HAL_UART_Receive_DMA(&huart6, (uint8_t *)rx_d -> Rx_data_6, 5); //rx_d -> buff_size);

	   // Test
	   //printf("Normal_DMA Rx ok\r\n");
     #elif DMA_Circular_Mode_ch6
	   rx_flag -> rx_end_flag_6 = 1;

	   // Test
 	   //printf("Circular DMA Rx ok\r\n");
     #endif
 	__HAL_UART_CLEAR_PEFLAG(&huart6);
    }
#endif

} // HAL_UART_RxCpltCallback func end
//--------------------------------------------------------------------------

// 2025, 04, 23 추가 = IDLE Call Back Func
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
#if USART_Ch_1_EN
  if(huart->Instance == USART1)
   {
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_1, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_1 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_1 = 1;

    #elif IDLE_IRQ_Mode_Ch1 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_1 = 1;

	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)&rx_d -> Rx_data_1, 20); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch1
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_1 = 1;
   #elif IDLE_DMA_Normal_Mode_ch1
      HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t *)&rx_d -> Rx_data_1, 20);//rx_d -> Rx_buff_size); // 50 Byte rx
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_1 = 1;
   #endif
  }
#endif


#if USART_Ch_2_EN
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_2, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_2 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_2 = 1;

    #elif IDLE_IRQ_Mode_Ch2 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_2 = 1;


	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t *)&rx_d -> Rx_data_2, 10); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch2
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      uint32_t event = HAL_UARTEx_GetRxEventType(huart);
      uint16_t dma_pos = Size;
      uint16_t old_pos = dma_old_pos;
      uint16_t length = 0;

      // 1. 순환 버퍼(Circular) 구조에서 정확한 데이터 길이 계산
      if(dma_pos >= old_pos) length = dma_pos - old_pos;
      else length = DMA_Rx_Lens - old_pos + dma_pos;

      // 2. 새로 들어온 길이만큼 복사 (인덱스 유지하면서 Rx_Buff에 누적)
       for(uint16_t i = 0; i < length; i++)
        {
          rx_d->Rx_Buff[rx_cnt->rx_cnt_2] = rx_d->Rx_data_2[(old_pos + i) % DMA_Rx_Lens];
          rx_cnt->rx_cnt_2++;

          if(rx_cnt->rx_cnt_2 >= DMA_Rx_Lens)
           {
            rx_cnt->rx_cnt_2 = 0; // 버퍼 오버플로우 방지 (처음으로 롤백)
           }
        }

       // 현재 위치를 다음 번 처리를 위해 저장
       dma_old_pos = dma_pos;

       #if debugging
         printf("SIZE=%u OLD=%u LEN=%u EVENT=%lu\r\n", Size, old_pos, length, event);
       #endif

       // 3. 오직 IDLE(EVENT=2) 이벤트가 발생했을 때만 문자열을 마감하고 플래그를 세웁니다.
       if(event == HAL_UART_RXEVENT_IDLE)
        {
         if(rx_cnt->rx_cnt_2 < DMA_Rx_Lens)
          {
            rx_d->Rx_Buff[rx_cnt->rx_cnt_2] = '\0';
          }
         rx_flag->rx_end_flag_2 = 1;
        }
   #elif IDLE_DMA_Normal_Mode_ch2
      rx_flag -> rx_end_flag_2 = 1;

      uint32_t event = HAL_UARTEx_GetRxEventType(huart);

             // 1. 노멀 모드이므로 매번 인덱스를 0부터 시작하여 채웁니다.
             rx_cnt->rx_cnt_2 = 0;

             // 2. 수신된 데이터 복사
             for (uint16_t i = 0; i < Size; i++)
             {
                 rx_d->Rx_Buff[rx_cnt->rx_cnt_2] = rx_d->Rx_data_2[i];
                 rx_cnt->rx_cnt_2++;

                 if (rx_cnt->rx_cnt_2 >= DMA_Rx_Lens)
                 {
                     break;
                 }
             }

     #if debugging
             printf("SIZE(LEN)=%u EVENT=%lu\r\n", Size, event);
     #endif

             // 3. IDLE 이벤트가 발생했을 때 문자열 마감 및 플래그 세팅
             if (event == HAL_UART_RXEVENT_IDLE)
             {
                 if (rx_cnt->rx_cnt_2 < DMA_Rx_Lens)
                 {
                     rx_d->Rx_Buff[rx_cnt->rx_cnt_2] = '\0';
                 }
                 rx_flag->rx_end_flag_2 = 1;
             }

             // [★핵심 노멀 모드 특징★]
             // 노멀 모드는 이벤트 후 DMA가 중지되므로 다음 수신을 위해 반드시 재시작해야 합니다!
             HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_d->Rx_data_2, DMA_Rx_Lens);

             // 노멀 모드에서도 Half-Transfer 인터럽트가 불필요하다면 아래와 같이 꺼줍니다.
             huart->hdmarx->Instance->CR &= ~DMA_SxCR_HTIE;

             // 재장전
             HAL_UARTEx_ReceiveToIdle_DMA(&huart2, &rx_d -> Rx_data_2, DMA_Rx_Lens);
   #endif
#endif


#if USART_Ch_3_EN
  if(huart->Instance == USART3)
   {
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_3, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_3 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_3 = 1;

    #elif IDLE_IRQ_Mode_Ch3 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_3 = 1;

	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)&rx_d -> Rx_data_3, 20); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch3
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_3 = 1;
   #elif IDLE_DMA_Normal_Mode_ch3
      HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t *)&rx_d -> Rx_data_3, 20);//rx_d -> Rx_buff_size); // 50 Byte rx
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_3 = 1;
    #endif
  }
#endif


#if USART_Ch_4_EN
  if(huart->Instance == USART4)
   {
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_4, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_4 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_4 = 1;

    #elif IDLE_IRQ_Mode_Ch4 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_4 = 1;

	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart4, (uint8_t *)&rx_d -> Rx_data_4, 20); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch4
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_4 = 1;
   #elif IDLE_DMA_Normal_Mode_ch4
      HAL_UARTEx_ReceiveToIdle_DMA(&huart4, (uint8_t *)&rx_d -> Rx_data_4, 20);//rx_d -> Rx_buff_size); // 50 Byte rx
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_4 = 1;
    #endif
  }
#endif


#if USART_Ch_5_EN
  if(huart->Instance == USART5)
   {
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_5, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_5 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_5 = 1;

    #elif IDLE_IRQ_Mode_Ch5 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_5 = 1;

	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart5, (uint8_t *)&rx_d -> Rx_data_5, 20); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch5
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_5 = 1;
   #elif IDLE_DMA_Normal_Mode_ch5
      HAL_UARTEx_ReceiveToIdle_DMA(&huart5, (uint8_t *)&rx_d -> Rx_data_5, 20);//rx_d -> Rx_buff_size); // 50 Byte rx
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_5 = 1;
    #endif
  }
#endif


#if USART_Ch_6_EN
  if(huart->Instance == USART6)
   {
    #if RTOS_Queue_Mode
	  // ISR에서 Queue로 데이터 전송
	  // 자료형 선언 = FreeRTOS 기본 정수 타입 (int 비슷, 4Byte)
	  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	  //xHigherPriorityTaskWoken = 전환 필요 여부 표시
	  xQueueSendFromISR(my_uart_irq, &rx_d -> Rx_data_6, &xHigherPriorityTaskWoken);

	  // 필요 시 컨텍스트 스위칭 = ISR이 끝난 뒤 바로 Task로 전환 가능
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	  rx_flag -> rx_end_flag_6 = 1;

    #elif RTOS_Sema_Mode // 세마포어바이러니 모드
      __NOP();
      rx_flag -> rx_end_flag_6 = 1;

    #elif IDLE_IRQ_Mode_Ch6 // Interrupt == 2024, 04, 23
	  rx_flag -> rx_end_flag_6 = 1;

	  // ReLoad
      HAL_UARTEx_ReceiveToIdle_IT(&huart6, (uint8_t *)&rx_d -> Rx_data_6, 20); // 50 Byte rx
   #elif IDLE_DMA_Circular_Mode_ch6
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_6 = 1;
   #elif IDLE_DMA_Normal_Mode_ch6
      HAL_UARTEx_ReceiveToIdle_DMA(&huart6, (uint8_t *)&rx_d -> Rx_data_6, 20);//rx_d -> Rx_buff_size); // 50 Byte rx
      // DMA가 반전송 또는 완료 후 자동으로 재시작되지 않게 설정
      __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
      rx_flag -> rx_end_flag_6 = 1;
    #endif
  }
#endif

}



/*
//===========================================================================================
//  IDLE Interrupt 사용 예제
#if IDLE_DMA_Ch3 // dma == 2024, 04, 23
     HAL_UARTEx_ReceiveToIdle_DMA(&huart3, (uint8_t *)&rx_d -> Rx_data_3, 100);//rx_d -> Rx_buff_size); // 50 Byte rx
  #elif IDLE_IRQ_Ch3 // Interrupt == 2024, 04, 23
     HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)&rx_d -> Rx_data_3, 100); // 50 Byte rx
  #else // == 2024, 04, 23
     HAL_UART_Receive_IT(&huart3, (uint8_t *)rx_d -> Rx_data_3, 10); //rx_d -> buff_size);
  #endif
*/


