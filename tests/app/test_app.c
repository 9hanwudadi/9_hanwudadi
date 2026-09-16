#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "App.h"
#include "GPIO.h"
#include "UART.h"
#include "NVIC.h"
#include "Switch.h"
#include "pca9685.h"
#include "servo.h"
#include "bluetooth.h"
#include "ultrasonic.h"
#include "battery.h"
#include "buzzer.h"
#include "light.h"

static unsigned int failures, checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { \
    ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); \
} } while (0)

u8 EA;
COMx_Define COM1;
u8 RX1_Buffer[COM_RX1_Lenth];
static char order[32];
static unsigned int order_count;
static char fail_stage;
static int8 optional_status, pose_status;
static int8 voltage_status, distance_status;
static float voltage, distance;
static unsigned int voltage_calls, distance_calls;
static u8 pose[8], pose_count;
static unsigned int pose_calls;
static unsigned int created_count, deleted_count;
static u8 created[3], deleted;
static jmp_buf task_end;
static unsigned int waits_remaining;

static int8 record(char stage)
{
    order[order_count++] = stage;
    order[order_count] = '\0';
    return stage == fail_stage ? DRIVER_ERR_IO : DRIVER_OK;
}

void host_extended_registers(void) { CHECK(EA == 1); (void)record('E'); }
u8 GPIO_Inilize(u8 port, GPIO_InitTypeDef *config)
{
    CHECK(port == 3 && config->Pin == 3 && config->Mode == 0);
    return record('G') == 0 ? 0 : 255;
}
u8 UART_Configuration(u8 port, COMx_InitDefine *config)
{
    CHECK(port == 1 && config->UART_Mode == 64);
    CHECK(config->UART_BRT_Use == 1 && config->UART_BaudRate == 115200UL);
    CHECK(config->Morecommunicate == 0 && config->UART_RxEnable == 1);
    CHECK(config->BaudRateDouble == 0);
    return record('U') == 0 ? 0 : 255;
}
u8 NVIC_UART1_Init(u8 enabled, u8 priority)
{
    CHECK(enabled == 1 && priority == 1);
    return record('N') == 0 ? 0 : 255;
}
void host_uart_switch(u8 pins) { CHECK(pins == 0); (void)record('W'); }
int8 PCA9685_Init(void) { return record('P'); }
int8 Servo_Init(void) { return record('S'); }
int8 Bluetooth_Init(void) { (void)record('B'); return optional_status; }
int8 Ultrasonic_Init(void) { (void)record('D'); return optional_status; }
int8 Battery_Init(void) { (void)record('V'); return optional_status; }
int8 Buzzer_Init(void) { (void)record('Z'); return optional_status; }
int8 Light_Init(void) { (void)record('L'); return optional_status; }
int8 Servo_SetPose(const u8 *angles, u8 count)
{
    unsigned int i;
    ++pose_calls;
    pose_count = count;
    for (i = 0; i < count && i < 8; ++i) { pose[i] = angles[i]; }
    return pose_status;
}
int8 Battery_GetVoltage(float *value)
{
    ++voltage_calls;
    *value = voltage; /* Poison values on errors catch missing status gates. */
    return voltage_status;
}
int8 Ultrasonic_GetDistance(float *value)
{
    ++distance_calls;
    *value = distance;
    return distance_status;
}
unsigned char os_wait(unsigned char event, unsigned char ticks, unsigned char dummy)
{
    CHECK(event == K_TMO && ticks > 0 && dummy == 0);
    if (--waits_remaining == 0) { longjmp(task_end, 1); }
    return 0;
}
unsigned char os_create_task(unsigned char id)
{
    if (created_count < 3) { created[created_count] = id; }
    ++created_count;
    return 0;
}
unsigned char os_delete_task(unsigned char id)
{
    ++deleted_count;
    deleted = id;
    return 0;
}

/* Real App code, with only task attributes removed by run.ps1.
 * Same TU permits checking the mandated file-static requested enum,
 * since unsupported modes intentionally have identical output poses. */
#include "App_System.c"
#include "APP_RobotMotion.c"
#include "APP_Command.c"
#include "APP_Safety.c"
#include "main.c"

static void reset(void)
{
    order_count = 0;
    order[0] = '\0';
    fail_stage = 0;
    optional_status = DRIVER_ERR_NOT_READY;
    pose_status = DRIVER_OK;
    voltage_status = distance_status = DRIVER_OK;
    voltage = 7.4f;
    distance = 100.0f;
    voltage_calls = distance_calls = pose_calls = 0;
    created_count = deleted_count = 0;
    pose_count = 0;
    memset(pose, 0, sizeof(pose));
    memset(&COM1, 0, sizeof(COM1));
    EA = 1;
}
static void run_task(void (*task)(void), unsigned int cycles)
{
    waits_remaining = cycles;
    if (setjmp(task_end) == 0) { task(); }
}
static void expect_pose(const u8 *expected)
{
    pose_calls = 0;
    run_task(task_robot_motion, 1);
    CHECK(pose_calls == 1 && pose_count == 8);
    CHECK(memcmp(pose, expected, 8) == 0);
}
static const u8 stop_pose[8] = {90, 90, 90, 90, 90, 90, 90, 90};
static const u8 stand_pose[8] = {90, 60, 90, 60, 90, 60, 90, 60};
static const u8 crouch_pose[8] = {90, 120, 90, 120, 90, 120, 90, 120};

