#ifndef __MYADC_H
#define __MYADC_H


//#include "main.h"
#include "adc.h"
#include "stdbool.h"

enum led_state
{
	LED_OFF=0,
	LED_ON,
};

typedef enum rain_state{
	RAIN_STOP=0,
	RAIN_SMALL,
	RAIN_HEAVY,
}MY_RAIN_STATE;


extern enum led_state led_state_1;
extern enum rain_state rain_state_1;

void my_oled_clear();
void adc_proc(bool *flag,bool *auto_manual_flag,uint16_t*bond_light);
#endif 
