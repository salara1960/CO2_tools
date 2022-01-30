#ifndef BLE_DIALOG_H
#define BLE_DIALOG_H

//----------------------------------------------------------------------------------

#include <QDialog>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>

//----------------------------------------------------------------------------------

namespace Ui {
    class bleDialog;
}

//----------------------------------------------------------------------------------
class bleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit bleDialog(QWidget *parent = nullptr);
    ~bleDialog();

public slots:
    void devSelected();

    void addDev(QList<QBluetoothDeviceInfo> *);

signals:
    void sig_sndSelInd(int);

private:
    Ui::bleDialog *ui;
    int idx;

};
//----------------------------------------------------------------------------------

#endif // BLE_DIALOG_H
