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

    enum {
        LOOP_TIMEOUT = 10
    };
    enum {
        QUERY_WEIGHT_DUCY_MULTIPLE = 4
    };

    void handle_tare_result(int,int);
    void handle_calibration_result(int,int,int);

private slots:

    int loop_timer_timeout();

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


    void on_actionabout_triggered();

    void on_set_temperature_button_clicked();

private:
    Ui::MainWindow *ui;
    communication m_comm;
    QTimer *m_loop_timer;
    int m_duty_multiple;
};

#endif // MAINWINDOW_H
