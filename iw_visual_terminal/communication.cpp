#include "qstring.h"
#include "communication.h"
#include "QtSerialPort/qserialport.h"
#include "QtSerialPort/QSerialPortInfo"
#include "qthread.h"
#include "QTime"

communication::communication(QObject *parent) : QObject(parent)
{
    m_serial = new QSerialPort(parent);
}

QStringList communication::get_port_name_list()
{
    QStringList m_serialPortName;

    QList<QSerialPortInfo>  serial_info;
    QSerialPortInfo info;

    serial_info = QSerialPortInfo::availablePorts();
    m_serialPortName.clear();

     foreach(info,serial_info) {
         m_serialPortName.append(info.portName());
     }
    return m_serialPortName;
}


/*打开串口*/
int communication::open_serial(QString port_name,int baudrates,int data_bits,int parity)
{

    bool success;

    m_serial->setPortName(port_name);
    if (m_serial->isOpen()) {
        return 0;
    }

    m_serial->setBaudRate(baudrates);
    if (data_bits == 8) {
        m_serial->setDataBits(QSerialPort::Data8);
    } else {
       m_serial->setDataBits(QSerialPort::Data7);
    }

    if (parity == 0){
        m_serial->setParity(QSerialPort::NoParity);
    } else if (parity == 1){
        m_serial->setParity(QSerialPort::OddParity);
    } else  {
         m_serial->setParity(QSerialPort::EvenParity);
    }

    success = m_serial->open(QSerialPort::ReadWrite);
    if (success) {
        qDebug("打开串口成功.");
        return 0;
    }
    qWarning("打开串口失败.");
    return -1;
}


/*关闭串口*/
int communication::close_serial(QString port_name)
{
    (void) port_name;

    m_serial->flush();
    m_serial->close();
    return 0;
}

int communication::is_serial_open()
{
    return m_serial->isOpen();
}


int communication::send_request(QByteArray request)
{
    m_serial->write(request);

    qDebug("request:");
    for (int i = 0;i < request.size();i++) {
        qDebug("%x",(uint8_t)request[i]);
    }
    if (m_serial->waitForBytesWritten(10)) {
        return 0;
    }

    return -1;
}

QByteArray communication::wait_response(int timeout)
{
    QTime time;
    QByteArray temp,response;

    qDebug("wait rsp...");
    time.start();

    /*循环读取直到超时*/
    while ( m_serial->waitForReadyRead(timeout)) {
        timeout = FRAME_TIMEOUT;
        temp = m_serial->readAll();
        response.append(temp);
    }

    for (int i = 0; i < response.size(); i++) {
         qDebug("%x",(uint8_t)response[i]);
    }

    qDebug("rsp complete.total time:%d",time.elapsed());

    return response;
}

int communication::lock_serial_mutex()
{
    m_serial_mutex.lock();
}

int communication::unlock_serial_mutex()
{
    m_serial_mutex.unlock();
}

/*净重值*/
int communication::query_net_weight(int *weight1,int *weight2,int *weight3,int *weight4)
{
    int rc;
    int addr = 0;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_NET_WEIGHT,1,&addr,protocol::PROTOCOL_WEIGHT_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
        if (rc == 0) {
            protocol.get_weight(weight1,weight2,weight3,weight4);
        }
    }
exit:
    unlock_serial_mutex();
    return rc;
}


/*去皮*/
int communication::remove_tare_weight(int addr)
{
    int rc;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_REMOVE_TARE,1,&addr,protocol::PROTOCOL_REMOVE_TARE_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
    }

exit:
    unlock_serial_mutex();
    return rc;
}


/*校准*/
int communication::calibrate_weight(int addr,int weight)
{
    int rc;
    int value[3];
    QByteArray request,response;
    protocol protocol;

    value[0] = addr;
    value[1] = weight >> 8;
    value[2] = weight & 0xff;

    request = protocol.construct(protocol::PROTOCOL_CODE_CALIBRATE,3,value,protocol::PROTOCOL_CALIBRATE_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
    }

exit:
    unlock_serial_mutex();
    return rc;
}

/*开锁*/
int communication::unlock_lcok()
{
    int rc;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_UNLOCK,0,0,protocol::PROTOCOL_UNLOCK_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
    }

exit:
    unlock_serial_mutex();
    return rc;
}

/*关锁*/
int communication::lock_lock()
{
    int rc;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_LOCK,0,0,protocol::PROTOCOL_LOCK_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
    }

exit:
    unlock_serial_mutex();
    return rc;
}

/*查询门状态*/
QString communication::query_door_status()
{
    int rc;
    QString status;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_DOOR_STATUS,0,0,protocol::PROTOCOL_DOOR_STATUS_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        protocol.parse(response);
    }
    status = protocol.get_door_status();

exit:
    unlock_serial_mutex();
    return status;
}


/*查询锁状态*/
QString communication::query_lock_status()
{
    int rc;
    QString status;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_LOCK_STATUS,0,0,protocol::PROTOCOL_LOCK_STATUS_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        protocol.parse(response);
    }
    status = protocol.get_lock_status();

exit:
    unlock_serial_mutex();
    return status;
}


/*查询温度*/
int communication::query_temperature(int *setting,int *temperature)
{
    int rc;
    int value;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_TEMPERATURE,0,0,protocol::PROTOCOL_TEMPERATURE_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
        if (rc == 0) {
            protocol.get_temperature(setting,temperature);
        }
    }

exit:
    unlock_serial_mutex();
    return rc;
}

/*设置温度*/
int communication::set_temperature(int temperature)
{
    int rc;
    int value;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_SET_TEMPERATURE,1,&temperature,protocol::PROTOCOL_SET_TEMPERATURE_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
    }

exit:
    unlock_serial_mutex();
    return rc;
}


/*查询称重单元数量*/
int communication::query_layer_cnt()
{
    int rc;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_LAYER_CNT,0,0,protocol::PROTOCOL_LAYER_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
        if (rc == 0) {
            rc = protocol.get_layer_cnt();
        }
    }

exit:
    unlock_serial_mutex();
    return rc;
}

/*查询固件版本*/
QString communication::query_fw_version()
{
    int rc;
    QString fw_version;
    QByteArray request,response;
    protocol protocol;

    request = protocol.construct(protocol::PROTOCOL_CODE_FW_VERSION,0,0,protocol::PROTOCOL_FW_VERSION_TIMEOUT);

    lock_serial_mutex();
    rc = send_request(request);
    if (rc == 0) {
        response = wait_response(protocol.get_timeout());
        rc = protocol.parse(response);
        if (rc == 0) {
            fw_version = protocol.get_fw_version();
        }
    }

exit:
    unlock_serial_mutex();
    return fw_version;
}
