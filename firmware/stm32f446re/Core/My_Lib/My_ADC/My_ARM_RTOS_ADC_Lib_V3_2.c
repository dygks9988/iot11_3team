

 /*

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

#include <string.h>
#include <stdio.h>
#include "main.h"

#include <My_ARM_RTOS_ADC_Lib_V3_2.h>
#include <My_ARM_RTOS_Value_Lib_V1_4.h>
//#include <My_ARM_RTOS_GPIO_Lib_V4_1.h>
#include <My_ARM_RTOS_UART_Lib_V5_2.h>

#include "os_common.h"

// adc buffer
_ADC_  adc_d = {
		         .ad_loop = 0,
				 .reg_ad_end_flag = 0,
				 .Inje_ad_end_flag = 0,
				 .ad_data = 0,
				 .avg_loop = 82,   //82
				 .ad_ch_nb = 1,    // 1 = 싱글채널, 싱글스탭
				 .Injec_ch_nb = 1, //
				 .ad_ch_cnt = 0,  // Buffer [1]부터 시작
				 .ad_buf = {0},
				 .ad_avg = {0},   // 2026, 05, 15 = AIOT 9기 추가
				 .Injec_buf = {0,},
				 .adc_data = {0},  // 2026, 1, 26 추가 = AIOT 7기
				 .volt = 0,
				 .ad_f = 0.0,
				 .volt_f = {0.0,},
				 .ad_run_flag = 0,
				 .test_loop = 0,
				 .mode_cnt = 0,
				 .ad_mode_flag = 0
               };
_ADC_ *adc = &adc_d;


// 전류센서 모듈 ==> SEN0214
_CUR_  Current = {
				  .Voltage = 0,
				  .Amps = 0,
				  .ACSoffset = 0, // 기준 값 0A일때 아날로그 값은 2500mV 이다.
				  .mVperAmp = 0, //5A = 185, 20A = 100, 30A = 66
				  .smoothAmps = 0
                 };
_CUR_  *cyr = &Current;


uint8_t Injec_set_buf[4] = {ADC_INJECTED_RANK_1, ADC_INJECTED_RANK_2, ADC_INJECTED_RANK_3, ADC_INJECTED_RANK_4};



// Macro func
#define ad_ch_nb_set(value) adc -> ad_ch_nb = value


// 켈리브레이션
void adc_Calibration(ADC_HandleTypeDef* adcHandle)
{
 if(adcHandle->Instance==adcHandle){ //ADC1
   while(!HAL_ADC_PollForConversion(adcHandle, 50) != HAL_OK);
   printf("ADC Test OK\r\n\n");
  }
}

void Mult_Ch_Display(uint16_t abuf, char ch)
{
  char d_buf[40];
  uint16_t volt;
  float ad_f;

  uint8_t *ch_msg;

  //2025, 02, 05 추가
  (ch == 0) ? ch_msg = "REG Joy_X" :
  (ch == 1) ? ch_msg = "REG LM35DZ Temp_Sensor" :
  (ch == 2) ? ch_msg = "REG VR Volt" :
  (ch == 3) ? ch_msg = "REG Thr Temp Sensor" :
  (ch == 4) ? ch_msg = "REG DAC 1 Data" :
  (ch == 5) ? ch_msg = "REG DAC 2 Data" :
  (ch == 6) ? ch_msg = "REg Joy_Y" :
  (ch == 7) ? ch_msg = "REg CDS Data" :
  (ch == 8) ? ch_msg = "REG MCU Temp Sensor" : 0;

  // 2025, 02, 04 추가
#if dis_hi
  printf("Test_Loop = %d, %s = %d, Volt = %.4f\r\n", adc -> test_loop, ch_msg, abuf, abuf * (3.3/4095));
#elif dis_mid
  // 방법 1
  sprintf(d_buf,"Test_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, abuf, abuf * (3.3/4095));
  printf("%s", d_buf);

#elif dis_Low
  // Low data
  tx_str("ADC Data = ", 2);
  tx_send(adc -> ad_buf[0]/ 1000 + 0x30, 2);
  tx_send(adc -> ad_buf[0]% 1000/100 + '0', 2);
  tx_send(adc -> ad_buf[0]% 100/10 + 48, 2);
  tx_send(adc -> ad_buf[0]% 10 + 0x30, 2);
  tx_str(",  ", 2);

  // Low Lable ==>> FND?�� ?��?��?�� 경우
  ad_f = (abuf * (3.3/4095));//0.0008130081);
  volt = (int)(ad_f * 1000);

  tx_str("VOlt = ", 2);
  tx_send(volt/1000 + 0x30, 2);
  tx_send('.', 2);
  tx_send(volt%1000/100 + '0', 2);
  tx_send(volt%100/10 + 0x30, 2);
  tx_send(volt%10 + 0x30, 2);
  tx_str("\r\n", 2);
#endif
}


//void Single_Ch_Display(uint8_t selector[])  // 배열변수로 받을때
void Single_Ch_Display(uint8_t *selector)   // 배열변수를 포인터로 받을때
{
  uint32_t adc_buf[20];

/*
  // 방법 1
#if ADC_Injec_Polling_Mode || ADC_Injec_IT_Mode
  // Only Injec Mode = Interrupt, Polling
  memcpy(adc_buf, adc -> Injec_buf, sizeof(adc -> Injec_buf));
#else
  // dma, Interrupt. Polling == REG Mode
  memcpy(adc_buf, adc -> ad_buf, sizeof(adc -> ad_buf));
#endif
*/

  // 방법 2
  if(strcmp(selector, "REG") == 0)
   {
	 memcpy(adc_buf, adc -> ad_buf, sizeof(adc -> ad_buf));
	 tx_str("REG Mode ", 2);
   }
  else if(strcmp(selector, "Injec") == 0)
   {
	memcpy(adc_buf, adc -> Injec_buf, sizeof(adc -> Injec_buf));
	 tx_str("Iniec Mode ", 2);
   }

  #if dis_Low
    tx_str("Test_Loop = ", 2);
    tx_send(adc -> test_loop / 1000 + 0x30, 2);
    tx_send(adc -> test_loop % 1000/100 + '0', 2);
    tx_send(adc -> test_loop % 100/10 + 48, 2);
    tx_send(adc -> test_loop % 10 + 0x30, 2);

    tx_str(", ", 2);
    tx(selector, 2, strlen(selector));
    tx_str(" Data = ",2);

    tx_send(adc_buf[0]/ 1000 + 0x30, 2);
    tx_send(adc_buf[0]% 1000/100 + '0', 2);
    tx_send(adc_buf[0]% 100/10 + 48, 2);
    tx_send(adc_buf[0]% 10 + 0x30, 2);

    adc -> volt = (adc_buf[0] * ((3.3)/4095) * 1000);// 정규화
    tx_str(", Volt = ", 2);
    tx_send(adc -> volt/ 1000 + 0x30, 2);
    tx_send('.', 2);
    tx_send(adc -> volt% 1000/100 + '0', 2);
    tx_send(adc -> volt% 100/10 + 48, 2);
    tx_send(adc -> volt% 10 + 0x30, 2);
    tx_str("\r\n", 2);
  #elif dis_mid
	sprintf(adc -> adc_data, "T_Loop = %d, %s Data = %d, Volt = %f\r\n", adc -> test_loop, selector, adc_buf[0], adc_buf[0] * (3.3/4095));
	//tx_str(adc -> adc_data, 2);
	printf("%s", adc -> adc_data);
  #elif dis_hi
	printf("T_Loop = %d, %s Data = %d, Volt = %f\r\n", adc -> test_loop, selector, adc_buf[0], adc_buf[0] * (3.3/4095));//0.0008058608
  #endif
 if(adc -> reg_ad_end_flag == 0 && adc -> Inje_ad_end_flag == 0) memset(adc -> ad_buf, 0, 20 * sizeof(unsigned char));
}

