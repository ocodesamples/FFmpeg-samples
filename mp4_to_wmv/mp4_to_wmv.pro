TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += /usr/local/include
LIBS += -L/usr/local/lib -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswscale
SOURCES += main.cpp