static void test_initialization(void)
{
    unsigned int i;
    static const char stages[] = "GUNPS";
    static const char *prefix[] = {"EG", "EGU", "EGUN", "EGUNWP", "EGUNWPS"};
    reset();
    CHECK(APP_System_Init() == 0);
    CHECK(strcmp(order, "EGUNWPSBDVZL") == 0);
    CHECK(pose_calls == 0);
    optional_status = DRIVER_ERR_IO;
    order_count = 0;
    CHECK(APP_System_Init() == 0);
    CHECK(strcmp(order, "EGUNWPSBDVZL") == 0);
    for (i = 0; i < sizeof(stages) - 1; ++i) {
        reset();
        fail_stage = stages[i];
        CHECK(APP_System_Init() == -3);
        CHECK(strcmp(order, prefix[i]) == 0);
    }
}
static void test_modes_and_pose_failures(void)
{
    unsigned int i;
    reset();
    CHECK(APP_Robot_SetMotion(ROBOT_MOTION_STOP) == 0);
    expect_pose(stop_pose);
    CHECK(APP_Robot_SetMotion(ROBOT_MOTION_STAND) == 0);
    expect_pose(stand_pose);
    CHECK(APP_Robot_SetMotion(ROBOT_MOTION_CROUCH) == 0);
    expect_pose(crouch_pose);
    CHECK(APP_Robot_SetMotion((RobotMotionMode)-1) == -1);
    CHECK(APP_Robot_SetMotion((RobotMotionMode)8) == -1);
    CHECK(APP_Robot_SetMotion((RobotMotionMode)256) == -1);
    expect_pose(crouch_pose);
    for (i = 3; i <= 7; ++i) {
        CHECK(APP_Robot_SetMotion((RobotMotionMode)i) == -2);
        CHECK(g_target_motion == (RobotMotionMode)i);
        expect_pose(stop_pose);
    }
    EA = 0;
    (void)APP_Robot_SetMotion(ROBOT_MOTION_STAND);
    CHECK(EA == 0);
    expect_pose(stand_pose);
    CHECK(EA == 0);
    pose_status = DRIVER_ERR_IO;
    run_task(task_robot_motion, 1);
    CHECK(g_target_motion == ROBOT_MOTION_STOP);
    pose_status = DRIVER_OK;
    expect_pose(stop_pose);
}
static void test_commands(void)
{
    static const u8 commands[] = {'0', '1', '2', 'w', 's', 'a', 'd', 'h'};
    unsigned int i;
    reset();
    for (i = 0; i < 8; ++i) {
        APP_Command_OnByte(commands[i]);
        CHECK(g_target_motion == (RobotMotionMode)i);
    }
    for (i = 0; i <= 255; ++i) {
        if (strchr("012wsadh", (int)i) == 0 || i == 0) {
            (void)APP_Robot_SetMotion(ROBOT_MOTION_CROUCH);
            APP_Command_OnByte((u8)i);
            CHECK(g_target_motion == ROBOT_MOTION_CROUCH);
        }
    }
    /* No receive API exists yet: the command task only yields. */
    (void)APP_Robot_SetMotion(ROBOT_MOTION_STAND);
    run_task(task_command, 2);
    CHECK(g_target_motion == ROBOT_MOTION_STAND);
}
static void check_safety(float volts, float cm, int8 vs, int8 ds, int8 result, int stop)
{
    reset();
    voltage = volts; distance = cm; voltage_status = vs; distance_status = ds;
    (void)APP_Robot_SetMotion(ROBOT_MOTION_CROUCH);
    CHECK(APP_Safety_Check() == result);
    CHECK(voltage_calls == 1 && distance_calls == 1);
    CHECK(g_target_motion == (stop ? ROBOT_MOTION_STOP : ROBOT_MOTION_CROUCH));
}
static void test_safety(void)
{
    check_safety(6.0f, 15.0f, 0, 0, 0, 0);
    check_safety(5.99f, 100.0f, 0, 0, 0, 1);
    check_safety(7.4f, 14.99f, 0, 0, 0, 1);
    check_safety(0.0f, 0.0f, -2, -2, -2, 0);
    check_safety(0.0f, 100.0f, -3, 0, -3, 0);
    check_safety(7.4f, 0.0f, 0, -4, -4, 0);
    check_safety(5.99f, 0.0f, 0, -2, -2, 1);
    check_safety(0.0f, 14.99f, -2, 0, -2, 1);
    check_safety(0.0f, 0.0f, -3, -4, -3, 0);
    reset();
    (void)APP_Robot_SetMotion(ROBOT_MOTION_STAND);
    voltage = 5.0f;
    run_task(task_safety, 2);
    CHECK(g_target_motion == ROBOT_MOTION_STOP);
    CHECK(voltage_calls == 2 && distance_calls == 2);
}
static void test_startup(void)
{
    reset();
    main_task();
    CHECK(created_count == 3);
    CHECK(created[0] == 1 && created[1] == 2 && created[2] == 3);
    CHECK(deleted_count == 1 && deleted == 0);
    reset();
    fail_stage = 'P';
    main_task();
    CHECK(created_count == 0);
    CHECK(deleted_count == 1 && deleted == 0);
}
int main(void)
{
    test_initialization();
    test_modes_and_pose_failures();
    test_commands();
    test_safety();
    test_startup();
    printf("%s: %u App checks, %u failures\n", failures ? "FAIL" : "PASS", checks, failures);
    return failures ? 1 : 0;
}
