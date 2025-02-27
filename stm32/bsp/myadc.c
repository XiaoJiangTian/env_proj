#include "myadc.h"

enum led_state led_state_1;
enum rain_state rain_state_1;

void adc_proc(bool *flag,bool *auto_manual_flag,uint16_t*bond_light)
{
	uint16_t num[2];
	for(int i=0;i<2;i++)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1,50);//等待转换完成
		num[i]=HAL_ADC_GetValue(&hadc1);
	}
	//HAL_ADCEx_Calibration_Start(&hadc1);
	
	(*flag) = num[0]>(*bond_light)?false:true;
	//printf("num:%d\r\n",num[0]);
	//smoke_value=num[2];
	
	if(num[1]>3800)
	{
		rain_state_1 = RAIN_STOP;
	}
	else if(num[1]<=3800 && num[1]>1200)
	{
		rain_state_1 = RAIN_SMALL;
	}
	else 
	{
		rain_state_1 = RAIN_HEAVY;
	}
	
	//printf("water value:%d\r\n",num[1]);
	
	if((*auto_manual_flag)==false)
	{
		if((*flag)==true)
		{
			
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);//flag为0对应暗，灯点亮
		}
	}
	//这里同样是因为控制方式不同需要分开讨论
	else 
	{
		if(led_state_1==LED_ON)
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
		}
		else 
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
		}
	}
}

