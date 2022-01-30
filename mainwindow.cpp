#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "widget.h"
#include "ble_dialog.h"
#include "mainwindow.h"
//******************************************************************************************************

qint32 serSpeed = 230400;//115200;

//******************************************************************************************************

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)//, conf(new SettingsDialog)
{
    ui->setupUi(this);

    MyError = 0;

    this->setWindowOpacity(0.94);//set the level of transparency
    this->setWindowIcon(QIcon(main_pic));
    this->setStyleSheet(tTip);
    ui->log->setReadOnly(true);


    first  = true;
    con = false;
    sdevName = "";
    sdev = nullptr;
    ms10 = 0;
    devTime = 0;
    rxData.clear();
    fileName.clear();
    chap.clear();
    tmr_rst = 0;

    graf = new Widget(ui->graf);

    //----  Set timer with period to 10 msec ----
    tmr_sec = startTimer(10);// 10 msec.
    if (tmr_sec <= 0) {
        MyError |= 2;//start_timer error
        throw TheError(MyError);
    }
    //--------------------------------------------

    ui->actionTemp->setVisible(false);
    ui->actionHUMI->setVisible(false);

    connect(this, SIGNAL(sigWrite(QByteArray &)), this, SLOT(slotWrite(QByteArray &)));

    connect(ui->actionVERSION,    &QAction::triggered,         this, &MainWindow::About);
    connect(ui->actionCONNECT,    &QAction::triggered,         this, &MainWindow::on_connect);
    connect(ui->actionDISCONNECT, &QAction::triggered,         this, &MainWindow::on_disconnect);
    connect(ui->actionCLEAR,      &QAction::triggered,         this, &MainWindow::clrLog);
    connect(this,                 &MainWindow::sig_grafic,     this, &MainWindow::grafic);
    connect(this,                 &MainWindow::sigConn,        this, &MainWindow::on_connect);
    connect(this,                 &MainWindow::sigDisc,        this, &MainWindow::on_disconnect);
    connect(this,                 &MainWindow::sig_on_answer,  this, &MainWindow::on_answer);
    connect(this,                 &MainWindow::sig_rst_screen, this, &MainWindow::rst_screen);

    ui->actionCONNECT->setEnabled(true);
    ui->actionPORT->setEnabled(true);
    ui->actionDISCONNECT->setEnabled(false);
    ui->actionCLEAR->setEnabled(true);

    //-------------------------------------------------------------------------------------------

    ui->cmd->setText("get");
    ui->cmd->setEnabled(false);
    ui->crlfBox->setCheckState(Qt::Checked);
    ui->crlfBox->setEnabled(false);
    ui->log->setEnabled(false);

    QFont font = QFont("Helvetica", 12);
    this->setFont(font);
    QString attr = "{border: 4px 4px 4px 4px;\
            border-color: rgb(0, 0, 0);\
            font: italic 14pt \"Sans Serif\";\
            color: rgb(255, 255, 255);}";
    ui->date_time->setStyleSheet("QLabel" + attr);
    ui->val_temp->setStyleSheet("QLabel" + attr);
    ui->val_humi->setStyleSheet("QLabel" + attr);
    ui->val_que->setStyleSheet("QLabel" + attr);


    ui->log->setTextColor(Qt::blue);

    this->setWindowTitle(title + " ver." + qApp->applicationVersion());

    conf = new SettingsDialog;
    if (!conf) {
        MyError |= 4;//create settings object error
        throw TheError(MyError);
    }

    connect(ui->actionPORT,       &QAction::triggered,     conf, &SettingsDialog::sig_confShow);
#ifdef SET_BLUETOOTH
    connect(ui->actionBLE, &QAction::triggered,         this, &MainWindow::beginBle);
    connect(this,          &MainWindow::sig_bleTimeOut, this, &MainWindow::slot_bleTimeOut);
    connect(this,          &MainWindow::sig_bleDone,    this, &MainWindow::slot_bleDone);
    connect(this,          &MainWindow::sig_bleGo,      this, &MainWindow::bleGo);
    ui->actionBLE->setVisible(true);
    ui->actionBLE->setEnabled(true);
#else
    ui->actionBLE->setVisible(false);
#endif

    ui->log->setTextColor(Qt::blue);

}
//----------------------------------------------------------------------------------------
//   Реализация диструктор основного класса программы
//
MainWindow::~MainWindow()
{
#ifdef SET_BLUETOOTH
    if (bleSocket) {
        bleSocket->disconnect();
        if (bleSocket->isOpen()) bleSocket->close();
        delete bleSocket;
        bleSocket = nullptr;
    }
    //slot_bleDone();
#endif

    deinitSerial();

    if (graf) delete graf;

    if (conf) serSpeed = conf->settings().baudRate;

    if (conf) delete conf;

    killTimer(tmr_sec);

    delete ui;
}
//-----------------------------------------------------------------------
//   Обработчик события от таймера - "истекло 10 мсек"
//
void MainWindow::timerEvent(QTimerEvent *event)
{
    if (tmr_sec == event->timerId()) {
        if (!(++ms10 % 100)) {
            if (devTime) {
                devTime++;
                one.dt = devTime;
            }
            ui->date_time->setText(" " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss"));
            ui->date_time->setToolTip("Текущая дата и время");
        }
    }
#ifdef SET_BLUETOOTH
    else if ((tmr_ble > 0) && (tmr_ble == event->timerId())) {
        emit sig_bleTimeOut();
    }
#endif
    else if (tmr_rst == event->timerId()) {
        emit sig_rst_screen();
    }

}
//-------------------------------------------------------------------------------------
//   Обработчик события - "нажата клавиша Enter на клавиатуре"
//
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) emit sig_on_answer();
}
//--------------------------------------------------------------------------------
QString MainWindow::time2str()
{
    return (QDateTime::currentDateTime().toString("dd.MM hh:mm:ss") + " | ");
}
//--------------------------------------------------------------------------------
//         Error class
//
MainWindow::TheError::TheError(int err)//error class descriptor
{
    code = err;
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
void MainWindow::clrLog()
{
    ui->log->clear();
}
//-----------------------------------------------------------------------
void MainWindow::toStatusLine(QString st, int pic)
{
    ui->status->clear();

    if (!st.length()) {
        ui->pic->clear();
        return;
    }

    QString pix;

    switch (pic) {
        case picClr:
            ui->pic->clear();
        break;
        case picInfo:
            pix = info_pic;
        break;
        case picTime:
            pix = timeout_pic;
        break;
        case picWar:
            pix = war_pic;
        break;
        case picErr:
            pix = err_pic;
        break;
        case picCon:
            pix = ucon_pic;
        break;
        case picDis:
            pix = udis_pic;
        break;
        case picOk:
            pix = ok_pic;
        break;
    }

    ui->pic->setPixmap(QPixmap(pix));
    ui->status->setText(st);
}
//-----------------------------------------------------------------------
void MainWindow::LogSave(const QString & st, Qt::GlobalColor cvet)
{
    if (st.length()) {
        ui->log->setTextColor(cvet);
        ui->log->append(st);
    }
}
//--------------------------------------------------------------------------------
int MainWindow::initSerial()
{
    deinitSerial();

    sdev = new QSerialPort(sdevName);
    if (sdev) {
        SettingsDialog::Settings p = conf->settings();
        sdevName = p.name;   sdev->setPortName(sdevName);
        sdev->setBaudRate(p.baudRate);
        sdev->setDataBits(p.dataBits);
        sdev->setParity(p.parity);
        sdev->setFlowControl(p.flowControl);
        sdev->setStopBits(p.stopBits);

        if (!sdev->open(QIODevice::ReadWrite)) {
            delete sdev;
            sdev = nullptr;
            return 1;
        } else {

            while (!sdev->atEnd()) rxData = sdev->readAll();
            rxData.clear();

            connect(sdev, &QSerialPort::readyRead, this, &MainWindow::ReadData);
            connect(sdev, &QSerialPort::errorOccurred, this, &MainWindow::slotError);

            return 0;
        }
    } else {
        MyError |= 1;//create serial_port_object error
        throw TheError(MyError);
    }

}
//--------------------------------------------------------------------------------
//   Метод удаляет созданный для последовательного порта объект
//
void MainWindow::deinitSerial()
{
    if (sdev) {
        if (sdev->isOpen()) sdev->close();
        sdev->disconnect();
        delete sdev;
        sdev = nullptr;
        rxData.clear();
    }
}
//--------------------------------------------------------------------------------
//   Без коментариев ....
//
void MainWindow::About()
{
    QString st = QString(tr("\nСервисная программа для устройства\n'CO2_sensor' - STM32F411 + MQ135 + SI7021 + GC9A01"));

    st.append("\nВерсия " + qApp->applicationVersion() + tr(" ") + BUILD);

    QMessageBox box;
    box.setStyleSheet("background-color: rgb(208, 208, 208);");
    box.setIconPixmap(QPixmap(salara_pic));
    box.setText(st);
    box.setWindowTitle(tr("О программе"));

    box.exec();
}
//-----------------------------------------------------------------------
//   Обработчик события - "подключиться к последовательному порту"
//
void MainWindow::on_connect()
{

    devTime = 0;

    if (con) return;

    if (!initSerial()) {

        ui->graf->setEnabled(true);
        memset((uint8_t *)&one, 0, sizeof(data_t));
        emit graf->sig_refresh((void *)&one, msg_con);

        toStatusLine(tr("Подключен к %1 : %2 %3%4%5 FlowControl %6")
                          .arg(sdevName)
                          .arg(conf->settings().stringBaudRate)
                          .arg(conf->settings().stringDataBits)
                          .arg(conf->settings().stringParity.at(0))
                          .arg(conf->settings().stringStopBits)
                          .arg(conf->settings().stringFlowControl), picCon);
        con = true;
        ui->actionCONNECT->setEnabled(false);
        ui->actionPORT->setEnabled(false);
        ui->actionDISCONNECT->setEnabled(true);
        ui->actionCLEAR->setEnabled(true);

        ui->cmd->setEnabled(true);
        ui->crlfBox->setEnabled(true);
        ui->log->setEnabled(true);


        ui->actionPORT->setToolTip(tr("Параметры последовательного порта:\n%1 %2 %3%4%5")
                                   .arg(sdevName)
                                   .arg(conf->settings().stringBaudRate)
                                   .arg(conf->settings().stringDataBits)
                                   .arg(conf->settings().stringParity.at(0))
                                   .arg(conf->settings().stringStopBits));

        on_answer();

    } else {
        toStatusLine(tr("Ошибка при открытии последовательного порта ") + sdevName, picErr);
        deinitSerial();
    }
}
//-----------------------------------------------------------------------
//   Обработчик события - "отключиться от последовательного порта"
//
void MainWindow::on_disconnect()
{

    devTime = 0;

    memset((uint8_t *)&one, 0, sizeof(data_t));
    emit graf->sig_refresh((void *)&one, msg_dis);

    if (!con) return;

    con = false;

    deinitSerial();

    toStatusLine(tr("Отключен от последовательного порта ") + sdevName, picDis);

    ui->log->setEnabled(false);
    ui->actionPORT->setEnabled(true);
    ui->actionCONNECT->setEnabled(true);
    ui->actionDISCONNECT->setEnabled(false);
    ui->cmd->setEnabled(false);
    ui->crlfBox->setEnabled(false);
    ui->graf->setEnabled(false);

}
//-----------------------------------------------------------------------
void MainWindow::rst_screen()
{
    killTimer(tmr_rst);
    tmr_rst = 0;

    devTime = 0;
    memset((uint8_t *)&one, 0, sizeof(data_t));
    if (graf) {
        emit graf->sig_refresh((void *)&one, msg_rst);
    }
}
//-----------------------------------------------------------------------
//   Метод-слот осуществляет выдачу стоки данных на устройство
//    через последовательный интерфейс
//
void MainWindow::slotWrite(QByteArray & mas)
{
    toStatusLine("", picClr);

    if (sdev) {
        QString m(mas);
        if (m.indexOf("restart") != -1) {

            tmr_rst = startTimer(3500);// 4 sec.
            if (tmr_rst <= 0) {
                MyError |= 2;//start_timer error
                throw TheError(MyError);
            }

        }
        if (con) sdev->write(m.toLocal8Bit());
    }
}
//-----------------------------------------------------------------------
void MainWindow::on_answer()
{
    QByteArray tmp = ui->cmd->text().toLocal8Bit();

    if (ui->crlfBox->checkState() == Qt::Checked) tmp.append(cr_lf);

    if (tmp.length() >= 2) emit sigWrite(tmp);
}
//------------------------------------------------------------------------------------
//  HEX string to BIN byte function
//
unsigned char MainWindow::myhextobin(const char *uk)
{
char line[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
unsigned char a = 0, b = 0, c = 0, i;

    for	(i = 0; i < 16; i++) { if (*uk == line[i]) { b = i; break; } else b = 0xff; }
    for	(i = 0; i < 16; i++) { if (*(uk + 1) == line[i]) { c = i; break; } else c = 0xff; }
    if ((b == 0xff) || (c == 0xff)) a = 0xff; else { b <<= 4;   a = b | c; }

    return a;
}
//-----------------------------------------------------------------------
//  Функция преобразует hex-строку в бинарный массив
//
void MainWindow::hexTobin(const char *in, QByteArray *out)
{
size_t i = 0, len = strlen(in) / 2;
const char *uk = in;

    while (i < len) {
        out->append( static_cast<char>(myhextobin(uk)) );
        i++;
        uk += 2;
    }
}
//-----------------------------------------------------------------------
//   Метод определения события "ответ получен" при приеме данных от устройства
//
int MainWindow::chkDone(QByteArray *buf)
{
int ret = -1;
QByteArray mas;

    int pos = buf->indexOf("0.", 0);
    if (pos > 0) {
        mas = buf->mid(pos, buf->length() - pos);
        *buf = mas;
    }

    if (buf->indexOf(jBegin, 0) == 0) {
        ret = buf->indexOf(jEnd, 0);
        if (ret != -1) ret += jEnd.length();
    } else {
        ret = buf->indexOf(cr_lf, 0);
        if (ret != -1) ret += cr_lf.length();
    }

    return ret;
}
//-----------------------------------------------------------------------------------
void MainWindow::mkDataFromStr(QString str)
{

    if (!str.length()) return;

    int pos = 0, pend = 0, end = str.length();
    float val = 0.0;
    QString tmp;
    bool ok = false;

    pos = str.indexOf(" | [que:", 0); //0.00:09:20 | [que:1] MQ135: adc=903 ppm=320, SI7021: temp=23.59 humi=24.40
    if (pos != -1) {
        tmp = str.mid(0, pos);
        QByteArray bf;
        bf.append(tmp);
        if (sscanf(bf.data(), "%d.%d:%d:%d", &dev_time.day, &dev_time.hour, &dev_time.min, &dev_time.sec) == 4) {
            devTime = one.dt = (dev_time.day * 86400) + (dev_time.hour * 3600) + (dev_time.min * 60) + dev_time.sec;
        }
    }

    pos = str.indexOf(_temp, 0);
    if (pos != -1) {
        pos += _temp.length();
        pend = str.indexOf(" ", pos);
        if (pend == -1) pend = end;
        if (pend != -1) {
            tmp = str.mid(pos, pend - pos);
            val = tmp.toFloat(&ok);
            if (ok) {
                one.temp = val;
            }
        }
    }
    pos = str.indexOf(_humi, 0);
    if (pos != -1) {
        pos += _humi.length();
        pend = str.indexOf(" ", pos);
        if (pend == -1) pend = end;
        if (pend != -1) {
            tmp = str.mid(pos, pend - pos);
            val = tmp.toFloat(&ok);
            if (ok) {
                one.humi = val;
            }
        }
    }
    pos = str.indexOf(_ppm, 0);
    if (pos != -1) {
        pos += _ppm.length();
        pend = str.indexOf(",", pos);
        if (pend == -1) pend = end;
        if (pend != -1) {
            tmp = str.mid(pos, pend - pos);
            val = tmp.toFloat(&ok);
            if (ok) {
                one.ppm = val;
                one.co2 = val / 1000.0;
            }
        }
    }
    pos = str.indexOf(_que, 0);
    if (pos != -1) {
        pos += _que.length();
        pend = str.indexOf("]", pos);
        if (pend == -1) pend = end;
        if (pend != -1) {
            tmp = str.mid(pos, pend - pos);
            pos = tmp.toInt(&ok, 10);
            if (ok) one.que = pos;
        }
    }

    if (one.temp && one.humi && one.ppm) {
        tmp.sprintf(" %.2f C", one.temp);
        ui->val_temp->setText(tmp);
        ui->val_temp->setToolTip("Температура (град.Цельсия)");

        tmp.sprintf(" %.2f %%", one.humi);
        ui->val_humi->setText(tmp);
        ui->val_humi->setToolTip("Влажность (проценты)");

        tmp.sprintf(" %.2f %%", one.co2);
        QString attr = "{color: rgb(0, 95, 0); background-color: rgb(0, 95, 0);}";//GREEN
        if ((one.ppm > 400.0) && (one.ppm < 600)) attr = "{color: rgb(0, 95, 0); background-color: rgb(255, 255, 0);}";//YELLOW
        else
        if ((one.ppm > 600.0) && (one.ppm < 1000)) attr = "{color: rgb(0, 95, 0); background-color: rgb(255, 0, 255);}";//MAGENTA;
        else
        if (one.ppm > 1000.0) attr = "{color: rgb(0, 95, 0); background-color: rgb(255, 0, 0);}";//RED;


        ui->val_que->setText(" " + QString::number(one.que, 10));
        ui->val_que->setToolTip("Количество сообщений в\nбуфере fifo устройства");

        tmp = tr("Queue: %1  Sensors: %2%3 C  %4%5 %  %6%7  CO2=%8 %")
                .arg(one.que)
                .arg(_temp).arg(one.temp)
                .arg(_humi).arg(one.humi)
                .arg(_ppm).arg(one.ppm).arg(one.co2);
        toStatusLine(tmp, picInfo);

        emit sig_grafic(msg_evt);
    }
}
//-----------------------------------------------------------------------
void MainWindow::ReadData()
{
int ix = -1;

    while (!sdev->atEnd()) {
        rxData += sdev->readAll();
        ix = chkDone(&rxData);
        if (ix != -1) {
            QString line;
            int pos = 0;
            line.append(rxData.mid(0, ix));
            QString lin(line);
            if (lin.indexOf(jBegin, 0) == 0) {
                jsMode = true;
                if ((pos = line.indexOf(jEnd, 0)) != -1) line.remove(pos, jEnd.length());
            } else {
                if ((pos = line.indexOf(cr_lf, 0)) != -1) line.remove(pos, cr_lf.length());
                jsMode = false;
            }
            mkDataFromStr(line);
            LogSave(line, Qt::black);
            if (rxData.length() >= (ix + 1)) rxData.remove(0, ix);
                                        else rxData.clear();
            break;
        }
    }

}
//-----------------------------------------------------------------------
void MainWindow::slotError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        MyError |= 8;//error reading from serial port
        throw TheError(MyError);
    }
}
//-----------------------------------------------------------------------------------
uint32_t MainWindow::get10ms()
{
    return ms10;
}
//---------------------------------------
uint32_t MainWindow::get_tmr(uint32_t tm)
{
    return (get10ms() + tm);
}
//---------------------------------------
int MainWindow::check_tmr(uint32_t tm)
{
    return (get10ms() >= tm ? 1 : 0);
}
//----------------------------------------------------------------------
int MainWindow::writes(const char *data, int len)
{
int ret = 0;

    if (sdev) {
        if (sdev->isOpen()) ret = static_cast<int>(sdev->write(data, len));
    }

    return ret;
}
//-----------------------------------------------------------------------
void MainWindow::grafic(int cd)
{
    if (graf) emit graf->sig_refresh((void *)&one, cd);
}
//-----------------------------------------------------------------------

