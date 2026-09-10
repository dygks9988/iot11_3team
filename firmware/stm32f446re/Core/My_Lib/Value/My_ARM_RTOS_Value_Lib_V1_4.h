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
 * V1.0 = 2025, 07, 16 FREERTOS 용으로 분리하여 작성 함 = AIOT 5기
 * V1.2 = 2025, 10, 23 범용 통합으로 작성함 = AIOT 6기
 * V1.3 = 2025, 11, 07 FREERTOS용 UART용 변수 추가함 = AIOT 6기
 * V1.4 = 2026, 01, 06 RTOS용으로 통합, 분리작성 = AIOT 7기
 */


#ifndef __My_ARM_LIB_VALUE__
#define __My_ARM_LIB_VALUE__


// default
typedef struct
  {
   uint16_t run_flag   : 1; // 0-7
   uint8_t  stop_flag  : 1;
   uint8_t  tg_flag    : 2;
   uint8_t  t1_flag    : 1;
   uint8_t  t2_flag    : 1;
   uint8_t  on_flag    : 3;
   uint8_t  redy_flag  : 1;
   uint16_t  rx_q_flag : 3;
   uint8_t  tm_flag    : 2;
   uint8_t  tm_end_flag : 1;
   uint8_t  sec_flag   : 3;
   uint8_t  min_flag   : 1;
   uint16_t  time_flag : 2;
   uint8_t  Bit_flag   : 1;
   uint8_t  Byte_flag  : 1;
   uint8_t  key_flag   : 1;  // 2026, 1, 13 추가 = AIOT 7기
   uint16_t IRQ_Flag   : 3;  // 2026, 1, 14 추가 = AIOT 7기
  }__CHK_FLAG__;

extern  volatile __CHK_FLAG__  flag;
extern  volatile __CHK_FLAG__  *fg;


// AIOT 6기 추가
typedef struct
{
  uint8_t  sec;
  uint8_t  min;
  uint8_t  hour;
  uint16_t tm_loop;
  uint8_t  t_loop;
}__TIME_T__;
extern  volatile __TIME_T__  time_val;
extern  volatile __TIME_T__  *tm;

// Key Sw Input Value
// 2026, 1, 13 추가 = AIOT 7기
typedef struct
{
  uint8_t  key;
  uint8_t  buf;
  uint16_t key_buf;
  uint16_t key_loop;
}__KEY__;

extern volatile __KEY__  key_sw;
extern volatile __KEY__  *sw_key;


typedef struct
{
 uint16_t cnt16;
 uint8_t  cnt8;
 int16_t  t_loop;
 int16_t  time_loop;
 int8_t   mode_cnt;
 int8_t   dan;
 int8_t   gu;
}__CNT__;
extern  volatile __CNT__  cnt;
extern  volatile __CNT__  *ct;

extern  volatile __CNT__  cnt1;
extern  volatile __CNT__  *ct1;

extern  volatile __CNT__  cnt2;
extern  volatile __CNT__  *ct2;

extern volatile uint8_t d_buf[50];




#endif /* PRJ_LIB_MY_VALUE_V1_0_H_ */
