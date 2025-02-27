#include "widget.h"
#include "ui_widget.h"
#include <QNetworkProxy>

#include <QNetworkInterface>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect_flag=0;
    socket = new QTcpSocket(this);//创建socket对象

    //Chat *c = new Chat(socket);

    QString macAddress = "10:FF:E0:20:9E:2E";
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    ui->connect_state_lineedit->setText("Off");
    ui->login_state_lineedit->setText("Off");


    //初始化下位机信息结构体
    env_info_st.rain_state = (char *)malloc(6);
    env_info_st.led_state = (char *)malloc(6);
    env_info_st.door_state = (char *)malloc(6);
    env_info_st.motor_state = (char *)malloc(6);
    env_info_st.dht11_temper = (char *)malloc(6);
    env_info_st.dht11_humility = (char *)malloc(6);

    env_info_st.sonic_distance = (char *)malloc(6);
    env_info_st.sonic_state = (char *)malloc(6);
    env_info_st.buzzer_state = (char *)malloc(6);

    connect(socket,&QTcpSocket::readyRead,this,&Widget::socket_read_data);

    //遍历所有接口并且对比MAC地址
    foreach (const QNetworkInterface &interface, interfaces) {
        QString interfaceMacAddress = interface.hardwareAddress();
//        qDebug() << "Interface with MAC Address" << interfaceMacAddress;
//        qDebug() << "Name:" << interface.name();
        if (interfaceMacAddress == macAddress) {
//            qDebug() << "Interface with MAC Address" << macAddress << "found:";
//            qDebug() << "Name:" << interface.name();
//            qDebug() << "HumanReadableName:" << interface.humanReadableName();

            QList<QNetworkAddressEntry> entries = interface.addressEntries();
            foreach (const QNetworkAddressEntry &entry, entries) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    //qDebug() << "IP Address:" << entry.ip().toString();
                    ui->client_iplineedit->setText(entry.ip().toString());
                }
            }
            return;
        }
    }

}

Widget::~Widget()
{
    delete ui;
}

int Widget::init_sql_st(char *name)
{
    for(int i=0;i<MAX_SQL_NUM;i++)
    {
        //这里不用的要什么时候去释放呢？
        if(sql_info_array[i].flag==0)
        {
            sql_info_array[i].flag = 1;
            sql_info_array[i].name = (char *)malloc(20);
            sql_info_array[i].id = (char *)malloc(10);
            sql_info_array[i].account = (char *)malloc(20);
            sql_info_array[i].password = (char *)malloc(20);
            return i;
        }
        else if(sql_info_array[i].flag==1 && strcmp(name,sql_info_array[i].name)==0)
        {
            return i;
        }
    }
    return -1;
}

