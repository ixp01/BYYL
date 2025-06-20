QT += core gui widgets

CONFIG += c++17

TARGET = LexerDemo
TEMPLATE = app

# 包含目录
INCLUDEPATH += src/lexer

# 源文件
SOURCES += \
    main_simple.cpp \
    src/lexer/token.cpp \
    src/lexer/dfa.cpp \
    src/lexer/lexer.cpp \
    src/lexer/minimizer.cpp

# 头文件
HEADERS += \
    src/lexer/token.h \
    src/lexer/dfa.h \
    src/lexer/lexer.h \
    src/lexer/minimizer.h

# 编译器标志
QMAKE_CXXFLAGS += -Wall -Wextra -std=c++17 