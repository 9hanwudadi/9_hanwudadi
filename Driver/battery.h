#ifndef QUADRUPED_BATTERY_H
#define QUADRUPED_BATTERY_H
#include "Driver_Common.h"
/* Init is inert; sampling needs confirmed ADC pin and divider values. */
int8 Battery_Init(void);
/* Null is invalid. On NOT_READY the output is left unchanged. */
int8 Battery_GetVoltage(float *voltage_v);
#endif
