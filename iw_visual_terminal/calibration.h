#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QObject>
#include "qtimer.h"
#include "qmutex.h"


class calibration : public QObject
{
    Q_OBJECT
public:
    explicit calibration(QObject *parent = nullptr);
    QMutex *serial_mutex;

signals:
    void req_calibration(int,int);

public slots:
    void calibration_event(int,int);

};

#endif // CALIBRATION_H
