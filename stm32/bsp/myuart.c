#include "myuart.h"


void printUsage();
void printManualUsage();
void printData(uint8_t num, uint8_t *dread);
void printSystemState(struct dht11_st *dht11, bool autoManualFlag, bool flag);
void setLimits( char *buf);
void processCommand(const char *buf, bool *autoManualFlag, uint8_t *num, uint8_t *dread, struct dht11_st *dht11, bool *flag, uint8_t *checkFlag);


void uartProc(bool *autoManualFlag, uint8_t *num, uint8_t *dread, struct dht11_st *dht11, bool *flag, uint8_t *checkFlag)
{
    if (rec_flag == 1) {
        rec_flag = 0;
        rec_buf[n] = '\0';
        n = 0;

        // 处理命令
        if (strcmp(rec_buf, "help") == 0) {
            printUsage();
        } else if (strcmp(rec_buf, "manual") == 0) {
            (*autoManualFlag) = true;
            printManualUsage();
        } else if (strcmp(rec_buf, "auto") == 0) {
            (*autoManualFlag) = false;
        } else if (strcmp(rec_buf, "request_data") == 0) {
            printData(*num, dread);
        } else if (strcmp(rec_buf, "show_sys_state") == 0) {
            printSystemState(dht11, *autoManualFlag, *flag);
        } else if (strncmp(rec_buf, "limit:", 6) == 0) {
            setLimits(rec_buf);
        } else if (strncmp(rec_buf, "motor->", 7) == 0 || strncmp(rec_buf, "door-->", 7) == 0 ||
                   strncmp(rec_buf, "sonic->", 7) == 0 || strncmp(rec_buf, "led--->", 7) == 0) {
            if (!(*autoManualFlag)) {
                printf("Error: Auto mode now (need: manual)\r\n");
            } else {
                processCommand(rec_buf, autoManualFlag, num, dread, dht11, flag, checkFlag);
            }
        }
    }
}

void printUsage() {
    // Function to print usage instructions
    printf("------------------------------\r\n");
    printf("Usage\r\n");
    printf("Mode:\r\n");
    printf("1. auto: default.\r\n");
    printf("2. manual: by command -> manual.\r\n");
    printf("Command (all the time):\r\n");
    printf("1. Command for data: request_data\r\n");
    printf("1. Command for state: show_sys_state\r\n");
    printf("------------------------------\r\n");
}

void printManualUsage() {
    // Function to print manual mode usage instructions
    printf("Usage (manual mode):\r\n");
    printf("Command for motor: motor -> motor_on_speed: s/f or motor_off\r\n");
    printf("Command for door: door --> door_open or door_close\r\n");
    printf("Command for sonic: sonic -> s_check or s_reload\r\n");
    printf("Command for LED: led ---> led_on or led_off\r\n");
    printf("Command for limits: limit: Dis(8-30): xx or Tp(20-35): xx or Lg(2000-3300): xx\r\n");
}

void printData(uint8_t num, uint8_t *dread) {
    if (num >= 1) {
        for (int i = 0; i < 24; i += 2) {
            printf("[%d] -> Temperature: %d°C, Humidity: %d%%\r\n", i + 1, dread[i], dread[i + 1]);
        }
    } else {
        printf("Not recording now!\r\n");
    }
}

void printSystemState(struct dht11_st *dht11, bool autoManualFlag, bool flag) {
    printf("The slave state:\r\n");
    if (door_state_1 == DOOR_CLOSE) {
        printf("Door State: Closed\r\n");
    } else {
        printf("Door State: Open\r\n");
    }

    printf("Temperature: %.2f\r\n", dht11->temp);
    printf("Humidity: %.2f\r\n", dht11->humi);

    if (!autoManualFlag) {
        if (dht11->temp < bond_temp) {
            printf("Motor State: Stop\r\n");
        } else if (dht11->temp >= bond_temp && dht11->temp < 35) {
            printf("Motor State: Slow\r\n");
        } else {
            printf("Motor State: Fast\r\n");
        }

        if (flag) {
            printf("LED State: Off\r\n");
        } else {
            printf("LED State: On\r\n");
        }
    } else {
        if (motor_state_1 == MOTOR_OFF) {
            printf("Motor State: Stop\r\n");
        } else if (motor_state_1 == MOTOR_ON_S) {
            printf("Motor State: Slow\r\n");
        } else {
            printf("Motor State: Fast\r\n");
        }

        if (led_state_1 == LED_OFF) {
            printf("LED State: Off\r\n");
        } else {
            printf("LED State: On\r\n");
        }
    }
}

