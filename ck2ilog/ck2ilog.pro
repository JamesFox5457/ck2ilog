#-------------------------------------------------
#
# Project created by QtCreator 2017-08-30T11:17:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ck2ilog
TEMPLATE = app


SOURCES += main.cpp\
        ck2ilogwindow.cpp \
    DataUtils/dataparser.cpp \
    DataUtils/fileio.cpp \
    DataUtils/scanres.cpp

HEADERS  += ck2ilogwindow.h \
    DataUtils/dataparser.h \
    DataUtils/fileio.h \
    DataUtils/scanres.h

FORMS    += ck2ilogwindow.ui
