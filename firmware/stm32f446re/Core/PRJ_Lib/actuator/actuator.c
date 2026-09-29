



#include "actuator.h"


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

// 모터의 조향 방식이 서보 조향에서 4륜구동 차동제어방식으로 변경
// DC모터를 2개씩 각각 다른 모터를 사용할 수 밖에 없었음

// 2026-09-28 로봇의 직선 방향이 기계적으로 오른쪽으로 치우쳐지는 문제가 발생하였다
// 해결하기위해 오른쪽 바퀴의 보정을 올려 평균 RPM이 왼쪽보다 5정도 높게 설정하였다(임시방편)
// 이 문제는 개루프 제어에선 문제 될 게 없지만 폐루프 제어를 하기 위해선 좀 더 좋은 해결 방향이 필요

// 저속 구간에서 바퀴가 회전하지 않는 문제를 해결하기 위해 구조체 멤버에 MIN CCR 추가
static Dc_Motor_HandleTypeDef jgb_hdcmotor[2] = {
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGB_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_1,
        .encoder_htim = &htim2,
		.encoder_cpr = JGB_CPR,
		.ccr_gain = 1.65,
		.min_ccr = 100
    },
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGB_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_2,
        .encoder_htim = &htim3,
		.encoder_cpr = JGB_CPR,
		.ccr_gain = 1.45,
		.min_ccr = 100
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
		.encoder_cpr = JGA_CPR,
		.ccr_gain = 1.45,
		.min_ccr = 100
    },
    {
        .target_rpm = 0,
        .current_rpm = 0,
        .max_rpm = JGA_MAX_RPM,
        .pwm_htim = &htim1,
        .pwm_channel = TIM_CHANNEL_4,
        .encoder_htim = &htim5,
		.encoder_cpr = JGA_CPR,
		.ccr_gain = 1.35,
		.min_ccr = 100
    }
};


Dc_Motor_HandleTypeDef* right_dcmotor_front = &jga_hdcmotor[0];
Dc_Motor_HandleTypeDef* left_dcmotor_front = &jga_hdcmotor[1];

Dc_Motor_HandleTypeDef* right_dcmotor_rear = &jgb_hdcmotor[0];
Dc_Motor_HandleTypeDef* left_dcmotor_rear = &jgb_hdcmotor[1];


// 모터 RPM TO CCR 변환식
// DC모터를 각각 다른 모터를 사용하며 함수 인자 형태를 max_rpm을 넣는 형태로 변경
// 2026-09-25 각 모터 축의 속도를 일정히 맞추기 위해 ccr_gain을 곱하는 연산을 추가

// 모터의 저속 구간에 바퀴가 돌지 않는 문제를 해결하기 위해 min ccr 연산 추가
static inline uint32_t rpm_to_ccr(Dc_Motor_HandleTypeDef* dc) {
	if(dc->target_rpm > dc->max_rpm)dc->target_rpm = dc->max_rpm;
	uint32_t ccr;
	ccr = (uint32_t)(dc->target_rpm) * DC_ARR / dc->max_rpm * dc->ccr_gain;

	if (ccr > DC_ARR)ccr = DC_ARR;

	if (ccr < dc->min_ccr)ccr = dc->min_ccr;

    return ccr;
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
	uint32_t ccr = rpm_to_ccr(dc);
	hw_motor_pwm(dc->pwm_htim,dc->pwm_channel,ccr);
}


// 엔코더 관련
void encoder_init(){
	HAL_TIM_Encoder_Start(right_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(right_dcmotor_rear->encoder_htim, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_rear->encoder_htim,TIM_CHANNEL_ALL);
}

// 정확한 주기로 측정해야함, 별도의 테스크가 필요할 것 같다
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
	printf("rr rpm = %d,target rpm = %d\r\n",right_dcmotor_rear->current_rpm,right_dcmotor_rear->target_rpm);
	printf("lr rpm = %d,target rpm = %d\r\n",left_dcmotor_rear->current_rpm,left_dcmotor_rear->target_rpm);

	printf("rf rpm = %d,target rpm = %d\r\n",right_dcmotor_front->current_rpm,right_dcmotor_front->target_rpm);
	printf("lf rpm = %d,target rpm = %d\r\n",left_dcmotor_front->current_rpm,left_dcmotor_front->target_rpm);
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





