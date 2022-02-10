//----------------------------------------------------------------------------------

#include "mainwindow.h"
#include <QApplication>
#include <QLockFile>
#include <QDir>
#include <QMessageBox>
#include <QTextCodec>
#include <iostream>

//----------------------------------------------------------------------------------
//
//                Основной модуль программы CO2_tools
//
//----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

    try {

        QApplication app(argc, argv);

        app.setApplicationVersion(vers);
        app.setApplicationName(title);
        app.setOrganizationName("Home");
        app.setOrganizationDomain("https://github.com/salara1960");

        QLockFile lockFile(QDir::temp().absoluteFilePath("bp_tools.lock"));
        if (!lockFile.tryLock(100)) {
            QMessageBox::warning(nullptr, "Attention !", "Program already started");

            return 1;
        }

        MainWindow wnd(nullptr);

        wnd.show();

        app.exec();
    }

    catch (MainWindow::TheError(er)) {
        int cerr = er.code;
        QString errStr = "", cerrStr;
        cerrStr.asprintf("%d", cerr);
        if (cerr > 0) {
            if (cerr & 1) errStr.append("Error create serial port object (" + cerrStr + ")\n");
            if (cerr & 2) errStr.append("Error starting internal timer (" + cerrStr + ")\n");
            if (cerr & 4) errStr.append("Error create settings object\n");
            if (cerr & 8) errStr.append("Error reading from serial port\n");
            if (cerr & 16) errStr.append("Error get memory for read file\n");
        } else errStr.append("Unknown Error (" + cerrStr + ")\n");

        if (errStr.length() > 0) perror(reinterpret_cast<char *>(cerrStr.data()));

        return cerr;
    }
    catch (std::bad_alloc const&) {
        perror("Error while get alloc memory (function new)\n");

        return -1;
    }

    return 0;
}

//----------------------------------------------------------------------------------
