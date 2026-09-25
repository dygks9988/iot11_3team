#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "c_stdlib.h"
#include "msg.h"

#include "tim.h"

#define BASE_SERVO_ANGLE 90

typedef struct{
    uint16_t target_rpm;
    int16_t current_rpm;
    uint16_t max_rpm;

    TIM_HandleTypeDef* pwm_htim;
    uint32_t pwm_channel;

    TIM_HandleTypeDef* encoder_htim;
    float encoder_cpr;
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
