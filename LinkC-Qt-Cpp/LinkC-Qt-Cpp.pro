#-------------------------------------------------
#
# Project created by QtCreator 2015-07-11T09:38:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LinkC-Qt-Cpp
TEMPLATE = app
LIBS += -lwsock32
QMAKE_CXXFLAGS += -std=c++11


SOURCES += main.cpp \
    LoginWindow.cpp \
    gurgle.cpp \  
    MainWindow.cpp

HEADERS  += \
    LoginWindow.h \
    gurgle.h \
    MainWindow.h
