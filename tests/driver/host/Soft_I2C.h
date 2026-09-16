#ifndef HOST_SOFT_I2C_H
#define HOST_SOFT_I2C_H
#include "Config.h"
void SI2C_WriteNbyte(u8 dev_addr, u8 mem_addr, u8 *p, u8 number);
#endif
