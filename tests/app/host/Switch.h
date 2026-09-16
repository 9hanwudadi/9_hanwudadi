#ifndef APP_HOST_SWITCH_H
#define APP_HOST_SWITCH_H
#include "Config.h"
#define UART1_SW_P30_P31 0
void host_uart_switch(u8 pins);
#define UART1_SW(pins) host_uart_switch(pins)
#endif
