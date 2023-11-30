#include "widget.h"
#include "ui_widget.h"
#include "bluechat.h"
#include "PyThreadStateLock.h"
#include <QDebug>
#include <iostream>
qreal flag_draw_i = 0; //线程画图标志位

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    init();
    initEcgWaveLineChart();
    // 初始化python
    if (!Py_IsInitialized())
    {
        //1.初始化Python解释器，这是调用操作的第一步
        Py_Initialize();
        if (!Py_IsInitialized()) {
            qDebug("Initial Python failed!");
//            emit failed();
        }
        else {

            //执行单句Python语句，用于给出调用模块的路径，否则将无法找到相应的调用模块
            // 初始化线程支持
            PyEval_InitThreads();

            PyRun_SimpleString("import lsl");
            PyRun_SimpleString("lsl.path.append('D:/graduate-program/bitalino/proj/')"); // 设置绝对路径

            // 启动子线程前执行，为了释放PyEval_InitThreads获得的全局锁，否则子线程可能无法获取到全局锁。
            PyEval_ReleaseThread(PyThreadState_Get());
            qDebug("Initial Python Success!");

        }
    }

    // 创建线程对象
    pRxdataThread = new rx_data_Thread(this);
    pDrawThread = new drawThread(this);

    // 调用run函数，每隔1s发送信号
    pRxdataThread->start();
//    pDrawThread->start();

    ui->pushButton_open->setEnabled(false);

    connect(pRxdataThread,SIGNAL(rx_data_signal()),this,SLOT(rx_data_slot()));
    connect(pDrawThread,SIGNAL(drawSignal()),this,SLOT(drawslot));
    connect(ui->pushButton,SIGNAL(clicked(bool)),this,SLOT(slotReadTxt()));

    connect(this,SIGNAL(destroyed()),this,SLOT(quitThreadSlot()));

    // 设置主目录，就是python的主目录
    Py_SetPythonHome(L"C:\\Users\\gly19\\AppData\\Local\\Programs\\Python\\Python38");
    //设置设备列表的标题
//    QListWidgetItem *item = new QListWidgetItem();
    //创建自定义的窗口 然后放入到listwidget中
    BtDevCell *bdc = new BtDevCell();
    //设置label的显示
    bdc->btName->setText("Name");
    bdc->btAddr->setText("Address");
    bdc->btRssi->setText("Signal");

    // 心电图表格初始化
    axisX_ECG->setLinePen(QPen(Qt::red, 1, Qt::DashDotDotLine, Qt::SquareCap, Qt::RoundJoin));


}

Widget::~Widget()
{
    delete ui;
}

void Widget::init()
{
    originListIndex = 0;
    axis_x_counts = 0;
}

//初始化ECG,心电图折线图
void Widget::initEcgWaveLineChart() {


    //TODO 其实这里可以用默认的坐标轴
    axisY_ECG = new QValueAxis();
    axisX_ECG = new QValueAxis();
    ecgSeries = new QLineSeries();//TODO 考虑改为平滑曲线
    ecgWaveLineChart = new QChart();

    //添加曲线到chart中
    ecgWaveLineChart->addSeries(ecgSeries);

    //设置坐标轴显示范围
    axisY_ECG->setRange(-0.3, 0.3);
    axisX_ECG->setRange(0, AXIS_X_MAX_COUNTS);
    axisX_ECG->setTickCount(30);
    axisY_ECG->setTickCount(10);

    //设置坐标轴的颜色，粗细和设置网格显示
    axisX_ECG->setGridLinePen(QPen(Qt::red, 1, Qt::DashDotDotLine, Qt::SquareCap, Qt::RoundJoin)); //网格样式
    axisY_ECG->setGridLinePen(QPen(Qt::red, 1, Qt::DashDotDotLine, Qt::SquareCap, Qt::RoundJoin));

    axisX_ECG->setLinePen(QPen(Qt::red, 1, Qt::DashDotDotLine, Qt::SquareCap, Qt::RoundJoin));//坐标轴样式
    axisY_ECG->setLinePen(QPen(Qt::red, 1, Qt::DashDotDotLine, Qt::SquareCap, Qt::RoundJoin));

    axisY_ECG->setGridLineVisible(true);//显示线框
    axisX_ECG->setGridLineVisible(true);

    axisX_ECG->setLabelsVisible(false);//不显示具体数值
    axisY_ECG->setLabelsVisible(false);


    //把坐标轴添加到chart中，第二个参数是设置坐标轴的位置，
    //只有四个选项，下方：Qt::AlignBottom，左边：Qt::AlignLeft，右边：Qt::AlignRight，上方：Qt::AlignTop
    ecgWaveLineChart->addAxis(axisX_ECG, Qt::AlignBottom);
    ecgWaveLineChart->addAxis(axisY_ECG, Qt::AlignLeft);

    //把曲线关联到坐标轴
    ecgSeries->attachAxis(axisX_ECG);
    ecgSeries->attachAxis(axisY_ECG);
    ecgSeries->setColor(QColor(Qt::black));//设置线的颜色
    ecgSeries->setUseOpenGL(true);//openGL加速
    setLineChartMargins(ecgWaveLineChart, 2);//设置折线图边距

    ecgWaveLineChart->legend()->hide();//不显示注释
    ui->ecgWaveLineChart->setChart(ecgWaveLineChart);

}

