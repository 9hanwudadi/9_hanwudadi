#ifndef __SERVO_H__
#define __SERVO_H__

#include "Config.h"
#include "Delay.h"
#include "UART.h"
#include "STC8H_PWM.h"
#include "NVIC.h"
#include "Switch.h"
#include "GPIO.h"
void Servo_init();
void Servo_set_angle(u8 servo_id, float angle);
int clip_value(int value, int min, int max);
#endif