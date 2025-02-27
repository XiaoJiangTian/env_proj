
#include "motor.h"
#include "tim.h"
/**************************************************************************
�������ܣ���ֵ��PWM�Ĵ���
��ڲ�����PWM
����  ֵ����
**************************************************************************/
#define AIN1_GPIO_Port GPIOC
#define AIN2_GPIO_Port GPIOC
#define AIN1_Pin GPIO_PIN_1
#define AIN2_Pin GPIO_PIN_3

//����ֻ����һ��

//void Set_Pwmb(int motor_right)//��ֵ��PWM�Ĵ���
//{
//	int pwmb_abs;
//	if(motor_right>0) Motor_Right(1);
//	else      Motor_Right(0);
//	pwmb_abs=myabs(motor_right);
//	__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,pwmb_abs);
//}
void Set_Pwma(int motor_left)//��ֵ��PWM�Ĵ���
{
	int pwma_abs;
	if(motor_left>0) Motor_run(1);
	else      Motor_run(0);
	pwma_abs=myabs(motor_left);
	__HAL_TIM_SET_COMPARE(&htim2,TIM_CHANNEL_3,pwma_abs);
}

void Motor_run(int flag)
{
	if(flag)
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_SET);//��01����ת
	}
	else 
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_RESET);//��10����ת
	}
		
}
//void Motor_Left()
//{
//		HAL_GPIO_WritePin(BIN1_GPIO_Port,BIN1_Pin,GPIO_PIN_RESET);
//		HAL_GPIO_WritePin(BIN2_GPIO_Port,BIN2_Pin,GPIO_PIN_SET);//��01����ת
//}
/*
*	�������ܣ�ȡ����ֵ
*	��ڲ�����int
*	����ֵ���� unsingned int
*/
int myabs(int num)
{
	int temp;
	if(num<0)	temp=-num;
	else temp =num;
	return temp;
}

enum motor_state motor_state_1;

void motor_proc(bool *auto_manual_flag,struct dht11_st *dht11,uint16_t *bond_temp)
{
//	 if((*auto_manual_flag)==0)
//	{
//		if(dht11->temp<(*bond_temp))
//		{
//			Set_Pwma(0);
//		}
//		else if(dht11->temp>=(*bond_temp) && dht11->temp<35)
//		{
//			Set_Pwma(50);
//		}
//		else
//		{
//			Set_Pwma(500);
//		}
//	}
//	else
//	{
//		//������if����������switch
//		if(motor_state_1==MOTOR_OFF)
//		{
//			Set_Pwma(0);
//		}
//		else if(motor_state_1==MOTOR_ON_S)
//		{
//			Set_Pwma(50);
//		}
//		else 
//		{
//			Set_Pwma(500);
//		}
//	}	
//	
	switch(*(int8_t *)auto_manual_flag)
	{
		case 0:
		{
			if(dht11->temp<(*bond_temp))
			{
				Set_Pwma(0);
			}
			else if(dht11->temp>=(*bond_temp) && dht11->temp<35)
			{
				Set_Pwma(50);
			}
			else
			{
				Set_Pwma(500);
			}
		}
		break;
		
		case 1:
		{
			if(motor_state_1==MOTOR_OFF)
			{
				Set_Pwma(0);
			}
			else if(motor_state_1==MOTOR_ON_S)
			{
				Set_Pwma(50);
			}
			else 
			{
				Set_Pwma(500);
			}
		}
		break;
	}
}

