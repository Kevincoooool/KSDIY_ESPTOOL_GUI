#-------------------------------------------------
#
# Project created by QtCreator 2016-05-27T10:58:03
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KSDIY_ESPTOOL_GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
TRANSLATIONS = i18n_zh_cn.ts i18n_en_us.ts
