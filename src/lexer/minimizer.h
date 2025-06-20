#ifndef MINIMIZER_H
#define MINIMIZER_H

#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include "dfa.h"

/**
 * @brief DFA最小化器类
 * 实现Hopcroft算法进行DFA状态最小化
 */
class DFAMinimizer {
private:
    // 状态等价类
    using StateSet = std::set<int>;
    using Partition = std::vector<StateSet>;
    
    const DFA* originalDFA;
    Partition currentPartition;
    std::map<char, std::set<char>> alphabet;
    
public:
    explicit DFAMinimizer(const DFA& dfa);
    ~DFAMinimizer() = default;
    
    /**
     * @brief 执行DFA最小化
     * @return 最小化后的DFA
     */
    DFA minimize();
    
    /**
     * @brief 获取最小化统计信息
     */
    struct MinimizationStats {
        size_t originalStates;
        size_t minimizedStates;
        size_t removedStates;
        double reductionRatio;
    };
    
    MinimizationStats getLastMinimizationStats() const;
    
private:
    // 初始化分割
    void initializePartition();
    
    // 细分分割
    bool refinePartition();
    
    // 检查两个状态是否可以区分
    bool areDistinguishable(int state1, int state2) const;
    
    // 获取状态在输入字符下的转换目标
    int getTransition(int state, char input) const;
    
    // 从最小化的分割构建新的DFA
    DFA buildMinimizedDFA() const;
    
    // 查找状态所属的等价类
    int findEquivalenceClass(int state) const;
    
    // 提取字母表
    void extractAlphabet();
    
    // 打印分割状态（调试用）
    void printPartition() const;
    
    // 验证最小化结果
    bool validateMinimization(const DFA& minimizedDFA) const;
    
    MinimizationStats stats;
};

/**
 * @brief DFA优化工具类
 */
class DFAOptimizer {
public:
    /**
     * @brief 对DFA进行完整优化
     * 包括状态最小化、无用状态移除等
     */
    static DFA optimize(const DFA& dfa);
    
    /**
     * @brief 移除不可达状态
     */
    static DFA removeUnreachableStates(const DFA& dfa);
    
    /**
     * @brief 移除死状态
     */
    static DFA removeDeadStates(const DFA& dfa);
    
    /**
     * @brief 合并等价状态
     */
    static DFA mergeEquivalentStates(const DFA& dfa);
    
    /**
     * @brief 优化状态转换表
     */
    static DFA optimizeTransitionTable(const DFA& dfa);
    
private:
    // 深度优先搜索查找可达状态
    static void dfsReachable(const DFA& dfa, int state, std::set<int>& visited);
    
    // 反向深度优先搜索查找有用状态
    static void dfsUseful(const DFA& dfa, int state, std::set<int>& visited);
};

/**
 * @brief 状态等价性分析器
 */
class StateEquivalenceAnalyzer {
public:
    explicit StateEquivalenceAnalyzer(const DFA& dfa);
    
    /**
     * @brief 分析状态等价性
     * @return 等价状态组
     */
    std::vector<std::set<int>> analyzeEquivalence();
    
    /**
     * @brief 检查两个状态是否等价
     */
    bool areEquivalent(int state1, int state2) const;
    
    /**
     * @brief 生成状态等价性矩阵
     */
    std::vector<std::vector<bool>> generateEquivalenceMatrix();
    
private:
    const DFA* dfa;
    std::vector<std::vector<bool>> equivalenceTable;
    
    // 构建等价性表
    void buildEquivalenceTable();
    
    // 标记不等价的状态对
    void markDistinguishablePairs();
    
    // 迭代细化等价性表
    void refineEquivalenceTable();
};

#endif // MINIMIZER_H 