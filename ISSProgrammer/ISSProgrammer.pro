#-------------------------------------------------
#
# Project created by QtCreator 2017-01-05T00:19:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = ISSProgrammer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        serialthread.cpp

HEADERS  += mainwindow.h\
        serialthread.h

FORMS    += mainwindow.ui
