#include "mainwindow.h"
#include "inputreader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CInputReader inputreader{open("/Users/julio/Documents/singleinteresareaNAME.log", O_RDONLY),0};
    MainWindow w(&inputreader, 0);
    w.show();

    return a.exec();
}
