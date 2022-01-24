#include "widget.h"
#include "ui_widget.h"

//--------------------------------------------------------------------------------
Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setWindowOpacity(1);//set the level of transparency
    rect = this->geometry();

    cmd = msg_ini;

    memset((uint8_t *)&rec, 0, sizeof(data_t));

    rc.setX(rect.width()/2 - 117);
    rc.setY(rect.height()/2 - 110);
    rc.setWidth(234);
    rc.setHeight(234);

    rc_co2.setX(rc.x() + (rc.width() / 10) - 4);
    rc_co2.setY(rc.y() + (rc.height() / 4));
    rc_co2.setWidth(rc.width() - (rc.width() / 6));
    rc_co2.setHeight(32);

    rc_time.setX(rc.x() + (rc.width() / 14));
    rc_time.setY(rc.y() + (rc.height() / 2) - 22);
    rc_time.setWidth(rc.width() - (rc.width() / 8));
    rc_time.setHeight(32);

    rc_humi.setX(rc_co2.x());
    rc_humi.setY(rc.y() + (rc.height() / 2) + 16);
    rc_humi.setWidth(rc.width() - (rc.width() / 6));
    rc_humi.setHeight(32);

    rc_temp.setX(rc_humi.x());
    rc_temp.setY(rc_humi.y() + 24);
    rc_temp.setWidth(rc_humi.width());
    rc_temp.setHeight(32);

    tmrs = startTimer(1000);// 1sec.

    connect(this,  &Widget::sig_refresh, this, &Widget::refresh);
}
//--------------------------------------------------------------------------------
Widget::~Widget()
{
    delete ui;
}
//--------------------------------------------------------------------------------
void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::FlatCap));
    painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));
    painter.drawEllipse(rc);

    switch (cmd) {
        case msg_ini:
        case msg_rst:
        case msg_dis:
            return;
        break;
        case msg_sec:
        case msg_evt:
        case msg_con:
        {
            QColor color(Qt::black);
            float co2 = rec.co2;
            if (rec.ppm < 400) {
                color = Qt::green;
            } else if ((rec.ppm >= 400) && (rec.ppm < 600)) {
                color = Qt::yellow;
            } else if ((rec.ppm >= 600) && (rec.ppm < 1000)) {
                color = Qt::magenta;
            } else if (rec.ppm >= 1000) {
                color = Qt::red;
            }

            Qt::GlobalColor cvet = all_color[ind];
            painter.setPen(QPen(cvet, 2, Qt::SolidLine, Qt::FlatCap));
            painter.drawEllipse(rc);
            ind++;
            if (ind >= MAX_COLOR) ind = 0;
            //
            painter.setFont(QFont("Arial Bold", 16));
            QString text = tr("CO2 : %1%").arg(co2);
            painter.setPen(QPen(color, 6, Qt::SolidLine, Qt::FlatCap));
            painter.drawText(rc_co2, Qt::AlignCenter, text);
            //
            sec2str(&text, rec.dt);
            painter.setFont(QFont("Sans Serif Bold", 18));
            painter.setPen(QPen(Qt::white, 6, Qt::SolidLine, Qt::FlatCap));
            painter.drawText(rc_time, Qt::AlignCenter, text);
            //
            painter.setFont(QFont("Arial italic", 14));
            text = tr("Humi : %1%").arg(rec.humi);
            painter.setPen(QPen(Qt::magenta, 4, Qt::SolidLine, Qt::FlatCap));
            painter.drawText(rc_humi, Qt::AlignCenter, text);

            text = tr("Temp : %1C").arg(rec.temp);
            painter.setPen(QPen(Qt::cyan, 4, Qt::SolidLine, Qt::FlatCap));
            painter.drawText(rc_temp, Qt::AlignCenter, text);
        }
        break;
    }

}
//--------------------------------------------------------------------------------
void Widget::refresh(void *s, int cd)
{
    data_t *rc = (data_t *)s;
    rec = *rc;
    cmd = cd;
    repaint();
}
//--------------------------------------------------------------------------------
void Widget::sec2str(QString *st, uint32_t dt)
{
    unsigned long sec = dt;
    unsigned long day = sec / (60 * 60 * 24);
    sec %= (60 * 60 * 24);
    unsigned long hour = sec / (60 * 60);
    sec %= (60 * 60);
    unsigned long min = sec / (60);
    sec %= 60;
    st->sprintf("%lu.%02lu:%02lu:%02lu", day, hour, min, sec);
}
//--------------------------------------------------------------------------------
void Widget::timerEvent(QTimerEvent *event)
{
    if (tmrs == event->timerId()) {
        if (rec.dt) {
            refresh((void *)&rec, msg_sec);
            rec.dt++;
        }
    }

}
//--------------------------------------------------------------------------------
