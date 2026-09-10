
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


#ifndef  __My_ARM_RTOS_GPIO_Lib__
#define  __My_ARM_RTOS_GPIO_Lib__

// 2025, 12, 24 수정 추가 == RTOS용
#define STM32F446       1
#define HAL_Lib         2

// Output Pin 설정시에만 활성화 할것
#define Out_Pin_Define_Nb1  3
//#define TIM_En   1

// Input Pin 설정시에만 활성화 할것
//#define In_Pin_Define_Nb1   4

// 2025, 06, 10 추가 = AIOT 4기생
#if STM32F446
   #include "stm32f4xx_hal.h" // F446 시리즈
   #include "main.h"
#elif STM32F103
   #include "stm32f1xx_hal.h"  // F103, F시리즈
   #include "main.h"
#elif STM32L152
   #include "stm32l1xx_hal.h"    //L152/151/162 시리즈
   #include "main.h"
#endif


#define Low  GPIO_PIN_RESET  //0
#define Hi   GPIO_PIN_SET    // 1

// 소스방식
#define ON    1
#define OFF   0

// 싱크방식
#define _ON   0
#define _OFF  1

                                   
// Key Input Reg                                   
#define Key_In_Pull_Down  2    // Key in 액티브 Hi
//#define Key_In_Pull_UP    3    // Key in 액티브 Low     
                                   

// Output Pin Define
// 2025, 04, 01 == AIOT 2기 추가
// 출력 핀 정의 = 핀번호는 PRj에 맞게 수정하여 사용하세요
// 2025, 12, 24 수정 == AIOT 7기
#if Out_Pin_Define_Nb1
  // Pin 맵핑 방법 1 == 권장 추천
  #define LED0_PIN   LED1_Pin
  #define LED1_PIN   LED2_Pin
  #define LED2_PIN   LED3_Pin
  #define LED3_PIN   LED4_Pin
  #define LED4_PIN   LED5_Pin
  #define LED5_PIN   LED6_Pin
  #define LED6_PIN   LED7_Pin
  #define LED7_PIN   LED8_Pin
#elif Out_Pin_Define_Nb3
  // Pin 맵핑 방법 3 = 강추, 고급기법 = 가장 좋음 = 핀 + 포트 같이 묶기
  typedef struct {
	  GPIO_TypeDef *port;
	  const uint16_t pin;
  } port_pin;

  #define LED0_PIN  {GPIOC, led0_Pin}
  #define LED1_PIN  {GPIOC, led1_Pin}
  #define LED2_PIN  {led2_GPIO_Port, led2_Pin}
  #define LED3_PIN  {GPIOC, led3_Pin}
  #define LED4_PIN  {GPIOC, GPIO_PIN_4}
  #define LED5_PIN  {GPIOC, led5_Pin}
  #define LED6_PIN  {led2_GPIO_Port, GPIO_PIN_4}
  #define LED7_PIN  {led7_GPIO_Port, led7_Pin}
  //ex: HAL_GPIO_WritePin(leds[0].port, leds[0].pin, GPIO_PIN_SET);
#elif Out_Pin_Define_Nb4
  // Pin 맵핑 방법 4 = 비추천 = 타입에러 발생소지 있음
  typedef enum
   {
	 LED0_PIN = led0_Pin,
	 LED1_PIN = led1_Pin,
	 LED2_PIN = led2_Pin,
	 LED3_PIN = led3_Pin,
	 LED4_PIN = GPIO_PIN_4,
	 LED5_PIN = GPIO_PIN_5,
	 LED6_PIN = led6_Pin,
	 LED7_PIN = led7_Pin
   } led_pin;
#elif Out_Pin_Define_Nb5
   // Pin 맵핑 방법 5 = 비추천(사용하지 말것) = 구조체 변수 형식 = 메모리 낭비 초래
   typedef struct
    {
     const uint16_t LED0_PIN;
     const uint16_t LED1_PIN;
     const uint16_t LED2_PIN;
     const uint16_t LED3_PIN;
     const uint16_t LED4_PIN;
     const uint16_t LED5_PIN;
     const uint16_t LED6_PIN;
     const uint16_t LED7_PIN;
    } GPIO_OUT;

  extern GPIO_OUT  Out;
  extern GPIO_OUT  *Gpio_Out;
#endif

