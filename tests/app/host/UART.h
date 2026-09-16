#ifndef APP_HOST_UART_H
#define APP_HOST_UART_H
#include "Config.h"
#define UART1 1
#define UART_8bit_BRTx 64
#define BRT_Timer1 1
#define COM_RX1_Lenth 128
typedef struct {
    u8 TX_send, TX_write, B_TX_busy, RX_Cnt, RX_TimeOut;
} COMx_Define;
typedef struct {
    u8 UART_Mode, UART_BRT_Use;
    u32 UART_BaudRate;
    u8 Morecommunicate, UART_RxEnable, BaudRateDouble;
} COMx_InitDefine;
extern COMx_Define COM1;
extern u8 RX1_Buffer[COM_RX1_Lenth];
u8 UART_Configuration(u8 port, COMx_InitDefine *config);
#endif
