#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qthread.h"
#include "qmessagebox.h"
#include "qdebug.h"
#include "dialog_about.h"
#include "communication.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    QRegExp regExp("-?[0-9]{0,2}");
    ui->set_temperature_edit->setValidator(new QRegExpValidator(regExp, this));
    ui->set_temperature_edit->setText(QString::number(6));

    /*添加端口*/
    QStringList m_serialPortName;

    m_serialPortName  = m_comm.get_port_name_list();
    QString name;
    foreach(name,m_serialPortName) {
        ui->port_list->addItem(name);
    }

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
    ui->parity_list->addItem("奇校验");
    ui->parity_list->addItem("偶校验");
    ui->parity_list->setCurrentIndex(0);

    ui->display_1->display("------");
    ui->display_2->display("------");
    ui->display_3->display("------");
    ui->display_4->display("------");

    m_loop_timer = new QTimer();
    m_loop_timer->setSingleShot(1);
    m_loop_timer->setInterval(LOOP_TIMEOUT);
    QObject::connect(m_loop_timer,SIGNAL(timeout()),this,SLOT(loop_timer_timeout()));
}

/*周期轮询*/
int MainWindow::loop_timer_timeout()
{
    int rc;
    QString status;
    QString fw_version;
    int weight1,weight2,weight3,weight4;
    int temperature,setting;


    rc = m_comm.query_net_weight(&weight1,&weight2,&weight3,&weight4);
   if (rc == 0) {
       if ((int16_t)weight1 == -1) {
         ui->display_1->display("err");
       } else {
         ui->display_1->display((int16_t)weight1);
       }

       if ((int16_t)weight2 == -1) {
         ui->display_2->display("err");
       } else {
         ui->display_2->display((int16_t)weight2);
       }

       if ((int16_t)weight3 == -1) {
         ui->display_3->display("err");
       } else {
         ui->display_3->display((int16_t)weight3);
       }

       if ((int16_t)weight4 == -1) {
         ui->display_4->display("err");
       } else {
         ui->display_4->display((int16_t)weight4);
       }

   } else {
         ui->display_1->display("err");
         ui->display_2->display("err");
         ui->display_3->display("err");
         ui->display_4->display("err");
   }



    /*主要轮询重量 兼顾其他*/
    m_duty_multiple++;
    if (m_duty_multiple >= QUERY_WEIGHT_DUCY_MULTIPLE) {

        ui->door_status_display->setStyleSheet("color:blue");
        ui->lock_status_display->setStyleSheet("color:blue");
        ui->temperature_display->setStyleSheet("color:blue");
        ui->temperature_setting_display->setStyleSheet("color:blue");
        ui->fw_version_display->setStyleSheet("color:blue");
        ui->weight_layer_display->setStyleSheet("color:blue");

        status = m_comm.query_door_status();
        if (status.compare("错误") == 0) {
            ui->door_status_display->setStyleSheet("color:red");
        }
        ui->door_status_display->setText(status);

        status = m_comm.query_lock_status();
        if (status.compare("错误") == 0) {
            ui->lock_status_display->setStyleSheet("color:red");
        }
        ui->lock_status_display->setText(status);


        rc = m_comm.query_temperature(&setting,&temperature);
        if (rc == 0) {
            ui->temperature_display->setText(QString::number(temperature));
            ui->temperature_setting_display->setText(QString::number(setting));
            if (temperature == 0x7f) {
                ui->temperature_display->setStyleSheet("color:red");
                ui->temperature_display->setText("错误");
            }
        } else {
            ui->temperature_display->setStyleSheet("color:red");
            ui->temperature_setting_display->setStyleSheet("color:red");
            ui->temperature_display->setText("错误");
            ui->temperature_setting_display->setText("错误");
        }

        rc = m_comm.query_layer_cnt();
        if (rc > 0) {
            ui->weight_layer_display->setText(QString::number(rc));
        } else {
            ui->weight_layer_display->setStyleSheet("color:red");
            ui->weight_layer_display->setText("错误");
        }

        fw_version = m_comm.query_fw_version();
        if(fw_version.isEmpty()) {
            ui->fw_version_display->setStyleSheet("color:red");
            ui->fw_version_display->setText("错误");
        } else {
            ui->fw_version_display->setText(fw_version);
        }

        m_duty_multiple  = 0;
    }
    m_loop_timer->start();

}


MainWindow::~MainWindow()
{
    delete ui;
}

