#ifndef QUADRUPED_ULTRASONIC_H
#define QUADRUPED_ULTRASONIC_H
#include "Driver_Common.h"
/* Init is inert; sampling needs confirmed trigger/echo pins and timing. */
int8 Ultrasonic_Init(void);
/* Null is invalid. On NOT_READY the output is left unchanged. */
int8 Ultrasonic_GetDistance(float *distance_cm);
#endif
