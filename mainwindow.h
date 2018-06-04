#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "cevent.h"
#include "inputreader.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(CInputReader* aReader, QWidget *parent = 0);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    CInputReader* iReader;
    CEventCollection iCollection;

public slots:
    void Text(std::string aLine)
    {
        auto event = CEvent::ReadLine(aLine);
        if (event != NULL)
            iCollection.AddEvent(*event);
    }
};

#endif // MAINWINDOW_H
