#include "registe.h"
#include "ui_registe.h"

registe::registe(QTcpSocket *s,login *l,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::registe)
{
    ui->setupUi(this);
    socket  = s;
    login1 = l;
}

registe::~registe()
{
    delete ui;
}

void registe::on_back_pushButton_clicked()
{
    this->close();
    login1->show();
}


void registe::on_registe_pushButton_clicked()
{
    QString name = ui->name_lineEdit->text();
    QString account  = ui->user_lineEdit->text();
    QString password = ui->password_lineEdit->text();
    if(!QString::compare(account,"") || !QString::compare(password,"")|| !QString::compare(name,""))
    {
        QMessageBox::warning(this,"注册提示","名字或账号或密码为空");
    }
    QString registe_info = name+":"+account+":"+password+":register";
    QByteArray trans_data = registe_info.toUtf8();

    if(socket->state()==QAbstractSocket::ConnectedState)
    {
        socket->write(trans_data);
    }
    else
    {
        qDebug()<<"Socket is not connected!";
    }
}

