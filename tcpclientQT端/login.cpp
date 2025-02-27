#include "login.h"
#include "ui_login.h"

login::login(QTcpSocket *s,Widget *w,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    socket = s;
    widget = w;

    //ui编程和cpp以及h文件编程不能混合使用
    //connect(login_pushButton,SIGNAL(clicked()),this,SLOT(on_login_pushButton_clicked)));
    //添加相关槽函数
    //connect(socket,&QTcpSocket::readyRead,this,&login::socket_read_data);
}

login::~login()
{
    delete ui;
}

void login::on_exit_pushButton_clicked()
{
    QString connect_info = "state:0";
    QByteArray trans_connect_info = connect_info.toUtf8();
    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_connect_info);
    } else {
        qDebug() << "Socket is not connected!";
    }

    this->close();
    widget->refresh_state();
    widget->show();
}


void login::on_register_pushButton_clicked()
{
    this->hide();
    registe *c = new registe(socket,this);
    c->show();
}


void login::on_manage_pushButton_clicked()
{
    //监测登录是否成功
    if(widget->login_flag == 1)
    {
        //qt里不太好封装，因为c++是在类的基础上实现的，每个从属于不同的类
        QString account = ui->user_lineEdit->text();
        QString password = ui->password_lineEdit->text();
        QString connect_info =account+":"+password+":goverment";
        QByteArray trans_data = connect_info.toUtf8();
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(trans_data);
        } else {
            qDebug() << "Socket is not connected!";
        }

        this->hide();
        gover *g = new gover(socket,this,widget);
        g->show();
    }
    else
    {
        QMessageBox::warning(this,"管理提示","还未登录");
    }

}


void login::on_login_pushButton_clicked()
{
    //连接具体过程
    QString name = ui->name_lineEdit->text();
    QString account = ui->user_lineEdit->text();
    QString password = ui->password_lineEdit->text();
    if(!QString::compare(account,"") || !QString::compare(password,"") ||!QString::compare(name,""))
    {
        QMessageBox::warning(this,"具体登录提示","名字或账号或密码为空");
        return;
    }

    QString connect_info = name+":"+account+":"+password+":login";
    QByteArray trans_data = connect_info.toUtf8();

    if (socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(trans_data);
    } else {
        qDebug() << "Socket is not connected!";
    }
    //发送完成，接受连接成功信息，是否登录在于widget接收的信息

}

//192.168.184.4
