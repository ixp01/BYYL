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
    src/gui/code_editor.cpp \
    src/gui/analysis_panel.cpp \
    src/lexer/token.cpp \
    src/lexer/dfa.cpp \
    src/lexer/lexer.cpp \
    src/lexer/minimizer.cpp \
    src/parser/grammar.cpp \
    src/parser/lalr.cpp \
    src/parser/parser.cpp \
    src/parser/ast.cpp \
    src/semantic/symbol_table.cpp \
    src/semantic/semantic_analyzer.cpp \
    src/codegen/intermediate_code.cpp \
    src/codegen/code_generator.cpp

# 头文件
HEADERS += \
    src/gui/mainwindow.h \
    src/gui/code_editor.h \
    src/gui/analysis_panel.h \
    src/lexer/token.h \
    src/lexer/dfa.h \
    src/lexer/lexer.h \
    src/lexer/minimizer.h \
    src/parser/grammar.h \
    src/parser/lalr.h \
    src/parser/parser.h \
    src/parser/ast.h \
    src/semantic/symbol_table.h \
    src/semantic/semantic_analyzer.h \
    src/codegen/intermediate_code.h \
    src/codegen/code_generator.h

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
