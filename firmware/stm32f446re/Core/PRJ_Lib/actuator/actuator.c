



#include "actuator.h"


// 현재 프로젝트 일정과 협업관리상 하드웨어 종속을 분리하기 어려움
// 서로 다른 모터를 각각 제어하기 위해 모터 핸들러에 MAX RPM 멤버를 추가

// 2026-09-24 모터 드라이버 L298 H-브릿지 구성 EN에 PWM 신호, IN1,IN2의 GPIO핀 구성이 동작을 하지 않음을 확인
// 이 이상 해결하려고 하면 프로젝트 진행에 차질이 있을 거라고 판단 프로젝트 범위를 축소하여 역회전 구동 기능을 제외한다.
// EN핀에 점퍼캡을 끼우고, IN1핀 PWM, IN2 GND 방식으로 회로를 구성할 예정이다.

// L298 EN핀이 3.3 전압으로 동작 하지 않는 것을 확인, PWM 신호의 승압 회로를 구성하기는 어렵다고 판단, 계획을 수정하지 않는다.



// @todo encoder pid, 직접 조종



// 모터의 조향 방식이 서보 조향에서 4륜구동 차동제어방식으로 변경
// DC모터를 2개씩 각각 다른 모터를 사용할 수 밖에 없었음

//10Mhz 9분주 ARR 999
//10kHz 100us주기 카운터
#define DC_ARR 999


//JGB
#define	JGB_MAX_RPM 330
//encoder
#define JGB_ENCODER_PPR 11
#define JGB_GEAR_RATIO 30
//CPR 1320
#define JGB_CPR (JGB_ENCODER_PPR * JGB_GEAR_RATIO * 4)


//JGA
#define JGA_MAX_RPM 280
//encoder
#define JGA_ENCODER_PPR 11
#define JGA_GEAR_RATIO 21.3f
//CPR 937.2
#define JGA_CPR (JGA_ENCODER_PPR * JGA_GEAR_RATIO * 4)
// 모터를 1회전 했을 때 21.3 X 11PPR X 4 FACTOR의 근사치인 933측정

// encoder 주기
#define ENCODER_DT 0.05f

static Dc_Motor_HandleTypeDef jgb_hdcmotor[2] = {
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGB_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_1,
        .encoder_htim = &htim2,
		.encoder_cpr = JGB_CPR
    },
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGB_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_2,
        .encoder_htim = &htim3,
		.encoder_cpr = JGB_CPR
    }
};

static Dc_Motor_HandleTypeDef jga_hdcmotor[2] = {
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGA_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_3,
        .encoder_htim = &htim4,
		.encoder_cpr = JGA_CPR
    },
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGA_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_4,
        .encoder_htim = &htim5,
		.encoder_cpr = JGA_CPR
    }
};


Dc_Motor_HandleTypeDef* right_dcmotor_front = &jga_hdcmotor[0];
Dc_Motor_HandleTypeDef* left_dcmotor_front = &jga_hdcmotor[1];

Dc_Motor_HandleTypeDef* right_dcmotor_rear = &jgb_hdcmotor[0];
Dc_Motor_HandleTypeDef* left_dcmotor_rear = &jgb_hdcmotor[1];


// 모터 RPM TO CCR 변환식
// DC모터를 각각 다른 모터를 사용하며 함수 인자 형태를 max_rpm을 넣는 형태로 변경
static inline uint32_t rpm_to_ccr(uint16_t rpm, uint16_t max_rpm) {
	if(rpm > max_rpm)rpm = max_rpm;
    return (uint32_t)(rpm) * DC_ARR / max_rpm;
}


// HAL 종속성
static void hw_motor_init(TIM_HandleTypeDef* htim,uint32_t tim_ch){
	HAL_TIM_PWM_Start(htim,tim_ch);
	__HAL_TIM_SET_COMPARE(htim,tim_ch,0);
}
static void hw_motor_pwm(TIM_HandleTypeDef* htim,uint32_t tim_ch,uint32_t ccr){
	__HAL_TIM_SET_COMPARE(htim,tim_ch,ccr);
}

static void set_dc_rpm(Dc_Motor_HandleTypeDef* dc){
	uint32_t ccr = rpm_to_ccr(dc->target_rpm, dc->max_rpm);
	hw_motor_pwm(dc->pwm_htim,dc->pwm_channel,ccr);
}




// 엔코더 관련
void encoder_init(){
	HAL_TIM_Encoder_Start(right_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(right_dcmotor_rear->encoder_htim, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_rear->encoder_htim,TIM_CHANNEL_ALL);
}

// 정확히 10ms 주기로 측정해야함, 별도의 테스크가 필요할 것 같다
void update_motor_rpm(Dc_Motor_HandleTypeDef *dc){
	// JGB25 370 CPR 1320
	// JGA25 370 CPR 937


	int16_t cnt = __HAL_TIM_GET_COUNTER(dc->encoder_htim);
	if(cnt < 0)cnt = -cnt;
	//printf 부하를 줄이기 위해 연산 뒤 int16_t로 다시 형변환 한다.
	dc->current_rpm = (int16_t)(((float)cnt * 60.0f / ENCODER_DT) / dc->encoder_cpr);
	__HAL_TIM_SET_COUNTER(dc->encoder_htim, 0);
}

void dis_rpm(){
	printf("rr rpm = %d\r\n",right_dcmotor_rear->current_rpm);
	printf("lr rpm = %d\r\n",left_dcmotor_rear->current_rpm);

	printf("rf rpm = %d\r\n",right_dcmotor_front->current_rpm);
	printf("lf rpm = %d\r\n",left_dcmotor_front->current_rpm);
}


// 공개 ap
void actuator_init(){
	hw_motor_init(right_dcmotor_front->pwm_htim,right_dcmotor_front->pwm_channel);
	hw_motor_init(right_dcmotor_rear->pwm_htim,right_dcmotor_rear->pwm_channel);

	hw_motor_init(left_dcmotor_front->pwm_htim,left_dcmotor_front->pwm_channel);
	hw_motor_init(left_dcmotor_rear->pwm_htim,left_dcmotor_rear->pwm_channel);


}

void actuator_process(Motor_Instruction_MsgTypeDef* instruction_msg){
	//Receive Msg

	right_dcmotor_front->target_rpm = instruction_msg->right_dc_rpm;
	right_dcmotor_rear->target_rpm = instruction_msg->right_dc_rpm;

	left_dcmotor_front->target_rpm = instruction_msg->left_dc_rpm;
    left_dcmotor_rear->target_rpm = instruction_msg->left_dc_rpm;

    // DcMotor PWM set
    set_dc_rpm(right_dcmotor_front);
    set_dc_rpm(right_dcmotor_rear);

    set_dc_rpm(left_dcmotor_front);
    set_dc_rpm(left_dcmotor_rear);
}





