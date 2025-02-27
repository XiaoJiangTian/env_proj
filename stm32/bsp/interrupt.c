#include "interrupt.h"

struct key_st key[4]={0,0,0,0};


uint16_t count500ms=0,count30ms=0,count5s=0,count100ms=0,count15s;
uint8_t hc_sr04_motor=5;
int8_t step=1;

uint8_t n=0;
uint8_t rec_flag=0;
char rec_buf[20];

bool ali_trans_flag;



//bool buzzer_value=0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		if(htim->Instance == TIM1)
		{
			key[0].key_value = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_10);
			key[1].key_value = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12);
			key[2].key_value = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14);
			key[3].key_value = HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_8);
			for(int i=0;i<4;i++)
			{	
				switch(key[i].single_state)
				{
					case 0:
					{
						if(key[i].key_value==1 )//要在按键检测里就对按键失效，在按键实现里置失效还是会执行一次的
						{
							key[i].press_count++;
							if(key[i].press_count>=10)
							{
								key[i].single_state=1;
							}
						}
					}
					break;
					
					case 1:
					{
						if(key[i].key_value==1)
						{
							key[i].press_count++;
						}
						else if(key[i].key_value==0)
						{
							key[i].single_state = 2;
							key[i].release_count++;
						}
					}
					break;
					
					
					case 2:
					{
						if(key[i].key_value==0)
						{
							key[i].release_count++;
							if(key[i].release_count>=10)
							{
								if(key[i].press_count>=2000)
								{
									key[i].key_long_press=1;
									key[i].single_state=0;
									key[i].release_count=0;
									key[i].press_count=0;
								}
								else
								{
									switch(key[i].double_state)
									{
										case 0:
										{
											key[i].double_state=1;
											key[i].double_count=0;
										}
										break;
										
										case 1:
										{
											key[i].key_double_press=1;
											key[i].double_state=0;
										}
										break;
									}
									key[i].single_state=0;
									key[i].press_count=0;
									key[i].release_count=0;
								}
							}
						}
					}
					break;
				}
				
				if(key[i].double_state==1)
				{
					key[i].double_count++;
					if(key[i].double_count>=300)
					{
						key[i].double_count=0;
						key[i].double_state=0;
						key[i].key_short_press=1;
					}
				}
			}
			
			
			//蜂鸣器报警
			if(buzzer_flag)
			{
				count500ms++;
				if(count500ms>=500)
				{
					buzzer_flag=!buzzer_flag;
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,buzzer_flag); //间隔响
					count500ms = 0;
				}
			}
			else 
			{
				count500ms = 0;
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0,GPIO_PIN_SET); //高电平不响
			}
			
			
			//超声波舵机
			if(check_flag==1)//check
			{
				count30ms++;
				if(count30ms>=30)
				{
					count30ms=0;
					
					//这一坨可以考虑移走
					if(hc_sr04_motor+step>25 ||hc_sr04_motor+step<5)
					{
						step = -step;
					}
					hc_sr04_motor += step;
					//printf("motor_value:%d\r\n",hc_sr04_motor);
					__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,hc_sr04_motor);//舵机巡视状态
				}
			}
			else if(check_flag==2)//do nothing
			{
				count30ms=0;
			}
			else
			{
				hc_sr04_motor=5;
				__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,hc_sr04_motor);//舵机复位状态
				count30ms=0;
			}
			
			//5s
			count5s++;
			if(count5s>=5000)
			{
				flag_5s=1;
				count5s=0;
			}
			
			if(ali_trans_flag==0)
			{
				count15s++;
			}
			if(count15s>=30000)//半分钟上传一次
			{
				ali_trans_flag=1;
				count15s=0;
			}
			
		
		}
		
		if(htim->Instance == TIM7)
		{
			rec_flag=1;
			HAL_TIM_Base_Stop_IT(&htim7);
		}

}



void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
	if(GPIO_Pin == GPIO_PIN_7)
	{
			if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_9) == GPIO_PIN_RESET)
			{
				//	--Encoder_Count;
					switch(key[3].key3_select)
					{
						case 0:
						{
							if(bond_dis>8)
							{
								bond_dis--;
							}
							
						}	
						break;
							
						case 1:
						{
							if(bond_temp>20)
							{
								bond_temp--;
							}
						}
						break;
							
						case 2:
						{
							if(bond_light>2000)
							{
								bond_light--;
							}
						}
						break;
					}
			}
	}
	else if(GPIO_Pin == GPIO_PIN_9)
	{
			if(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_7) == GPIO_PIN_RESET)
			{
				switch(key[3].key3_select)
				{
						case 0:
						{
							if(bond_dis<30)
							{
								bond_dis++;
							}
							
						}	
						break;
							
						case 1:
						{
							if(bond_temp<35)
							{
								bond_temp++;
							}
						}
						break;
							
						case 2:
						{
							if(bond_light<3300)
							{
								bond_light++;
							}
						}
						break;
				}	
			}
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		__HAL_TIM_SetCounter(&htim7,0);
		if(n==0)
		{
			__HAL_TIM_CLEAR_FLAG(&htim7,TIM_FLAG_UPDATE);
			HAL_TIM_Base_Start_IT(&htim7);
		}	
		rec_buf[n++]=rec;
		HAL_UART_Receive_IT(huart,&rec,1);
	}
}

