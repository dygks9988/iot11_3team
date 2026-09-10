
// My ARM RTOS GPIO Lib
// Song Myoung Gyu
// 010-2402-4398
// End Ver = V4.0 = 2025, 12, 24

// Ver1.0 = 2018, 07, 09
// Ver2.0 = 2019, 11, 08 //
// Ver3.0 = 2020, 03, 12
//    - AVR GPIO Bit  = bit_set, bit_clr, bit_tg, bit-chk
//    - AVR GPIO Port = PortA_L, PortA_H, PortB_L, PortB_H, PortC_L, PortC_H,
// Ver3.2 = 2020, 04, 21
//      - gpio write func Macro func 추가
// Ver 3.4 == 2024, 01, 10 = BIt func 추가
// Ver 3.5 == 2024, 01, 16 = Arduino Map 함수 구현 추가
// Ver 3.6 == 2025, 04, 03 = 임의의 포트의 임의의 Bit로 8Bit Port 구성 함수 추가 = AIOT 1개생
// Ver 3.7 == 2025, 06, 10 = Bit 제어 함수 추가 = AIOT 4개생
// Ver 3.8 == 2025, 06, 20 = 임의이 포트의 임의의 Bit로 8Bit 입력 Portr구성 함수 추가
// Ver 3.9 == 2025, 12, 24 = Bit, Byte, Word 제어용 함수 추가
// Ver 4.0 == 2025, 12, 24 = RTOS 용으로 개편 == AIOT 7기
// Ver 4.1 == 2025, 12, 30 = RTOS 용으로 개편 추가편집 및 동작검증 완료 == AIOT 7기


#include "main.h"
#include "My_ARM_RTOS_GPIO_Lib_V4_1.h"


// Output Pin Define
// 2025, 04, 01 == AIOT 2기 추가
// 프로젝트에 맞게 핀 번호 맵핑하여 수정하면서 사용하세요...
#if Out_Pin_Define_Nb2
  // Pin 맵핑 방법 2 == 권장 = 핀 번호를 상수로 선언 = 자료형 타입 안전성 중시할때....
  const uint16_t LED0_PIN = led0_Pin;
  const uint16_t LED1_PIN = led1_Pin;
  const uint16_t LED2_PIN = led2_Pin;
  const uint16_t LED3_PIN = led3_Pin;
  const uint16_t LED4_PIN = GPIO_PIN_4;
  const uint16_t LED5_PIN = GPIO_PIN_5;
  const uint16_t LED6_PIN = led6_Pin;
  const uint16_t LED7_PIN = led7_Pin;
#elif Out_Pin_Define_Nb3
 port_pin led_pins[] = {
		  LED0_PIN,
		  LED1_PIN,
		  LED2_PIN,
		  LED3_PIN,
		  LED4_PIN,
		  LED5_PIN,
		  LED6_PIN,
		  LED7_PIN
    };
#elif Out_Pin_Define_Nb4
   // enum 변수
   led_pin  Led_Pins; // enum 변수
#elif Out_Pin_Define_Nb5
  // Pin 맵핑 방법 5 = 비추천(사용하지 말것) = 구조체 변수 형식 = 메모리 낭비 초래
  GPIO_OUT Out = {
      		led0_Pin,
      		led1_Pin,
      		led2_Pin,
      		led3_Pin,
      		led4_Pin,
      		led5_Pin,
      		led6_Pin,
      		led7_Pin   //GPIO_PIN_7
         }; // 핀 번호는 프로젝트에 맞게 수정하여 사용하세요

   GPIO_OUT *Gpio_Out = &Out;
#endif

// 2025, 06, 19 추가 == AIOT4기 추가
GPIO io; //enum 변수
uint8_t gpio_io[8] = {OUT_Bit_0,OUT_Bit_1,OUT_Bit_2,OUT_Bit_3,OUT_Bit_4,OUT_Bit_5,OUT_Bit_6,OUT_Bit_7};
//=========================================================================

// Input Pin define
// 프로젝트에 맞게 핀 번호 맵핑하여 수정하면서 사용하세요...
#if In_Pin_Define_Nb2
   const uint16_t IN_Bit_0 = sw1_pin;
   const uint16_t IN_Bit_1 = sw2_pin;
   const uint16_t IN_Bit_2 = sw3_pin;
   const uint16_t IN_Bit_3 = sw4_pin;
   const uint16_t IN_Bit_4 = sw5_pin;
   const uint16_t IN_Bit_5 = sw6_pin;
   const uint16_t IN_Bit_6 = sw7_pin;
   const uint16_t IN_Bit_7 = sw8_pin;
#elif In_Pin_Define_Nb3

#elif In_Pin_Define_Nb4

#endif

