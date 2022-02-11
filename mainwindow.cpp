#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "widget.h"
#include "mainwindow.h"
//******************************************************************************************************

qint32 serSpeed = 230400;

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

    graf = new itWidget(ui->graf);

    //----  Set timer with period to 10 msec ----
    tmr_sec = startTimer(10);// 10 msec.
    if (tmr_sec <= 0) {
        MyError |= 2;//start_timer error
        throw TheError(MyError);
    }
    //--------------------------------------------

    ui->actionTemp->setVisible(false);
    ui->actionHUMI->setVisible(false);

    connect(this, &MainWindow::sigWrite, this, &MainWindow::slotWrite);

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
            font: italic 12pt \"Sans Serif\";\
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

    ui->log->setTextColor(Qt::blue);

    connect(ui->actionPORT, &QAction::triggered,         conf, &SettingsDialog::sig_confShow);
#ifdef SET_BLUETOOTH
    QString atr = "{border: 2px 2px 2px 2px;\
            border-color: rgb(0, 0, 0);\
            font: italic 8pt \"Sans Serif\";\
            background-color: rgb(120, 120, 120);\
            color: rgb(255, 255, 255);}";
    ui->sql->setStyleSheet("QWidget" + atr);
    minWinSql = ui->sql->minimumSize();
    maxWinSql = ui->sql->maximumSize();

    connect(ui->actionBLE,  &QAction::triggered,         this, &MainWindow::beginBle);
    connect(this,           &MainWindow::sig_bleTimeOut, this, &MainWindow::slot_bleTimeOut);
    connect(this,           &MainWindow::sig_bleDone,    this, &MainWindow::slot_bleDone);
    connect(this,           &MainWindow::sig_bleGo,      this, &MainWindow::bleGo);
    connect(this,           &MainWindow::sig_bleStat,    this, &MainWindow::slot_bleStat);
    connect(this,           &MainWindow::sig_sqlGo,      this, &MainWindow::sqlGo);

    ui->actionBLE->setVisible(true);
    ui->actionBLE->setEnabled(true);
    ui->actionBLE->setChecked(false);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    connect(this, &MainWindow::sig_iniSql, this, &MainWindow::slot_iniSql);
    connect(this, &MainWindow::sig_recsSql, this, &MainWindow::TotalRecords);

    connect(this, &MainWindow::sig_getDevIndex, this, &MainWindow::getDevIndex);
    connect(this, &MainWindow::sig_delRecDB, this, &MainWindow::delRecDB);

    tmr_sql = startTimer(2000);// 2 msec.
    if (tmr_sql <= 0) {
        MyError |= 2;//start_timer error
        throw TheError(MyError);
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#else
    ui->actionBLE->setVisible(false);
#endif

}
//----------------------------------------------------------------------------------------
//   Реализация диструктор основного класса программы
//
MainWindow::~MainWindow()
{
#ifdef SET_BLUETOOTH
    if (db.isOpen()) db.close();

    if (bleSocket) {
        bleSocket->disconnect();
        if (bleSocket->isOpen()) bleSocket->close();
        delete bleSocket;
        bleSocket = nullptr;
    }

    if (tmr_sql > 0) {
        killTimer(tmr_sql);
        tmr_sql = 0;
    }
    if (tmr_ble > 0) {
        killTimer(tmr_ble);
        tmr_ble = 0;
    }
    if (tbl) {
        delete tbl;
        tbl = nullptr;
    }
#endif

    deinitSerial();

    if (graf) delete graf;

    if (conf) serSpeed = conf->settings().baudRate;

    if (conf) delete conf;

    if (tmr_sec > 0) killTimer(tmr_sec);

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
            //
            if (bleScan) {
                tmr_ble_scan++;
                emit sig_bleScan();
            } else {
                tmr_ble_scan = 0;
            }
        }
    }
    else if (tmr_rst == event->timerId()) {
        emit sig_rst_screen();
    }
#ifdef SET_BLUETOOTH
    else if (tmr_ble == event->timerId()) {
        if (tmr_ble > 0) emit sig_bleTimeOut();
    } else if (tmr_sql == event->timerId()) {
        emit sig_iniSql();
    }
