#include "bluechat.h"
#include "ui_bluechat.h"

//全局变量，保存下拉框所选择的文本信息
QString Mesg;

bluechat::bluechat(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::bluechat)
{
    ui->setupUi(this);

    connect(ui->btnClose,&QPushButton::clicked,[=](){
        if(this->isClose)
        {
            ui->btnClose->setText("开启");
            this->isClose = false;
        }
        else
        {
             ui->btnClose->setText("关闭");
             this->isClose = true;
        }
    });
    //清除数据接受区内容按钮点击事件
    connect(ui->btnClear,&QPushButton::clicked,[=](){
        ui->tbRevData->append("1123");
        ui->tbRevData->clear();
    });

}


bluechat::~bluechat()
{
    delete ui;
}
