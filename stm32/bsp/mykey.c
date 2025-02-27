#include "mykey.h"

void my_oled_clear();

void key_proc(struct key_st *key,uint8_t *check_flag,uint8_t *dis_index)
{
	static uint8_t door_state=5;
	for(uint8_t i=0;i<4;i++)
	{
		if(key[i].key_short_press==1)
		{
			key[i].key_short_press=0;
			switch(i)
			{
				//开关门
				case 0:
				{
					door_state = door_state+20>25?5:door_state+20;
					__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,door_state);
					door_state_1 = door_state_1==DOOR_CLOSE?DOOR_OPEN:DOOR_CLOSE;
				}
				break;
				
				//启动或关闭超声波舵机
				case 1:
				{
					(*check_flag) = (*check_flag)>0?motor_reload:motor_check;
					//OLED_ShowString(1,11,"on");
				}
				break;
				//用于切换屏幕使用
				case 2:
				{
					key[2].key2_screen=key[2].key2_screen+1>4?0:key[2].key2_screen+1;
					if(key[2].key2_screen!=3 && key[3].key3_select!=0)
					{
						key[3].key3_select=0;
					}
					if(key[2].key2_screen!=4 && (*dis_index)!=0)
					{
						(*dis_index)=0;
					}
				}
				break;
				//界面4切换更改项，界面5切换存储数据
				case 3:
				{
					if(key[2].key2_screen==3)
					{
						key[3].key3_select = key[3].key3_select+1>2?0:key[3].key3_select+1;
					}
					else if(key[2].key2_screen==4)
					{
						(*dis_index) = (*dis_index)+6>23?0:(*dis_index)+6;
						if((*dis_index)==0)
						{
							//OLED_Clear();
							my_oled_clear();
						}
					}
				}
				break;
			}
		}
	}
}

void my_oled_clear()
{
	OLED_ShowString(1,1,"                ");
	OLED_ShowString(2,1,"                ");
	OLED_ShowString(3,1,"                ");
	OLED_ShowString(4,1,"                ");
}
