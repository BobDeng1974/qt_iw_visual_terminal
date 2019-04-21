#include "tare.h"

tare::tare(QObject *parent) : QObject(parent)
{
}

/*去皮*/
void tare::tare_event(int level)
{
    serial_mutex->lock();
    emit req_tare(level);
}
