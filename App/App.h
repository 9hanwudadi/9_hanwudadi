#ifndef QUADRUPED_APP_H
#define QUADRUPED_APP_H

#include "Config.h"
#include "RTX51TNY.H"
#include "Driver_Common.h"

#define TASK_ROBOT_MOTION 1
#define TASK_COMMAND      2
#define TASK_SAFETY       3

#define APP_OK             DRIVER_OK
#define APP_ERR_PARAM      DRIVER_ERR_PARAM
#define APP_ERR_NOT_READY  DRIVER_ERR_NOT_READY
#define APP_ERR_IO         DRIVER_ERR_IO
#define APP_ERR_BUSY       DRIVER_ERR_BUSY

typedef enum {
    ROBOT_MOTION_STOP = 0,
    ROBOT_MOTION_STAND,
    ROBOT_MOTION_CROUCH,
    ROBOT_MOTION_FORWARD,
    ROBOT_MOTION_BACKWARD,
    ROBOT_MOTION_TURN_LEFT,
    ROBOT_MOTION_TURN_RIGHT,
    ROBOT_MOTION_GREET
} RobotMotionMode;

int8 APP_System_Init(void);
int8 APP_Robot_SetMotion(RobotMotionMode mode);
void APP_Command_OnByte(u8 value);
int8 APP_Safety_Check(void);

void task_robot_motion(void) _task_ TASK_ROBOT_MOTION;
void task_command(void) _task_ TASK_COMMAND;
void task_safety(void) _task_ TASK_SAFETY;

#endif
