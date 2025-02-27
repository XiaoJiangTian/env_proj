#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

//my include
#include <QHostAddress>
#include <QMessageBox>
#include <login.h>
#include <parashow.h>
#include <paraset.h>
#include <string.h>

#include <QTimer>
#include <QThread>
QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE


#define MAX_SQL_NUM  1000

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    bool connect_flag;
    bool login_flag=0;
    bool manage_flag = 0;
    bool env_rec_flag = 0;
    void refresh_state();
    int init_sql_st(char *name);
    struct sql_info
    {
        char *name;
        char *id;
        char *account;
        char *password;
        bool flag=0;
    };

    struct env_info
    {
        char *rain_state;
        char *led_state;
        char *dht11_temper;
        char *door_state;
        char *motor_state;
        char *dht11_humility;

        char *buzzer_state;
        char *sonic_state;
        char *sonic_distance;
    };

    struct env_info env_info_st;
    struct sql_info sql_info_array[MAX_SQL_NUM];

private slots:
    void on_exit_pushButton_clicked();

    void on_connect_pushButton_clicked();

    void on_cancel_connect_pushButton_clicked();

    void on_login_pushButton_clicked();

    void on_parameter_get_pushButton_clicked();

    void on_parameter_set_pushButton_clicked();

    void socket_read_data();

    void onTimeout();
private:
    Ui::Widget *ui;
    QTcpSocket *socket;
};
#endif // WIDGET_H