// 2025, 06, 19 추가 == AIOT4기 추가
typedef enum
 {
  #if Out_Pin_Define_Nb1 || Out_Pin_Define_Nb4
     OUT_Bit_0 = LED0_PIN,
     OUT_Bit_1 = LED1_PIN,
     OUT_Bit_2 = LED2_PIN,
     OUT_Bit_3 = LED3_PIN,
     OUT_Bit_4 = LED4_PIN,
     OUT_Bit_5 = LED5_PIN,
     OUT_Bit_6 = LED6_PIN,
     OUT_Bit_7 = LED7_PIN
  #else Out_Pin_Define_Nb3  || Out_Pin_Define_Nb2 || Out_Pin_Define_Nb5
     OUT_Bit_0 = GPIO_PIN_0,
     OUT_Bit_1 = GPIO_PIN_1,
     OUT_Bit_2 = GPIO_PIN_2,
     OUT_Bit_3 = GPIO_PIN_3,
     OUT_Bit_4 = GPIO_PIN_4,
     OUT_Bit_5 = GPIO_PIN_5,
     OUT_Bit_6 = GPIO_PIN_6,
     OUT_Bit_7 = GPIO_PIN_7
  #endif
 } GPIO;

extern GPIO io;
//======================================================

// Input Pin define
// 프로젝트에 맞게 핀 번호 맵핑하여 수정하면서 사용하세요...
#if In_Pin_Define_Nb1
   #define IN_Bit_0  sw1_pin
   #define IN_Bit_1  sw2_pin
   #define IN_Bit_2  sw3_pin
   #define IN_Bit_3  sw4_pin
   #define IN_Bit_4  sw5_pin
   #define IN_Bit_5  sw6_pin
   #define IN_Bit_6  sw7_pin
   #define IN_Bit_7  sw8_pin
#elif In_Pin_Define_Nb3

#elif In_Pin_Define_Nb4

#endif

//==========================================================================================
// 매크로함수 선언 == 2025, 12, 30 추가 편집
// Bit IO방식 Control Lib

//1. GPIO OUTPUT LIB  == RMW 방식
// RMW 방식
#define D_Out(port, pin, sts)  HAL_GPIO_WritePin(port, pin, sts)
#define Out_Bit(port, pin, sts)  HAL_GPIO_WritePin(port, pin, sts)
#define Out_Bit_Tg(port, pin)  HAL_GPIO_TogglePin(port, pin)

//2. Ver 3.2 = 2020, 04, 21
#define Out_Bit_A(pin, out_data_bits) HAL_GPIO_WritePin(GPIOA, pin, out_data_bits)
#define Out_Bit_B(pin, out_data_bits) HAL_GPIO_WritePin(GPIOB, pin, out_data_bits)
#define Out_Bit_C(pin, out_data_bits) HAL_GPIO_WritePin(GPIOC, pin, out_data_bits)

//3. Ver 3.2 = 2020, 04, 21
#define Led_out_A_bit(pin, out_data_bits) Out_Bit_A(pin, (out_data_bits) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define Led_out_B_bit(pin, out_data_bits) Out_Bit_B(pin, (out_data_bits) ? GPIO_PIN_SET : GPIO_PIN_RESET)
#define Led_out_C_bit(pin, out_data_bits) Out_Bit_C(pin, (out_data_bits) ? GPIO_PIN_SET : GPIO_PIN_RESET)

//4. 2023, 07, 14 추가 = Low Lable Codeing = OK
#define Bit_out_H_P(GPIOx, Bit) (GPIOx -> ODR |= (1 << Bit))
#define Bit_out_H_N(GPIOx, Bit) (GPIOx -> ODR |= ~(0 << Bit))
#define Bit_out_L_P(GPIOx, Bit) (GPIOx -> ODR &= (0 << Bit))
#define Bit_out_L_N(GPIOx, Bit) (GPIOx -> ODR &= ~(1 << Bit))
//ex == Bit_out_H_N(led1_GPIO_Port, 7); == Bit를 비트 번호로 넣어야 함

//5. BSRR 레지스터 활용 Bit Control Lib == OK

// 레지스터 사용법 = 2025, 06, 20 AIOT 4기 추가
// 이 방법 추천 = ODR 사용하는 RMW 방식보단 안전 = 특히 Interrupt 사용시 안전
// ODR 방식 RWM 방식 사용시 인터럽트 방식에서 에기치 못한 오동작이 발생할 소지가 없지않아 있음
// 즉 중간에 인터럽트기 끼면 다른 핀 상태가 깨질 수 있음
// 그래서 BSRR 레지스터 방식 사용 권장 == RTOS 나 Interrupt 사용시 안전 = BSRR은 쓰기 한 번으로 끝남
// BSRR 레지스터 하위 16비트 == SET(HI, 1 출력시 사용)
// BSRR 레지스터 상위 16비트 == RESET(LOW, 0 출력시 사용)
#define GPIO_Pin_Low(GPIOx, Pin) (GPIOx -> BSRR = (1 << (Pin + 16)))
#define GPIO_Pin_Hi(GPIOx, Pin) (GPIOx -> BSRR = (1 << Pin))
// EX:  GPIO_Pin_Hi(GPIOA, 3)

