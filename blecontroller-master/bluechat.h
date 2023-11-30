#ifndef BLUECHAT_H
#define BLUECHAT_H

#include <QMainWindow>
#include "QLowEnergyService"

QT_BEGIN_NAMESPACE
namespace Ui { class bluechat; }
QT_END_NAMESPACE

class bluechat : public QMainWindow
{
    Q_OBJECT

    public:
        bluechat(QWidget *parent = nullptr);
        bool isClose = true;
        void ConnectCharacteristic(QLowEnergyService * m_service);

        ~bluechat();

    private:
        Ui::bluechat *ui;


//    private slots:
//        void Show_Main_Page();

    signals:
        void main_page();
};
#endif // BLUECHAT_H
