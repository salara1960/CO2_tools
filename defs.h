/*
    Автор : Ильминский А.Н.
    Дата : 17.11.2022
    Сервисная прогамма для работы с устройством "CO2 sensor"
*/


#ifndef DEFS_H
#define DEFS_H


#include <inttypes.h>
#include <QString>
#include <QDateTime>

#include "version.h"
#include "bnumber.h"

//***********************************************************************************


#define SET_DEBUG

//#define SET_BLUETOOTH
#ifdef SET_BLUETOOTH
    #define SET_BLE_DEVICE
#endif


#define NEXT 0
#define LAST 1


#define max_buf           2048
#define BUF_SIZE          max_buf

#define TO_DEV_SIZE       1024

#define MIN_HUMI_VALUE    0// %
#define MAX_HUMI_VALUE  100// %
#define MIN_TEMP_VALUE  -40// град Цельсия
#define MAX_TEMP_VALUE   50// град Цельсия



#define _1s 100
#define _2s _1s * 2
#define _3s _1s * 3
#define _4s _1s * 4
#define _5s _1s * 5



// Маркосы-аналоги функций htons() htonl()
#define HTONS(x) \
    ((uint16_t)((x >> 8) | ((x << 8) & 0xff00)))
#define HTONL(x) \
    ((uint32_t)((x >> 24) | ((x >> 8) & 0xff00) | ((x << 8) & 0xff0000) | ((x << 24) & 0xff000000)))


//***********************************************************************************

enum {
    errNo = 0,   // "Ok",
    errExec,     // "Operation error",//"Ошибка выполнения операции",
    errRstFlash, // "FLASH reset error",//"Ошибка сброса FLASH",
    errWrFlash,  // "FLASH write error",//"Ошибка записи FLASH",
    errNoAck,    // "Error waiting for response",//"Ошибка ожидания ответа",
    errCmdReject,// "Command rejected",//"Команда отклонена",
    errSigInt,   // "Abort from keyboard",//"Процедура прервана с клавиатуры",
    errFatal,    // "Fatal error",//"Неисправимая ошибка",
    errFileLen,  // "Error fileLen",
    errFileOpen, // "Error fOpen",
    errGetMem,   // "Error getMem",
    errRdFile,   // "Error fRead",
    errWrFile,
    errUnknown   // "Unknown error"//Неизвестная ошибка"
};

enum {
    picClr = 0,
    picInfo,
    picTime,
    picWar,
    picErr,
    picCon,
    picDis,
    picOk
};

enum {
    gHumi,
    gTemp,
    gNone
};

enum {
    qFifo = 0,
    qErr,
    qTemp,
    qHumi,
    qNone
};

enum {
    noneBit = 0,
    i2cBit = 1,
    kellerBit = 2,
    framBit = 4,
    tempBit = 8,
    framFullBit = 16,
    vsBit = 32,
    fifoBit = 64,
    unknownBit = 128
};

enum {
    jpress = 0,
    jtemp,
    jhumi
};


enum {
    msg_ini = 0,
    msg_con,
    msg_dis,
    msg_sec,
    msg_evt,
    msg_rst,
    msg_none
};

typedef struct {
    float temp;
    float humi;
    uint16_t ppm;
    float co2;
    uint16_t que;
    uint32_t dt;
} data_t;


//***********************************************************************************

#endif // DEFS_H
