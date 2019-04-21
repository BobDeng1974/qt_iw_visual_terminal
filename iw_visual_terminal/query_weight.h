#ifndef QUERY_WEIGHT_H
#define QUERY_WEIGHT_H

#include <QObject>
#include "qtimer.h"
#include "qmutex.h"

class query_weight : public QObject
{
    Q_OBJECT
public:
    explicit query_weight(QObject *parent = nullptr);
    QMutex *serial_mutex;

signals:
    void req_query_weight();
public slots:
    void query_weight_event();

};

#endif // QUERY_WEIGHT_H
