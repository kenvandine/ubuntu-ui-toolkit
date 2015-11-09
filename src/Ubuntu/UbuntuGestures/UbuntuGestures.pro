TEMPLATE=lib
TARGET=UbuntuGestures

QT *= core-private gui-private qml qml-private quick quick-private

CONFIG += dll no_keywords c++11

INCLUDEPATH+=$$PWD

DEFINES += UBUNTUGESTURES_LIBRARY
CMAKE_MODULE_TESTS = -

load(qt_build_config)
load(ubuntu_qt_module)

HEADERS += candidateinactivitytimer.h \
           debughelpers.h \
           timer.h \
           timesource.h \
           touchownershipevent.h \
           touchregistry.h \
           unownedtouchevent.h \
           ubuntugesturesglobal.h \
           pool.h \

SOURCES += candidateinactivitytimer.cpp \
           debughelpers.cpp \
           timer.cpp \
           timesource.cpp \
           touchownershipevent.cpp \
           touchregistry.cpp \
           unownedtouchevent.cpp \
