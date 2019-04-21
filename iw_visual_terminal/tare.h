#ifndef TARE_H
#define TARE_H

#include <QObject>
#include "qtimer.h"
#include "qmutex.h"

class tare : public QObject
{
    Q_OBJECT
public:
    explicit tare(QObject *parent = nullptr);
    QMutex *serial_mutex;

signals:
    void req_tare(int);

public slots:
    void tare_event(int);
};

#endif // TARE_H
