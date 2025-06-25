#include "performance_monitor.h"
#include <QDebug>
#include <QMutexLocker>
#include <algorithm>

// 平台相关的内存监控
#ifdef Q_OS_LINUX
#include <unistd.h>
#include <fstream>
#include <string>
#elif defined(Q_OS_WIN)
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_MAC)
#include <mach/mach.h>
#endif

PerformanceMonitor::PerformanceMonitor(QObject* parent)
    : QObject(parent)
    , sourceLines_(0)
    , tokenCount_(0)
    , astNodes_(0)
    , astDepth_(0)
    , maxMemoryUsage_(0)
    , symbolTableSize_(0)
    , dfaStates_(0)
    , lalrStates_(0)
    , isMonitoring_(false)
{
}

PerformanceMonitor::~PerformanceMonitor()
{
}

void PerformanceMonitor::startTiming(const QString& phase)
{
    QMutexLocker locker(&mutex_);
    
    if (!timers_.contains(phase)) {
        timers_[phase] = QElapsedTimer();
    }
    
    timers_[phase].start();
    isMonitoring_ = true;
    
    qDebug() << "Performance: Started timing for" << phase;
}

void PerformanceMonitor::endTiming(const QString& phase)
{
    QMutexLocker locker(&mutex_);
    
    if (timers_.contains(phase) && timers_[phase].isValid()) {
        double elapsed = timers_[phase].elapsed();
        timeResults_[phase] = elapsed;
        
        qDebug() << "Performance:" << phase << "took" << elapsed << "ms";
    } else {
        qWarning() << "Performance: No timer found for phase" << phase;
    }
}

void PerformanceMonitor::resetTiming()
{
    QMutexLocker locker(&mutex_);
    
    timers_.clear();
    timeResults_.clear();
    
    // 重置监控数据
    sourceLines_ = 0;
    tokenCount_ = 0;
    astNodes_ = 0;
    astDepth_ = 0;
    maxMemoryUsage_ = 0;
    symbolTableSize_ = 0;
    dfaStates_ = 0;
    lalrStates_ = 0;
    
    isMonitoring_ = false;
    
    qDebug() << "Performance: Reset all timing data";
}

void PerformanceMonitor::recordMemoryUsage()
{
    size_t currentUsage = getProcessMemoryUsage();
    if (currentUsage > maxMemoryUsage_) {
        maxMemoryUsage_ = currentUsage;
    }
}

size_t PerformanceMonitor::getCurrentMemoryUsage()
{
    return getProcessMemoryUsage();
}

void PerformanceMonitor::recordProgramFeatures(int lines, int tokens, int astNodes, int astDepth)
{
    QMutexLocker locker(&mutex_);
    
    sourceLines_ = lines;
    tokenCount_ = tokens;
    astNodes_ = astNodes;
    astDepth_ = astDepth;
    
    qDebug() << "Performance: Recorded program features -"
             << "Lines:" << lines
             << "Tokens:" << tokens  
             << "AST Nodes:" << astNodes
             << "AST Depth:" << astDepth;
}

void PerformanceMonitor::recordCompilerState(int symbolTableSize, int dfaStates, int lalrStates)
{
    QMutexLocker locker(&mutex_);
    
    symbolTableSize_ = symbolTableSize;
    dfaStates_ = dfaStates;
    lalrStates_ = lalrStates;
    
    qDebug() << "Performance: Recorded compiler state -"
             << "Symbol Table Size:" << symbolTableSize
             << "DFA States:" << dfaStates
             << "LALR States:" << lalrStates;
}

PerformanceResult PerformanceMonitor::getResult(const QString& testName, const QString& category)
{
    QMutexLocker locker(&mutex_);
    
    PerformanceResult result;
    result.testCaseName = testName;
    result.category = category;
    
    // 程序特征
    result.sourceLines = sourceLines_;
    result.tokenCount = tokenCount_;
    result.astNodes = astNodes_;
    result.astDepth = astDepth_;
    
    // 时间性能
    result.lexicalTime = timeResults_.value("lexical", 0.0);
    result.syntaxTime = timeResults_.value("syntax", 0.0);
    result.semanticTime = timeResults_.value("semantic", 0.0);
    result.codegenTime = timeResults_.value("codegen", 0.0);
    result.totalTime = result.lexicalTime + result.syntaxTime + 
                      result.semanticTime + result.codegenTime;
    
    // 空间性能
    result.memoryUsage = maxMemoryUsage_;
    result.symbolTableSize = symbolTableSize_;
    result.dfaStates = dfaStates_;
    result.lalrStates = lalrStates_;
    
    // 处理效率计算
    if (result.totalTime > 0) {
        result.tokensPerSecond = (result.tokenCount * 1000.0) / result.totalTime; // 转换为秒
        result.linesPerSecond = (result.sourceLines * 1000.0) / result.totalTime;
    }
    
    // 状态判断
    if (result.totalTime > 0 && result.tokenCount > 0) {
        result.status = "Success";
    } else {
        result.status = "Failed";
        result.errorMessage = "No valid timing or parsing data";
    }
    
    return result;
}

void PerformanceMonitor::clearResults()
{
    resetTiming();
}

bool PerformanceMonitor::isMonitoring() const
{
    return isMonitoring_;
}

size_t PerformanceMonitor::getProcessMemoryUsage()
{
#ifdef Q_OS_LINUX
    // Linux下通过/proc/self/status获取内存使用情况
    std::ifstream file("/proc/self/status");
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            // 提取内存使用量(KB)
            std::string memStr = line.substr(7);
            // 移除单位"kB"
            size_t pos = memStr.find("kB");
            if (pos != std::string::npos) {
                memStr = memStr.substr(0, pos);
            }
            
            // 转换为数字
            try {
                return std::stoull(memStr);
            } catch (...) {
                return 0;
            }
        }
    }
    return 0;
    
#elif defined(Q_OS_WIN)
    // Windows下使用GetProcessMemoryInfo
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024; // 转换为KB
    }
    return 0;
    
#elif defined(Q_OS_MAC)
    // macOS下使用mach API
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
        return info.resident_size / 1024; // 转换为KB
    }
    return 0;
    
#else
    // 其他平台暂时返回0
    return 0;
#endif
} 