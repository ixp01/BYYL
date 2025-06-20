#include "grammar.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>

// Symbol类实现
Symbol::Symbol(const std::string& n, GrammarSymbolType t, int symbolId, TokenType tType)
    : name(n), type(t), tokenType(tType), id(symbolId) {}

bool Symbol::isTerminal() const {
    return type == GrammarSymbolType::TERMINAL;
}

bool Symbol::isNonTerminal() const {
    return type == GrammarSymbolType::NON_TERMINAL;
}

bool Symbol::isEpsilon() const {
    return type == GrammarSymbolType::EPSILON;
}

std::string Symbol::toString() const {
    switch (type) {
        case GrammarSymbolType::TERMINAL:
            return name;
        case GrammarSymbolType::NON_TERMINAL:
            return name;
        case GrammarSymbolType::EPSILON:
            return "ε";
        default:
            return "UNKNOWN";
    }
}

bool Symbol::operator==(const Symbol& other) const {
    return name == other.name && type == other.type;
}

bool Symbol::operator!=(const Symbol& other) const {
    return !(*this == other);
}

bool Symbol::operator<(const Symbol& other) const {
    if (type != other.type) {
        return type < other.type;
    }
    return name < other.name;
}

// Production类实现
Production::Production(int prodId, const Symbol& lhs, const std::vector<Symbol>& rhs)
    : id(prodId), left(lhs), right(rhs), precedence(0) {}

bool Production::isEpsilonProduction() const {
    return right.size() == 1 && right[0].isEpsilon();
}

size_t Production::getRightSize() const {
    return right.size();
}

Symbol Production::getRightSymbol(size_t index) const {
    if (index < right.size()) {
        return right[index];
    }
    return Symbol(); // 返回默认符号
}

std::string Production::toString() const {
    std::ostringstream oss;
    oss << left.toString() << " -> ";
    
    if (right.empty() || isEpsilonProduction()) {
        oss << "ε";
    } else {
        for (size_t i = 0; i < right.size(); ++i) {
            if (i > 0) oss << " ";
            oss << right[i].toString();
        }
    }
    
    return oss.str();
}

bool Production::operator==(const Production& other) const {
    return id == other.id && left == other.left && right == other.right;
}

// LRItem类实现
LRItem::LRItem(int prodId, size_t dot, const std::set<Symbol>& look)
    : productionId(prodId), dotPosition(dot), lookahead(look) {}

Symbol LRItem::getNextSymbol(const Production& prod) const {
    if (dotPosition < prod.right.size()) {
        return prod.right[dotPosition];
    }
    return Symbol(); // 返回默认符号表示没有下一个符号
}

bool LRItem::isComplete(const Production& prod) const {
    return dotPosition >= prod.right.size() || 
           (prod.right.size() == 1 && prod.right[0].isEpsilon());
}

LRItem LRItem::advance() const {
    return LRItem(productionId, dotPosition + 1, lookahead);
}

std::string LRItem::toString(const Production& prod) const {
    std::ostringstream oss;
    oss << prod.left.toString() << " -> ";
    
    for (size_t i = 0; i < prod.right.size(); ++i) {
        if (i == dotPosition) {
            oss << "• ";
        }
        oss << prod.right[i].toString();
        if (i < prod.right.size() - 1) {
            oss << " ";
        }
    }
    
    if (dotPosition >= prod.right.size()) {
        oss << " •";
    }
    
    if (!lookahead.empty()) {
        oss << " [";
        bool first = true;
        for (const auto& sym : lookahead) {
            if (!first) oss << ", ";
            oss << sym.toString();
            first = false;
        }
        oss << "]";
    }
    
    return oss.str();
}

bool LRItem::operator==(const LRItem& other) const {
    return productionId == other.productionId && 
           dotPosition == other.dotPosition && 
           lookahead == other.lookahead;
}

bool LRItem::operator<(const LRItem& other) const {
    if (productionId != other.productionId) {
        return productionId < other.productionId;
    }
    if (dotPosition != other.dotPosition) {
        return dotPosition < other.dotPosition;
    }
    return lookahead < other.lookahead;
}

// LRItemSet类实现
void LRItemSet::addItem(const LRItem& item) {
    items.insert(item);
}

bool LRItemSet::hasItem(const LRItem& item) const {
    return items.find(item) != items.end();
}

size_t LRItemSet::size() const {
    return items.size();
}

bool LRItemSet::empty() const {
    return items.empty();
}

