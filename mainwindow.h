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
#include <QProgressBar>
#include <QString>

#include "defs.h"

#ifdef SET_BLUETOOTH

    #include <QBluetoothLocalDevice>
    #include <QBluetoothUuid>
    #include <QBluetoothServiceDiscoveryAgent>
    #include <QtBluetooth/QBluetoothSocket>
    #include <QtBluetooth/qbluetoothaddress.h>
    #include <QtBluetooth/qbluetoothserviceinfo.h>

    #include <QtBluetooth/qbluetoothserver.h>

    #include <QtSql/QtSql>
    #include <QtSql/QSqlDatabase>
    #include <QtSql/QSqlQuery>
    #include <QtSql/QSqlError>

    #include <QtWidgets/QTableWidget>
    #include <QtWidgets/QTableWidgetItem>
    #include <QTableView>
    #include <QHeaderView>

enum {
    idConnect = 0,
    idDisconnect
};

enum {
    iNum = 0,
    iName,
    iAddr,
    iRssi,
    iTime
};

#define MAX_REC_LEN 64

typedef struct {
    int number;
    char name[MAX_REC_LEN]; //s_name TEXT,
    quint64 addr;           //s_addr INTEGER,
    qint16 rssi;            //s_rssi INTEGER,
    uint epoth;             //s_epoch TIMESTAMP
} get_recs_t;

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

const QString del_pic     = "png/delete.png";
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
const QString net_pic     = "png/network.png";
const QString net32_pic   = "png/ibcon32.png";
const QString net64_pic   = "png/ibcon64.png";
const QString tTip = "QToolTip { color: blue; background-color: white; border: 3px solid #676767; }";

extern qint32 serSpeed;

const QString dt_fmt = "dd.MM.yy hh:mm:ss";

//********************************************************************************

namespace Ui {
class MainWindow;
}

class SettingsDialog;
class itWidget;
//class bleDialog;

//********************************************************************************
class itItem: public QTableWidgetItem
{
    const QString fmt = dt_fmt;//"dd.MM.yyyy hh:mm:ss";

public:

     bool operator< (const QTableWidgetItem &other) const
     {
        if (this->text().contains(" ")) {
            QDateTime it = QDateTime::fromString(this->text(), fmt);
            QDateTime cur = QDateTime::fromString(other.text(), fmt);
            return (it.toTime_t() < cur.toTime_t());
        } else {
            if (this->text().contains("."))
                return (this->text().toFloat(nullptr) < other.text().toFloat(nullptr));
            else
                return (this->text().toInt() < other.text().toInt());
        }
     }
};
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
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:

    int initSerial();
    void deinitSerial();
    void LogSave(const QString &, Qt::GlobalColor);
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
    void getDevIndex(int);
    void rst_screen();
#ifdef SET_BLUETOOTH
    QString time2str();
    void beginBle();
    void bleDiscoverFinished();
    void bleGo();
    void bleSocketError(QBluetoothSocket::SocketError);
    void bleSocketConnected();
    void bleSocketRead();
    void slot_bleTimeOut();
    void slot_bleDone();
    //
    void bleScanError(QBluetoothDeviceDiscoveryAgent::Error);
    void bleSocketStateChanged();
    void slot_bleScan();
    void slot_bleStat(bool);
    void slot_iniSql();
    QString rec2str(QSqlQuery *);
    int TotalRecords();
    bool insToDB(get_recs_t *);
    int findByAddr(quint64);
    void readAllDB();
    void delRecDB(const int nr);
    void mkRecDB(const QBluetoothDeviceInfo *, get_recs_t *);
    void sqlGo();
    void mkSqlTable();
    int getAllDB();
    void tblContextMenu(int);
    void slotSelRecord();
    void slotDelRecord();
    void colSort(int);
#endif

signals:
    void sigWrite(QByteArray &);
    void sigAbout();
    void sigConn();
    void sigDisc();
    void sig_on_answer();
    void sig_grafic(int);
    void sig_rst_screen();
#ifdef SET_BLUETOOTH
    void sig_bleTimeOut();
    void sig_bleDone();
    void sig_bleGo();
    void sig_bleScan();
    void sig_bleStat(bool);
    void sig_iniSql();
    void sig_recsSql();
    void sig_sqlGo();
    void sig_getDevIndex(int);
    void sig_delRecDB(const int nr);
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
    itWidget *graf = nullptr;

#ifdef SET_BLUETOOTH
    QBluetoothSocket *bleSocket = nullptr;

    const QString bleRemoteMarker = "JDY-25M";
    const QString bleRemoteMac = "???";//"11:89:C0:90:0B:2F";//"FC:58:FA:A9:6F:D8";//"20:07:19:0B:52:AB";//
    const QString db_name = "ble_dev.db3";
    const QString db_tabl = "ble";
    const QString ble_stat[MAX_BLE_STAT] = {
        "CONNECTED",
        "DISCONNECT"
    };
    const QString sep = " ";


    QBluetoothLocalDevice *localDevice = nullptr;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
    get_recs_t infoDev;
    QList<QBluetoothDeviceInfo>list;
    QBluetoothAddress bleAddr;
    QString bleDevStr, bleAddrStr, bleDevNameAddr;
    QByteArray blePack;
    bool ble_connect = false;
    int tmr_ble, tmr_rst;
    const int tmr_ble_wait = 10000;
    bool bleFind = false;

    int index;

    QProgressBar *bleScan = nullptr;
    uint32_t tmr_ble_scan = 0;
    bool ble_proc = false;

    QByteArray rdata;
    QSqlDatabase db;
    bool openDB = false;
    int rec_count = 0;
    int tmr_sql = 0;
    QString tbl_stat;
    QString mk_table_db3;
    QTableWidget *tbl = nullptr;
    QTableView *tv = nullptr;
    QList<get_recs_t>rec_list;
    QSize minWinSql, maxWinSql;
    int selRow = -1;
    int con_number = -1;
#endif

};




#endif // MAINWINDOW_H
