#include "light.h"

int8 Light_Init(void)
{
    return DRIVER_OK;
}

int8 Light_Set(u8 light_id, u8 enabled)
{
    light_id = light_id; /* C51-compatible unused parameters. */
    enabled = enabled;
    return DRIVER_ERR_NOT_READY;
}
