#ifndef QUADRUPED_LIGHT_H
#define QUADRUPED_LIGHT_H
//#include "Driver_Common.h"
#include "Config.h"
#include "GPIO.h"
#include "Delay.h"


/* Init is inert; outputs need confirmed IDs, pins, and active levels. */
//int8 Light_Init(void);
int8 Light_Set(u8 light_id, u8 enabled);

#define		LED_SW		P45
#define		LED_F		P35
#define		LED1		P27
#define		LED2		P26
#define		LED3		P15
#define		LED4		P14
#define		LED5		P23
#define		LED6		P22
#define		LED7		P21
#define		LED8		P20


int8 Light_Init(void); //LED初始化

void All_Dark(void); //所有灯熄灭

void Front_Lights(void); //前灯(LED,LED9)全部点亮

void Dancing_Lights(void); //跳舞时使用


#endif
