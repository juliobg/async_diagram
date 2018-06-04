#ifndef CINPUTREADER_H
#define CINPUTREADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QObject>
#include <QSocketNotifier>
#include <QTextStream>

class CInputReader : public QObject
{
    Q_OBJECT
public:
    CInputReader(int fd, QObject *parent) :
        QObject(parent), iStream(fdopen(fd, "r")),
        iNotifier(fd, QSocketNotifier::Read)
    {

        connect(&iNotifier, SIGNAL(activated(int)), this, SLOT(Text()));
    }
signals:
    void textReceived(std::string message);
public slots:
    void Text()
    {
        QString line = iStream.readLine();
        emit textReceived(line.toStdString());
    }
private:
    QSocketNotifier iNotifier;
    QTextStream iStream;
};

#endif // CINPUTREADER_H
