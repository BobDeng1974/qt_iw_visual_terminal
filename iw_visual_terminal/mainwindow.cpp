#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qthread.h"
#include "qmessagebox.h"
#include "qdebug.h"
#include "communication.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /*添加端口*/
    ui->port_list->addItem("COM1");
    ui->port_list->addItem("COM2");
    ui->port_list->addItem("COM3");
    ui->port_list->addItem("COM4");
    ui->port_list->addItem("COM5");
    ui->port_list->addItem("COM6");
    ui->port_list->addItem("COM7");
    ui->port_list->addItem("COM8");
    ui->port_list->addItem("COM9");
    ui->port_list->setCurrentIndex(0);

    /*添加波特率*/
    ui->baudrate_list->addItem("115200");
    ui->baudrate_list->addItem("57600");
    ui->baudrate_list->addItem("19200");
    ui->baudrate_list->addItem("9600");
    ui->baudrate_list->setCurrentIndex(0);

    /*添加数据位*/
    ui->data_bits_list->addItem("8");
    ui->data_bits_list->addItem("7");
    ui->data_bits_list->setCurrentIndex(0);


    /*添加校验类型*/
    ui->parity_list->addItem("无校验");
    ui->parity_list->addItem("偶校验");
    ui->parity_list->addItem("奇校验");
    ui->parity_list->setCurrentIndex(0);


    loop_weight_timer = new QTimer(this);
    loop_weight_timer->setInterval(100);

    opened = false;

    QThread *comm_thread = new QThread(0);

    comm = new communication(0);

    comm->moveToThread(comm_thread);
    comm->serial->moveToThread(comm_thread);

    comm_thread->start();

    QObject::connect(loop_weight_timer,SIGNAL(timeout()),this,SLOT(on_loop_weight_timer_timeout()));

     /*-从UI到comm--*/
    QObject::connect(this,SIGNAL(req_open_serial_port(QString ,int ,int ,int )),comm,SLOT(on_open_serial_port_event(QString ,int ,int ,int )));
    QObject::connect(this,SIGNAL(req_close_serial_port(QString)),comm,SLOT(on_close_serial_port_event(QString)));

    QObject::connect(this,SIGNAL(req_loop_weight(void)),comm->m_weight,SLOT(query_weight_event(void)));
    QObject::connect(this,SIGNAL(req_tare(int)),comm->m_tare,SLOT(tare_event(int)));
    QObject::connect(this,SIGNAL(req_calibration(int,int)),comm->m_calibration,SLOT(calibration_event(int,int)));

    /*-从comm到UI--*/
    QObject::connect(comm,SIGNAL(rsp_open_serial_port_result(int )),this,SLOT(on_rsp_open_serial_port_result(int )));
    QObject::connect(comm,SIGNAL(rsp_close_serial_port_result(int )),this,SLOT(on_rsp_close_serial_port_result(int )));

    QObject::connect(comm,SIGNAL(rsp_loop_query_weight_result(int ,int ,int ,int ,int )),this,SLOT(on_rsp_loop_weight_result(int ,int ,int ,int ,int )));
    QObject::connect(comm,SIGNAL(rsp_tare_result(int,int)),this,SLOT(on_rsp_tare_result(int,int)));
    QObject::connect(comm,SIGNAL(rsp_calibration_result(int,int,int)),this,SLOT(on_rsp_calibration_result(int,int,int)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*串口打开操作结果*/
void MainWindow::on_rsp_open_serial_port_result(int result)
{
    if (result == 0) {
        opened = true;
        ui->open_button->setText("关闭");
        qDebug("串口打开成功！");
        /*开启轮询净重定时器*/
        loop_weight_timer->start();


    } else {
        QMessageBox::warning(this,"错误",ui->port_list->currentText() + "打开失败！",QMessageBox::Ok);
    }
}

/*串口关闭操作结果*/
void MainWindow::on_rsp_close_serial_port_result(int result)
{
    if (result == 0) {
        opened = false;
        ui->open_button->setText("打开");
        loop_weight_timer->stop();
        qDebug("串口关闭成功！");

    } else {
        QMessageBox::warning(this,"错误",ui->port_list->currentText() + "关闭失败！",QMessageBox::Ok);
    }
}

/*轮询净重超时处理*/
void MainWindow::on_loop_weight_timer_timeout()
{
    on_rsp_loop_weight_result(-1,0,0,0,0);
    /*发送轮询净重信号*/
    loop_weight_timer->start();
}


/*净重结果显示*/
void MainWindow::on_rsp_loop_weight_result(int result,int weight1,int weight2,int weight3,int weight4)
{
    /*成功读取*/
    if (result == 0) {
        if ((int16_t)weight1 == -1) {/*错误标志*/
            ui->display_1->display("err");
        } else {
            ui->display_1->display((int16_t)weight1);
        }

        if ((int16_t)weight2 == -1) {/*错误标志*/
            ui->display_2->display("err");
        } else {
            ui->display_2->display((int16_t)weight2);
        }

        if ((int16_t)weight3 == -1) {/*错误标志*/
            ui->display_3->display("err");
        } else {
            ui->display_3->display((int16_t)weight3);
        }

        if ((int16_t)weight4 == -1) {/*错误标志*/
            ui->display_4->display("err");
        } else {
            ui->display_4->display((int16_t)weight4);
        }

    } else {

         ui->display_1->display(opened == true ? "err" :"------");
         ui->display_2->display(opened == true ? "err" :"------");
         ui->display_3->display(opened == true ? "err" :"------");
         ui->display_4->display(opened == true ? "err" :"------");

    }
    loop_weight_timer->start();
}


/*去皮结果显示*/

void MainWindow::on_rsp_tare_result(int level,int result)
{
    if (result == 0) {
       QMessageBox::information(this,"成功","第" + QString::number( level)+ "层去皮成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"错误","第" + QString::number( level)+ "层去皮失败！",QMessageBox::Ok);
    }

}


/*校准结果显示*/
void MainWindow::on_rsp_calibration_result(int level,int calibration_weight,int result)
{
    if (result == 0) {
       QMessageBox::information(this,"成功","第" + QString::number( level)+ "层" + QString::number(calibration_weight) +"校准成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"错误","第" + QString::number( level)+ "层" + QString::number(calibration_weight) +"校准失败！",QMessageBox::Ok);
    }

}



/*点击打开或者关闭按键*/
void MainWindow::on_open_button_clicked()
{
    if (opened) {
        /*发送关闭串口信号*/
        emit req_close_serial_port(ui->port_list->currentText());
    } else {

        /*发送打开串口信号*/
        emit req_open_serial_port(ui->port_list->currentText(),ui->baudrate_list->currentText().toInt(),ui->data_bits_list->currentText().toInt(),ui->parity_list->currentIndex());
    }
}

/*第4层去皮*/
void MainWindow::on_tare_button_4_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送去皮信号*/
    emit req_tare(4);

}
/*第3层去皮*/
void MainWindow::on_tare_button_3_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送去皮信号*/
    emit req_tare(3);
}

/*第2层去皮*/
void MainWindow::on_tare_button_2_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送去皮信号*/
    emit req_tare(2);
}
/*第1层去皮*/
void MainWindow::on_tare_button_1_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送去皮信号*/
    emit req_tare(1);
}


