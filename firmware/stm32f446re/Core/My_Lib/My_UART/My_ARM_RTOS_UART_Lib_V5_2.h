/*
 * uart.h
 *
 *  Created on: Nov 12, 2025
 *      Author: Administrator
 */



/*
 * my_adc_lib_v2_2.h
 *
 *  송  명  규
 *  011 - 2402-4398
 *  mgsong@hanmail.net
 *
// My Uart lib
// 송 명 규
// 010-2402-4398
// End Ver = V5.1
 *
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

*/


#ifndef __MY_ARM_RTOS_UART_Lib__
#define __MY_ARM_RTOS_UART_Lib__


#include <stdio.h>  // printf, sprintf 사용하기 위해서
#include <stdarg.h>
#include "main.h"
#include "usart.h"
#include "My_ARM_RTOS_Value_Lib_V1_4.h"

//#define debugging   1

#define DMA_Rx_Lens  10


// App Define
#define My_Uart_LIB_EN  1
#define Console_Ch2		2
#define USART_Ch_2_EN	3
//#define USART_Ch_3_EN	4
//#define USART_Ch_1_EN	4
//#define UART_Ch_4_EN	5
#define IDEL_Mode       6


#if IDEL_Mode
// IDLE CH1
//#define IDLE_IRQ_Mode_Ch1   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch1  10 // DMA Normal_Mode
//#define IDLE_DMA_Circular_Mode_ch1  11 // DMA Circular Mode

// IDLE CH2
//#define IDLE_IRQ_Mode_Ch2   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch2  10 // DMA Normal_Mode
#define IDLE_DMA_Circular_Mode_ch2  11 // DMA Circular Mode

// IDLE CH3
//#define IDLE_IRQ_Mode_Ch3   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch3  10 // DMA Normal_Mode
//#define IDLE_DMA_Circular_Mode_ch3  11 // DMA Circular Mode

// IDLE CH4
//#define IDLE_IRQ_Mode_Ch4   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch4  10 // DMA Normal_Mode
//#define IDLE_DMA_Circular_Mode_ch4  11 // DMA Circular Mode

// IDLE CH5
//#define IDLE_IRQ_Mode_Ch5   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch5  10 // DMA Normal_Mode
//#define IDLE_DMA_Circular_Mode_ch5  11 // DMA Circular Mode

// IDLE CH6
//#define IDLE_IRQ_Mode_Ch6   9 // Interrupt Mode
//#define IDLE_DMA_Normal_Mode_ch6  10 // DMA Normal_Mode
//#define IDLE_DMA_Circular_Mode_ch6  11 // DMA Circular Mode


#else
// Ch별 Run Mode Set
// ch1
#define IRQ_Mode_ch1         6 // Interrupt Mode
//#define DMA_Normal_Mode_ch1  7 // DMA Normal_Mode
//#define DMA_Circular_Mode_ch1  8 // DMA Circular Mode

// ch2
//#define IRQ_Mode_ch2         9 // Interrupt Mode
//#define DMA_Normal_Mode_ch2  10 // DMA Normal_Mode
#define DMA_Circular_Mode_ch2  11 // DMA Circular Mode

//ch3
//#define IRQ_Mode_ch3           6 // Interrupt Mode
//#define DMA_Normal_Mode_ch3  7 // DMA Normal_Mode
//#define DMA_Circular_Mode_ch3  8 // DMA Circular Mode

//ch4
#define IRQ_Mode_ch4           6 // Interrupt Mode
///#define DMA_Normal_Mode_ch4  7 // DMA Normal_Mode
//#define DMA_Circular_Mode_ch4  8 // DMA Circular Mode

//ch5
#define IRQ_Mode_ch5           6 // Interrupt Mode
//#define DMA_Normal_Mode_ch5  7 // DMA Normal_Mode
//#define DMA_Circular_Mode_ch5  8 // DMA Circular Mode

//ch6
//#define IRQ_Mode_ch6           6 // Interrupt Mode
//#define DMA_Normal_Mode_ch6  7 // DMA Normal_Mode
//#define DMA_Circular_Mode_ch6  8 // DMA Circular Mode

#endif


#ifdef My_Uart_LIB_EN
  #define Usart_Ch_1	1
  #define Usart_Ch_2	2
  #define Usart_Ch_3	3
  #define Uart_Ch_4	    4  // L152
  #define Uart_Ch_5	    5  // L152
  #define Usart_Ch_6	6  // F746
  #define _CR_	0x0d  //13
  #define _LF_	0x0a  //10

  extern int __io_putchar(int ch);
  extern int fputc(int ch, FILE *f);
  extern int _write(int file, char *ptr, int len);
  extern int __io_getchar(void);

  void vprint(uint8_t ch, const char *fmt, va_list argp);
  void my_printf(uint8_t ch, const char *fmt, ...); // custom printf() function
  void tx_send(uint8_t tx_data, uint8_t ch);
  void tx_str(uint8_t  *tx_d, char ch);        // v3.5
  void tx(uint8_t *tx_d, char ch, char lans); // v3.5
  void rx_1byte(uint8_t *ch);
#endif


#if RTOS
  #include "FreeRTOS.h"
  #include "queue.h"
  #include "semphr.h"

  extern QueueHandle_t my_uart_irq;
#endif


// uart flag
typedef struct
{
 volatile uint8_t rx_end_flag_1 : 1; // uart1 rx end flag
 volatile uint8_t rx_end_flag_2 : 1; // uart2 rx end flag
 volatile uint8_t rx_end_flag_3 : 1; // uart3 rx end flag
 volatile uint8_t rx_end_flag_4 : 1; // uart4 rx end flag
 volatile uint8_t rx_end_flag_5 : 1; // uart5 rx end flag
 volatile uint8_t rx_end_flag_6 : 1; // 2023, 8, 6 inc
 volatile int8_t rx_ok_flag     : 1;
 volatile int8_t rx_ready_flag  : 1;
}_RX_FLAG_;


// UART Value
typedef struct
 {
  volatile uint8_t Rx_data_1[20];  // uart 1 rx_buf
  volatile uint8_t Rx_data_2[20];  // uart 2 rx_buf
  volatile uint8_t Rx_data_3[20];  // uart 3 rx_buf
  volatile uint8_t Rx_data_4[20];  // uart 3 rx_buf
  volatile uint8_t Rx_data_5[20];  // uart 3 rx_buf
  volatile uint8_t Rx_data_6[20];  // 2023, 08, 06 inc
  volatile uint8_t Rx_Buff[20];    // 2026, 08, 01 inc
  volatile uint16_t buff_size;
  volatile uint16_t RxDataLen;
 }_RX_DATA_;


// rx byte counter
 typedef struct
 {
   volatile uint8_t rx_cnt_1;    // uart 1 수신 Data Counter
   volatile uint8_t rx_cnt_2;    // uart 2 수신 Data Counter
   volatile uint8_t rx_cnt_3;    // uart 3 수신 Data Counter
   volatile uint8_t rx_cnt_4;    // uart 4 수신 Data Counter
   volatile uint8_t rx_cnt_5;    // uart 5 수신 Data Counter
   volatile uint8_t rx_cnt_6;    // 2023, 8, 6 inc
 }_RX_CNT_;


extern  _RX_FLAG_  Rxd_Sts;
extern  _RX_FLAG_  *rx_flag;
extern  _RX_DATA_  rx_buf;
extern  _RX_DATA_  *rx_d;
extern  _RX_CNT_   Rx_Cnt;
extern  _RX_CNT_   *rx_cnt;


#endif /* MY_LIB_UART_MY_ARM_RTOS_UART_LIB_V5_1_H_ */
