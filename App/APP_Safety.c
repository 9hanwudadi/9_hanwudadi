#include "App.h"
#include "battery.h"
#include "ultrasonic.h"

/* Initial thresholds only; confirm battery chemistry, divider, sensor
 * placement, and stopping distance before operating real hardware. */
#define BATTERY_MIN_VOLTAGE 6.0f
#define OBSTACLE_MIN_DISTANCE_CM 15.0f

static volatile int8 g_safety_status = APP_ERR_NOT_READY;

int8 APP_Safety_Check(void)
{
    float voltage;
    float distance;
    int8 battery_status;
    int8 ultrasonic_status;

    battery_status = Battery_GetVoltage(&voltage);
    ultrasonic_status = Ultrasonic_GetDistance(&distance);
    if ((battery_status == DRIVER_OK && voltage < BATTERY_MIN_VOLTAGE) ||
        (ultrasonic_status == DRIVER_OK && distance < OBSTACLE_MIN_DISTANCE_CM)) {
        (void)APP_Robot_SetMotion(ROBOT_MOTION_STOP);
    }
    /* Report the first unavailable/error sensor even if the other sensor
     * confirmed a hazard and requested STOP. Always sample both sensors. */
    if (battery_status != DRIVER_OK) {
        return battery_status;
    }
    return ultrasonic_status;
}

void task_safety(void) _task_ TASK_SAFETY
{
    for (;;) {
        g_safety_status = APP_Safety_Check();
        os_wait(K_TMO, 10, 0);
    }
}
