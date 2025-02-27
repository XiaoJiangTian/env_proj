#ifndef __MOTOR_H
#define __MOTOR_H

//#include "main.h"
#include "DHT11.h"
//��������Ҫ�������õı�����������.h�������ⲿ������Ҳ���Խ��к궨�壬��һЩ�ṹ�壬ö�ٵĶ��壬�����ļ����ú���ʶ����Щ�������͡�
#define motor_check 1
#define motor_stop 2
#define motor_reload 0

typedef enum motor_state
{
	MOTOR_OFF=0,
	MOTOR_ON_S,
	MOTOR_ON_F,
}MY_MOTOR_STATE;


extern enum motor_state motor_state_1;

extern	int myabs(int num);
//void Set_Pwmb(int motor_right);
void Set_Pwma(int motor_left);
int myabs(int num);
void Motor_run(int flag);
void motor_proc(bool *auto_manual_flag,struct dht11_st *dht11,uint16_t *bond_temp);
//void Motor_Left();


#endif 


