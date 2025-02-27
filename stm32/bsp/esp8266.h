#ifndef __ESP8266_H
#define __ESP8266_H

//#include "main.h"
#include "usart.h"
#include "string.h"
#include "cJSON.h"
#include "stdio.h"
#include "stdlib.h"
#include "oled.h"
void ESP8266_Init(void);
void Ali_Yun_Init(void);
void Ali_Yun_Send(struct ali_state_st *ali_state_st);
void ESP8266_Clear(void);
#endif

