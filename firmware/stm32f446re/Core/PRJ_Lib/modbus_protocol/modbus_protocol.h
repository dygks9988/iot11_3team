/*
 * modbus_protocol.h
 *
 *  Created on: Sep 28, 2026
 *      Author: pc
 */

#ifndef PRJ_LIB_MODBUS_PROTOCOL_MODBUS_PROTOCOL_H_
#define PRJ_LIB_MODBUS_PROTOCOL_MODBUS_PROTOCOL_H_

#include "c_stdlib.h"

// UART 8E1 or 8N2를 사용해야함
// Actuator Slave addr 0x01

// Modbus Request Frame RTU
typedef struct{
	uint8_t slave_addr;
	uint8_t function_code;
	uint16_t start_address;
	uint16_t quantity;
	uint16_t crc;
}ModbusRTU_FrameTypeDef;

// Modbus Response Frame
typedef struct{
	uint8_t slave_addr;
	uint8_t function_code;
	uint8_t byte_Count;
	uint8_t coil_data[5];
	uint16_t crc;
}ModbusResponse_FrameTypeDef;

// Function Code Table
// 해당 프로젝트에선 Read Holding Registers와 Write_Single_Coil로 제한된 기능한 사용 예정.
typedef enum{
	Read_Coils = 0x01,
	Read_Discrete_Inputs = 0x02,
	Read_Holding_Registers = 0x03,
	Read_Input_Registers = 0x04,
	Write_Single_Coil = 0x05,
	Write_Single_Register = 0x06,
	Write_Multiple_Coils = 0x0f,
	Write_Multiple_Registers = 0x10
}ModbusFunctionCode_TableTypeDef;

// LF = Left Front
// RF = Right Front
// LR = Left Rear
// RR = Right Rear
typedef enum{
	LF_MOTOR_RPM = 0x00,
	RF_MOTOR_RPM = 0x01,
	LR_MOTOR_RPM = 0x02,
	RR_MOTOR_RPM = 0x03,
}ActuatorRegister_MapTypeDef;

typedef enum{
	ROBOT_STATE = 0x00
}ActuatorCoil_MapTypeDef;


void modbus_process(uint8_t *rx_buf);


#endif /* PRJ_LIB_MODBUS_PROTOCOL_MODBUS_PROTOCOL_H_ */
