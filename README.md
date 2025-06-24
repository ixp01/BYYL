# 编译器前端 (Compiler Frontend)

一个完整的编译器前端实现，包含词法分析、语法分析、语义分析和中间代码生成四个阶段，配备现代化的Qt图形用户界面。

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-lightgrey.svg)
![Language](https://img.shields.io/badge/language-C%2B%2B17-orange.svg)
![Qt](https://img.shields.io/badge/Qt-5.15%2B-green.svg)

## 📋 项目概述

本项目是一个用于编译原理教学和研究的完整编译器前端实现，支持类C语言的编译过程。项目采用模块化设计，每个编译阶段都可以独立测试和验证。

### 🎯 主要特性

- ✅ **完整的四阶段编译器前端**
  - 词法分析 (Lexical Analysis)
  - 语法分析 (Syntax Analysis) 
  - 语义分析 (Semantic Analysis)
  - 代码生成 (Code Generation)

- ✅ **先进的算法实现**
  - DFA最小化算法
  - LALR(1)语法分析器
  - 符号表管理
  - 三地址码生成

- ✅ **现代化图形界面**
  - 基于Qt5/Qt6的用户界面
  - 语法高亮编辑器
  - 实时错误标记
  - 多标签页分析结果展示

- ✅ **完善的测试体系**
  - 单元测试覆盖各个模块
  - 自动化测试脚本
  - 丰富的测试用例

## 🏗️ 项目结构

```
.
├── src/                    # 源代码目录
│   ├── lexer/             # 词法分析器
│   │   ├── token.h/cpp    # Token定义
│   │   ├── dfa.h/cpp      # DFA实现
│   │   ├── lexer.h/cpp    # 词法分析器
│   │   └── minimizer.h/cpp # DFA最小化
│   ├── parser/            # 语法分析器
│   │   ├── grammar.h/cpp  # 文法定义
│   │   ├── lalr.h/cpp     # LALR分析器
│   │   ├── parser.h/cpp   # 语法分析器
│   │   └── ast.h/cpp      # 抽象语法树
│   ├── semantic/          # 语义分析器
│   │   ├── symbol_table.h/cpp      # 符号表
│   │   └── semantic_analyzer.h/cpp # 语义分析器
│   ├── codegen/          # 代码生成器
│   │   ├── intermediate_code.h/cpp # 中间代码表示
│   │   └── code_generator.h/cpp    # 代码生成器
│   ├── gui/              # 图形用户界面
│   │   ├── mainwindow.h/cpp       # 主窗口
│   │   ├── analysis_panel.h/cpp   # 分析面板
│   │   ├── code_editor.h/cpp      # 代码编辑器
│   │   └── ...                    # 其他UI组件
│   └── utils/            # 工具类
├── tests/                # 单元测试
│   ├── test_main.cpp     # 测试主程序
│   ├── test_lexer.cpp    # 词法分析器测试
│   ├── test_parser.cpp   # 语法分析器测试
│   ├── test_semantic.cpp # 语义分析器测试
│   └── test_codegen.cpp  # 代码生成器测试
├── test_cases/           # 集成测试用例
│   ├── simple_assignment.c
│   ├── arithmetic.c
│   ├── variables.c
│   ├── expressions.c
│   ├── errors.c
│   └── complex.c
├── run_tests.py         # 自动化测试脚本
├── test_cases.txt       # 详细测试说明
├── complier.pro         # Qt项目文件
├── main.cpp            # 程序入口
└── README.md           # 项目说明
```

## 🛠️ 编译环境要求

### 系统要求
- **操作系统**: Linux (推荐 Ubuntu 18.04+), Windows 10+, macOS 10.14+
- **编译器**: GCC 7.0+ 或 Clang 6.0+ (支持C++17)
- **Qt框架**: Qt 5.15+ 或 Qt 6.0+

### Ubuntu/Debian 安装依赖
```bash
sudo apt update
sudo apt install build-essential qt5-default qt5-qmake \
                 qtbase5-dev qttools5-dev-tools \
                 libqt5widgets5 libqt5gui5 libqt5core5a
```

### CentOS/RHEL 安装依赖
```bash
sudo yum groupinstall "Development Tools"
sudo yum install qt5-qtbase-devel qt5-qttools-devel
```

### Windows 安装依赖
1. 安装 [Qt 5.15+](https://www.qt.io/download)
2. 安装 MinGW 或 Visual Studio 2019+
3. 确保 qmake 在系统 PATH 中

### macOS 安装依赖
```bash
# 使用 Homebrew
brew install qt5
export PATH="/usr/local/opt/qt5/bin:$PATH"
```

## 🚀 编译和运行

### 方法一：使用 qmake (推荐)
```bash
# 1. 生成 Makefile
qmake complier.pro

# 2. 编译项目
make

# 3. 运行程序
./CompilerFrontend
```

### 方法二：使用 Qt Creator
```bash
# 1. 打开 Qt Creator
qtcreator complier.pro

# 2. 点击 "构建" 按钮编译
# 3. 点击 "运行" 按钮启动程序
```

### 清理编译文件
```bash
make clean
# 或者
make distclean  # 更彻底的清理
```

## 📚 使用指南

### 基础使用流程

1. **启动程序**
   ```bash
   ./CompilerFrontend
   ```

2. **编写代码**
   - 在代码编辑器中输入类C语言代码
   - 支持语法高亮和自动缩进
   - 实时显示行号和错误标记

3. **查看分析结果**
   - **词法分析标签页**: 查看Token序列和DFA信息
   - **语法分析标签页**: 查看AST结构和语法信息
   - **语义分析标签页**: 查看符号表和类型检查结果
   - **代码生成标签页**: 查看生成的三地址码

### 支持的语言特性

#### 数据类型
```c
int a;           // 整数类型
float b;         // 浮点类型
char c;          // 字符类型
bool flag;       // 布尔类型
```

#### 变量声明和初始化
```c
int x = 10;               // 声明并初始化
float pi = 3.14159;       // 浮点数
char ch = 'A';            // 字符
int arr[100];             // 数组声明
```

#### 表达式和运算符
```c
int result = (a + b) * c;          // 算术表达式
bool valid = (x > 0) && (y < 10);  // 逻辑表达式
int bits = a & b | c ^ d;          // 位运算
```

#### 控制结构
```c
// 条件语句
if (condition) {
    // ...
} else {
    // ...
}

// 循环语句
while (i < n) {
    i++;
}

for (int i = 0; i < 10; i++) {
    // ...
}
```

#### 函数定义
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
```

## 🧪 测试

### 运行自动化测试
```bash
# 生成测试用例并运行测试
python3 run_tests.py
```

### 运行单元测试
```bash
# 编译测试程序
cd tests
qmake
make

# 运行测试
./test_runner
```

### 手动测试建议

1. **词法分析测试**
   - 测试各种Token类型识别
   - 验证关键字、标识符、常量的正确解析
   - 检查注释处理和错误恢复

2. **语法分析测试**
   - 测试表达式优先级和结合性
   - 验证控制结构的解析
   - 检查语法错误的检测和报告

3. **语义分析测试**
   - 测试变量声明和作用域
   - 验证类型检查和类型转换
   - 检查未声明变量和重复声明的检测

4. **代码生成测试**
   - 验证三地址码的正确性
   - 检查临时变量的分配
   - 测试基本块划分和优化信息

## 🎨 界面功能

### 主要功能
- **代码编辑器**: 语法高亮、行号显示、自动缩进
- **分析面板**: 四个标签页显示各阶段分析结果
- **错误面板**: 显示编译错误和警告信息
- **状态栏**: 显示当前状态和光标位置

### 快捷键
- `Ctrl+N`: 新建文件
- `Ctrl+O`: 打开文件
- `Ctrl+S`: 保存文件
- `Ctrl+F`: 查找/替换
- `F5`: 运行分析
- `F11`: 全屏模式

## 🔧 高级配置

### 编译选项
在 `complier.pro` 中可以配置：
```pro
# 启用调试信息
CONFIG += debug

# 启用优化
CONFIG += release

# 添加额外的编译标志
QMAKE_CXXFLAGS += -Wall -Wextra
```

### 自定义设置
- **字体设置**: 通过"设置"菜单调整编辑器字体
- **主题设置**: 支持亮色和暗色主题切换
- **分析选项**: 可以启用/禁用特定的分析步骤

## 🐛 故障排除

### 常见问题

1. **编译错误: "Qt5Core not found"**
   ```bash
   # 确保Qt开发包已安装
   sudo apt install qtbase5-dev
   ```

2. **运行时错误: "cannot find libQt5Widgets.so"**
   ```bash
   # 设置LD_LIBRARY_PATH
   export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH
   ```

3. **界面显示异常**
   ```bash
   # 检查Qt版本兼容性
   qmake --version
   ```

### 调试模式
编译调试版本：
```bash
qmake CONFIG+=debug complier.pro
make
gdb ./CompilerFrontend
```

## 🤝 贡献指南

### 参与贡献
1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

### 代码规范
- 使用 C++17 标准
- 遵循 Qt 编码规范
- 添加必要的注释和文档
- 确保所有测试通过

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 👥 作者

- **xpjiao** - *初始工作* - [GitHub](https://github.com/ixp01)

## 🙏 致谢

- Qt Framework 团队提供优秀的GUI框架
- 编译原理经典教材《龙书》的理论指导
- 开源社区的宝贵贡献和反馈

## 📞 联系方式

如有问题或建议，请通过以下方式联系：

- 📧 Email: [your-email@example.com]
- 🐛 Issues: [GitHub Issues](https://github.com/ixp01/BYYL/issues)
- 📖 Wiki: [项目Wiki](https://github.com/ixp01/BYYL/wiki)

---

**⭐ 如果这个项目对你有帮助，请给个Star！** 