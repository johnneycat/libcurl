#include "mainwindow.h"

#include <QApplication>


#include <windows.h>
#include <tchar.h>

int main(int argc, char *argv[])
{

    TCHAR szPathCopy[_MAX_PATH];
    GetTempPath(_MAX_PATH,szPathCopy);
    GetTempFileName(szPathCopy,_T("Tempfilename"),0,szPathCopy);


    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
