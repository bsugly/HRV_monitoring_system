#include "lsl_rx_data.h"

rx_data_Thread::rx_data_Thread(QObject *parent) : QThread(parent)
{

}

void rx_data_Thread::run()
{
//    调用lsl接受数据函数----python
    while(1)
    {
        emit rx_data_signal();
        QThread::usleep(1);
    }
}
