#include "tim.h"

#include "actuator.h"

// 현재 프로젝트 일정과 협업관리상 하드웨어 종속을 분리하기 어려움
typedef struct{
    uint16_t target_rpm;
    uint16_t current_rpm;

    TIM_HandleTypeDef *htim;
    uint32_t pwm_channel;
}Dc_Motor_HandleTypeDef;

typedef struct{
    uint16_t target_angle;

    TIM_HandleTypeDef *htim;
    uint32_t pwm_channel;
}Servo_Motor_HandleTypeDef;

static Dc_Motor_HandleTypeDef hdcmotor[2] = {
    {0,0,&htim1,TIM_CHANNEL_1},
    {0,0,&htim1,TIM_CHANNEL_2}
};

static Dc_Motor_HandleTypeDef* right_dcmotor = &hdcmotor[0];
static Dc_Motor_HandleTypeDef* left_dcmotor = &hdcmotor[1];

static Servo_Motor_HandleTypeDef hservo1 = {0, &htim2,TIM_CHANNEL_1};
static Servo_Motor_HandleTypeDef* servo = &hservo1;


//10Mhz 9분주 ARR 999
//10kHz 100us주기 카운터
//@todo 아직 모터가 오지 않았음 도착하고 튜닝예정 현재는 무부하 기준

#define DC_ARR 999
#define	MAX_RPM 330

// 변환식
static inline uint32_t rpm_to_ccr(uint16_t rpm) {
	if(rpm > MAX_RPM)rpm = MAX_RPM;
    return (uint32_t)(rpm) * DC_ARR / MAX_RPM;
}

//servo

//Position "0" (1.5 ms pulse) is middle, "90" (~2ms pulse) is all the way to the left.
//ms pulse) is middle, "90" (~2 ms pulse) is all the way to the right, "the left.

// 50Hz ARR 19999
#define MAX_CCR 2000
#define MIN_CCR 1000
#define MAX_ANGLE 180

static inline uint32_t angle_to_ccr(uint16_t angle){
	if(angle > MAX_ANGLE)angle = MAX_ANGLE;
	return ((((MAX_CCR - MIN_CCR) / MAX_ANGLE) *(uint32_t)angle)  + MIN_CCR);
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
	uint32_t ccr = rpm_to_ccr(dc->target_rpm);
	hw_motor_pwm(dc->htim,dc->pwm_channel,ccr);
}

static void set_servo_angle(Servo_Motor_HandleTypeDef* servo){
	uint32_t ccr = angle_to_ccr(servo->target_angle);
	hw_motor_pwm(servo->htim,servo->pwm_channel,ccr);
}



// 공개 ap
void actuator_init(){
	hw_motor_init(right_dcmotor->htim,right_dcmotor->pwm_channel);
	hw_motor_init(left_dcmotor->htim,left_dcmotor->pwm_channel);
	hw_motor_init(servo->htim,servo->pwm_channel);

	servo ->target_angle = BASE_SERVO_ANGLE;
	set_servo_angle(servo);
}


void actuator_process(Motor_Instruction_MsgTypeDef* instruction_msg){
	//Receive Msg
    right_dcmotor->target_rpm = instruction_msg->right_dc_rpm;
    left_dcmotor->target_rpm = instruction_msg->left_dc_rpm;

    servo ->target_angle = instruction_msg->servo_angle;

    // DcMotor PWM set
    set_dc_rpm(right_dcmotor);
    set_dc_rpm(left_dcmotor);

    // ServoMotor PWM set
    set_servo_angle(servo);
}





