#include "light.h"

//int8 Light_Init(void)
//{
//    return DRIVER_OK;
//}

int8 Light_Set(u8 light_id, u8 enabled)
{
    light_id = light_id; /* C51-compatible unused parameters. */
    enabled = enabled;
    return 1;
}

int8 Light_Init(void){
	P2_MODE_OUT_PP(GPIO_Pin_LOW | GPIO_Pin_6 | GPIO_Pin_7);
	P1_MODE_OUT_PP(GPIO_Pin_4 | GPIO_Pin_5)
	LED_SW = 0;
	return 1;
}

void All_Dark(void){
	LED_F = LED1 = LED2 = LED3 = LED4 = LED5 = LED6 = LED7 = LED8 = 1;
}

void Front_Lights(void){
	All_Dark();
	LED_F = 0;
}

void Dancing_Lights(void){
	while(1){
		LED_F = LED1 = LED2 = LED3 = LED4 = LED5 = LED6 = LED7 = LED8 = 0;
		//os_wait2(K_TMO, 4);
		delay_ms(150);
		LED_SW = LED_F = LED1 = LED2 = LED3 = LED4 = LED5 = LED6 = LED7 = LED8 = 1;
		//os_wait2(K_TMO, 4);
		delay_ms(150);
	}
}
