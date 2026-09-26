#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "c_stdlib.h"
#include "msg.h"

#include "tim.h"


// 현재 프로젝트 일정과 협업관리상 하드웨어 종속을 분리하기 어려움
// 서로 다른 모터를 각각 제어하기 위해 모터 핸들러에 MAX RPM 멤버를 추가

// 2026-09-24 모터 드라이버 L298 H-브릿지 구성 EN에 PWM 신호, IN1,IN2의 GPIO핀 구성이 동작을 하지 않음을 확인
// 이 이상 해결하려고 하면 프로젝트 진행에 차질이 있을 거라고 판단 프로젝트 범위를 축소하여 역회전 구동 기능을 제외한다.
// EN핀에 점퍼캡을 끼우고, IN1핀 PWM, IN2 GND 방식으로 회로를 구성할 예정이다.

// L298 EN핀이 3.3 전압으로 동작 하지 않는 것을 확인, PWM 신호의 승압 회로를 구성하기는 어렵다고 판단, 계획을 수정하지 않는다.

// @todo encoder pid, 직접 조종

// 2026-09-25 각 모터 축의 속도를 맞추기 위해 ccr_gain 추가
typedef struct{
    uint16_t target_rpm;
    int16_t current_rpm;
    uint16_t max_rpm;

    TIM_HandleTypeDef* pwm_htim;
    uint32_t pwm_channel;

    TIM_HandleTypeDef* encoder_htim;
    float encoder_cpr;

    float ccr_gain;
}Dc_Motor_HandleTypeDef;

extern Dc_Motor_HandleTypeDef* right_dcmotor_front;
extern Dc_Motor_HandleTypeDef* right_dcmotor_rear;
extern Dc_Motor_HandleTypeDef* left_dcmotor_front;
extern Dc_Motor_HandleTypeDef* left_dcmotor_rear;

void actuator_init();
void actuator_process(Motor_Instruction_MsgTypeDef* instruction_msg);

void encoder_init();
void update_motor_rpm(Dc_Motor_HandleTypeDef *dc);
void dis_rpm();

#endif
