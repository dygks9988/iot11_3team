/*
 * modbus_protocol_task.c
 *
 *  Created on: Sep 30, 2026
 *      Author: pc
 */

#include "modbus_protocol.h"
#include "robot_mode.h"

#include "My_ARM_RTOS_UART_Lib_V5_2.h"

#include "os_common.h"



#define MODBUS_RX_BUF_SIZE 20

#define MODBUS_DATA_MAX_SIZE 20
#define MODBUS_TX_BUF_SIZE   (MODBUS_DATA_MAX_SIZE + 5)

void modbus_protocol_task(){
	ModbusResponse_FrameTypeDef modb_resp_frame;
	Modbus_ParserTypeDef modb_parser;
	ModbusRxMsgTypeDef msg;


	for(;;){
		xQueueReceive(modbQueueHandle,&msg, portMAX_DELAY);



		memset(&modb_resp_frame, 0, sizeof(modb_resp_frame));
		memset(&modb_parser, 0, sizeof(modb_parser));

		if(modbus_parsing(msg.data,msg.len,&modb_parser) == true){

			modb_resp_frame.slave_addr = ACTUATOR_SLAVE_ADDR;

			uint8_t tx_buf[MODBUS_TX_BUF_SIZE] = {0};
			uint8_t tx_len = 0;

			// 배열 인덱스 변수
			uint8_t data_len = 0;

			uint16_t rpm = 0;
		;

			switch(modb_parser.cmd){
				case Read_Coils:
					if(modb_parser.flags & MODBUS_COIL_FLAG_ROBOT_STATE){
						modb_resp_frame.data[data_len++] = robot_state_read();
					}

					modb_resp_frame.function_code = Read_Coils;
					modb_resp_frame.byte_Count = data_len;


					tx_len = modbus_tx_package(&modb_resp_frame,tx_buf);

					break;
				case Read_Holding_Registers:
					if(modb_parser.flags & MODBUS_REG_FLAG_RF_RPM){
						rpm = motor_rpm_read(RF_MOTOR_RPM_REG_ADR);

						modb_resp_frame.data[data_len++] = (rpm >> 8) & 0xFF;
						modb_resp_frame.data[data_len++] = rpm & 0xFF;
					}
					if(modb_parser.flags & MODBUS_REG_FLAG_LF_RPM){
						rpm = motor_rpm_read(LF_MOTOR_RPM_REG_ADR);

						modb_resp_frame.data[data_len++] = (rpm >> 8) & 0xFF;
						modb_resp_frame.data[data_len++] = rpm & 0xFF;

					}
					if(modb_parser.flags & MODBUS_REG_FLAG_RR_RPM){
						rpm = motor_rpm_read(RR_MOTOR_RPM_REG_ADR);

						modb_resp_frame.data[data_len++] = (rpm >> 8) & 0xFF;
						modb_resp_frame.data[data_len++] = rpm & 0xFF;

					}
					if(modb_parser.flags & MODBUS_REG_FLAG_LR_RPM){
						rpm = motor_rpm_read(LR_MOTOR_RPM_REG_ADR);

						modb_resp_frame.data[data_len++] = (rpm >> 8) & 0xFF;
						modb_resp_frame.data[data_len++] = rpm & 0xFF;

					}

					modb_resp_frame.function_code = Read_Holding_Registers;
					modb_resp_frame.byte_Count = data_len;

					tx_len = modbus_tx_package(&modb_resp_frame,tx_buf);
					break;
				case Write_Single_Coil:
					if(modb_parser.flags & MODBUS_COIL_FLAG_ROBOT_STATE){
						Robot_CmdTypeDef cmd;
						if(modb_parser.robot_state_value == 0){
						    cmd = ROBOT_CMD_STOP;
						}
						else{
							cmd = ROBOT_CMD_START;
						}
						xQueueSend(modeQueueHandle,&cmd,10);
					}
				default:
					break;
			}

			// 송신 할 데이터가 있다면
			if(tx_len > 0)tx(tx_buf, MODBUS_UART_CH, tx_len);
		}
	}
}
