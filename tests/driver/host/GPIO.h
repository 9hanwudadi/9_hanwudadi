#ifndef HOST_GPIO_H
#define HOST_GPIO_H
#include "Config.h"
#define GPIO_OUT_OD 2
#define GPIO_Pin_2 0x04
#define GPIO_Pin_3 0x08
#define GPIO_P3 3
typedef struct {
    u8 Mode;
    u8 Pin;
} GPIO_InitTypeDef;
u8 GPIO_Inilize(u8 GPIO, GPIO_InitTypeDef *GPIOx);
#endif
