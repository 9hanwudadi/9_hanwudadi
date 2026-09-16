#include "ultrasonic.h"

int8 Ultrasonic_Init(void)
{
    return DRIVER_OK;
}

int8 Ultrasonic_GetDistance(float *distance_cm)
{
    if (distance_cm == 0) {
        return DRIVER_ERR_PARAM;
    }
    return DRIVER_ERR_NOT_READY;
}
