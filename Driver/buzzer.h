#ifndef QUADRUPED_BUZZER_H
#define QUADRUPED_BUZZER_H
//#include "Driver_Common.h"
#include "Config.h"
#include "GPIO.h"
#include "Switch.h" 
#include "STC8H_PWM.h"
#include "NVIC.h"
#include "Delay.h"
/* Init is inert; sound generation needs confirmed pin and timer/PWM. */
//int8 Buzzer_Init(void);

#define	L1	1
#define	L2	2
#define	L3	3
#define	L4	4
#define	L5	5
#define	L6	6
#define	L7	7

#define N0 0

#define	N1	L1 + 7
#define	N2	L2 + 7
#define	N3	L3 + 7
#define	N4	L4 + 7
#define	N5	L5 + 7
#define	N6	L6 + 7
#define	N7	L7 + 7

#define	H1	N1 + 7
#define	H2	N2 + 7
#define	H3	N3 + 7
#define	H4	N4 + 7
#define	H5	N5 + 7
#define	H6	N6 + 7
#define	H7	N7 + 7

#define BUZZER P00

int8 Buzzer_Init(void); //蜂鸣器初始化

void Dancing_Buzzer(void); //跳舞音乐



int8 Buzzer_Beep(u16 frequency_hz, u16 duration_ms);
/* Safe no-op until hardware allocation is implemented. */
void Buzzer_Stop(void);
#endif