std::string LRItemSet::toString() const {
    std::ostringstream oss;
    oss << "I" << id << ":\n";
    for (const auto& item : items) {
        oss << "  " << item.productionId << ": ";
        // 注意：这里需要Production信息，在实际使用中需要传入Grammar引用
        oss << "\n";
    }
    return oss.str();
}

bool LRItemSet::operator==(const LRItemSet& other) const {
    return items == other.items;
}

// Grammar类实现
Grammar::Grammar() : nextSymbolId(0), nextProductionId(0) {
    // 初始化特殊符号
    epsilonSymbol = Symbol("ε", GrammarSymbolType::EPSILON, nextSymbolId++);
    endSymbol = Symbol("$", GrammarSymbolType::TERMINAL, nextSymbolId++, TokenType::END_OF_FILE);
    
    symbols["ε"] = epsilonSymbol;
    symbols["$"] = endSymbol;
    terminals.insert(endSymbol);
}

Symbol Grammar::addTerminal(const std::string& name, TokenType tokenType) {
    if (hasSymbol(name)) {
        return getSymbol(name);
    }
    
    Symbol symbol(name, GrammarSymbolType::TERMINAL, nextSymbolId++, tokenType);
    symbols[name] = symbol;
    terminals.insert(symbol);
    return symbol;
}

Symbol Grammar::addNonTerminal(const std::string& name) {
    if (hasSymbol(name)) {
        return getSymbol(name);
    }
    
    Symbol symbol(name, GrammarSymbolType::NON_TERMINAL, nextSymbolId++);
    symbols[name] = symbol;
    nonTerminals.insert(symbol);
    return symbol;
}

Symbol Grammar::getSymbol(const std::string& name) const {
    auto it = symbols.find(name);
    return (it != symbols.end()) ? it->second : Symbol();
}

bool Grammar::hasSymbol(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

int Grammar::addProduction(const Symbol& left, const std::vector<Symbol>& right) {
    Production prod(nextProductionId++, left, right);
    productions.push_back(prod);
    return prod.id;
}

int Grammar::addProduction(const std::string& left, const std::vector<std::string>& right) {
    Symbol leftSym = getSymbol(left);
    if (leftSym.name.empty()) {
        leftSym = addNonTerminal(left);
    }
    
    std::vector<Symbol> rightSyms;
    for (const auto& name : right) {
        Symbol sym = getSymbol(name);
        if (sym.name.empty()) {
            // 默认创建为非终结符，后续可以转换
            sym = addNonTerminal(name);
        }
        rightSyms.push_back(sym);
    }
    
    return addProduction(leftSym, rightSyms);
}

const Production& Grammar::getProduction(int id) const {
    for (const auto& prod : productions) {
        if (prod.id == id) {
            return prod;
        }
    }
    static Production empty;
    return empty;
}

const std::vector<Production>& Grammar::getProductions() const {
    return productions;
}

std::vector<int> Grammar::getProductionsForSymbol(const Symbol& symbol) const {
    std::vector<int> result;
    for (const auto& prod : productions) {
        if (prod.left == symbol) {
            result.push_back(prod.id);
        }
    }
    return result;
}

void Grammar::setStartSymbol(const Symbol& symbol) {
    startSymbol = symbol;
}

void Grammar::setStartSymbol(const std::string& name) {
    Symbol sym = getSymbol(name);
    if (!sym.name.empty()) {
        startSymbol = sym;
    }
}

Symbol Grammar::getStartSymbol() const {
    return startSymbol;
}

Symbol Grammar::getEndSymbol() const {
    return endSymbol;
}

Symbol Grammar::getEpsilonSymbol() const {
    return epsilonSymbol;
}

const std::set<Symbol>& Grammar::getTerminals() const {
    return terminals;
}

const std::set<Symbol>& Grammar::getNonTerminals() const {
    return nonTerminals;
}

std::set<Symbol> Grammar::getAllSymbols() const {
    std::set<Symbol> allSymbols = terminals;
    allSymbols.insert(nonTerminals.begin(), nonTerminals.end());
    return allSymbols;
}

void Grammar::computeFirstSets() {
    firstSets.clear();
    
    // 初始化终结符的FIRST集
    for (const auto& terminal : terminals) {
        firstSets[terminal].insert(terminal);
    }
    
    // ε的FIRST集
    firstSets[epsilonSymbol].insert(epsilonSymbol);
    
    // 迭代计算非终结符的FIRST集
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& prod : productions) {
            std::set<Symbol> newFirstSet;
            std::set<Symbol> visiting;
            computeFirstSet(prod.left, newFirstSet, visiting);
            
            if (newFirstSet != firstSets[prod.left]) {
                firstSets[prod.left] = newFirstSet;
                changed = true;
            }
        }
    }
}