// 2026, 01, 28 수정  = AIOT 7기
void Adc_Setup(ADC_HandleTypeDef *hadc, uint8_t reg_adc_ch_nb, uint8_t Injec_ch_nb)
{
 memset(adc -> ad_buf, 0, 20 *sizeof(uint8_t));
 memset(adc -> Injec_buf, 0, 5 *sizeof(uint8_t));

 adc_Calibration(hadc);

#if ADC_Reg_DMA_Nomal_Mode || ADC_Reg_DMA_Circular_Mode // 2026, 01, 28 수정
    //ad ch set
    ad_ch_nb_set(reg_adc_ch_nb);
    adc -> ad_ch_cnt = 0;
    adc -> test_loop = 0;
    HAL_ADC_Start_DMA(hadc, &adc -> ad_buf, adc -> ad_ch_nb);


#elif ADC_Injec_Polling_Mode && ADC_Reg_Polling_Mode
      // 폴링방식
      ad_ch_nb_set(reg_adc_ch_nb); // Reg Ch Data
      adc -> Injec_ch_nb = Injec_ch_nb;
      adc -> ad_ch_cnt = 0;
      adc -> test_loop = 0;
      HAL_ADC_Start(hadc);
      HAL_ADCEx_InjectedStart(hadc);

#elif ADC_Injec_IRQ_Mode && ADC_Reg_IRQ_Mode
      // IRQ
      ad_ch_nb_set(reg_adc_ch_nb); //reg ch set
      adc -> Injec_ch_nb = Injec_ch_nb;
      adc -> ad_ch_cnt = 0;
      adc -> test_loop = 0;
      adc  -> mode_cnt = 0;    // 2026, 05, 12 AIOT 9기 추가
     // adc -> ad_mode_flag = 0; // 2026, 05, 12 AIOT 9기 추가
      HAL_ADC_Start_IT(hadc);
      HAL_ADCEx_InjectedStart_IT(hadc);

#elif ADC_Injec_IRQ_Mode
    adc -> Injec_ch_nb = Injec_ch_nb;
    adc -> test_loop = 0;
    HAL_ADCEx_InjectedStart_IT(hadc);

#elif ADC_Reg_IRQ_Mode
    //ad ch set
    ad_ch_nb_set(reg_adc_ch_nb);
    adc -> ad_ch_cnt = 0;
    adc -> test_loop = 0;
    HAL_ADC_Start_IT(hadc);

#elif ADC_Injec_Polling_Mode
    adc -> Injec_ch_nb = Injec_ch_nb;
    adc -> ad_ch_cnt = 0;
    adc -> test_loop = 0;
    HAL_ADCEx_InjectedStart(hadc);

#elif ADC_Reg_Polling_Mode
    //ad ch set
    ad_ch_nb_set(reg_adc_ch_nb);
    adc -> ad_ch_cnt = 0; //멀티채널시 적용됨 = 채널카운터 및 채널비교 및 저장버퍼 인덱스값
    adc -> test_loop = 0;
    HAL_ADC_Start(hadc);
#elif ADC_Reg_Injec


#endif
}


