
### 이동형 환경 데이터 수집 및 로봇 제어 시스템

## 프로젝트 개요
라인 트레이싱 기반 이동형 로봇이 정해진 경로를 주행하며 환경 데이터를 수집·저장하고, 센서 및 로봇의 동작 상태를 실시간으로 모니터링하는 시스템

## 주요 데이터 흐름
**SW**


<img width="549" height="412" alt="image" src="https://github.com/user-attachments/assets/fb23e3ad-ec43-4149-9472-9f8d68355d29" />


<img width="549" height="412" alt="image" src="https://github.com/user-attachments/assets/66cd5faa-abab-4da2-863f-c617064a5863" />

초기 버전 SW구성도,팀원의 이해를 돕기 위해 슈퍼루프로 비선점형 태스크를 모방 하였다

추후 선점형 태스크로 우선순위를 부여할 예정

이동형 시스템이 동작하기 위한 최소 기준

추후 모터 PID 루프 구조로 라인트레이싱 안정성과 초음파 센서를 추가해 장애물 감지 기반의 안전 정지 기능을 구현을 고려중
