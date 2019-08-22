#ifndef STABLE_DATA_H
#define STABLE_DATA_H
#include "qqueue.h"
#include "qmath.h"
#include "qdebug.h"

class stable_data
{
public:
    int clear();
    stable_data();
    int put_data(int value);
    float variance();
    float standard_deviation();


    enum {
        CIRCLE_BUFFER_SIZE = 10
    };


private:
    int m_buffer[CIRCLE_BUFFER_SIZE];
    int m_size;
    int m_offset;
    float m_sum;
    float m_average;

    float m_variance;
    float m_standard_deviation;
};

#endif // STABLE_DATA_H
