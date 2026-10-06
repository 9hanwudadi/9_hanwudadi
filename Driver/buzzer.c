#include "buzzer.h"

//int8 Buzzer_Init(void)
//{
//    return DRIVER_OK;
//}

u16 code hz[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976, 2093};

u16 code FREQS[] = {
	523 * 1, 587 * 1, 659 * 1, 698 * 1, 784 * 1, 880 * 1, 988 * 1, 
	523 * 2, 587 * 2, 659 * 2, 698 * 2, 784 * 2, 880 * 2, 988 * 2, 
	523 * 4, 587 * 4, 659 * 4, 698 * 4, 784 * 4, 880 * 4, 988 * 4, 
	523 * 8, 587 * 8, 659 * 8, 698 * 8, 784 * 8, 880 * 8, 988 * 8, 
};

u8 code notes[] = {
	H1,N1,N5,L5,	H2,N2,N5,L5,	H1,N1,N5,L5,	H3,N3,N5,L5,
	H1,N1,N5,L5,	H2,N2,N5,L5,	H1,N1,N5,L5,	H5,N5,N5,L5,
	H1,N1,N5,L5,	H2,N2,N5,L5,	H1,N1,N5,L5,	H3,N3,N5,L5,
	H1,N1,N5,L5,	H2,N2,N5,L5,	H1,N1,N5,L5,	H5,N5,N5,L5
	
	
};

u8 code durations[] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

static void GPIO_config(void) {
    GPIO_InitTypeDef	GPIO_InitStructure;		
    GPIO_InitStructure.Pin  = GPIO_Pin_0;		
    GPIO_InitStructure.Mode = GPIO_OUT_PP;	    
    GPIO_Inilize(GPIO_P0, &GPIO_InitStructure);
}

void	PWM_config(u16 hz_output)
{
	PWMx_InitDefine		PWMx_InitStructure;
    
    u16 period = (MAIN_Fosc / hz_output);
	
	PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	
	PWMx_InitStructure.PWM_Duty   	 	= 0;	            
	PWMx_InitStructure.PWM_EnoSelect    = ENO5P;			
	PWM_Configuration(PWM5, &PWMx_InitStructure);			

	PWMx_InitStructure.PWM_Period   = period - 1;			
	PWMx_InitStructure.PWM_DeadTime = 0;					
	PWMx_InitStructure.PWM_MainOutEnable= ENABLE;			
	PWMx_InitStructure.PWM_CEN_Enable   = ENABLE;			
	PWM_Configuration(PWMB, &PWMx_InitStructure);			

	PWM5_SW(PWM5_SW_P00);					

	NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
}

int8 Buzzer_Init(void){
    EAXSFR(); 
    GPIO_config();
    PWM_config(1000);
	return 1;
}


void Buzzer_play(u16 hz_value){
    u16 period = (MAIN_Fosc / hz_value);
    u16 duty = period * 0.5f; 
    PWMB_AutoReload(period - 1);	
    PWMB_Duty5(duty);
    PWMB_CC5E_Enable();
}

void Buzzer_beep(u16 tone){ 
    Buzzer_play(FREQS[tone - 1]);
}

void Buzzer_stop(void){
    PWMB_CC5E_Disable();
}

void Dancing_Buzzer(void){
	u8 i, len;
	len = sizeof(notes) / sizeof(notes[0]);
    for(i = 0; i < len; i++){
        // 按照指定音调输出
        Buzzer_beep(notes[i]);
        
        // 每个音调后, 做休眠
        delay_ms(150);
        
        // 音调之间做短暂间隔
        Buzzer_stop();
        delay_ms(50);
    }
    
    Buzzer_stop();
}



