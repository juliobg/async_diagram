#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(CInputReader* aReader, QWidget *parent) :
    QMainWindow(parent), iReader(aReader),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->widget->SetEventList(&iCollection);
    connect(iReader, SIGNAL(textReceived(std::string)), this, SLOT(Text(std::string)));

}


MainWindow::~MainWindow()
{
    delete ui;
}
