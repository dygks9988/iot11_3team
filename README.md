
### 이동형 환경 데이터 수집 로봇 제어 시스템

## 프로젝트 개요
라인 트레이싱 기반 이동형 로봇이 지정 경로를 주행하며 환경 센서 데이터를 수집하고,

MCU와 Linux 간 통신을 통해 데이터를 저장·전송하며 로봇 및 센서 상태를 실시간 모니터링하는 시스템

## 현재 구현 상태
- 웹 대쉬보드 틀
- 라인트레이싱 로직
- 모터 제어 로직

라인 트레이싱로직, 모터 제어 로직 동작 확인
로봇 몸체에서 동작하는 것은 확인이 필요하다


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

## ModeBUS Protocol
설계 필요

