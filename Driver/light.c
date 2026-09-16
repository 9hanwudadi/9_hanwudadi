#include "light.h"

int8 Light_Init(void)
{
    return DRIVER_OK;
}

int8 Light_Set(u8 light_id, u8 enabled)
{
    (void)light_id;
    (void)enabled;
    return DRIVER_ERR_NOT_READY;
}
