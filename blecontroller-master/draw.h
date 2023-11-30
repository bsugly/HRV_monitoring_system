#ifndef DRAW_H
#define DRAW_H

#include <QObject>
#include <QThread>

class drawThread : public QThread
{
    Q_OBJECT
public:
    explicit drawThread(QObject *parent = nullptr);

signals:
    void drawSignal();//自定义信号
protected:
    void run() ; //线程入口函数

};


#endif // DRAW_H
