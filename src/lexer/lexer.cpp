#include "lexer.h"
#include <sstream>
#include <cctype>
#include <algorithm>

// ==================== LexicalError 实现 ====================

std::string LexicalError::toString() const {
    std::stringstream ss;
    ss << "Lexical Error at line " << line << ", column " << column 
       << ": " << message;
    if (!context.empty()) {
        ss << " (context: \"" << context << "\")";
    }
    return ss.str();
}

// ==================== LexicalResult 实现 ====================

void LexicalResult::addToken(const Token& token) {
    tokens.push_back(token);
}

void LexicalResult::addError(const LexicalError& error) {
    errors.push_back(error);
    success = false;
}

bool LexicalResult::hasErrors() const {
    return !errors.empty();
}

void LexicalResult::printTokens() const {
    std::cout << "=== Lexical Analysis Results ===" << std::endl;
    std::cout << "Total tokens: " << tokens.size() << std::endl;
    
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << "[" << i << "] " << tokens[i].toString() << std::endl;
    }
}

void LexicalResult::printErrors() const {
    if (errors.empty()) {
        std::cout << "No lexical errors found." << std::endl;
        return;
    }
    
    std::cout << "=== Lexical Errors ===" << std::endl;
    for (const auto& error : errors) {
        std::cout << error.toString() << std::endl;
    }
}

// ==================== Lexer 实现 ====================

Lexer::Lexer() 
    : position(0), currentLine(1), currentColumn(1), atEnd(false), currentChar('\0') {
    dfa = std::make_unique<DFA>();
    dfa->buildStandardDFA();
    
    // 默认忽略空白符和注释
    ignoredTokens.insert(TokenType::WHITESPACE);
    ignoredTokens.insert(TokenType::COMMENT);
}

Lexer::Lexer(const std::string& source) : Lexer() {
    setSource(source);
}

void Lexer::setSource(const std::string& source) {
    sourceCode = source;
    reset();
}

void Lexer::addIgnoredTokenType(TokenType type) {
    ignoredTokens.insert(type);
}

LexicalResult Lexer::analyze() {
    LexicalResult result;
    reset();
    
    while (!isAtEnd()) {
        try {
            Token token = getNextToken();
            
            if (token.type == TokenType::ERROR) {
                // 处理错误Token
                result.addError(LexicalError(token.value, token.line, token.column));
            } else if (token.type != TokenType::END_OF_FILE) {
                // 检查是否需要忽略此Token
                if (ignoredTokens.find(token.type) == ignoredTokens.end()) {
                    result.addToken(token);
                }
            } else {
                result.addToken(token);
                break;
            }
        } catch (const std::exception& e) {
            result.addError(LexicalError(e.what(), currentLine, currentColumn));
            advance(); // 跳过错误字符，继续分析
        }
    }
    
    return result;
}

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (isAtEnd()) {
        return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
    }
    
    return scanToken();
}

bool Lexer::isAtEnd() const {
    return position >= sourceCode.length();
}

int Lexer::getCurrentLine() const {
    return currentLine;
}

int Lexer::getCurrentColumn() const {
    return currentColumn;
}

size_t Lexer::getCurrentPosition() const {
    return position;
}

void Lexer::reset() {
    position = 0;
    currentLine = 1;
    currentColumn = 1;
    atEnd = false;
    currentChar = sourceCode.empty() ? '\0' : sourceCode[0];
}

void Lexer::setDFA(std::unique_ptr<DFA> customDFA) {
    dfa = std::move(customDFA);
}

std::string Lexer::getSourceRange(size_t start, size_t end) const {
    if (start >= sourceCode.length() || end > sourceCode.length() || start > end) {
        return "";
    }
    return sourceCode.substr(start, end - start);
}

// 静态工具方法
bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) {
    return isAlpha(c) || isDigit(c);
}

// 私有方法实现
void Lexer::advance() {
    if (!isAtEnd()) {
        updatePosition(currentChar);
        position++;
        currentChar = isAtEnd() ? '\0' : sourceCode[position];
    }
}

char Lexer::peek() const {
    return currentChar;
}

char Lexer::peekNext() const {
    if (position + 1 >= sourceCode.length()) {
        return '\0';
    }
    return sourceCode[position + 1];
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && isWhitespace(peek())) {
        advance();
    }
}

