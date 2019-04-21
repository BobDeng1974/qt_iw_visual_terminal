#include "calibration.h"
#include "qtimer.h"

calibration::calibration(QObject *parent) : QObject(parent)
{

}

/*去皮*/
void calibration::calibration_event(int level,int weight)
{
    serial_mutex->lock();
    emit req_calibration(level,weight);
}