// 2025, 06, 10 추가 = AIOT4기생 추가
#if In_Pin_Define_Nb1 || In_Pin_Define_Nb2
   uint8_t Gpio_In[8] = {IN_Bit_0,IN_Bit_1,IN_Bit_2,IN_Bit_3,IN_Bit_4,IN_Bit_5,IN_Bit_6,IN_Bit_7};
#endif
//==========================================================================

// 2025, 12, 25 AIOT 7기 전면 추가 및 수정함
#if HAL_Lib

//1. RMW 방식 Bit Func Lib == 2024, 01, 10 추가
void bit_tg(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
  // 사용시 주의 필요 = 특히 아래 bit_tg_1 하고 연속하여 같이 사용하지 마세요
  // 8421 코드값으로 설정해야 됨
  uint32_t buf;

  buf = GPIOx->ODR;
  GPIOx-> ODR = ~(buf & Bit); // GPIO_PIN_5 잘됨
}

// AIOT 4기생 == 2025, 06, 10 추가검증 완료
void bit_tg_1(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
  // 전체 토클
  // Bit 번호로 설정해야 됨
  GPIOx-> ODR ^= (1 << Bit); // 5는 잘됨, GPIO_PIN_5는 안됨
}

void bit_set_func(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
 Out_Bit(GPIOx, Bit, GPIO_PIN_SET);
}

// AIOT 4기생 == 2025, 06, 10 추가검증 완료
void bit_set_func_1(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
 // Bit 번호로 설정해야
 GPIOx ->ODR |= (1 << Bit); // org = GPIO_PIN_5 동작안함
}

void bit_clr_func(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
  Out_Bit(GPIOx, Bit, GPIO_PIN_RESET);
}

// AIOT 4기생 == 2025, 06, 10 추가검증 완료
void bit_clr_func_1(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
 // Bit 번호로 설정해야
 GPIOx ->ODR &= ~(1 << Bit); //org = GPIO_PIN_5 동작함
}

// AIOT 2기생 == 2025, 03, 24 추가검증 완료
int bit_chk_func(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
  return (GPIOx -> ODR & (1<< Bit)); //GPIO_PIN_5 동작 안됨
}
//*****************************************************

// Byte / Word IO 방식 Control Lib

//2. Byte / Word Out Lib

// AIOT 5기 추가 == 2025, 06, 11
void bit_to_byte(GPIO_TypeDef *GPIOx, uint16_t Bit)
{
 uint16_t buf = 0;

 for(int k = 7; k >=0; k--)
  {
   if((Bit & 0x80) == 0x80) buf |= (1 << k);
   else buf &= ~(1 << k);
   Bit <<= 1;
  }

 GPIOx -> ODR = ~buf;
}

// AIOT 4기생 == 2025, 06, 11 추가검증 완료
// AIOT 6기생 == 2025, 10, 14 추가보강함
void bit_to_word(GPIO_TypeDef *GPIOx, uint16_t data, uint16_t Bit)
{
  //Bits = 8Bit: 7, 16Bit : 15, MAX 16Bit
  uint16_t buf = 0;

  for(int k = 0; k <= Bit; k++)
   {
	if((data & 0x80) == 0x80) buf |= (1 << k);
	else buf &= ~(1 << k);

	data <<= 1;
   }

 GPIOx -> ODR = ~buf; // 싱크방식이므로 반전함
}

// AIOT 5기 == 2025, 08, 12
// My User Port
void My_Port_Byte_Wr(unsigned char data)
{
 Out_Bit(GPIOB, OUT_Bit_0, data%2);     // bit 0 = lsb
 Out_Bit(GPIOC, OUT_Bit_1, data/2%2);   // bit 1
 Out_Bit(GPIOC, OUT_Bit_2, data/4%2);   // bit 2
 Out_Bit(GPIOA, OUT_Bit_3, data/8%2);   // bit 3
 Out_Bit(GPIOB, OUT_Bit_4, data/16%2);  // bit 4
 Out_Bit(GPIOB, OUT_Bit_5, data/32%2);  // bit 5
 Out_Bit(GPIOB, OUT_Bit_6, data/64%2);  // bit 6
 Out_Bit(GPIOB, OUT_Bit_7, data/128%2); // bit 7 = msb
}

//2016, 05, 08 추가
// Byte, Word Control
#define Port_Byte_out(GPIOx, Data) GPIOx -> ODR = Data
#define Port_Byte_out_H(GPIOx, Data) GPIOx -> ODR = ((GPIOx -> ODR & 0x00ff) | Data)
#define Port_Byte_out_L(GPIOx, Data) GPIOx -> ODR = ((GPIOx -> ODR & 0xff00) | Data)

