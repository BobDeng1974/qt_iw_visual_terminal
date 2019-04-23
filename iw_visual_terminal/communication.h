#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QMutex>
#include "crc16.h"
#include "qqueue.h"
#include "req_param.h"


class communication : public QObject
{
    Q_OBJECT
public:
    explicit communication(QObject *parent = nullptr);
    QSerialPort *m_serial;
    enum {
        FRAME_TIMEOUT = 5,
        REQ_TIMEOUT = 5,
        RSP_TIMEOUT = 50,
        RSP_LOCK_TIMEOUT = 1000,
        RSP_UNLOCK_TIMEOUT = 1000,
        PERIOD_TIMEOUT = 5

    };

    enum {
        REQ_CODE_TARE = 1,
        REQ_CODE_CALIBRATION = 2,
        REQ_CODE_QUERY_WEIGHT = 3,
        REQ_CODE_QUERY_DOOR_STATUS = 0x11,
        REQ_CODE_UNLOCK = 0x21,
        REQ_CODE_LOCK = 0x22,
        REQ_CODE_QUERY_LOCK_STATUS = 0x23,
        REQ_CODE_QUERY_TEMPERATURE = 0x41
    };
    void handle_query_weight_req(void);

public slots:
    void handle_open_serial_port_req(QString port_name,int baudrates,int data_bits,int parity);
    void handle_close_serial_port_req(QString port_name);

    void handle_req_timeout_event(void);
    void handle_rsp_timeout_event(void);
    void handle_query_weight_timeout_event(void);
    void handle_frame_timeout_event();

    void handle_rsp_ready_event(void);

    void handle_tare_req(int);
    void handle_calibration_req(int,int);
    void handle_req_unlock();
    void handle_req_lock();
    void handle_query_lock_status();
    void handle_query_door_status();
    void handle_query_temperature();

signals:

    void rsp_open_serial_port_result(int result);
    void rsp_close_serial_port_result(int result);

    void rsp_query_weight_result(int result,int level,int value);
    void rsp_tare_result(int level,int result);
    void rsp_calibration_result(int level,int calibration_weight,int result);

    void rsp_unlock_result(int);
    void rsp_lock_result(int);
    void rsp_query_lock_status(QString status);
    void rsp_query_door_status(QString status);
    void rsp_query_temperature(int);

private:
    QQueue<req_param> *m_req_queue;
    QTimer *m_req_timer;
    QTimer *m_rsp_timer;
    QTimer *m_period_timer;
    QTimer *m_frame_timer;


    bool   m_opened;
    crc16 *m_crc;
    bool   m_busy;

    int req_level;
    int req_code;
    int req_weight;
    uint8_t rsp[100];
    int rsp_size;

};

#endif // COMMUNICATION_H
