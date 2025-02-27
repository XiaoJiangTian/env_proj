#include "hc_sr04.h"

#define TRIG_H	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_1,GPIO_PIN_SET);
#define TRIG_L	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_1,GPIO_PIN_RESET);

//#define Echo_GPIO_Port GPIOE
//#define Echo_Pin GPIO_PIN_3

//uint16_t s1,s2;
//float distance;


////通过宏定义加延时，来实现器件的开启时序
//float hc_sr04_detect()
//{
//		Trig_L;
//		Trig_H;
//		delay_us(12);
//		//HAL_Delay(1);//拉高要求是10us
//		Trig_L; //拉低
//		while(HAL_GPIO_ReadPin(Echo_GPIO_Port ,Echo_Pin)==GPIO_PIN_RESET)//等待回响信号
//		{
//		}
//		
//		htim4.Instance->CNT=0; 
//		s1=htim4.Instance->CNT;
//		while(HAL_GPIO_ReadPin(Echo_GPIO_Port ,Echo_Pin)==GPIO_PIN_SET)
//		{
//		}
//		s2=htim4.Instance->CNT;
//		distance=(s2-s1)*0.034/2;//cm是这样的，us
//		//HAL_Delay(500);
//		return distance;
//}

float distant;      //测量距离
uint32_t measure_Buf[3] = {0};   //存放定时器计数值的数组
uint8_t  measure_Cnt = 0;    //状态标志位
uint32_t high_time;   //超声波模块返回的高电平时间
 
 
//===============================================读取距离

//float sr04_data(void)
//{
//	
//	 
//	 
//	 
//}
//	 
float SR04_GetData(void)
{
	while(1)
	{
		switch (measure_Cnt){
    case 0:
         TRIG_H;
         delay_us(12);
         TRIG_L;
    
        measure_Cnt++;
        __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
        HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1); //启动输入捕获       或者: __HAL_TIM_ENABLE(&htim5);                                                                                            
    break;
    case 3:
        high_time = measure_Buf[1]- measure_Buf[0];    //高电平时间
        //printf("\r\n----高电平时间-%d-us----\r\n",high_time);                           
        distant=(high_time*0.034)/2;  //单位cm
        //printf("\r\n-检测距离为-%.2f-cm-\r\n",distant);          
        measure_Cnt = 0;  //清空标志位
        TIM4->CNT=0;     //清空计时器计数
				return distant;
    break;
    }
	}
	
	
	
	


}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//
{
    
    if(TIM4 == htim->Instance)// 判断触发的中断的定时器为TIM4
    {
        switch(measure_Cnt){
            case 1:
                measure_Buf[0] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//获取当前的捕获值.
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);  //设置为下降沿捕获
                measure_Cnt++;                                            
                break;              
            case 2:
                measure_Buf[1] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//获取当前的捕获值.
                HAL_TIM_IC_Stop_IT(&htim4,TIM_CHANNEL_1); //停止捕获   或者: __HAL_TIM_DISABLE(&htim5);
                measure_Cnt++;  
                
        }
    
    }
    
}


void hc_sr04_proc(float *dis,uint8_t *check_flag,uint16_t *bond_dis)
{
	float sum=0;
	for(uint8_t i=0;i<2;i++)
	{
		sum+=SR04_GetData();
	}
	(*dis) = sum/2.0;//做一个均值滤波？
	//printf("\r\n-检测距离为-%.2f-cm-\r\n",dis);          
	
	if((*dis)<(*bond_dis) && (*check_flag)==motor_check)
	{
		(*check_flag)=motor_stop;
		//蜂鸣器以另一种方式报警
	}
	else if((*check_flag)==motor_stop && (*dis)>=(*bond_dis))
	{
		(*check_flag)=motor_check;
	}
}

