#ifndef __DHT11_H
#define __DHT11_H
 
 

//#include "main.h"
#include "tim.h"
#include "stdbool.h"

#define DHT11_DQ_OUT_HIGH HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET)
#define DHT11_DQ_OUT_LOW 	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET)
#define DHT11_DQ_IN	 HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)
 
//IO方向设置
void DS18B20_IO_IN(void);
void DS18B20_IO_OUT(void);
	
uint8_t DHT11_Init(void);//初始化DHT11
uint8_t DHT11_Read_Data(uint8_t *tempi,uint8_t *tempf,uint8_t *humi,uint8_t*humf);
uint8_t DHT11_Read_Byte(void);//读出一个字节
uint8_t DHT11_Read_Bit(void);//读出一个位
uint8_t DHT11_Check(void);//检测是否存在DHT11
void DHT11_Rst(void);//复位DHT11  
void delay_us(uint16_t us);

//
struct dht11_st
{
	float temp,humi;
};

void dht11_measure(struct dht11_st *dht11,uint8_t *Dwrite_t,bool *flag_5s_t,uint8_t *write_index_t,uint8_t *count_write_t);

#endif 
