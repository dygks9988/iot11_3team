/*
 * robot_mode.h
 *
 *  Created on: Sep 29, 2026
 *      Author: pc
 */

#ifndef PRJ_LIB_ROBOT_MODE_ROBOT_MODE_H_
#define PRJ_LIB_ROBOT_MODE_ROBOT_MODE_H_


#include "c_stdlib.h"

typedef enum{
	ROBOT_IDLE,
	ROBOT_DRIVING
}Robot_ModeTypeDef;

typedef enum{
	ROBOT_CMD_NONE = 0,
	ROBOT_CMD_TOGGLE,
	ROBOT_CMD_START,
	ROBOT_CMD_STOP
}Robot_CmdTypeDef;


void mode_change(Robot_ModeTypeDef* mode,Robot_CmdTypeDef cmd);




#endif /* PRJ_LIB_ROBOT_MODE_ROBOT_MODE_H_ */
