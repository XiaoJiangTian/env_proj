#include "esp8266.h"


#define      WIFI_NAME      "7erkeyT"       // wifi名
#define      WIFI_PASS      "txjandtsh"       // wifi密码
extern int esp_cnt;
extern uint8_t esp_buff[ESPBUFF_MAX_SIZE];
extern DMA_HandleTypeDef hdma_usart2_rx;


//void USART2_IRQHandler(void)
//{
//  /* USER CODE BEGIN USART2_IRQn 0 */
// 
//  /* USER CODE END USART2_IRQn 0 */
//  HAL_UART_IRQHandler(&huart2);
//  /* USER CODE BEGIN USART2_IRQn 1 */
//  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE) != RESET)    // 空闲中断的标志位
//  {
//    HAL_UART_DMAStop(&huart2);                               //停止接收
//    esp_cnt = ESPBUFF_MAX_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);    // 计算接收的数据长度
//    HAL_UART_Transmit(&huart1,esp_buff,esp_cnt,1000);
////    printf("rec = %s\r\n",esp_buff);
//    HAL_UART_Receive_DMA(&huart2,esp_buff,ESPBUFF_MAX_SIZE);         // 开启DMA继续接收
//    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
//  }
//  /* USER CODE END USART2_IRQn 1 */
//}

uint8_t ESP8266_SendCmd(char *cmd,char *res);

//连接wifi
void ESP8266_Init(void)
{
    HAL_UART_Receive_DMA(&huart2,esp_buff,ESPBUFF_MAX_SIZE);    // 开启DMA接收
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);      // 开启串口的空闲中断
    while(ESP8266_SendCmd("AT+RST\r\n", "ready")) //复位，等待目标信息
    while(ESP8266_SendCmd("AT\r\n","OK")){} //AT
    while(ESP8266_SendCmd("AT+CWMODE=1\r\n","OK")){}//设置模式
    //加入wifi热点
    while(ESP8266_SendCmd("AT+CWJAP=\""WIFI_NAME"\",\""WIFI_PASS"\"\r\n","OK")){}//连接wifi
    // 设置   
    //printf("success");
}

void ESP8266_Clear()
{
    memset(esp_buff,0,sizeof(esp_buff));
    esp_cnt = 0;
}

//串口，使用寄存器操作发送数据
void ESP8266_SendString(char *str,uint8_t len)
{
    uint8_t i=0;
    for(i=0;i<len;i++)
    {
        USART2->DR = *str;
        str++;
        HAL_Delay(1);
    }
}

uint8_t ESP8266_SendCmd(char *cmd,char *res)
{
    uint8_t num = 200;
    ESP8266_Clear();
    // 发送指令
    ESP8266_SendString(cmd,strlen((const char *)cmd));
    while(num--)
    {
        if(strstr((const char*)esp_buff,(const char *)res)!=NULL)//检验返回值
        {
            ESP8266_Clear();
            return 0;
        }
        HAL_Delay(10);
    }
    return 1;
}

//uint8_t ESP8266_SendCmd1(char *cmd,char *res)
//{
//    uint8_t num = 200;
//    ESP8266_Clear();
//    // 发送指令
//    ESP8266_SendString(cmd,strlen((const char *)cmd));
////    while(num--)
////    {
////        if(strstr((const char*)esp_buff,(const char *)res)!=NULL)//检验返回值
////        {
////            ESP8266_Clear();
////            return 0;
////        }
////        HAL_Delay(10);
////    }
//		ESP8266_Clear();
//    return 0;
//}



//连接阿里云部分


#define      ALI_USERNAME       "mqtt_stm32&k1m42D6OwDF"                                         // 用户名
#define      ALICLIENTLD            "esp8266|securemode=3\\,signmethod=hmacsha1\\,timestamp=100|"                // 客户id,两个反斜杠，第一个用于转义
#define      ALI_PASSWD         "5956FC8C37143ED42BF7A391063BE39CA706230D"           // MQTT 密码
#define      ALI_MQTT_HOSTURL   "k1m42D6OwDF.iot-as-mqtt.cn-shanghai.aliyuncs.com"            // mqtt连接的网址
#define      ALI_PORT           "1883"                // 端口
 


