#ifndef __MYKEY_H
#define __MYKEY_H

//#include "main.h"
#include "interrupt.h"
#include "OLED.h"
void key_proc(struct key_st *key,uint8_t *check_flag,uint8_t *dis_index);
#endif
