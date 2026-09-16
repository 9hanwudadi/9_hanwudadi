#include "App.h"
#include "servo.h"

static volatile RobotMotionMode g_target_motion = ROBOT_MOTION_STOP;
static volatile int8 g_motion_status = APP_ERR_NOT_READY;

/* Calibration placeholders: servo IDs are four hip/knee pairs. STOP
 * requests a full neutral pose; it does not cut power or disable PWM.
 * Verify joint order, direction, zero, and mechanical limits before use. */
static const u8 g_stop_pose[SERVO_COUNT] =
    {90, 90, 90, 90, 90, 90, 90, 90};
static const u8 g_stand_pose[SERVO_COUNT] =
    {90, 60, 90, 60, 90, 60, 90, 60};
static const u8 g_crouch_pose[SERVO_COUNT] =
    {90, 120, 90, 120, 90, 120, 90, 120};

int8 APP_Robot_SetMotion(RobotMotionMode mode)
{
    u8 saved_ea;

    if (mode < ROBOT_MOTION_STOP || mode > ROBOT_MOTION_GREET) {
        return APP_ERR_PARAM;
    }
    /* Keep the enum update atomic even with a 16-bit enum representation.
     * Call from task context; do not call App APIs from an ISR. */
    saved_ea = EA;
    EA = 0;
    g_target_motion = mode;
    EA = saved_ea;
    if (mode >= ROBOT_MOTION_FORWARD) {
        return APP_ERR_NOT_READY;
    }
    return APP_OK;
}

void task_robot_motion(void) _task_ TASK_ROBOT_MOTION
{
    RobotMotionMode mode;
    u8 saved_ea;
    const u8 *pose;

    for (;;) {
        saved_ea = EA;
        EA = 0;
        mode = g_target_motion;
        EA = saved_ea;

        switch (mode) {
        case ROBOT_MOTION_STAND:
            pose = g_stand_pose;
            break;
        case ROBOT_MOTION_CROUCH:
            pose = g_crouch_pose;
            break;
        case ROBOT_MOTION_STOP:
        default:
            /* Future gait modes retain their requested state, but use
             * the safe-stop table until their algorithms are implemented. */
            pose = g_stop_pose;
            break;
        }
        g_motion_status = Servo_SetPose(pose, SERVO_COUNT);
        if (g_motion_status != DRIVER_OK) {
            /* An I/O error may have applied only a prefix of the pose.
             * Retry the entire safe-stop pose on the next task cycle. */
            (void)APP_Robot_SetMotion(ROBOT_MOTION_STOP);
        }
        os_wait(K_TMO, 2, 0);
    }
}
