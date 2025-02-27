#ifndef __MOTOR_H
#define __MOTOR_H

//#include "main.h"
#include "DHT11.h"
//像这种想要别人引用的变量，都可在.h中声明外部变量，也可以进行宏定义，和一些结构体，枚举的定义，其他文件引用后都能识别这些数据类型。
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


