#include "bluetooth.h"

int8 Bluetooth_Init(void)
{
    return DRIVER_OK;
}

int8 Bluetooth_SendByte(u8 value)
{
    (void)value;
    return DRIVER_ERR_NOT_READY;
}
