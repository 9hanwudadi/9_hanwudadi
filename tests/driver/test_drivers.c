#include <stdio.h>
#include <stdlib.h>
#include "pca9685.h"
#include "servo.h"
#include "bluetooth.h"
#include "ultrasonic.h"
#include "battery.h"
#include "buzzer.h"
#include "light.h"
#include "GPIO.h"

#define CHECK(expr) do { if (!(expr)) { \
    printf("FAIL line %d: %s\n", __LINE__, #expr); exit(1); \
} } while (0)

struct BusWrite {
    u8 address;
    u8 reg;
    u8 count;
    u8 bytes[4];
};
static struct BusWrite writes[64];
static unsigned int write_count;
static unsigned int fail_at;
static unsigned int elapsed_ms;
static unsigned int wake_write_count;
static unsigned int gpio_call_count;
static unsigned int writes_before_gpio;
static u8 gpio_port;
static u8 gpio_mode;
static u8 gpio_pins;
static u8 gpio_result;
u8 F0;

u8 GPIO_Inilize(u8 port, GPIO_InitTypeDef *config)
{
    ++gpio_call_count;
    writes_before_gpio = write_count;
    gpio_port = port;
    gpio_mode = config->Mode;
    gpio_pins = config->Pin;
    return gpio_result;
}

/* Hardware spy: preserve bus-write order/payload and the vendor ACK flag. */
void SI2C_WriteNbyte(u8 address, u8 reg, u8 *bytes, u8 count)
{
    u8 i;
    CHECK(write_count < 64);
    CHECK(count > 0 && count <= 4);
    writes[write_count].address = address;
    writes[write_count].reg = reg;
    writes[write_count].count = count;
    for (i = 0; i < count; ++i) {
        writes[write_count].bytes[i] = bytes[i];
    }
    ++write_count;
    F0 = (u8)(write_count == fail_at);
}

void delay_ms(unsigned char ms)
{
    elapsed_ms += ms;
    wake_write_count = write_count;
}

static void reset_bus(void)
{
    write_count = 0;
    fail_at = 0;
    elapsed_ms = 0;
    wake_write_count = 0;
    gpio_call_count = 0;
    writes_before_gpio = 0;
    gpio_port = 0;
    gpio_mode = 0;
    gpio_pins = 0;
    gpio_result = 0;
    F0 = 0;
}

static void test_i2c_pin_initialization(void)
{
    reset_bus();
    CHECK(PCA9685_Init() == 0);
    CHECK(gpio_call_count == 1);
    CHECK(gpio_port == 3);
    CHECK(gpio_pins == 0x0C);
    CHECK(gpio_mode == 2);
    CHECK(writes_before_gpio == 0 && write_count == 4);
    reset_bus();
    /* GPIO_Inilize returns u8, so the vendor's FAIL (-1) is 255. */
    gpio_result = 255;
    CHECK(PCA9685_Init() == -3);
    CHECK(gpio_call_count == 1 && write_count == 0);
    reset_bus();
    CHECK(Servo_Init() == 0);
    CHECK(gpio_call_count == 1);
    CHECK(gpio_port == 3 && gpio_pins == 0x0C && gpio_mode == 2);
    CHECK(writes_before_gpio == 0 && write_count == 4);
    reset_bus();
    gpio_result = 255;
    CHECK(Servo_Init() == -3);
    CHECK(gpio_call_count == 1 && write_count == 0);
}

static void expect_pwm(unsigned int index, u8 reg, u8 on_lo,
                       u8 on_hi, u8 off_lo, u8 off_hi)
{
    CHECK(index < write_count);
    CHECK(writes[index].address == 0x80);
    CHECK(writes[index].reg == reg);
    CHECK(writes[index].count == 4);
    CHECK(writes[index].bytes[0] == on_lo);
    CHECK(writes[index].bytes[1] == on_hi);
    CHECK(writes[index].bytes[2] == off_lo);
    CHECK(writes[index].bytes[3] == off_hi);
}

static void expect_frequency(u16 hz, u8 prescale)
{
    unsigned int i;
    reset_bus();
    CHECK(PCA9685_SetFrequency(hz) == 0);
    CHECK(write_count == 4);
    for (i = 0; i < 4; ++i) {
        CHECK(writes[i].address == 0x80);
        CHECK(writes[i].count == 1);
    }
    /* Sleep with auto-increment, program prescaler, wake, then restart. */
    CHECK(writes[0].reg == 0x00 && writes[0].bytes[0] == 0x30);
    CHECK(writes[1].reg == 0xFE && writes[1].bytes[0] == prescale);
    CHECK(writes[2].reg == 0x00 && writes[2].bytes[0] == 0x20);
    CHECK(elapsed_ms >= 1 && wake_write_count == 3);
    CHECK(writes[3].reg == 0x00 && writes[3].bytes[0] == 0xA0);
}

