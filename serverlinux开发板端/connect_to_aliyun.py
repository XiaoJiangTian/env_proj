# import pika
# import time


# amqp_host = 'iot-06z00a7dl4ogq6o.amqp.iothub.aliyuncs.com'
# amqp_vhost = '/k1m42D6OwDF'
# amqp_username = 'k1m42D6OwDF/mqtt_stm32' 
# amqp_password = '8ff06c2a70783ce92ef2f1ed5facaff4'  

# credentials = pika.PlainCredentials(amqp_username, amqp_password)
# parameters = pika.ConnectionParameters(host=amqp_host,
#                                        virtual_host=amqp_vhost,
#                                        credentials=credentials)

# try:
#     connection = pika.BlockingConnection(parameters)
# except pika.exceptions.AMQPConnectionError as e:
#     print(f"Failed to connect to AMQP service: {e}")
#     exit(1)

# channel = connection.channel()

# exchange_name = 'iot_exchange'
# channel.exchange_declare(exchange=exchange_name, exchange_type='topic')


# routing_key = f'{amqp_username}/user/get'  
# message = 'Hello from AMQP to Aliyun IoT!'
# channel.basic_publish(exchange=exchange_name, routing_key=routing_key, body=message)

# print(f" [x] Sent '{message}'")


# connection.close()

import pika

# AMQP���ӵ�������Ϣ
amqp_host = 'iot-06z00a7dl4ogq6o.amqp.iothub.aliyuncs.com'
amqp_port = 5672
amqp_username = xxx#'LTAI5tF5kvPVHbZU4ZpnM34G'
amqp_password = XXX#'wRCjR9E63s2bMxHB3dtBhXf04gkIIC'
virtual_host = '/k1m42D6OwDF'  # AMQP��������, ͨ����ProductKey���?

# ����AMQP����
credentials = pika.PlainCredentials(amqp_username, amqp_password)
parameters = pika.ConnectionParameters(host=amqp_host,
                                       port=amqp_port,
                                       virtual_host=virtual_host,
                                       credentials=credentials)

# ��������
connection = pika.BlockingConnection(parameters)
channel = connection.channel()

# ������������ʹ��amq.topicΪĬ�Ͻ�������
exchange_name = 'amq.topic'
channel.exchange_declare(exchange=exchange_name, exchange_type='topic')

# ����·�ɼ���ͨ����ProductKey/DeviceName/Topic�ĸ�ʽ��
routing_key = '/k1m42D6OwDF/mqtt_stm32/user/get'

# Ҫ���͵���Ϣ
message = 'Hello, this is a message from AMQP client to Aliyun IoT!'

# ������Ϣ
channel.basic_publish(exchange=exchange_name, routing_key=routing_key, body=message)

print(f" [x] Sent '{message}' to exchange '{exchange_name}' with routing key '{routing_key}'")

# �ر�����
connection.close()
