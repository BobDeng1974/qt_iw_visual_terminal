#include "protocol.h"
#include "qdebug.h"

protocol::protocol()
{
    m_layer_cnt = -1;
    m_door_status = "错误";
    m_lock_status = "错误";
    m_fw_version = "错误";
}

protocol::~protocol()
{

}


/*构建协议数据*/
QByteArray protocol::construct(int code,int value_cnt, int *value,int timeout)
{
    int offset = 0;
    uint16_t crc_cal;
    QByteArray request;
    m_code = code;
    m_timeout = timeout;

    request[offset ++] = 0x01;
    request[offset ++] = code;
    for (int i = 0;i< value_cnt;i++) {
        request[offset ++] = *value ++;
    }
    crc_cal = m_crc.calculate((uint8_t *)request.data(),offset);
    request[offset ++] = crc_cal >> 8;
    request[offset ++] = crc_cal & 0xFF;

    return request;
}


/*解析回应的协议数据*/
int protocol::parse(QByteArray response)
{

    int rc;
    int code,size,value_size;
    uint16_t crc_cal,crc_recv;
    QByteArray temp;

    size = response.size();

    if (size < PROTOCOL_MIN_SIZE) {
        qDebug() << QString("回应字节数量错误");
        return -10;
    }
    if (response.at(0) != 0x01) {
        qDebug() << QString("回应head错误");
        return -11;
    }
    code = (uint8_t)response.at(1);

    if (code != m_code) {
        qDebug("回应code:%d != %d错误",code,m_code);
        return -15;
    }

    crc_cal = m_crc.calculate((uint8_t *)response.data(),size - 2);
    crc_recv = (uint8_t)response[size - 1]  | (uint8_t) response[size - 2] << 8;
    if (crc_cal != crc_recv) {
        qDebug() << QString("回应crc错误") << "cal:" << crc_cal <<"recv:" << crc_recv;
        return -13;
    }
    value_size = size - 1 - 1 - 2;
    switch (code) {
       case PROTOCOL_CODE_REMOVE_TARE:

           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           if (response.at(2) == 1) {
               rc = 0;
           } else {
               rc = -1;
           }
           break;
       case PROTOCOL_CODE_CALIBRATE:
           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           if (response.at(2) == 1) {
               rc = 0;
           } else {
               rc = -1;
           }
           break;
       case PROTOCOL_CODE_NET_WEIGHT:
           if (value_size != 40) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }

           m_weight[0] = (uint8_t)response[2] * 256 + (uint8_t)response[3];
           m_weight[1] = (uint8_t)response[4] * 256 + (uint8_t)response[5];
           m_weight[2] = (uint8_t)response[6] * 256 + (uint8_t)response[7];
           m_weight[3] = (uint8_t)response[8] * 256 + (uint8_t)response[9];
           rc = 0;
           break;
       case PROTOCOL_CODE_LAYER_CNT:
           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           m_layer_cnt = response.at(2);
           rc = 0;
           break;
       case PROTOCOL_CODE_DOOR_STATUS:
           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           if (response.at(2) == 1) {
               m_door_status = "打开";
           } else {
               m_door_status = "关闭";
           }
           rc = 0;
           break;
       case PROTOCOL_CODE_UNLOCK:
           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           if (response.at(2) == 1) {
               rc = 0;
           } else {
               rc = -1;
           }
           break;
       case PROTOCOL_CODE_LOCK:
           if (value_size != 1) {
               qDebug() << QString("回应值数量错误");
               return -15;
           }
           if (response.at(2) == 1) {
               rc = 0;
           } else {
               rc = -1;
           }
           break;
    case PROTOCOL_CODE_LOCK_STATUS:
        if (value_size != 1) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(2) == 1) {
            m_lock_status = "打开";
        } else {
            m_lock_status = "关闭";
        }
        rc = 0;
        break;
    case PROTOCOL_CODE_TEMPERATURE:
        if (value_size != 2) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        m_temperature_setting = (int8_t)response.at(2);
        m_temperature = (int8_t)response.at(3);
        rc = 0;
        break;
    case PROTOCOL_CODE_FW_VERSION:
        if (value_size != 3) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        m_fw_version =QString::number(response.at(2)) + "." + QString::number(response.at(3))+ "." + QString::number(response.at(4));
        rc = 0;
        break;
    case PROTOCOL_CODE_SET_TEMPERATURE:
        if (value_size != 1) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(2) == 1) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    case PROTOCOL_CODE_CTRL_COMPRESSOR:
        if (value_size != 1) {
            qDebug() << QString("回应值数量错误");
            return -15;
        }
        if (response.at(2) == 1) {
            rc = 0;
        } else {
            rc = -1;
        }
        break;
    default:
        rc = -1;
    }

   qDebug("rc = %d",rc);
   return  rc;
}


int protocol::get_code()
{
    return m_code;
}

int protocol::get_timeout()
{
    return m_timeout;

}

int protocol::get_layer_cnt()
{
    return m_layer_cnt;
}

QString protocol::get_fw_version()
{
    return m_fw_version;
}

QString protocol::get_door_status()
{
    return m_door_status;
}

QString protocol::get_lock_status()
{
    return m_lock_status;
}

int protocol::get_temperature(int *setting, int *temperature)
{
    *setting = m_temperature_setting;
    *temperature = m_temperature;

    return 0;
}
int protocol::get_weight(int *weight1, int *weight2, int *weight3, int *weight4)
{
    *weight1 = m_weight[0];
    *weight2 = m_weight[1];
    *weight3 = m_weight[2];
    *weight4 = m_weight[3];
    return 0;
}


























