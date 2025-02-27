#ifndef __MYUART_H
#define __MYUART_H
//#include "main.h"
#include "interrupt.h"
#include "OLED.h"
#include "string.h"
#include "stdlib.h"

void uartProc(bool *autoManualFlag, uint8_t *num, uint8_t *dread, struct dht11_st *dht11, bool *flag, uint8_t *checkFlag);
#endif

