#include "qstring.h"
#include "communication.h"
#include "QtSerialPort/qserialport.h"
#include "QtSerialPort/QSerialPortInfo"
#include "qthread.h"
#include "QTime"

communication::communication(QObject *parent) : QObject(parent)
{
    m_opened = false;
    m_busy = false;
    rsp_size = 0;
    duty_multiple = 0;
    m_serial = new QSerialPort(parent);
    m_req_queue = new QQueue<req_param>();

    m_req_timer = new QTimer(this);
    m_req_timer->setInterval(REQ_TIMEOUT);

    m_crc = new crc16();


    QObject::connect(m_req_timer,SIGNAL(timeout()),this,SLOT(handle_req_timeout_event()));

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

/*插入周期轮询请求*/
void communication::insert_period_query_req()
{

    handle_query_weight_req();

    /*主要轮询重量 兼顾其他*/
    duty_multiple++;
    if (duty_multiple >= QUERY_WEIGHT_DUCY_MULTIPLE) {
        handle_query_door_status();
        handle_query_lock_status();
        handle_query_temperature();
        handle_query_weight_layer();
        handle_query_fw_version();
        duty_multiple  = 0;
    }

}

/*处理请求队列*/
void communication::handle_req_timeout_event()
{
    req_param req;

    if (!m_opened) {
        qWarning("串口是关闭的，无法发送数据.");
    }
    m_req_timer->stop();

    /*没有请求 就插入周期请求*/
    if (m_req_queue->isEmpty()) {
        insert_period_query_req();
    }

    /*关掉定时器并处理请求*/
    m_req_timer->stop();

    req = m_req_queue->dequeue();
    qDebug("process a req...");

    req_level = req.level;
    req_code = req.code;
    req_weight = req.value;

    m_serial->write(req.send,req.size);

    wait_rsp(req.timeout);
    handle_rsp();
}

/*等待回应*/
void communication::wait_rsp(int timeout)
{
    qint64 time;

    time = QDateTime::currentMSecsSinceEpoch();

    rsp_size = 0;
    QByteArray rsp_array;

    qDebug("wait start time:%d",time);

    /*循环读取直到超时*/
    while ( m_serial->waitForReadyRead(timeout)) {

        timeout = CONTINUE_TIMEOUT;
        rsp_array = m_serial->readAll();
        if (rsp_array.size() + rsp_size >= 100) {
            qDebug("rsp size overflow.");
            rsp_size = 0;
        } else {
            memcpy(&rsp[rsp_size],(uint8_t*)rsp_array.data(),rsp_array.size());
            rsp_size += rsp_array.size();
            rsp_array.clear();
        }
    }

    time = QDateTime::currentMSecsSinceEpoch();
    qDebug("wait end time:%d",time);

}

/*处理回应*/
void communication::handle_rsp()
{
    int rc = -1;
    uint16_t crc_recv,crc_calculate;

    int weight1 = 0,weight2 = 0,weight3 = 0,weight4 = 0;
    int temperature = 0x7f;
    int temperature_setting = 0x7f;
    int weight_layer = 0;
    int fw_version = 0;

    QString status("错误");

    m_busy = false;


    if (rsp_size == 0) {
        rc = -10;
        qWarning("回应超时");
        goto err_exit;
    }

    if (rsp_size < 4) {
        qWarning("回应长度小于4错误");

        rc = -11;/*长度错误*/
        goto err_exit;
    }

    crc_recv = rsp[rsp_size - 2] * 256 + rsp[rsp_size - 1];
    crc_calculate = m_crc->calculate_crc(rsp,rsp_size - 2);
    if (crc_recv != crc_calculate) {
        qWarning("crc recv:%d != %d crc calculate.",crc_recv,crc_calculate);
        rc = -12;/*校验错误*/
        goto err_exit;
    }

    if (req_code != rsp[1]) {
        qWarning("recv code:%d != req code:%d err.",rsp[1],req_code);
        rc = -13;/*操作码错误*/
        goto err_exit;
    }

    switch (req_code ) {
    case REQ_CODE_TARE:/*去皮*/
    case REQ_CODE_CALIBRATION:/*校准*/
    case REQ_CODE_UNLOCK:/*开锁*/
    case REQ_CODE_LOCK:/*关锁*/
    case REQ_CODE_SET_TEMPERATURE:/*设置温度*/
        if (rsp_size == 5 ) {
            if (rsp[2] == 0x01) { /*成功*/
                rc = 0;
            } else {
                rc = -1;/*失败*/
            }

        } else {
            rc = -20;/*协议错误*/
        }
    break;
     case REQ_CODE_QUERY_WEIGHT:/*净重*/
        if (rsp_size == 44 ) {
            weight1 = rsp[2] * 256 + rsp[3];
            weight2 = rsp[4] * 256 + rsp[5];
            weight3 = rsp[6] * 256 + rsp[7];
            weight4 = rsp[8] * 256 + rsp[9];
            rc = 0;
        } else {
            rc = -20;/*协议错误*/
        }
    break;
    case REQ_CODE_QUERY_DOOR_STATUS:/*门状态*/
    case REQ_CODE_QUERY_LOCK_STATUS:/*锁状态*/
         if (rsp_size == 5 ) {
            rc = 0;
            if (rsp[2] == 0x01) {
                 status = "打开";
            } else {
                 status = "关闭";
            }

        }
    break;
    case REQ_CODE_QUERY_TEMPERATURE:/*温度*/
         if (rsp_size == 6) {
             rc = 0;
             temperature_setting = rsp[2];
             temperature = rsp[3];
          }
    break;

    case REQ_CODE_QUERY_WEIGHT_LAYER:/*称重单元数量*/
        if (rsp_size == 5) {
            rc = 0;
            weight_layer = rsp[2];
         }
    break;
    case REQ_CODE_QUERY_FW_VERSION:/*固件版本*/
        if (rsp_size == 7) {
            rc = 0;
            fw_version = rsp[2] << 16 |rsp[3] << 8 |rsp[4];
         }
    break;
    default:
        qWarning("协议错误.");

    }


err_exit:
    /*去皮*/
    if (req_code == REQ_CODE_TARE) {
        emit rsp_tare_result(req_level,rc);
    } else if (req_code == REQ_CODE_CALIBRATION) {
        emit rsp_calibration_result(req_level,req_weight,rc);
    } else if (req_code == REQ_CODE_QUERY_WEIGHT) {
        emit rsp_query_weight_result(rc,req_level,weight1,weight2,weight3,weight4);
    } else if (req_code == REQ_CODE_UNLOCK) {
        emit rsp_unlock_result(rc);
    }else if (req_code == REQ_CODE_LOCK) {
        emit rsp_lock_result(rc);
    }else if (req_code == REQ_CODE_QUERY_DOOR_STATUS) {
        emit rsp_query_door_status(rc,status);
    }else if(req_code == REQ_CODE_QUERY_LOCK_STATUS) {
        emit rsp_query_lock_status(rc,status);
    } else if (req_code == REQ_CODE_QUERY_TEMPERATURE) {
        emit rsp_query_temperature_result(rc,temperature_setting,temperature);
    } else if (req_code == REQ_CODE_SET_TEMPERATURE ) {
        emit rsp_set_temperature_result(rc);
    } else if (req_code == REQ_CODE_QUERY_WEIGHT_LAYER ) {
        emit rsp_query_weight_layer_result(rc,weight_layer);
    } else if (req_code == REQ_CODE_QUERY_FW_VERSION ) {
        emit rsp_query_fw_vrersion_result(rc,fw_version);
    }

    qDebug("rsp result--> code:%d rc:%d",req_code,rc);
    qDebug("restart req timer.");
    m_req_timer->start();
}





/*打开串口*/
void communication::handle_open_serial_port_req(QString port_name,int baudrates,int data_bits,int parity)
{

    bool success;

    m_serial->setPortName(port_name);
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
        m_opened = true;

        m_req_timer->start();/*开始请求*/

        emit rsp_open_serial_port_result(0);/*发送成功信号*/
    } else {
        qWarning("打开串口失败.");
        emit rsp_open_serial_port_result(-1);/*发送失败信号*/
    }
}


