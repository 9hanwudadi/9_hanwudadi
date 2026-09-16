#ifndef QUADRUPED_BUZZER_H
#define QUADRUPED_BUZZER_H
#include "Driver_Common.h"
/* Init is inert; sound generation needs confirmed pin and timer/PWM. */
int8 Buzzer_Init(void);
int8 Buzzer_Beep(u16 frequency_hz, u16 duration_ms);
/* Safe no-op until hardware allocation is implemented. */
void Buzzer_Stop(void);
#endif
