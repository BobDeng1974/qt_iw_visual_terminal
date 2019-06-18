#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "communication.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void handle_open_serial_port_result(int result);
    void handle_close_serial_port_result(int result);

    void handle_query_weight_result(int result,int level,int ,int,int,int);
    void handle_tare_result(int,int);
    void handle_calibration_result(int,int,int);
    void handle_set_temperature_result(int);

    void handle_unlock_result(int);
    void handle_lock_result(int);

signals:
    void req_open_serial_port(QString port_name,int baudrates,int data_bits,int parity);
    void req_close_serial_port(QString port_name);

    void req_tare(int);
    void req_calibration(int,int);
    void req_unlock();
    void req_lock();
    void req_set_temperature(int);

private slots:
    void on_open_button_clicked();

    void on_tare_button_4_clicked();

    void on_tare_button_3_clicked();
    
    void on_tare_button_2_clicked();

    void on_tare_button_1_clicked();

    void on_calibration_zero_button_4_clicked();

    void on_calibration_zero_button_3_clicked();

    void on_calibration_zero_button_2_clicked();

    void on_calibration_zero_button_1_clicked();

    void on_calibration_2000_button_4_clicked();

    void on_calibration_2000_button_3_clicked();

    void on_calibration_2000_button_2_clicked();

    void on_calibration_2000_button_1_clicked();

    void on_calibration_5000_button_4_clicked();

    void on_calibration_5000_button_3_clicked();

    void on_calibration_5000_button_2_clicked();

    void on_calibration_5000_button_1_clicked();

    void on_all_on_top_check_button_stateChanged(int arg1);

    void on_open_lock_button_clicked();

    void on_close_lock_button_clicked();

    void handle_rsp_query_door_status(int rc,QString status);
    void handle_rsp_query_lock_status(int rc,QString status);
    void handle_rsp_query_temperature_result(int,int,int);
    void handle_rsp_query_weight_layer_result(int,int);

    void on_actionabout_triggered();

    void on_set_temperature_button_clicked();

private:
    Ui::MainWindow *ui;
    communication *comm;
    bool opened;
};

#endif // MAINWINDOW_H
