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

/* Diagnostic init results only: inert Init success does not mean that
 * optional hardware is operational. Each operation must report readiness.
 * Order: Bluetooth, ultrasonic, battery, buzzer, light. */
static volatile int8 g_optional_init_status[5];

int8 APP_System_Init(void)
{
    GPIO_InitTypeDef gpio;
    COMx_InitDefine uart;
    int8 status;
    u8 i;

    EA = 1;
    EAXSFR();
    for (i = 0; i < 5; ++i) {
        g_optional_init_status[i] = APP_ERR_NOT_READY;
    }

    /* Core console: UART1 P3.0/P3.1, Timer1 baud clock, 24 MHz MCU.
     * Timer0 remains owned by RTX51 Tiny. */
    gpio.Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpio.Mode = GPIO_PullUp;
    if (GPIO_Inilize(GPIO_P3, &gpio) != SUCCESS) {
        return APP_ERR_IO;
    }
    uart.UART_Mode = UART_8bit_BRTx;
    uart.UART_BRT_Use = BRT_Timer1;
    uart.UART_BaudRate = 115200UL;
    uart.Morecommunicate = DISABLE;
    uart.UART_RxEnable = ENABLE;
    uart.BaudRateDouble = DISABLE;
    if (UART_Configuration(UART1, &uart) != SUCCESS) {
        return APP_ERR_IO;
    }
    if (NVIC_UART1_Init(ENABLE, Priority_1) != SUCCESS) {
        return APP_ERR_IO;
    }
    UART1_SW(UART1_SW_P30_P31);

    /* Software I2C configuration belongs to PCA9685_Init. Servo_Init
     * currently repeats this idempotent initialization in its Driver. */
    status = PCA9685_Init();
    if (status != DRIVER_OK) {
        return status;
    }
    status = Servo_Init();
    if (status != DRIVER_OK) {
        return status;
    }

    g_optional_init_status[0] = Bluetooth_Init();
    g_optional_init_status[1] = Ultrasonic_Init();
    g_optional_init_status[2] = Battery_Init();
    g_optional_init_status[3] = Buzzer_Init();
    g_optional_init_status[4] = Light_Init();
    return APP_OK;
}
