#ifndef QUADRUPED_LIGHT_H
#define QUADRUPED_LIGHT_H
#include "Driver_Common.h"
/* Init is inert; outputs need confirmed IDs, pins, and active levels. */
int8 Light_Init(void);
int8 Light_Set(u8 light_id, u8 enabled);
#endif
