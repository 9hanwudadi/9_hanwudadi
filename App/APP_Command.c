#include "App.h"

void APP_Command_OnByte(u8 value)
{
    RobotMotionMode mode;

    switch (value) {
    case '0': mode = ROBOT_MOTION_STOP; break;
    case '1': mode = ROBOT_MOTION_STAND; break;
    case '2': mode = ROBOT_MOTION_CROUCH; break;
    case 'w': mode = ROBOT_MOTION_FORWARD; break;
    case 's': mode = ROBOT_MOTION_BACKWARD; break;
    case 'a': mode = ROBOT_MOTION_TURN_LEFT; break;
    case 'd': mode = ROBOT_MOTION_TURN_RIGHT; break;
    case 'h': mode = ROBOT_MOTION_GREET; break;
    default: return;
    }
    (void)APP_Robot_SetMotion(mode);
}

void task_command(void) _task_ TASK_COMMAND
{
    for (;;) {
        /* Receive adapter pending Driver API and confirmed UART allocation.
         * Future task-context bytes enter through APP_Command_OnByte. */
        os_wait(K_TMO, 1, 0);
    }
}
