/* USER CODE BEGIN Header */

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "stdio.h"
#include "DHT11.h"
#include "stdbool.h"
#include "interrupt.h"
#include "hc_sr04.h"
#include "w25q64.h"
#include "stdlib.h"
#include "string.h"
#include "esp8266.h"
#include "mykey.h"
#include "myuart.h"
#include "motor.h"
#include "myadc.h"
#include "stm32f1xx_it.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


//全局变量

//void rk_set_proc(bool *auto_manual_flag);
void rk_set_proc(bool *auto_manual_flag, uint8_t *motor_state_1, bool *led_state_1, uint8_t *check_flag, bool *buzzer_flag, bool *door_state_1);

void ali_update(bool *ali_trans_flag,float dis,struct dht11_st dht11,bool flag);

bool buzzer_flag;
uint8_t check_flag=0;
uint16_t bond_temp=30,bond_light=3000,bond_dis=10;
bool flag_5s=0;//这里从1修改到了0，防止初始存一次空数据
uint8_t rec;
struct ali_state_st ali_state_st_1;


//char * extract_between(const char *str,char start_char,char end_char)
//{
//    char *start = strchr(str,start_char); //在字符串中找字符的函数
//    if(start==NULL)
//    {
//        return NULL;
//    }
//    //printf("%s\n",start);
//    //有就跳过当前找到的字符
//    start++;
//    char *end = strchr(start,end_char);
//    if(end==NULL)
//    {
//        return NULL;
//    }
//    size_t len = end - start;
//    //开辟新内存用于存储
//    char *result = (char *)malloc(len+1);
//    //printf("%s\n",result);
//    if(result==NULL)
//    {
//        return NULL;
//    }
//    strncpy(result,start,len);
//    result[len]='\0';
//    return result;
//}

const char* extract_between(const char *str, char start, char end) {
    const char *start_pos = strchr(str, start);
    if (start_pos == NULL) return NULL;
    start_pos++; // 跳过开始字符
    const char *end_pos = strchr(start_pos, end);
    if (end_pos == NULL) return NULL;
    return start_pos;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM4_Init();
  MX_TIM8_Init();
  MX_TIM6_Init();
  MX_SPI1_Init();
  MX_TIM7_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	
	//局部变量，通过传指针传递
	struct dht11_st dht11;
	bool flag = 0;
	float dis=0;

	uint8_t write_index=0;
	uint8_t count_write=0;
	uint8_t num=0;
	
	uint8_t ID[4];
	uint8_t Dread[24];
	uint8_t Dwrite[24];
	bool auto_manual_flag;
	uint8_t dis_index=0;
//	uint16_t smoke_value;
	OLED_Init();
	if(DHT11_Init())
	{
		OLED_ShowString(3, 3, "dht11:wrong");
	}
	//按键中断
	HAL_TIM_Base_Start_IT(&htim1);
	__HAL_TIM_SetAutoreload(&htim1,1000);
	//电机PWM
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
	//超声波舵机
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
	//超声波舵机复位
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,5);
	//用作dht11计时
	HAL_TIM_Base_Start(&htim6);
	//开门舵机
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1); 
	//控制门舵机复位
	__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,5);
	//串口接收中断
	HAL_UART_Receive_IT(&huart1,&rec,1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	w25q64_init(ID);
	//与阿里云通信部分
	ESP8266_Init();
	Ali_Yun_Init();
	//Ali_Yun_Send();
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
			
		//接收rk的设置信息
		rk_set_proc(&auto_manual_flag,(uint8_t *)&motor_state_1,(bool *)&led_state_1,&check_flag,&buzzer_flag,(bool *)&door_state_1);
		
		//温湿度		
		dht11_measure(&dht11,Dwrite,&flag_5s,&write_index,&count_write);//最后一个参数到底是为何
		
		//电机风扇
		motor_proc(&auto_manual_flag,&dht11,&bond_temp);
		//光敏处理
		adc_proc(&flag,&auto_manual_flag,&bond_light); //做了修改
		//超声波
		hc_sr04_proc(&dis,&check_flag,&bond_dis); //修改了延时 //耗时1
		//屏幕显示
		//为何这个bond_light为无法正确显示，因为在interrupt定义其为extern ,这种应该在定义它的.h文件中，而不是其他的.c文件中
		screen_proc(&auto_manual_flag,key,&dht11,&bond_temp,&bond_dis,&bond_light,&flag,&dis,&check_flag,&buzzer_flag,Dread,&dis_index,&num); //做了一些else if修改
		
		//按键
		key_proc(key,&check_flag,&dis_index); //做了修改
		//屏幕刷新
		screen_flash(key);
		//存储信息
		w25q64_proc(&count_write,Dwrite,Dread,&write_index,&num);
		//串口转蓝牙通信部分
		uartProc(&auto_manual_flag,&num,Dread,&dht11,&flag,&check_flag);
		
		//ali
		ali_update(&ali_trans_flag,dis,dht11,flag);
//		if(ali_trans_flag)
//		{
//			//printf("trans begin!\r\n");
//			ali_trans_flag=0;
//			
//			//新增属性
//			ali_state_st_1.sonic_dis = dis;
//			ali_state_st_1.sonic_s = check_flag;
//			ali_state_st_1.buzzer_s = buzzer_flag;
//			
//			
//			ali_state_st_1.dht11_humi=dht11.humi;
//			ali_state_st_1.dht11_temper=dht11.temp;
//			ali_state_st_1.door_s=door_state_1;
//			ali_state_st_1.led_s=flag;
//			ali_state_st_1.motor_s=motor_state_1;
//			ali_state_st_1.rain_s = rain_state_1;
//			Ali_Yun_Send(&ali_state_st_1); //耗时2
//			printf("run 3\r\n");
//		}
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void ali_update(bool *ali_trans_flag,float dis,struct dht11_st dht11,bool flag)
{
		if(ali_trans_flag)
		{
			//printf("trans begin!\r\n");
			ali_trans_flag=0;
			
			//新增属性
			ali_state_st_1.sonic_dis = dis;
			ali_state_st_1.sonic_s = check_flag;
			ali_state_st_1.buzzer_s = buzzer_flag;
			
			
			ali_state_st_1.dht11_humi=dht11.humi;
			ali_state_st_1.dht11_temper=dht11.temp;
			ali_state_st_1.door_s=door_state_1;
			ali_state_st_1.led_s=flag;
			ali_state_st_1.motor_s=motor_state_1;
			ali_state_st_1.rain_s = rain_state_1;
			Ali_Yun_Send(&ali_state_st_1); //耗时2
			//printf("run 3\r\n");
		}
}




void update_state(uint8_t *state, uint8_t new_state) {
    if (*state != new_state) {
        *state = new_state;
        // 这里可以执行相应的硬件操作
    }
}
void rk_set_proc(bool *auto_manual_flag, uint8_t *motor_state_1, bool *led_state_1, uint8_t *check_flag, bool *buzzer_flag, bool *door_state_1)
{
    if (rec_set_flag) {
			
				rec_set_flag = 0;
        char *search_str = "_s\"";
        char *temp_char = (char *)esp_buff;
        uint8_t count = 0;

        while ((temp_char = strstr(temp_char, search_str)) != NULL) {
            count++;

            const char *extract_str = extract_between(temp_char, ':', ',');
            int8_t value = atoi(extract_str);

            if (value == 1 || value == 0 || (value == 2 && count == 1)) {
                // rk发来命令进行设置就会变为手动模式
                *auto_manual_flag = true;

                switch (count) {
                    case 1:
                        update_state(motor_state_1, value);
                        break;
                    case 2:
                        update_state((uint8_t *)led_state_1, value);
                        break;
                    case 3:
                        update_state(check_flag, value);
                        break;
                    case 4:
                        update_state((uint8_t *)buzzer_flag, value);
                        break;
                    case 5:
                        if(*door_state_1!=value)
                        {
                            if(value==1)
                            {
                                __HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,25);
                            }
                            else if(value==0)
                            {
                                __HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,5);
                            }
                        }
                        update_state((uint8_t *)door_state_1, value);
                        break;
                    default:
                        break;
                }
            }
            temp_char += strlen(search_str);
        }
        // 清除接收缓冲区
        ESP8266_Clear();
    }
}