//拥有该socket的都用这个函数，
//整个工程网络接受内容相关的都在这里处理
void Widget::socket_read_data()
{
    //处理接受连接数的部分

    QByteArray rec_buffer;
    rec_buffer = socket->readAll();
    char *rec_buffer_char = rec_buffer.data();

    qDebug()<<"widget:"<<tr(rec_buffer);

    if(strcmp(rec_buffer_char,"connected")==0)
    {

        QString client_name = ui->client_name_lineedit->text();
        QString client_ip = ui->client_iplineedit->text();
        QString client_info = client_name+":"+client_ip;
        QByteArray trans_client_info = client_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_client_info);
        } else {
            qDebug() << "Socket is not connected!";
        }
    }

    else if(strcmp(rec_buffer_char,"login:yes")==0)
    {
        login_flag = 1;
        QMessageBox::information(this,"登录提示","成功登录！");
    }
    else if(strcmp(rec_buffer_char,"login:no")==0)
    {
        login_flag = 0;
        QMessageBox::warning(this,"登录提示","错误信息");
    }
    else if(strcmp(rec_buffer_char,"registe:no")==0)
    {
        //login_flag = 0;
        QMessageBox::warning(this,"注册提示","错误信息");
    }
    else if(strcmp(rec_buffer_char,"registe:yes")==0)
    {
        //login_flag = 0;
        QMessageBox::warning(this,"注册提示","注册成功");
    }

    //获取数据库信息
    else if(strncmp(rec_buffer_char,"manage:",7)==0)
    {
        //qDebug()<<"in manage";
        //分解数据并且放入结构体数组
        rec_buffer_char=strtok(rec_buffer_char,"\n");
        strtok(rec_buffer_char,":");
        uint8_t count = 0;
        int16_t pos = 0;
        char *temp_str; //strtok的返回值不需要申请空间
        while ((temp_str=strtok(NULL,":"))!=NULL) {
            //获得空位
            //qDebug()<<temp_str;
            if(count % 4==0)
            {
                count=0;
                pos = init_sql_st(temp_str);
                if(pos == -1)
                {
                    qDebug()<<"no space for client info st";
                    return;
                }
            }
            switch (count) {
                //name
                case 0:
                {
                    //qDebug()<<"0:"<<temp_str;
                    strncpy(sql_info_array[pos].name,temp_str,strlen(temp_str)+1);
                }
                break;
                //id
                case 1:
                {
                    //qDebug()<<"1:"<<temp_str;
                    strncpy(sql_info_array[pos].id,temp_str,strlen(temp_str)+1);
                }
                break;
                //account
                case 2:
                {
                    //qDebug()<<"2:"<<temp_str;
                    strncpy(sql_info_array[pos].account,temp_str,strlen(temp_str)+1);
                }
                break;
                //password
                case 3:
                {
                    //qDebug()<<"3:"<<temp_str;
                    strncpy(sql_info_array[pos].password,temp_str,strlen(temp_str)+1);
                }
                break;
            }
            count++;


        }
        manage_flag = 1;
    }
    //这好像也不会出？
    else if(strcmp(rec_buffer_char,"goverment:no")==0)
    {
        QMessageBox::information(this,"修改提示","该人员不存在");
    }

    else if(strncmp(rec_buffer_char,"return_data",11)==0)
    {
        //接收到回复信息 雨，led，dht11温度，门状态，电机状态，dht11湿度
        char temp_st[9][6];
        char *temp_str;
        strtok(rec_buffer_char,":");
        for(int i=0;i<9;i++)
        {
            temp_str = strtok(NULL,":");
            if(temp_str==NULL)
            {
                QMessageBox::warning(this,"接收数据错误","接收数据不全");
                return;
            }
            strncpy(temp_st[i],temp_str,strlen(temp_str)+1);
        }
        strncpy(env_info_st.motor_state,temp_st[0],strlen(temp_st[0])+1);
        strncpy(env_info_st.led_state,temp_st[1],strlen(temp_st[1])+1);
        strncpy(env_info_st.rain_state,temp_st[2],strlen(temp_st[2])+1);
        strncpy(env_info_st.dht11_temper,temp_st[3],strlen(temp_st[3])+1);
        strncpy(env_info_st.sonic_state,temp_st[4],strlen(temp_st[4])+1);
        strncpy(env_info_st.buzzer_state,temp_st[5],strlen(temp_st[5])+1);
        strncpy(env_info_st.sonic_distance,temp_st[6],strlen(temp_st[6])+1);

        strncpy(env_info_st.door_state,temp_st[7],strlen(temp_st[7])+1);
        strncpy(env_info_st.dht11_humility,temp_st[8],strlen(temp_st[8])+1);
        env_rec_flag = 1;
    }
    else if(strcmp(rec_buffer_char,"empty_data")==0)
    {
        QMessageBox::warning(this,"request_data","数据未更新");
    }
}



void Widget::on_exit_pushButton_clicked()
{
    this->close();
}


