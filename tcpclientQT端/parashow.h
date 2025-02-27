#ifndef PARASHOW_H
#define PARASHOW_H

#include <QWidget>
#include <QTcpSocket>
#include <widget.h>

namespace Ui {
class parashow;
}

class Widget;

class parashow : public QWidget
{
    Q_OBJECT

public:
    explicit parashow(QTcpSocket *s,Widget *w,QWidget *parent = nullptr);
    ~parashow();

private slots:
    void on_pushButton_clicked();

    void on_request_data_pushButton_clicked();

    void onTimeout_1();

private:
    Ui::parashow *ui;
    QTcpSocket *socket;
    Widget *widget;

};

#endif // PARASHOW_H
