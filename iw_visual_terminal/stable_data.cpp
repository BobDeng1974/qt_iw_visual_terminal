#include "stable_data.h"

stable_data::stable_data()
{
    m_size = 0;
    m_offset = 0;
    m_sum = 0;

    m_variance = -1;
    m_standard_deviation = -1;

    memset(m_buffer,0,CIRCLE_BUFFER_SIZE);

}

int stable_data::clear()
{
    /*
    m_size = 0;
    m_offset = 0;
    m_sum = 0;

    m_variance = -1;
    m_standard_deviation = -1;

    memset(m_buffer,0,CIRCLE_BUFFER_SIZE);
    */
    return 0;
}
int stable_data::put_data(int value)
{
    float variance = 0;
    m_variance = -1;
    m_standard_deviation = -1;

    /*没有装满*/
    if (m_size < CIRCLE_BUFFER_SIZE) {
        m_buffer[m_offset ++] = value;
        m_sum += value;
    } else {
        m_sum -= m_buffer[m_offset];
        m_buffer[m_offset ++] = value;
        m_sum += value;
        /*平均数*/
        m_average = m_sum / CIRCLE_BUFFER_SIZE;
        for (int i = 0; i < CIRCLE_BUFFER_SIZE; i++) {
            if (m_buffer[i] != m_average) {
                variance += pow(m_buffer[i] - m_average,2);
            }
        }
        /*方差*/
        m_variance = variance / CIRCLE_BUFFER_SIZE;
        /*标准差*/
        m_standard_deviation = sqrtf(m_variance);
        qDebug("size:%d 方差：%.2f 标准差：%.2f",m_size,m_variance,m_standard_deviation);

    }
    m_size ++;
    if (m_offset >= CIRCLE_BUFFER_SIZE) {
        m_offset = 0;
    }
    return 0;
}


float stable_data::variance()
{
    return m_variance;
}

float stable_data::standard_deviation()
{
    return m_standard_deviation;
}

