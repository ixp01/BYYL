QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 项目名称
TARGET = CompilerFrontend
TEMPLATE = app

# 包含目录
INCLUDEPATH += src/lexer src/parser src/semantic src/codegen src/gui src/utils

# 源文件
SOURCES += \
    main.cpp \
    src/gui/mainwindow.cpp \
    src/lexer/token.cpp

# 头文件
HEADERS += \
    src/gui/mainwindow.h \
    src/lexer/token.h \
    src/parser/ast.h

# UI文件
FORMS += \
    src/gui/ui/mainwindow.ui

# 测试配置（可选编译）
test {
    SOURCES += tests/test_main.cpp
    TARGET = test_runner
    QT -= gui widgets
}

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra -std=c++17

# Debug配置
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
    QMAKE_CXXFLAGS += -g -O0
}

# Release配置
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += -O2 -DNDEBUG
}

# 默认部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
