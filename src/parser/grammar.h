#ifndef GRAMMAR_H
#define GRAMMAR_H

#include <vector>
#include <set>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "../lexer/token.h"

/**
 * @brief 文法符号类型枚举
 */
enum class GrammarSymbolType {
    TERMINAL,       // 终结符
    NON_TERMINAL,   // 非终结符
    EPSILON         // 空符号ε
};

/**
 * @brief 文法符号类
 */
class Symbol {
public:
    std::string name;           // 符号名称
    GrammarSymbolType type;     // 符号类型
    TokenType tokenType;        // 对应的Token类型（仅终结符有效）
    int id;                     // 符号唯一ID
    
    Symbol() = default;
    Symbol(const std::string& n, GrammarSymbolType t, int symbolId = -1, 
           TokenType tType = TokenType::UNKNOWN);
    
    bool isTerminal() const;
    bool isNonTerminal() const;
    bool isEpsilon() const;
    
    std::string toString() const;
    
    // 比较运算符
    bool operator==(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;
    bool operator<(const Symbol& other) const;
};

/**
 * @brief 产生式类
 */
class Production {
public:
    int id;                     // 产生式ID
    Symbol left;                // 左部非终结符
    std::vector<Symbol> right;  // 右部符号串
    int precedence;             // 优先级
    std::string action;         // 语义动作（可选）
    
    Production() : id(-1), precedence(0) {}
    Production(int prodId, const Symbol& lhs, const std::vector<Symbol>& rhs);
    
    bool isEpsilonProduction() const;
    size_t getRightSize() const;
    Symbol getRightSymbol(size_t index) const;
    
    std::string toString() const;
    
    // 比较运算符
    bool operator==(const Production& other) const;
};

/**
 * @brief LR项目类
 */
class LRItem {
public:
    int productionId;           // 产生式ID
    size_t dotPosition;         // 点号位置
    std::set<Symbol> lookahead; // 前瞻符号集
    
    LRItem() : productionId(-1), dotPosition(0) {}
    LRItem(int prodId, size_t dot, const std::set<Symbol>& look = {});
    
    Symbol getNextSymbol(const Production& prod) const;
    bool isComplete(const Production& prod) const;
    LRItem advance() const;
    
    std::string toString(const Production& prod) const;
    
    // 比较运算符
    bool operator==(const LRItem& other) const;
    bool operator<(const LRItem& other) const;
};

/**
 * @brief LR项目集类
 */
class LRItemSet {
public:
    int id;                     // 项目集ID
    std::set<LRItem> items;     // 项目集合
    std::map<Symbol, int> transitions; // 转换表：符号 -> 下一状态ID
    
    LRItemSet() : id(-1) {}
    explicit LRItemSet(int setId) : id(setId) {}
    
    void addItem(const LRItem& item);
    bool hasItem(const LRItem& item) const;
    size_t size() const;
    bool empty() const;
    
    std::string toString() const;
    
    // 比较运算符
    bool operator==(const LRItemSet& other) const;
};

/**
 * @brief 上下文无关文法类
 */
class Grammar {
private:
    std::vector<Production> productions;        // 产生式集合
    std::map<std::string, Symbol> symbols;      // 符号表
    std::set<Symbol> terminals;                 // 终结符集合
    std::set<Symbol> nonTerminals;              // 非终结符集合
    Symbol startSymbol;                         // 开始符号
    Symbol endSymbol;                           // 结束符号$
    Symbol epsilonSymbol;                       // 空符号ε
    
    // 文法分析结果
    std::map<Symbol, std::set<Symbol>> firstSets;  // FIRST集
    std::map<Symbol, std::set<Symbol>> followSets; // FOLLOW集
    
    int nextSymbolId;                           // 下一个符号ID
    int nextProductionId;                       // 下一个产生式ID

public:
    Grammar();
    ~Grammar() = default;
    
    // 符号管理
    Symbol addTerminal(const std::string& name, TokenType tokenType = TokenType::UNKNOWN);
    Symbol addNonTerminal(const std::string& name);
    Symbol getSymbol(const std::string& name) const;
    bool hasSymbol(const std::string& name) const;
    
    // 产生式管理
    int addProduction(const Symbol& left, const std::vector<Symbol>& right);
    int addProduction(const std::string& left, const std::vector<std::string>& right);
    const Production& getProduction(int id) const;
    const std::vector<Production>& getProductions() const;
    std::vector<int> getProductionsForSymbol(const Symbol& symbol) const;
    
    // 文法设置
    void setStartSymbol(const Symbol& symbol);
    void setStartSymbol(const std::string& name);
    Symbol getStartSymbol() const;
    Symbol getEndSymbol() const;
    Symbol getEpsilonSymbol() const;
    
    // 集合访问
    const std::set<Symbol>& getTerminals() const;
    const std::set<Symbol>& getNonTerminals() const;
    std::set<Symbol> getAllSymbols() const;
    
    // FIRST和FOLLOW集计算
    void computeFirstSets();
    void computeFollowSets();
    std::set<Symbol> getFirstSet(const Symbol& symbol) const;
    std::set<Symbol> getFirstSet(const std::vector<Symbol>& symbols) const;
    std::set<Symbol> getFollowSet(const Symbol& symbol) const;
    
    // 文法验证
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;
    
    // 文法分析
    bool isLL1() const;
    bool isLR0() const;
    bool isLR1() const;
    bool isSLR1() const;
    bool isLALR1() const;
    
    // 工具方法
    void print() const;
    void printFirstSets() const;
    void printFollowSets() const;
    std::string toString() const;
    
    // 预定义文法构建
    static Grammar buildSimpleExpressionGrammar();
    static Grammar buildStandardCGrammar();
    static Grammar buildTestGrammar();
    
private:
    // 内部辅助方法
    bool computeFirstSet(const Symbol& symbol, std::set<Symbol>& result, 
                        std::set<Symbol>& visiting) const;
    bool computeFollowSet(const Symbol& symbol, std::set<Symbol>& result, 
                         std::set<Symbol>& visiting) const;
    void validateSymbolConsistency(std::vector<std::string>& errors) const;
    void validateProductionConsistency(std::vector<std::string>& errors) const;
};

/**
 * @brief 文法构建器辅助类
 */
class GrammarBuilder {
public:
    explicit GrammarBuilder(Grammar& g) : grammar(g) {}
    
    // 流式接口
    GrammarBuilder& terminal(const std::string& name, TokenType type = TokenType::UNKNOWN);
    GrammarBuilder& nonTerminal(const std::string& name);
    GrammarBuilder& production(const std::string& left, const std::vector<std::string>& right);
    GrammarBuilder& startSymbol(const std::string& name);
    
    Grammar& build();
    
private:
    Grammar& grammar;
};

#endif // GRAMMAR_H 