void Widget::oneTimeOutReadFromFifoAction() {
    qDebug() << QString("fifo中剩余的数据个数%1").arg(m_EcgShortQueue.size());
        //环形队列如果内部没有数据的时候，fifoOut一直为0
        qint16 drawPoint = 0;
        if (axis_x_counts == AXIS_X_MAX_COUNTS)
        {
            axis_x_counts = 0;
        }
        if (m_EcgShortQueue.size() != 0)
        {
            drawPoint = m_EcgShortQueue.dequeue();
        }
        ecgSeries->replace(axis_x_counts, QPointF(axis_x_counts, drawPoint));//这种方式CPU消耗较小，相对于替换所有的PointBuffer，即下面的方法
        axis_x_counts++;

}

//模拟500ms收到一个数据包 以后这部分就可以是TCP或者是串口接收到一次数据就加入一次缓存队列
void Widget::oneTimeOutWriteToFifoAction(qint16 tempInt16) {

    for (int i = originListIndex; i < (125 + originListIndex); i++)
    {
//		qint16 tempInt16 = originList.at(i).toInt();
        m_EcgShortQueue.enqueue(tempInt16);
    }
    originListIndex += 125;

    //如果剩下的数据不足以支撑下一次数据读取
    if ((originListIndex + 125) >= originListSize)
    {
        ecgWaveReadTimer->stop();
    }
}

void Widget::on_ShowPushButton_clicked()
{
//        readEcgData();
//        ecgSeries->clear();
//        //提前添加完点，就不用考虑是否是第一次绘制，只用替换现有点的数据
//        for (int i = 0; i < AXIS_X_MAX_COUNTS; i++)
//        {
//            //ecgPointBuffer.append(QPointF(i, 0));
//            *ecgSeries << QPointF(i, 0);
//        }

//        //模拟TCP数据到来的定时器
//        ecgWaveReadTimer = new QTimer(this);
//        connect(ecgWaveReadTimer, SIGNAL(timeout()), this, SLOT(oneTimeOutWriteToFifoAction()));
//        ecgWaveReadTimer->start(500);//500ms执行一次

//        //******绘制折线定时器***********
//        ecgWaveDrawTimer = new QTimer(this);
//        ecgWaveDrawTimer->setInterval(DRAW_ECG_INTERVAL);
//        ecgWaveDrawTimer->setTimerType(Qt::PreciseTimer);//精确
//        connect(ecgWaveDrawTimer, SIGNAL(timeout()), this, SLOT(oneTimeOutReadFromFifoAction()));
//        ecgWaveDrawTimer->start();
    qDebug() << "flag_draw_i =" << flag_draw_i;
}

void Widget::onStopPushButtonClick() {
    ecgWaveDrawTimer->stop();

}

