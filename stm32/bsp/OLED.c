#include "OLED_Font.h"
#include "main.h"
#include "gpio.h"
#include "OLED.h"
//#include "main.c"
/*引脚配置*/
#define OLED_W_SCL(x)		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,x)
#define OLED_W_SDA(x)		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,x)


//GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
//GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

/*引脚初始化*/
void OLED_I2C_Init(void)
{
	
//标准库的做法
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//	
//	GPIO_InitTypeDef GPIO_InitStructure;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED显示一个字符
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
	}
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
  * @brief  OLED显示数字（十进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~4294967295
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十进制，带符号数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：-2147483648~2147483647
  * @param  Length 要显示数字的长度，范围：1~10
  * @retval 无
  */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
  * @brief  OLED显示数字（十六进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
  * @param  Length 要显示数字的长度，范围：1~8
  * @retval 无
  */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
  * @brief  OLED显示数字（二进制，正数）
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
  * @param  Length 要显示数字的长度，范围：1~16
  * @retval 无
  */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}


enum door_state door_state_1;
//void screen_proc(bool *auto_manual_flag,struct key_st *key,struct dht11_st*dht11,uint16_t *bond_temp,uint16_t *bond_dis,uint16_t *bond_light,bool *flag,float *dis,uint8_t *check_flag,bool *buzzer_flag,uint8_t*Dread,uint8_t *dis_index,uint8_t *num)
//{
//	char test[12];
//	//模式的区分
//	if((*auto_manual_flag)==0)
//	{
//		//sprintf(test,"A");
//		OLED_ShowString(1,15,"A");
//	}
//	else 
//	{
//		//sprintf(test,"M");
//		OLED_ShowString(1,15,"M");
//	}
//	
//	
//	//界面1：环境参数
//	if(key[2].key2_screen==0)
//	{
//		//printf("page1\r\n");
//		//sprintf(test,"  Env-Para  ");
//		OLED_ShowString(1,1,"Env-Para-1  ");	
//		sprintf(test,"t:%.1fC h:%.1f%%",dht11->temp,dht11->humi);
//		//printf("error1-1\r\n");
//		OLED_ShowString(4,1,test);
//		
//		if(door_state_1==DOOR_OPEN)
//		{
//			sprintf(test,"door:o");
//		}
//		else 
//		{
//			sprintf(test,"door:c");
//		}
//		OLED_ShowString(2,9,test);
//		//printf("error1-2\r\n");
//		
//		
//		if((*auto_manual_flag)==0)
//		{
//			//电机
//			//printf("error 1\r\n");
//			if(dht11->temp<(*bond_temp))
//			{
//				sprintf(test,"Motor:St");
//			}
//			else if(dht11->temp>=(*bond_temp) && dht11->temp<35)
//			{
//				sprintf(test,"Motor:On");
//			}
//			else
//			{
//				sprintf(test,"Motor:Fa");
//			}
//			OLED_ShowString(3,1,test);
//			//led
//			//printf("error2\r\n");
//			if((*flag)==1)
//			{
//				sprintf(test,"led:off");
//			}
//			else
//			{
//				sprintf(test,"led:on ");
//			}
//			OLED_ShowString(2,1,test);
//		}
//		else //蓝牙控制
//		{
//			if(motor_state_1 == MOTOR_OFF)
//			{
//				sprintf(test,"Motor:St");
//			}
//			else if(motor_state_1 == MOTOR_ON_S)
//			{
//				sprintf(test,"Motor:On");
//			}
//			else 
//			{
//				sprintf(test,"Motor:Fa");
//			}
//			OLED_ShowString(3,1,test);
//			
//			
//			if(led_state_1==LED_OFF)
//			{
//				sprintf(test,"led:off");
//			}
//			else
//			{
//				sprintf(test,"led:on ");
//			}
//			OLED_ShowString(2,1,test);
//		}
//	}
//	//环境参数2
//	
//	else if(key[2].key2_screen==1)
//	{
//		//printf("page2\r\n");
//		OLED_ShowString(1,1,"Env-Para-2  ");	
//		
//		if(rain_state_1==RAIN_STOP)
//		{
//			OLED_ShowString(3,1,"Rain:Stop ");
//		}
//		else if(rain_state_1==RAIN_SMALL)
//		{
//			OLED_ShowString(3,1,"Rain:Small");
//		}
//		else 
//		{
//			OLED_ShowString(3,1,"Rain:Heavy");
//		}
//		
//		sprintf(test,"smoke_value:wait");
//		OLED_ShowString(4,1,test);
//	}
//	//界面3：安全护卫