/*关闭串口*/
void communication::handle_close_serial_port_req(QString port_name)
{
    (void) port_name;

    m_serial->flush();
    m_serial->close();

    m_req_timer->stop();
    m_opened = false;

    emit rsp_close_serial_port_result(0);/*发送关闭串口成功信号*/
}

/*轮询净重值*/

void communication::handle_query_weight_req()
{
    req_param query_weight;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    query_weight.timeout = RSP_WEIGHT_TIMEOUT;

    query_weight.code = REQ_CODE_QUERY_WEIGHT;

    query_weight.size = 5;

    query_weight.send[0] = 0x01;
    query_weight.send[1] = query_weight.code;

    query_weight.level = 0;
    query_weight.send[2] = query_weight.level;

    crc16 = m_crc->calculate_crc((uint8_t *)query_weight.send,query_weight.size - 2);
    query_weight.send[3] = (crc16 >> 8);
    query_weight.send[4] = (crc16 & 0xFF);
    m_req_queue->enqueue(query_weight);

    qDebug("enqueue query weight req. queue size:%d.",m_req_queue->size());

}


/*去皮*/
void communication::handle_tare_req(int level)
{
    req_param tare;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    tare.timeout = RSP_REMOVE_TARE_TIMEOUT;
    tare.level = level;
    tare.code = REQ_CODE_TARE;
    tare.size = 5;

    tare.send[0] = 0x01;
    tare.send[1] = 0x01;
    tare.send[2] = (char)tare.level ;/*称号*/


    crc16 = m_crc->calculate_crc((uint8_t *)tare.send,tare.size - 2);
    tare.send[3] = (crc16 >> 8);
    tare.send[4] = (crc16 & 0xFF);

    m_req_queue->enqueue(tare);

    qDebug("tare queue size:%d.",m_req_queue->size());
}