void Byte_out_L(GPIO_TypeDef *GPIOx, uint16_t out_data) // uint32_t = org = data sheet ��
{
  // Low 8bit out = default
  // ver 3.0
  uint16_t buf;

  buf =  GPIOx -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0xff00;
  GPIOx -> ODR = (buf | out_data);

  // old ver = v2.0
  // GPIOx -> ODR = (out_data & 0x00ff);
}

void Byte_out_H(GPIO_TypeDef *GPIOx, uint16_t out_data) // uint32_t = org = data sheet ��
{
   // High 8bit out
   // ver 3.0 = 2020,03,13 추가
   uint16_t buf;

   buf =  GPIOx -> ODR; // GPIOx PORT PIN ���¸� ����
   buf &= 0x00ff;
   out_data <<= 8;
   GPIOx -> ODR = (buf | out_data);

	/*
	// old ver = v2.0
	out_data <<= 8;
	GPIOx -> ODR = (out_data & 0xffff);
	*/
}

void Word_out(GPIO_TypeDef *GPIOx, uint16_t out_data)
{
   // 16 bit out
	GPIOx -> ODR = out_data;
}

//2019, 11, 08 추가
void PortA_out_L(uint16_t out_data)
{
  // Low 8bit out = default
  // ver 3.0
  uint16_t buf;

  buf =  GPIOA -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0xff00;
  GPIOA -> ODR = (buf | out_data);
}

void PortA_out_H(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가
  uint16_t buf;

  buf =  GPIOA -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0x00ff;
  out_data <<= 8;
  GPIOA -> ODR = (buf | out_data);
}

void PortA_out_WD(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가

  // 16 bit out
  GPIOA -> ODR = out_data;
}

void PortB_out_L(uint16_t out_data)
{
  // Low 8bit out = default
  // ver 3.0
  uint16_t buf;

  buf =  GPIOB -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0xff00;
  GPIOB -> ODR = (buf | out_data);
}

void PortB_out_H(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가
  uint16_t buf;

  buf =  GPIOB -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0x00ff;
  out_data <<= 8;
  GPIOB -> ODR = (buf | out_data);
}

void PortB_out_WD(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가

  // 16 bit out
  GPIOB -> ODR = out_data;
}

void PortC_out_L(uint16_t out_data)
{
  // Low 8bit out = default
  // ver 3.0
  uint16_t buf;

  buf =  GPIOC -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0xff00;
  GPIOC -> ODR = (buf | out_data);
}

void PortC_out_H(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가
  uint16_t buf;

  buf =  GPIOC -> ODR; // GPIOx PORT PIN ���¸� ����
  buf &= 0x00ff;
  out_data <<= 8;
  GPIOC -> ODR = (buf | out_data);
}

void PortC_out_WD(uint16_t out_data)
{
  // High 8bit out
  // ver 3.0 = 2020,03,13 추가

  // 16 bit out
  GPIOC -> ODR = out_data;
}
//-----------------------------------------------------------

//3. Byte / Word Input Lib
// 2025, 06, 10 추가 = AIOT4기생 추가
// My User Input Port Func
#if In_Pin_Define_Nb1 || In_Pin_Define_Nb2
   int My_Port_Byte_Rd()
    {
      uint16_t buf = 0;

      buf = In_Bit(GPIOA,  IN_Bit_0) * 1;     // bit 0 = lsb
      buf |= In_Bit(GPIOA, IN_Bit_1) * 2;   // bit 1
      buf |= In_Bit(GPIOA, IN_Bit_2) * 4;   // bit 2
      buf |= In_Bit(GPIOB, IN_Bit_3) * 8;   // bit 3
      buf |= In_Bit(GPIOB, IN_Bit_4) * 16;  // bit 4
      buf |= In_Bit(GPIOB, IN_Bit_5) * 32;  // bit 5
      buf |= In_Bit(GPIOB, IN_Bit_6) * 64;  // bit 6
      buf |= In_Bit(GPIOB, IN_Bit_7) * 128; // bit 7 = msb

      return buf;
    }

  // 2026, 08, 16 추가  
 void My_Byte_Key_rd()
 {
   uint8_t buf = 0;

    buf  =  (In_Bit(sw1_GPIO_Port, sw1_Pin) << 0);       // bit 0 * 1 = lsb
    buf  |= (In_Bit(sw2_GPIO_Port, sw2_Pin) << 1);       // bit 1 * 2
    buf  |= (In_Bit(sw3_GPIO_Port, sw3_Pin) << 2);       // bit 2 * 4
    buf  |= (In_Bit(sw4_GPIO_Port, sw4_Pin) << 3);       // bit 3 * 8
    buf  |= (In_Bit(sw5_GPIO_Port, sw5_Pin) << 4);       // bit 4 * 16
    buf  |= (In_Bit(sw6_GPIO_Port, sw6_Pin) << 32);     // bit 5 * 32
    buf  |= (In_Bitn(sw6_GPIO_Port, sw6_Pin) << 64);     // bit 6 * 64
    buf  |= (In_Bit(sw6_GPIO_Port, sw6_Pin) << 128);     // bit 7 * 128

#if Key_In_Pull_Down // Key in 액티브 Hi
   return (int)buf;
#elif Key_In_Pull_UP // Key In 엑티브 Low
  retuen (int)(~buf & 0x3f);
#endif
 }   
