#include "dfa.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ==================== DFAState 实现 ====================

DFAState::DFAState(int id, DFAStateType type, TokenType tokenType)
    : id(id), type(type), tokenType(tokenType) {
}

void DFAState::addTransition(char c, int nextStateId) {
    transitions[c] = nextStateId;
}

void DFAState::addRangeTransition(char start, char end, int nextStateId) {
    for (char c = start; c <= end; ++c) {
        transitions[c] = nextStateId;
    }
}

int DFAState::getNextState(char c) const {
    auto it = transitions.find(c);
    return (it != transitions.end()) ? it->second : -1;
}

bool DFAState::isAccepting() const {
    return type == DFAStateType::ACCEPTING;
}

std::string DFAState::toString() const {
    std::stringstream ss;
    ss << "State " << id << " (" 
       << (type == DFAStateType::ACCEPTING ? "ACCEPTING" : 
           type == DFAStateType::ERROR ? "ERROR" : "NORMAL") << ")";
    if (type == DFAStateType::ACCEPTING) {
        ss << " -> " << Token::getTypeString(tokenType);
    }
    return ss.str();
}

// ==================== DFA 实现 ====================

DFA::DFA() : startState(0), currentState(0) {
    buildCharacterClasses();
}

void DFA::buildCharacterClasses() {
    // 构建字符分类映射，用于优化状态转换
    // 字母
    for (char c = 'a'; c <= 'z'; ++c) charClassMap[c] = 'L';
    for (char c = 'A'; c <= 'Z'; ++c) charClassMap[c] = 'L';
    charClassMap['_'] = 'L';
    
    // 数字
    for (char c = '0'; c <= '9'; ++c) charClassMap[c] = 'D';
    
    // 空白字符
    charClassMap[' '] = 'W';
    charClassMap['\t'] = 'W';
    charClassMap['\n'] = 'W';
    charClassMap['\r'] = 'W';
    
    // 运算符
    charClassMap['+'] = 'O';
    charClassMap['-'] = 'O';
    charClassMap['*'] = 'O';
    charClassMap['/'] = 'O';
    charClassMap['%'] = 'O';
    charClassMap['='] = 'O';
    charClassMap['!'] = 'O';
    charClassMap['<'] = 'O';
    charClassMap['>'] = 'O';
    charClassMap['&'] = 'O';
    charClassMap['|'] = 'O';
    
    // 分隔符
    charClassMap['('] = 'P';
    charClassMap[')'] = 'P';
    charClassMap['{'] = 'P';
    charClassMap['}'] = 'P';
    charClassMap['['] = 'P';
    charClassMap[']'] = 'P';
    charClassMap[';'] = 'P';
    charClassMap[','] = 'P';
    charClassMap['.'] = 'P';
    
    // 字符串引号
    charClassMap['"'] = 'Q';
    charClassMap['\''] = 'Q';
}

char DFA::getCharClass(char c) const {
    auto it = charClassMap.find(c);
    return (it != charClassMap.end()) ? it->second : 'X'; // X表示其他字符
}

void DFA::buildStandardDFA() {
    states.clear();
    acceptingStates.clear();
    
    // 状态0：起始状态
    addState(DFAStateType::NORMAL);
    setStartState(0);
    
    // 构建各个识别模块
    buildIdentifierDFA();
    buildNumberDFA();
    buildOperatorDFA();
    buildDelimiterDFA();
    buildStringLiteralDFA();
    buildCommentDFA();
}

int DFA::addState(DFAStateType type, TokenType tokenType) {
    int newStateId = states.size();
    states.emplace_back(newStateId, type, tokenType);
    
    if (type == DFAStateType::ACCEPTING) {
        acceptingStates.insert(newStateId);
    }
    
    return newStateId;
}

void DFA::setStartState(int stateId) {
    if (stateId >= 0 && stateId < static_cast<int>(states.size())) {
        startState = stateId;
        currentState = stateId;
    }
}

void DFA::addTransition(int fromState, char c, int toState) {
    if (fromState >= 0 && fromState < static_cast<int>(states.size()) &&
        toState >= 0 && toState < static_cast<int>(states.size())) {
        states[fromState].addTransition(c, toState);
    }
}

void DFA::addRangeTransition(int fromState, char start, char end, int toState) {
    if (fromState >= 0 && fromState < static_cast<int>(states.size()) &&
        toState >= 0 && toState < static_cast<int>(states.size())) {
        states[fromState].addRangeTransition(start, end, toState);
    }
}

void DFA::addStringTransition(int fromState, const std::string& str, int toState) {
    int currentStateId = fromState;
    
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        
        if (i == str.length() - 1) {
            // 最后一个字符，转换到目标状态
            addTransition(currentStateId, c, toState);
        } else {
            // 中间字符，创建中间状态
            int nextStateId = addState(DFAStateType::NORMAL);
            addTransition(currentStateId, c, nextStateId);
            currentStateId = nextStateId;
        }
    }
}

void DFA::reset() {
    currentState = startState;
}