//以主题为中心
#define      ALI_TOPIC_SET          "/k1m42D6OwDF/mqtt_stm32/user/get" //set用于接受（都是相对于设备而言）
#define      ALI_TOPIC_POST         "/k1m42D6OwDF/mqtt_stm32/user/env_info" //post用于发布
#define      ALI_TOPIC_POST1         "/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post" //post用于发布
#define      ALI_TOPIC_POST_RK3568   "/k1m42D6OwDF/mqtt_stm32/user/rec_rk3568_set" //接收来自rk的设置信息
#define      ALI_TOPIC_POST_CANCEL		"/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post_reply"

void Ali_Yun_Topic(void);
void Ali_Yun_Init(void)
{
    //设置用户名，密码
    while(ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\"NULL\",\""ALI_USERNAME"\",\""ALI_PASSWD"\",0,0,\"\"\r\n","OK")){} //""里的内容代表字符串，如果是想显示发送"需用转义字符\
    HAL_Delay(10);
    // 设置客服id
    while(ESP8266_SendCmd("AT+MQTTCLIENTID=0,\""ALICLIENTLD"\"\r\n","OK")){}
 
    // 连接阿里云  AT+MQTTCONN=0,"iot-06z00b28nanp9ew.mqtt.iothub.aliyuncs.com",1883,1
    while(ESP8266_SendCmd("AT+MQTTCONN=0,\""ALI_MQTT_HOSTURL"\",1883,1\r\n","OK")){}
	
		//至此阿里云网站上设备已经在线
    Ali_Yun_Topic();
}
 
void Ali_Yun_Topic(void)
{
    //"AT+MQTTPUB=0,\"发布的主题\",\""; //SUB是订阅主题，即接受阿里云的信息，PUB是上传主题，发送给阿里云
		//while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_SET"\",1\r\n","OK")){}//第一个0是设备索引，第二个1是版本索引
			
		while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_POST_RK3568"\",1\r\n","OK")){}//订阅rk的主题，用来接收rk的设置信息
		
		
		
		//while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_POST_CANCEL"\",1\r\n","OK")){}//取消订阅
		
		//while(ESP8266_SendCmd("AT+MQTTUNSUB=0,\""ALI_TOPIC_POST_CANCEL"\"\r\n","OK")){}//取消订阅
			
		//while(ESP8266_SendCmd("AT+MQTTUNSUB=0,\"/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post_reply\"\r\n", "OK")){}

 
    while(ESP8266_SendCmd("AT+MQTTPUB=0,\""ALI_TOPIC_POST"\",\"test info from txj\",1,0\r\n","OK")){}
}


