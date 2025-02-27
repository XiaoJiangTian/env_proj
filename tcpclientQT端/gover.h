#ifndef GOVER_H
#define GOVER_H

#include <QWidget>
#include <QTcpSocket>
#include <QStandardItem>
#include "widget.h"
namespace Ui {
class gover;
}

class login;
class Widget;

class gover : public QWidget
{
    Q_OBJECT

public:
    explicit gover(QTcpSocket *s,login *l,Widget *w,QWidget *parent = nullptr);
    ~gover();
    void init_gover_page();
private slots:
    void on_return_pushButton_clicked();

    void on_update_pushButton_clicked();

    void on_clear_pushButton_clicked();

    void on_confirm_pushButton_clicked();

    void on_delete_pushButton_clicked();

private:
    Ui::gover *ui;
    QTcpSocket *socket;
    login *login1;
    QStandardItemModel model;
    Widget *widget;
};

#endif // GOVER_H