//	else if(key[2].key2_screen==2)
//	{
//		//printf("page3\r\n");
//		//sprintf(test,"Safe-Guard");
//		OLED_ShowString(1,1,"Safe-Guard");
//		sprintf(test,"Hc-Dis:%.2f  ",(*dis));
//		OLED_ShowString(4,1,test);
//		
//		if((*dis)<(*bond_dis)&&(*check_flag)==motor_stop)
//		{
//			//sprintf(test,"Status:Warn");
//			OLED_ShowString(2,1,"Status:Warn");
//			
//			//sprintf(test,"Bz:On ");
//			OLED_ShowString(3,1,"Bz:On ");
//			//处于警告状态时，会报警
//			(*buzzer_flag) = 1;
//		}
//		else if((*dis)>=(*bond_dis) && (*check_flag) == motor_check)
//		{
//			//sprintf(test,"Status:Run ");
//			OLED_ShowString(2,1,"Status:Run ");
//			
//			//sprintf(test,"Bz:Off");
//			OLED_ShowString(3,1,"Bz:Off");
//			(*buzzer_flag) = 0;
//		}
//		else if((*check_flag)==motor_reload)
//		{
//			//sprintf(test,"Status:Stop");
//			OLED_ShowString(2,1,"Status:Stop");
//			
//			//sprintf(test,"Bz:Off");
//			OLED_ShowString(3,1,"Bz:Off");
//			(*buzzer_flag) = 0;
//		}
//		
//		
//	}
//	//界面4：限制参数界面

//	else if(key[2].key2_screen==3)
//	{
//		//printf("page4\r\n");
//		static uint8_t old_select=0;
//		
//		
//		//sprintf(test,"Bond-Para");
//		OLED_ShowString(1,1,"Bond-Para");
//		
//		sprintf(test,"Dis:%dcm",(*bond_dis));
//		OLED_ShowString(2,1,test);
//		
//		sprintf(test,"Tp:%dC",(*bond_temp));
//		OLED_ShowString(3,1,test);
////		printf("oled:%d\r\n",(*bond_light));
////		printf("addr2:%d\r\n",bond_light);
//		sprintf(test,"Lg:%d",(*bond_light));
//		OLED_ShowString(4,1,test);
//		
//		sprintf(test,"<-");
//		switch(key[3].key3_select)
//		{
//			case 0:
//			{
//				if(old_select!=key[3].key3_select)
//				{
//					OLED_ShowString(3,11,"  ");
//					OLED_ShowString(4,11,"  ");
//					old_select = key[3].key3_select;
//				}
//				OLED_ShowString(2,11,test);

//			}
//			break;
//			
//			case 1:
//			{
//				if(old_select!=key[3].key3_select)
//				{
//					OLED_ShowString(2,11,"  ");
//					OLED_ShowString(4,11,"  ");
//					old_select = key[3].key3_select;
//				}
//				OLED_ShowString(3,11,test);
//			}
//			break;
//			
//			case 2:
//			{
//				if(old_select!=key[3].key3_select)
//				{
//					OLED_ShowString(2,11,"  ");
//					OLED_ShowString(3,11,"  ");
//					old_select = key[3].key3_select;
//				}
//				OLED_ShowString(4,11,test);
//			}
//			break;
//		}
//	}
//	else if(key[2].key2_screen==4)
//	{
//		//printf("page5\r\n");
//		sprintf(test,"Store-Value  %d",*num);
//		OLED_ShowString(1,1,test);
//		sprintf(test,"v%d:%dC  v%d:%d%%",(*dis_index),Dread[(*dis_index)],(*dis_index)+1,Dread[(*dis_index)+1]);
//		OLED_ShowString(2,1,test);
//		
//		sprintf(test,"v%d:%dC  v%d:%d%%",(*dis_index)+2,Dread[(*dis_index)+2],(*dis_index)+3,Dread[(*dis_index)+3]);
//		OLED_ShowString(3,1,test);
//		
//		sprintf(test,"v%d:%dC  v%d:%d%%",(*dis_index)+4,Dread[(*dis_index)+4],(*dis_index)+5,Dread[(*dis_index)+5]);
//		OLED_ShowString(4,1,test);
//	}
//}