//void rk_set_proc(bool *auto_manual_flag)
//{
//		if(rec_set_flag)
//		{
//			printf("rec_set_data:%s\r\n",esp_buff);
//			rec_set_flag = 0;
//			char *search_str = "_s\"";
//			char *temp_char = (char *)esp_buff;
//		  char trans_info[120];
//			memset(trans_info,0,120);
//			uint8_t count =0;
//			int8_t rec_set_flag[5]={0};
//			while ((temp_char=strstr(temp_char,search_str))!=NULL)
//			{
//				count++;
//					 //printf("str:%s\n",temp_char);
//				const char *extract_str = extract_between(temp_char,':',',');
//				//printf("extract str:%s\n",extract_str);
//				//sprintf(per_info,"%s:",extract_str);
//				//strcat(trans_info,per_info);
//				//这里的操作需要修改，标志位无法让其执行操作，是执行了操作会让标志位置位
//				//这里重写一个函数，用于处理这种上传型的数据
//				if(atoi(extract_str)==1 || atoi(extract_str)==0 || (atoi(extract_str)==2&&count==1))
//				{
//					//rk发来命令进行设置就会变为手动模式
//					(*auto_manual_flag)=1;
//					if(count == 1)
//					{
//						uint8_t new_motor_state_1 = atoi(extract_str);
//						if(new_motor_state_1!=motor_state_1)
//						{
//							motor_state_1 = new_motor_state_1;
//						}
//					}
//					//对应灯的
//					else if(count==2)
//					{
//						bool new_flag = atoi(extract_str);
//						if(new_flag!=led_state_1)
//						{
////							if(new_flag==0)
////							{
////								HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
////							}
////							else if(new_flag==1)
////							{
////								HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
////							}
//							led_state_1 = new_flag;
//						}
//					}
//					//对应舵机状态
//					else if(count == 3)
//					{
//						check_flag = atoi(extract_str);
//					}
//					//对应喇叭状态
//					else if(count==4)
//					{
//						buzzer_flag = atoi(extract_str);
//					}
//					//门状态对应
//					else if(count==5)
//					{
//						bool new_door_state_1 = atoi(extract_str);
//						if(new_door_state_1!=door_state_1)
//						{
//							if(new_door_state_1==1)
//							{
//								__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,25);
//							}
//							else if(new_door_state_1==0)
//							{
//								__HAL_TIM_SET_COMPARE(&htim8,TIM_CHANNEL_1,5);
//							}
//							
//							door_state_1 = new_door_state_1;
//						}
//					}
//				}
//				temp_char += strlen(search_str);
//			}
//			//清除接收缓冲区
//			//printf("set success\r\n");
//			ESP8266_Clear();
//		}
//}

int fputc(int ch,FILE *p)
{
	char c = ch;
	HAL_UART_Transmit(&huart1,(uint8_t *)&c,1,50);
	return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
