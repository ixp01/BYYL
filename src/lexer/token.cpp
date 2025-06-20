#include "token.h"
#include <sstream>

// 静态成员初始化
std::map<std::string, TokenType> Token::keywordMap = {
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

std::map<TokenType, std::string> Token::typeStringMap = {
    {TokenType::IDENTIFIER, "IDENTIFIER"},
    {TokenType::NUMBER, "NUMBER"},
    {TokenType::REAL, "REAL"},
    {TokenType::STRING, "STRING"},
    {TokenType::IF, "IF"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::WHILE, "WHILE"},
    {TokenType::FOR, "FOR"},
    {TokenType::DO, "DO"},
    {TokenType::BREAK, "BREAK"},
    {TokenType::CONTINUE, "CONTINUE"},
    {TokenType::RETURN, "RETURN"},
    {TokenType::INT, "INT"},
    {TokenType::FLOAT, "FLOAT"},
    {TokenType::BOOL, "BOOL"},
    {TokenType::TRUE, "TRUE"},
    {TokenType::FALSE, "FALSE"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::PLUS, "PLUS"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::MULTIPLY, "MULTIPLY"},
    {TokenType::DIVIDE, "DIVIDE"},
    {TokenType::MODULO, "MODULO"},
    {TokenType::EQ, "EQ"},
    {TokenType::NE, "NE"},
    {TokenType::LT, "LT"},
    {TokenType::LE, "LE"},
    {TokenType::GT, "GT"},
    {TokenType::GE, "GE"},
    {TokenType::AND, "AND"},
    {TokenType::OR, "OR"},
    {TokenType::NOT, "NOT"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::COMMA, "COMMA"},
    {TokenType::LPAREN, "LPAREN"},
    {TokenType::RPAREN, "RPAREN"},
    {TokenType::LBRACE, "LBRACE"},
    {TokenType::RBRACE, "RBRACE"},
    {TokenType::LBRACKET, "LBRACKET"},
    {TokenType::RBRACKET, "RBRACKET"},
    {TokenType::END_OF_FILE, "END_OF_FILE"},
    {TokenType::ERROR, "ERROR"},
    {TokenType::COMMENT, "COMMENT"},
    {TokenType::WHITESPACE, "WHITESPACE"},
    {TokenType::DOT, "DOT"},
    {TokenType::NEWLINE, "NEWLINE"},
    {TokenType::UNKNOWN, "UNKNOWN"}
};

Token::Token(TokenType t, const std::string& v, int l, int c)
    : type(t), value(v), line(l), column(c) {
}

bool Token::isKeyword() const {
    return type >= TokenType::IF && type <= TokenType::FALSE;
}

bool Token::isOperator() const {
    return (type >= TokenType::ASSIGN && type <= TokenType::MODULO) ||
           (type >= TokenType::EQ && type <= TokenType::NOT);
}

std::string Token::getTypeString() const {
    auto it = typeStringMap.find(type);
    if (it != typeStringMap.end()) {
        return it->second;
    }
    return "UNKNOWN";
}

std::string Token::toString() const {
    std::ostringstream oss;
    oss << "Token{" << getTypeString() << ", \"" << value 
        << "\", " << line << ":" << column << "}";
    return oss.str();
}

const std::map<std::string, TokenType>& Token::getKeywordMap() {
    return keywordMap;
}

std::string Token::getTypeString(TokenType type) {
    auto it = typeStringMap.find(type);
    if (it != typeStringMap.end()) {
        return it->second;
    }
    return "UNKNOWN";
} 