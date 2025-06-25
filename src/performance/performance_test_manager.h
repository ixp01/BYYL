#ifndef PERFORMANCE_TEST_MANAGER_H
#define PERFORMANCE_TEST_MANAGER_H

#include "performance_monitor.h"
#include "test_case_generator.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QTimer>
#include <memory>

// 前向声明
class Lexer;
class Grammar;
class LALRParser;
class SemanticAnalyzer;
class CodeGenerator;

/**
 * @brief 性能测试管理器类
 */
class PerformanceTestManager : public QObject {
    Q_OBJECT
    
public:
    enum class TestLevel {
        QUICK,          // 快速测试 (仅小规模)
        STANDARD,       // 标准测试 (小+中规模)
        COMPREHENSIVE   // 全面测试 (所有规模)
    };
    
    explicit PerformanceTestManager(QObject* parent = nullptr);
    ~PerformanceTestManager();
    
    // 测试控制
    void runPerformanceTests(TestLevel level);
    void stopTests();
    void pauseTests();
    void resumeTests();
    
    // 测试用例管理
    void addTestCase(const TestCase& testCase);
    void removeTestCase(const QString& name);
    void clearTestCases();
    void enableTestCase(const QString& name, bool enabled);
    
    // 结果管理
    QVector<PerformanceResult> getResults() const;
    PerformanceStatistics getStatistics() const;
    void clearResults();
    
    // 导出功能
    bool exportResults(const QString& filePath, const QString& format = "csv");
    bool exportReport(const QString& filePath);
    
    // 状态查询
    bool isRunning() const;
    int getCurrentTestIndex() const;
    int getTotalTestCount() const;
    double getProgress() const;
    
signals:
    void testStarted(const QString& testName);
    void testCompleted(const PerformanceResult& result);
    void testFailed(const QString& testName, const QString& error);
    void allTestsCompleted(const QVector<PerformanceResult>& results);
    void progressChanged(int current, int total, double percentage);
    void statusChanged(const QString& status);
    void testingStopped();
    
private slots:
    void runNextTest();
    void onTestTimeout();
    
private:
    // 测试执行
    void executeTest(const TestCase& testCase);
    bool runSingleTest(const TestCase& testCase, PerformanceResult& result);
    
    // 编译器组件测试
    bool testLexicalAnalysis(const QString& code, PerformanceResult& result);
    bool testSyntaxAnalysis(const std::vector<Token>& tokens, PerformanceResult& result);
    bool testSemanticAnalysis(std::shared_ptr<ASTNode> ast, PerformanceResult& result);
    bool testCodeGeneration(std::shared_ptr<ASTNode> ast, PerformanceResult& result);
    
    // 统计分析
    PerformanceStatistics calculateStatistics() const;
    QString findBottleneckStage(const QVector<PerformanceResult>& results) const;
    
    // 数据成员
    QVector<TestCase> testCases_;
    QVector<PerformanceResult> results_;
    std::unique_ptr<PerformanceMonitor> monitor_;
    
    // 测试状态
    bool isRunning_;
    bool isPaused_;
    int currentTestIndex_;
    TestLevel currentLevel_;
    
    // 测试组件
    std::unique_ptr<Lexer> lexer_;
    std::unique_ptr<Grammar> grammar_;
    std::unique_ptr<LALRParser> parser_;
    std::unique_ptr<SemanticAnalyzer> semanticAnalyzer_;
    std::unique_ptr<CodeGenerator> codeGenerator_;
    
    // 定时器
    QTimer* testTimer_;
    QTimer* timeoutTimer_;
    
    // 配置
    int testTimeoutMs_;
    bool enableDetailedLogging_;
};

/**
 * @brief 性能测试线程类
 */
class PerformanceTestThread : public QThread {
    Q_OBJECT
    
public:
    explicit PerformanceTestThread(PerformanceTestManager* manager, QObject* parent = nullptr);
    
    void setTestCases(const QVector<TestCase>& testCases);
    void stop();
    
signals:
    void testProgress(const QString& testName, double progress);
    void testCompleted(const PerformanceResult& result);
    void allTestsFinished();
    
protected:
    void run() override;
    
private:
    PerformanceTestManager* manager_;
    QVector<TestCase> testCases_;
    bool shouldStop_;
};

#endif // PERFORMANCE_TEST_MANAGER_H 