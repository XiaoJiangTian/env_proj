#include "hc_sr04.h"

#define TRIG_H	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_1,GPIO_PIN_SET);
#define TRIG_L	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_1,GPIO_PIN_RESET);

//#define Echo_GPIO_Port GPIOE
//#define Echo_Pin GPIO_PIN_3

//uint16_t s1,s2;
//float distance;


////ͨ���궨�����ʱ����ʵ�������Ŀ���ʱ��
//float hc_sr04_detect()
//{
//		Trig_L;
//		Trig_H;
//		delay_us(12);
//		//HAL_Delay(1);//����Ҫ����10us
//		Trig_L; //����
//		while(HAL_GPIO_ReadPin(Echo_GPIO_Port ,Echo_Pin)==GPIO_PIN_RESET)//�ȴ������ź�
//		{
//		}
//		
//		htim4.Instance->CNT=0; 
//		s1=htim4.Instance->CNT;
//		while(HAL_GPIO_ReadPin(Echo_GPIO_Port ,Echo_Pin)==GPIO_PIN_SET)
//		{
//		}
//		s2=htim4.Instance->CNT;
//		distance=(s2-s1)*0.034/2;//cm�������ģ�us
//		//HAL_Delay(500);
//		return distance;
//}

float distant;      //��������
uint32_t measure_Buf[3] = {0};   //��Ŷ�ʱ������ֵ������
uint8_t  measure_Cnt = 0;    //״̬��־λ
uint32_t high_time;   //������ģ�鷵�صĸߵ�ƽʱ��
 
 
//===============================================��ȡ����

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
        HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1); //�������벶��       ����: __HAL_TIM_ENABLE(&htim5);                                                                                            
    break;
    case 3:
        high_time = measure_Buf[1]- measure_Buf[0];    //�ߵ�ƽʱ��
        //printf("\r\n----�ߵ�ƽʱ��-%d-us----\r\n",high_time);                           
        distant=(high_time*0.034)/2;  //��λcm
        //printf("\r\n-������Ϊ-%.2f-cm-\r\n",distant);          
        measure_Cnt = 0;  //��ձ�־λ
        TIM4->CNT=0;     //��ռ�ʱ������
				return distant;
    break;
    }
	}
	
	
	
	


}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//
{
    
    if(TIM4 == htim->Instance)// �жϴ������жϵĶ�ʱ��ΪTIM4
    {
        switch(measure_Cnt){
            case 1:
                measure_Buf[0] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//��ȡ��ǰ�Ĳ���ֵ.
                __HAL_TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);  //����Ϊ�½��ز���
                measure_Cnt++;                                            
                break;              
            case 2:
                measure_Buf[1] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//��ȡ��ǰ�Ĳ���ֵ.
                HAL_TIM_IC_Stop_IT(&htim4,TIM_CHANNEL_1); //ֹͣ����   ����: __HAL_TIM_DISABLE(&htim5);
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
	(*dis) = sum/2.0;//��һ����ֵ�˲���
	//printf("\r\n-������Ϊ-%.2f-cm-\r\n",dis);          
	
	if((*dis)<(*bond_dis) && (*check_flag)==motor_check)
	{
		(*check_flag)=motor_stop;
		//����������һ�ַ�ʽ����
	}
	else if((*check_flag)==motor_stop && (*dis)>=(*bond_dis))
	{
		(*check_flag)=motor_check;
	}
}

