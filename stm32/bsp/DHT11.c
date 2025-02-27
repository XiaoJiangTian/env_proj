#include "DHT11.h"	


//这里用的是那个，该定时器的autoreload就不能设上限，
void delay_us(uint16_t us)
{
    uint16_t differ = 0xffff-us-5;                
    __HAL_TIM_SET_COUNTER(&htim6,differ);    
    HAL_TIM_Base_Start(&htim6);        
    
    while(differ < 0xffff-5)
    {   
        differ = __HAL_TIM_GET_COUNTER(&htim6);     
    }
    HAL_TIM_Base_Stop(&htim6);
}



void DHT11_IO_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.Pin = GPIO_PIN_3;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
}
 
void DHT11_IO_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_3;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
}
 
 
//复位DHT11
void DHT11_Rst(void)	   
{                 
	DHT11_IO_OUT(); 	//设置为输出
	DHT11_DQ_OUT_LOW; 	//拉低DQ
	HAL_Delay(20);    	//拉低至少18ms
	DHT11_DQ_OUT_HIGH; 	//DQ=1 
	delay_us(30);     	//主机拉高20~40us
}
 
//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
uint8_t DHT11_Check(void) 	   
{   
	uint8_t retry=0;
	DHT11_IO_IN();      //设置为输出	 
	while (DHT11_DQ_IN&&retry<100)//DHT11会拉低40~80us
	{
		retry++;
		delay_us(1);
	};	 
	if(retry>=100)return 1;
	else retry=0;
	while (!DHT11_DQ_IN&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++;
		delay_us(1);
	};
	if(retry>=100)return 1;	    
	return 0;
}
 
//从DHT11读取一个位
//返回值：1/0
uint8_t DHT11_Read_Bit(void) 			 
{
 	uint8_t retry=0;
	while(DHT11_DQ_IN&&retry<100)//等待变为低电平
	{
		retry++;
		delay_us(1);
	}
	retry=0;
	while(!DHT11_DQ_IN&&retry<100)//等待变高电平
	{
		retry++;
		delay_us(1);
	}
	delay_us(40);//等待40us
	if(DHT11_DQ_IN)return 1;
	else return 0;		   
}
 
//从DHT11读取一个字节
//返回值：读到的数据
uint8_t DHT11_Read_Byte(void)    
{        
	uint8_t i,dat;
	dat=0;
	for (i=0;i<8;i++) 
	{
   		dat<<=1; 
	    dat|=DHT11_Read_Bit();
    }						    
    return dat;
}
 
//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
uint8_t DHT11_Read_Data(uint8_t *tempi,uint8_t *tempf,uint8_t *humi,uint8_t*humf)    
{        
 	uint8_t buf[5];
	uint8_t i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			*humi=buf[0];
			*humf=buf[1];
			
			*tempi=buf[2];
			*tempf=buf[3];
		}
	}else return 1;
	return 0;	    
}
 
//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在     	 
uint8_t DHT11_Init(void)
{ 
  DHT11_Rst();
	return DHT11_Check();
}


void dht11_measure(struct dht11_st *dht11,uint8_t *Dwrite_t,bool *flag_5s_t,uint8_t *write_index_t,uint8_t *count_write_t)
{
	static unsigned char tempi=0,tempf=0,humii=0,humif=0;
	DHT11_Read_Data(&tempi,&tempf,&humii,&humif);
	dht11->temp = tempi+tempf/10.0;
	dht11->humi = humii+humif/10.0;
	//存数据
	if((*flag_5s_t)==1)//5s一记
	{
		(*flag_5s_t)=0;
		Dwrite_t[(*write_index_t)++]=dht11->temp;
		Dwrite_t[(*write_index_t)++]=dht11->humi;
		(*count_write_t)++;//当等于5时写了
	}
}