void Widget::on_connect_pushButton_clicked()
{

    QString client_name = ui->client_name_lineedit->text();
    QString client_ip = ui->client_iplineedit->text();

    QString ip = ui->iplineedit->text();
    QString port = ui->portlineedit->text();

    if(!QString::compare(client_ip,"") || !QString::compare(client_name,"") || !QString::compare(ip,"") || !QString::compare(ip,""))
    {
        QMessageBox::warning(this,"连接警告","请完整填充客户端名称、客户端IP地址、服务器IP地址、服务器端口号");
        return;
    }

    socket->setProxy(QNetworkProxy::NoProxy);
    socket->connectToHost(ip,port.toUInt());
    //捕获连接成功的信号
    connect(socket,&QTcpSocket::connected,[this]()
    {
        if(connect_flag==0)
        {
            QMessageBox::information(this,"连接提示","连接服务器成功");
        }
        //在这里进行页面切换
        connect_flag = 1;
        ui->connect_state_lineedit->setText("On");

    });
    //连接断开，通过创建捕获信号连接
    connect(socket,&QTcpSocket::disconnected,[this]()
    {
        if(connect_flag==1)
        {
            QMessageBox::warning(this,"连接提示","连接异常，网络断开");
        }
        connect_flag=0;
        ui->connect_state_lineedit->setText("Off");
    });

}



void Widget::on_cancel_connect_pushButton_clicked()
{
    socket->disconnectFromHost();
    if (socket->state() == QAbstractSocket::UnconnectedState || socket->waitForDisconnected(3000)) {
        qDebug() << "Disconnected from server";
    }
}


void Widget::on_login_pushButton_clicked()
{
    if(connect_flag==1)//连接成功后再进入登录界面
    {
        QString connect_info = "state:1";
        QByteArray trans_connect_info = connect_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_connect_info);
        } else {
            qDebug() << "Socket is not connected!";
        }

        login *c = new login(socket,this);
        this->hide();
        c->show(); //进入登录界面
    }
    else
    {
        QMessageBox::warning(this,"登录提示","服务器还未连接");
    }
}


void Widget::onTimeout()
{
    QString request_info = "request_data";
    QByteArray trans_request_info = request_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_request_info);
    } else {
        qDebug() << "Socket is not connected!";
    }
}
//参数获取界面
void Widget::on_parameter_get_pushButton_clicked()
{
    //需要加检测登录成功的逻辑，才能进入后面的参数显示部分
    if(login_flag==1)
    {
        QString connect_info = "state:2\n";
        QByteArray trans_connect_info = connect_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_connect_info);
        } else {
            qDebug() << "Socket is not connected!";
        }


        //QThread::msleep(500);
        //这可能有问题
        QTimer::singleShot(2000,this,SLOT(onTimeout()));
//        connect_info = "request_data";
//        trans_connect_info = connect_info.toUtf8();
//        QString request_info = "request_data";
//        QByteArray trans_request_info = request_info.toUtf8();
//        if (socket->state() == QAbstractSocket::ConnectedState) {
//            socket->write(trans_connect_info);
//        } else {
//            qDebug() << "Socket is not connected!";
//        }

        parashow *p = new parashow(socket,this);
        this->hide();
        p->show();
    }
    else
    {
        QMessageBox::warning(this,"参数获取或设置","请先完成登录");
    }

}



void Widget::refresh_state()
{
    if(login_flag==1)
    {
        ui->login_state_lineedit->setText("On");
    }
}


//参数设置界面
void Widget::on_parameter_set_pushButton_clicked()
{

    if(login_flag==1)
    {
        QString connect_info = "state:2";
        QByteArray trans_connect_info = connect_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_connect_info);
        } else {
            qDebug() << "Socket is not connected!";
        }

        //需要加检测登录成功的逻辑，才能进入后面的参数设置部分
        paraset *p = new paraset(socket,this);
        this->hide();
        p->show();
    }
    else
    {
        QMessageBox::warning(this,"参数获取或设置","请先完成登录");
    }

}

