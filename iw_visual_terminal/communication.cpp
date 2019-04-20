#include "qstring.h"
#include "communication.h"
#include "QtSerialPort/qserialport.h"

communication::communication(QObject *parent) : QObject(parent)
{
    serial_opened = false;

    serial = new QSerialPort(0);
    wait_rsp_timer = new QTimer(0);
    serial_mutex = new QMutex;
    crc = new crc16();

    wait_rsp_timer->setInterval(100);

    QObject::connect(wait_rsp_timer,SIGNAL(timeout()),this,SLOT(on_wait_rsp_timer_timeout()));
    QObject::connect(serial,SIGNAL(readyRead()),this,SLOT(on_rsp_ready()));
}

void communication::on_wait_rsp_timer_timeout()
{
    /*通信超时 代码-10*/
    serial_mutex->unlock();
     /*去皮*/
    if (req_code == 0x01) {
        emit rsp_tare_result(req_level,-10);
    } else if (req_code == 2) {
        /*校准*/
        emit rsp_calibration_result(req_level,req_param,-10);
    } else if (req_code == 3) {
        /*净重*/
        emit rsp_loop_query_weight_result(-10,0,0,0,0);
    } else {
        qWarning("内部错误：req_code:%d",req_code);
    }

}

void communication::on_rsp_ready()
{
    QByteArray rsp_array;

    int rc = 0;
    int weight1,weight2,weight3,weight4;

    uint8_t *rsp_data;
    int rsp_size;

    uint16_t crc_recv,crc_calculate;

    wait_rsp_timer->stop();/*停止定时器*/


    if (!serial->isOpen()) {
        rc = -30;
        goto err_exit;
    }
        rsp_array = serial->readAll();
        rsp_data = (uint8_t*)rsp_array.data();
        rsp_size = rsp_array.size();




        if (rsp_size < 4) {
            qWarning("回应长度小于4错误");
            rc = -11;/*长度错误*/
            goto err_exit;
        }

        crc_recv = rsp_data[rsp_size - 2] * 256 + rsp_data[rsp_size - 1];
        crc_calculate = crc->calculate_crc(rsp_data,rsp_size - 2);
        if (crc_recv != crc_calculate) {
            qWarning("crc recv:%d != %d crc calculate.",crc_recv,crc_calculate);
            rc = -12;/*校验错误*/
            goto err_exit;
        }
        switch (req_code ) {
        case 0x01:/*去皮*/
        case 0x02:/*校准*/
            if (rsp_size == 5 && rsp_data[1] == req_code) {
                if (rsp_data[2] == 0x01) { /*成功*/
                    rc = 0;
                } else {
                    rc = -1;/*失败*/
                }

            } else {
                rc = -20;/*协议错误*/
            }
        break;
         case 0x03:/*净重*/
            if (rsp_size == 24 && rsp_data[1] == 0x03) {
                weight1 = rsp_data[2] * 256 + rsp_data[3];
                weight2 = rsp_data[4] * 256 + rsp_data[5];
                weight3 = rsp_data[6] * 256 + rsp_data[7];
                weight4 = rsp_data[8] * 256 + rsp_data[9];
                rc = 0;

            } else {
                rc = -20;/*协议错误*/
            }
        break;
        default:
            qWarning("协议错误.");

        }


err_exit:
        /*去皮*/
        if (req_code == 0x01) {
            emit rsp_tare_result(req_level,rc);
        } else if (req_code == 0x02) {
            emit rsp_calibration_result(req_level,req_param,rc);
        } else if (req_code == 0x03) {
            emit rsp_loop_query_weight_result(rc,weight1,weight2,weight3,weight4);
        }

}



/*打开串口*/
void communication::on_open_serial_port_event(QString port_name,int baudrates,int data_bits,int parity)
{

    bool success;

    serial->setPortName(port_name);
    serial->setBaudRate(baudrates);
    if (data_bits == 8) {
        serial->setDataBits(QSerialPort::Data8);
    } else {
       serial->setDataBits(QSerialPort::Data7);
    }

    if (parity == 0){
        serial->setParity(QSerialPort::NoParity);
    } else if (parity == 1){
        serial->setParity(QSerialPort::OddParity);
    } else  {
         serial->setParity(QSerialPort::EvenParity);
    }

    success = serial->open(QSerialPort::ReadWrite);
    if (success) {
        qDebug("打开串口成功.");
        serial_opened = true;
        emit rsp_open_serial_port_result(0);/*发送成功信号*/
    } else {
        qWarning("打开串口失败.");
        emit rsp_open_serial_port_result(-1);/*发送失败信号*/
    }
}


/*关闭串口*/
void communication::on_close_serial_port_event(QString port_name)
{
    (void) port_name;

    serial->flush();

    serial->close();

    emit rsp_close_serial_port_result(0);/*发送关闭串口成功信号*/
}

/*轮询净重值*/
void communication::on_loop_query_weight_event()
{
    QByteArray query_weight;

    uint16_t crc16;

    serial_mutex->lock();
    query_weight.resize(5);

    query_weight[0] = 0x01;
    query_weight[1] = 0x03;
    query_weight[2] = 0x00;
    query_weight[3] = 0x00;
    query_weight[4] = 0x00;

    crc16 = crc->calculate_crc((uint8_t *)query_weight.data(),query_weight.size() - 2);
    query_weight.insert(3,(char)(crc16 >> 8));
    query_weight.insert(4,(char)(crc16 & 0xFF));

    if (serial_opened) {
        serial->write(query_weight.data(),query_weight.size());
        wait_rsp_timer->start();/*启动回应超时定时器*/
    } else {
        qWarning("串口是关闭的，无法发送数据.");
    }
}


/*去皮*/
void communication::on_tare_event(int level)
{
    QByteArray tare;

    uint16_t crc16;

    serial_mutex->lock();
    tare.resize(5);

    tare[0] = 0x01;
    tare[1] = 0x01;
    tare[2] = (uint8_t)level ;/*称号*/
    tare[3] = 0x00;
    tare[4] = 0x00;

    crc16 = crc->calculate_crc((uint8_t *)tare.data(),tare.size() - 2);
    tare.insert(3,(char)(crc16 >> 8));
    tare.insert(4,(char)(crc16 & 0xFF));

    if (serial_opened) {
        serial->write(tare.data(),tare.size());
        wait_rsp_timer->start();/*启动回应超时定时器*/
    } else {
        qWarning("串口是关闭的，无法发送数据.");
    }
}


/*校准*/
void communication::on_calibration_event(int level,int calibration_weight)
{
    QByteArray calibration;

    uint16_t crc16;

    serial_mutex->lock();
    calibration.resize(7);

    calibration[0] = 0x01;
    calibration[1] = 0x02;
    calibration[2] = (uint8_t)level;

    calibration[3] = calibration_weight >> 8;
    calibration[4] = calibration_weight & 0xFF;

    calibration[5] = 0x00;
    calibration[6] = 0x00;

    crc16 = crc->calculate_crc((uint8_t *)calibration.data(),calibration.size() - 2);
    calibration.insert(5,(char)(crc16 >> 8));
    calibration.insert(6,(char)(crc16 & 0xFF));

    if (serial_opened) {
        serial->write(calibration.data(),calibration.size());
        wait_rsp_timer->start();/*启动回应超时定时器*/
    } else {
        qWarning("串口是关闭的，无法发送数据.");
    }
}