//阿里云数据上传
void Ali_Yun_Send(struct ali_state_st *ali_state_st)
{
    
  char params_buf[400];
	//char *str = NULL;
	memset(params_buf,0,sizeof(params_buf));
	
	/*
		sprintf(params_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"{\\\"params\\\":\
	{\\\"dht11_temper\\\":%.2f\\,\\\"dht11_huminity\\\":%.2f\\,\\\"rain\\\":%d\\,\\\"led_state\\\":%d\\,\\\"door_state\\\":%d\\,\\\"motor_state\\\":%d\\}\
	\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",\
	*/
	
	
//	sprintf(params_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"{\\\"params\\\":\
//	{\\\"dht11_temper\\\":%.2f\\,\\\"dht11_huminity\\\":%.2f\\,\\\"rain\\\":%d\\,\\\"led_state\\\":%d\\,\\\"door_state\\\":%d\\,\\\"motor_state\\\":%d\\,\\\"buzzer_state\\\":%d\\,\\\"sonic_state\\\":%d\\,\\\"sonic_distance\\\":%.2f\\}\
//	\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",\
	
//,\\\"b_s\\\":%d\\,\\\"s_s\\\":%d\\,\\\"s_d\\\":%.2f\\
	
	sprintf(params_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"{\\\"params\\\":\
	{\\\"dht11_t\\\":%.2f\\,\\\"dht11_h\\\":%.2f\\,\\\"rain\\\":%d\\,\\\"l_s\\\":%d\\,\\\"d_s\\\":%d\\,\\\"m_s\\\":%d\\,\\\"b_s\\\":%d\\,\\\"s_s\\\":%d\\,\\\"s_d\\\":%.2f\\}\
	\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",\
	ali_state_st->dht11_temper,ali_state_st->dht11_humi,ali_state_st->rain_s,ali_state_st->led_s,ali_state_st->door_s,ali_state_st->motor_s,ali_state_st->buzzer_s,ali_state_st->sonic_s,ali_state_st->sonic_dis);//,ali_state_st->buzzer_s,ali_state_st->sonic_s,ali_state_st->sonic_dis
	//printf("run0\r\n");
	ESP8266_SendCmd(params_buf,"OK"); //这里会涉及等待回复，所以会慢点
	
	//printf("run1\r\n");
//	sprintf(params_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"{\\\"params\\\":\
//	{\\\"sonic_state\\\":%d\\,\\\"buzzer_state\\\":%d\\,\\\"sonic_distance\\\":%.2f\\}\
//	\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",\
//	ali_state_st->sonic_s,ali_state_st->buzzer_s,ali_state_st->sonic_dis);//,ali_state_st->buzzer_s,ali_state_st->sonic_s,ali_state_st->sonic_dis
//	ESP8266_SendCmd(params_buf,"OK"); //这里会涉及等待回复，所以会慢点
	
	
	
	
  ESP8266_Clear(); //清空缓冲区
	//cJSON_Delete(send_cjson);
//  if(str!=NULL){
//   free(str);
//   str = NULL;
//   printf("释放str空间成功\r\n");
//  }
	//char msg_buf[300];
    //char data_value_buf[24];
//    uint16_t move_num = 0;
    //cJSON *send_cjson = NULL;
    
		//char str_test[256];
    //int i=0;
 
    //printf("str = %p\r\n",&str);
 
    //cJSON *params_cjson = NULL;
    //memset(msg_buf,0,sizeof(msg_buf));
		//printf("str1");
    
		//printf("str2");	
    //memset(data_value_buf,0,sizeof(data_value_buf));
		//printf("str3");
    // "{\\\"params\\\":{\\\"temperature\\\":%f\\,\\\"Humidity\\\":%f\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"   因为需要转义字符的存在
		// 原始json数据："{\"params\":{\"temperature\":%f\,\"Humidity\":%f}\,\"version\":\"1.0.0\"}" 变成上面格式的原因是什么，\以及，以及"都需要转义表示
	
//		printf("test point\r\n");
//		//大json
//    send_cjson = cJSON_CreateObject();   // 创建cjson
// 
//    // 构建发送的json，其中一个小对象，包含temperature和humidity
//    params_cjson = cJSON_CreateObject();
 
////============================================== 发送的数据================================================
//    float temp=50;
//		printf("cjson发送数据 temp_value = %d\r\n",temp);
//    printf("cjson发送数据 humi_value = %d\r\n",hum);
		
		//往对象中添加键值对
//    cJSON_AddNumberToObject(params_cjson,"DHT11",temp);
//    //cJSON_AddNumberToObject(params_cjson,"Humidity",hum);
// 
////============================================== 发送的数据================================================
//    // 加入主的json数据中
//    cJSON_AddItemToObject(send_cjson, "params", params_cjson); //这次的值就是一个json对象
//    cJSON_AddItemToObject(send_cjson,"version",cJSON_CreateString("1.0.0"));
//    str = cJSON_PrintUnformatted(send_cjson);
//		//snprintf(str_test,256,"%s",str);
//    printf("json格式 = %s\r\n",str);
//		
//		
// 
//    // 加转义字符
//    for(i=0;*str!='\0';i++)
//    {
//			printf("num:%d\r\n",i);
//			printf("%c\r\n",*str);
//        params_buf[i] = *str;
//        if(*(str+1)=='"'||*(str+1)==',') //下一个字符遇到"或者,加//
//        {
//            params_buf[++i] = '\\';
//        }
//        str++;
//        move_num++;
//    }
//    str = str - move_num;

		
		
//		sprintf(params_buf,"{\\\"params\\\":\
//		{\\\"dht11_temper\\\":%.2f\\,\\\"dht11_huminity\\\":%.2f\\,\\\"rain\\\":%d\\,\\\"led_state\\\":%d\\,\\\"door_state\\\":%d\\,\\\"motor_state\\\":%d\\}\
//		\\,\\\"version\\\":\\\"1.0.0\\\"}",\
//		ali_state_st->dht11_temper,ali_state_st->dht11_humi,ali_state_st->rain_s,ali_state_st->led_s,ali_state_st->door_s,ali_state_st->motor_s);
		
		
    //printf("params_buf = %s\r\n",params_buf);
    // 整理所有数据，关键
    //sprintf((char *)msg_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"%s\",1,0\r\n",params_buf); //相当于这里用宏和%s效果一样，但是宏得用""括起来，否则被视为字符串
    //printf("开始发送数据:%s\r\n",msg_buf);	
}