void Grammar::computeFollowSets() {
    followSets.clear();
    
    // 开始符号的FOLLOW集包含$
    followSets[startSymbol].insert(endSymbol);
    
    // 迭代计算FOLLOW集
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (const auto& prod : productions) {
            for (size_t i = 0; i < prod.right.size(); ++i) {
                const Symbol& currentSym = prod.right[i];
                
                if (currentSym.isNonTerminal()) {
                    std::set<Symbol> newFollowSet = followSets[currentSym];
                    
                    // 计算β的FIRST集
                    std::vector<Symbol> beta(prod.right.begin() + i + 1, prod.right.end());
                    std::set<Symbol> betaFirst = getFirstSet(beta);
                    
                    // 添加FIRST(β) - {ε}
                    for (const auto& sym : betaFirst) {
                        if (!sym.isEpsilon()) {
                            newFollowSet.insert(sym);
                        }
                    }
                    
                    // 如果ε ∈ FIRST(β)，添加FOLLOW(A)
                    if (betaFirst.count(epsilonSymbol) > 0) {
                        const auto& followA = followSets[prod.left];
                        newFollowSet.insert(followA.begin(), followA.end());
                    }
                    
                    if (newFollowSet != followSets[currentSym]) {
                        followSets[currentSym] = newFollowSet;
                        changed = true;
                    }
                }
            }
        }
    }
}

std::set<Symbol> Grammar::getFirstSet(const Symbol& symbol) const {
    auto it = firstSets.find(symbol);
    return (it != firstSets.end()) ? it->second : std::set<Symbol>();
}

std::set<Symbol> Grammar::getFirstSet(const std::vector<Symbol>& symbols) const {
    std::set<Symbol> result;
    
    for (const auto& symbol : symbols) {
        std::set<Symbol> symbolFirst = getFirstSet(symbol);
        
        // 添加FIRST(symbol) - {ε}
        for (const auto& sym : symbolFirst) {
            if (!sym.isEpsilon()) {
                result.insert(sym);
            }
        }
        
        // 如果ε不在FIRST(symbol)中，停止
        if (symbolFirst.count(epsilonSymbol) == 0) {
            break;
        }
        
        // 如果这是最后一个符号且ε ∈ FIRST(symbol)，添加ε
        if (&symbol == &symbols.back()) {
            result.insert(epsilonSymbol);
        }
    }
    
    // 如果符号串为空，返回{ε}
    if (symbols.empty()) {
        result.insert(epsilonSymbol);
    }
    
    return result;
}

std::set<Symbol> Grammar::getFollowSet(const Symbol& symbol) const {
    auto it = followSets.find(symbol);
    return (it != followSets.end()) ? it->second : std::set<Symbol>();
}

bool Grammar::validate() const {
    auto errors = getValidationErrors();
    return errors.empty();
}

std::vector<std::string> Grammar::getValidationErrors() const {
    std::vector<std::string> errors;
    
    validateSymbolConsistency(errors);
    validateProductionConsistency(errors);
    
    return errors;
}

void Grammar::print() const {
    std::cout << "Grammar:\n";
    std::cout << "Start Symbol: " << startSymbol.toString() << "\n\n";
    
    std::cout << "Productions:\n";
    for (const auto& prod : productions) {
        std::cout << "  " << prod.id << ": " << prod.toString() << "\n";
    }
    
    std::cout << "\nTerminals: ";
    for (const auto& term : terminals) {
        std::cout << term.toString() << " ";
    }
    
    std::cout << "\nNon-terminals: ";
    for (const auto& nonterm : nonTerminals) {
        std::cout << nonterm.toString() << " ";
    }
    std::cout << "\n";
}

void Grammar::printFirstSets() const {
    std::cout << "FIRST Sets:\n";
    for (const auto& pair : firstSets) {
        std::cout << "FIRST(" << pair.first.toString() << ") = { ";
        for (const auto& sym : pair.second) {
            std::cout << sym.toString() << " ";
        }
        std::cout << "}\n";
    }
}

void Grammar::printFollowSets() const {
    std::cout << "FOLLOW Sets:\n";
    for (const auto& pair : followSets) {
        std::cout << "FOLLOW(" << pair.first.toString() << ") = { ";
        for (const auto& sym : pair.second) {
            std::cout << sym.toString() << " ";
        }
        std::cout << "}\n";
    }
}

