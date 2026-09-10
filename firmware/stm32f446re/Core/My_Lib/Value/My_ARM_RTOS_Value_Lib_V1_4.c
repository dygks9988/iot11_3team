/*
 * My_Value_V1_0.h
 *
 *  Created on: Jul 16, 2025
 *      Author: Administrator
 * 송명규
 * 010-2402-4398
 * mgsong@hanmail.net
 *
 * Ver UP List
 * V1.0 =  2025, 07, 16 FREERTOS 용으로 분리하여 작성 함 = AIOT 5기
 * V1.2 =  2025, 10, 23 범용 통합으로 작성함 = AIOT 6기
 * V1.3 =  2025, 11, 07 FREERTOS용 UART용 변수 추가함 = AIOT 6기
 * V1.4 =  2026, 01, 06 RTOS용으로 통합, 분리작성 = AIOT 7기
 */

#include "main.h"
#include "My_ARM_RTOS_Value_Lib_V1_4.h"


// RTOS
volatile __CHK_FLAG__  flag = {
  		        .run_flag  =  0,
		        .stop_flag =  0,
  		        .tg_flag   =  2,
  		        .t1_flag   =  0,
  		        .t2_flag   =  0,
  		        .on_flag   =  3,
  		        .redy_flag =  0,
		        .rx_q_flag =  5,
				.tm_flag   =  1,
				.tm_end_flag = 0,
				.sec_flag  =  2,
				.min_flag  =  0,
                .time_flag =  1,
				.Bit_flag  =  0,
				.Byte_flag =  0,
				.key_flag  =  0,
				.IRQ_Flag  =  0
       };
volatile __CHK_FLAG__  *fg = &flag;


// AIOT 6기 추가
volatile __TIME_T__ time_val = {
	            .sec = 0,
	            .min = 15,
			    .hour = 3,
			    .tm_loop = 0,
			    .t_loop = 0
          };
volatile __TIME_T__  *tm = &time_val;


// Key Sw Input Value
// 2026, 1, 13 추가 = AIOT 7기
volatile __KEY__   key_sw = {
		      .key = 0,
			  .buf = 0,
		      .key_buf = 0,
			  .key_loop = 0
           };
volatile __KEY__   *sw_key = &key_sw;


volatile __CNT__  cnt = {
		     .cnt16 = 1234,
		     .cnt8 = 0,
			 .t_loop = 0,
			 .time_loop = 0,
			 .mode_cnt = 0,
			 .dan = 0, // 2026, 1, 13 추가 = AIOT 7기
			 .gu = 0
            };
volatile __CNT__  *ct = &cnt;

volatile __CNT__  cnt1 = {1000, 1, 10, 0, 0,1, 1};
volatile __CNT__  *ct1 = &cnt1;

volatile __CNT__  cnt2 = {2000, 2, 20, 0, 0, 1, 1};
volatile __CNT__  *ct2 = &cnt2;


volatile uint8_t  d_buf[50];








