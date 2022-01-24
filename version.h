/*
    Автор : Ильминский А.Н.
    Дата : 17.01.2022
    Сервисная прогамма для работы с устройством "CO2 sensor"
*/


#ifndef VERSION_H
#define VERSION_H

#include <QString>

const QString vers = QString("%1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg(VERSION_BUILD);

#endif // VERSION_H
