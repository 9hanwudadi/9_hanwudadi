#ifndef APP_HOST_CONFIG_H
#define APP_HOST_CONFIG_H
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef signed char int8;
#define SUCCESS 0
#define ENABLE 1
#define DISABLE 0
extern u8 EA;
void host_extended_registers(void);
#define EAXSFR() host_extended_registers()
#endif
