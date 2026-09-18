


/*
 * my_adc_lib_v1_3.h
 *
 *  송  명  규
 *  011 - 2402-4398
 *  mgsong@hanmail.net
 *
 *  V1.0 == 2015, 4, 8 = 최조작성
 *  V1.5 == 2020, 10, 24 = 모드추가
 *  V2.0 == 2023, 7, 2 = 기능 대폭보강
 *  V2.1 == 2023, 9, 25 = 함수추가
 *  V2.2 == 2024, 02, 06 = 구조체포인터 추가, 함수명 교체
 *  V2.3 == 2024, 08, 28 = 함수명 추가 및 코드 수정 보강
 *                       = IoT 8기 때 보강
 *  V2.4 == 2025, 02, 04 = displat 방식 변경 = AIOT 1기
 *  V2.5 == 2025, 11, 11 = Mult Mode, Injec Mode 추가 = AIOT 6기
 *
 *  V3.0 == 2025, 11, 12 = FREERTOS용으로 작성 = AIOT6기
 *  V3.1 == 2025, 11, 18 = FREERTOS용 기능보강, 변수, 함수 추가 = AIOT6기
 *  V3.2 == 2026, 05, 13 = FREERTOS용 코드정리, 기능정리 추가 = AIOT 9기
*/


#ifndef __MY_ARM_RTOS_ADC_Lib__
#define __MY_ARM_RTOS_ADC_Lib__


#include "stm32f4xx_hal.h"
#include <main.h>

// App define

//1. Polling_Mode
//#define ADC_Reg_Polling_Mode   1
//#define ADC_Injec_Polling_Mode   2
//------------------------------------------------------------------

//2. Interrupt Mode
//#define ADC_Reg_IRQ_Mode   3
//#define ADC_Injec_IRQ_Mode  4
//-----------------------------------------------------

//3. REGU DMA Mode == Injec 모드에서는 Interrupt만 동작함 = DMA 기능은 없음
//#define ADC_Reg_DMA_Nomal_Mode  5
#define ADC_Reg_DMA_Circular_Mode 6
//===============================================================

//4. Run Mode
// single Ch
//#define single_con        7      // 싱글채널 비연속 모드 = (디스컨티뉴 모드)
//#define single_continuous_con  8    //  싱글채널 연속 모드 = 컨티뉴모드


// Mult CH
#define mult_single_con  9       // 멀티채널 비연속컨버전 = 디스컨티뉴 모드 = 싱글 컨버전 모드
//#define mult_continuous_con  10  // DMA에서는 필히 사용할 것, 멀티채널 연속모드 = 컨티뉴모드
//-------------------------------------------------------------------

//5. display Mode
//#define dis_Low   11 // tx_send
//#define dis_mid   12 // sprintf
#define dis_hi      13 // Dev_printf
//-----------------------------------------------------------------------

// ADC.c 소스코드 define
#define  AVG_LOOP    82  //default = 32, 42, 82

// adc Buffer
typedef struct
{
 uint16_t  ad_loop;
 int8_t    reg_ad_end_flag;
 int8_t    Inje_ad_end_flag;
 uint16_t  ad_data;
 uint8_t   avg_loop; //82
 int8_t    ad_ch_nb;  // 1 = 싱글채널, 싱글스탭
 int8_t    Injec_ch_nb;
 int8_t    ad_ch_cnt;

 uint16_t  ad_buf[20]; // 32
 uint32_t  Injec_buf[5];
 uint16_t  adc_data[20]; // 2026, 1, 26 추가 = AIOT 7기
 uint32_t  ad_avg[20];   // 2026, 05, 15 = AIOT 9기

 uint16_t  volt;
 float     ad_f;
 float     volt_f[20];
 int8_t    ad_run_flag;
 uint32_t  test_loop;

 // 2026년 05, 12추가 = AIOT 9기
 uint8_t  mode_cnt;
 uint8_t  ad_mode_flag;
}_ADC_;
extern _ADC_ adc_d;
extern _ADC_ *adc;

// 전류센서 모듈 ==> SEN0214
typedef struct
{
 double   Voltage;
 double   Amps;
 int8_t   ACSoffset; // 기준 값 0A일때 아날로그 값은 2500mV 이다.
 uint8_t  mVperAmp; //5A = 185, 20A = 100, 30A = 66
 double smoothAmps;
}_CUR_;
extern _CUR_ Current;
extern _CUR_ *cur;


#if dis_mid
  #include <stdio.h>
  #include <string.h>

#endif



#endif /* MY_LIB_ADC_MY_ARM_RTOS_ADC_LIB_V1_2_H_ */










