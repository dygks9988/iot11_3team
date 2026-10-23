
# 이동형 환경 데이터 수집 및 화재대응 로봇 제어 시스템



## 프로젝트 개요

라인 트레이싱 기반 이동형 로봇이 지정 경로를 자율 주행하며 환경 데이터를 수집하고, STM32와 Raspberry Pi 간 Modbus RTU 통신을 통해 센서 데이터, 모터 RPM, 로봇 동작 상태를 수집·모니터링하는 시스템이다.

수집된 데이터는 웹 대시보드에서 확인할 수 있으며 DB에 저장한다.화재 감지용 카메라 영상을 웹에 실시간 스트리밍한다. 영상 기반 화재 후보 감지와 CO2 센서 데이터를 함께 활용하여 화재 상황을 판단하고, 화재가 감지되면 로봇을 자동 정지시킨 뒤 웹을 통해 상태를 확인할 수 있도록 구성하였다.

**Raspberry Pi Master와 두 STM32 Slave로 역할을 분리하고, RS-485 기반 Modbus RTU 멀티드롭 통신을 통해 환경 데이터 수집·로봇 제어·웹 모니터링 기능을 통합하였다.**

## 대시보드

<img width="700" height="500" alt="trimmed_after10" src="https://github.com/user-attachments/assets/1fd2d9a4-d585-4687-ada0-4b9e62e98e2d" />


## 라인트레이싱 동작 영상

<img width="700" height="500" alt="line_tracking_demo_readme_" src="https://github.com/user-attachments/assets/ca46c798-3617-42ce-9b3b-e2e6d72c6281" />

## 영상 기반 화재 감지 및 웹 모니터링

<img width="700" height="500" alt="fire_detection_demo" src="https://github.com/user-attachments/assets/f5438dc1-c458-45c3-84f5-9f8a0cb37e8d" />
<img width="452" height="70" alt="image" src="https://github.com/user-attachments/assets/f4ebe7a3-12a1-418c-99ff-e7c676213868" />

**※ 화재 오탐지를 줄이기 위해 영상 기반 화재 후보 검출 결과와 CO2 센서 데이터를 함께 판단한다.**

## 시스템 로그


<img width="640" height="512" alt="robot_log" src="https://github.com/user-attachments/assets/a240559f-0fa1-4249-a400-4a629ca4b8c4" />

**Master Raspberry Pi의 시스템 로그이며, Opencv 화재 후보 감지, Master-Slave간 ModbusRTU 패킷 확인 및 DB 저장 로그를 확인 할 수 있다.**

### 핵심 기술 적용 경험 및 설계 의도

