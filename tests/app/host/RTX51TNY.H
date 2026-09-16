#ifndef APP_HOST_RTX51TNY_H
#define APP_HOST_RTX51TNY_H
#define K_TMO 1
unsigned char os_wait(unsigned char event, unsigned char ticks, unsigned char dummy);
unsigned char os_create_task(unsigned char id);
unsigned char os_delete_task(unsigned char id);
#endif
