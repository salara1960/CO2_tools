#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
/*
    Класс для создания эмулятора круглого дисплея IPS GC9A01
*/

#include <QPainter>
#include <QTextItem>
#include <QString>

#include "defs.h"

//---------------------------------------------------------------------

#define MAX_COLOR 8

//---------------------------------------------------------------------

namespace Ui {
    class itWidget;
}

//---------------------------------------------------------------------
class itWidget : public QWidget
{
    Q_OBJECT

public:
    explicit itWidget(QWidget *parent = 0);
    ~itWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

public slots:
    void refresh(void *, int);
    void sec2str(QString *, uint32_t);

signals:
    void sig_refresh(void *, int);

private:
    Ui::itWidget *ui;
    QRect rect, rc, rc_co2, rc_time, rc_humi, rc_temp;
    data_t rec;
    int tmrs, ind = 0, cmd;
    Qt::GlobalColor all_color[MAX_COLOR] = {
        Qt::blue,
        Qt::red,
        Qt::green,
        Qt::cyan,
        Qt::magenta,
        Qt::yellow,
        Qt::white,
        Qt::black
    };

};
//---------------------------------------------------------------------


#endif // WIDGET_H
