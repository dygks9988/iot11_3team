
### 이동형 환경 데이터 수집 로봇 제어 시스템



## 프로젝트 개요

라인 트레이싱 기반 이동형 로봇이 지정 경로를 주행하며 환경 센서 데이터를 수집하고,



MCU와 Linux 간 통신을 통해 데이터를 저장·전송하며 로봇 및 센서 상태를 실시간 모니터링하는 시스템



## 라인트레이싱 동작 영상





## 시스템 구상도

Master Rasp



Protocol UART



Slave Actuator STM32



Slave Enviroment STM32





## 현재 구현 상태

**웹**

- 웹 대쉬보드 틀



**구동부**

- 5채널 IR 센서 기반 라인 트레이싱 로직 구현

- 목표 RPM 기반 개루프 모터 제어 구현

- 엔코더 기반 실제 RPM 측정 및 모터별 보정값 적용

- 주행/정지 모드 제어 로직 구현

- 로봇 차체 구동부 배선 완료

- Modbus protocol (Read coil,reg) (Write coil)등 제한된 기능 구현 



## 주요 데이터 흐름

**SW**





<img width="549" height="412" alt="image" src="https://github.com/user-attachments/assets/fb23e3ad-ec43-4149-9472-9f8d68355d29" />



<img width="549" height="412" alt="image" src="https://github.com/user-attachments/assets/1ff7f645-6c25-4663-b0d9-bb12617ef6de" />







초기 버전 SW구성도,팀원의 이해를 돕기 위해 슈퍼루프로 비선점형 태스크를 모방 하였다.



추후 선점형 태스크로 우선순위를 부여할 예정이다.



이동형 시스템이 동작하기 위한 최소 기준이다.



추후 일정이 된다면 모터 PID 루프 구조로 주행 안정성을 높이고, 초음파 기반 안전 정지 구현을 고려중이다.







## UART 계획

DMA Circular

IDLE event callback



## ModBUS Protocol



제한된 기능만 사용



**Function Code**

- Read_Coils

- Read_Holding_Registers

- Write_Single_Coil





### Atuator Slave



**ActuatorReisterMap**



- LF_MOTOR_RPM = 0x00


- RF_MOTOR_RPM = 0x01


- LR_MOTOR_RPM = 0x02


- RR_MOTOR_RPM = 0x03



**ActuatorCoilMap**



- ROBOT_STATE = 0x00



## 디버깅

하드웨어 구성 중 L298N 모터 드라이버 입력핀에서 개발 보드 GPIO OUTPUT핀으로 전류가 흘러 개발 보드 회로가 타는 상황 발생



**해결방안**



-> 입력핀에서 전류가 흘러도 문제상황이 발생하지 않도록 저항으로 보호 회로를 구성



L298N H-Bridge 회로의 EN핀이 GPIO 3.3V로 HIGH 로직을 받지 못하는 상황이 발생



**해결방안**



-> 모터의 역회전 기능을 포기하고 하드웨어 회로를 변경, EN핀에 점퍼캡을 끼우고, IN1핀 PWM, IN2 GND 방식으로 회로를 구성



## 발견된 문제점



5채널 IR센서중 가장 왼쪽 센서의 기능이 현저하게 떨어져 라인트레이싱에 치명적인 문제를 야기하고 있다.

Modbus protocol slave 응답의 에러율이 20%에 근사 된다. UARTEventCallback의 idle flag 문제로 추측하고 있으며 라즈베리파이와 연동 되는 것을 확인 한 뒤 디버깅 할 예정이다.