bool DFA::processChar(char c) {
    if (currentState < 0 || currentState >= static_cast<int>(states.size())) {
        return false;
    }
    
    int nextState = states[currentState].getNextState(c);
    if (nextState == -1) {
        return false; // 无有效转换
    }
    
    currentState = nextState;
    return true;
}

bool DFA::isInAcceptingState() const {
    return currentState >= 0 && 
           currentState < static_cast<int>(states.size()) &&
           states[currentState].isAccepting();
}

TokenType DFA::getCurrentTokenType() const {
    if (isInAcceptingState()) {
        return states[currentState].tokenType;
    }
    return TokenType::UNKNOWN;
}

int DFA::getCurrentState() const {
    return currentState;
}

size_t DFA::getStateCount() const {
    return states.size();
}

void DFA::printDFA() const {
    std::cout << "=== DFA Structure ===" << std::endl;
    std::cout << "Start State: " << startState << std::endl;
    std::cout << "Current State: " << currentState << std::endl;
    std::cout << "Total States: " << states.size() << std::endl;
    
    for (const auto& state : states) {
        std::cout << state.toString() << std::endl;
        for (const auto& transition : state.transitions) {
            std::cout << "  '" << transition.first << "' -> State " << transition.second << std::endl;
        }
    }
}

bool DFA::validate() const {
    // 检查起始状态是否有效
    if (startState < 0 || startState >= static_cast<int>(states.size())) {
        return false;
    }
    
    // 检查所有状态转换是否指向有效状态
    for (const auto& state : states) {
        for (const auto& transition : state.transitions) {
            int targetState = transition.second;
            if (targetState < 0 || targetState >= static_cast<int>(states.size())) {
                return false;
            }
        }
    }
    
    return true;
}

std::pair<TokenType, std::string> DFA::recognizeToken(const std::string& input) const {
    // 创建临时DFA实例进行识别
    DFA tempDFA = *this;
    tempDFA.reset();
    
    std::string recognizedToken = "";
    TokenType lastAcceptedType = TokenType::UNKNOWN;
    
    for (char c : input) {
        if (!tempDFA.processChar(c)) {
            break; // 无法继续处理
        }
        
        recognizedToken += c;
        
        if (tempDFA.isInAcceptingState()) {
            lastAcceptedType = tempDFA.getCurrentTokenType();
        }
    }
    
    return {lastAcceptedType, recognizedToken};
}

// ==================== DFA构建的具体实现 ====================

void DFA::buildIdentifierDFA() {
    // 标识符: [a-zA-Z_][a-zA-Z0-9_]*
    
    // 状态1：识别标识符第一个字符（字母或下划线）
    int identifierStart = addState(DFAStateType::NORMAL);
    
    // 状态2：标识符接受状态
    int identifierAccept = addState(DFAStateType::ACCEPTING, TokenType::IDENTIFIER);
    
    // 从起始状态到标识符开始状态
    addRangeTransition(0, 'a', 'z', identifierStart);
    addRangeTransition(0, 'A', 'Z', identifierStart);
    addTransition(0, '_', identifierStart);
    
    // 从标识符开始状态到接受状态
    addRangeTransition(identifierStart, 'a', 'z', identifierAccept);
    addRangeTransition(identifierStart, 'A', 'Z', identifierAccept);
    addRangeTransition(identifierStart, '0', '9', identifierAccept);
    addTransition(identifierStart, '_', identifierAccept);
    
    // 标识符接受状态的自循环
    addRangeTransition(identifierAccept, 'a', 'z', identifierAccept);
    addRangeTransition(identifierAccept, 'A', 'Z', identifierAccept);
    addRangeTransition(identifierAccept, '0', '9', identifierAccept);
    addTransition(identifierAccept, '_', identifierAccept);
}

void DFA::buildNumberDFA() {
    // 整数: [0-9]+
    // 浮点数: [0-9]+\.[0-9]+
    
    // 状态3：整数接受状态
    int integerAccept = addState(DFAStateType::ACCEPTING, TokenType::NUMBER);
    
    // 状态4：小数点状态
    int dotState = addState(DFAStateType::NORMAL);
    
    // 状态5：浮点数接受状态
    int floatAccept = addState(DFAStateType::ACCEPTING, TokenType::REAL);
    
    // 从起始状态识别数字
    addRangeTransition(0, '0', '9', integerAccept);
    
    // 整数的自循环
    addRangeTransition(integerAccept, '0', '9', integerAccept);
    
    // 从整数状态到小数点
    addTransition(integerAccept, '.', dotState);
    
    // 从小数点状态到浮点数接受状态
    addRangeTransition(dotState, '0', '9', floatAccept);
    
    // 浮点数的自循环
    addRangeTransition(floatAccept, '0', '9', floatAccept);
}

