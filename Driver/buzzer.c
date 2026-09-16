#include "buzzer.h"

int8 Buzzer_Init(void)
{
    return DRIVER_OK;
}

int8 Buzzer_Beep(u16 frequency_hz, u16 duration_ms)
{
    frequency_hz = frequency_hz; /* C51-compatible unused parameters. */
    duration_ms = duration_ms;
    return DRIVER_ERR_NOT_READY;
}

void Buzzer_Stop(void)
{
    /* No output has been allocated or enabled by this template. */
}
