#include "draw.h"

drawThread::drawThread(QObject *parent) : QThread(parent)
{

}

void drawThread::run()
{
    while(1)
    {
        emit drawSignal();
        QThread::usleep(10000);
    }
}
