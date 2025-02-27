#ifndef __HC_SR04_H
#define __HC_SR04_H

//#include "main.h"
#include "tim.h"
#include "DHT11.h"
#include "stdbool.h"
#include "motor.h"

//float hc_sr04_detect(void);
float SR04_GetData(void);
void hc_sr04_proc(float *dis,uint8_t *check_flag,uint16_t *bond_dis);
#endif



