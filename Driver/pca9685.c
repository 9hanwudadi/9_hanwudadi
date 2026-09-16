#include "pca9685.h"
#include "Soft_I2C.h"
#include "Delay.h"
#include "GPIO.h"

#define PCA9685_WRITE_ADDRESS 0x80
#define PCA9685_MODE1         0x00
#define PCA9685_PRESCALE      0xFE
#define PCA9685_LED0_ON_L     0x06

static int8 PCA9685_WriteRegister(u8 reg, u8 value)
{
    SI2C_WriteNbyte(PCA9685_WRITE_ADDRESS, reg, &value, 1);
    /* The imported software I2C leaves its last ACK result in F0. */
    if (F0) {
        return DRIVER_ERR_IO;
    }
    return DRIVER_OK;
}

int8 PCA9685_Init(void)
{
    GPIO_InitTypeDef pins;

    /* Soft_I2C uses P3.2 (SCL) and P3.3 (SDA); external pull-ups required. */
    pins.Mode = GPIO_OUT_OD;
    pins.Pin = GPIO_Pin_2 | GPIO_Pin_3;
    if (GPIO_Inilize(GPIO_P3, &pins) != 0) {
        return DRIVER_ERR_IO;
    }
    return PCA9685_SetFrequency(50);
}

int8 PCA9685_SetFrequency(u16 frequency_hz)
{
    u32 denominator;
    u8 prescale;
    int8 status;

    if (frequency_hz < 40 || frequency_hz > 1000) {
        return DRIVER_ERR_PARAM;
    }
    denominator = 4096UL * frequency_hz;
    prescale = (u8)((25000000UL + denominator / 2UL) / denominator - 1UL);

    /* PRESCALE may only be changed while asleep. Keep auto-increment on. */
    status = PCA9685_WriteRegister(PCA9685_MODE1, 0x30);
    if (status != DRIVER_OK) {
        return status;
    }
    status = PCA9685_WriteRegister(PCA9685_PRESCALE, prescale);
    if (status != DRIVER_OK) {
        return status;
    }
    status = PCA9685_WriteRegister(PCA9685_MODE1, 0x20);
    if (status != DRIVER_OK) {
        return status;
    }
    /* Allow more than 500 us for the internal oscillator to stabilize. */
    delay_ms(1);
    return PCA9685_WriteRegister(PCA9685_MODE1, 0xA0);
}

int8 PCA9685_SetPwm(u8 channel, u16 on_count, u16 off_count)
{
    u8 bytes[4];
    u8 reg;

    if (channel > 15 || on_count > 4095 || off_count > 4095) {
        return DRIVER_ERR_PARAM;
    }
    reg = (u8)(PCA9685_LED0_ON_L + channel * 4);
    bytes[0] = (u8)on_count;
    bytes[1] = (u8)(on_count >> 8);
    bytes[2] = (u8)off_count;
    bytes[3] = (u8)(off_count >> 8);
    SI2C_WriteNbyte(PCA9685_WRITE_ADDRESS, reg, bytes, 4);
    if (F0) {
        return DRIVER_ERR_IO;
    }
    return DRIVER_OK;
}