void displayDoorState(enum door_state state, int x, int y);
void displayMotorState(enum motor_state state, int x, int y);


void displayLedState(enum led_state state, int x, int y);
void displayRainState(enum rain_state state, int x, int y);


//void displayBuzzerState(BuzzerState state, int x, int y);
void displayBondParameters(uint16_t bond_dis, uint16_t bond_temp, uint16_t bond_light, int x, int y, uint8_t select);


void screen_proc(bool *auto_manual_flag, struct key_st *key, struct dht11_st *dht11, uint16_t *bond_temp, uint16_t *bond_dis, uint16_t *bond_light, bool *flag, float *dis, uint8_t *check_flag, bool *buzzer_flag, uint8_t *Dread, uint8_t *dis_index, uint8_t *num) {
    char test[12];

    // 模式的区分，三目运算符来替代if else
    OLED_ShowString(1, 15, (*auto_manual_flag) ? "M" : "A");

    // 界面1：环境参数
    if (key[2].key2_screen == 0) {
        OLED_ShowString(1, 1, "Env-Para-1  ");
        sprintf(test, "t:%.1fC h:%.1f%%", dht11->temp, dht11->humi);
        OLED_ShowString(4, 1, test);
        
        displayDoorState(door_state_1, 2, 9);
        
        if (*auto_manual_flag == 0) {
					//可以封装成同一个函数，只要执行的操作一样，入口条件不一样可以通过传入的参数来调整。
            displayMotorState((dht11->temp < *bond_temp) ? MOTOR_OFF : (dht11->temp < 35) ? MOTOR_ON_S : MOTOR_ON_F, 3, 1);//两个三目运算符
            displayLedState((*flag) ? LED_OFF : LED_ON, 2, 1);
        } else {
            displayMotorState(motor_state_1, 3, 1);
            displayLedState(led_state_1, 2, 1);
        }
    }

    // 界面2：环境参数2
    else if (key[2].key2_screen == 1) {
        OLED_ShowString(1, 1, "Env-Para-2  ");
        displayRainState(rain_state_1, 3, 1);
        sprintf(test, "smoke_value:wait");
        OLED_ShowString(4, 1, test);
    }

    // 界面3：安全护卫
    else if (key[2].key2_screen == 2) {
        OLED_ShowString(1, 1, "Safe-Guard");
        sprintf(test, "Hc-Dis:%.2f  ", *dis);
        OLED_ShowString(4, 1, test);

        if (*dis < *bond_dis && *check_flag == motor_stop) {
            OLED_ShowString(2, 1, "Status:Warn");
            OLED_ShowString(3, 1, "Bz:On ");
            *buzzer_flag = 1;
        } else if (*dis >= *bond_dis && *check_flag == motor_check) {
            OLED_ShowString(2, 1, "Status:Run ");
            OLED_ShowString(3, 1, "Bz:Off");
            *buzzer_flag = 0;
        } else if (*check_flag == motor_reload) {
            OLED_ShowString(2, 1, "Status:Stop");
            OLED_ShowString(3, 1, "Bz:Off");
            *buzzer_flag = 0;
        }
    }

    // 界面4：限制参数界面
    else if (key[2].key2_screen == 3) {
        static uint8_t old_select = 0;

        OLED_ShowString(1, 1, "Bond-Para");
        displayBondParameters(*bond_dis, *bond_temp, *bond_light, 2, 1, key[3].key3_select);

        // 更新选中状态
        if (old_select !=key[3].key3_select) {
            switch (key[3].key3_select) {
                case 0:
                    OLED_ShowString(3, 11, "  ");
                    OLED_ShowString(4, 11, "  ");
                    break;
                case 1:
                    OLED_ShowString(2, 11, "  ");
                    OLED_ShowString(4, 11, "  ");
                    break;
                case 2:
                    OLED_ShowString(2, 11, "  ");
                    OLED_ShowString(3, 11, "  ");
                    break;
            }
            old_select = key[3].key3_select;
        }
        sprintf(test, "<-");
        OLED_ShowString(2 + key[3].key3_select, 11, test);
    }

    // 界面5：存储值
    else if (key[2].key2_screen == 4) {
        sprintf(test, "Store-Value  %d", *num);
        OLED_ShowString(1, 1, test);
        sprintf(test, "v%d:%dC  v%d:%d%%", *dis_index, Dread[*dis_index], *dis_index + 1, Dread[*dis_index + 1]);
        OLED_ShowString(2, 1, test);
        
        sprintf(test, "v%d:%dC  v%d:%d%%", *dis_index + 2, Dread[*dis_index + 2], *dis_index + 3, Dread[*dis_index + 3]);
        OLED_ShowString(3, 1, test);
        
        sprintf(test, "v%d:%dC  v%d:%d%%", *dis_index + 4, Dread[*dis_index + 4], *dis_index + 5, Dread[*dis_index + 5]);
        OLED_ShowString(4, 1, test);
    }
}