void DFA::buildOperatorDFA() {
    // 单字符运算符
    int plusState = addState(DFAStateType::ACCEPTING, TokenType::PLUS);
    int minusState = addState(DFAStateType::ACCEPTING, TokenType::MINUS);
    int multiplyState = addState(DFAStateType::ACCEPTING, TokenType::MULTIPLY);
    int divideState = addState(DFAStateType::ACCEPTING, TokenType::DIVIDE);
    int moduloState = addState(DFAStateType::ACCEPTING, TokenType::MODULO);
    int assignState = addState(DFAStateType::ACCEPTING, TokenType::ASSIGN);
    
    // 双字符运算符
    int eqState = addState(DFAStateType::ACCEPTING, TokenType::EQ);
    int neState = addState(DFAStateType::ACCEPTING, TokenType::NE);
    int leState = addState(DFAStateType::ACCEPTING, TokenType::LE);
    int geState = addState(DFAStateType::ACCEPTING, TokenType::GE);
    int ltState = addState(DFAStateType::ACCEPTING, TokenType::LT);
    int gtState = addState(DFAStateType::ACCEPTING, TokenType::GT);
    
    // 单字符运算符转换
    addTransition(0, '+', plusState);
    addTransition(0, '-', minusState);
    addTransition(0, '*', multiplyState);
    addTransition(0, '/', divideState);
    addTransition(0, '%', moduloState);
    addTransition(0, '=', assignState);
    addTransition(0, '<', ltState);
    addTransition(0, '>', gtState);
    
    // 双字符运算符转换
    addTransition(assignState, '=', eqState);  // ==
    
    int notState = addState(DFAStateType::NORMAL);
    addTransition(0, '!', notState);
    addTransition(notState, '=', neState);     // !=
    
    addTransition(ltState, '=', leState);      // <=
    addTransition(gtState, '=', geState);      // >=
}

void DFA::buildDelimiterDFA() {
    // 分隔符
    addTransition(0, '(', addState(DFAStateType::ACCEPTING, TokenType::LPAREN));
    addTransition(0, ')', addState(DFAStateType::ACCEPTING, TokenType::RPAREN));
    addTransition(0, '{', addState(DFAStateType::ACCEPTING, TokenType::LBRACE));
    addTransition(0, '}', addState(DFAStateType::ACCEPTING, TokenType::RBRACE));
    addTransition(0, '[', addState(DFAStateType::ACCEPTING, TokenType::LBRACKET));
    addTransition(0, ']', addState(DFAStateType::ACCEPTING, TokenType::RBRACKET));
    addTransition(0, ';', addState(DFAStateType::ACCEPTING, TokenType::SEMICOLON));
    addTransition(0, ',', addState(DFAStateType::ACCEPTING, TokenType::COMMA));
    addTransition(0, '.', addState(DFAStateType::ACCEPTING, TokenType::DOT));
}

void DFA::buildStringLiteralDFA() {
    // 字符串字面量: "..."
    int stringStart = addState(DFAStateType::NORMAL);
    int stringContent = addState(DFAStateType::NORMAL);
    int stringAccept = addState(DFAStateType::ACCEPTING, TokenType::STRING);
    
    // 开始引号
    addTransition(0, '"', stringStart);
    
    // 字符串内容（除了引号和换行符的所有字符）
    for (int c = 32; c < 127; ++c) {
        if (c != '"' && c != '\n') {
            addTransition(stringStart, static_cast<char>(c), stringContent);
            addTransition(stringContent, static_cast<char>(c), stringContent);
        }
    }
    
    // 结束引号
    addTransition(stringStart, '"', stringAccept); // 空字符串
    addTransition(stringContent, '"', stringAccept);
}

void DFA::buildCommentDFA() {
    // 单行注释: //...
    int commentStart = addState(DFAStateType::NORMAL);
    int commentLine = addState(DFAStateType::ACCEPTING, TokenType::COMMENT);
    
    // 第一个斜杠
    int slashState = addState(DFAStateType::NORMAL);
    addTransition(0, '/', slashState);
    
    // 第二个斜杠，开始注释
    addTransition(slashState, '/', commentStart);
    
    // 注释内容（除换行符外的所有字符）
    for (int c = 32; c < 127; ++c) {
        if (c != '\n') {
            addTransition(commentStart, static_cast<char>(c), commentLine);
            addTransition(commentLine, static_cast<char>(c), commentLine);
        }
    }
}

// ==================== DFABuilder 实现 ====================

DFA DFABuilder::buildLexerDFA() {
    DFA dfa;
    dfa.buildStandardDFA();
    
    // 添加关键字识别
    addKeywordStates(dfa, 0);
    
    return dfa;
}

void DFABuilder::addKeywordStates(DFA& dfa, int startState) {
    // 为每个关键字添加专门的状态路径
    std::vector<std::pair<std::string, TokenType>> keywords = {
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"do", TokenType::DO},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"return", TokenType::RETURN},
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"bool", TokenType::BOOL},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE}
    };
    
    for (const auto& keyword : keywords) {
        int keywordAcceptState = dfa.addState(DFAStateType::ACCEPTING, keyword.second);
        dfa.addStringTransition(startState, keyword.first, keywordAcceptState);
    }
}

void DFABuilder::optimizeDFA(DFA& dfa) {
    // DFA优化将在minimizer.cpp中实现
    // 这里暂时不做处理
} 