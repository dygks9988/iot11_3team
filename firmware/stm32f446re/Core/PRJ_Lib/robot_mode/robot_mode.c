/*
 * robot_mode.c
 *
 *  Created on: Sep 29, 2026
 *      Author: pc
 */

#include "robot_mode.h"
#include "os_common.h"
#include "gpio.h"
#include "tim.h"

void mode_change(Robot_ModeTypeDef* mode,Robot_CmdTypeDef cmd){
	switch(cmd){
	case ROBOT_CMD_NONE:
		break;
	case ROBOT_CMD_TOGGLE:
		if(*mode != ROBOT_DRIVING){
			*mode = ROBOT_DRIVING;
			// ADC 트리거용 타이머
			// 명령 생성용 ADC 세마포어로 동기화 하였기 때문에 타이머가 시작되면 시스템 파이프라인이 시작된다
			HAL_TIM_Base_Start(&htim8);
			break;
		}
		if(*mode != ROBOT_IDLE){
			*mode = ROBOT_IDLE;
			HAL_TIM_Base_Stop(&htim8);
			break;
		}

	case ROBOT_CMD_START:
		if(*mode != ROBOT_DRIVING){
			*mode = ROBOT_DRIVING;
			HAL_TIM_Base_Start(&htim8);
		}
		break;
	case ROBOT_CMD_STOP:
		if(*mode != ROBOT_IDLE){
			*mode = ROBOT_IDLE;
			HAL_TIM_Base_Stop(&htim8);
		}
		break;
	default:
		break;
	}
	return;
}



