#-------------------------------------------------
#
# Project created by QtCreator 2015-02-25T19:37:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Onset
TEMPLATE = app


SOURCES += main.cpp\
        onset.cpp \
    qcustomplot.cpp \
    audio.cpp \
    audioplot.cpp \
    transform.cpp

HEADERS  += onset.h \
    qcustomplot.h \
    audio.h \
    audioplot.h \
    transform.h

FORMS    += onset.ui

LIBS     += -L$$PWD -lbass

RESOURCES += \
    res/resources.qrc