/*第4层0点校准*/
void MainWindow::on_calibration_zero_button_4_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(4,0);
}

/*第3层0点校准*/
void MainWindow::on_calibration_zero_button_3_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(3,0);
}

/*第2层0点校准*/
void MainWindow::on_calibration_zero_button_2_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(2,0);
}

/*第1层0点校准*/
void MainWindow::on_calibration_zero_button_1_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(1,0);
}

/*第4层2000g校准*/
void MainWindow::on_calibration_2000_button_4_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(4,2000);
}

/*第3层2000g校准*/
void MainWindow::on_calibration_2000_button_3_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(3,2000);
}
/*第2层2000g校准*/
void MainWindow::on_calibration_2000_button_2_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(2,2000);
}

/*第1层2000g校准*/
void MainWindow::on_calibration_2000_button_1_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(1,2000);
}

/*第4层5000g校准*/
void MainWindow::on_calibration_5000_button_4_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(4,5000);
}
/*第3层5000g校准*/
void MainWindow::on_calibration_5000_button_3_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(3,5000);
}
/*第2层5000g校准*/
void MainWindow::on_calibration_5000_button_2_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(2,5000);
}
/*第1层5000g校准*/
void MainWindow::on_calibration_5000_button_1_clicked()
{
    if (opened == false) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    /*发送校准信号*/
    emit req_calibration(1,5000);
}

void MainWindow::on_all_on_top_check_button_stateChanged(int arg1)
{
    if (ui->all_on_top_check_button->isChecked()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }
    this->show();
}