#if ADC_Reg_Polling_Mode || ADC_Injec_Polling_Mode // #if-1
// 폴링방식 모드 == 2025, 11, 12 =  AIOT6기 추가
void Polling_Injec_single_ch_single_con(ADC_HandleTypeDef *hadc)
{
 HAL_ADCEx_InjectedStart(hadc);
  /* 변환 완료 대기 */
 HAL_ADCEx_InjectedPollForConversion(hadc, HAL_MAX_DELAY);
 adc -> Injec_buf[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
 HAL_ADCEx_InjectedStop(hadc);
 adc -> test_loop++;

 printf("Test_Lopp = %d, Jnjec Mode Data = %d, Volt = %f\r\n", adc -> test_loop, adc-> Injec_buf[0], adc-> Injec_buf[0] * (3.3/4095));
}

// 폴링방식 모드 == 2025, 11, 12 =  AIOT6기 추가
void Polling_Injec_single_ch_continu_con(ADC_HandleTypeDef *hadc)
{
 HAL_ADCEx_InjectedStart(hadc);
 /* 변환 완료 대기 */
 HAL_ADCEx_InjectedPollForConversion(hadc, HAL_MAX_DELAY);
 adc -> Injec_buf[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
 adc -> test_loop++;
 printf("Test_Loop = %d, Jnjec Mode Data = %d, Volt = %f\r\n", adc -> test_loop, adc-> Injec_buf[0], adc-> Injec_buf[0] * (3.3/4095));
}
//--------------------------------------------------------------------------------------------------------------------------------------------

// 폴링방식 모드 == 2025, 11, 12 =  AIOT6기 추가
void Injec_Poll_Mult_ch(ADC_HandleTypeDef *hadc)
{
  //2026, 01, 29 추가 = AIOT 7기
  uint8_t *ch_msg;

  // 스켄(멀티), 비연속모드(디스컨티뉴모드)
  for(int ch = 1; ch <=  adc -> Injec_ch_nb; ch++)
   {
	 /* Injected Conversion Start */
	 //2026, 01, 29 추가 = AIOT 7기
	 (ch == 1) ? ch_msg = "Injec VR Volt Data" :
     (ch == 2) ? ch_msg = "Injec DAC 2 Data" :
	 (ch == 3) ? ch_msg = "Injec Joy_X Data" :
	 (ch == 4) ? ch_msg = "Injec Joy_Y Data" : 0;

     #if mult_single_con || ADC_Injec_Polling_Mode // Injec 연속컨버전 모드 폴링시 추가
	 // 연속모드에서는 강제로 Start를 주며는 정상 동작함
	   HAL_ADCEx_InjectedStart(hadc);
     #endif

	 /* 변환 완료 대기 */
	 HAL_ADCEx_InjectedPollForConversion(hadc, HAL_MAX_DELAY);

	 /* 변환 결과 읽기 */
	 adc -> Injec_buf[ch] = HAL_ADCEx_InjectedGetValue(hadc, Injec_set_buf[ch]);

//	 if(k == 1) adc -> Injec_buf[k] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
//	 else if(k == 2) adc -> Injec_buf[k] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
//	 else if(k ==3) adc -> Injec_buf[k] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_3);
//	 else if(k==4) adc -> Injec_buf[k] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_4);

	 printf("Test_Loop = %d, %s = %d, Volt = %f\r\n", adc -> test_loop, ch_msg, adc -> Injec_buf[ch], adc -> Injec_buf[ch] * (3.3/4095));
     HAL_Delay(500);
   }
  printf("\r\n");

  #if mult_single_con
    HAL_ADCEx_InjectedStop(hadc);
  #endif

   adc -> test_loop++;
   memset(adc -> Injec_buf, 0, 4 * sizeof(unsigned char));
}
//-----------------------------------------------------------

// 방법 1 = 멀티채널 싱글(비연속/디스커티뉴), 컨티뉴(연속) 컨버전 = 초창기 버전
//         디스풀레이 출력을 직접 한거 = 직접 출력방식
// scan Channel - Single  Conversion Mode ==  Polling
void Mult_ch_single_con(ADC_HandleTypeDef* adcHandle)
{
  //2026, 01, 29 추가 = AIOT 7기
  uint8_t *ch_msg;

 //scan Channel - Single Single Conversion == Polling
 for(int ch = 0; ch <= adc -> ad_ch_nb-1; ch++)
  {
	//2026, 01, 29 추가 = AIOT 7기
	//2026, 05, 13 추가 = AIOT 9기
	(ch == 0) ? ch_msg = "REG Joy_X" :
	(ch == 1) ? ch_msg = "REG LM35DZ Temp_Sensor" :
	(ch == 2) ? ch_msg = "REG VR Volt" :
	(ch == 3) ? ch_msg = "REG Thr Temp Sensor" :
	(ch == 4) ? ch_msg = "REG DAC 1 Data" :
	(ch == 5) ? ch_msg = "REG DAC 2 Data" :
	(ch == 6) ? ch_msg = "REg Joy_Y" :
	(ch == 7) ? ch_msg = "REg CDS Data" :
	(ch == 8) ? ch_msg = "REG MCU Temp Sensor" : 0;

   #if mult_single_con
	// 연속모드에서는 강제로 Start를 줘도 동작하지 않음
	  HAL_ADC_Start(adcHandle);
   #endif
	HAL_ADC_PollForConversion(adcHandle, 100);  //busy
	adc -> ad_buf[ch] = HAL_ADC_GetValue(adcHandle);
	 //HAL_ADC_Stop(adcHandle);
	// 직접 display 출력한거
	// adc 버퍼값도 직접 출력
	printf("Test_Loop = %d, %s = %d, Volt = %f\r\n", adc -> test_loop, ch_msg, adc -> ad_buf[ch], adc -> ad_buf[ch] * (3.3/4095));
	HAL_Delay(500);
  }
printf("\r\n");

adc -> test_loop++;

#if mult_single_con
   HAL_ADC_Stop(adcHandle);
#endif
memset(adc -> ad_buf, 0, 20 *sizeof(uint16_t));
}

// 방법 2 = 멀티채널 싱글(비연속/디스커티뉴), 컨티뉴(연속) 컨버전 = 추천
// Mult_Ch Single & Contiinuous Conversiond == Polling
void adc_Mult_Ch_Polling(ADC_HandleTypeDef* adcHandle, uint32_t *adc_p, int8_t adc_ch_nb)
{
  //HAL_ADC_Start(adcHandle);  // Scan Channel, Contiinuous Conversiondl일때

  for(int i = 0; i <= adc_ch_nb-1; i++)
    {
	  // 연속모드에서는 강제로 Start를 줘도 동작하지 않음
      #if mult_single_con
	    HAL_ADC_Start(adcHandle); //Scan Channel, Single Conversion일때   //(&hadc1);
      #endif

	  HAL_ADC_PollForConversion(adcHandle, 100);
	  adc_p[i] = HAL_ADC_GetValue(adcHandle); //hadc1

	  adc -> test_loop++;

	  Mult_Ch_Display(adc_p[i], i);
	  HAL_Delay(500);
    }
  printf("\r\n");

#if mult_single_con
  HAL_ADC_Stop(adcHandle);
#endif
}

// 2024, 08, 29 추가 및 수정 보강
// Single Channel - Single Contiinuous Mode ==  Polling
void Polling_single_ch_continu_con(ADC_HandleTypeDef* adcHandle)
{
  HAL_ADC_PollForConversion(adcHandle, 100);
  adc -> ad_buf[0] = HAL_ADC_GetValue(adcHandle); //hadc1

  // 2026, 1, 26 추가
  adc -> test_loop++;

  // 2025m 02, 04 추가
 #if dis_Low
  tx_str("Test_Loop = ", 2);
  tx_send(adc -> test_loop / 1000 + 0x30, 2);
  tx_send(adc -> test_loop % 1000/100 + '0', 2);
  tx_send(adc -> test_loop % 100/10 + 48, 2);
  tx_send(adc -> test_loop % 10 + 0x30, 2);

  tx_str(", AD data  = ", 2);
  tx_send(adc -> ad_buf[0]/ 1000 + 0x30, 2);
  tx_send(adc -> ad_buf[0]% 1000/100 + '0', 2);
  tx_send(adc -> ad_buf[0]% 100/10 + 48, 2);
  tx_send(adc -> ad_buf[0]% 10 + 0x30, 2);

  adc -> volt = (adc -> ad_buf[0] * ((3.3)/4095) * 1000);// 정규화
  tx_str(", Volt = ", 2);
  tx_send(adc -> volt/ 1000 + 0x30, 2);
  tx_send('.', 2);
  tx_send(adc -> volt% 1000/100 + '0', 2);
  tx_send(adc -> volt% 100/10 + 48, 2);
  tx_send(adc -> volt% 10 + 0x30, 2);
  tx_str("\r\n", 2);
 #elif dis_mid
  sprintf(adc -> adc_data,"T_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));
  //tx_str(adbuf, 2);
  printf("%s", adc -> adc_data);
 #elif dis_hi
  printf("T_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));
 #endif
}

// 2024, 08, 28 추가 및 수정 보강
// Single Channel - Single Conversion Mode ==  Polling
void Polling_single_ch_single_con(ADC_HandleTypeDef* adcHandle)
 {
  HAL_ADC_Start(adcHandle);  //Single Channel, Single Conversion일때   //(&hadc1);
  HAL_ADC_PollForConversion(adcHandle, 100);
  adc -> ad_buf[0] = HAL_ADC_GetValue(adcHandle); //hadc1
  HAL_ADC_Stop(adcHandle);

  //2026, 01, 26 추가 = AIoT 7기 추가
  adc -> test_loop++;

  // 2025m 02, 04 추가
 #if dis_Low
  tx_str("T_Loop = ", 2);
  tx_send(adc -> test_loop/ 1000 + 0x30, 2);
  tx_send(adc -> test_loop% 1000/100 + '0', 2);
  tx_send(adc -> test_loop% 100/10 + 48, 2);
  tx_send(adc -> test_loop% 10 + 0x30, 2);

  tx_str(", ADC Data = ", 2);
  tx_send(adc -> ad_buf[0]/ 1000 + 0x30, 2);
  tx_send(adc -> ad_buf[0]% 1000/100 + '0', 2);
  tx_send(adc -> ad_buf[0]% 100/10 + 48, 2);
  tx_send(adc -> ad_buf[0]% 10 + 0x30, 2);

  adc -> volt = (adc -> ad_buf[0] * ((3.3)/4095) * 1000);// 정규화
  tx_str(",  Volt = ", 2);
  tx_send(adc -> volt/ 1000 + 0x30, 2);
  tx_send('.', 2);
  tx_send(adc -> volt% 1000/100 + '0', 2);
  tx_send(adc -> volt% 100/10 + 48, 2);
  tx_send(adc -> volt% 10 + 0x30, 2);
  tx_str("\r\n", 2);
 #elif dis_mid
  sprintf(adc -> adc_data,"T_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));
  //tx_str(adbuf, 2);
  printf("%s", adc -> adc_data);
 #elif dis_hi
  printf("T_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));
 #endif
}


// Mult_Ch Single & Contiinuous Conversiond == Polling
// 82번 평균 낸거
void adc_Mult_Polling_avg(ADC_HandleTypeDef* adcHandle, uint32_t *adc_p, int8_t adc_ch_nb, int8_t avg_loop)
{
  int8_t i, k;

  // buff init
  for(i = 0; i < adc_ch_nb; i++)
   {
	adc_p[i]=0;
   }

  for(k = 0; k < avg_loop; k++)	// avg
   {
     for(i = 0; i < adc_ch_nb; i++)
      {
    	HAL_ADC_Start(adcHandle); //&hadc1);
        HAL_ADC_PollForConversion(adcHandle, 100); //&hadc,
        adc_p[i] += HAL_ADC_GetValue(adcHandle);   //(&hadc);
        HAL_Delay(1);
      }
     HAL_ADC_Stop(adcHandle); //(&hadc1);
   }

  for(i = 0; i < adc_ch_nb; i++)
   {
    adc_p[i] = (adc_p[i] / avg_loop);

   // 2026, 8, 1추가
   #if debugging
     Mult_Ch_Display(adc_p[i], i);
   #endif
   HAL_Delay(10);
   }

  // 2026, 8, 1추가
  #if debugging
    printf("\r\n");
  #endif
}


// AIOT 6기 추가 =  2025, 7, 13추가
void Polling_single_ch_avg(ADC_HandleTypeDef* hadc, uint8_t loop)
{
 adc -> avg_loop = loop;

 for(int i = 0; i <= adc -> avg_loop; i++)
  {
    #if ADC_Reg_Polling_Mode && ADC_Injec_Polling_Mode

	 // REG Mode + Injec Mode
	 HAL_ADC_Start(hadc);// ADC1
	 HAL_ADC_PollForConversion(hadc, 100);
	 adc -> ad_buf[0] += HAL_ADC_GetValue(hadc); //hadc1
	 HAL_ADC_Stop(hadc);
	 //-----------------------
	 HAL_ADCEx_InjectedStart(hadc);
	 /* 변환 완료 대기 */
	 HAL_ADCEx_InjectedPollForConversion(hadc, HAL_MAX_DELAY);
	 adc -> Injec_buf[0] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
	 HAL_ADCEx_InjectedStop(hadc);
    #elif ADC_Reg_Polling_Mode
      // REG Mode
	  HAL_ADC_Start(hadc);// ADC1
	  HAL_ADC_PollForConversion(hadc, 100);
	  adc -> ad_avg[0] += HAL_ADC_GetValue(hadc); //hadc1
	  HAL_ADC_Stop(hadc);
    #elif ADC_Injec_Polling_Mode
	  // Injec Mode
	  HAL_ADCEx_InjectedStart(hadc);
   	  /* 변환 완료 대기 */
	  HAL_ADCEx_InjectedPollForConversion(hadc, HAL_MAX_DELAY);
	  adc -> Injec_buf[0] += HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
	  HAL_ADCEx_InjectedStop(hadc);
    #endif

	HAL_Delay(10);
  }

adc -> test_loop++;

// Display
#if ADC_Reg_Polling_Mode && ADC_Injec_Polling_Mode
   // REG Mode + Injec Mode display
   adc -> ad_buf[0] /= adc -> avg_loop;
   adc -> Injec_buf[0] /= adc -> avg_loop;
   printf("Reg T_Loop = %d, ADC data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));//0.0008058608
   printf("Injec T_Loop = %d, ADC data = %d, Volt = %f\r\n", adc -> test_loop, adc -> Injec_buf[0], adc -> Injec_buf[0] * (3.3/4095));//0.0008058608
   memset(adc -> ad_buf, 0, 20 * sizeof(unsigned char));
   memset(adc -> Injec_buf, 0, 5 * sizeof(unsigned char));
#elif ADC_Reg_Polling_Mode
  // REG Mode display
  adc -> ad_avg[0] /= adc -> avg_loop;
   #if dis_Low
     tx_str("Test_Loop = ", 2);
     tx_send(adc -> test_loop / 1000 + 0x30, 2);
     tx_send(adc -> test_loop % 1000/100 + '0', 2);
     tx_send(adc -> test_loop % 100/10 + 48, 2);
     tx_send(adc -> test_loop % 10 + 0x30, 2);

     tx_str(", AD data  = ", 2);
     tx_send(adc -> ad_buf[0]/ 1000 + 0x30, 2);
     tx_send(adc -> ad_buf[0]% 1000/100 + '0', 2);
     tx_send(adc -> ad_buf[0]% 100/10 + 48, 2);
     tx_send(adc -> ad_buf[0]% 10 + 0x30, 2);

     adc -> volt = (adc -> ad_buf[0] * ((3.3)/4095) * 1000);// 정규화
     tx_str(", Volt = ", 2);
     tx_send(adc -> volt/ 1000 + 0x30, 2);
     tx_send('.', 2);
     tx_send(adc -> volt% 1000/100 + '0', 2);
     tx_send(adc -> volt% 100/10 + 48, 2);
     tx_send(adc -> volt% 10 + 0x30, 2);
     tx_str("\r\n", 2);
   #elif dis_mid
     sprintf(adc -> adc_data,"T_Loop = %d, ADC Data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_buf[0], adc -> ad_buf[0] * (3.3/4095));
     //tx_str(adbuf, 2);
     printf("%s", adc -> adc_data);
   #elif dis_hi
     printf("T_Loop = %d, ADC data = %d, Volt = %f\r\n", adc -> test_loop, adc -> ad_avg[0], adc -> ad_avg[0] * (3.3/4095));//0.0008058608
     memset(adc -> ad_avg, 0, 20 * sizeof(unsigned char));
   #endif
#elif ADC_Injec_Polling_Mode
   //Injec Mode display
   adc -> Injec_buf[0] /= adc -> avg_loop;
   printf("T_Loop = %d, ADC data = %d, Volt = %f\r\n", adc -> test_loop, adc -> Injec_buf[0], adc -> Injec_buf[0] * (3.3/4095));//0.0008058608
   memset(adc -> Injec_buf, 0, 5 * sizeof(unsigned char));
#endif
}
//=====================================================================================================================================================

// ADC Interrupt Mode
#elif ADC_Reg_IRQ_Mode || ADC_Injec_IRQ_Mode //#if-1-2
// 2026, 01, 27 추가 == AIot 7기
  // adc Channel set == // 1 = 싱글채널

//void Single_Ch_IRQ_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t selector[]) //배열변수로 받을때
void Single_Ch_IRQ_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t *selector) // 배열변수를 포인터로 받을때
   {
     //싱글채널 싱글컨버전 = Interrupt
     adc -> test_loop++;

     // display
     Single_Ch_Display(selector); //selector = 1 = REG Mode

     //아래 코드는 일정한 시간 간격으로 할때
     HAL_Delay(delay);
     #if single_con
       HAL_ADC_Start_IT(hadc);
     #endif
   }

  // 2026, 01, 30 추가 = AIOT 7기
  // 멀티채널 Interrupt Display
  void Mult_Ch_IRQ_Func(ADC_HandleTypeDef *hadc, uint32_t *p,  uint16_t delay)
   {
     adc -> reg_ad_end_flag = 0;

     for(int i = 0; i <= adc -> ad_ch_nb-1; i++)
       {
         Mult_Ch_Display(p[i], i);
         adc -> test_loop++;
         HAL_Delay(100);
       }
      printf("\r\n\n");

     HAL_Delay(delay);

     #if mult_single_con
       HAL_ADC_Start_IT(hadc);
     #endif
   }

  //Injec Mode == 2026, 1, 27 == AIoT 7기
//  void Injec_Single_Ch_IRQ_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t selector[]) // 배열변수로 받을때
  void Injec_Single_Ch_IRQ_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t *selector) // 배열변수를 포인터로 받을때
   {
      //싱글채널 싱글컨버전 = Interrupt
      adc -> Inje_ad_end_flag= 0;

      // display
      adc -> test_loop++;
      Single_Ch_Display(selector); // selector = 2 = Inje

      //아래 코드는 일정한 시간 간격으로 할때
      HAL_Delay(delay);
      #if single_con
        HAL_ADCEx_InjectedStart_IT(hadc);
      #endif
   }

void Injec_scan_IT(ADC_HandleTypeDef *hadc)
{// 멀티채널
  HAL_ADCEx_InjectedStart_IT(hadc);
}

void Regu_scan_IT(ADC_HandleTypeDef *hadc)
{ // 멀티채널
  HAL_ADC_Start_IT(hadc);
}

// 2026, 01, 30 추가 = AIOT 7기
// 멀티채널 Interrupt Display
void Mult_Ch_Injec_IRQ_Func(ADC_HandleTypeDef *hadc, uint32_t *p,  uint16_t delay)
 {
  char adbuf[200];

    adc -> Inje_ad_end_flag = 0;
    adc -> test_loop++; // 2026, 5, 12추가 // AIOT9기

    #if dis_mid || dis_hi
     sprintf(adbuf, " T_Loop = %d,\r\n VR Data = %d,\r\n "
    		 "DAC 2 Data = %d,\r\n  Joy_X Data = %d,\r\n Joy_Y Data = %d,\r\n",
			 adc -> test_loop, p[0], p[1], p[2], p[3]);

	   printf("%s\r\n", adbuf);
     #endif

     HAL_Delay(delay);

     #if mult_single_con // Injec 연속 컨버전 모드에서는 삭제해야됨 = 수동으로시작 = 그렇지않으면 동작안함
       HAL_ADCEx_InjectedStart_IT(hadc);
     #endif
}


//====================================================================
// DMA Mode
#elif ADC_Reg_DMA_Nomal_Mode || ADC_Reg_DMA_Circular_Mode // #if-1-3

//void Single_DMA_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t selector[]) //배열변수로 받을경우
void Single_DMA_Func(ADC_HandleTypeDef *hadc, uint16_t delay, uint8_t *selector) // 배열변수를 포인터로 받을경우
{
 //DMA
 adc -> reg_ad_end_flag = 0;

 adc -> test_loop++;

 // display
 Single_Ch_Display(selector); //selector = 1 = REG Mode

 //아래 코드는 일정한 시간 간격으로 할때
 HAL_Delay(delay);
 #if ADC_Reg_DMA_Nomal_Mode
   HAL_ADC_Start_DMA(hadc, &adc -> ad_buf, adc -> ad_ch_nb);
 #endif
}

// 2026, 01, 30 추가 = AIOT 7기
// 멀티채널 Interrupt Display
void Mult_Ch_DMA_Func(ADC_HandleTypeDef *hadc, uint32_t *p,  uint16_t delay)
 {
     char adbuf[300];

     adc -> reg_ad_end_flag = 0;
     // 2026, 01, 30 추가
     adc -> test_loop++;

     #if dis_mid || dis_hi
       sprintf(adbuf, "DMA T_Loop = %d,\r\n MCU_Pwr = %d,\r\n VR Data = %d,\r\n "
    		 "LM35 Data = %d,\r\n Thr Temp Data = %d,\r\n MCU Temp Data = %d,\r\n "
			 "cds Data = %d,\r\n joy_x Data = %d,\r\n joy_y = %d\r\n Mcu_Temp = %d\r\n",
			 adc -> test_loop,  adc -> ad_buf[0],  adc -> ad_buf[1],  adc -> ad_buf[2],  adc -> ad_buf[3],  adc -> ad_buf[4],  adc -> ad_buf[5],  adc -> ad_buf[6],  adc -> ad_buf[7], adc -> ad_buf[8]);

	   printf("%s\r\n\n", adbuf);
     #endif

     HAL_Delay(delay);

     #if ADC_Reg_DMA_Nomal_Mode
       HAL_ADC_Start_DMA(hadc, &adc -> ad_buf, adc -> ad_ch_nb);
     #endif
 }

#endif
//------------------------------------------------------------------------


/* ---- Injected 변환 완료 콜백 ---- */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  // AIOT 9기때 대폭 수정 = 2026, 05, 14
 if(hadc -> Instance == ADC1)
  { // if-1
	// AIOT 9기때 대폭 수정 = 2026, 05, 14
    #if ADC_Injec_IRQ_Mode
	  #if single_con //single ch single con
	     HAL_ADCEx_InjectedStop_IT(hadc);
		 adc -> Injec_buf[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
		 adc -> Inje_ad_end_flag = 1;
         // Display 후에 다시 시작할 것

      #elif single_continuous_con
		 adc -> Injec_buf[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
		 adc -> Inje_ad_end_flag = 1;

      #elif mult_single_con // 비연속모드 = 디스컨티뉴모드
		 HAL_ADCEx_InjectedStop_IT(hadc);
		 adc -> Injec_buf[adc -> ad_ch_cnt] = HAL_ADCEx_InjectedGetValue(hadc, Injec_set_buf[adc -> ad_ch_cnt]);

  	     // AIOT 9기 수정 = 2026, 05, 14
		 if(adc -> ad_ch_cnt >= (adc -> Injec_ch_nb-1)) // org = 1
		  {
		    HAL_ADCEx_InjectedStop_IT(hadc);
		    adc -> ad_ch_cnt = 0;
		    adc -> Inje_ad_end_flag = 1;
		  }
		 else
		  {
		   adc -> ad_ch_cnt++; // // AIOT 9기 수정 = 2026, 05, 14
		   HAL_ADCEx_InjectedStart_IT(hadc);
           __NOP();
		   asm("nop");
		  }

         #elif mult_continuous_con
		   adc -> Injec_buf[adc -> ad_ch_cnt] = HAL_ADCEx_InjectedGetValue(hadc, Injec_set_buf[adc -> ad_ch_cnt]);

		   if(adc -> ad_ch_cnt >= (adc -> Injec_ch_nb-1)) // org = 1
		     {
		      adc -> ad_ch_cnt = 0;
		      adc -> Inje_ad_end_flag = 1;
   		     }
		   else
		    {
			 // AIOT 9기 수정 = 2026, 05, 14
			 adc -> ad_ch_cnt++;
		    }
         #endif
      #endif
  }


/*
   참고용으로 보세요
      for(int k = 1; k <= adc -> Injec_ch_nb; k++)
       {
    	  adc -> Injec_buf[k] = HAL_ADCEx_InjectedGetValue(hadc, Injec_set_buf[k]);
    	  if(k == adc -> Injec_ch_nb) adc -> Inje_ad_end_flag = 1;

    	 /*
    	 if(k == 1)
    	  {
    	   adc -> Injec_buf[0] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
    	  }
    	 else if(k == 2)
    	 {
    	  adc -> Injec_buf[1] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
    	 }
    	 else if(k == 3)
    	 {
    	  adc -> Injec_buf[2] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_3);
    	 }
    	 else if(k== 4)
    	 {
    	   adc -> Injec_buf[3] = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_4);
    	 }
    	if(k == adc -> Injec_ch_nb) adc -> Inje_ad_end_flag = 1;
    	//*/
       //} // for-end
}


void XferCpltCallback(ADC_HandleTypeDef* hdma)
{
 // xxIT.C ADC DMA 핸들러에 콜백함수를 선언 해줘야하고
 //	실습은 멀티채널할때== 3개 채널만 나옴 = 비추천
 //adc -> reg_ad_end_flag = 1; //rtos = dma방식

 #if ADC_Reg_DMA_Nomal_Mode
   HAL_ADC_Stop_DMA(hdma);
 #endif
}

// REG Mode
// ADC Call back func
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* adcHandle)
{
 // xxIT.C ADC DMA 핸들러에 콜백함수를 선언 안 해줘도 됨
 // 전 채널 다 나옴 == 값도 잘 나옴
 // 가급적 이 콜백함수 사용 == 적극추천
#if ADC_Reg_DMA_Nomal_Mode || ADC_Reg_DMA_Circular_Mode
	//adc -> reg_ad_end_flag = 1; //rtos = dma방식

	// 프로젝트용 rtos semaphore
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(adcSemaphoreHandle , &xHigherPriorityTaskWoken);
	// 필요하면 context switch
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);


   #if ADC_Reg_DMA_Nomal_Mode
	  HAL_ADC_Stop_DMA(adcHandle);
   #endif


#elif ADC_Reg_IRQ_Mode
	if (__HAL_ADC_GET_FLAG(adcHandle, ADC_FLAG_EOC))
	 //if (adcHandle->Instance == ADC1)
	  {
         #if single_con
		   HAL_ADC_Stop_IT(adcHandle);
		   adc -> ad_buf[0] = HAL_ADC_GetValue(adcHandle);
		   adc -> reg_ad_end_flag = 1;
		   // Display 후에 다시 시작할 것

         #elif single_continuous_con
		   adc -> ad_buf[0] = HAL_ADC_GetValue(adcHandle);
		   adc -> reg_ad_end_flag = 1;

         #elif mult_single_con // 비연속모드 = 디스컨티뉴모드
		   HAL_ADC_Stop_IT(adcHandle);
		   adc -> ad_buf[adc -> ad_ch_cnt] = HAL_ADC_GetValue(adcHandle);

		   // AIOT 9기 수정 = 2026, 05, 14
		   if(adc -> ad_ch_cnt >= (adc -> ad_ch_nb-1)) // org = 1
		    {
			 HAL_ADC_Stop_IT(adcHandle);
		     adc -> ad_ch_cnt = 0;
		     adc -> reg_ad_end_flag = 1;
		    }
		   else
		    {
			  adc -> ad_ch_cnt++; // // AIOT 9기 수정 = 2026, 05, 14
			  HAL_ADC_Start_IT(adcHandle);
              __NOP();
		      asm("nop");
		    }

         #elif mult_continuous_con
		   adc -> ad_buf[adc -> ad_ch_cnt] = HAL_ADC_GetValue(adcHandle);

		   if(adc -> ad_ch_cnt >= (adc -> ad_ch_nb-1)) // org = 1
		     {
		      adc -> ad_ch_cnt = 0;
		   	  adc -> reg_ad_end_flag = 1;
   		     }
		   else
		    {
			 // AIOT 9기 수정 = 2026, 05, 14
			 adc -> ad_ch_cnt++;
		    }
         #endif
	 }
#endif
}

