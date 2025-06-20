#include "minimizer.h"
#include <iostream>
#include <algorithm>
#include <queue>

// ==================== DFAMinimizer 实现 ====================

DFAMinimizer::DFAMinimizer(const DFA& dfa) : originalDFA(&dfa) {
    extractAlphabet();
    stats.originalStates = dfa.getStateCount();
}

DFA DFAMinimizer::minimize() {
    std::cout << "Starting DFA minimization..." << std::endl;
    std::cout << "Original DFA has " << originalDFA->getStateCount() << " states" << std::endl;
    
    // 初始化分割
    initializePartition();
    
    // 迭代细分直到不能再细分
    bool changed = true;
    int iteration = 0;
    while (changed) {
        std::cout << "Minimization iteration " << ++iteration << std::endl;
        changed = refinePartition();
        if (changed) {
            std::cout << "Partition refined, current groups: " << currentPartition.size() << std::endl;
        }
    }
    
    std::cout << "Minimization completed after " << iteration << " iterations" << std::endl;
    
    // 构建最小化的DFA
    DFA minimizedDFA = buildMinimizedDFA();
    
    // 更新统计信息
    stats.minimizedStates = minimizedDFA.getStateCount();
    stats.removedStates = stats.originalStates - stats.minimizedStates;
    stats.reductionRatio = static_cast<double>(stats.removedStates) / stats.originalStates;
    
    std::cout << "Minimization results:" << std::endl;
    std::cout << "  Original states: " << stats.originalStates << std::endl;
    std::cout << "  Minimized states: " << stats.minimizedStates << std::endl;
    std::cout << "  Removed states: " << stats.removedStates << std::endl;
    std::cout << "  Reduction ratio: " << (stats.reductionRatio * 100) << "%" << std::endl;
    
    return minimizedDFA;
}

DFAMinimizer::MinimizationStats DFAMinimizer::getLastMinimizationStats() const {
    return stats;
}

void DFAMinimizer::initializePartition() {
    // 初始分割：接受状态和非接受状态
    StateSet acceptingStates;
    StateSet nonAcceptingStates;
    
    for (size_t i = 0; i < originalDFA->getStateCount(); ++i) {
        // 注意：这里需要访问DFA的内部状态信息
        // 为了演示，我们假设状态ID连续且从0开始
        // 实际实现中可能需要DFA提供更多接口
        if (i < originalDFA->getStateCount()) {
            nonAcceptingStates.insert(static_cast<int>(i));
        }
    }
    
    // 将接受状态按Token类型进一步分组
    std::map<TokenType, StateSet> acceptingGroups;
    
    // 这里需要DFA提供接口来访问状态信息
    // 为了简化，我们先创建基本的分割
    if (!acceptingStates.empty()) {
        currentPartition.push_back(acceptingStates);
    }
    if (!nonAcceptingStates.empty()) {
        currentPartition.push_back(nonAcceptingStates);
    }
    
    std::cout << "Initial partition created with " << currentPartition.size() << " groups" << std::endl;
}

bool DFAMinimizer::refinePartition() {
    Partition newPartition;
    bool changed = false;
    
    for (const auto& group : currentPartition) {
        if (group.size() <= 1) {
            // 单个状态的组不需要细分
            newPartition.push_back(group);
            continue;
        }
        
        // 尝试细分当前组
        std::map<std::vector<int>, StateSet> subgroups;
        
        for (int state : group) {
            // 为每个状态创建签名（基于转换行为）
            std::vector<int> signature;
            
            for (const auto& charGroup : alphabet) {
                for (char c : charGroup.second) {
                    int targetState = getTransition(state, c);
                    int targetGroup = findEquivalenceClass(targetState);
                    signature.push_back(targetGroup);
                    break; // 只需要每个字符类的一个代表
                }
            }
            
            subgroups[signature].insert(state);
        }
        
        // 如果组被细分了
        if (subgroups.size() > 1) {
            changed = true;
            for (const auto& subgroup : subgroups) {
                newPartition.push_back(subgroup.second);
            }
        } else {
            newPartition.push_back(group);
        }
    }
    
    currentPartition = newPartition;
    return changed;
}

bool DFAMinimizer::areDistinguishable(int state1, int state2) const {
    // 检查两个状态是否可以通过某个输入字符串区分
    
    // 如果一个是接受状态，另一个不是，则可以区分
    // 这里需要DFA提供接口来检查状态类型
    
    // 检查在所有输入字符下的转换行为
    for (const auto& charGroup : alphabet) {
        for (char c : charGroup.second) {
            int next1 = getTransition(state1, c);
            int next2 = getTransition(state2, c);
            
            if (next1 != next2) {
                // 递归检查目标状态
                if (next1 == -1 || next2 == -1) {
                    return true; // 一个有转换，另一个没有
                }
                // 这里应该递归检查，但为了避免无限递归，我们简化处理
            }
            break; // 只检查每个字符类的代表
        }
    }
    
    return false;
}

