#include "lalr.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <queue>
#include <iomanip>

// LALRAction类实现
std::string LALRAction::toString() const {
    switch (type) {
        case LALRActionType::SHIFT:
            return "s" + std::to_string(value);
        case LALRActionType::REDUCE:
            return "r" + std::to_string(value);
        case LALRActionType::ACCEPT:
            return "acc";
        case LALRActionType::ERROR:
            return "err";
        default:
            return "unknown";
    }
}

bool LALRAction::operator==(const LALRAction& other) const {
    return type == other.type && value == other.value;
}

bool LALRAction::operator!=(const LALRAction& other) const {
    return !(*this == other);
}

// LALRTable类实现
void LALRTable::setAction(int state, const Symbol& symbol, const LALRAction& action) {
    auto key = std::make_pair(state, symbol);
    
    // 检查是否已存在不同的动作（冲突检测）
    auto it = actionTable.find(key);
    if (it != actionTable.end() && it->second != action) {
        std::ostringstream oss;
        oss << "Action conflict at state " << state << " on symbol " << symbol.toString()
            << ": " << it->second.toString() << " vs " << action.toString();
        conflicts.push_back(oss.str());
    }
    
    actionTable[key] = action;
}

void LALRTable::setGoto(int state, const Symbol& symbol, int nextState) {
    auto key = std::make_pair(state, symbol);
    gotoTable[key] = nextState;
}

LALRAction LALRTable::getAction(int state, const Symbol& symbol) const {
    auto key = std::make_pair(state, symbol);
    auto it = actionTable.find(key);
    return (it != actionTable.end()) ? it->second : LALRAction(LALRActionType::ERROR);
}

int LALRTable::getGoto(int state, const Symbol& symbol) const {
    auto key = std::make_pair(state, symbol);
    auto it = gotoTable.find(key);
    return (it != gotoTable.end()) ? it->second : -1;
}

bool LALRTable::hasConflicts() const {
    detectConflicts();
    return !conflicts.empty();
}

std::vector<std::string> LALRTable::getConflicts() const {
    detectConflicts();
    return conflicts;
}

void LALRTable::detectConflicts() const {
    conflicts.clear();
    
    // 检查ACTION表中的冲突
    std::map<std::pair<int, Symbol>, std::vector<LALRAction>> actionsMap;
    
    for (const auto& entry : actionTable) {
        actionsMap[entry.first].push_back(entry.second);
    }
    
    for (const auto& entry : actionsMap) {
        if (entry.second.size() > 1) {
            std::ostringstream oss;
            oss << "Conflict at state " << entry.first.first 
                << " on symbol " << entry.first.second.toString() << ": ";
            for (size_t i = 0; i < entry.second.size(); ++i) {
                if (i > 0) oss << " vs ";
                oss << entry.second[i].toString();
            }
            conflicts.push_back(oss.str());
        }
    }
}

void LALRTable::print(const Grammar& grammar) const {
    std::cout << "LALR Parsing Table:\n\n";
    printAction(grammar);
    std::cout << "\n";
    printGoto(grammar);
}

void LALRTable::printAction(const Grammar& grammar) const {
    std::cout << "ACTION Table:\n";
    std::cout << std::setw(8) << "State";
    
    // 打印终结符表头
    for (const auto& terminal : grammar.getTerminals()) {
        std::cout << std::setw(8) << terminal.toString();
    }
    std::cout << "\n";
    
    // 打印每个状态的ACTION
    for (int state = 0; state < numStates; ++state) {
        std::cout << std::setw(8) << state;
        
        for (const auto& terminal : grammar.getTerminals()) {
            LALRAction action = getAction(state, terminal);
            if (action.isError()) {
                std::cout << std::setw(8) << "";
            } else {
                std::cout << std::setw(8) << action.toString();
            }
        }
        std::cout << "\n";
    }
}

