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
signals:
    void req_loop_weight(void);
    void req_tare(int);
    void req_calibration(int,int);
public slots:
    void on_loop_weight_timer_timeout(void);
    void on_rsp_loop_weight_result(int,int,int,int,int,int);
    void on_rsp_tare_result(int,int);
    void on_calibration_result(int,int,int);
private:
    Ui::MainWindow *ui;
    communication *comm;
    QTimer *loop_weight_timer;
    bool opened;

};

#endif // MAINWINDOW_H
