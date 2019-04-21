#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QMutex>
#include "crc16.h"
#include "query_weight.h"
#include "tare.h"
#include "calibration.h"

class communication : public QObject
{
    Q_OBJECT
public:
    explicit communication(QObject *parent = nullptr);
    QSerialPort *serial;
    query_weight *m_weight;
    tare *m_tare;
    calibration *m_calibration;



public slots:
    void on_open_serial_port_event(QString port_name,int baudrates,int data_bits,int parity);
    void on_close_serial_port_event(QString port_name);

    void on_loop_query_weight_event(void);
    void on_tare_event(int level);
    void on_calibration_event(int level,int calibration_weight);
    void on_wait_rsp_timer_timeout(void);
    void on_rsp_ready();

signals:
    void rsp_open_serial_port_result(int result);
    void rsp_close_serial_port_result(int result);

    void rsp_loop_query_weight_result(int result,int level1,int level2,int level3,int level4);
    void rsp_tare_result(int level,int result);
    void rsp_calibration_result(int level,int calibration_weight,int result);

private:

    QTimer *wait_rsp_timer;
    bool serial_is_busy;
    QMutex *serial_mutex;
    bool serial_opened;
    crc16 *crc;
    int req_level;
    int req_code;
    int req_param;

};

#endif // COMMUNICATION_H
