#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QTcpSocket>
#include "widget.h"
#include "registe.h"
#include "gover.h"
#include <QMessageBox>
namespace Ui {
class login;
}
class Widget;

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QTcpSocket *s,Widget *w,QWidget *parent = nullptr);
    ~login();

private slots:
    void on_exit_pushButton_clicked();

    void on_register_pushButton_clicked();

    void on_manage_pushButton_clicked();

    void on_login_pushButton_clicked();

    //void socket_read_data();

private:
    Ui::login *ui;
    QTcpSocket *socket;
    Widget *widget;
};

#endif // LOGIN_H