int DFAMinimizer::getTransition(int state, char input) const {
    // 这里需要DFA提供接口来获取状态转换
    // 为了演示，我们返回一个占位值
    // 实际实现中需要调用DFA的方法
    return -1; // 表示无转换
}

DFA DFAMinimizer::buildMinimizedDFA() const {
    DFA minimizedDFA;
    
    // 为每个等价类创建一个新状态
    std::map<int, int> groupToNewState;
    
    for (size_t i = 0; i < currentPartition.size(); ++i) {
        int newStateId = minimizedDFA.addState();
        groupToNewState[static_cast<int>(i)] = newStateId;
        
        // 检查这个组是否包含原始的起始状态
        if (currentPartition[i].count(0) > 0) { // 假设起始状态ID为0
            minimizedDFA.setStartState(newStateId);
        }
    }
    
    // 构建新的转换表
    for (size_t groupIdx = 0; groupIdx < currentPartition.size(); ++groupIdx) {
        const StateSet& group = currentPartition[groupIdx];
        int representative = *group.begin(); // 选择组中的代表状态
        int newStateId = groupToNewState[static_cast<int>(groupIdx)];
        
        // 为代表状态的所有转换创建新转换
        for (const auto& charGroup : alphabet) {
            for (char c : charGroup.second) {
                int targetState = getTransition(representative, c);
                if (targetState != -1) {
                    int targetGroup = findEquivalenceClass(targetState);
                    int newTargetState = groupToNewState[targetGroup];
                    minimizedDFA.addTransition(newStateId, c, newTargetState);
                }
                break; // 只处理每个字符类的代表
            }
        }
    }
    
    return minimizedDFA;
}

int DFAMinimizer::findEquivalenceClass(int state) const {
    for (size_t i = 0; i < currentPartition.size(); ++i) {
        if (currentPartition[i].count(state) > 0) {
            return static_cast<int>(i);
        }
    }
    return -1; // 状态不在任何组中
}

void DFAMinimizer::extractAlphabet() {
    // 提取DFA使用的字母表
    // 这里需要DFA提供接口来获取所有使用的字符
    // 为了演示，我们创建一个基本的字母表
    
    std::set<char> letters;
    for (char c = 'a'; c <= 'z'; ++c) letters.insert(c);
    for (char c = 'A'; c <= 'Z'; ++c) letters.insert(c);
    alphabet['L'] = letters;
    
    std::set<char> digits;
    for (char c = '0'; c <= '9'; ++c) digits.insert(c);
    alphabet['D'] = digits;
    
    std::set<char> operators = {'+', '-', '*', '/', '%', '=', '!', '<', '>', '&', '|'};
    alphabet['O'] = operators;
    
    std::set<char> delimiters = {'(', ')', '{', '}', '[', ']', ';', ',', '.'};
    alphabet['P'] = delimiters;
}

void DFAMinimizer::printPartition() const {
    std::cout << "Current partition:" << std::endl;
    for (size_t i = 0; i < currentPartition.size(); ++i) {
        std::cout << "Group " << i << ": {";
        bool first = true;
        for (int state : currentPartition[i]) {
            if (!first) std::cout << ", ";
            std::cout << state;
            first = false;
        }
        std::cout << "}" << std::endl;
    }
}

bool DFAMinimizer::validateMinimization(const DFA& minimizedDFA) const {
    // 验证最小化的DFA是否正确
    // 这里可以添加各种验证逻辑
    return minimizedDFA.validate();
}

// ==================== DFAOptimizer 实现 ====================

DFA DFAOptimizer::optimize(const DFA& dfa) {
    std::cout << "Starting DFA optimization..." << std::endl;
    
    // 1. 移除不可达状态
    DFA step1 = removeUnreachableStates(dfa);
    std::cout << "After removing unreachable states: " << step1.getStateCount() << " states" << std::endl;
    
    // 2. 移除死状态
    DFA step2 = removeDeadStates(step1);
    std::cout << "After removing dead states: " << step2.getStateCount() << " states" << std::endl;
    
    // 3. 最小化状态
    DFAMinimizer minimizer(step2);
    DFA step3 = minimizer.minimize();
    std::cout << "After minimization: " << step3.getStateCount() << " states" << std::endl;
    
    // 4. 优化转换表
    DFA optimized = optimizeTransitionTable(step3);
    std::cout << "After transition table optimization: " << optimized.getStateCount() << " states" << std::endl;
    
    std::cout << "DFA optimization completed!" << std::endl;
    return optimized;
}

DFA DFAOptimizer::removeUnreachableStates(const DFA& dfa) {
    std::set<int> reachableStates;
    
    // 从起始状态开始DFS
    dfsReachable(dfa, 0, reachableStates); // 假设起始状态ID为0
    
    // 构建只包含可达状态的新DFA
    DFA newDFA;
    std::map<int, int> oldToNew;
    
    // 创建新状态
    for (int oldState : reachableStates) {
        int newState = newDFA.addState();
        oldToNew[oldState] = newState;
        
        if (oldState == 0) { // 假设0是起始状态
            newDFA.setStartState(newState);
        }
    }
    
    // 复制转换关系
    // 这里需要DFA提供接口来遍历转换
    
    return newDFA;
}

