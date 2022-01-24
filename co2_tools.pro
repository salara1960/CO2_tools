QT       += core gui serialport charts qml quick quickwidgets bluetooth

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

requires(qtConfig(combobox))

TARGET = CO2_Tools
TEMPLATE = app

#Application version
VERSION_MAJOR = 1
VERSION_MINOR = 4
VERSION_BUILD = 0
DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH +=.
DEPENDPATH +=.

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    widget.cpp

HEADERS += \
    bnumber.h \
    defs.h \
    mainwindow.h \
    settingsdialog.h \
    ver.h \
    version.h \
    widget.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

#DISTFILES += \
#    compass.qml