#endif

}
//-------------------------------------------------------------------------------------
//   Обработчик события - "нажата клавиша Enter на клавиатуре"
//
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) emit sig_on_answer();
}
//--------------------------------------------------------------------------------
void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (event->size() == minimumSize()) {
        if (tbl) {
            ui->sql->resize(minWinSql);
            QSize z = ui->sql->size();
            tbl->resize(z);
        }
    } else {
        if (tbl) {
            ui->sql->resize(maxWinSql);
            QSize z = ui->sql->size();
            QSize sz = QSize(z.width() - 60, z.height() - 10);
            tbl->resize(sz);
        }
    }
}
//--------------------------------------------------------------------------------
QString MainWindow::time2str()
{
    return (QDateTime::currentDateTime().toString("dd.MM hh:mm:ss") + " | ");
}
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//         Error class
//
MainWindow::TheError::TheError(int err)//error class descriptor
{
    code = err;
}
//--------------------------------------------------------------------------------
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
        case picDevOk:
            pix = devOk_pic;
        break;
        case picDel:
            pix = del_pic;
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
    QString st = "\nСервисная программа для устройства '";
    st.append(title);
    st += "' -\nSTM32F411 + MQ135 + SI7021 + GC9A01";
#ifdef SET_BLUETOOTH
    st.append(" + JDY-25M");
#endif

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
                     .arg(sdevName,
                          conf->settings().stringBaudRate,
                          conf->settings().stringDataBits,
                          conf->settings().stringParity.at(0),
                          conf->settings().stringStopBits,
                          conf->settings().stringFlowControl), picCon);
        con = true;
        ui->actionCONNECT->setEnabled(false);
        ui->actionPORT->setEnabled(false);
        ui->actionDISCONNECT->setEnabled(true);
        ui->actionCLEAR->setEnabled(true);

        ui->cmd->setEnabled(true);
        ui->crlfBox->setEnabled(true);
        ui->log->setEnabled(true);


        ui->actionPORT->setToolTip(tr("Параметры последовательного порта:\n%1 %2 %3%4%5")
                                   .arg(sdevName,
                                   conf->settings().stringBaudRate,
                                   conf->settings().stringDataBits,
                                   conf->settings().stringParity.at(0),
                                   conf->settings().stringStopBits));

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
//        Слот для обработки события 'restart' устройства
//
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
//    через последовательный интерфейс UART
//
void MainWindow::slotWrite(QByteArray & mas)
{
    if (sdev && con) {
        toStatusLine("", picClr);

        QString m(mas);
        sdev->write(m.toLocal8Bit());

        if (m.indexOf("restart") != -1) {
#ifdef SET_BLUETOOTH
            ui->device->clear();
            ui->lab_device->clear();
            ui->val_humi->clear();
            ui->val_que->clear();
            ui->val_temp->clear();
#endif
            tmr_rst = startTimer(3600);// 4 sec.
            if (tmr_rst <= 0) {
                MyError |= 2;//start_timer error
                throw TheError(MyError);
            }
        }
#ifdef SET_BLUETOOTH
        else if ((m.indexOf("at+") == 0) || (m.indexOf("AT+") == 0)) {
            m = m.toUpper();
            if ((m.indexOf("AT+RESET") == 0) || (m.indexOf("AT+RST") == 0)) {
                if (ble_connect) {
                    ui->lab_device->clear();
                    ui->device->clear();
                    emit sig_bleDone();
                }
            }
        }
#endif
    }
}
//-----------------------------------------------------------------------------------
//          Слот для обработки события 'отправить данные на устроцство'
//
void MainWindow::on_answer()
{
    QByteArray tmp = ui->cmd->text().toLocal8Bit();

    if (ui->crlfBox->checkState() == Qt::Checked) tmp.append(cr_lf);

    if (tmp.length() >= 2) emit sigWrite(tmp);
}
//------------------------------------------------------------------------------------
//                   HEX string to BIN byte function
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
//-------------------------------------------------------------------------------------
//         Функция преобразует hex-строку в бинарный массив
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
//------------------------------------------------------------------------------------
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
//      Функция выполняет парсер данных, полученных от устройства
//
void MainWindow::mkDataFromStr(QString str)
{

    if (!str.length()) return;

    int pos = 0, pend = 0, end = str.length();
    float val = 0.0;
    QString tmp;
    bool ok = false;

#ifdef SET_BLUETOOTH
    bool yes = false;
    for (int i = 0; i < MAX_BLE_STAT; i++) {
        pos = str.indexOf(ble_stat[i], 0);
        if (pos != -1) {
            switch (i) {
                case idConnect:
                    ble_connect = true;
                    yes = true;
                break;
                case idDisconnect:
                    ble_connect = false;
                    con_number = -1;
                    yes = true;
                break;
            }
        }
        if (yes) break;
    }
    if (yes) {
        emit sig_bleStat(ble_connect);
        return;
    }
#endif

    pos = str.indexOf(" | [que:", 0); //0.00:09:20 | [que:1] MQ135: adc=903 ppm=320, SI7021: temp=23.59 humi=24.40
    if (pos != -1) {
        tmp = str.mid(0, pos);
        QByteArray bf(tmp.toLocal8Bit());
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
        ui->val_temp->setText(tr(" %1 C").arg(one.temp));
        ui->val_temp->setToolTip(tr("Температура (град.Цельсия)"));

        ui->val_humi->setText(tr(" %1 %").arg(one.humi));
        ui->val_humi->setToolTip(tr("Влажность (проценты)"));

        ui->val_que->setText(" " + QString::number(one.que, 10));
        ui->val_que->setToolTip(tr("Количество сообщений в\nбуфере fifo устройства"));

        tmp = tr("Queue: %1  Sensors: %2%3 C  %4%5 %  %6%7  CO2=%8 %")
                .arg(one.que)
                .arg(_temp).arg(one.temp)
                .arg(_humi).arg(one.humi)
                .arg(_ppm).arg(one.ppm).arg(one.co2);
        toStatusLine(tmp, picInfo);

        emit sig_grafic(msg_evt);
    }
}
//--------------------------------------------------------------------------------
//             Слот приема данных от устройства
//
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
//---------------------------------------------------------------------------------
//         Слот обработки ошибки при приеме данных от устройства
//
void MainWindow::slotError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        MyError |= 8;//error reading from serial port
        throw TheError(MyError);
    }
}
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//            Функции для формирования временных интервалов
//
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
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
//        функция выдачи данных на устройство (в последовательный порт UART)
//
int MainWindow::writes(const char *data, int len)
{
int ret = 0;

    if (sdev) {
        if (sdev->isOpen()) ret = static_cast<int>(sdev->write(data, len));
    }

    return ret;
}
//-----------------------------------------------------------------------
//      Слот для отправки данных эмулятору графического дисплея
//
void MainWindow::grafic(int cd)
{
    if (graf) emit graf->sig_refresh((void *)&one, cd);
}
//-----------------------------------------------------------------------