DFA DFAOptimizer::removeDeadStates(const DFA& dfa) {
    std::set<int> usefulStates;
    
    // 从所有接受状态开始反向DFS
    for (size_t i = 0; i < dfa.getStateCount(); ++i) {
        // 如果是接受状态，则从它开始反向搜索
        // 这里需要DFA提供接口来判断是否为接受状态
        dfsUseful(dfa, static_cast<int>(i), usefulStates);
    }
    
    // 构建只包含有用状态的新DFA
    // 实现类似于removeUnreachableStates
    return dfa; // 占位返回
}

DFA DFAOptimizer::mergeEquivalentStates(const DFA& dfa) {
    // 使用状态等价性分析器
    StateEquivalenceAnalyzer analyzer(dfa);
    auto equivalenceGroups = analyzer.analyzeEquivalence();
    
    // 基于等价组构建新DFA
    DFA newDFA;
    // 实现合并逻辑
    
    return newDFA;
}

DFA DFAOptimizer::optimizeTransitionTable(const DFA& dfa) {
    // 优化转换表，例如压缩稀疏转换
    return dfa; // 暂时返回原DFA
}

void DFAOptimizer::dfsReachable(const DFA& dfa, int state, std::set<int>& visited) {
    if (visited.count(state) > 0) {
        return;
    }
    
    visited.insert(state);
    
    // 遍历所有可能的转换
    // 这里需要DFA提供接口来获取状态的所有转换
}

void DFAOptimizer::dfsUseful(const DFA& dfa, int state, std::set<int>& visited) {
    if (visited.count(state) > 0) {
        return;
    }
    
    visited.insert(state);
    
    // 反向遍历转换
    // 这里需要DFA提供反向转换的接口
}

// ==================== StateEquivalenceAnalyzer 实现 ====================

StateEquivalenceAnalyzer::StateEquivalenceAnalyzer(const DFA& dfa) : dfa(&dfa) {
    size_t stateCount = dfa.getStateCount();
    equivalenceTable.resize(stateCount, std::vector<bool>(stateCount, true));
    buildEquivalenceTable();
}

std::vector<std::set<int>> StateEquivalenceAnalyzer::analyzeEquivalence() {
    std::vector<std::set<int>> equivalenceGroups;
    std::vector<bool> processed(dfa->getStateCount(), false);
    
    for (size_t i = 0; i < dfa->getStateCount(); ++i) {
        if (processed[i]) continue;
        
        std::set<int> group;
        group.insert(static_cast<int>(i));
        processed[i] = true;
        
        // 查找与状态i等价的所有状态
        for (size_t j = i + 1; j < dfa->getStateCount(); ++j) {
            if (!processed[j] && equivalenceTable[i][j]) {
                group.insert(static_cast<int>(j));
                processed[j] = true;
            }
        }
        
        equivalenceGroups.push_back(group);
    }
    
    return equivalenceGroups;
}

bool StateEquivalenceAnalyzer::areEquivalent(int state1, int state2) const {
    if (state1 >= 0 && state1 < static_cast<int>(equivalenceTable.size()) &&
        state2 >= 0 && state2 < static_cast<int>(equivalenceTable.size())) {
        return equivalenceTable[state1][state2];
    }
    return false;
}

std::vector<std::vector<bool>> StateEquivalenceAnalyzer::generateEquivalenceMatrix() {
    return equivalenceTable;
}

void StateEquivalenceAnalyzer::buildEquivalenceTable() {
    // 初始化：所有状态对都假设等价
    
    // 标记明显不等价的状态对
    markDistinguishablePairs();
    
    // 迭代细化
    refineEquivalenceTable();
}

void StateEquivalenceAnalyzer::markDistinguishablePairs() {
    size_t stateCount = dfa->getStateCount();
    
    for (size_t i = 0; i < stateCount; ++i) {
        for (size_t j = i + 1; j < stateCount; ++j) {
            // 如果一个是接受状态，另一个不是，则不等价
            // 这里需要DFA提供接口来判断状态类型
            
            // 如果接受不同类型的Token，也不等价
            // 这里需要DFA提供Token类型信息
            
            // 暂时标记为等价，后续在refineEquivalenceTable中细化
        }
    }
}

void StateEquivalenceAnalyzer::refineEquivalenceTable() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        size_t stateCount = dfa->getStateCount();
        
        for (size_t i = 0; i < stateCount; ++i) {
            for (size_t j = i + 1; j < stateCount; ++j) {
                if (!equivalenceTable[i][j]) continue;
                
                // 检查在所有输入下的转换行为
                // 如果转换到不等价的状态，则这两个状态也不等价
                
                // 这里需要DFA提供转换信息的接口
                // 为了演示，我们暂时跳过具体实现
            }
        }
    }
} 