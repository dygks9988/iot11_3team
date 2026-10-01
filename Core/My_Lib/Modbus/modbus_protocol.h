/*
 * modbus_protocol.h
 *
 *  Created on: Sep 29, 2026
 *      Author: PC
 */

#ifndef MY_LIB_MODBUS_MODBUS_PROTOCOL_H_
#define MY_LIB_MODBUS_MODBUS_PROTOCOL_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// UART 8E1 or 8N2를 사용해야함
// Sensor Slave addr 0x01

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
    uint8_t byte_count;
    uint16_t data[10];
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

// temp = 온도 (℃)
// humid = 습도 (%)
// co2 = 이산화탄소 농도 (ppm)
// dust = 미세먼지 농도 (ug/m^3)
// zone = 현재구역
typedef enum{
    REG_Temp_Addr = 0x00,
	REG_Humid_Addr = 0x01,
	REG_Co2_Addr = 0x02,
	REG_Dust_Addr = 0x03,
	REG_Zone_Addr = 0x04
}SensorRegister_MapTypeDef;


#define REG_FLAG_TEMP	(1 << REG_Temp_Addr)	// (1 << 0)
#define REG_FLAG_HUMID	(1 << REG_Humid_Addr)	// (1 << 1)
#define REG_FLAG_CO2	(1 << REG_Co2_Addr)		// (1 << 2)
#define REG_FLAG_DUST	(1 << REG_Dust_Addr)	// (1 << 3)
#define REG_FLAG_ZONE	(1 << REG_Zone_Addr)	// (1 << 4)

typedef struct{
    ModbusFunctionCode_TableTypeDef cmd;

    uint8_t flags;
}Modbus_ParserTypedef;

static uint16_t modbus_crc16(const unsigned char *buf, unsigned int len);

bool modbus_parsing(uint8_t *rx_buf, Modbus_ParserTypedef* modbus_parser, ModbusRTU_FrameTypeDef* rtu_frame);
void modbus_packing(uint8_t *tx_buf, ModbusResponse_FrameTypeDef* modbus_packer, ModbusRTU_FrameTypeDef* rtu_frame);

#endif /* MY_LIB_MODBUS_MODBUS_PROTOCOL_H_ */
