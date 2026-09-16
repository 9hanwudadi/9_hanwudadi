#ifndef QUADRUPED_PCA9685_H
#define QUADRUPED_PCA9685_H

#include "Driver_Common.h"

/* Init configures P3.2/P3.3 as open-drain, then selects 50 Hz without
 * writing servo positions. The software I2C bus requires external pull-ups. */
int8 PCA9685_Init(void);
/* Owns MODE1: internal 25 MHz clock, auto-increment, no subaddresses. */
int8 PCA9685_SetFrequency(u16 frequency_hz);
/* Call Init first. Channel 0..15, counts 0..4095 (no full-on/off flags). */
int8 PCA9685_SetPwm(u8 channel, u16 on_count, u16 off_count);

/* Serialize calls in task context: software I2C and F0 are shared. */
#endif