Token Lexer::scanToken() {
    char c = peek();
    
    // 标识符和关键字
    if (isAlpha(c)) {
        return scanIdentifierOrKeyword();
    }
    
    // 数字
    if (isDigit(c)) {
        return scanNumber();
    }
    
    // 字符串字面量
    if (c == '"') {
        return scanString();
    }
    
    // 注释
    if (c == '/' && peekNext() == '/') {
        return scanComment();
    }
    
    // 运算符
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' ||
        c == '=' || c == '!' || c == '<' || c == '>' || c == '&' || c == '|') {
        return scanOperator();
    }
    
    // 分隔符
    if (c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']' ||
        c == ';' || c == ',' || (c == '.' && !isDigit(peekNext()))) {
        return scanDelimiter();
    }
    
    // 未知字符 - 处理UTF-8编码字符
    if (static_cast<unsigned char>(c) > 127) {
        // 跳过UTF-8字符序列
        advance();
        while (!isAtEnd() && (static_cast<unsigned char>(peek()) & 0xC0) == 0x80) {
            advance();
        }
        // 忽略非ASCII字符，继续分析
        return getNextToken();
    }
    
    std::string errorMsg = "Unexpected character: '";
    errorMsg += c;
    errorMsg += "'";
    advance(); // 跳过错误字符
    return makeErrorToken(errorMsg);
}

Token Lexer::scanIdentifierOrKeyword() {
    size_t start = position;
    int startLine = currentLine;
    int startColumn = currentColumn;
    
    // 第一个字符必须是字母或下划线
    if (!isAlpha(peek())) {
        return makeErrorToken("Invalid identifier start");
    }
    
    // 读取标识符的所有字符
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        advance();
    }
    
    std::string text = sourceCode.substr(start, position - start);
    
    // 检查是否是关键字
    TokenType type = identifyKeyword(text);
    if (type == TokenType::IDENTIFIER) {
        // 确实是标识符
        return Token(TokenType::IDENTIFIER, text, startLine, startColumn);
    } else {
        // 是关键字
        return Token(type, text, startLine, startColumn);
    }
}

Token Lexer::scanNumber() {
    size_t start = position;
    int startLine = currentLine;
    int startColumn = currentColumn;
    TokenType numberType = TokenType::NUMBER;
    
    // 读取整数部分
    while (!isAtEnd() && isDigit(peek())) {
        advance();
    }
    
    // 检查是否有小数点
    if (!isAtEnd() && peek() == '.' && isDigit(peekNext())) {
        numberType = TokenType::REAL;
        advance(); // 跳过小数点
        
        // 读取小数部分
        while (!isAtEnd() && isDigit(peek())) {
            advance();
        }
    }
    
    std::string text = sourceCode.substr(start, position - start);
    return Token(numberType, text, startLine, startColumn);
}

Token Lexer::scanString() {
    size_t start = position;
    int startLine = currentLine;
    int startColumn = currentColumn;
    
    advance(); // 跳过开始的引号
    
    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            return makeErrorToken("Unterminated string literal");
        }
        advance();
    }
    
    if (isAtEnd()) {
        return makeErrorToken("Unterminated string literal");
    }
    
    advance(); // 跳过结束的引号
    
    // 提取字符串内容（不包括引号）
    std::string text = sourceCode.substr(start + 1, position - start - 2);
    return Token(TokenType::STRING, text, startLine, startColumn);
}

Token Lexer::scanComment() {
    size_t start = position;
    int startLine = currentLine;
    int startColumn = currentColumn;
    
    advance(); // 跳过第一个斜杠
    advance(); // 跳过第二个斜杠
    
    // 读取到行末
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
    
    std::string text = sourceCode.substr(start, position - start);
    return Token(TokenType::COMMENT, text, startLine, startColumn);
}

