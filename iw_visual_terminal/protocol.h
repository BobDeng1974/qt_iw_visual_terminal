#ifndef REQ_PARAM_H
#define REQ_PARAM_H

#include "qbytearray.h"
#include "qstring.h"
#include "crc16.h"

class protocol
{
public:
    explicit protocol();
    ~protocol();
    enum {
        PROTOCOL_CODE_REMOVE_TARE = 0x01,
        PROTOCOL_CODE_CALIBRATE = 0x02,
        PROTOCOL_CODE_NET_WEIGHT = 0x03,
        PROTOCOL_CODE_LAYER_CNT = 0x04,
        PROTOCOL_CODE_DOOR_STATUS = 0x11,
        PROTOCOL_CODE_UNLOCK = 0x21,
        PROTOCOL_CODE_LOCK = 0x22,
        PROTOCOL_CODE_LOCK_STATUS = 0x23,
        PROTOCOL_CODE_TEMPERATURE = 0x41,
        PROTOCOL_CODE_FW_VERSION = 0x52,
        PROTOCOL_CODE_SET_TEMPERATURE = 0x0A,
    };
    enum {
        PROTOCOL_REMOVE_TARE_TIMEOUT = 600,
        PROTOCOL_CALIBRATE_TIMEOUT = 600,
        PROTOCOL_WEIGHT_TIMEOUT = 60,
        PROTOCOL_LAYER_TIMEOUT = 60,
        PROTOCOL_DOOR_STATUS_TIMEOUT = 60,
        PROTOCOL_UNLOCK_TIMEOUT = 1100,
        PROTOCOL_LOCK_TIMEOUT = 1100,
        PROTOCOL_LOCK_STATUS_TIMEOUT = 60,
        PROTOCOL_TEMPERATURE_TIMEOUT = 60,
        PROTOCOL_FW_VERSION_TIMEOUT = 60,
        PROTOCOL_SET_TEMPERATURE_TIMEOUT = 600,
    };
    enum {
        PROTOCOL_MIN_SIZE = 5

    };

    QByteArray construct(int code,int value_cnt,int *value,int timeout);
    int parse(QByteArray response);
    int get_code();
    int get_timeout();
    int get_temperature(int *setting,int *temperature);
    int get_layer_cnt();
    QString get_fw_version();
    QString get_door_status();
    QString get_lock_status();
    int get_weight(int *weight1,int *weight2,int *weight3,int *weight4);

private:
    int m_code;
    int m_timeout;

    QString m_door_status;
    QString m_lock_status;
    QString m_fw_version;

    int m_temperature_setting;
    int m_temperature;
    int m_layer_cnt;

    int m_weight[4];
    crc16 m_crc;
};

#endif // REQ_PARAM_H