#ifdef SET_BLUETOOTH

//-----------------------------------------------------------------------
//-------------------------   Bluetooth   -------------------------------
//-----------------------------------------------------------------------

void MainWindow::beginBle()
{
    ui->actionBLE->setChecked(true);
    ui->log->setEnabled(true);


    list.clear();
    index = -1;
    bleWin = nullptr;

    bleFind = false;
    bleDevStr.clear();
    bleAddrStr.clear();
    bleDevNameAddr.clear();
    //

    if (localDevice) {
        delete localDevice;
        localDevice = nullptr;
    }
    localDevice = new QBluetoothLocalDevice(this);
    if (localDevice->isValid()) {
        localDevice->powerOn();
        //localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        localDevice->setHostMode(QBluetoothLocalDevice::HostConnectable);
        LogSave("\n" + time2str() + "Local bluetooth device: " + localDevice->name() +
                " (" + localDevice->address().toString().trimmed() + ")", Qt::blue);
        //
        if (discoveryAgent) {
            discoveryAgent->disconnect();
            delete discoveryAgent;
            discoveryAgent = nullptr;
        }
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        if (discoveryAgent) {
            LogSave(time2str() + "Start discovery bluetooth devices...", Qt::blue);
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,         this, &MainWindow::bleDiscoverFinished);
            connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled,         this, &MainWindow::bleDiscoverFinished);
            //connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &MainWindow::bleAddDevice);
#ifdef SET_BLE_DEVICE
            discoveryAgent->setLowEnergyDiscoveryTimeout(tmr_ble_wait);
            connect(discoveryAgent,
                    static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(&QBluetoothDeviceDiscoveryAgent::error),
                    this,
                    &MainWindow::bleScanError);
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#else
            discoveryAgent->start();
#endif
        } else LogSave(time2str() + "Error create QBluetoothDeviceDiscoveryAgent !", Qt::blue);
    } else LogSave(time2str() + "Bluetooth is not available on this device", Qt::blue);

}
//-----------------------------------------------------------------------
void MainWindow::bleAddDevice(const QBluetoothDeviceInfo & param)
{
#ifdef SET_BLE_DEVICE
    if (!(param.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)) return;
#endif
    if (!bleFind) {
        QString st;
        if (param.name().indexOf(bleRemoteMarker) != -1) {
            st = bleRemoteMarker + " bluetooth device found : " + param.name() + " (" + param.address().toString() + ")";
            if (param.address().toString() == bleRemoteMac) {
                discoveryAgent->disconnect();
                discoveryAgent->stop();
                infoDev = param;
                bleDevStr = infoDev.name().trimmed();
                bleAddrStr = infoDev.address().toString().trimmed();
                bleDevNameAddr = bleDevStr +
                        " (" + bleAddrStr +
                        ") rssi: " + QString::number(infoDev.rssi(), 10);
                bleFind = true;
            }
            toStatusLine(st, picInfo);
            LogSave(time2str() + st, Qt::blue);
        }

        if (bleFind) emit sig_bleGo();

    }
}
//-----------------------------------------------------------------------
void MainWindow::bleScanError(QBluetoothDeviceDiscoveryAgent::Error er)
{
    toStatusLine("Error scan ble_devices. (" + QString::number(static_cast<int>(er), 10) + ")", picErr);

    emit sig_bleDone();
}
//-----------------------------------------------------------------------
void MainWindow::bleDiscoverFinished()
{
    QString st = "Device discover finished";
    LogSave(time2str() + st, Qt::blue);
    //
    //QList<QBluetoothDeviceInfo>ls;
    //ls.clear();
    list.clear();
    list = discoveryAgent->discoveredDevices();
    discoveryAgent->disconnect();
    discoveryAgent->stop();
    if (!list.isEmpty()) {
        for (int i = 0; i < list.size(); i++) {
            //if (list.at(i).name().indexOf(bleRemoteMarker) != -1) {
                st = "Device: " + list.at(i).name().trimmed() +
                     " (" + list.at(i).address().toString().trimmed() +
                     ") rssi: " + QString::number(list.at(i).rssi(), 10);
                //ls.append(list.at(i));
                LogSave(time2str() + st, Qt::blue);
            //}
        }
    } else {
        st = "No devices found";
        LogSave(time2str() + st, Qt::blue);
    }

    if (!list.isEmpty()) {
        //list.clear();
        //list = ls;
        emit sig_bleGo();
    } else {
        emit sig_bleDone();
    }
}
//-----------------------------------------------------------------------
void MainWindow::slot_bleDone()
{

    LogSave(time2str() + "BLE done processing.", Qt::blue);

    if (tmr_ble > 0) {
        killTimer(tmr_ble);
        tmr_ble = 0;
    }

    if (bleSocket) {
        bleSocket->disconnect();
        //delete bleSocket;
        bleSocket = nullptr;
    }
    ble_connect = false;

    index = -1;
    list.clear();
    if (bleWin) {
        delete bleWin;
        bleWin = nullptr;
    }

    //if (!con) ui->log->setEnabled(false);
    ui->actionBLE->setChecked(false);

}
//-----------------------------------------------------------------------
void MainWindow::slot_bleTimeOut()
{
    killTimer(tmr_ble);
    tmr_ble = 0;

    QPixmap pm(war_pic);
    QString st = "No selected bluetooth device !";
    if (index >= 0) st = "Timeout connect to device " + bleDevNameAddr;
    toStatusLine(st, picWar);
    LogSave(time2str() + st, Qt::blue);

    emit sig_bleDone();

    QMessageBox::critical(this, "Error", st, QMessageBox::StandardButton::Ok);
}
//-----------------------------------------------------------------------
void MainWindow::getDevIndex(int ix)
{
    index = ix;
    if (index >= 0) {
        bleFind = true;
        infoDev = list.at(index);
        bleDevNameAddr = infoDev.name().trimmed() + " (" + infoDev.address().toString().trimmed() + ")";

        if (bleWin) {
            delete bleWin;
            bleWin = nullptr;
        }

        emit sig_bleGo();
    } else {
        emit sig_bleDone();
    }
}
//-----------------------------------------------------------------------
void MainWindow::bleGo()
{
    //
    if (list.size() && (index < 0)) {
        if (!bleWin) {
            bleWin = new bleDialog(this);
            if (bleWin) {
                bleWin->addDev(&list);
                bleWin->show();
                connect(bleWin, &bleDialog::sig_sndSelInd, this, &MainWindow::getDevIndex);
                tmr_ble = startTimer(60000);// wait connection 5 sec.
                if (tmr_ble <= 0) {
                    MyError |= 2;//start_timer error
                    throw TheError(MyError);
                }
            }
        }
    }
    //
    if (index < 0) return;
    //
    if (bleSocket) {
        bleSocket->disconnect();
        delete bleSocket;
        bleSocket = nullptr;
    }
    //
    QString st;
    int pf = picErr;

    //QBluetoothRfcommSocket bs;

    bleSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    if (bleSocket) {
        blePack.clear();
        //QBluetoothUuid uuid(tr("FFE0--0000-1000-8000-00805F9B34FB"));//"{0000110E-0000-1000-8000-00805F9B34FB}"; // UUID of the 'A/V Remote Control' service
        QBluetoothUuid uuid = QBluetoothUuid(QBluetoothUuid::SerialPort);
        /*
        if (localDevice->pairingStatus(infoDev.address()) != QBluetoothLocalDevice::Paired) {
            localDevice->requestPairing(infoDev.address(), QBluetoothLocalDevice::Paired);
        }
        */
        bleSocket->connectToService(infoDev.address(), uuid, QIODevice::ReadWrite);
        //bleSocket->connectToService(infoDev.address(), QIODevice::ReadWrite);

        //bleSocket->open(QIODevice::ReadWrite);

        connect(bleSocket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(bleSocketError(QBluetoothSocket::SocketError)));
        connect(bleSocket, SIGNAL(stateChanged(QBluetoothSocket::SocketState)), this, SLOT(bleSocketStateChanged()));
        connect(bleSocket, &QBluetoothSocket::connected, this, &MainWindow::bleSocketConnected);
        connect(bleSocket, &QBluetoothSocket::disconnected, this, &MainWindow::slot_bleDone);
        connect(bleSocket, &QBluetoothSocket::readyRead, this, &MainWindow::bleSocketRead);

        st = "Wait connect to device '" + bleDevNameAddr + "'  " + QString::number(tmr_ble_wait / 1000, 10) + " sec...";
        pf = picInfo;

        /*tmr_ble = 0;
        tmr_ble = startTimer(tmr_ble_wait);// wait connection 5 sec.
        if (tmr_ble <= 0) {
            MyError |= 2;//start_timer error
            throw TheError(MyError);
        }*/
        //
        if (localDevice->pairingStatus(infoDev.address()) != QBluetoothLocalDevice::Paired) {
            localDevice->requestPairing(infoDev.address(), QBluetoothLocalDevice::Paired);
        }
        //
    } else {
        st = "Error create socket for ble connection with defice " + bleDevNameAddr;
    }

    toStatusLine(st, pf);
    LogSave(time2str() + st, Qt::blue);
}
//-----------------------------------------------------------------------
void MainWindow::bleSocketStateChanged()
{
    QString st = "";
    switch ((int)bleSocket->state()) {
        case QAbstractSocket::UnconnectedState:
            st = "unconnected";
        break;
        case QAbstractSocket::HostLookupState:
            st = "host lookup";
        break;
        case QAbstractSocket::ConnectingState:
            st = "connecting";
        break;
        case QAbstractSocket::ConnectedState:
            st = "connected";
        break;
        case QAbstractSocket::BoundState:
            st = "bound";
        break;
        case QAbstractSocket::ListeningState:
            st = "listening";
        break;
        case QAbstractSocket::ClosingState:
            st = "closing";
        break;
            //default : st = "unknown state";
    }
    if (st.length()) LogSave(time2str() + "BLE socket state change to : " + st, Qt::blue);
}
//-----------------------------------------------------------------------
void MainWindow::bleSocketError(QBluetoothSocket::SocketError bleErr)
{
    if (tmr_ble > 0) {
        killTimer(tmr_ble);
        tmr_ble = 0;
        //disconnect(this, &MainWindow::sig_bleTimeOut, this, &MainWindow::slot_bleTimeOut);
    }

    QString qs = "BLE socket ERROR (" + QString::number(static_cast<int>(bleErr), 10) + ") : " + bleSocket->errorString();

    LogSave(time2str() + qs, Qt::blue);
    toStatusLine(qs, picDis);

    emit sig_bleDone();

    QMessageBox::critical(this, "ERROR", qs, QMessageBox::StandardButton::Ok);
}
//-----------------------------------------------------------------------
void MainWindow::bleSocketConnected()
{
    ble_connect = true;

    toStatusLine("Connection to device " + bleSocket->peerName(), picCon);
}
//-----------------------------------------------------------------------
void MainWindow::bleSocketRead()
{

    if (!bleSocket || !ble_connect) return;

    blePack += bleSocket->readAll();

    int ix = chkDone(&blePack);
    if (ix != -1) {
        QString line;
        int pos = 0;
        line.append(blePack.mid(0, ix));
        QString lin(line);
        if (lin.indexOf(jBegin, 0) == 0) {
            jsMode = true;
            if ((pos = line.indexOf(jEnd, 0)) != -1) line.remove(pos, jEnd.length());
        } else {
            if ((pos = line.indexOf(cr_lf, 0)) != -1) line.remove(pos, cr_lf.length());
            jsMode = false;
        }

        LogSave(line, Qt::blue);
        if (blePack.length() >= (ix + 1)) blePack.remove(0, ix);
                                     else blePack.clear();
        //
        if (line.length() >= 50) {
//            if (!jsMode) mkDataFromStr(line);
//                    else mkDataFromJson(line);
        }
        //
    }
}
//-----------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------


