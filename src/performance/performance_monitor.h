#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QMap>
#include <QVector>
#include <QMutex>
#include <memory>

/**
 * @brief 性能测试结果数据结构
 */
struct PerformanceResult {
    QString testCaseName;
    QString category;           // 测试类别（小/中/大规模）
    QString description;        // 测试描述
    
    // 程序特征
    int sourceLines;           // 源代码行数
    int tokenCount;            // Token数量
    int astNodes;              // AST节点数
    int astDepth;              // AST深度
    
    // 时间性能（毫秒）
    double lexicalTime;        // 词法分析时间
    double syntaxTime;         // 语法分析时间
    double semanticTime;       // 语义分析时间
    double codegenTime;        // 代码生成时间
    double totalTime;          // 总时间
    
    // 空间性能
    size_t memoryUsage;        // 内存使用量(KB)
    int symbolTableSize;       // 符号表大小
    int dfaStates;             // DFA状态数
    int lalrStates;            // LALR状态数
    
    // 处理效率
    double tokensPerSecond;    // Token处理速度
    double linesPerSecond;     // 行处理速度
    QString status;            // 测试状态
    QString errorMessage;      // 错误信息（如果有）
    
    // 构造函数
    PerformanceResult() 
        : sourceLines(0), tokenCount(0), astNodes(0), astDepth(0)
        , lexicalTime(0), syntaxTime(0), semanticTime(0), codegenTime(0), totalTime(0)
        , memoryUsage(0), symbolTableSize(0), dfaStates(0), lalrStates(0)
        , tokensPerSecond(0), linesPerSecond(0)
        , status("Unknown") {}
};

/**
 * @brief 测试用例数据结构
 */
struct TestCase {
    QString name;              // 测试用例名称
    QString description;       // 描述
    QString sourceCode;        // 源代码
    QString category;          // 分类（小/中/大规模）
    int expectedLines;         // 预期行数
    int expectedTokens;        // 预期Token数
    bool isEnabled;            // 是否启用
    
    TestCase() : expectedLines(0), expectedTokens(0), isEnabled(true) {}
    
    TestCase(const QString& n, const QString& desc, const QString& code, 
             const QString& cat, int lines = 0, int tokens = 0)
        : name(n), description(desc), sourceCode(code), category(cat)
        , expectedLines(lines), expectedTokens(tokens), isEnabled(true) {}
};

/**
 * @brief 性能统计信息
 */
struct PerformanceStatistics {
    // 时间统计
    double avgTotalTime;
    double maxTotalTime;
    double minTotalTime;
    
    // 各阶段平均时间
    double avgLexicalTime;
    double avgSyntaxTime;
    double avgSemanticTime;
    double avgCodegenTime;
    
    // 处理效率
    double avgThroughput;      // 平均吞吐量 (lines/sec)
    double maxThroughput;
    double minThroughput;
    
    // 资源使用
    double avgMemoryUsage;
    double maxMemoryUsage;
    
    // 瓶颈分析
    QString bottleneckStage;   // 性能瓶颈阶段
    double bottleneckRatio;    // 瓶颈阶段时间占比
    
    // 测试概览
    int totalTests;
    int successfulTests;
    int failedTests;
    double successRate;
    
    PerformanceStatistics() 
        : avgTotalTime(0), maxTotalTime(0), minTotalTime(0)
        , avgLexicalTime(0), avgSyntaxTime(0), avgSemanticTime(0), avgCodegenTime(0)
        , avgThroughput(0), maxThroughput(0), minThroughput(0)
        , avgMemoryUsage(0), maxMemoryUsage(0)
        , bottleneckRatio(0)
        , totalTests(0), successfulTests(0), failedTests(0), successRate(0) {}
};

/**
 * @brief 性能监控器类
 */
class PerformanceMonitor : public QObject {
    Q_OBJECT
    
public:
    explicit PerformanceMonitor(QObject* parent = nullptr);
    ~PerformanceMonitor();
    
    // 计时控制
    void startTiming(const QString& phase);
    void endTiming(const QString& phase);
    void resetTiming();
    
    // 内存监控
    void recordMemoryUsage();
    size_t getCurrentMemoryUsage();
    
    // 数据记录
    void recordProgramFeatures(int lines, int tokens, int astNodes, int astDepth);
    void recordCompilerState(int symbolTableSize, int dfaStates, int lalrStates);
    
    // 结果获取
    PerformanceResult getResult(const QString& testName, const QString& category = "");
    void clearResults();
    
    // 是否正在监控
    bool isMonitoring() const;
    
private:
    QMap<QString, QElapsedTimer> timers_;
    QMap<QString, double> timeResults_;
    QMutex mutex_;
    
    // 监控数据
    int sourceLines_;
    int tokenCount_;
    int astNodes_;
    int astDepth_;
    size_t maxMemoryUsage_;
    int symbolTableSize_;
    int dfaStates_;
    int lalrStates_;
    
    bool isMonitoring_;
    
    // 内存监控辅助方法
    size_t getProcessMemoryUsage();
};

#endif // PERFORMANCE_MONITOR_H 