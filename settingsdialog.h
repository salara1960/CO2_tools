#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>

#define TOTAL_SPEED 11

extern qint32 serSpeed;

//---------------------------------------------------------------------------

QT_BEGIN_NAMESPACE

namespace Ui {
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
    };

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    Settings settings() const;


private slots:
    void showPortInfo(int idx);
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void updateSettings();

    void slot_confShow();

private:
    void fillPortsParameters();
    void fillPortsInfo();

signals:
    void sigUpdate();
    void sig_confShow();

private:

    const int spd_all[TOTAL_SPEED] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 500000, 576000, 921600, 1000000};

    Ui::SettingsDialog *m_ui = nullptr;
    Settings m_currentSettings;
    QIntValidator *m_intValidator = nullptr;
    int curPort = -1;
};

//---------------------------------------------------------------------------

#endif // SETTINGSDIALOG_H
