#-------------------------------------------------
#
# Project created 2016-05-29T13:49:48
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = k_ver
TEMPLATE = app

SOURCES += main.cpp\
           mainwindow.cpp

HEADERS += mainwindow.h

FORMS   += mainwindow.ui

# LIBS += -L where are your freenect libs  & -l libraries needed
LIBS    += -L /home/nadie/accesorios/OpenNI2/Redist -lOpenNI2 -lfreenect -lusb-1.0

# INCLUDEPATH += tell QT where do you hide your include's
INCLUDEPATH += /usr/include/libusb-1.0 \
               /home/nadie/accesorios/OpenNI2/Include #pásalo a /usr/include/OpenNI2

# due to problems compiling openni::Array
QMAKE_CXXFLAGS  += -std=gnu++11
