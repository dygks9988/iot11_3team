/*
 * Mode_task.c
 *
 *  Created on: Sep 29, 2026
 *      Author: pc
 */
#include "c_stdlib.h"
#include "os_common.h"

#include "robot_mode.h"
#include "msg.h"

static Robot_ModeTypeDef robot_mode = ROBOT_IDLE;;

void mode_task(){

	Robot_CmdTypeDef robot_cmd = ROBOT_CMD_NONE;
	// 모터 정지용 큐
	Motor_Instruction_MsgTypeDef stop_msg = {0,0};
	for(;;){
		if(xQueueReceive(modeQueueHande,&robot_cmd, portMAX_DELAY) == pdTRUE){
			mode_change(&robot_mode,robot_cmd);
			// 모드 변경후 로봇 상태가 대기상태라면 모터정지
			if(robot_mode == ROBOT_IDLE)
			xQueueSend(insQueueHandle,&stop_msg,10);
		}
	}
}

