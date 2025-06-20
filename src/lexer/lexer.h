#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <set>
#include "token.h"
#include "dfa.h"

/**
 * @brief 词法错误类
 */
class LexicalError {
public:
    std::string message;
    int line;
    int column;
    std::string context;
    
    LexicalError(const std::string& msg, int line, int col, const std::string& ctx = "")
        : message(msg), line(line), column(col), context(ctx) {}
    
    std::string toString() const;
};

/**
 * @brief 词法分析结果类
 */
class LexicalResult {
public:
    std::vector<Token> tokens;
    std::vector<LexicalError> errors;
    bool success;
    
    LexicalResult() : success(true) {}
    
    void addToken(const Token& token);
    void addError(const LexicalError& error);
    bool hasErrors() const;
    void printTokens() const;
    void printErrors() const;
};

/**
 * @brief 词法分析器类
 */
class Lexer {
private:
    std::string sourceCode;          // 源代码
    size_t position;                 // 当前位置
    int currentLine;                 // 当前行号
    int currentColumn;               // 当前列号
    std::unique_ptr<DFA> dfa;        // DFA实例
    
    // 词法分析状态
    bool atEnd;
    char currentChar;
    
    // 忽略的Token类型
    std::set<TokenType> ignoredTokens;
    
public:
    Lexer();
    explicit Lexer(const std::string& source);
    ~Lexer() = default;
    
    // 设置源代码
    void setSource(const std::string& source);
    
    // 添加要忽略的Token类型（如空白符、注释）
    void addIgnoredTokenType(TokenType type);
    
    // 执行词法分析
    LexicalResult analyze();
    
    // 逐个获取Token（流式处理）
    Token getNextToken();
    
    // 检查是否到达末尾
    bool isAtEnd() const;
    
    // 获取当前位置信息
    int getCurrentLine() const;
    int getCurrentColumn() const;
    size_t getCurrentPosition() const;
    
    // 重置词法分析器
    void reset();
    
    // 设置DFA（允许使用自定义DFA）
    void setDFA(std::unique_ptr<DFA> customDFA);
    
    // 获取源代码的指定范围
    std::string getSourceRange(size_t start, size_t end) const;
    
    // 静态工具方法
    static bool isWhitespace(char c);
    static bool isAlpha(char c);
    static bool isDigit(char c);
    static bool isAlphaNumeric(char c);
    
private:
    // 内部辅助方法
    void advance();                  // 前进一个字符
    char peek() const;               // 查看当前字符
    char peekNext() const;           // 查看下一个字符
    void skipWhitespace();           // 跳过空白字符
    
    // Token识别方法
    Token scanToken();
    Token scanIdentifierOrKeyword();
    Token scanNumber();
    Token scanString();
    Token scanComment();
    Token scanOperator();
    Token scanDelimiter();
    
    // 错误处理
    Token makeErrorToken(const std::string& message);
    void reportError(const std::string& message);
    
    // 关键字识别
    TokenType identifyKeyword(const std::string& text) const;
    
    // 字符串处理辅助
    std::string extractString(size_t start, size_t end) const;
    void updatePosition(char c);
};

/**
 * @brief 词法分析器工厂类
 */
class LexerFactory {
public:
    // 创建标准C-like语言的词法分析器
    static std::unique_ptr<Lexer> createStandardLexer();
    
    // 创建带自定义DFA的词法分析器
    static std::unique_ptr<Lexer> createCustomLexer(std::unique_ptr<DFA> dfa);
    
    // 配置词法分析器的忽略规则
    static void configureIgnoreRules(Lexer& lexer, bool ignoreComments = true, 
                                    bool ignoreWhitespace = true);
};

#endif // LEXER_H 