#ifdef SET_BLUETOOTH

//-----------------------------------------------------------------------
//----------------------  Bluetooth functions  --------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//     Функция формирует строку из данных одной записи базы данных
//
QString MainWindow::rec2str(QSqlQuery *q)
{
    QString tp, tmp = " num:" + q->value(iNum).toString() + sep; //number
    tmp += " name:'" + q->value(iName).toString() + "'" + sep; //s_name
    tmp += " addr:" + QString::number(q->value(iAddr).toULongLong(), 16) + sep; //s_addr
    tmp += " rssi:" + q->value(iRssi).toString() + sep; //s_rssi
    bool good = false;
    QDateTime td = QDateTime::fromTime_t(q->value(iTime).toUInt(&good));
    if (good) {
        tp = " epoch:'" + td.toString("dd.MM.yyyy hh:mm:ss") + "'";
    }
    tmp += tp + "\n";

    return tmp;
}
//-----------------------------------------------------------------------
//    Функция создает таблицу 'ble' в базе данных
//
void MainWindow::slot_iniSql()
{
    if (tmr_sql > 0) {
        killTimer(tmr_sql);
        tmr_sql = 0;
    }
    mk_table_db3 = "CREATE TABLE IF NOT EXISTS `" + db_tabl + "` (number INTEGER primary key autoincrement, " +
                   "s_name TEXT, s_addr INTEGER, s_rssi INTEGER, s_epoch TIMESTAMP);";
    tbl_stat = "Sqlite3 data base '" + db_name + "' : ";
    Qt::GlobalColor color = Qt::darkGreen;
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(db_name);
    openDB = db.open();
    if (openDB) {
        QSqlQuery query(db);
        if (query.exec(mk_table_db3)) tbl_stat += "Table '"+ db_tabl + "' present !";
                                else  tbl_stat += "Warning : " + query.lastError().text();
        LogSave(time2str() + tbl_stat, color);

        rec_count = TotalRecords();

        tbl_stat = "Table '" + db_tabl + "' contain " + QString::number(rec_count, 10) + " records";
    } else {
        tbl_stat = "DB '" + db_name + "' not open !!!";
        color = Qt::red;
    }
    LogSave(time2str() + tbl_stat, color);

    if (rec_count > 0) {
        getAllDB();//readAllDB();
        mkSqlTable();
    }
}
//-----------------------------------------------------------------------
//   Функция возвращает количество записей в таблице
//           'ble' у базы данных 'ble_dev'
//
int MainWindow::TotalRecords()
{
int ret = 0;

    if (openDB) {
        QSqlQuery query(db);
        if (query.exec("select count(*) from `" + db_tabl + "`;")) {
            bool gd = false;
            query.first();
            ret = query.value(0).toInt(&gd);
            if (!gd) ret = 0;
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
int MainWindow::findByAddr(quint64 addr)
{
int ret = -1;

    if (openDB) {
        QString req = tr("SELECT * FROM `ble` WHERE s_addr=%1 order by number desc limit %2;").arg(addr).arg(rec_count);
        QString tmp = "";
        QSqlQuery query(db);
        if (query.exec(req)) {
            while (query.next()) {
                bool good = false;
                quint64 adr = query.value(iAddr).toULongLong(&good);
                if (good) {
                    if (adr == addr) {
                        ret = query.value(iNum).toInt();
                        tmp = " Num:" + QString::number(ret, 10) + sep +
                              " Addr:" + QString::number(addr, 16) +
                              " Name:'" + query.value(iName).toString() + "' <- device already PRESENT";
                        break;
                    }
                }
            }
        }
        if (tmp.length()) {
            toStatusLine(tmp, picInfo);
            LogSave(time2str() + tmp, Qt::darkGreen);
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
void MainWindow::readAllDB()
{
    if (!rec_count || !openDB) return;

    QString req = tr("SELECT * FROM `ble` order by number desc limit %1;").arg(rec_count);
    QString tmp = "";
    int cnt = 0;
    QSqlQuery query(db);
    if (query.exec(req)) {
        while (query.next()) {
            cnt++;
            tmp += "[" + QString::number(cnt, 10) + "]"+ sep + rec2str(&query);
        }
        tmp += "Total records: " + QString::number(cnt, 10);
        LogSave(time2str() + tmp, Qt::darkGreen);
    } else {
        LogSave(time2str() + "Error reading from DB -> " + query.lastError().text(), Qt::red);
    }

}
//--------------------------------------------------------------------------------
int MainWindow::getAllDB()
{
    if (!rec_count || !openDB) return 0;

    rec_list.clear();
    QString req = tr("SELECT * FROM `ble` order by number desc limit %1;").arg(rec_count);
    get_recs_t rc;
    QSqlQuery query(db);
    if (query.exec(req)) {
        while (query.next()) {
            memset(&rc, 0, sizeof(get_recs_t));
            rc.number = query.value(iNum).toInt();
            QByteArray mas(query.value(iName).toString().trimmed().toLocal8Bit());
            int len = mas.length();
            if (len > MAX_REC_LEN - 1) len = MAX_REC_LEN - 1;
            memcpy(rc.name, mas.data(), len);
            rc.addr = query.value(iAddr).toULongLong();
            rc.rssi = query.value(iRssi).toInt();
            rc.epoth = query.value(iTime).toUInt();
            rec_list << rc;
        }
    }

    return rec_list.length();
}
//--------------------------------------------------------------------------------
void MainWindow::delRecDB(const int nr)
{
int cnt = rec_count;

    if (openDB) {
        QSqlQuery query(db);
        QString tp = tr("DELETE FROM `ble` WHERE number=%1;").arg(nr);
        if (query.exec(tp)) {
            //
            rec_count = TotalRecords();
            tp = "From DB: Record #" + QString::number(nr, 10) + " delete ";
            if ((rec_count + 1) == cnt) {//record delete OK
                tp += "OK.";
            } else {
                tp += "Error.";
            }
            tp += " Total records:" + QString::number(rec_count, 10);
            LogSave(time2str() + tp, Qt::darkGreen);

            getAllDB();
            mkSqlTable();
        }
    }
}
//-----------------------------------------------------------------------
bool MainWindow::insToDB(get_recs_t *rec)
{
bool ret = false;
int cnt = rec_count;

    if (openDB) {
        QString tmp = tr("INSERT INTO `ble` (s_name, s_addr, s_rssi, s_epoch) VALUES ('%1', %2, %3, %4);")
                     .arg(rec->name).arg(rec->addr).arg(rec->rssi).arg(rec->epoth);
        QString tp = " Insert to table '" + db_tabl + "' device '" + rec->name + "' with index #";
        QSqlQuery query(db);
        if (query.exec(tmp)) {
            if (query.exec("select * from `" + db_tabl + "` order by number desc limit 1;")) {
                while (query.next()) {
                    tp += query.value(iNum).toString();;
                    break;
                }
            }
            ret = true;
        } else {
            tp += " Error:" + query.lastError().text();
        }

        rec_count = TotalRecords();
        if (rec_count == (cnt + 1)) {
            tp += " OK.";
        } else {
            tp += " Error.";
        }
        tp += " | Total records: " + QString::number(rec_count, 10);
        LogSave(time2str() + tp, Qt::darkGreen);
    }

    return ret;
}
//-----------------------------------------------------------------------
void MainWindow::mkRecDB(const QBluetoothDeviceInfo *info, get_recs_t *rc)
{
    rc->number = 0;

    QByteArray mas(info->name().trimmed().toLocal8Bit());
    strncpy(rc->name, mas.data(), MAX_REC_LEN - 1);

    rc->addr = info->address().toUInt64();

    rc->rssi = info->rssi();

    rc->epoth = QDateTime::currentDateTimeUtc().toTime_t();
}
//-----------------------------------------------------------------------
void MainWindow::slot_bleStat(bool stat)
{
    if (!stat) {
        ui->device->clear();
        ui->lab_device->clear();
        ui->lab_device->setToolTip("");
        toStatusLine("Disconnected with device '" + bleDevNameAddr, picInfo);

        emit sig_bleDone();
    } else {
        ui->lab_device->setPixmap(QPixmap(net32_pic));
        if ((index >= 0) && (rec_list.size())) {
            ui->device->setText(bleDevNameAddr);
            ui->lab_device->setToolTip("Connected to " + bleDevNameAddr);
            toStatusLine("Connected with device '" + bleDevNameAddr, picInfo);
        }
    }
}
//-----------------------------------------------------------------------
void MainWindow::slot_bleScan()
{
    if (bleScan) bleScan->setValue(tmr_ble_scan);
}
//-----------------------------------------------------------------------
void MainWindow::beginBle()
{

    if (ble_proc) {
        ui->actionBLE->setChecked(true);
        return;
    }
    ble_proc = true;

    ui->actionBLE->setChecked(true);
    ui->log->setEnabled(true);

    list.clear();
    index = -1;

    bleFind = false;
    bleDevStr.clear();
    bleAddrStr.clear();
    bleDevNameAddr.clear();

    if (localDevice) {
        delete localDevice;
        localDevice = nullptr;
    }

    localDevice = new QBluetoothLocalDevice(this);
    if (localDevice->isValid()) {
        localDevice->powerOn();
        localDevice->setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        //localDevice->setHostMode(QBluetoothLocalDevice::HostConnectable);
        LogSave(time2str() + "Local bluetooth device: " + localDevice->name() +
                " (" + localDevice->address().toString().trimmed() + ")", Qt::blue);

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

#ifdef SET_BLE_DEVICE
            discoveryAgent->setLowEnergyDiscoveryTimeout(tmr_ble_wait);
            connect(discoveryAgent,
                    static_cast<void (QBluetoothDeviceDiscoveryAgent::*)(QBluetoothDeviceDiscoveryAgent::Error)>(&QBluetoothDeviceDiscoveryAgent::error),
                    this,
                    &MainWindow::bleScanError);
            discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
#else
            discoveryAgent->start();
            tmr_ble = startTimer(tmr_ble_wait << 1);// wait connection 10 sec.
            if (tmr_ble <= 0) {
                MyError |= 2;//start_timer error
                throw TheError(MyError);
            }
#endif
            //
            if (bleScan) {
                delete bleScan;
                bleScan = nullptr;
            }
            bleScan = new QProgressBar(this);
            if (bleScan) {
                tmr_ble_scan = 0;
                QRect rr = this->geometry();
                int w = 350, h = 26;//50;//56;
                bleScan->setRange(0, tmr_ble_wait / 1000);
                bleScan->setValue(0);
                bleScan->resize(w, h);
                bleScan->move(rr.width()/2 - (w >> 1), rr.height()/2 - (h >> 1) + 3);
                bleScan->setStyleSheet("*::chunk {background-color: blue; border-radius: 4;}");
                bleScan->setAlignment(Qt::AlignCenter);
                connect(this, &MainWindow::sig_bleScan, this, &MainWindow::slot_bleScan);
                bleScan->show();
            }
            //
        } else {
            LogSave(time2str() + "Error create QBluetoothDeviceDiscoveryAgent !", Qt::blue);
        }
    } else {
        LogSave(time2str() + "Bluetooth is not available on this device", Qt::blue);
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
#ifndef SET_BLE_DEVICE
    if (tmr_ble) {
        killTimer(tmr_ble);
        tmr_ble = 0;
    }
#endif
    /*
    QList<QBluetoothDeviceInfo>ls;
    ls.clear();
    */
    st.clear();
    list.clear();
    list = discoveryAgent->discoveredDevices();
    discoveryAgent->disconnect();
    discoveryAgent->stop();
    if (!list.isEmpty()) {
        for (int i = 0; i < list.size(); i++) {
            //if (list.at(i).name().indexOf(bleRemoteMarker) != -1) {
                st += "Device: " + list.at(i).name().trimmed() +
                     " (" + list.at(i).address().toString().trimmed() +
                     ") rssi: " + QString::number(list.at(i).rssi(), 10) + "\n";
                //ls.append(list.at(i));
            //}
        }
    } else {
        st = "No devices found";
    }
    if (st.length()) {
        LogSave(time2str() + st, Qt::blue);
    }

    if (!list.isEmpty()) {
        //list.clear();
        //list = ls;
        index = -1;
        emit sig_sqlGo();
    } else {
        emit sig_bleDone();
    }
}
//-----------------------------------------------------------------------
void MainWindow::slot_bleDone()
{

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

    ui->actionBLE->setChecked(false);

    ble_proc = false;
}
//-----------------------------------------------------------------------
void MainWindow::slot_bleTimeOut()
{
    if (tmr_ble > 0) {
        killTimer(tmr_ble);
        tmr_ble = 0;
    }

    if (ble_connect) return;

    QString st = "No selected bluetooth device !";
    if (index >= 0) st = "Timeout connect to device " + bleDevNameAddr;
    toStatusLine(st, picWar);
    LogSave(time2str() + st, Qt::blue);

    emit sig_bleDone();

    QMessageBox::critical(this, "Error", st, QMessageBox::StandardButton::Ok);
}
//-----------------------------------------------------------------------
//     Функция запускает процедуру "установить соединение"
//           с выбранным устройством bluetooth
//
void MainWindow::getDevIndex(int ix)
{
    if (ix < 0) return;

    int i = -1;
    while (++i < rec_list.length()) {
        if (ix == rec_list.at(i).number) {
            index = i;
        }
    }

    if (index >= 0) {
        bleFind = true;
        infoDev = rec_list.at(index);
        bleAddr = static_cast<QBluetoothAddress>(infoDev.addr);// (QBluetoothAddress)infoDev.addr;
        bleDevNameAddr = QString(infoDev.name) + " (" + QString::number(infoDev.addr, 16) + ")";
        con_number = ix;

        emit sig_bleGo();
    } else {
        emit sig_bleDone();
    }
}
//-----------------------------------------------------------------------
//       Функция формирует таблицу устройств из данных в списке
//
void MainWindow::mkSqlTable()
{
    if (!rec_list.length()) return;

    if (tbl) {
        delete tbl;
        tbl = nullptr;
    }

    if (!tbl) {
        QRect rr = ui->sql->geometry();
        QPalette pal = QPalette(ui->sql->palette());
        QFont font = QFont(ui->sql->font());

        tbl = new QTableWidget(ui->sql);// создаем саму таблицу
        if (tbl) {
            tbl->setFont(font);
            tbl->setPalette(pal);
            tbl->resize(rr.width(), rr.height());
            tbl->setRowCount(rec_list.length());//количество строк
            tbl->setColumnCount(iTime + 1); //5 - количество столбцов
            //
            int row, column;
            get_recs_t rc;
            QStringList lst;
            lst << tr("ID") << tr("NAME") << tr("ADDR") << tr("RSSI") << tr("TIME");
            for (column = 0; column < tbl->columnCount(); column++) {
                tbl->setHorizontalHeaderLabels(lst);
            }
            tbl->setSortingEnabled(true);
            tbl->resizeRowsToContents();
            for (row = 0; row < tbl->rowCount(); row++) {
                tbl->resizeColumnsToContents();
                rc = rec_list.at(row);
                lst.clear();
                lst << QString::number(rc.number, 10)
                    << QString(rc.name)
                    << QString::number(rc.addr, 16).toUpper()
                    << QString::number(rc.rssi)
                    << QDateTime::fromTime_t(rc.epoth).toString(dt_fmt);
                for (column = 0; column < tbl->columnCount(); column++) {
                    itItem *item = new itItem;
                    item->setText(lst.at(column));
                    item->setTextAlignment(Qt::AlignCenter);
                    item->setToolTip(tr("Для сортировки по столбцу\nкликните мышкой по заголовку столбца"));
                    tbl->setItem(row, column, item);
                }
                tbl->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            }
            ui->sql->minimumSize();
            tbl->minimumSize();
            //
            connect(tbl->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(colSort(int)));
            connect(tbl->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(tblContextMenu(int)));
            //
            tbl->show();
        }
    }
}
//-----------------------------------------------------------------------
//         Функция формирует контекстное меню для таблицы
//
void MainWindow::tblContextMenu(int row)//const QPoint& pos)
{
        QRect rt = tbl->geometry();

        int hig = 10;
        QPoint pos(rt.x() + (hig << 1), rt.y() + (hig * row));

        QMenu *menu = new QMenu(nullptr);
        if (menu) {
            selRow = row;
            QAction *selDevice = new QAction(QIcon(devOk_pic), "Select", tbl);
            QAction *delDevice = new QAction(QIcon(del_pic), "Delete", tbl);
            connect(selDevice, SIGNAL(triggered()), this, SLOT(slotSelRecord()));
            connect(delDevice, SIGNAL(triggered()), this, SLOT(slotDelRecord()));
            menu->addAction(selDevice);
            menu->addAction(delDevice);
            menu->popup(tbl->viewport()->mapToGlobal(pos));
            menu->show();
        }
}
//-----------------------------------------------------------------------
// Слот запускает процедуру соединения с выбранным из таблицы устройством
//
void MainWindow::slotSelRecord()
{
    int row = tbl->selectionModel()->currentIndex().row();
    if (row < 0) return;

    QString snum = tbl->item(row, iNum)->text();
    bool good = false;
    int num = snum.toInt(&good, 10);
    if (!good) num = -1;

    snum = QString::number(row + 1, 10) + " (num=" + QString::number(num, 10) + ")";
    toStatusLine("Selected record by index #" + snum, picDevOk);

#ifdef SET_DEBUG
    qDebug() << "Selected record by index #" + snum;
#endif

    if (!ble_connect) {
        emit sig_getDevIndex(num);
    } else {
        QMessageBox::warning(this, tr("Установить соединение"), tr("Соединение уже установлено c\n'") + ui->device->text() + "'", QMessageBox::Ok);
    }
}
//-----------------------------------------------------------------------
//   Слот запускает процедуру удаление выбранной записи
//            из таблицы и базы данных
//
void MainWindow::slotDelRecord()
{
    int row = tbl->selectionModel()->currentIndex().row();
    if (row < 0) return;

    QString snum = tbl->item(row, iNum)->text();
    bool good = false;
    int num = snum.toInt(&good, 10);
    if (!good) {
        toStatusLine(tr("Индекс записи ошибочный - операция невозможна"), picErr);
        return;
    }

    if (ble_connect) {
        if (con_number == num) {
            QMessageBox::warning(this,
                                 tr("Удаление записи"),
                                 tr("Удаление невозможною\nУстройство '") + ui->device->text() + "' занято !",
                                 QMessageBox::Ok);
            return;
        }
    }

    snum = QString::number(row + 1, 10) + " (num=" + QString::number(num, 10) + ")" + " con_number=" + QString::number(con_number, 10);

    if (QMessageBox::warning(this,
                             tr("Удаление записи"),
                             tr("Вы уверены, что хотите удалить эту запись?"),
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        toStatusLine("Cancel delete record by index #" + snum, picWar);

#ifdef SET_DEBUG
    qDebug() << "Cancel delete record by index #" + snum;
#endif

    } else {
        toStatusLine("Delete record by index #" + snum + " confirm OK.", picDel);

#ifdef SET_DEBUG
    qDebug() << "Delete record by index #" + snum + " confirm OK.";
#endif

        emit sig_delRecDB(num);// <- delete record

    }
}
//-----------------------------------------------------------------------
//      Функция выполняет соритровку данных по столбцу таблицы
//
void MainWindow::colSort(int c)
{
    static Qt::SortOrder dir = Qt::SortOrder::AscendingOrder;
    tbl->sortByColumn(c, dir);
    if (dir == Qt::SortOrder::AscendingOrder)
        dir = Qt::SortOrder::DescendingOrder;
    else
        dir = Qt::SortOrder::AscendingOrder;
}
//-----------------------------------------------------------------------
//      Функция заносит данные об устройствах bluetooth
//         из списка в базу данных sqlite3, а также
//            формирует таблицу из этих данных
//
void MainWindow::sqlGo()
{
    //------------   Stop progress bar   ------------
    if (bleScan) {
        delete bleScan;
        bleScan = nullptr;
    }
    ui->actionBLE->setChecked(false);
    ble_proc = false;
    //-----------------------------------------------
    if (list.size()) {
        if (openDB) {
            rec_list.clear();
            get_recs_t rc;
            int ix = -1;
            while (++ix < list.size()) {
                if (findByAddr(list.at(ix).address().toUInt64()) == -1) {//запись с таким mac-адресом отсутствует в базе
                    mkRecDB(&list.at(ix), &rc);
                    if (insToDB(&rc)) {
                        rec_list << rc;
                    }
                }
            }
            //------------ Create table with bluetooth devices ---------
            if (rec_list.length() > 0) {
                getAllDB();
                mkSqlTable();
            }
            //----------------------------------------------------------
        }
    }
}
//-----------------------------------------------------------------------
void MainWindow::bleGo()
{
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
    //
    bleSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    if (bleSocket) {
        blePack.clear();
        //
        // "{FDA50693A4E24FB1AFCFC6EB07647825}" {00000000-0000-0000-0000-000000000000}
        // Service UUID: FFE0  (Service UUID）
        // Service UUID: FFE1  (Used for transparent transmission)
        // Service UUID: FFE2  (Used for transparent transmission)
        // Service UUID: FFE3  (MESH data receiving and sending, MESH instruction receiving and sending, APP control IO, parameter configuration)
        //
        //QBluetoothUuid uuid(tr("FFE0--0000-1000-8000-00805F9B34FB"));//"{0000110E-0000-1000-8000-00805F9B34FB}"; // UUID of the 'A/V Remote Control' service
        //QBluetoothUuid uuid(tr("18010000-0000-0000-0000-000000000000"));// QBluetoothUuid::ServiceClassUuid::GenericAttribute=0x1801
        //QBluetoothUuid uuid(tr("18000000-0000-0000-0000-000000000000"));// QBluetoothUuid::ServiceClassUuid::GenericAccess=0x1800
        //QBluetoothUuid uuid(tr("11010000-0000-0000-0000-000000000000"));// QBluetoothUuid::SerialPort = 4353
        /*
        socket = new QBluetoothSocket(QBluetoothSocket::RfcommSocket, this);
        socket->connectToService(QBluetoothAddress(selectedDevice.address()), QBluetoothUuid(QBluetoothUuid::SerialPort));

        connect(socket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(socketError(QBluetoothSocket::SocketError)));
        connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(socketRead()));
        */
        QBluetoothUuid uuid(QBluetoothUuid::SerialPort);
        bleSocket->connectToService(bleAddr, uuid, QIODevice::ReadWrite);
        //bleSocket->connectToService(QBluetoothAddress(infoDev.address()), QBluetoothUuid(QBluetoothUuid::SerialPort), QIODevice::ReadWrite);

        connect(bleSocket, SIGNAL(error(QBluetoothSocket::SocketError)), this, SLOT(bleSocketError(QBluetoothSocket::SocketError)));
        connect(bleSocket, &QBluetoothSocket::stateChanged, this, &MainWindow::bleSocketStateChanged);
        connect(bleSocket, &QBluetoothSocket::connected, this, &MainWindow::bleSocketConnected);
        connect(bleSocket, &QBluetoothSocket::disconnected, this, &MainWindow::slot_bleDone);
        connect(bleSocket, &QBluetoothSocket::readyRead, this, &MainWindow::bleSocketRead);

        st = "Wait connect to device '" + bleDevNameAddr + "'  " + QString::number(tmr_ble_wait / 1000, 10) + " sec...";
        pf = picInfo;

        tmr_ble = startTimer(tmr_ble_wait);// wait connection 10 sec.
        if (tmr_ble <= 0) {
            MyError |= 2;//start_timer error
            throw TheError(MyError);
        }
        /*
        if (localDevice->pairingStatus(infoDev.address()) != QBluetoothLocalDevice::Paired) {
            localDevice->requestPairing(infoDev.address(), QBluetoothLocalDevice::Paired);
        }
        */
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
    if (tmr_ble > 0) {
        killTimer(tmr_ble);
        tmr_ble = 0;
        //disconnect(this, &MainWindow::sig_bleTimeOut, this, &MainWindow::slot_bleTimeOut);
    }

    ble_connect = true;

    toStatusLine("Connection to device " + bleSocket->peerName(), picCon);
}
//-----------------------------------------------------------------------
void MainWindow::bleSocketRead()
{

//    if (!bleSocket || !ble_connect) return;

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

        LogSave("ble: " + line, Qt::green);
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


