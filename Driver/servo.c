#include "servo.h"
#include "pca9685.h"

/* Neutral defaults only. Confirm channels, directions (+1/-1), zero
 * offsets and the 1000..2000 us range on the actual robot before motion. */
static const u8 servo_channels[SERVO_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7};
static const int8 servo_directions[SERVO_COUNT] = {1, 1, 1, 1, 1, 1, 1, 1};
static const int8 servo_zero_offsets[SERVO_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0};

static int8 Servo_Convert(u8 servo_id, u8 angle_deg, u16 *off_count)
{
    int16 calibrated;
    u32 pulse_us;

    if (servo_id >= SERVO_COUNT || angle_deg > 180) {
        return DRIVER_ERR_PARAM;
    }
    calibrated = angle_deg;
    if (servo_directions[servo_id] < 0) {
        calibrated = 180 - calibrated;
    }
    calibrated += servo_zero_offsets[servo_id];
    if (calibrated < 0 || calibrated > 180) {
        return DRIVER_ERR_PARAM;
    }
    pulse_us = 1000UL + ((u32)calibrated * 1000UL + 90UL) / 180UL;
    *off_count = (u16)((pulse_us * 4096UL + 10000UL) / 20000UL);
    return DRIVER_OK;
}

int8 Servo_Init(void)
{
    return PCA9685_Init();
}

int8 Servo_SetAngle(u8 servo_id, u8 angle_deg)
{
    u16 off_count;
    int8 status;

    status = Servo_Convert(servo_id, angle_deg, &off_count);
    if (status != DRIVER_OK) {
        return status;
    }
    return PCA9685_SetPwm(servo_channels[servo_id], 0, off_count);
}

int8 Servo_SetPose(const u8 *angles, u8 count)
{
    u8 i;
    u16 counts[SERVO_COUNT];
    int8 status;

    if (angles == 0 || count != SERVO_COUNT) {
        return DRIVER_ERR_PARAM;
    }
    for (i = 0; i < SERVO_COUNT; ++i) {
        status = Servo_Convert(i, angles[i], &counts[i]);
        if (status != DRIVER_OK) {
            return status;
        }
    }
    for (i = 0; i < SERVO_COUNT; ++i) {
        status = PCA9685_SetPwm(servo_channels[i], 0, counts[i]);
        if (status != DRIVER_OK) {
            return status;
        }
    }
    return DRIVER_OK;
}
