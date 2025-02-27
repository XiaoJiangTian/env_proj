#ifndef __OLED_H
#define __OLED_H

#include "DHT11.h"
#include "interrupt.h"
#include "stdbool.h"
#include "motor.h"
#include "myadc.h"
#include "stdio.h"
#include "mykey.h"

enum door_state
{
	DOOR_CLOSE=0,
	DOOR_OPEN,
};
extern enum door_state door_state_1;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

//void screen_proc(struct key_st *key,struct dht11_st*dht11,uint16_t *bond_temp,uint16_t *bond_dis,uint16_t *bond_light,bool *flag,float *dis,uint8_t *check_flag,bool *buzzer_flag,uint8_t*Dread,uint8_t *dis_index,uint8_t *num);

void screen_proc(bool *auto_manual_flag,struct key_st *key,struct dht11_st*dht11,uint16_t *bond_temp,uint16_t *bond_dis,uint16_t *bond_light,bool *flag,float *dis,uint8_t *check_flag,bool *buzzer_flag,uint8_t*Dread,uint8_t *dis_index,uint8_t *num);
void screen_flash(struct key_st *key);

#endif
