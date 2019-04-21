#include "query_weight.h"

class communication;

query_weight::query_weight(QObject *parent) : QObject(parent)
{

}

/*轮询净重*/
void query_weight::query_weight_event()
{
    serial_mutex->lock();
    emit req_query_weight();
}
