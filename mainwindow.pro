#-------------------------------------------------#
# Project created by QtCreator 2024-01-01         #
#-------------------------------------------------#

QT       += core gui widgets

TARGET = QWKExample_MainWindow
TEMPLATE = app

CONFIG += c++17

# 头文件
HEADERS += \
    FramelessDialog.h \
    framelesshelper.hpp \
    framelesswindow.h

# 源文件
SOURCES += \
    FramelessDialog.cpp \
    framelesswindow.cpp \
    main.cpp

# 资源文件
RESOURCES +=

# 包含路径
INCLUDEPATH += $$PWD/thirdparty/qwindowkit/include

# QWKWidgets 库配置
CONFIG(debug, debug|release) {
    # Debug 版本
    LIBS += -L$$PWD/thirdparty/qwindowkit/lib -lQWKWidgetsd -lWidgetFramed -lQWKCored
} else {
    # Release 版本
    LIBS += -L$$PWD/thirdparty/qwindowkit/lib -lQWKWidgets -lWidgetFrame -lQWKCore
}

RESOURCES += \
    shared/resources/shared.qrc
