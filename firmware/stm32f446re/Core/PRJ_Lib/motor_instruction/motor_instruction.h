#ifndef __MOTOR_INSTRUCTION__
#define __MOTOR_INSTRUCTION__

#include "c_stdlib.h"
#include "msg.h"



void line_sensor_init();
bool motor_instruction_create(Motor_Instruction_MsgTypeDef* Instruction_Msg,uint16_t* ir_sensor);


#endif