void setLimits( char *buf) {
    char *tempLimit = NULL;
    uint16_t tempNum = 0;
    strtok(buf, ":");
    tempLimit = strtok(NULL, ":");
    
    if (strncmp(tempLimit, "Dis", 3) == 0) {
        tempLimit = strtok(NULL, ":");
        tempNum = atoi(tempLimit);
        if (tempNum >= 8 && tempNum <= 30) {
            bond_dis = tempNum;
        } else {
            printf("Wrong distance value\r\n");
        }
    } else if (strncmp(tempLimit, "Tp", 2) == 0) {
        tempLimit = strtok(NULL, ":");
        tempNum = atoi(tempLimit);
        if (tempNum >= 20 && tempNum <= 35) {
            bond_temp = tempNum;
        } else {
            printf("Wrong temperature value\r\n");
        }
    } else if (strncmp(tempLimit, "Lg", 2) == 0) {
        tempLimit = strtok(NULL, ":");
        tempNum = atoi(tempLimit);
        if (tempNum >= 2000 && tempNum <= 3300) {
            bond_light = tempNum;
        } else {
            printf("Wrong light value\r\n");
        }
    } else {
        printf("Wrong limit element selected\r\n");
        printf("Temp: %s\r\n", tempLimit);
    }
}

void processCommand(const char *buf, bool *autoManualFlag, uint8_t *num, uint8_t *dread, struct dht11_st *dht11, bool *flag, uint8_t *checkFlag) {
    if (strcmp(buf, "led--->led_on") == 0) {
        led_state_1 = LED_ON;
    } else if (strcmp(buf, "led--->led_off") == 0) {
        led_state_1 = LED_OFF;
    }

    if (strcmp(buf, "motor->motor_on_speed:s") == 0) {
        motor_state_1 = MOTOR_ON_S;
    } else if (strcmp(buf, "motor->motor_on_speed:f") == 0) {
        motor_state_1 = MOTOR_ON_F;
    } else if (strcmp(buf, "motor->motor_off") == 0) {
        motor_state_1 = MOTOR_OFF;
    }

    if (strcmp(buf, "door-->door_open") == 0) {
        door_state_1 = DOOR_OPEN;
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 25);
    } else if (strcmp(buf, "door-->door_close") == 0) {
        door_state_1 = DOOR_CLOSE;
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 5);
    }

    if (strcmp(buf, "sonic->s_check") == 0) {
        (*checkFlag) = motor_check;
    } else if (strcmp(buf, "sonic->s_reload") == 0) {
        (*checkFlag) = motor_reload;
    }
}