/*校准*/
void communication::handle_calibration_req(int level,int calibration_weight)
{
    req_param calibration;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    calibration.timeout = RSP_CALIBRATION_TIMEOUT;
    calibration.level = level;
    calibration.code = REQ_CODE_CALIBRATION;
    calibration.value = calibration_weight;


    calibration.size = 7;

    calibration.send[0] = 0x01;
    calibration.send[1] = calibration.code;
    calibration.send[2] = (uint8_t)level;

    calibration.send[3] = calibration_weight >> 8;
    calibration.send[4] = calibration_weight & 0xFF;



    crc16 = m_crc->calculate_crc((uint8_t *)calibration.send,calibration.size - 2);
    calibration.send[5] = (crc16 >> 8);
    calibration.send[6] = (crc16 & 0xFF);

    m_req_queue->enqueue(calibration);

    qDebug("cal queue size:%d.",m_req_queue->size());
}

/*开锁*/
void communication::handle_req_unlock()
{
    req_param unlock;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }
    unlock.timeout = RSP_UNLOCK_TIMEOUT;

    unlock.code = REQ_CODE_UNLOCK;


    unlock.size = 4;

    unlock.send[0] = 0x01;
    unlock.send[1] = unlock.code;


    crc16 = m_crc->calculate_crc((uint8_t *)unlock.send,unlock.size - 2);
    unlock.send[2] = (crc16 >> 8);
    unlock.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(unlock);

    qDebug("unlock queue size:%d.",m_req_queue->size());
}

/*关锁*/
void communication::handle_req_lock()
{
    req_param lock;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    lock.timeout = RSP_LOCK_TIMEOUT;
    lock.code = REQ_CODE_LOCK;

    lock.size = 4;

    lock.send[0] = 0x01;
    lock.send[1] = lock.code;


    crc16 = m_crc->calculate_crc((uint8_t *)lock.send,lock.size - 2);
    lock.send[2] = (crc16 >> 8);
    lock.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(lock);

    qDebug("unlock queue size:%d.",m_req_queue->size());
}

/*查询门状态*/
void communication::handle_query_door_status()
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_DOOR_STATUS_TIMEOUT;
    status.code = REQ_CODE_QUERY_DOOR_STATUS;
    status.size = 4;

    status.send[0] = 0x01;
    status.send[1] = status.code;


    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[2] = (crc16 >> 8);
    status.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);

    qDebug("enqueue query door status req.queue size:%d.",m_req_queue->size());
}


/*查询锁状态*/
void communication::handle_query_lock_status()
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_LOCK_STATUS_TIMEOUT;
    status.code = REQ_CODE_QUERY_LOCK_STATUS;


    status.size = 4;

    status.send[0] = 0x01;
    status.send[1] = status.code;


    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[2] = (crc16 >> 8);
    status.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);
    qDebug("enqueue query lock status req. queue size:%d.",m_req_queue->size());
}

/*查询温度*/
void communication::handle_query_temperature()
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_TEMPERATURE_TIMEOUT;
    status.code = REQ_CODE_QUERY_TEMPERATURE;


    status.size = 4;

    status.send[0] = 0x01;
    status.send[1] = status.code;


    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[2] = (crc16 >> 8);
    status.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);
    qDebug("enqueue query temperature req. queue size:%d.",m_req_queue->size());
}

/*设置温度*/
void communication::handle_set_temperature(int temperature)
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_SET_TEMPERATURE_TIMEOUT;
    status.code = REQ_CODE_SET_TEMPERATURE;


    status.size = 5;

    status.send[0] = 0x01;
    status.send[1] = status.code;
    status.send[2] = temperature;

    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[3] = (crc16 >> 8);
    status.send[4] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);
    qDebug("enqueue set temperature req. queue size:%d.",m_req_queue->size());
}

/*查询称重单元数量*/
void communication::handle_query_weight_layer()
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_TEMPERATURE_TIMEOUT;
    status.code = REQ_CODE_QUERY_WEIGHT_LAYER;


    status.size = 4;

    status.send[0] = 0x01;
    status.send[1] = status.code;


    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[2] = (crc16 >> 8);
    status.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);
    qDebug("enqueue query weight layer req. queue size:%d.",m_req_queue->size());
}

/*查询固件版本*/
void communication::handle_query_fw_version()
{
    req_param status;

    uint16_t crc16;

    if (m_opened == false){
        return;
    }

    status.timeout = RSP_FW_VERSION_TIMEOUT;
    status.code = REQ_CODE_QUERY_FW_VERSION;


    status.size = 4;

    status.send[0] = 0x01;
    status.send[1] = status.code;


    crc16 = m_crc->calculate_crc((uint8_t *)status.send,status.size - 2);
    status.send[2] = (crc16 >> 8);
    status.send[3] = (crc16 & 0xFF);

    m_req_queue->enqueue(status);
    qDebug("enqueue query fw req. queue size:%d.",m_req_queue->size());
}
