
// My_Lib
//#include "My_ARM_RTOS_ADC_Lib_V3_2.h"

// prj_Lib
#include "motor_instruction.h"
#include "actuator.h"


#define KP 8
#define KD 0 // todo
#define DT 0.1f



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

// IR 센서 센싱
static IRSensor_State_TypeDef ir_sensing(int8_t* error){
    
    IRSensor_State_TypeDef ir_state = 0;
    IRSensor_Pattern_TypeDef ir_pattern = 0;

    uint32_t adc_buf[5] = {0,0,0,0,0};
    uint8_t high_cnt = 0;

    // ADC 측정 My Lib 사용
    //adc_Mult_Polling_avg(&hadc1,adc_buf,5,5);

    // 라인 상태 확인
    for(uint8_t i = 0;i < 5; i++){
    if(adc_buf[i] > 2000)adc_buf[i] = 1;
    else adc_buf[i] = 0;

    if(adc_buf[i] == 1) high_cnt++;
    // 패턴 생성
    ir_pattern |= (adc_buf[i] << i);
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
    int16_t corretion = KP * error + KD * (error - prv_error) / DT;

    prv_error = error;

    return corretion;
}

// 모터 명령 생성 공개 API
bool motor_instruction_created(Motor_Instruction_MsgTypeDef* Instruction_Msg){
    int8_t error;
    int16_t corretion;

    // @todo 모드별 RPM
    int16_t right_mode_rpm = 30;
    int16_t left_mode_rpm = 30;

    IRSensor_State_TypeDef ir_state = ir_sensing(&error);

    switch (ir_state)
    {
        // @todo IR 상태별 제어 로직
    case IR_STATE_NONE:
        return false;
    case IR_STATE_ERROR:
        return false;
    case IR_STATE_NORMAL:
        corretion = pd_correction(error);

        left_mode_rpm += (corretion/5);
        right_mode_rpm -= (corretion/5);

        Instruction_Msg -> servo_angle = BASE_SERVO_ANGLE + corretion;
        Instruction_Msg -> right_dc_rpm = right_mode_rpm;
        Instruction_Msg -> left_dc_rpm = left_mode_rpm;
        break;
    default:
    return false;

    }
    return true;
}