| 적용 장치 | 핵심 기술 | 설계 목적 및 적용 이유 |
|---|---|---|
| **Raspberry Pi Master / STM32 Slave** | RS-485 + Modbus RTU | 복수의 STM32 슬레이브를 한 RS-485 버스에 연결하는 멀티드롭 구조를 위해 채택. 주소,기능 코드,레지스터 기반 프로토콜로 확장성 확보. Modbus Poll로 레지스터 읽기 응답 검증 |
| **Master Raspberry Pi** | OpenCV + CO2 센서 데이터 | 영상 기반 화재 후보 검출 결과와 Environment STM32에서 수집한 CO2 센서 데이터를 함께 판단하여 오탐지를 줄이기 위해 적용 |
| **Actuator STM32** | Timer Trigger + ADC DMA | 타이머로 센서 수집 주기를 일정하게 유지하고, DMA를 활용하여 CPU의 데이터 수집 부담을 줄이기 위해 적용 |
| **Actuator STM32** | FreeRTOS Semaphore / Queue | Semaphore로 ISR과 센서 처리 태스크를 동기화하고, Queue를 통해 모터 제어 명령 전달 및 UART 수신 데이터를 파서 태스크로 전달하기 위해 적용 |
| **Actuator STM32** | IR 센서 + PD 제어 | 라인 위치 오차와 오차 변화량을 반영하여 좌우 모터 출력을 조정하기 위해 적용 |
| **Actuator STM32** | Timer Encoder + PWM | 엔코더로 모터별 실제 RPM을 측정하고, 구동 특성 차이를 고려한 PWM 출력 보정값을 적용하기 위해 활용 |
| **Actuator STM32** | UART DMA Circular + IDLE | CPU의 바이트 단위 수신 처리 부담을 줄이고, IDLE 이벤트를 기준으로 Modbus RTU 요청 데이터를 처리하기 위해 적용 |
| **Sensor STM32** | FreeRTOS Mutex / Queue | Mutex로 공유 센서 데이터(구조체)를 보호하고, Queue를 통해 Modbus 요청 메시지를 전달하기 위해 적용 |
| **Sensor STM32** | GPIO 타이밍 제어 + 임계구역 | 온습도 센서(DHT11)의 디지털 신호 읽기 시 taskENTER_CRITICAL()과 타이머를 활용해 펄스 폭을 정밀 측정하기 위해 적용 |
| **Sensor STM32** | UART 통신 + Thread Flag | CO2 센서(CM1106)와 UART 통신을 수행하고, 수신 완료 이벤트를 osThreadFlagsWait()로 효율적으로 처리하기 위해 적용 |
| **Sensor STM32** | ADC 샘플링 + 타이머 제어 | 미세먼지 센서 구동 시 타이머로 LED 제어 타이밍을 맞추고, ADC Injected 모드로 아날로그 값을 정밀 측정하기 위해 적용 |


## 시스템 아키텍처

  ### Integrated System
<img width="1024" height="576" alt="image" src="https://github.com/user-attachments/assets/f4def5d1-ea76-4a96-b0f6-3b00172e4ec3" />




### Actuator

<img width="1000" height="700" alt="image" src="https://github.com/user-attachments/assets/407d7064-0888-4759-b391-8e102d21b40d" />


### Sensor
<img width="1000" height="700" alt="freertos_architecture_v3" src="https://github.com/user-attachments/assets/14978be2-69f9-4045-9b45-92e1c69ac98f" />


| 측정 항목 | 측정 결과 | 의미 |
|---|---:|---|
| ADC DMA Complete ISR | 4.128 µs | ADC DMA 완료 인터럽트 처리 시간 |
| Motor Instruction Task | 2.76 µs | 센서값 판단 및 모터 명령 생성 시간 |
| Actuator Task | 3.00 µs | 명령 수신 후 모터 출력 반영 시간 |
| End-to-End Latency | 32.625 µs | ADC DMA 완료부터 Actuator 출력 반영까지의 전체 지연시간 |
| Control Period | 10.000637 ms | 제어 루프 실행 주기 |

ADC DMA Complete ISR → Motor Instruction Task → Actuator Task 경로의 End-to-End latency를 GPIO 토글과 Logic Analyzer로 측정한 결과

약 32.625 µs였으며, 10 ms 제어 주기 대비 충분한 실행 여유를 확인하였다.







## 역할 분담
### 이요한
- 전체 시스템 아키텍처 설계 및 시스템 통합
- STM32 구동부 펌웨어 및 FreeRTOS 기반 제어 구조 설계
- Modbus RTU 기반 Master–Slave 통신 구조 설계 및 구동부 프로토콜 처리 로직 구현
- 라인트레이싱 및 모터 제어 로직 구현
- Raspberry Pi Flask 프로그램 작성
  
### 홍원표
- STM32 센서부 펌웨어 및 FreeRTOS 기반 온/습도, CO2, 미세먼지 데이터 수집 로직 구현
- 센서부(Slave 1) 및 Raspberry pi(Master) Modbus RTU 프로토콜 처리 로직 구현
- Raspberry Pi MySQL 기반 DB 생성 및 수집된 센서 데이터 통합 관리 환경 구축
  
### 박지우






## 현재 구현 상태

### 웹

- 대시보드
- 환경 데이터 차트
- 영상 스트리밍
- 웹 요청 (로봇 상태 제어)

