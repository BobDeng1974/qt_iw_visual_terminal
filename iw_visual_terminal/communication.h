#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QMutex>
#include "crc16.h"
#include "qqueue.h"
#include "protocol.h"


class communication : public QObject
{
    Q_OBJECT
public:
    explicit communication(QObject *parent = nullptr);
    QSerialPort *m_serial;
    enum {
        LOOP_TIMEOUT = 5,
        FRAME_TIMEOUT = 5
    };

    enum {
        QUERY_WEIGHT_DUCY_MULTIPLE = 4
    };

    QStringList get_port_name_list();
    int open_serial(QString port_name,int baud_rates,int data_bits,int parity);
    int close_serial(QString port_name);
    int is_serial_open();

    int remove_tare_weight(int addr);
    int calibrate_weight(int addr,int weight);
    int set_temperature(int setting);
    int lock_lock();
    int unlock_lcok();
    int query_net_weight(int *weight1,int *weight2,int *weight3,int *weight4);
    int query_temperature(int *setting ,int *temperature);
    QString query_door_status();
    QString query_lock_status();
    int query_layer_cnt();
    QString query_fw_version();

public slots:


signals:


private:
    int send_request(QByteArray);
    QByteArray wait_response(int timeout);
    int lock_serial_mutex();
    int unlock_serial_mutex();
    QMutex m_serial_mutex;


};

#endif // COMMUNICATION_H
