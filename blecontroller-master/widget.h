#ifndef WIDGET_H
#define WIDGET_H
#include <Python.h>
#include <QWidget>
#include <QDebug>
#include <QTreeWidget>
#include <QtCharts>
#include "bluetooth/ble.h"
#include "btdevcell.h"
#include "QListWidgetItem"
#include "lsl_rx_data.h"
#include "draw.h"

#define FIFO_BUFFER_SIZE 1200 // software buffer size (in bytes) 4s数据， 考虑buffer区域扩大为8s+区域
#define DRAW_ECG_INTERVAL 1
#define AXIS_X_MAX_COUNTS 100


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    QList<QBluetoothDeviceInfo> devInfos;
    ~Widget();

    void init();

public slots:
    void rx_data_slot();
    void drawslot();
    // 线程结束函数
    void quitThreadSlot();
private slots:
    void oneTimeOutReadFromFifoAction();
    void oneTimeOutWriteToFifoAction(qint16 tempInt16);
    void onStopPushButtonClick();

    void on_pushButton_connect_clicked();

    void on_pushButton_service_clicked();

    void on_pushButton_clicked();

    void on_pushButton_open_clicked();

    void on_pushButton_closed_clicked();

    void on_ShowPushButton_clicked();

    void slotReadTxt();//读文件

private:
    Ui::Widget *ui;
    // 线程对象
    rx_data_Thread *pRxdataThread;
    drawThread * pDrawThread;

    // ECG
    void initEcgWaveLineChart();
    void setLineChartMargins(QChart * chart, int margin);
    void drawEcgWave(int x,qint16 data);
    void readEcgData();

    void ecgDataFifoIn(qint16 inputData);
    qint16 ecgDataFifoOut();
    // ECG 图
    QChart *ecgWaveLineChart;
    QValueAxis *axisX_ECG;
    QValueAxis *axisY_ECG;
    QLineSeries *ecgSeries;//
    QStringList originList;

    QTimer *ecgWaveDrawTimer;
    QTimer *ecgWaveReadTimer;

    int originListSize;
    int originListIndex;
    int axis_x_counts;
    int16_t draw_i = 0; //画图标志位
    const char* data_ecg[AXIS_X_MAX_COUNTS];
//    double data_ecg[AXIS_X_MAX_COUNTS];
    qreal global_data = 0; ////线程数据
    QVector<QPointF> ecgPointBuffer;
    QQueue <qint16>m_EcgShortQueue;

    //******** fifo相关 ***********
    typedef struct {
        qint16  data_buf[FIFO_BUFFER_SIZE]; // FIFO buffer 16位有符号数
        quint16 i_first;                    // index of oldest data byte in buffer
        quint16 i_last;                     // index of newest data byte in buffer
        quint16 num_bytes;                  // number of bytes currently in buffer
    }sw_fifo_typedef;
    sw_fifo_typedef ecg_data_fifo = { {0}, 0, 0, 0 }; // declare a receive software buffer

                                //------------- fifo中标志位的配置 ---------------
    quint8 fifo_not_empty_flag; // this flag is automatically set and cleared by the software buffer
    quint8 fifo_full_flag;      // this flag is automatically set and cleared by the software buffer
    quint8 fifo_ovf_flag;       // this flag is not automatically cleared by the software buffer
    //蓝牙
    BLE * m_ble;
signals:
    void main_page();
};
#endif // WIDGET_H
