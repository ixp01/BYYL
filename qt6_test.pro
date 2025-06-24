QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Qt6兼容性支持
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat
}

CONFIG += c++17

TARGET = Qt6Test
TEMPLATE = app

SOURCES += qt6_check.cpp 