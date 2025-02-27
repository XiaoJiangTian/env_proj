#include "paraset.h"
#include "ui_paraset.h"

paraset::paraset(QTcpSocket *s,Widget *w,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::paraset)
{
    ui->setupUi(this);
    socket = s;
    widget = w;
}

paraset::~paraset()
{
    delete ui;
}

void paraset::on_back_pushButton_clicked()
{
    QString connect_info = "state:0";
    QByteArray trans_connect_info = connect_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_connect_info);
    } else {
        qDebug() << "Socket is not connected!";
    }
    this->close();
    widget->show();
}


//清空数据
void paraset::on_clear_pushButton_clicked()
{
    ui->sonic_state_lineEdit->clear();
    ui->buzzer_state_lineEdit->clear();
    ui->led_state_lineEdit->clear();
    ui->door_state_lineEdit->clear();
    ui->fan_state_lineEdit->clear();
}


int8_t paraset::check_test(QString temp)
{
    if(QString::compare(temp,"On") && QString::compare(temp,"Off") && QString::compare(temp,""))
    {
        QMessageBox::warning(this,"参数错误","sonic参数错误");
        return 0;
    }
    return 1;
}

void paraset::on_upload_pushButton_clicked()
{
    //bool empty_flag[5];
    QString sonic_s = ui->sonic_state_lineEdit->text();
    if(check_test(sonic_s)==0)
    {
        return;
    }
    QString buzzer_s = ui->buzzer_state_lineEdit->text();
    if(check_test(buzzer_s)==0)
    {
        return;
    }
    QString led_s = ui->led_state_lineEdit->text();
    if(check_test(led_s)==0)
    {
        return;
    }
    QString door_s = ui->door_state_lineEdit->text();
    if(check_test(door_s)==0)
    {
        return;
    }

    QString fan_s = ui->fan_state_lineEdit->text();
    if(QString::compare(fan_s,"Fast") && QString::compare(fan_s,"Off") && QString::compare(fan_s,"Slow")&& QString::compare(fan_s,""))
    {
        QMessageBox::warning(this,"参数错误","sonic参数错误");
        return;
    }



    if(!QString::compare(sonic_s,"")&&!QString::compare(buzzer_s,"")&&!QString::compare(led_s,"")&&!QString::compare(door_s,"")&&!QString::compare(fan_s,""))
    {
        QMessageBox::warning(this,"上传错误","请至少填充一项控制信息");
        return;
    }

    QString connect_info = "set_data:"+(!QString::compare(sonic_s,"")?"empty":sonic_s)+":"+(!QString::compare(buzzer_s,"")?"empty":buzzer_s)+":"\
            +(!QString::compare(led_s,"")?"empty":led_s)+":"+(!QString::compare(door_s,"")?"empty":door_s)+":"+(!QString::compare(fan_s,"")?"empty":fan_s);
    qDebug()<<connect_info;
    QByteArray trans_connect_info = connect_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_connect_info);
    } else {
        qDebug() << "Socket is not connected!";
    }

}