void LALRTable::printGoto(const Grammar& grammar) const {
    std::cout << "GOTO Table:\n";
    std::cout << std::setw(8) << "State";
    
    // 打印非终结符表头
    for (const auto& nonTerminal : grammar.getNonTerminals()) {
        std::cout << std::setw(8) << nonTerminal.toString();
    }
    std::cout << "\n";
    
    // 打印每个状态的GOTO
    for (int state = 0; state < numStates; ++state) {
        std::cout << std::setw(8) << state;
        
        for (const auto& nonTerminal : grammar.getNonTerminals()) {
            int nextState = getGoto(state, nonTerminal);
            if (nextState == -1) {
                std::cout << std::setw(8) << "";
            } else {
                std::cout << std::setw(8) << nextState;
            }
        }
        std::cout << "\n";
    }
}

// LALRState类实现
void LALRState::addTransition(const Symbol& symbol, int nextState) {
    transitions[symbol] = nextState;
}

int LALRState::getTransition(const Symbol& symbol) const {
    auto it = transitions.find(symbol);
    return (it != transitions.end()) ? it->second : -1;
}

bool LALRState::hasTransition(const Symbol& symbol) const {
    return transitions.find(symbol) != transitions.end();
}

std::string LALRState::toString(const Grammar& grammar) const {
    std::ostringstream oss;
    oss << "State " << id << ":\n";
    
    for (const auto& item : itemSet.items) {
        const Production& prod = grammar.getProduction(item.productionId);
        oss << "  " << item.toString(prod) << "\n";
    }
    
    if (!transitions.empty()) {
        oss << "Transitions:\n";
        for (const auto& trans : transitions) {
            oss << "  " << trans.first.toString() << " -> " << trans.second << "\n";
        }
    }
    
    return oss.str();
}

bool LALRState::operator==(const LALRState& other) const {
    return itemSet == other.itemSet;
}

// LALRAutomaton类实现
void LALRAutomaton::build() {
    // 步骤1: 构建LR(0)项目集族
    buildLR0ItemSets();
    
    // 步骤2: 合并同心项目集（LALR的关键步骤）
    mergeLALRStates();
    
    // 步骤3: 计算前瞻符号
    computeLookaheads();
}

const LALRState& LALRAutomaton::getState(int id) const {
    if (id >= 0 && id < static_cast<int>(states.size())) {
        return states[id];
    }
    static LALRState empty;
    return empty;
}

LALRState& LALRAutomaton::getState(int id) {
    if (id >= 0 && id < static_cast<int>(states.size())) {
        return states[id];
    }
    static LALRState empty;
    return empty;
}

int LALRAutomaton::addState(const LRItemSet& itemSet) {
    int stateId = static_cast<int>(states.size());
    LALRState state(stateId);
    state.itemSet = itemSet;
    state.itemSet.id = stateId;
    states.push_back(state);
    return stateId;
}

