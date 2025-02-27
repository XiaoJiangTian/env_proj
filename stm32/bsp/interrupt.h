#ifndef __INTERRUPT_H
#define __INTERRUPT_H


//#include "main.h"
#include "stdbool.h"
#include "tim.h"
#include "stdio.h"
struct key_st
{
	bool key_value;
	bool key_short_press;
	bool key_long_press;
	bool key_double_press;
	
	uint16_t press_count;
	uint16_t release_count;
	uint16_t single_state;
	uint16_t double_state;
	uint16_t double_count;
	
	uint16_t key2_screen;
	uint8_t key3_select;
};

extern char rec_buf[20];
extern uint8_t n;
extern uint8_t rec_flag;
extern struct key_st key[4];
extern bool ali_trans_flag;
#endif