//6. 여러핀 동시제어하기
// 2023, 07, 14 추가 = Low Lable Codeing
#define Mult_Out_Bit_Hi(GPIOx, Pins) (GPIOx -> BSRR = Pins)
#define Mult_Out_Bit_Low(GPIOx, Pins) (GPIOx -> BSRR = (Pins << 16)) // + 16해도 됨
// #define Mult_Out_Bit_Low(GPIOx, Pins) \   // 줄 바꾸어 쓰기
//         ((GPIOx) -> BSRR = ((Pins) << 16)) // + 16해도 됨
// ex : Mult_Out_Bit_Hi(GPIOA, (1U << 7) | (1U << 6) | (1U << 3))
//      Mult_Out_Bit_Low(GPIOA, (1U << 7) | (1U << 6) | (1U << 3))

//7. 동시에 Hi/Low 제어하기
#define Mult_Out_Bit_Low_Hi(GPIOx, Hi_Pins, Low_Pins) (GPIOx->BSRR = (Low_Pins << 16) | Hi_Pins)
// ex : PA7, PA6 Low / PA3, PA2 Hi
//      mult_Out_Bit_Low_Hi(GPIOC, (1U << 7) | (1U << 6), (1U << 3) | (1U << 2))

//8. 2025, 12, 25 추가정리
#define GPIO_PIN_HIGH(GPIOx, Pin) (GPIOx->BSRR = (1 << Pin))
#define GPIO_PIN_LOW(GPIOx, Pin)  (GPIOx->BSRR = (1 << (Pin + 16)))
#define GPIO_PIN_TOGGLE(GPIOx, Pin) (GPIOx->ODR ^= (1 << Pin))

#define GPIO_PINS_HIGH(GPIOx, Mask) (GPIOx->BSRR = Mask)
#define GPIO_PINS_LOW(GPIOx, Mask)  (GPIOx->BSRR = (Mask << 16))
#define GPIO_PINS_SET_RESET(GPIOx, SMask, RMask) \
        (GPIOx->BSRR = (RMask << 16) | SMask)

// Bit Input
//9. GPIO INPUT LIB
#define In_Bit(port, pin) HAL_GPIO_ReadPin(port, pin)
//=============================================================================

// 레지스터 사용법 = 2025, 06, 20 AIOT 4기 추가
// inline 의미 == 해당함수 호출대신 해당함수 코드 삽입, MCU 속도향상, 짧고 자주사용하는 함수에 좋음, static inline 연동
// 빌드시 컴파일러에 호출한 함수를 호출하지 말고 함수 내용 자체를 그자리에 복사하듯 넣어줘 라는 의미 의 C언어 키워드임
// 즉 함수를 호출하는 코드에서 그함수 자체를 호출하는게 아니라 그 함수의 코드 내용을 그 함수를 호출하는위치에 복사하여
// 그 코드가 직접 그위치에 삽입될 가능성이 높음
// 함수 호출대신 코드 자체를 삽입하라는 힌트로 짧고 자주쓰는 함수에서 속도향상을 위해 사용
// GPIO 제어처럼 빠른 실행이 중요한 코드에서 자주 사용됨 = 사용권장
// 함수호출 오버헤드가 없고, GPIO토글, 틸레이 비드조작에 메우 유리하다
// 함수 호출 오버헤드 없이 바로 레지스터 접근 가능
// 주로 사용하는경우 == 코드가 짧을때, 자주 호출될때, 하드웨어 제어시(GPIO, 터이머, SPI Bit-Banging), 실시간 성능이 중요할때
//                 단 코드가 길때는 코드 사이즈가 증가하는 문제가 있을 수 있다.
// 그러나 컴파일러가 상황에 따라서 무시할 수도 있음
// 컴파일러가 무시하는 경우 == 함수 코드가 너무 큼, 재귀함수, 주소흫 사용하는경우, 최적화 옵션 OFF(-O0)
// 최적화 옵션(-O2,-O3)에 따라 효과가 달라질수 있음
static inline void GPIO_Pin_Out_Low(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
  // Bit 번호로 설정해야 됨
  GPIOx->BSRR = (1U << (Pin + 16));
}

static inline void GPIO_Pin_Out_Hi(GPIO_TypeDef *GPIOx, uint16_t Pin)
{
  // Bit 번호로 설정해야 됨
  GPIOx->BSRR = (1U << Pin);
}

// 2025, 12, 24 == AIOT 7기 추가
static inline void sts_tg(GPIO_TypeDef *GPIOx, uint8_t sts_flag, uint8_t bits)
{
 // Bit 번호로 설정해야 됨
 if(sts_flag ==1) GPIOx -> BSRR = (1U << bits);
 else GPIOx -> BSRR = (1U << (bits + 16));
}

#endif