static void test_pca9685(void)
{
    unsigned int i;
    reset_bus();
    CHECK(PCA9685_SetFrequency(0) == -1);
    CHECK(PCA9685_SetFrequency(39) == -1);
    CHECK(PCA9685_SetFrequency(1001) == -1);
    CHECK(PCA9685_SetFrequency(65535U) == -1);
    CHECK(PCA9685_SetPwm(16, 0, 0) == -1);
    CHECK(PCA9685_SetPwm(255, 0, 0) == -1);
    CHECK(PCA9685_SetPwm(0, 4096, 0) == -1);
    CHECK(PCA9685_SetPwm(0, 0, 4096) == -1);
    CHECK(PCA9685_SetPwm(0, 65535U, 65535U) == -1);
    CHECK(write_count == 0);
    CHECK(PCA9685_SetPwm(0, 0, 0) == 0);
    expect_pwm(0, 0x06, 0, 0, 0, 0);
    CHECK(PCA9685_SetPwm(15, 4095, 4095) == 0);
    expect_pwm(1, 0x42, 0xFF, 0x0F, 0xFF, 0x0F);
    CHECK(PCA9685_SetPwm(3, 0x123, 0xABC) == 0);
    expect_pwm(2, 0x12, 0x23, 0x01, 0xBC, 0x0A);
    /* Rounded 25 MHz / (4096 * Hz) minus one, hand-derived literals. */
    expect_frequency(40, 152);
    expect_frequency(50, 121);
    expect_frequency(1000, 5);
    reset_bus();
    CHECK(PCA9685_Init() == 0);
    CHECK(write_count == 4 && writes[1].bytes[0] == 121);
    for (i = 1; i <= 4; ++i) {
        reset_bus();
        fail_at = i;
        CHECK(PCA9685_SetFrequency(50) == -3);
        CHECK(write_count == i);
    }
    reset_bus();
    fail_at = 1;
    CHECK(PCA9685_SetPwm(0, 0, 307) == -3);
    CHECK(write_count == 1);
    reset_bus();
    fail_at = 1;
    CHECK(PCA9685_Init() == -3);
}

static void test_servo(void)
{
    u8 pose[8] = {0, 45, 90, 135, 180, 0, 90, 180};
    u8 invalid[8] = {0, 0, 0, 0, 0, 0, 0, 181};
    reset_bus();
    CHECK(Servo_SetAngle(8, 90) == -1);
    CHECK(Servo_SetAngle(255, 90) == -1);
    CHECK(Servo_SetAngle(0, 181) == -1);
    CHECK(Servo_SetAngle(0, 255) == -1);
    CHECK(Servo_SetPose(NULL, 8) == -1);
    CHECK(Servo_SetPose(pose, 0) == -1);
    CHECK(Servo_SetPose(pose, 7) == -1);
    CHECK(Servo_SetPose(pose, 9) == -1);
    CHECK(Servo_SetPose(invalid, 8) == -1);
    CHECK(write_count == 0);
    CHECK(Servo_SetPose(pose, 8) == 0);
    CHECK(write_count == 8);
    /* At 50 Hz: 1000/1250/1500/1750/2000 us round to these counts. */
    expect_pwm(0, 0x06, 0, 0, 205, 0);
    expect_pwm(1, 0x0A, 0, 0, 0, 1);
    expect_pwm(2, 0x0E, 0, 0, 51, 1);
    expect_pwm(3, 0x12, 0, 0, 102, 1);
    expect_pwm(4, 0x16, 0, 0, 154, 1);
    expect_pwm(5, 0x1A, 0, 0, 205, 0);
    expect_pwm(6, 0x1E, 0, 0, 51, 1);
    expect_pwm(7, 0x22, 0, 0, 154, 1);
    reset_bus();
    CHECK(Servo_SetAngle(0, 1) == 0);
    expect_pwm(0, 0x06, 0, 0, 206, 0);
    reset_bus();
    CHECK(Servo_Init() == 0);
    CHECK(write_count == 4 && writes[1].bytes[0] == 121);
    reset_bus();
    fail_at = 1;
    CHECK(Servo_Init() == -3);
    reset_bus();
    fail_at = 1;
    CHECK(Servo_SetAngle(0, 90) == -3);
    reset_bus();
    fail_at = 3;
    CHECK(Servo_SetPose(pose, 8) == -3);
    CHECK(write_count == 3);
}

static void test_unallocated_hardware(void)
{
    float value;
    reset_bus();
    CHECK(Bluetooth_Init() == 0);
    CHECK(Ultrasonic_Init() == 0);
    CHECK(Battery_Init() == 0);
    CHECK(Buzzer_Init() == 0);
    CHECK(Light_Init() == 0);
    CHECK(Bluetooth_SendByte(0) == -2);
    CHECK(Bluetooth_SendByte(255) == -2);
    CHECK(Ultrasonic_GetDistance(NULL) == -1);
    CHECK(Battery_GetVoltage(NULL) == -1);
    value = 123.0f;
    CHECK(Ultrasonic_GetDistance(&value) == -2 && value == 123.0f);
    CHECK(Battery_GetVoltage(&value) == -2 && value == 123.0f);
    CHECK(Buzzer_Beep(0, 0) == -2);
    CHECK(Buzzer_Beep(1000, 100) == -2);
    Buzzer_Stop();
    CHECK(Light_Set(0, 1) == -2);
    CHECK(Light_Set(255, 255) == -2);
    CHECK(write_count == 0);
}

int main(void)
{
    test_i2c_pin_initialization();
    test_pca9685();
    test_servo();
    test_unallocated_hardware();
    puts("PASS: PCA9685, servo, and unallocated hardware behavior");
    return 0;
}
