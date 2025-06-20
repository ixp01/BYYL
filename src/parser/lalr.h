#ifndef LALR_H
#define LALR_H

#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "grammar.h"
#include "ast.h"
#include "../lexer/token.h"

/**
 * @brief LALR分析动作类型
 */
enum class LALRActionType {
    SHIFT,      // 移入
    REDUCE,     // 规约
    ACCEPT,     // 接受
    ERROR       // 错误
};

/**
 * @brief LALR分析动作
 */
class LALRAction {
public:
    LALRActionType type;
    int value;              // 移入时为状态号，规约时为产生式号
    
    LALRAction() : type(LALRActionType::ERROR), value(-1) {}
    LALRAction(LALRActionType t, int v = -1) : type(t), value(v) {}
    
    bool isShift() const { return type == LALRActionType::SHIFT; }
    bool isReduce() const { return type == LALRActionType::REDUCE; }
    bool isAccept() const { return type == LALRActionType::ACCEPT; }
    bool isError() const { return type == LALRActionType::ERROR; }
    
    std::string toString() const;
    
    bool operator==(const LALRAction& other) const;
    bool operator!=(const LALRAction& other) const;
};

/**
 * @brief LALR分析表
 */
class LALRTable {
public:
    // ACTION表：状态 × 终结符 -> 动作
    std::map<std::pair<int, Symbol>, LALRAction> actionTable;
    
    // GOTO表：状态 × 非终结符 -> 状态
    std::map<std::pair<int, Symbol>, int> gotoTable;
    
    int numStates;
    
    LALRTable() : numStates(0) {}
    
    // 设置ACTION表项
    void setAction(int state, const Symbol& symbol, const LALRAction& action);
    
    // 设置GOTO表项
    void setGoto(int state, const Symbol& symbol, int nextState);
    
    // 获取ACTION表项
    LALRAction getAction(int state, const Symbol& symbol) const;
    
    // 获取GOTO表项
    int getGoto(int state, const Symbol& symbol) const;
    
    // 检查是否有冲突
    bool hasConflicts() const;
    std::vector<std::string> getConflicts() const;
    
    // 打印分析表
    void print(const Grammar& grammar) const;
    void printAction(const Grammar& grammar) const;
    void printGoto(const Grammar& grammar) const;
    
    // 表的统计信息
    size_t getActionTableSize() const { return actionTable.size(); }
    size_t getGotoTableSize() const { return gotoTable.size(); }
    
private:
    // 检测和记录冲突
    mutable std::vector<std::string> conflicts;
    void detectConflicts() const;
};

/**
 * @brief LALR状态
 */
class LALRState {
public:
    int id;
    LRItemSet itemSet;
    std::map<Symbol, int> transitions;
    
    LALRState() : id(-1) {}
    explicit LALRState(int stateId) : id(stateId), itemSet(stateId) {}
    
    void addTransition(const Symbol& symbol, int nextState);
    int getTransition(const Symbol& symbol) const;
    bool hasTransition(const Symbol& symbol) const;
    
    std::string toString(const Grammar& grammar) const;
    
    bool operator==(const LALRState& other) const;
};

/**
 * @brief LALR自动机
 */
class LALRAutomaton {
private:
    std::vector<LALRState> states;
    const Grammar* grammar;
    int startState;
    
public:
    explicit LALRAutomaton(const Grammar& g) : grammar(&g), startState(0) {}
    
    // 构建LALR自动机
    void build();
    
    // 获取状态
    const LALRState& getState(int id) const;
    LALRState& getState(int id);
    
    // 状态数量
    size_t getStateCount() const { return states.size(); }
    
    // 添加状态
    int addState(const LRItemSet& itemSet);
    
    // 查找状态
    int findState(const LRItemSet& itemSet) const;
    
    // 打印自动机
    void print() const;
    
    // 构建分析表
    LALRTable buildParsingTable() const;
    
private:
    // 计算闭包
    LRItemSet closure(const LRItemSet& itemSet) const;
    
    // 计算GOTO函数
    LRItemSet gotoFunction(const LRItemSet& itemSet, const Symbol& symbol) const;
    
    // 计算LR(0)项目集族
    void buildLR0ItemSets();
    
    // 合并同心项目集（LALR关键步骤）
    void mergeLALRStates();
    
    // 计算前瞻符号
    void computeLookaheads();
    
