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

#define MODBUS_UART_CH 6

typedef enum{
	ENVIROMENT_SALVE_ADDR = 0x01,
	ACTUATOR_SLAVE_ADDR = 0x02
}ModebusSlave_AddrTypeDef;


typedef struct{
	uint8_t slave_addr;
	uint8_t function_code;
	uint16_t start_address;
	uint16_t values ;
	uint16_t crc;
}ModbusRTU_FrameTypeDef;

typedef struct
{
    uint8_t data[20];
    uint8_t len;
} ModbusRxMsgTypeDef;

// Modbus Response Frame
typedef struct{
	uint8_t slave_addr;
	uint8_t function_code;
	uint8_t byte_Count;
	uint8_t data[20];
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

typedef enum{
	ROBOT_STATE_COIL_ADR = 0x00
}ActuatorCoil_MapTypeDef;

// LF = Left Front
// RF = Right Front
// LR = Left Rear
// RR = Right Rear
typedef enum{
	RF_MOTOR_RPM_REG_ADR = 0x00,
	LF_MOTOR_RPM_REG_ADR = 0x01,
	RR_MOTOR_RPM_REG_ADR = 0x02,
	LR_MOTOR_RPM_REG_ADR = 0x03,
}ActuatorRegister_MapTypeDef;

// 파서 프로세스에서 파싱한 뒤 해당 타겟에 비트 플래그를 띄운다.
// 파서 태스크에서 해당 타겟과 명령을 수행한다
// 예시 parser->flags |= MODBUS_FLAG_ROBOT_STATE
#define MODBUS_REG_FLAG_RF_RPM  	(1 << RF_MOTOR_RPM_REG_ADR)
#define MODBUS_REG_FLAG_LF_RPM 		(1 << LF_MOTOR_RPM_REG_ADR)
#define MODBUS_REG_FLAG_RR_RPM  	(1 << RR_MOTOR_RPM_REG_ADR)
#define MODBUS_REG_FLAG_LR_RPM  	(1 << LR_MOTOR_RPM_REG_ADR)

#define MODBUS_COIL_FLAG_ROBOT_STATE (1 << ROBOT_STATE_COIL_ADR)


typedef struct{
	ModbusFunctionCode_TableTypeDef cmd;
	uint8_t flags;

	uint8_t robot_state_value;
}Modbus_ParserTypeDef;

// 파서 프로세스
bool modbus_parsing(uint8_t *rx_buf,uint8_t rx_len,Modbus_ParserTypeDef* modbus_parser);
uint8_t modbus_tx_package(ModbusResponse_FrameTypeDef* resp_frame,uint8_t* tx_buf);


#endif /* PRJ_LIB_MODBUS_PROTOCOL_MODBUS_PROTOCOL_H_ */
