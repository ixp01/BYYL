# 编译器前端 - 编译原理课程设计

![项目状态](https://img.shields.io/badge/状态-已完成-brightgreen.svg)
![阶段进度](https://img.shields.io/badge/阶段8-100%25-blue.svg)
![代码行数](https://img.shields.io/badge/代码行数-16K+-orange.svg)

## 🎯 项目简介

这是一个完整的编译器前端实现，包含词法分析、语法分析、语义分析和中间代码生成四个核心阶段，并提供了专业的Qt图形用户界面。

### ✨ 核心特性

- **🔍 词法分析**: DFA + Hopcroft最小化算法 (97.06%压缩率)
- **📝 语法分析**: LALR(1)分析器 + AST构建
- **🔎 语义分析**: 符号表管理 + 16种错误检测
- **⚙️ 代码生成**: 三地址码中间表示
- **🖥️ 图形界面**: 专业IDE级别的用户界面

### 📊 项目统计

- **总代码量**: 16,325 行 C++/Qt代码
- **文档数量**: 1,634 行 Markdown文档
- **开发阶段**: 8个阶段，循序渐进
- **性能指标**: <100ms语法高亮，<50MB内存使用

## 🚀 快速开始

### 编译运行
```bash
# 安装依赖 (Ubuntu)
sudo apt install qt5-default qttools5-dev-tools

# 编译项目
qmake
make

# 运行程序
./CompilerFrontend
```

### 功能测试
```bash
# 词法分析测试
g++ -std=c++17 -O2 -I. -Isrc/lexer -o debug_lexer debug_lexer.cpp src/lexer/*.cpp
./debug_lexer

# 性能测试
g++ -std=c++17 -O2 -I. -Isrc/lexer -o performance_test performance_test.cpp src/lexer/*.cpp
./performance_test
```

## 📖 文档导航

- **[项目文档](docs/README.md)** - 详细的项目说明
- **[用户指南](docs/USER_GUIDE.md)** - 软件使用说明  
- **[技术架构](docs/ARCHITECTURE.md)** - 核心算法和设计
- **[开发总结](docs/PROJECT_SUMMARY.md)** - 完整开发历程

## 🏆 技术亮点

1. **DFA最小化**: 实现Hopcroft算法，状态压缩率达97.06%
2. **LALR语法分析**: 高效的自底向上语法分析
3. **实时语法高亮**: <100ms响应时间的多线程高亮
4. **现代GUI设计**: 专业IDE级别的用户界面
5. **完整错误处理**: 涵盖16种语义错误类型

## 📈 性能数据

- **词法分析**: 1.37×10⁷ 字节/秒
- **DFA压缩**: 从136状态减少到4状态
- **语法高亮**: <100ms实时响应
- **内存占用**: <50MB (包含GUI)
- **编译速度**: 各阶段<10ms (1000行代码)

## 🎓 课程信息

- **课程**: 编译原理课程设计
- **时间**: 2024-2025学年
- **语言**: C++17 + Qt5
- **平台**: Linux (Ubuntu 20.04+)

## 📞 联系方式

- **项目地址**: https://github.com/ixp01/BYYL
- **问题反馈**: GitHub Issues
- **技术交流**: 欢迎讨论编译原理相关技术

---

**🏅 项目成果**: 超出课程要求，达到企业级编译器工具水准 