QT += core serialport widgets
QT -= gui

CONFIG += c++17

TARGET = JointImpedanceControl
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    motorcontrol.cpp

HEADERS += \
    mainwindow.h \
    motorcontrol.h

# 默认构建目录
DESTDIR = $$PWD/bin

# Windows特定设置
win32 {
    RC_ICONS = app.ico
}

# 发布模式优化
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}
