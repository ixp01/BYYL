# 性能测试模块

## 概述

性能测试模块是编译器前端的重要组成部分，用于评估编译器在不同规模程序上的性能表现。该模块提供了完整的性能监控、测试用例生成和结果分析功能。

## 模块组成

### 1. PerformanceMonitor (性能监控器)
- **功能**: 负责收集编译过程中的时间和内存使用数据
- **特性**: 
  - 高精度计时(毫秒级)
  - 跨平台内存监控(Linux/Windows/macOS)
  - 线程安全的数据收集
  - 实时性能数据记录

### 2. TestCaseGenerator (测试用例生成器)
- **功能**: 自动生成不同规模和复杂度的测试程序
- **测试规模**:
  - **小规模** (< 100行): 基本表达式、简单控制结构
  - **中规模** (100-500行): 函数定义、数组操作、嵌套结构
  - **大规模** (500-2000行): 复杂程序、深度嵌套、大量变量

### 3. PerformanceTestManager (性能测试管理器)
- **功能**: 协调整个性能测试流程
- **特性**:
  - 测试级别控制(快速/标准/全面)
  - 并行测试执行
  - 结果统计分析
  - 导出功能

### 4. PerformancePanel (性能测试界面)
- **功能**: 提供用户友好的性能测试GUI界面
- **特性**:
  - 实时进度显示
  - 结果表格展示
  - 统计信息面板
  - 数据导出功能

## 性能指标

### 时间指标
- **词法分析时间**: Token识别和DFA处理耗时
- **语法分析时间**: AST构建和LALR分析耗时
- **语义分析时间**: 符号表管理和类型检查耗时
- **代码生成时间**: 中间代码生成耗时
- **总编译时间**: 完整编译流程耗时

### 空间指标
- **内存使用峰值**: 编译过程中的最大内存占用
- **符号表大小**: 符号表中的符号数量
- **DFA状态数**: 词法分析器的状态数量
- **LALR状态数**: 语法分析器的状态数量

### 效率指标
- **处理速度**: 每秒处理的代码行数 (lines/sec)
- **Token处理速度**: 每秒处理的Token数量 (tokens/sec)
- **内存效率**: 单位代码的内存消耗 (KB/line)

## 使用方法

### 1. 基本使用流程

```cpp
// 1. 创建性能监控器
auto monitor = std::make_unique<PerformanceMonitor>();

// 2. 开始性能监控
monitor->startTiming("lexical");
// 执行词法分析...
monitor->endTiming("lexical");

// 3. 记录程序特征
monitor->recordProgramFeatures(lines, tokens, astNodes, astDepth);

// 4. 获取结果
PerformanceResult result = monitor->getResult("test_case_name");
```

### 2. 批量性能测试

```cpp
// 1. 创建测试管理器
auto testManager = std::make_unique<PerformanceTestManager>();

// 2. 运行性能测试
testManager->runPerformanceTests(TestLevel::STANDARD);

// 3. 获取结果
QVector<PerformanceResult> results = testManager->getResults();
PerformanceStatistics stats = testManager->getStatistics();
```

### 3. GUI界面使用

1. 启动编译器前端程序
2. 切换到"性能测试"标签页
3. 选择测试级别：
   - **快速测试**: 仅运行小规模测试用例
   - **标准测试**: 运行小规模和中规模测试用例
   - **全面测试**: 运行所有规模的测试用例
4. 点击"开始测试"按钮
5. 查看实时进度和结果
6. 导出测试报告

## 测试用例示例

### 小规模测试用例
```c
int main() {
    int a = 10;
    int b = 20;
    int c = a + b * 2;
    return c;
}
```

### 中规模测试用例
```c
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = factorial(i);
    }
    return arr[9];
}
```

### 大规模测试用例
- 包含多个函数定义
- 复杂的控制流结构
- 大量变量声明和操作
- 深度嵌套的代码块

## 性能测试结果示例

| 测试用例 | 规模 | 总时间(ms) | 词法(ms) | 语法(ms) | 语义(ms) | 代码生成(ms) | 内存(KB) | 吞吐量(lines/s) |
|---------|------|------------|----------|----------|----------|--------------|----------|----------------|
| simple_arithmetic | 小规模 | 2.8 | 0.8 | 1.2 | 0.5 | 0.3 | 1024 | 1785 |
| function_def | 中规模 | 24.5 | 3.2 | 12.8 | 6.4 | 2.1 | 2048 | 1836 |
| complex_prog | 大规模 | 166.4 | 18.7 | 89.3 | 45.6 | 12.8 | 8192 | 1446 |

## 性能优化建议

基于性能测试结果，可以识别编译器的性能瓶颈：

1. **词法分析优化**:
   - DFA最小化
   - Token缓存策略

2. **语法分析优化**:
   - LALR表压缩
   - AST节点内存池

3. **语义分析优化**:
   - 符号表哈希优化
   - 作用域管理优化

4. **代码生成优化**:
   - 中间代码优化
   - 寄存器分配优化

## 扩展功能

### 1. 自定义测试用例
```cpp
TestCase customTest("my_test", "自定义测试", sourceCode, "中规模", 100, 500);
testManager->addTestCase(customTest);
```

### 2. 性能比较分析
- 不同版本编译器的性能对比
- 不同优化级别的性能分析
- 不同平台的性能差异分析

### 3. 自动化回归测试
- 集成到CI/CD流程
- 性能退化检测
- 自动生成性能报告

## 注意事项

1. **测试环境一致性**: 确保在相同的硬件和软件环境下进行测试
2. **多次测试取平均值**: 减少系统负载波动对结果的影响
3. **内存泄漏检测**: 长时间测试时注意内存使用情况
4. **测试用例覆盖性**: 确保测试用例覆盖各种语言特性

## 技术细节

### 内存监控实现
- **Linux**: 通过 `/proc/self/status` 读取VmRSS
- **Windows**: 使用 `GetProcessMemoryInfo` API
- **macOS**: 使用 `mach_task_basic_info` 结构

### 高精度计时
- 使用 `QElapsedTimer` 提供毫秒级精度
- 支持多阶段嵌套计时
- 线程安全的计时器管理

### 数据导出格式
- **CSV**: 原始数据，便于进一步分析
- **HTML**: 格式化报告，包含图表
- **JSON**: 结构化数据，便于程序处理

这个性能测试模块为编译器前端提供了全面的性能评估能力，有助于识别性能瓶颈并指导优化工作。 