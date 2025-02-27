#ifndef REGISTE_H
#define REGISTE_H

#include <QWidget>
#include <QTcpSocket>
#include <login.h>
namespace Ui {
class registe;
}

class login;

class registe : public QWidget
{
    Q_OBJECT

public:
    explicit registe(QTcpSocket *s,login *l,QWidget *parent = nullptr);
    ~registe();

private slots:
    void on_back_pushButton_clicked();

    void on_registe_pushButton_clicked();

private:
    Ui::registe *ui;
    QTcpSocket *socket;
    login *login1;
};

#endif // REGISTE_H
