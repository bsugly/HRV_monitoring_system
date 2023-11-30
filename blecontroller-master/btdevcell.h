#ifndef BTDEVCELL_H
#define BTDEVCELL_H

#include <QWidget>
#include "QLabel"
#include "QListWidgetItem"

namespace Ui {
class BtDevCell;
}

class BtDevCell : public QWidget
{
    Q_OBJECT

public:
    explicit BtDevCell(QWidget *parent = nullptr);
    ~BtDevCell();
    QLabel *btName;
    QLabel *btAddr;
    QLabel *btRssi;

private:
    Ui::BtDevCell *ui;

};

#endif // BTDEVCELL_H
