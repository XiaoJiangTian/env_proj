#include "esp8266.h"


#define      WIFI_NAME      "7erkeyT"       // wifi��
#define      WIFI_PASS      "txjandtsh"       // wifi����
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
//  if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE) != RESET)    // �����жϵı�־λ
//  {
//    HAL_UART_DMAStop(&huart2);                               //ֹͣ����
//    esp_cnt = ESPBUFF_MAX_SIZE - __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);    // ������յ����ݳ���
//    HAL_UART_Transmit(&huart1,esp_buff,esp_cnt,1000);
////    printf("rec = %s\r\n",esp_buff);
//    HAL_UART_Receive_DMA(&huart2,esp_buff,ESPBUFF_MAX_SIZE);         // ����DMA��������
//    __HAL_UART_CLEAR_IDLEFLAG(&huart2);
//  }
//  /* USER CODE END USART2_IRQn 1 */
//}

uint8_t ESP8266_SendCmd(char *cmd,char *res);

//����wifi
void ESP8266_Init(void)
{
    HAL_UART_Receive_DMA(&huart2,esp_buff,ESPBUFF_MAX_SIZE);    // ����DMA����
    __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);      // �������ڵĿ����ж�
    while(ESP8266_SendCmd("AT+RST\r\n", "ready")) //��λ���ȴ�Ŀ����Ϣ
    while(ESP8266_SendCmd("AT\r\n","OK")){} //AT
    while(ESP8266_SendCmd("AT+CWMODE=1\r\n","OK")){}//����ģʽ
    //����wifi�ȵ�
    while(ESP8266_SendCmd("AT+CWJAP=\""WIFI_NAME"\",\""WIFI_PASS"\"\r\n","OK")){}//����wifi
    // ����   
    //printf("success");
}

void ESP8266_Clear()
{
    memset(esp_buff,0,sizeof(esp_buff));
    esp_cnt = 0;
}

//���ڣ�ʹ�üĴ���������������
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
    // ����ָ��
    ESP8266_SendString(cmd,strlen((const char *)cmd));
    while(num--)
    {
        if(strstr((const char*)esp_buff,(const char *)res)!=NULL)//���鷵��ֵ
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
//    // ����ָ��
//    ESP8266_SendString(cmd,strlen((const char *)cmd));
////    while(num--)
////    {
////        if(strstr((const char*)esp_buff,(const char *)res)!=NULL)//���鷵��ֵ
////        {
////            ESP8266_Clear();
////            return 0;
////        }
////        HAL_Delay(10);
////    }
//		ESP8266_Clear();
//    return 0;
//}



//���Ӱ����Ʋ���


#define      ALI_USERNAME       "mqtt_stm32&k1m42D6OwDF"                                         // �û���
#define      ALICLIENTLD            "esp8266|securemode=3\\,signmethod=hmacsha1\\,timestamp=100|"                // �ͻ�id,������б�ܣ���һ������ת��
#define      ALI_PASSWD         "5956FC8C37143ED42BF7A391063BE39CA706230D"           // MQTT ����
#define      ALI_MQTT_HOSTURL   "k1m42D6OwDF.iot-as-mqtt.cn-shanghai.aliyuncs.com"            // mqtt���ӵ���ַ
#define      ALI_PORT           "1883"                // �˿�
 


//������Ϊ����
#define      ALI_TOPIC_SET          "/k1m42D6OwDF/mqtt_stm32/user/get" //set���ڽ��ܣ�����������豸���ԣ�
#define      ALI_TOPIC_POST         "/k1m42D6OwDF/mqtt_stm32/user/env_info" //post���ڷ���
#define      ALI_TOPIC_POST1         "/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post" //post���ڷ���
#define      ALI_TOPIC_POST_RK3568   "/k1m42D6OwDF/mqtt_stm32/user/rec_rk3568_set" //��������rk��������Ϣ
#define      ALI_TOPIC_POST_CANCEL		"/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post_reply"

void Ali_Yun_Topic(void);
void Ali_Yun_Init(void)
{
    //�����û���������
    while(ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\"NULL\",\""ALI_USERNAME"\",\""ALI_PASSWD"\",0,0,\"\"\r\n","OK")){} //""������ݴ����ַ��������������ʾ����"����ת���ַ�\
    HAL_Delay(10);
    // ���ÿͷ�id
    while(ESP8266_SendCmd("AT+MQTTCLIENTID=0,\""ALICLIENTLD"\"\r\n","OK")){}
 
    // ���Ӱ�����  AT+MQTTCONN=0,"iot-06z00b28nanp9ew.mqtt.iothub.aliyuncs.com",1883,1
    while(ESP8266_SendCmd("AT+MQTTCONN=0,\""ALI_MQTT_HOSTURL"\",1883,1\r\n","OK")){}
	
		//���˰�������վ���豸�Ѿ�����
    Ali_Yun_Topic();
}
 
