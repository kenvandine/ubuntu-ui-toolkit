TEMPLATE = lib
TARGET = ../UbuntuTest
QT += core-private qml qml-private quick quick-private

equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 2) {
    QT += v8-private
}

CONFIG += qt plugin no_keywords

QMAKE_CXXFLAGS += -Werror

TARGET = $$qtLibraryTarget($$TARGET)
uri = Ubuntu.Test

HEADERS += \
    uctestcase.h \

SOURCES += \
    uctestcase.cpp \

# deployment rules for the plugin
installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
target.path = $$installPath
INSTALLS += target
