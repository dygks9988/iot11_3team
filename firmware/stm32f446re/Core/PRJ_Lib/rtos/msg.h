#ifndef MSG_H
#define MSG_H

#include "c_stdlib.h"

// 공유 자원 폴더
// 다른 작업자와 공유자원 작업을 할 경우 이 폴더 변수를 사용할 것
// 공유되는 작업이 있다면 반드시 해당 작업자와 상의 해야함

// 액추에이터 태스크로 보낼 메세지
// motor_instruction 태스크 소유
typedef struct{
    uint16_t servo_angle;
    uint16_t right_dc_rpm;
    uint16_t left_dc_rpm;
}Motor_Instruction_MsgTypeDef;



#endif
