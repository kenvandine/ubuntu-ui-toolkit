TEMPLATE = app
TARGET = quick-plus-scene
QT += qml core-private gui-private quick-private
qtHaveModule(widgets): QT += widgets
CONFIG += c++11

SOURCES += quickplusscene.cpp

DEFINES += QML_RUNTIME_TESTING QT_QML_DEBUG_NO_WARNING
INCLUDEPATH += $${OUT_PWD}/../../include
LIBS += -L$${OUT_PWD}/../../lib -lquickplus

!equals(DISABLE_LTTNG, "1") {
    LIBS += -lquickplus-lttng
} else {
    DEFINES += DISABLE_LTTNG
}

load(qt_tool)