    // 传播前瞻符号
    void propagateLookaheads();
    
    // 自发生成前瞻符号
    std::set<Symbol> spontaneousLookaheads(const LRItem& item) const;
    
    // 检查两个项目集的核心是否相同
    bool haveSameCore(const LRItemSet& set1, const LRItemSet& set2) const;
    
    // 获取项目集的核心
    std::set<LRItem> getCore(const LRItemSet& itemSet) const;
};

/**
 * @brief LALR分析器
 */
class LALRParser {
private:
    const Grammar* grammar;
    std::unique_ptr<LALRAutomaton> automaton;
    std::unique_ptr<LALRTable> parseTable;
    
    // 分析栈
    std::vector<int> stateStack;
    std::vector<std::unique_ptr<ASTNode>> nodeStack;
    
    // 当前分析状态
    std::vector<Token> tokens;
    size_t tokenIndex;
    
    // 错误处理
    std::vector<std::string> errors;
    bool hasError;
    
public:
    explicit LALRParser(const Grammar& g);
    ~LALRParser() = default;
    
    // 构建分析器
    bool build();
    
    // 检查文法是否为LALR(1)
    bool isLALR1() const;
    
    // 分析输入
    std::unique_ptr<ASTNode> parse(const std::vector<Token>& inputTokens);
    
    // 获取分析表
    const LALRTable* getParsingTable() const { return parseTable.get(); }
    
    // 获取自动机
    const LALRAutomaton* getAutomaton() const { return automaton.get(); }
    
    // 错误处理
    bool hasErrors() const { return hasError; }
    const std::vector<std::string>& getErrors() const { return errors; }
    void clearErrors();
    
    // 调试功能
    void printAutomaton() const;
    void printParsingTable() const;
    
private:
    // 分析步骤
    bool shift(int state);
    bool reduce(int productionId);
    bool accept();
    void reportError(const std::string& message);
    
    // 错误恢复
    bool errorRecovery();
    void synchronize();
    
    // AST构建
    std::unique_ptr<ASTNode> buildASTNode(const Production& production, 
                                          std::vector<std::unique_ptr<ASTNode>>& children);
    
    // Token映射
    Symbol tokenToSymbol(const Token& token) const;
    
    // 获取当前Token
    Token getCurrentToken() const;
    Token peekToken(int offset = 0) const;
    bool isAtEnd() const;
    
    // 分析栈操作
    void pushState(int state);
    int popState();
    int currentState() const;
    void pushNode(std::unique_ptr<ASTNode> node);
    std::unique_ptr<ASTNode> popNode();
    
    // 初始化
    void initializeParsing(const std::vector<Token>& inputTokens);
    void resetParsing();
};

/**
 * @brief LALR分析器构建器
 */
class LALRParserBuilder {
public:
    static std::unique_ptr<LALRParser> build(const Grammar& grammar);
    static bool validateGrammar(const Grammar& grammar, std::vector<std::string>& errors);
    static LALRTable buildParsingTable(const Grammar& grammar);
    
private:
    static bool checkLALRConditions(const Grammar& grammar, std::vector<std::string>& errors);
    static void optimizeParsingTable(LALRTable& table);
};

/**
 * @brief LALR冲突解决器
 */
class LALRConflictResolver {
public:
    struct ConflictInfo {
        int state;
        Symbol symbol;
        LALRAction action1;
        LALRAction action2;
        std::string description;
    };
    
    explicit LALRConflictResolver(const Grammar& g) : grammar(&g) {}
    
    // 检测冲突
    std::vector<ConflictInfo> detectConflicts(const LALRTable& table) const;
    
    // 解决移入/规约冲突
    bool resolveShiftReduceConflict(LALRTable& table, const ConflictInfo& conflict) const;
    
    // 解决规约/规约冲突
    bool resolveReduceReduceConflict(LALRTable& table, const ConflictInfo& conflict) const;
    
    // 自动解决所有可能的冲突
    bool resolveAllConflicts(LALRTable& table) const;
    
private:
    const Grammar* grammar;
    
    // 优先级和结合性规则
    int getOperatorPrecedence(const Symbol& symbol) const;
    enum class Associativity { LEFT, RIGHT, NONE };
    Associativity getOperatorAssociativity(const Symbol& symbol) const;
};

#endif // LALR_H 