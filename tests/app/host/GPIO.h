#ifndef APP_HOST_GPIO_H
#define APP_HOST_GPIO_H
#include "Config.h"
#define GPIO_P3 3
#define GPIO_Pin_0 1
#define GPIO_Pin_1 2
#define GPIO_PullUp 0
typedef struct { u8 Mode; u8 Pin; } GPIO_InitTypeDef;
u8 GPIO_Inilize(u8 port, GPIO_InitTypeDef *config);
#endif
