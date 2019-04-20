#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qthread.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    loop_weight_timer = new QTimer();
    loop_weight_timer->setInterval(110);
    //loop_weight_timer->start();


    QThread *comm_thread = new QThread();

    comm = new communication(0);
    comm->moveToThread(comm_thread);


    QObject::connect(loop_weight_timer,SIGNAL(timeout()),this,SLOT(on_loop_weight_timer_timeout()));


}

MainWindow::~MainWindow()
{
    delete ui;
}