Token Lexer::scanOperator() {
    char c = peek();
    int startLine = currentLine;
    int startColumn = currentColumn;
    
    advance();
    
    // 检查双字符运算符
    char next = peek();
    
    switch (c) {
        case '+':
            if (next == '=') {
                advance();
                return Token(TokenType::PLUS_ASSIGN, "+=", startLine, startColumn);
            }
            return Token(TokenType::PLUS, "+", startLine, startColumn);
        case '-':
            if (next == '=') {
                advance();
                return Token(TokenType::MINUS_ASSIGN, "-=", startLine, startColumn);
            }
            return Token(TokenType::MINUS, "-", startLine, startColumn);
        case '*':
            if (next == '=') {
                advance();
                return Token(TokenType::MUL_ASSIGN, "*=", startLine, startColumn);
            }
            return Token(TokenType::MULTIPLY, "*", startLine, startColumn);
        case '/':
            if (next == '=') {
                advance();
                return Token(TokenType::DIV_ASSIGN, "/=", startLine, startColumn);
            }
            return Token(TokenType::DIVIDE, "/", startLine, startColumn);
        case '%':
            if (next == '=') {
                advance();
                return Token(TokenType::MOD_ASSIGN, "%=", startLine, startColumn);
            }
            return Token(TokenType::MODULO, "%", startLine, startColumn);
        case '=':
            if (next == '=') {
                advance();
                return Token(TokenType::EQ, "==", startLine, startColumn);
            }
            return Token(TokenType::ASSIGN, "=", startLine, startColumn);
        case '!':
            if (next == '=') {
                advance();
                return Token(TokenType::NE, "!=", startLine, startColumn);
            }
            return Token(TokenType::NOT, "!", startLine, startColumn);
        case '<':
            if (next == '=') {
                advance();
                return Token(TokenType::LE, "<=", startLine, startColumn);
            }
            return Token(TokenType::LT, "<", startLine, startColumn);
        case '>':
            if (next == '=') {
                advance();
                return Token(TokenType::GE, ">=", startLine, startColumn);
            }
            return Token(TokenType::GT, ">", startLine, startColumn);
        case '&':
            if (next == '&') {
                advance();
                return Token(TokenType::AND, "&&", startLine, startColumn);
            }
            break;
        case '|':
            if (next == '|') {
                advance();
                return Token(TokenType::OR, "||", startLine, startColumn);
            }
            break;
    }
    
    return makeErrorToken("Unknown operator");
}

Token Lexer::scanDelimiter() {
    char c = peek();
    int startLine = currentLine;
    int startColumn = currentColumn;
    
    advance();
    
    switch (c) {
        case '(': return Token(TokenType::LPAREN, "(", startLine, startColumn);
        case ')': return Token(TokenType::RPAREN, ")", startLine, startColumn);
        case '{': return Token(TokenType::LBRACE, "{", startLine, startColumn);
        case '}': return Token(TokenType::RBRACE, "}", startLine, startColumn);
        case '[': return Token(TokenType::LBRACKET, "[", startLine, startColumn);
        case ']': return Token(TokenType::RBRACKET, "]", startLine, startColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine, startColumn);
        case ',': return Token(TokenType::COMMA, ",", startLine, startColumn);
        case '.': return Token(TokenType::DOT, ".", startLine, startColumn);
    }
    
    return makeErrorToken("Unknown delimiter");
}

Token Lexer::makeErrorToken(const std::string& message) {
    return Token(TokenType::ERROR, message, currentLine, currentColumn);
}

void Lexer::reportError(const std::string& message) {
    std::cerr << "Lexical Error at line " << currentLine 
              << ", column " << currentColumn << ": " << message << std::endl;
}

TokenType Lexer::identifyKeyword(const std::string& text) const {
    // 使用Token类的关键字映射
    auto it = Token::keywordMap.find(text);
    if (it != Token::keywordMap.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

std::string Lexer::extractString(size_t start, size_t end) const {
    if (start >= sourceCode.length() || end > sourceCode.length() || start > end) {
        return "";
    }
    return sourceCode.substr(start, end - start);
}

void Lexer::updatePosition(char c) {
    if (c == '\n') {
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
}

// ==================== LexerFactory 实现 ====================

std::unique_ptr<Lexer> LexerFactory::createStandardLexer() {
    auto lexer = std::make_unique<Lexer>();
    configureIgnoreRules(*lexer, true, true);
    return lexer;
}

std::unique_ptr<Lexer> LexerFactory::createCustomLexer(std::unique_ptr<DFA> dfa) {
    auto lexer = std::make_unique<Lexer>();
    lexer->setDFA(std::move(dfa));
    configureIgnoreRules(*lexer, true, true);
    return lexer;
}

void LexerFactory::configureIgnoreRules(Lexer& lexer, bool ignoreComments, bool ignoreWhitespace) {
    if (ignoreComments) {
        lexer.addIgnoredTokenType(TokenType::COMMENT);
    }
    if (ignoreWhitespace) {
        lexer.addIgnoredTokenType(TokenType::WHITESPACE);
    }
} 