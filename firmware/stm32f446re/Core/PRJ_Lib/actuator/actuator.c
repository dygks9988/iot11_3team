

#include "tim.h"

#include "actuator.h"


// 현재 프로젝트 일정과 협업관리상 하드웨어 종속을 분리하기 어려움
// 서로 다른 모터를 각각 제어하기 위해 모터 핸들러에 MAX RPM 멤버를 추가
// 배선후 기능을 추가하기는 매우 어렵다고 판단, 미리 추후 추가할 기능의 배선을 먼저 하기위해 구조체 멤버를 대폭 추가

// @todo encoder pid, 직접 조종
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
}Motor_PinTypeDef;

typedef struct{
    uint16_t target_rpm;
    uint16_t current_rpm;
    uint16_t max_rpm;

    TIM_HandleTypeDef* pwm_htim;
    uint32_t pwm_channel;

    Motor_PinTypeDef cw;
    Motor_PinTypeDef ccw;

    TIM_HandleTypeDef* encoder_htim;
}Dc_Motor_HandleTypeDef;




//10Mhz 9분주 ARR 999
//10kHz 100us주기 카운터
//@todo 튜닝예정 현재는 무부하 기준

// 모터의 조향 방식이 서보 조향에서 4륜구동 차동제어방식으로 변경
// DC모터를 2개씩 각각 다른 모터를 사용할 수 밖에 없었음
#define DC_ARR 999

#define	JGB_MAX_RPM 330
#define JGA_MAX_RPM 280


static Dc_Motor_HandleTypeDef jgb_hdcmotor[2] = {
    {0,0,JGB_MAX_RPM,&htim1,TIM_CHANNEL_1,{CW1_GPIO_Port,CW1_Pin},{CCW1_GPIO_Port,CCW1_Pin},&htim2},
	{0,0,JGB_MAX_RPM,&htim1,TIM_CHANNEL_2,{CW2_GPIO_Port,CW2_Pin},{CCW2_GPIO_Port,CCW2_Pin},&htim3}
};

static Dc_Motor_HandleTypeDef jga_hdcmotor[2] = {
	{0,0,JGA_MAX_RPM,&htim1,TIM_CHANNEL_3,{CW3_GPIO_Port,CW3_Pin},{CCW3_GPIO_Port,CCW3_Pin},&htim4},
	{0,0,JGA_MAX_RPM,&htim1,TIM_CHANNEL_4,{CW4_GPIO_Port,CW4_Pin},{CCW4_GPIO_Port,CCW4_Pin},&htim5}
};

static Dc_Motor_HandleTypeDef* right_dcmotor_front = &jga_hdcmotor[0];
static Dc_Motor_HandleTypeDef* right_dcmotor_rear = &jgb_hdcmotor[0];

static Dc_Motor_HandleTypeDef* left_dcmotor_front = &jga_hdcmotor[1];
static Dc_Motor_HandleTypeDef* left_dcmotor_rear = &jgb_hdcmotor[1];




// 변환식
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

// 모터 방향 제어 추후 확장 기능을 위해 추가
static void set_motor_stop(Dc_Motor_HandleTypeDef* dc){
	HAL_GPIO_WritePin(dc->cw.port, dc->cw.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(dc->ccw.port, dc->ccw.pin, GPIO_PIN_RESET);
}

static void set_cw(Dc_Motor_HandleTypeDef* dc){
	HAL_GPIO_WritePin(dc->cw.port, dc->cw.pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(dc->ccw.port, dc->ccw.pin, GPIO_PIN_RESET);
}

static void set_ccw(Dc_Motor_HandleTypeDef* dc){
	HAL_GPIO_WritePin(dc->cw.port, dc->cw.pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(dc->ccw.port, dc->ccw.pin, GPIO_PIN_SET);
}



// 엔코더 관련
void encoder_init(){
	HAL_TIM_Encoder_Start(right_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(right_dcmotor_rear->encoder_htim, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_front->encoder_htim,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(left_dcmotor_rear->encoder_htim,TIM_CHANNEL_ALL);
}

// 정확한 주기로 측정해야함, 별도의 테스크가 필요할 것 같다
void encoding_motor(Dc_Motor_HandleTypeDef *dc){
	// 100ms 주기
	// JGB25 370 DC Motor Encoder CPR 1320기준
	// JGA25 370 정확한 CPR수치 필요

	uint32_t cnt = __HAL_TIM_GET_COUNTER(dc->encoder_htim);
	float rpm  = ((float)cnt * 600.0f) / 1320.0f;
	printf("rpm = %f\r\n",rpm);
	__HAL_TIM_SET_COUNTER(dc->encoder_htim, 0);
}


// 공개 ap
void actuator_init(){
	hw_motor_init(right_dcmotor_front->pwm_htim,right_dcmotor_front->pwm_channel);
	hw_motor_init(right_dcmotor_rear->pwm_htim,right_dcmotor_rear->pwm_channel);

	hw_motor_init(left_dcmotor_front->pwm_htim,left_dcmotor_front->pwm_channel);
	hw_motor_init(left_dcmotor_rear->pwm_htim,left_dcmotor_rear->pwm_channel);

	// 현재 라인트레이싱 기반구조 추후 직접 조종모드가 필요하다면 리팩토링 필요
	set_cw(right_dcmotor_front);
	set_cw(right_dcmotor_rear);
	set_ccw(left_dcmotor_front);
	set_ccw(left_dcmotor_rear);

	encoder_init();
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





