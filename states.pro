#-------------------------------------------------
#
# Project created by QtCreator 2015-12-31T00:15:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = states
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    stateswidget.cpp \
    cevent.cpp \
    inputreader.cpp

HEADERS  += mainwindow.h \
    stateswidget.h \
    cevent.h \
    inputreader.h \
    quadtree.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += -std=c++11
