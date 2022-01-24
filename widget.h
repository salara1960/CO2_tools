#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTextItem>

#include "defs.h"


#define MAX_COLOR 8


//QT_BEGIN_NAMESPACE

namespace Ui {
    class Widget;
}

//QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *event) override;
    virtual void timerEvent(QTimerEvent *event) override;

public slots:
    void refresh(void *, int);
    void sec2str(QString *, uint32_t);

//private slots:
//    void refresh();
signals:
    void sig_refresh(void *, int);

private:
    Ui::Widget *ui;
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


#endif // WIDGET_H