void Ali_Yun_Topic(void)
{
    //"AT+MQTTPUB=0,\"����������\",\""; //SUB�Ƕ������⣬�����ܰ����Ƶ���Ϣ��PUB���ϴ����⣬���͸�������
		//while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_SET"\",1\r\n","OK")){}//��һ��0���豸�������ڶ���1�ǰ汾����
			
		while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_POST_RK3568"\",1\r\n","OK")){}//����rk�����⣬��������rk��������Ϣ
		
		
		
		//while(ESP8266_SendCmd("AT+MQTTSUB=0,\""ALI_TOPIC_POST_CANCEL"\",1\r\n","OK")){}//ȡ������
		
		//while(ESP8266_SendCmd("AT+MQTTUNSUB=0,\""ALI_TOPIC_POST_CANCEL"\"\r\n","OK")){}//ȡ������
			
		//while(ESP8266_SendCmd("AT+MQTTUNSUB=0,\"/sys/k1m42D6OwDF/mqtt_stm32/thing/event/property/post_reply\"\r\n", "OK")){}

 
    while(ESP8266_SendCmd("AT+MQTTPUB=0,\""ALI_TOPIC_POST"\",\"test info from txj\",1,0\r\n","OK")){}
}


//�����������ϴ�
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
	ESP8266_SendCmd(params_buf,"OK"); //������漰�ȴ��ظ������Ի�����
	
	//printf("run1\r\n");
//	sprintf(params_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"{\\\"params\\\":\
//	{\\\"sonic_state\\\":%d\\,\\\"buzzer_state\\\":%d\\,\\\"sonic_distance\\\":%.2f\\}\
//	\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",\
//	ali_state_st->sonic_s,ali_state_st->buzzer_s,ali_state_st->sonic_dis);//,ali_state_st->buzzer_s,ali_state_st->sonic_s,ali_state_st->sonic_dis
//	ESP8266_SendCmd(params_buf,"OK"); //������漰�ȴ��ظ������Ի�����
	
	
	
	
  ESP8266_Clear(); //��ջ�����
	//cJSON_Delete(send_cjson);
//  if(str!=NULL){
//   free(str);
//   str = NULL;
//   printf("�ͷ�str�ռ�ɹ�\r\n");
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
    // "{\\\"params\\\":{\\\"temperature\\\":%f\\,\\\"Humidity\\\":%f\\}\\,\\\"version\\\":\\\"1.0.0\\\"}"   ��Ϊ��Ҫת���ַ��Ĵ���
		// ԭʼjson���ݣ�"{\"params\":{\"temperature\":%f\,\"Humidity\":%f}\,\"version\":\"1.0.0\"}" ��������ʽ��ԭ����ʲô��\�Լ����Լ�"����Ҫת���ʾ
	
//		printf("test point\r\n");
//		//��json
//    send_cjson = cJSON_CreateObject();   // ����cjson
// 
//    // �������͵�json������һ��С���󣬰���temperature��humidity
//    params_cjson = cJSON_CreateObject();
 
////============================================== ���͵�����================================================
//    float temp=50;
//		printf("cjson�������� temp_value = %d\r\n",temp);
//    printf("cjson�������� humi_value = %d\r\n",hum);
		
		//����������Ӽ�ֵ��
//    cJSON_AddNumberToObject(params_cjson,"DHT11",temp);
//    //cJSON_AddNumberToObject(params_cjson,"Humidity",hum);
// 
////============================================== ���͵�����================================================
//    // ��������json������
//    cJSON_AddItemToObject(send_cjson, "params", params_cjson); //��ε�ֵ����һ��json����
//    cJSON_AddItemToObject(send_cjson,"version",cJSON_CreateString("1.0.0"));
//    str = cJSON_PrintUnformatted(send_cjson);
//		//snprintf(str_test,256,"%s",str);
//    printf("json��ʽ = %s\r\n",str);
//		
//		
// 
//    // ��ת���ַ�
//    for(i=0;*str!='\0';i++)
//    {
//			printf("num:%d\r\n",i);
//			printf("%c\r\n",*str);
//        params_buf[i] = *str;
//        if(*(str+1)=='"'||*(str+1)==',') //��һ���ַ�����"����,��//
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
    // �����������ݣ��ؼ�
    //sprintf((char *)msg_buf,"AT+MQTTPUB=0,\""ALI_TOPIC_POST1"\",\"%s\",1,0\r\n",params_buf); //�൱�������ú��%sЧ��һ�������Ǻ����""��������������Ϊ�ַ���
    //printf("��ʼ��������:%s\r\n",msg_buf);	
}
