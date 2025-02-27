#ifndef PARASET_H
#define PARASET_H

#include <QWidget>
#include <QTcpSocket>
#include <widget.h>
namespace Ui {
class paraset;
}

class Widget;

class paraset : public QWidget
{
    Q_OBJECT

public:
    explicit paraset(QTcpSocket *s,Widget *w,QWidget *parent = nullptr);
    ~paraset();
    int8_t check_test(QString temp);
private slots:
    void on_back_pushButton_clicked();

    void on_clear_pushButton_clicked();

    void on_upload_pushButton_clicked();

private:
    Ui::paraset *ui;
    QTcpSocket *socket;
    Widget *widget;
};

#endif // PARASET_H
