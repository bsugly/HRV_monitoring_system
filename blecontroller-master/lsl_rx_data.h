#ifndef LSL_RX_DATA_H
#define LSL_RX_DATA_H
#include <QObject>
#include <QThread>

class rx_data_Thread : public QThread
{
    Q_OBJECT
public:
    explicit rx_data_Thread(QObject *parent = nullptr);

signals:
    void rx_data_signal();

protected:
    void run();
};
#endif // LSL_RX_DATA_H
