//My_Lib
#include "My_ARM_RTOS_ADC_Lib_V3_2.h"

// prj_Lib
#include "motor_instruction.h"
#include "actuator.h"

// HAL_Lib
#include "adc.h"
#include "tim.h"

#define IR_THRESHOLD 2000
#define IR_NB 5


#define KP 8
#define KD 0 // todo
#define DT 0.01f



// NONE = 라인이탈
typedef enum{
    IR_STATE_NONE = 0,
    IR_STATE_ERROR,
    IR_STATE_NORMAL 
}IRSensor_State_TypeDef;

typedef enum{
    IR_RIGHT1 = 0b00001,
    IR_RIGHT2 = 0b00011,
    IR_RIGHT3 = 0b00010,
    IR_RIGHT4 = 0b00110,
    IR_MIDDLE = 0b00100,
    IR_LEFT1 = 0b01100,
    IR_LEFT2 = 0b01000,
    IR_LEFT3 = 0b11000,
    IR_LEFT4 = 0b10000
}IRSensor_Pattern_TypeDef;



static int8_t prv_error;





// 라인판단
static IRSensor_State_TypeDef line_detect(int8_t* error, uint16_t* ir_sensor){
    
    IRSensor_State_TypeDef ir_state = 0;
    uint8_t ir_pattern = 0;

    uint8_t line_buf[IR_NB] = {0,0,0,0,0};

    uint8_t high_cnt = 0;

    // ADC 측정 My Lib 사용
    // 라인 상태 확인

    for(uint8_t i = 0;i < IR_NB; i++){
    if(ir_sensor[i] > IR_THRESHOLD)line_buf[i] = 1;
    else line_buf[i] = 0;

    if(line_buf[i] == 1) high_cnt++;
    // 패턴 생성
    ir_pattern |= (line_buf[i] << i);
    }

    // 에러 확인
    if(high_cnt == 0)ir_state = IR_STATE_NONE;
    else if(high_cnt >= 3)ir_state= IR_STATE_ERROR;
    else ir_state= IR_STATE_NORMAL;

    // 오차 생성
    switch(ir_state){
        case IR_STATE_NONE:
        return IR_STATE_NONE;

        case IR_STATE_ERROR:
        return IR_STATE_ERROR;
        
        case IR_STATE_NORMAL:
        switch (ir_pattern){
            case IR_RIGHT1:
                *error = 4;
                break;
            case IR_RIGHT2:
                *error = 3;
                break;
            case IR_RIGHT3:
                *error = 2;
                break;
            case IR_RIGHT4:
                *error = 1;
                break;
            case IR_MIDDLE:
                *error = 0;
                break;
            case IR_LEFT1:
                *error = -1;
                break;
            case IR_LEFT2:
                *error = -2;
                break;
            case IR_LEFT3:
                *error = -3;
                break;
            case IR_LEFT4:
                *error = -4;
                break;
            default:
                // 정의된 라인이 아니라면 IR_STATE_ERROR
                return IR_STATE_ERROR;
            }

    }
    return IR_STATE_NORMAL;
}

// PD 제어 방향 값 리턴
static inline int16_t pd_correction(int8_t error){
    int16_t correction = KP * error + KD * (error - prv_error) / DT;

    prv_error = error;

    return correction;
}

// 라인 센서 init
void line_sensor_init(){
	// 학원 라이브러리 사용
	// DMA 서큘러
	// ADC 타임트리거, 스캔 컨버전 모드 사용
	// 타임 트리거 주기 10ms
	Adc_Setup(&hadc1, IR_NB, 0);
	HAL_TIM_Base_Start(&htim8);

}


// 모터 명령 생성 공개 API
// DMA의 시퀸스가 끝나면 동작하는 프로세스
// RTOS IPC로 태스크가 레디 되어야한다
// My_Lib adc -> ad_buf가 인자값으로 들어와야 한다

bool motor_instruction_create(Motor_Instruction_MsgTypeDef* Instruction_Msg,uint16_t* ir_sensor){
    int8_t error;
    int16_t correction;

    // @todo 모드별 RPM
    uint16_t right_mode_rpm = 100;
    uint16_t left_mode_rpm = 100;

    IRSensor_State_TypeDef ir_state = line_detect(&error,ir_sensor);


    switch (ir_state)
    {
        // @todo IR 상태별 제어 로직

    case IR_STATE_NONE:
        return false;
    case IR_STATE_ERROR:
    	// 다음 샘플링을 기다리는 구조로 가면 좋을 것 같다 에러 카운트
    	return false;
    case IR_STATE_NORMAL:
    	correction = pd_correction(error);

        left_mode_rpm += (correction/3);
        right_mode_rpm -= (correction/3);

        Instruction_Msg -> right_dc_rpm = right_mode_rpm;
        Instruction_Msg -> left_dc_rpm = left_mode_rpm;

        break;
    default:
    return false;

    }
    return true;
}

