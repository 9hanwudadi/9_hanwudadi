#include "battery.h"

int8 Battery_Init(void)
{
    return DRIVER_OK;
}

int8 Battery_GetVoltage(float *voltage_v)
{
    if (voltage_v == 0) {
        return DRIVER_ERR_PARAM;
    }
    return DRIVER_ERR_NOT_READY;
}
