#ifndef QUADRUPED_BLUETOOTH_H
#define QUADRUPED_BLUETOOTH_H
#include "Driver_Common.h"
/* Init is inert; sending needs confirmed UART and pin allocation. */
int8 Bluetooth_Init(void);
int8 Bluetooth_SendByte(u8 value);
#endif
