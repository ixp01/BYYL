#include <iostream>
#include <cassert>
#include <vector>
#include <utility>
#include "../src/lexer/token.h"

/**
 * @brief 测试Token类的基本功能
 */
void testToken() {
    std::cout << "Testing Token class..." << std::endl;
    
    // 测试Token构造
    Token token1(TokenType::IDENTIFIER, "variable", 1, 5);
    assert(token1.type == TokenType::IDENTIFIER);
    assert(token1.value == "variable");
    assert(token1.line == 1);
    assert(token1.column == 5);
    
    // 测试关键字检查
    Token keywordToken(TokenType::IF, "if", 1, 1);
    assert(keywordToken.isKeyword());
    assert(!token1.isKeyword());
    
    // 测试运算符检查
    Token opToken(TokenType::PLUS, "+", 1, 10);
    assert(opToken.isOperator());
    assert(!token1.isOperator());
    
    // 测试字符串表示
    std::string tokenStr = token1.toString();
    assert(tokenStr.find("IDENTIFIER") != std::string::npos);
    assert(tokenStr.find("variable") != std::string::npos);
    
    std::cout << "✓ Token tests passed!" << std::endl;
}

/**
 * @brief 测试关键字映射
 */
void testKeywordMapping() {
    std::cout << "Testing keyword mapping..." << std::endl;
    
    const auto& keywordMap = Token::getKeywordMap();
    
    // 测试几个关键字
    assert(keywordMap.at("if") == TokenType::IF);
    assert(keywordMap.at("while") == TokenType::WHILE);
    assert(keywordMap.at("int") == TokenType::INT);
    assert(keywordMap.at("true") == TokenType::TRUE);
    
    std::cout << "✓ Keyword mapping tests passed!" << std::endl;
}

/**
 * @brief 打印所有定义的Token类型（用于调试）
 */
void printTokenTypes() {
    std::cout << "\nDefined token types:" << std::endl;
    
    // 创建各种类型的token进行测试
    std::vector<std::pair<TokenType, std::string>> testTokens = {
        {TokenType::IDENTIFIER, "test"},
        {TokenType::NUMBER, "123"},
        {TokenType::IF, "if"},
        {TokenType::PLUS, "+"},
        {TokenType::ASSIGN, "="},
        {TokenType::SEMICOLON, ";"},
        {TokenType::LPAREN, "("},
        {TokenType::RBRACE, "}"}
    };
    
    for (const auto& [type, value] : testTokens) {
        Token token(type, value);
        std::cout << "  " << token.toString() << std::endl;
    }
}

int main() {
    std::cout << "=== Compiler Frontend Test Suite ===" << std::endl;
    
    try {
        testToken();
        testKeywordMapping();
        printTokenTypes();
        
        std::cout << "\n✅ All tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "\n❌ Test failed with unknown exception!" << std::endl;
        return 1;
    }
} 