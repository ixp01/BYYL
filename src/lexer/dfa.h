#ifndef DFA_H
#define DFA_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include <unordered_map>
#include "token.h"

/**
 * @brief DFA状态类型
 */
enum class DFAStateType {
    NORMAL,     // 普通状态
    ACCEPTING,  // 接受状态
    ERROR       // 错误状态
};

/**
 * @brief DFA状态类
 */
class DFAState {
public:
    int id;                              // 状态ID
    DFAStateType type;                   // 状态类型
    TokenType tokenType;                 // 如果是接受状态，对应的Token类型
    std::map<char, int> transitions;     // 状态转换表：字符 -> 下一状态ID
    
    DFAState(int id, DFAStateType type = DFAStateType::NORMAL, 
             TokenType tokenType = TokenType::UNKNOWN);
    
    // 添加转换
    void addTransition(char c, int nextStateId);
    
    // 添加字符范围转换
    void addRangeTransition(char start, char end, int nextStateId);
    
    // 获取下一状态
    int getNextState(char c) const;
    
    // 检查是否是接受状态
    bool isAccepting() const;
    
    // 获取状态信息字符串
    std::string toString() const;
};

/**
 * @brief 确定性有限自动机(DFA)类
 */
class DFA {
private:
    std::vector<DFAState> states;        // 所有状态
    int startState;                      // 起始状态ID
    int currentState;                    // 当前状态ID
    std::set<int> acceptingStates;       // 接受状态集合
    
    // 字符分类映射
    std::unordered_map<char, char> charClassMap;
    
    // 构建字符分类
    void buildCharacterClasses();
    
    // 获取字符类别
    char getCharClass(char c) const;

public:
    DFA();
    ~DFA() = default;
    
    // 构建预定义的DFA
    void buildStandardDFA();
    
    // 添加状态
    int addState(DFAStateType type = DFAStateType::NORMAL, 
                 TokenType tokenType = TokenType::UNKNOWN);
    
    // 设置起始状态
    void setStartState(int stateId);
    
    // 添加转换
    void addTransition(int fromState, char c, int toState);
    
    // 添加字符范围转换
    void addRangeTransition(int fromState, char start, char end, int toState);
    
    // 添加字符串转换（用于关键字）
    void addStringTransition(int fromState, const std::string& str, int toState);
    
    // 重置到起始状态
    void reset();
    
    // 处理单个字符
    bool processChar(char c);
    
    // 检查当前是否为接受状态
    bool isInAcceptingState() const;
    
    // 获取当前状态对应的Token类型
    TokenType getCurrentTokenType() const;
    
    // 获取当前状态ID
    int getCurrentState() const;
    
    // 获取状态数量
    size_t getStateCount() const;
    
    // 打印DFA结构（调试用）
    void printDFA() const;
    
    // 验证DFA的完整性
    bool validate() const;
    
    // 从字符串识别Token
    std::pair<TokenType, std::string> recognizeToken(const std::string& input) const;
    
private:
    // 构建标识符识别的DFA部分
    void buildIdentifierDFA();
    
    // 构建数字识别的DFA部分
    void buildNumberDFA();
    
    // 构建运算符识别的DFA部分
    void buildOperatorDFA();
    
    // 构建分隔符识别的DFA部分
    void buildDelimiterDFA();
    
    // 构建字符串字面量识别的DFA部分
    void buildStringLiteralDFA();
    
    // 构建注释识别的DFA部分
    void buildCommentDFA();
};

/**
 * @brief DFA构建器辅助类
 */
class DFABuilder {
public:
    static DFA buildLexerDFA();
    static void addKeywordStates(DFA& dfa, int startState);
    static void optimizeDFA(DFA& dfa);
};

#endif // DFA_H 