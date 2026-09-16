#ifndef QUADRUPED_SERVO_H
#define QUADRUPED_SERVO_H

#include "Driver_Common.h"

#define SERVO_COUNT 8

/* Selects 50 Hz but does not issue a pose. Calibrate tables in servo.c. */
int8 Servo_Init(void);
/* Call Init first; retain 50 Hz. Invalid IDs/angles are never clamped. */
int8 Servo_SetAngle(u8 servo_id, u8 angle_deg);
/* Requires eight angles; validates all before any write. An I/O error
 * can leave an already-written prefix applied; the caller handles safety. */
int8 Servo_SetPose(const u8 *angles, u8 count);

#endif
