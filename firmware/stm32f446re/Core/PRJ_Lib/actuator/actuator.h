#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "c_stdlib.h"
#include "msg.h"


#define BASE_SERVO_ANGLE 90


void actuator_init();
void actuator_process(Motor_Instruction_MsgTypeDef instruction_msg);



#endif