//**** 从文本框中读取ECG数据 ***
void Widget::readEcgData() {
//    QString origin = ui->inputTextEdit->toPlainText();
//    originList.clear();
//    originList = origin.split(",");
//    originListSize = originList.count();
//    qDebug() << QString("数据大小%1").arg(originListSize);
}

void Widget::ecgDataFifoIn(qint16 inputData) {
    //////////////////////////////////////////////
          /* Explicitly clear the source of interrupt if necessary */

    if (ecg_data_fifo.num_bytes == FIFO_BUFFER_SIZE) {      // if the sw buffer is full
        fifo_ovf_flag = 1;                     // set the overflow flag

    //TODO fifo_ovf_flag，这个标志影响是否还写入，或者是清空已有数据，然后重新谢写入？
    }
    else if (ecg_data_fifo.num_bytes < FIFO_BUFFER_SIZE) { // if there's room in the sw buffer

     ///////////////////////////////////////////////////
     /* read error/status reg here if desired         */
     /* handle any hardware RX errors here if desired */
     ///////////////////////////////////////////////////

     /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ecg_data_fifo.data_buf[ecg_data_fifo.i_last] = inputData;/* enter pointer to UART rx hardware buffer here */ // store the received data as the newest data element in the sw buffer
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ecg_data_fifo.i_last++;                              // increment the index of the most recently added element
        ecg_data_fifo.num_bytes++;                           // increment the bytes counter

    }
    if (ecg_data_fifo.num_bytes == FIFO_BUFFER_SIZE) {      // if sw buffer just filled up
        fifo_full_flag = 1;
        //TODO fifo_ovf_flag，这个标志影响是否还写入，或者是清空已有数据，然后重新谢写入？
        // set the  FIFO full flag
    }
    if (ecg_data_fifo.i_last == FIFO_BUFFER_SIZE) {         // if the index has reached the end of the buffer,
        ecg_data_fifo.i_last = 0;                            // roll over the index counter
    }
    fifo_not_empty_flag = 1;                 // set received-data flag
} // end UART RX IRQ handler


//如果在环形队列中没有数据，返回为0
qint16 Widget::ecgDataFifoOut() {
    //////////////////////////////////////////////

      /* Explicitly clear the source of interrupt if necessary */
    qint16 outputData = 0;
    if (ecg_data_fifo.num_bytes == FIFO_BUFFER_SIZE) { // if the sw buffer is full
        fifo_full_flag = 0;               // clear the buffer full flag because we are about to make room
    }

    if (ecg_data_fifo.num_bytes > 0) {                 // if data exists in the sw buffer

        outputData = ecg_data_fifo.data_buf[ecg_data_fifo.i_first]; // place oldest data element in the buffer

        ecg_data_fifo.i_first++;                        // increment the index of the oldest element
        ecg_data_fifo.num_bytes--;                      // decrement the bytes counter
    }
    if (ecg_data_fifo.i_first == FIFO_BUFFER_SIZE) {   // if the index has reached the end of the buffer,
        ecg_data_fifo.i_first = 0;                      // roll over the index counter
    }
    if (ecg_data_fifo.num_bytes == 0) {                // if no more data exists
        fifo_not_empty_flag = 0;          // clear flag
    }
    return outputData;
}

//设置chartView的边缘宽度
void Widget::setLineChartMargins(QChart *chart, int margin) {
    QMargins m_Margin;
    m_Margin.setLeft(margin);
    m_Margin.setBottom(margin);
    m_Margin.setRight(margin);
    m_Margin.setTop(margin);
    chart->setMargins(m_Margin);
}