**추가 항목**
- MOTOR RPM 데이터 차트 (폐루프 제어에 유용)

---
### 라즈베리 파이
- OpenCV 불꽃 감지 및 대응(모터 정지)
- ModBUS Protocol Master 요청
---

**구동부**

- 5채널 IR 센서 기반 라인 트레이싱 로직 구현

- 목표 RPM 기반 개루프 모터 제어 구현

- 엔코더 기반 실제 RPM 측정 및 모터별 보정값 적용

- 주행/정지 모드 제어 로직 구현

- 로봇 차체 구동부 배선 완료

- Modbus protocol (Read coil,reg) (Write coil)등 제한된 기능 구현

- RS485 마스터, 슬레이브간 테스트 완료.


**센서부**

- 온습도 센서(DHT11) : GPIO 비트뱅 + 타이머 기반 마이크로초 단위 신호 측정으로 온습도 값 수집

- CO2 센서(CM1106) : UART DMA 기반 요청/응답 방식으로 CO2 농도(ppm) 측정

- 미세먼지 센서(GP2Y1010AU0F) : LED 펄스 타이밍에 맞춘 ADC 샘플링으로 미세먼지 농도 산출

- Modbus protocol (Read Holding Registers) 제한된 기능 구현

**개선 사항**
- 직접 조종 모드(웹에서 자율주행과 직접조종을 토글)
- 초음파 센서기반 Deadline을 맞춘 real time 정지 정책

## 디버깅

### 개발 보드 MCU 회로 손상
하드웨어 구성 중 L298N 모터 드라이버 입력핀에서 개발 보드 GPIO OUTPUT핀으로 전류가 흘러 개발 보드 회로가 타는 상황 발생

**해결방안**

-> 입력핀에서 전류가 흘러도 문제상황이 발생하지 않도록 저항으로 보호 회로를 구성

### 모터 드라이버 회로 구성중 로직 레벨오류
L298N H-Bridge 회로의 EN핀이 GPIO 3.3V로 HIGH 로직을 받지 못하는 상황이 발생

**해결방안**

-> 모터의 역회전 기능을 포기하고 하드웨어 회로를 변경, EN핀에 점퍼캡을 끼우고, IN1핀 PWM, IN2 GND 방식으로 회로를 구성

### Modbus 응답 에러율 약 20%
Modbus protocol Actuator Slave 응답의 에러율이 20%에 근사하는 상황 발생

IDLE EventCallback의 문제로 추측하고 Callback의 QueueSendFromISR직전의 수신버퍼 로그를 출력하였다

어느 특정 타이밍에서 수신 에러가 발생하고 있었고, 에러를 정확히 재현하기 위해 ST-LINK 디버거를 사용

특정 타이밍의 수신 버퍼가 깨져있는 상황을 확인하였다.

DMA Circular와 수신 버퍼의 크기 불일치로 수신 버퍼 외부의 메모리에 데이터가 넘어가는 상황이 발생하고 있었다.

**해결방안**

-> 수신버퍼와 DMA Circular의 길이를 정확히 맞춰 통신 요청 515회 중 오류 0회.
<img width="675" height="360" alt="image" src="https://github.com/user-attachments/assets/c6376608-6d79-4596-aea2-dafc299ef2fd" />


5채널 IR센서중 가장 왼쪽 센서의 기능이 현저하게 떨어져 라인트레이싱에 치명적인 문제를 야기하고 있다 - > 센서 교체를 통해 해결하였다.




## ModBUS Protocol

Serial : UART,RS-485


제한된 기능만 사용



### Function Code

- Read_Coils

- Read_Holding_Registers

- Write_Single_Coil





### Actuator Slave



**ActuatorRegisterMap**



- RF_MOTOR_RPM = 0x00


- LF_MOTOR_RPM = 0x01


- RR_MOTOR_RPM = 0x02


- LR_MOTOR_RPM = 0x03



**ActuatorCoilMap**



- ROBOT_STATE = 0x00

**프로젝트 일정상 Write Single coil의 응답을 후순위로 미루었다**