int LALRAutomaton::findState(const LRItemSet& itemSet) const {
    for (size_t i = 0; i < states.size(); ++i) {
        if (states[i].itemSet == itemSet) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void LALRAutomaton::print() const {
    std::cout << "LALR Automaton:\n\n";
    for (const auto& state : states) {
        std::cout << state.toString(*grammar) << "\n";
    }
}

LALRTable LALRAutomaton::buildParsingTable() const {
    LALRTable table;
    table.numStates = static_cast<int>(states.size());
    
    for (const auto& state : states) {
        // 为每个项目构建ACTION和GOTO表项
        for (const auto& item : state.itemSet.items) {
            const Production& prod = grammar->getProduction(item.productionId);
            
            if (item.isComplete(prod)) {
                // 规约项目
                if (prod.left == grammar->getStartSymbol() && 
                    item.lookahead.count(grammar->getEndSymbol()) > 0) {
                    // 接受项目
                    table.setAction(state.id, grammar->getEndSymbol(), 
                                   LALRAction(LALRActionType::ACCEPT));
                } else {
                    // 普通规约项目
                    for (const auto& lookSym : item.lookahead) {
                        table.setAction(state.id, lookSym, 
                                       LALRAction(LALRActionType::REDUCE, prod.id));
                    }
                }
            } else {
                // 移入项目
                Symbol nextSym = item.getNextSymbol(prod);
                if (!nextSym.name.empty() && state.hasTransition(nextSym)) {
                    int nextState = state.getTransition(nextSym);
                    
                    if (nextSym.isTerminal()) {
                        table.setAction(state.id, nextSym, 
                                       LALRAction(LALRActionType::SHIFT, nextState));
                    } else {
                        table.setGoto(state.id, nextSym, nextState);
                    }
                }
            }
        }
    }
    
    return table;
}

LRItemSet LALRAutomaton::closure(const LRItemSet& itemSet) const {
    LRItemSet result = itemSet;
    bool changed = true;
    
    while (changed) {
        changed = false;
        std::set<LRItem> newItems;
        
        for (const auto& item : result.items) {
            const Production& prod = grammar->getProduction(item.productionId);
            
            if (!item.isComplete(prod)) {
                Symbol nextSym = item.getNextSymbol(prod);
                
                if (nextSym.isNonTerminal()) {
                    // 计算前瞻符号
                    std::vector<Symbol> beta;
                    for (size_t i = item.dotPosition + 1; i < prod.right.size(); ++i) {
                        beta.push_back(prod.right[i]);
                    }
                    
                    for (const auto& lookSym : item.lookahead) {
                        beta.push_back(lookSym);
                        std::set<Symbol> firstBeta = grammar->getFirstSet(beta);
                        beta.pop_back();
                        
                        // 为nextSym的所有产生式添加项目
                        auto prodIds = grammar->getProductionsForSymbol(nextSym);
                        for (int prodId : prodIds) {
                            LRItem newItem(prodId, 0, firstBeta);
                            if (result.items.find(newItem) == result.items.end()) {
                                newItems.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
        
        for (const auto& newItem : newItems) {
            result.addItem(newItem);
        }
    }
    
    return result;
}

LRItemSet LALRAutomaton::gotoFunction(const LRItemSet& itemSet, const Symbol& symbol) const {
    LRItemSet result;
    
    for (const auto& item : itemSet.items) {
        const Production& prod = grammar->getProduction(item.productionId);
        
        if (!item.isComplete(prod)) {
            Symbol nextSym = item.getNextSymbol(prod);
            
            if (nextSym == symbol) {
                LRItem newItem = item.advance();
                result.addItem(newItem);
            }
        }
    }
    
    return closure(result);
}

void LALRAutomaton::buildLR0ItemSets() {
    // 创建初始项目集
    LRItemSet initialSet;
    auto startProds = grammar->getProductionsForSymbol(grammar->getStartSymbol());
    
    if (!startProds.empty()) {
        LRItem initialItem(startProds[0], 0, {grammar->getEndSymbol()});
        initialSet.addItem(initialItem);
    }
    
    LRItemSet closedInitial = closure(initialSet);
    int initialStateId = addState(closedInitial);
    startState = initialStateId;
    
    // 使用工作队列算法构建所有项目集
    std::queue<int> workQueue;
    workQueue.push(initialStateId);
    
    while (!workQueue.empty()) {
        int currentStateId = workQueue.front();
        workQueue.pop();
        
        LALRState& currentState = getState(currentStateId);
        std::set<Symbol> symbols;
        
        // 收集可以转换的符号
        for (const auto& item : currentState.itemSet.items) {
            const Production& prod = grammar->getProduction(item.productionId);
            
            if (!item.isComplete(prod)) {
                Symbol nextSym = item.getNextSymbol(prod);
                if (!nextSym.name.empty()) {
                    symbols.insert(nextSym);
                }
            }
        }
        
        // 为每个符号计算GOTO
        for (const auto& symbol : symbols) {
            LRItemSet gotoSet = gotoFunction(currentState.itemSet, symbol);
            
            if (!gotoSet.empty()) {
                int targetStateId = findState(gotoSet);
                
                if (targetStateId == -1) {
                    // 创建新状态
                    targetStateId = addState(gotoSet);
                    workQueue.push(targetStateId);
                }
                
                // 添加转换
                currentState.addTransition(symbol, targetStateId);
            }
        }
    }
}

void LALRAutomaton::mergeLALRStates() {
    // LALR的关键：合并具有相同核心的状态
    std::vector<std::vector<int>> coreGroups;
    std::vector<bool> processed(states.size(), false);
    
    for (size_t i = 0; i < states.size(); ++i) {
        if (processed[i]) continue;
        
        std::vector<int> group;
        group.push_back(i);
        processed[i] = true;
        
        for (size_t j = i + 1; j < states.size(); ++j) {
            if (!processed[j] && haveSameCore(states[i].itemSet, states[j].itemSet)) {
                group.push_back(j);
                processed[j] = true;
            }
        }
        
        if (group.size() > 1) {
            coreGroups.push_back(group);
        }
    }
    
    // 合并同核心状态的前瞻符号
    for (const auto& group : coreGroups) {
        int primaryState = group[0];
        
        for (size_t i = 1; i < group.size(); ++i) {
            int secondaryState = group[i];
            
            // 合并前瞻符号
            for (auto& primaryItem : states[primaryState].itemSet.items) {
                for (const auto& secondaryItem : states[secondaryState].itemSet.items) {
                    if (primaryItem.productionId == secondaryItem.productionId &&
                        primaryItem.dotPosition == secondaryItem.dotPosition) {
                        // 找到对应项目，合并前瞻符号
                        for (const auto& look : secondaryItem.lookahead) {
                            const_cast<LRItem&>(primaryItem).lookahead.insert(look);
                        }
                    }
                }
            }
        }
    }
    
    // 移除被合并的状态（这里简化处理，实际应该重新编号）
    // 注意：这是简化实现，完整实现需要重新编号状态和更新转换
}

void LALRAutomaton::computeLookaheads() {
    // 计算和传播前瞻符号
    const_cast<Grammar*>(grammar)->computeFirstSets();
    const_cast<Grammar*>(grammar)->computeFollowSets();
    
    propagateLookaheads();
}

void LALRAutomaton::propagateLookaheads() {
    bool changed = true;
    
    while (changed) {
        changed = false;
        
        for (auto& state : states) {
            for (auto& item : state.itemSet.items) {
                const Production& prod = grammar->getProduction(item.productionId);
                
                if (!item.isComplete(prod)) {
                    Symbol nextSym = item.getNextSymbol(prod);
                    
                    if (state.hasTransition(nextSym)) {
                        int nextStateId = state.getTransition(nextSym);
                        LALRState& nextState = getState(nextStateId);
                        
                        // 查找下一状态中对应的项目
                        LRItem targetItem = item.advance();
                        
                        for (auto& nextItem : nextState.itemSet.items) {
                            if (const_cast<LRItem&>(nextItem).productionId == targetItem.productionId &&
                                const_cast<LRItem&>(nextItem).dotPosition == targetItem.dotPosition) {
                                
                                // 传播前瞻符号
                                size_t oldSize = const_cast<LRItem&>(nextItem).lookahead.size();
                                for (const auto& look : item.lookahead) {
                                    const_cast<LRItem&>(nextItem).lookahead.insert(look);
                                }
                                
                                if (const_cast<LRItem&>(nextItem).lookahead.size() > oldSize) {
                                    changed = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

std::set<Symbol> LALRAutomaton::spontaneousLookaheads(const LRItem& item) const {
    // 计算自发生成的前瞻符号
    const Production& prod = grammar->getProduction(item.productionId);
    
    if (item.isComplete(prod)) {
        return grammar->getFollowSet(prod.left);
    }
    
    std::vector<Symbol> beta;
    for (size_t i = item.dotPosition + 1; i < prod.right.size(); ++i) {
        beta.push_back(prod.right[i]);
    }
    
    return grammar->getFirstSet(beta);
}

bool LALRAutomaton::haveSameCore(const LRItemSet& set1, const LRItemSet& set2) const {
    return getCore(set1) == getCore(set2);
}

std::set<LRItem> LALRAutomaton::getCore(const LRItemSet& itemSet) const {
    std::set<LRItem> core;
    
    for (const auto& item : itemSet.items) {
        // 创建不带前瞻符号的项目
        LRItem coreItem(item.productionId, item.dotPosition);
        core.insert(coreItem);
    }
    
    return core;
}

// LALRParser类实现
LALRParser::LALRParser(const Grammar& g) 
    : grammar(&g), tokenIndex(0), hasError(false) {
    automaton = std::make_unique<LALRAutomaton>(g);
}

bool LALRParser::build() {
    try {
        // 计算FIRST和FOLLOW集
        const_cast<Grammar*>(grammar)->computeFirstSets();
        const_cast<Grammar*>(grammar)->computeFollowSets();
        
        // 构建LALR自动机
        automaton->build();
        
        // 构建分析表
        parseTable = std::make_unique<LALRTable>(automaton->buildParsingTable());
        
        // 检查冲突
        if (parseTable->hasConflicts()) {
            auto conflicts = parseTable->getConflicts();
            for (const auto& conflict : conflicts) {
                errors.push_back(conflict);
            }
            hasError = true;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        reportError("Failed to build LALR parser: " + std::string(e.what()));
        return false;
    }
}

bool LALRParser::isLALR1() const {
    return parseTable && !parseTable->hasConflicts();
}

std::unique_ptr<ASTNode> LALRParser::parse(const std::vector<Token>& inputTokens) {
    initializeParsing(inputTokens);
    
    while (!isAtEnd()) {
        Token currentToken = getCurrentToken();
        Symbol currentSymbol = tokenToSymbol(currentToken);
        
        int state = currentState();
        LALRAction action = parseTable->getAction(state, currentSymbol);
        
        if (action.isShift()) {
            if (!shift(action.value)) {
                return nullptr;
            }
        } else if (action.isReduce()) {
            if (!reduce(action.value)) {
                return nullptr;
            }
        } else if (action.isAccept()) {
            return accept() ? popNode() : nullptr;
        } else {
            reportError("Unexpected token: " + currentToken.value);
            if (!errorRecovery()) {
                return nullptr;
            }
        }
    }
    
    reportError("Unexpected end of input");
    return nullptr;
}

bool LALRParser::shift(int state) {
    Token token = getCurrentToken();
    
    // 创建对应的AST节点
    if (token.type == TokenType::IDENTIFIER) {
        pushNode(std::make_unique<IdentifierNode>(token.value, token.line, token.column));
    } else if (token.type == TokenType::NUMBER || token.type == TokenType::REAL) {
        pushNode(std::make_unique<LiteralNode>(token.type, token.value, token.line, token.column));
    } else {
        pushNode(nullptr); // 运算符等不需要创建特殊节点
    }
    
    pushState(state);
    tokenIndex++;
    
    return true;
}

bool LALRParser::reduce(int productionId) {
    const Production& prod = grammar->getProduction(productionId);
    
    // 弹出产生式右部对应的状态和节点
    std::vector<std::unique_ptr<ASTNode>> children;
    for (size_t i = 0; i < prod.right.size(); ++i) {
        popState();
        children.insert(children.begin(), popNode());
    }
    
    // 构建新的AST节点
    std::unique_ptr<ASTNode> newNode = buildASTNode(prod, children);
    
    // 查找GOTO表
    int state = currentState();
    int nextState = parseTable->getGoto(state, prod.left);
    
    if (nextState == -1) {
        reportError("GOTO table error for symbol: " + prod.left.toString());
        return false;
    }
    
    pushNode(std::move(newNode));
    pushState(nextState);
    
    return true;
}

bool LALRParser::accept() {
    return !hasError && nodeStack.size() == 1;
}

void LALRParser::reportError(const std::string& message) {
    errors.push_back(message);
    hasError = true;
}

std::unique_ptr<ASTNode> LALRParser::buildASTNode(const Production& production, 
                                                  std::vector<std::unique_ptr<ASTNode>>& children) {
    // 根据产生式构建对应的AST节点
    // 这里需要根据具体的文法来实现
    
    if (production.left.name == "E" || production.left.name == "T") {
        if (children.size() == 3) {
            // 二元表达式：E -> E + T 或 T -> T * F
            TokenType op = TokenType::PLUS; // 需要根据实际运算符确定
            
            return std::make_unique<BinaryExprNode>(
                std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(children[0].release())),
                op,
                std::unique_ptr<ExprNode>(dynamic_cast<ExprNode*>(children[2].release()))
            );
        } else if (children.size() == 1) {
            // 传递：E -> T 或 T -> F
            return std::move(children[0]);
        }
    }
    
    // 默认创建一个占位符节点
    return std::make_unique<IdentifierNode>("placeholder");
}

Symbol LALRParser::tokenToSymbol(const Token& token) const {
    // 将Token转换为Grammar中的Symbol
    for (const auto& terminal : grammar->getTerminals()) {
        if (terminal.tokenType == token.type) {
            return terminal;
        }
    }
    
    return grammar->getEndSymbol(); // 默认返回结束符号
}

void LALRParser::initializeParsing(const std::vector<Token>& inputTokens) {
    tokens = inputTokens;
    tokens.push_back(Token(TokenType::END_OF_FILE, "$", -1, -1)); // 添加结束符
    tokenIndex = 0;
    
    stateStack.clear();
    nodeStack.clear();
    errors.clear();
    hasError = false;
    
    stateStack.push_back(0); // 初始状态
}

Token LALRParser::getCurrentToken() const {
    if (tokenIndex < tokens.size()) {
        return tokens[tokenIndex];
    }
    return Token(TokenType::END_OF_FILE, "$", -1, -1);
}

bool LALRParser::isAtEnd() const {
    return tokenIndex >= tokens.size() || 
           (tokenIndex == tokens.size() - 1 && tokens[tokenIndex].type == TokenType::END_OF_FILE);
}

int LALRParser::currentState() const {
    return stateStack.empty() ? -1 : stateStack.back();
}

void LALRParser::pushState(int state) {
    stateStack.push_back(state);
}

int LALRParser::popState() {
    if (stateStack.empty()) return -1;
    int state = stateStack.back();
    stateStack.pop_back();
    return state;
}

void LALRParser::pushNode(std::unique_ptr<ASTNode> node) {
    nodeStack.push_back(std::move(node));
}

std::unique_ptr<ASTNode> LALRParser::popNode() {
    if (nodeStack.empty()) return nullptr;
    auto node = std::move(nodeStack.back());
    nodeStack.pop_back();
    return node;
}

void LALRParser::clearErrors() {
    errors.clear();
    hasError = false;
}

void LALRParser::printAutomaton() const {
    if (automaton) {
        automaton->print();
    }
}

void LALRParser::printParsingTable() const {
    if (parseTable) {
        parseTable->print(*grammar);
    }
}

bool LALRParser::errorRecovery() {
    // 简单的错误恢复：跳过当前token
    tokenIndex++;
    return tokenIndex < tokens.size();
}

// LALRParserBuilder类实现
std::unique_ptr<LALRParser> LALRParserBuilder::build(const Grammar& grammar) {
    auto parser = std::make_unique<LALRParser>(grammar);
    
    if (!parser->build()) {
        return nullptr;
    }
    
    return parser;
} 