#include "parashow.h"
#include "ui_parashow.h"

parashow::parashow(QTcpSocket *s,Widget *w,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::parashow)
{
    ui->setupUi(this);
    socket = s;
    widget = w;
}

parashow::~parashow()
{
    delete ui;
}

void parashow::on_pushButton_clicked()
{
    QString connect_info = "state:0";
    QByteArray trans_connect_info = connect_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_connect_info);
    } else {
        qDebug() << "Socket is not connected!";
    }
    widget->env_rec_flag=0;
    this->close();
    widget->show();
}


void parashow::onTimeout_1()
{

    if(widget->env_rec_flag==1)
    {
        QTimer::singleShot(5000,this,SLOT(onTimeout_1()));

        //唤醒服务器信息
        QString connect_info = "wake";
        QByteArray trans_connect_info = connect_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_connect_info);
        } else {
            qDebug() << "Socket is not connected!";
        }

        ui->rain_lineEdit->setText(widget->env_info_st.rain_state);
        ui->rain_lineEdit->setAlignment(Qt::AlignCenter);

        ui->led_lineEdit->setText(widget->env_info_st.led_state);
        ui->led_lineEdit->setAlignment(Qt::AlignCenter);

        ui->temper_lineEdit->setText(QString(widget->env_info_st.dht11_temper)+"℃");
        ui->temper_lineEdit->setAlignment(Qt::AlignCenter);
        ui->humility_lineEdit->setText(QString(widget->env_info_st.dht11_humility)+"%");
        ui->humility_lineEdit->setAlignment(Qt::AlignCenter);

        ui->door_lineEdit->setText(widget->env_info_st.door_state);
        ui->door_lineEdit->setAlignment(Qt::AlignCenter);

        ui->fan_lineEdit->setText(widget->env_info_st.motor_state);
        ui->fan_lineEdit->setAlignment(Qt::AlignCenter);

        ui->buzzer_lineEdit->setText(widget->env_info_st.buzzer_state);
        ui->buzzer_lineEdit->setAlignment(Qt::AlignCenter);

        ui->sonic_lineEdit->setText(widget->env_info_st.sonic_state);
        ui->sonic_lineEdit->setAlignment(Qt::AlignCenter);
        //QString combine_dis = widget->env_info_st.sonic_distance + "cm";
        ui->sonic_dis_lineEdit->setText(QString(widget->env_info_st.sonic_distance)+"cm");

        QString env_state;
        if(atoi(widget->env_info_st.dht11_temper)<10)
        {
            env_state += "低温";
        }
        else if(atoi(widget->env_info_st.dht11_temper)<25)
        {
            env_state += "常温";
        }
        else if(atoi(widget->env_info_st.dht11_temper)<60)
        {
            env_state += "高温";
        }

        if(atoi(widget->env_info_st.dht11_humility)<30)
        {
            env_state += ":低湿度";
        }
        else if(atoi(widget->env_info_st.dht11_humility)<50)
        {
            env_state += ":正常湿度";
        }
        else if(atoi(widget->env_info_st.dht11_humility)<100)
        {
            env_state += ":高湿度";
        }
        ui->env_lineEdit->setText(env_state);

        if(atoi(widget->env_info_st.sonic_distance)<20)
        {
            ui->safe_lineEdit->setText("不安全");
        }
        else
        {
            ui->safe_lineEdit->setText("安全");
        }

    }
    else
    {
        QMessageBox::warning(this,"获取提示","未正确获取环境信息");
    }
}

void parashow::on_request_data_pushButton_clicked()
{
    QString request_info = "request_data";
    QByteArray trans_request_info = request_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_request_info);
    } else {
        qDebug() << "Socket is not connected!";
    }

    QTimer::singleShot(1000,this,SLOT(onTimeout_1())); //1秒出发一次，接收来自服务器的数据，实时更新
}