#endif

uint16_t Byte_rd_L(GPIO_TypeDef *GPIOx)
{
  // Low 8 bit input
  uint16_t buf;
  buf =  GPIOx -> IDR;
  return (buf & 0x00ff);
}

uint16_t Byte_rd_H(GPIO_TypeDef *GPIOx)
{
  // high 8bit input
  uint16_t buf;

  buf =  GPIOx -> IDR;
  return (buf & 0xff00);
}

uint16_t Word_rd(GPIO_TypeDef *GPIOx)
{
  // 16bit input = 2020, 03, 13 ����
  uint16_t buf;

  buf = GPIOx -> IDR;
  return buf;
}

uint16_t PortA_rd_L(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOA -> IDR;
  return (buf & 0x00ff);
}

uint16_t PortA_rd_H(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOA -> IDR;
  return (buf & 0xff00);
}

uint16_t PortA_rd_WD(void)
{
  int16_t buf;

  buf = GPIOA -> IDR;
  return buf;
}

uint16_t PortB_rd_L(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOB -> IDR;
  return (buf & 0x00ff);
}

uint16_t PortB_rd_H(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOB -> IDR;
  return (buf & 0xff00);
}

uint16_t PortB_rd_WD(void)
{
  int16_t buf;

  buf = GPIOB -> IDR;
  return buf;
}

uint16_t PortC_rd_L(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOC -> IDR;
  return (buf & 0x00ff);
}

uint16_t PortC_rd_H(void)
{
  // Low 8 bit input
  // ver 3.0
  uint16_t buf;

  buf =  GPIOC -> IDR;
  return (buf & 0xff00);
}

uint16_t PortC_rd_WD(void)
{
  int16_t buf;

  buf = GPIOC -> IDR;
  return buf;
}
//-----------------------------------------------------------

//4. System Control

// SyStem delay LIB = 2023, 05, 17 = IOT 2기
#define Delay_ms(val)  HAL_Delay(val)

// Arduino Map 함수 구현 == 2024, 01, 16 추가
long my_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 2025, 06, 20 AIOT 4기 추가
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
 GPIO_InitTypeDef GPIO_InitStruct = {0};
 GPIO_InitStruct.Pin = GPIO_Pin;
 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
 HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
 GPIO_InitTypeDef GPIO_InitStruct = {0};
 GPIO_InitStruct.Pin = GPIO_Pin;
 GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
 GPIO_InitStruct.Pull = GPIO_PULLUP;
 HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

#if TIM_En
 // Timer Interrupt 사용시 적용 == 2026, 2, 12 추가 = AIOT 7기
 void Delay_Us(TIM_HandleTypeDef *htim, uint16_t set_us_time)
  {
   // 1usec time delay func
   // 50mhz/ 50(49-1) = 1mhz = t : 1us
   // 이벤트 주기는 1usec
   uint16_t buf;

   buf = __HAL_TIM_GET_COUNTER(htim);

   while((uint16_t)(__HAL_TIM_GET_COUNTER(htim) - buf) < set_us_time);
}

void Delay_Ms(TIM_HandleTypeDef *htim, uint16_t set_ms_time)
{
  // 1usec time delay func
  // 50mhz/ 50(49-1) = 1mhz = t : 1us
  // 이벤트 주기는 1usec
  uint16_t buf;

  while(set_ms_time--)
   {
	buf = __HAL_TIM_GET_COUNTER(htim);
	while((uint16_t)(__HAL_TIM_GET_COUNTER(htim) - buf) < 1000);
  }
}

// 초음파 센서에 사용 == 캡처 인터럽트 콜백함수 안에서 호출
// Trg 신호 Hi 출력 후 10usec delay시 호출
void My_Delay_Us(TIM_HandleTypeDef *htim, uint16_t time)
{
  __HAL_TIM_SET_COUNTER(htim, 0);
  while(__HAL_TIM_GET_COUNTER(htim) < time);
}
#endif
//========================================================

#elif LL_Lib

//========================================================

#elif CMSYS_Lib

//===========================================================

#elif STD_Lib
  // 초창기 라이브러리로 더이상 업그레이드 안되고, 지원 중지하여
  // 지금은 사용할 수 없습니다.


#endif