// 预定义文法构建
Grammar Grammar::buildSimpleExpressionGrammar() {
    Grammar grammar;
    
    // 添加终结符
    grammar.addTerminal("id", TokenType::IDENTIFIER);
    grammar.addTerminal("num", TokenType::NUMBER);
    grammar.addTerminal("+", TokenType::PLUS);
    grammar.addTerminal("*", TokenType::MULTIPLY);
    grammar.addTerminal("(", TokenType::LPAREN);
    grammar.addTerminal(")", TokenType::RPAREN);
    
    // 添加非终结符
    grammar.addNonTerminal("E");
    grammar.addNonTerminal("T");
    grammar.addNonTerminal("F");
    
    // 添加产生式
    // E -> E + T | T
    grammar.addProduction("E", {"E", "+", "T"});
    grammar.addProduction("E", {"T"});
    
    // T -> T * F | F
    grammar.addProduction("T", {"T", "*", "F"});
    grammar.addProduction("T", {"F"});
    
    // F -> (E) | id | num
    grammar.addProduction("F", {"(", "E", ")"});
    grammar.addProduction("F", {"id"});
    grammar.addProduction("F", {"num"});
    
    grammar.setStartSymbol("E");
    
    return grammar;
}

Grammar Grammar::buildTestGrammar() {
    Grammar grammar;
    
    // 简单的LR测试文法
    // S -> A a | b
    // A -> b
    
    grammar.addTerminal("a", TokenType::IDENTIFIER);
    grammar.addTerminal("b", TokenType::IDENTIFIER);
    grammar.addNonTerminal("S");
    grammar.addNonTerminal("A");
    
    grammar.addProduction("S", {"A", "a"});
    grammar.addProduction("S", {"b"});
    grammar.addProduction("A", {"b"});
    
    grammar.setStartSymbol("S");
    
    return grammar;
}

// 私有辅助方法实现
bool Grammar::computeFirstSet(const Symbol& symbol, std::set<Symbol>& result, 
                             std::set<Symbol>& visiting) const {
    if (visiting.count(symbol) > 0) {
        return false; // 检测到循环
    }
    
    visiting.insert(symbol);
    
    if (symbol.isTerminal() || symbol.isEpsilon()) {
        result.insert(symbol);
        visiting.erase(symbol);
        return true;
    }
    
    // 对于非终结符，查看所有产生式
    for (const auto& prod : productions) {
        if (prod.left == symbol) {
            if (prod.isEpsilonProduction()) {
                result.insert(epsilonSymbol);
            } else {
                std::set<Symbol> prodFirst = getFirstSet(prod.right);
                result.insert(prodFirst.begin(), prodFirst.end());
            }
        }
    }
    
    visiting.erase(symbol);
    return true;
}

void Grammar::validateSymbolConsistency(std::vector<std::string>& errors) const {
    // 检查开始符号是否为非终结符
    if (!startSymbol.isNonTerminal()) {
        errors.push_back("Start symbol must be a non-terminal");
    }
    
    // 检查是否有未定义的符号在产生式中使用
    for (const auto& prod : productions) {
        if (symbols.find(prod.left.name) == symbols.end()) {
            errors.push_back("Undefined symbol in production: " + prod.left.name);
        }
        
        for (const auto& sym : prod.right) {
            if (symbols.find(sym.name) == symbols.end()) {
                errors.push_back("Undefined symbol in production: " + sym.name);
            }
        }
    }
}

void Grammar::validateProductionConsistency(std::vector<std::string>& errors) const {
    // 检查是否有产生式的左部为终结符
    for (const auto& prod : productions) {
        if (prod.left.isTerminal()) {
            errors.push_back("Production left-hand side cannot be terminal: " + prod.left.name);
        }
    }
    
    // 检查开始符号是否有产生式
    bool hasStartProduction = false;
    for (const auto& prod : productions) {
        if (prod.left == startSymbol) {
            hasStartProduction = true;
            break;
        }
    }
    
    if (!hasStartProduction && !startSymbol.name.empty()) {
        errors.push_back("No productions found for start symbol: " + startSymbol.name);
    }
}

// GrammarBuilder类实现
GrammarBuilder& GrammarBuilder::terminal(const std::string& name, TokenType type) {
    grammar.addTerminal(name, type);
    return *this;
}

GrammarBuilder& GrammarBuilder::nonTerminal(const std::string& name) {
    grammar.addNonTerminal(name);
    return *this;
}

GrammarBuilder& GrammarBuilder::production(const std::string& left, 
                                          const std::vector<std::string>& right) {
    grammar.addProduction(left, right);
    return *this;
}

GrammarBuilder& GrammarBuilder::startSymbol(const std::string& name) {
    grammar.setStartSymbol(name);
    return *this;
}

Grammar& GrammarBuilder::build() {
    return grammar;
} 