void screen_flash(struct key_st *key)
{
	static uint8_t old_screen=0;
	if(old_screen!=key[2].key2_screen)
	{
		my_oled_clear();
//		OLED_ShowString(1,1,"                ");
//		OLED_ShowString(2,1,"                ");
//		OLED_ShowString(3,1,"                ");
//		OLED_ShowString(4,1,"                ");
		old_screen = key[2].key2_screen;
	}
}

void displayDoorState(enum door_state state, int x, int y) {
    char *str = (state == DOOR_OPEN) ? "door:o" : "door:c";
    OLED_ShowString(x, y, str);
}

void displayMotorState(enum motor_state state, int x, int y) {
    char *str = (state == MOTOR_OFF) ? "Motor:St" : (state == MOTOR_ON_S) ? "Motor:On" : "Motor:Fa";
    OLED_ShowString(x, y, str);
}

void displayLedState(enum led_state state, int x, int y) {
    char *str = (state == LED_OFF) ? "led:off" : "led:on ";
    OLED_ShowString(x, y, str);
}

void displayRainState(enum rain_state state, int x, int y) {
    char *str = (state == RAIN_STOP) ? "Rain:Stop " : (state == RAIN_SMALL) ? "Rain:Small" : "Rain:Heavy";
    OLED_ShowString(x, y, str);
}

//void displayBuzzerState(BuzzerState state, int x, int y) {
//    const char *str = (state == BZ_OFF) ? "Bz:Off" : "Bz:On ";
//    OLED_ShowString(x, y, str);
//}

void displayBondParameters(uint16_t bond_dis, uint16_t bond_temp, uint16_t bond_light, int x, int y, uint8_t select) {
    char test[20];
		sprintf(test, "Dis:%dcm", bond_dis);
    OLED_ShowString(x, y, test);
    
    sprintf(test, "Tp:%dC", bond_temp);
    OLED_ShowString(x + 1, y, test);
    
    sprintf(test, "Lg:%d", bond_light);
    OLED_ShowString(x + 2, y, test);
    
    char arrow[3] = "<-";
    switch (select) {
        case 0:
            OLED_ShowString(x, 11, arrow);
            break;
        case 1:
            OLED_ShowString(x + 1, 11, arrow);
            break;
        case 2:
            OLED_ShowString(x + 2, 11, arrow);
            break;
    }
}
