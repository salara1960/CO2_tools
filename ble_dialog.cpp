#include "defs.h"
#include "ble_dialog.h"
#include "ui_ble_dialog.h"
#include <QStringList>

//----------------------------------------------------------------------------------
bleDialog::bleDialog(QWidget *parent) : QDialog(parent), ui(new Ui::bleDialog)
{
    ui->setupUi(this);

    ui->blebox->clear();
    ui->blebox->addItems(QStringList() << "Empty");
    idx = -1;
    //
    //connect(this, SIGNAL(quit()), this, SLOT(done()));
    connect(ui->select, &QPushButton::pressed, this, &bleDialog::devSelected);
}
//----------------------------------------------------------------------------------
bleDialog::~bleDialog()
{
    delete ui;
}
//----------------------------------------------------------------------------------
void bleDialog::addDev(QList<QBluetoothDeviceInfo> *lst)
{
    ui->blebox->clear();
    int i = -1;
    QStringList stx;
    //stx << "No device selected";
    QString tp, tmp;
    while (++i < lst->size()) {
        tp = lst->at(i).name().trimmed() +
               " (" +
               lst->at(i).address().toString().trimmed() +
               ") rssi:" +
               QString::number(lst->at(i).rssi(), 10);
        stx << tp;
        tmp.append(tp + "<br>");

    }
    ui->blebox->addItems(stx);

    ui->text->setHtml(tmp);


    /*if (stx.size()) {
        QString st, serv;
        for (int i = 0; i < stx.size(); i++) {
            st.append(stx.at(i) + "<br>");
            if (lst->at(i).serviceUuids().size()) st.append("   services:<br>");
            int j = -1;
            serv.clear();
            while (++j < lst->at(i).serviceUuids().size()) {
                serv += "    " + lst->at(i).deviceUuid().toString() + "<br>";
            }
            st.append(serv);
        }
        ui->text->setHtml(st);
    }*/
}
//----------------------------------------------------------------------------------
void bleDialog::devSelected()
{
    idx = ui->blebox->currentIndex();
    ui->text->setHtml("Index=" + QString::number(idx, 10) + "\ndev : " + ui->blebox->itemText(idx));
    if (idx >= 0) {
        emit sig_sndSelInd(idx);
    }
}
//----------------------------------------------------------------------------------



