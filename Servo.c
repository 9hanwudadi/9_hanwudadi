#include "Servo.h"

// 预分频
#define PRESCALER   10

// 输出频率 50Hz (周期20ms)
#define FREQ        50

// 周期计数值 (24 000 000 / 10 / 50 = 48000)
#define PERIOD (MAIN_Fosc / PRESCALER / FREQ)

// 定义8个舵机的占空比结构体
PWMx_Duty dutyB = {0};

// 初始化8个引脚
static void GPIO_config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Mode = GPIO_OUT_PP; // 推挽输出

    // 配置P2.0, P2.2, P2.4, P2.6
    GPIO_InitStructure.Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_Inilize(GPIO_P2, &GPIO_InitStructure);

    // 配置P0.0, P0.1, P0.2, P0.3
    GPIO_InitStructure.Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Inilize(GPIO_P0, &GPIO_InitStructure);
}

// 配置PWM
static void PWM_config(void)
{
    PWMx_InitDefine PWMx_InitStructure;

    // 1. 配置PWMA (PWM1~PWM4)
    PWMx_InitStructure.PWM_Mode      = CCMRn_PWM_MODE1;
    PWMx_InitStructure.PWM_Duty      = 0;
    PWMx_InitStructure.PWM_EnoSelect = ENO1P;
    PWM_Configuration(PWM1, &PWMx_InitStructure);
    
    PWMx_InitStructure.PWM_EnoSelect = ENO2P;
    PWM_Configuration(PWM2, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_EnoSelect = ENO3P;
    PWM_Configuration(PWM3, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_EnoSelect = ENO4P;
    PWM_Configuration(PWM4, &PWMx_InitStructure);

    // PWMA通用寄存器配置
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMA, &PWMx_InitStructure);
    PWMA_Prescaler(PRESCALER - 1);

    // 2. 配置PWMB (PWM5~PWM8)
    PWMx_InitStructure.PWM_EnoSelect = ENO5P;
    PWM_Configuration(PWM5, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_EnoSelect = ENO6P;
    PWM_Configuration(PWM6, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_EnoSelect = ENO7P;
    PWM_Configuration(PWM7, &PWMx_InitStructure);

    PWMx_InitStructure.PWM_EnoSelect = ENO8P;
    PWM_Configuration(PWM8, &PWMx_InitStructure);

    // PWMB通用寄存器配置
    PWMx_InitStructure.PWM_Period        = PERIOD - 1;
    PWMx_InitStructure.PWM_DeadTime      = 0;
    PWMx_InitStructure.PWM_MainOutEnable = ENABLE;
    PWMx_InitStructure.PWM_CEN_Enable    = ENABLE;
    PWM_Configuration(PWMB, &PWMx_InitStructure);
    PWMB_Prescaler(PRESCALER - 1);

    // 3. 引脚映射 (与GPIO初始化对应)
    PWM1_USE_P20P21(); // PWM1 -> P2.0
    PWM2_USE_P22P23(); // PWM2 -> P2.2
    PWM3_USE_P24P25(); // PWM3 -> P2.4
    PWM4_USE_P26P27(); // PWM4 -> P2.6
    PWM5_USE_P00();    // PWM5 -> P0.0
    PWM6_USE_P01();    // PWM6 -> P0.1
    PWM7_USE_P02();    // PWM7 -> P0.2
    PWM8_USE_P03();    // PWM8 -> P0.3

    // 关闭中断，纯硬件输出
    NVIC_PWM_Init(PWMA, DISABLE, Priority_0);
    NVIC_PWM_Init(PWMB, DISABLE, Priority_0);
}

void Servo_init(){
    GPIO_config();
    PWM_config();
}

// 把数字value限定在min和max之间
int clip_value(int value, int min, int max){
    if(value < min) return min;
    if(value > max) return max;
    return value;
}

/**********************************************************
 * @brief 设置指定舵机的角度
 * @param servo_id 舵机编号(1~8)
 * @param angle [0, 180] 目标角度
 **********************************************************/
void Servo_set_angle(u8 servo_id, float angle) {
    u16 duty;
    u16 pwm_val;
    
    angle = (float)clip_value((int)angle, 0, 180);
    
    // 线性映射: 角度[0,180] -> 脉宽[500,2500]us
    duty = 500 + (angle * 2000 / 180.0f);
    duty *= 0.99f; // 稍微缩小范围，防止舵机堵转
    
    // 将脉宽时间转换为PWM计数值
    pwm_val = PERIOD * duty / 20000; // 20ms -> 20000us

    // 根据舵机编号，更新对应通道的占空比并写入硬件
    switch(servo_id) {
        case 1: dutyB.PWM1_Duty = pwm_val; UpdatePwm(PWM1, &dutyB); break;
        case 2: dutyB.PWM2_Duty = pwm_val; UpdatePwm(PWM2, &dutyB); break;
        case 3: dutyB.PWM3_Duty = pwm_val; UpdatePwm(PWM3, &dutyB); break;
        case 4: dutyB.PWM4_Duty = pwm_val; UpdatePwm(PWM4, &dutyB); break;
        case 5: dutyB.PWM5_Duty = pwm_val; UpdatePwm(PWM5, &dutyB); break;
        case 6: dutyB.PWM6_Duty = pwm_val; UpdatePwm(PWM6, &dutyB); break;
        case 7: dutyB.PWM7_Duty = pwm_val; UpdatePwm(PWM7, &dutyB); break;
        case 8: dutyB.PWM8_Duty = pwm_val; UpdatePwm(PWM8, &dutyB); break;
        default: break; // 如果传入错误编号，什么都不做
    }
}