# encoding=utf-8

import time
import sys
import hashlib
import hmac
import base64
import stomp
import ssl
import schedule
import threading
import os

def connect_and_subscribe(conn):
    # ���̴���й¶���ܻᵼ�� AccessKey й¶������в�˺���������Դ�İ�ȫ�ԡ����´���ʾ��ʹ�û���������ȡ AccessKey �ķ�ʽ���е��ã������ο�
    accessKey = "LTAI5tF5kvPVHbZU4ZpnM34G"
    accessSecret = "wRCjR9E63s2bMxHB3dtBhXf04gkIIC"
    consumerGroupId = "DEFAULT_GROUP"
    # iotInstanceId��ʵ��ID��
    iotInstanceId = "iot-06z00a7dl4ogq6o"
    clientId = "linux_stm32_server"
    # ǩ��������֧��hmacmd5��hmacsha1��hmacsha256��
    signMethod = "hmacsha1"
    timestamp = current_time_millis()
    # userName��װ��������μ�AMQP�ͻ��˽���˵���ĵ���
    # ��ʹ�ö����ƴ��䣬��userName��Ҫ���encode=base64����������˻Ὣ��Ϣ��base64����������͡�������ӷ�����μ���һ�½ڡ���������Ϣ��˵������
    username = clientId + "|authMode=aksign" + ",signMethod=" + signMethod \
                    + ",timestamp=" + timestamp + ",authId=" + accessKey \
                    + ",iotInstanceId=" + iotInstanceId \
                    + ",consumerGroupId=" + consumerGroupId + "|"
    signContent = "authId=" + accessKey + "&timestamp=" + timestamp
    # ����ǩ����password��װ��������μ�AMQP�ͻ��˽���˵���ĵ���
    password = do_sign(accessSecret.encode("utf-8"), signContent.encode("utf-8"))
    
    conn.set_listener('', MyListener(conn))
    conn.connect(username, password, wait=True)
    # �����ʷ���Ӽ�������½����Ӽ������
    schedule.clear('conn-check')
    schedule.every(1).seconds.do(do_check,conn).tag('conn-check')

class MyListener(stomp.ConnectionListener):
    def __init__(self, conn):
        self.conn = conn

    def on_error(self, frame):
        print('received an error "%s"' % frame.body)

    def on_message(self, frame):
        print('received a message "%s"' % frame.body)

    def on_heartbeat_timeout(self):
        print('on_heartbeat_timeout')

    def on_connected(self, headers):
        print("successfully connected")
        conn.subscribe(destination='/topic/#', id=1, ack='auto')
        print("successfully subscribe")

    def on_disconnected(self):
        print('disconnected')
        connect_and_subscribe(self.conn)



def current_time_millis():
    return str(int(round(time.time() * 1000)))

def do_sign(secret, sign_content):
    m = hmac.new(secret, sign_content, digestmod=hashlib.sha1)
    return base64.b64encode(m.digest()).decode("utf-8")

# ������ӣ����δ���������½���
def do_check(conn):
    print('check connection, is_connected: %s', conn.is_connected(),flush=True)
    
    if (not conn.is_connected()):
        try:
            connect_and_subscribe(conn)
        except Exception as e:
            print('disconnected, ', e)

# ��ʱ���񷽷����������״̬
def connection_check_timer():
    while 1:
        schedule.run_pending()
        time.sleep(10)
#def main():
    #���￪ʼִ��
    #  ������������μ�AMQP�ͻ��˽���˵���ĵ�������ֱ����������������Ҫ��amqps://ǰ׺
conn = stomp.Connection([('iot-06z00a7dl4ogq6o.amqp.iothub.aliyuncs.com',  61614)], heartbeats=(0,300))
conn.set_ssl(for_hosts=[('iot-06z00a7dl4ogq6o.amqp.iothub.aliyuncs.com',  61614)], ssl_version=ssl.PROTOCOL_TLS)


try:
    connect_and_subscribe(conn)
except Exception as e:
    print('connecting failed')
    raise e
            
        # �첽�߳����ж�ʱ���񣬼������״̬
thread = threading.Thread(target=connection_check_timer)
thread.start()
# if __name__ == "__main__":
#     main()