void Widget::rx_data_slot()
{
    // 获取全局锁
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    PyObject * pModule = NULL;
    PyObject * pFunc = NULL;

    pModule = PyImport_ImportModule("lsl");//调用的Python文件名  py文件放置exe同级

    if (pModule == NULL)
    {
        PyErr_Print();
        std::cout << "PyImport_ImportModule Fail!" << std::endl;
        return;
    }



    // 画图
    if(draw_i <=AXIS_X_MAX_COUNTS)
    {
        // 设置调用的函数名
        pFunc = PyObject_GetAttrString(pModule, "received_data");

        // 转换输入数据，将字典输入
        PyObject* dict = PyDict_New();
    //    for(auto data : inputData){
    //        PyDict_SetItem(dict, Py_BuildValue("s", data.first.c_str()), Py_BuildValue("d", data.second)); // 这里要注意"s"表示c类型的字符串，所以要加c_str()否则报错
    //    }

        // 创建参数对象
        PyObject* args = PyTuple_New(1);
        PyTuple_SetItem(args, 0, dict); // 0表示数据dict在参数args中的位置索引

    //    PyObject *pReturn = NULL; //返回值
        PyObject* globalDict = PyModule_GetDict(pModule);

        PyObject* data = PyDict_GetItemString(globalDict, "data");
        PyObject* strObj = PyObject_Str(data);  // 获取对象的字符串表示
        const char* strValue = PyUnicode_AsUTF8(strObj);  // 将字符串对象转换为C字符串

        PyObject* data_time = PyDict_GetItemString(globalDict, "data_time");
        PyObject* strObj_time = PyObject_Str(data_time);  // 获取对象的字符串表示
        const char* strValue_time = PyUnicode_AsUTF8(strObj_time);  // 将字符串对象转换为C字符串

//        std::cout<<"result_time: "<<strValue_time<<std::endl;
        std::cout<<"result_data: "<<strValue<<std::endl;

//        ui->lineEdit->setText(strValue);

        QString strValue_data(strValue);
        double realvalue_data = strValue_data.toDouble();


//         使用 QTextStream 设置输出精度
//        QTextStream out(stdout);
//        out.setRealNumberPrecision(20);
//        out << "Value: " << realvalue_data << endl;

        data_ecg[draw_i] = strValue;
        ui->lineEdit->setText(strValue_time);
//        std::cout<<"result_time: "<<data_ecg[draw_i]<<std::endl;
//        std::cout<<"result_data: "<<realvalue_data<<std::endl;

        if(qstrcmp(data_ecg[draw_i],data_ecg[draw_i-1]) != 0)
        {
            ecgSeries->append(draw_i, realvalue_data);
//            std::cout<<"realvalue_data: "<<realvalue_data<<std::endl;
            draw_i++;
            flag_draw_i++;//计算包长度
            qDebug() << "flag_draw_i =" << flag_draw_i;
            qDebug() << "draw_i =" << draw_i;
        }
    } else{
            draw_i = 0;
            data_ecg[AXIS_X_MAX_COUNTS] = {0};
            ecgSeries->clear();
        }


    // Py_Finalize();//注意这一句不能要，否则会运行崩溃
    // 释放全局锁
    PyGILState_Release(gstate);

//    // DEBUG---1.获取一个随机数
//    QString str_rand=QString("%1").arg(qrand()%30);
//    // 2.显示到文本
//    ui->lineEdit->setText(str_rand);
}

void Widget::drawslot()
{
    //画图函数---这里选用搜索蓝牙例子


}

void Widget::quitThreadSlot()
{
    pRxdataThread->quit();
    pRxdataThread->wait();

    pDrawThread->quit();
    pDrawThread->wait();
}

void Widget::on_pushButton_open_clicked()
{
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_closed->setEnabled(true);
    pRxdataThread->start();
}

void Widget::on_pushButton_closed_clicked()
{
    ui->pushButton_closed->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    pRxdataThread->terminate();
}


void Widget::on_pushButton_connect_clicked()
{

}

void Widget::on_pushButton_service_clicked()
{

}

void Widget::on_pushButton_clicked()
{
        bluechat *blt= new bluechat();
//        int blt = blt.Show_Main_Page();
//        this->hide();//隐藏登录对话框
        blt->show();


}
void Widget::slotReadTxt()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr,"Open",QApplication::applicationDirPath(),"*.txt;;*.*");
    if(fileName.isEmpty())
        return;

    ui->lineEdit->setText(fileName);
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(nullptr,"Warning","Open file failed!");
        return;
    }

    QTextStream text(&file);
    while (!text.atEnd()) {
        QString strLine = text.readLine();
        ui->textEdit->append(strLine);
    }
}

// ...

void MyObject::receiveMessage(const QString& message)
{
    // 在这里处理接收到的消息
}




