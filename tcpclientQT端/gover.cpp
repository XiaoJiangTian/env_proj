#include "gover.h"
#include "ui_gover.h"

gover::gover(QTcpSocket *s,login *l,Widget *w,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::gover)
{
    ui->setupUi(this);
    ui->tableView->setModel(&model);
    socket = s;
    login1 = l;
    widget = w;
//    while(widget->manage_flag==0)
//    {

//    }
    init_gover_page();
}

gover::~gover()
{
    delete ui;
}

void gover::init_gover_page()
{
    model.clear();
    model.setHorizontalHeaderLabels(QStringList{"姓名","编号","账号","密码"});

    //获得数据库信息并且显示

    //做个刷新按键


    qDebug()<<"run over";

//    for(int i=0;i<STAFFSIZE;i++)
//    {
//        if(staffArray[i].useflag==1)
//        {
//            QList<QStandardItem *>items;
//            items.append(new QStandardItem(staffArray[i].id));
//            items.append(new QStandardItem(staffArray[i].name));
//            //tempmoney=strtok(NULL,"/");
//            items.append(new QStandardItem(staffArray[i].age));
//            //temptxt = strtok(NULL,"/");
//            items.append(new QStandardItem(staffArray[i].gender));
//            items.append(new QStandardItem(staffArray[i].salary));
//            //根据数组显示，并且写回文件（全部写回）
//            fprintf(stafffile,"%s/%s/%s/%s/%s/\n",staffArray[i].id,staffArray[i].name,staffArray[i].age,staffArray[i].gender,staffArray[i].salary);

//            model.appendRow(items);
//        }
//    }
}

void gover::on_return_pushButton_clicked()
{
    this->close();
    login1->show();
}


void gover::on_update_pushButton_clicked()
{
    model.clear();
    model.setHorizontalHeaderLabels(QStringList{"姓名","编号","账号","密码"});
    for(int i=0;i<MAX_SQL_NUM;i++)
    {
        if(widget->sql_info_array[i].flag==1)
        {
            widget->sql_info_array[i].flag=0;
            QList<QStandardItem *>items;
            items.append(new QStandardItem(widget->sql_info_array[i].name));
            items.append(new QStandardItem(widget->sql_info_array[i].id));
            //tempmoney=strtok(NULL,"/");
            items.append(new QStandardItem(widget->sql_info_array[i].account));
            //temptxt = strtok(NULL,"/");
            items.append(new QStandardItem(widget->sql_info_array[i].password));

            //根据数组显示，并且写回文件（全部写回）
            //fprintf(stafffile,"%s/%s/%s/%s/%s/\n",staffArray[i].id,staffArray[i].name,staffArray[i].age,staffArray[i].gender,staffArray[i].salary);
            model.appendRow(items);
            free(widget->sql_info_array[i].name);
            free(widget->sql_info_array[i].id);
            free(widget->sql_info_array[i].account);
            free(widget->sql_info_array[i].password);
        }
    }
}


//清除所以填充信息
void gover::on_clear_pushButton_clicked()
{
    ui->name_lineEdit->clear();
    ui->serial_lineEdit->clear();
    ui->account_lineEdit->clear();
    ui->password_lineEdit->clear();
}


void gover::on_confirm_pushButton_clicked()
{
    QString change_name = ui->name_lineEdit->text();
    QString change_id = ui->serial_lineEdit->text();
    QString change_account = ui->account_lineEdit->text();
    QString change_password = ui->password_lineEdit->text();

    if(!QString::compare(change_name,"") ||!QString::compare(change_id,"") ||!QString::compare(change_account,"") ||!QString::compare(change_password,"") )
    {
        QMessageBox::warning(this,"修改警告","请完善修改人和修改内容");
        return;
    }

    QString goverment_info = change_name+":"+change_id+":"+change_account+":"+change_password+":goverment_change";
    QByteArray trans_goverment_info = goverment_info.toUtf8();

    if(socket->state()==QAbstractSocket::ConnectedState)
    {
        socket->write(trans_goverment_info);
    }
    else
    {
        qDebug()<<"Socket is not connected!";
    }
}


//update login_info set id='3', account='pass', password='pass' where name='vn'

void gover::on_delete_pushButton_clicked()
{
    QString change_name = ui->name_lineEdit->text();
    if(!QString::compare(change_name,""))
    {
        QMessageBox::warning(this,"删除警告","请确定删除人名字");
        return;
    }

    QString goverment_info = change_name+":goverment_delete";
    QByteArray trans_goverment_info = goverment_info.toUtf8();

    if(socket->state()==QAbstractSocket::ConnectedState)
    {
        socket->write(trans_goverment_info);
    }
    else
    {
        qDebug()<<"Socket is not connected!";
    }
}