//void uart_proc(bool *auto_manual_flag,uint8_t *num,uint8_t *Dread,struct dht11_st*dht11,bool *flag,uint8_t *check_flag)
//{
//	
//	if(rec_flag == 1)
//	{
//		rec_flag=0;
//		rec_buf[n]='\0';
//		n=0;
//		
//		//数据太长容易断
//		if(strcmp(rec_buf,"help")==0)
//		{
//			printf("------------------------------\r\n");
//			printf("Usage\r\n");
//			printf("Mode:\r\n");
//			printf("1.auto:default.\r\n");
//			printf("2.manual:by command->manual.\r\n");
//			printf("Command(all the time):\r\n");
//			printf("1.command for data :request_data\r\n");
//			printf("1.command for state :show_sys_state\r\n");
//			printf("------------------------------\r\n");
//			//printf("\r\n");
//		}
//		
//		if(strcmp(rec_buf,"manual")==0)
//		{
//			(*auto_manual_flag)=1;
//			printf("Usage(manual mode):\r\n");
//			printf("command for motor:motor->motor_on_speed:s/f or motor_off\r\n");
//			printf("command for door :door-->door_open or door_close\r\n");
//			printf("command for sonic:sonic->s_check or s_reload\r\n");
//			printf("command for sonic:led--->led_on or led_off\r\n");
//			printf("command for limit:limit:Dis(8-30):xx or Tp(20-35):xx or Lg(2000-3300):xx");
//		}
//		else if(strcmp(rec_buf,"auto")==0)
//		{
//			(*auto_manual_flag)=0;
//			
//		}
//		
//		
//		//一直有效
//		if(strcmp(rec_buf,"request_data")==0 && (*num)>=1)
//		{
//			for(int i=0;i<24;i+=2)
//			{
//				printf("[%d]->temperature:%d℃,huminity:%d%%\r\n",i+1,Dread[i],Dread[i+1]);
//			}
//			
//		}
//		else if(strcmp(rec_buf,"request_data")==0 && (*num)==0)
//		{
//			printf("not record now!\r\n");
//			
//		}
//		
//		
//		
//		if(strcmp(rec_buf,"show_sys_state")==0)
//		{
//			printf("the slave state:\r\n");
//			if(door_state_1==DOOR_CLOSE)
//			{
//				printf("door_state:close\r\n");
//			}
//			else 
//			{
//				printf("door_state:open\r\n");
//			}
//			
//			printf("temperature:%.2f\r\n",dht11->temp);
//			printf("huminity:%.2f\r\n",dht11->humi);
//			
//			if((*auto_manual_flag)==0)
//			{
//				if(dht11->temp<bond_temp)
//				{
//					printf("motor_state:stop\r\n");
//				}
//				else if(dht11->temp>=bond_temp && dht11->temp<35)
//				{
//					printf("motor_state:slow\r\n");
//				}
//				else
//				{
//					printf("motor_state:fast\r\n");
//				}
//				
//				if((*flag)==1)
//				{
//					printf("led_state:off\r\n");
//				}
//				else
//				{
//					printf("led_state:on\r\n");
//				}
//				
//				
//			}
//			else if((*auto_manual_flag)==1)
//			{
//				if(motor_state_1==MOTOR_OFF)
//				{
//					printf("motor_state:stop\r\n");
//				}
//				else if(motor_state_1==MOTOR_ON_S)
//				{
//					printf("motor_state:slow\r\n");
//				}
//				else 
//				{
//					printf("motor_state:fast\r\n");
//				}
//				if(led_state_1==LED_OFF)
//				{
//					printf("led_state:off\r\n");
//				}
//				else
//				{
//					printf("led_state:on\r\n");
//				}	
//			}
//		}
//		
//		
//		//更改限制
//		if((*auto_manual_flag)==1)
//		{
//			if(strncmp(rec_buf,"limit:",6)==0)
//			{
//				char *temp_limit=NULL;
//				uint16_t temp_num=0;
//				strtok(rec_buf,":");
//				temp_limit = strtok(NULL,":");
//				if(strncmp(temp_limit,"Dis",3)==0)
//				{
//					temp_limit = strtok(NULL,":");
//					temp_num=atoi(temp_limit);
//					if(temp_num>=8 &&temp_num<=30)
//					{
//						bond_dis = temp_num;
//					}
//					else
//					{
//							printf("wrong dis value\r\n");
//					}
//				}
//				else if(strncmp(temp_limit,"Tp",2)==0)
//				{
//					temp_limit = strtok(NULL,":");
//					temp_num=atoi(temp_limit);
//					if(temp_num>=20 &&temp_num<=35)
//					{
//						bond_temp = temp_num;
//					}
//					else
//					{
//							printf("wrong temperature value\r\n");
//					}
//				}
//				else if(strncmp(temp_limit,"Lg",2)==0)
//				{
//					temp_limit = strtok(NULL,":");
//					temp_num=atoi(temp_limit);
//					if(temp_num>=2000 &&temp_num<=3300)
//					{
//						bond_light = temp_num;
//					}
//					else
//					{
//						printf("wrong light value\r\n");
//						
//					}
//				}
//				else
//				{
//					printf("wrong limit elements select\r\n");
//					printf("temp:%s\r\n",temp_limit);
//				}
//			}
//		}
//		
//		
//		
//		//模式需要手动下才生效
//		if(((strncmp(rec_buf,"motor->",7)==0) || (strncmp(rec_buf,"door-->",7)==0) || (strncmp(rec_buf,"sonic->",7)==0)|| (strncmp(rec_buf,"led--->",7)==0)) && (*auto_manual_flag)==0 )
//		{
//			printf("error:auto_mode now(need:manual)\r\n");
//		}
//		else
//		{
//			if(strcmp(rec_buf,"led--->led_on")==0)
//			{
//				led_state_1 = LED_ON;
//			}
//			else if(strcmp(rec_buf,"led--->led_off")==0)
//			{
//				led_state_1 = LED_OFF;
//			}
//			
//			
//			if(strcmp(rec_buf,"motor->motor_on_speed:s")==0)
//			{
//				motor_state_1 = MOTOR_ON_S;
//			}
//			//这个电压不够会导致蓝牙失效
//			else if(strcmp(rec_buf,"motor->motor_on_speed:f")==0)
//			{
//				motor_state_1=MOTOR_ON_F;
//			}
//			else if(strcmp(rec_buf,"motor->motor_off")==0)
//			{
//				motor_state_1 = MOTOR_OFF;
//			}
//			
//			
//			if(strcmp(rec_buf,"door-->door_open")==0)
//			{
//				door_state_1=DOOR_OPEN;
//				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,25);
//			}
//			else if(strcmp(rec_buf,"door-->door_close")==0)
//			{
//				door_state_1=DOOR_CLOSE;
//				__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,5);
//			}
//			//不能加stop的逻辑，因为到大于限制距离其又会自动扫描旋转
//			
//			if(strcmp(rec_buf,"sonic->s_check")==0)
//			{
//				//sonic_steer_state_1=S_CHECK;
//				(*check_flag) = motor_check;
//			}

//			else if(strcmp(rec_buf,"sonic->s_reload")==0)
//			{
//				//sonic_steer_state_1=S_RELOAD;
//				(*check_flag) = motor_reload;
//			}
//		}

//	}
//}