/*去皮结果显示*/
void MainWindow::handle_tare_result(int level,int result)
{
    if (result == 0) {
       QMessageBox::information(this,"成功","第" + QString::number( level)+ "层去皮成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"错误","第" + QString::number( level)+ "层去皮失败！",QMessageBox::Ok);
    }

}

/*校准结果显示*/
void MainWindow::handle_calibration_result(int level,int calibration_weight,int result)
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
    int rc;
    if (!m_comm.is_serial_open()) {
        rc = m_comm.open_serial(ui->port_list->currentText(),ui->baudrate_list->currentText().toInt(),ui->data_bits_list->currentText().toInt(),ui->parity_list->currentText().toInt());
        if (rc == 0) {
            ui->open_button->setText("关闭");
            qDebug("串口打开成功！");
            ui->port_list->setEnabled(false);
            ui->data_bits_list->setEnabled(false);
            ui->baudrate_list->setEnabled(false);
            ui->parity_list->setEnabled(false);
            m_loop_timer->start();
        } else {
             QMessageBox::warning(this,"错误",ui->port_list->currentText() + "打开失败！",QMessageBox::Ok);
        }
    } else {
       m_comm.close_serial(ui->port_list->currentText());
       ui->open_button->setText("打开");
       qDebug("关闭成功！");
       ui->display_1->display("------");
       ui->display_2->display("------");
       ui->display_3->display("------");
       ui->display_4->display("------");

       ui->port_list->setEnabled(true);
       ui->data_bits_list->setEnabled(true);
       ui->baudrate_list->setEnabled(true);
       ui->parity_list->setEnabled(true);

       ui->door_status_display->setStyleSheet("color:black");
       ui->lock_status_display->setStyleSheet("color:black");
       ui->temperature_display->setStyleSheet("color:black");
       ui->temperature_setting_display->setStyleSheet("color:black");
       ui->weight_layer_display->setStyleSheet("color:black");
       ui->fw_version_display->setStyleSheet("color:black");

       ui->door_status_display->setText("未知");
       ui->lock_status_display->setText("未知");
       ui->temperature_display->setText("未知");
       ui->temperature_setting_display->setText("未知");
       ui->weight_layer_display->setText("未知");
       ui->fw_version_display->setText("未知");
       m_loop_timer->stop();
    }
}

/*第4层去皮*/
void MainWindow::on_tare_button_4_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.remove_tare_weight(4);
    handle_tare_result(4,rc);

}
/*第3层去皮*/
void MainWindow::on_tare_button_3_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.remove_tare_weight(3);
    handle_tare_result(3,rc);
}

/*第2层去皮*/
void MainWindow::on_tare_button_2_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.remove_tare_weight(2);
    handle_tare_result(2,rc);
}
/*第1层去皮*/
void MainWindow::on_tare_button_1_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.remove_tare_weight(1);
    handle_tare_result(1,rc);
}


/*第4层0点校准*/
void MainWindow::on_calibration_zero_button_4_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(4,0);
    handle_calibration_result(4,0,rc);
}

/*第3层0点校准*/
void MainWindow::on_calibration_zero_button_3_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(3,0);
    handle_calibration_result(3,0,rc);
}

/*第2层0点校准*/
void MainWindow::on_calibration_zero_button_2_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(2,0);
    handle_calibration_result(2,0,rc);
}

/*第1层0点校准*/
void MainWindow::on_calibration_zero_button_1_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(1,0);
    handle_calibration_result(1,0,rc);
}

/*第4层2000g校准*/
void MainWindow::on_calibration_2000_button_4_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(4,2000);
    handle_calibration_result(4,2000,rc);
}

/*第3层2000g校准*/
void MainWindow::on_calibration_2000_button_3_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(3,2000);
    handle_calibration_result(3,2000,rc);
}
/*第2层2000g校准*/
void MainWindow::on_calibration_2000_button_2_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(2,2000);
    handle_calibration_result(2,2000,rc);
}

/*第1层2000g校准*/
void MainWindow::on_calibration_2000_button_1_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(1,2000);
    handle_calibration_result(1,2000,rc);
}

/*第4层5000g校准*/
void MainWindow::on_calibration_5000_button_4_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(4,5000);
    handle_calibration_result(4,5000,rc);
}
/*第3层5000g校准*/
void MainWindow::on_calibration_5000_button_3_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(3,5000);
    handle_calibration_result(3,5000,rc);
}
/*第2层5000g校准*/
void MainWindow::on_calibration_5000_button_2_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(2,5000);
    handle_calibration_result(2,5000,rc);
}
/*第1层5000g校准*/
void MainWindow::on_calibration_5000_button_1_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.calibrate_weight(1,5000);
    handle_calibration_result(1,5000,rc);
}

void MainWindow::on_all_on_top_check_button_stateChanged(int arg1)
{
    (void)arg1;
    if (ui->all_on_top_check_button->isChecked()) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
    }
    this->show();
}



void MainWindow::on_open_lock_button_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.unlock_lcok();
    if (rc == 0) {
       QMessageBox::information(this,"成功","开锁成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"失败","开锁失败！",QMessageBox::Ok);
    }
}

void MainWindow::on_close_lock_button_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.lock_lock();
    if (rc == 0) {
       QMessageBox::information(this,"成功","关锁成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"失败","关锁失败！",QMessageBox::Ok);
    }
}



void MainWindow::on_actionabout_triggered()
{
    Dialog_about about(this);
    about.setWindowTitle("关于");
    about.exec();
}

void MainWindow::on_set_temperature_button_clicked()
{
    int rc;
    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.set_temperature(ui->set_temperature_edit->text().toInt());
    if (rc == 0) {
       QMessageBox::information(this,"成功","设置温度成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"失败","设置温度失败！",QMessageBox::Ok);
    }
}



void MainWindow::on_pwr_on_compressor_button_clicked()
{
    int rc;

    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.control_pwr_on_compressor();
    if (rc == 0) {
       QMessageBox::information(this,"成功","开压缩机成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"失败","开压缩机失败！",QMessageBox::Ok);
    }
}

void MainWindow::on_pwr_off_compressor_button_clicked()
{
    int rc;

    if (!m_comm.is_serial_open()) {
        QMessageBox::warning(this,"错误","串口没有打开！",QMessageBox::Ok);
        return;
    }
    rc = m_comm.control_pwr_off_compressor();
    if (rc == 0) {
       QMessageBox::information(this,"成功","关压缩机成功！",QMessageBox::Ok);
    } else {
       QMessageBox::warning(this,"失败","关压缩机失败！",QMessageBox::Ok);
    }
}
