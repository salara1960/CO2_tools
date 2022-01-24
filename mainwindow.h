#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <inttypes.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __linux__
    #include <arpa/inet.h>
    #include <endian.h>
    #include <termios.h>
    #include <unistd.h>
#else
    #include <windows.h>
#endif

#include <QSystemTrayIcon>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QByteArray>
#include <QFile>
#include <QDateTime>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>
#include <QPainter>
#include <QCheckBox>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFileDialog>
#include <QIODevice>

#include "defs.h"


#ifdef SET_BLUETOOTH

    #include <QBluetoothLocalDevice>
    #include <QBluetoothServiceDiscoveryAgent>
    #include <QtBluetooth/QBluetoothSocket>
    #include <QtBluetooth/qbluetoothaddress.h>
    #include <QtBluetooth/qbluetoothserviceinfo.h>

    //#include <QtBluetooth/qbluetoothserver.h>
    //#include <QBluetoothRfcommSocket>

#endif

//********************************************************************************

typedef struct {
    int day;
    int hour;
    int min;
    int sec;
} dev_time_t;

//********************************************************************************

const QString title = "CO2_tools";


const QString con_pic     = "png/conn.png";
const QString dis_pic     = "png/dis.png";
const QString salara_pic  = "png/niichavo.png";
const QString close_pic   = "png/close.png";
const QString ucon_pic    = "png/uCon.png";
const QString udis_pic    = "png/uDis.png";
const QString timeout_pic = "png/wait.png";
const QString info_pic    = "png/info1.png";
const QString war_pic     = "png/warning.png";
const QString err_pic     = "png/error.png";
const QString ok_pic      = "png/ok.png";
const QString humi_pic    = "png/speed.png";
const QString temp_pic    = "png/temp.png";
const QString main_pic    = "png/co2.png";
const QString ini_pic     = "png/settings.png";
const QString pdf_pic     = "png/pdf.png";
const QString stop_pic    = "png/Stop.png";
const QString devErr_pic  = "png/devErr.png";
const QString devOk_pic   = "png/devOk.png";

const QString tTip = "QToolTip { color: blue; background-color: white; border: 3px solid #676767; }";
extern qint32 serSpeed;

//********************************************************************************

namespace Ui {
class MainWindow;
}

class SettingsDialog;
class Widget;

//********************************************************************************

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    class TheError {
        public :
            int code;
            TheError(int);
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int chkDone(QByteArray *buf);
    void toStatusLine(QString, int);

protected:
    virtual void timerEvent(QTimerEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *) override;

public slots:

    int initSerial();
    void deinitSerial();
    void LogSave(const QString &);
    void About();
    unsigned char myhextobin(const char *);
    void hexTobin(const char *, QByteArray *);
    void clrLog();
    int writes(const char *data, int len);
    //
    uint32_t get10ms();
    uint32_t get_tmr(uint32_t);
    int check_tmr(uint32_t);

private slots:
    void ReadData();
    void slotError(QSerialPort::SerialPortError);
    void on_answer();
    void on_connect();
    void on_disconnect();
    void mkDataFromStr(QString);
    void slotWrite(QByteArray &);
    void grafic(int);

#ifdef SET_BLUETOOTH
    void beginBle();
    void bleDiscoverFinished();
    void bleGo();
    void bleSocketError(QBluetoothSocket::SocketError);
    void bleSocketConnected();
    void bleSocketRead();
    void slot_bleTimeOut();
    void slot_bleDone();
    //
    void bleAddDevice(const QBluetoothDeviceInfo &);
    void bleScanError(QBluetoothDeviceDiscoveryAgent::Error);
    void bleSocketStateChanged();
#endif

signals:
    void sigWrite(QByteArray &);
    void sigAbout();
    void sigConn();
    void sigDisc();
    void sig_on_answer();
    void sig_grafic(int);

#ifdef SET_BLUETOOTH
    void sig_bleTimeOut();
    void sig_bleDone();
    void sig_bleGo();
#endif

private:
    Ui::MainWindow *ui;
    int tmr_sec, MyError;
    QSerialPort *sdev;
    QByteArray rxData;
    QString sdevName;
    bool first, con;
    dev_time_t dev_time;
    uint32_t devTime;
    bool jsMode = false;
    SettingsDialog *conf = nullptr;
    uint32_t ms10;
    uint8_t dbg = 1;
    uint8_t devErr;
    int ackLen, ibuff_len = 0;
    char buff[BUF_SIZE + 8] = {0};
    char ibuff[BUF_SIZE + 8] = {0};
    uint8_t to_dev_data[TO_DEV_SIZE];
    QByteArray fileName;
    QString chap;
    const QString devErrMask = "devError:";
    const QByteArray cr_lf = "\r\n";
    const QByteArray jBegin = "{";
    const QByteArray jEnd = ">\r\n";
    const QString formatTime = "dd.MM.yyyy hh:mm:ss.zzz";
    const QString _temp = "temp=";
    const QString _humi = "humi=";
    const QString _ppm = "ppm=";
    const QString _que = "que:";
    data_t one;
    Widget *graf = nullptr;

#ifdef SET_BLUETOOTH
    QBluetoothSocket *bleSocket = nullptr;

    const QString bleRemoteMarker = "JDY-";//25M";
    const QString bleRemoteMac = "?";//"11:89:C0:90:0B:2F";//"FC:58:FA:A9:6F:D8";//"20:07:19:0B:52:AB";//

    QBluetoothLocalDevice *localDevice = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    QString bleDevStr, bleAddrStr, bleDevNameAddr;
    QBluetoothAddress bleAddr;
    QByteArray blePack;
    bool ble_connect = false;
    QBluetoothDeviceInfo infoDev;
    int tmr_ble;
    const int tmr_ble_wait = 16000;
    bool bleFind = false;
#endif

};




#endif // MAINWINDOW_H
