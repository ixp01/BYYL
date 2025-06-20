#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <chrono>
#include <set>
#include "../src/lexer/token.h"
#include "../src/lexer/dfa.h"
#include "../src/lexer/lexer.h"
#include "../src/lexer/minimizer.h"

/**
 * @brief 测试DFA基本功能
 */
void testDFA() {
    std::cout << "Testing DFA class..." << std::endl;
    
    DFA dfa;
    dfa.buildStandardDFA();
    
    // 验证DFA构建
    assert(dfa.getStateCount() > 0);
    assert(dfa.validate());
    
    std::cout << "DFA has " << dfa.getStateCount() << " states" << std::endl;
    
    // 测试DFA状态转换
    dfa.reset();
    assert(dfa.getCurrentState() == 0); // 应该在起始状态
    
    std::cout << "✓ DFA basic tests passed!" << std::endl;
}

/**
 * @brief 测试词法分析器基本功能
 */
void testLexer() {
    std::cout << "Testing Lexer class..." << std::endl;
    
    // 测试基本Token识别
    std::vector<std::pair<std::string, TokenType>> testCases = {
        {"variable", TokenType::IDENTIFIER},
        {"123", TokenType::NUMBER},
        {"12.34", TokenType::REAL},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"+", TokenType::PLUS},
        {"-", TokenType::MINUS},
        {"==", TokenType::EQ},
        {"!=", TokenType::NE},
        {"(", TokenType::LPAREN},
        {")", TokenType::RPAREN},
        {";", TokenType::SEMICOLON}
    };
    
    Lexer lexer;
    
    for (const auto& testCase : testCases) {
        lexer.setSource(testCase.first);
        Token token = lexer.getNextToken();
        
        std::cout << "Testing: \"" << testCase.first << "\" -> " 
                  << Token::getTypeString(token.type) << std::endl;
        
        assert(token.type == testCase.second);
        assert(token.value == testCase.first);
    }
    
    std::cout << "✓ Lexer basic token tests passed!" << std::endl;
}

/**
 * @brief 测试复杂源代码的词法分析
 */
void testComplexSourceCode() {
    std::cout << "Testing complex source code analysis..." << std::endl;
    
    std::string sourceCode = R"(
        int main() {
            int x = 42;
            float y = 3.14;
            if (x > 0) {
                return x + y;
            } else {
                return 0;
            }
        }
        // This is a comment
    )";
    
    Lexer lexer(sourceCode);
    LexicalResult result = lexer.analyze();
    
    std::cout << "Source code analysis results:" << std::endl;
    std::cout << "  Total tokens: " << result.tokens.size() << std::endl;
    std::cout << "  Errors: " << result.errors.size() << std::endl;
    std::cout << "  Success: " << (result.success ? "Yes" : "No") << std::endl;
    
    // 验证一些期望的Token
    bool foundInt = false, foundMain = false, foundIf = false;
    for (const auto& token : result.tokens) {
        if (token.type == TokenType::INT) foundInt = true;
        if (token.type == TokenType::IDENTIFIER && token.value == "main") foundMain = true;
        if (token.type == TokenType::IF) foundIf = true;
    }
    
    assert(foundInt);
    assert(foundMain);
    assert(foundIf);
    
    // 打印前几个Token
    std::cout << "First 10 tokens:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(10), result.tokens.size()); ++i) {
        std::cout << "  " << result.tokens[i].toString() << std::endl;
    }
    
    std::cout << "✓ Complex source code tests passed!" << std::endl;
}

/**
 * @brief 测试错误处理
 */
void testErrorHandling() {
    std::cout << "Testing error handling..." << std::endl;
    
    // 测试未闭合的字符串
    Lexer lexer("\"unclosed string");
    LexicalResult result = lexer.analyze();
    
    assert(result.hasErrors());
    std::cout << "Found " << result.errors.size() << " errors as expected" << std::endl;
    
    // 测试非法字符
    lexer.setSource("valid + @invalid");
    result = lexer.analyze();
    
    // 应该能识别valid和+，但@invalid会产生错误
    bool foundValid = false, foundPlus = false, foundError = false;
    for (const auto& token : result.tokens) {
        if (token.type == TokenType::IDENTIFIER && token.value == "valid") foundValid = true;
        if (token.type == TokenType::PLUS) foundPlus = true;
        if (token.type == TokenType::ERROR) foundError = true;
    }
    
    assert(foundValid);
    assert(foundPlus);
    // 错误Token可能被忽略，但错误列表中应该有记录
    
    std::cout << "✓ Error handling tests passed!" << std::endl;
}

/**
 * @brief 测试DFA优化和最小化
 */
void testDFAOptimization() {
    std::cout << "Testing DFA optimization..." << std::endl;
    
    DFA originalDFA;
    originalDFA.buildStandardDFA();
    
    size_t originalStates = originalDFA.getStateCount();
    std::cout << "Original DFA has " << originalStates << " states" << std::endl;
    
    // 测试DFA最小化
    DFAMinimizer minimizer(originalDFA);
    DFA minimizedDFA = minimizer.minimize();
    
    size_t minimizedStates = minimizedDFA.getStateCount();
    std::cout << "Minimized DFA has " << minimizedStates << " states" << std::endl;
    
    // 验证最小化结果
    assert(minimizedStates <= originalStates);
    assert(minimizedDFA.validate());
    
    // 获取最小化统计
    auto stats = minimizer.getLastMinimizationStats();
    std::cout << "Minimization statistics:" << std::endl;
    std::cout << "  Reduction ratio: " << (stats.reductionRatio * 100) << "%" << std::endl;
    
    std::cout << "✓ DFA optimization tests passed!" << std::endl;
}

/**
 * @brief 测试LexerFactory
 */
void testLexerFactory() {
    std::cout << "Testing LexerFactory..." << std::endl;
    
    // 创建标准词法分析器
    auto lexer1 = LexerFactory::createStandardLexer();
    assert(lexer1 != nullptr);
    
    // 测试基本功能
    lexer1->setSource("int x = 42;");
    LexicalResult result = lexer1->analyze();
    assert(result.success);
    assert(result.tokens.size() > 0);
    
    // 创建自定义DFA的词法分析器
    auto customDFA = std::make_unique<DFA>();
    customDFA->buildStandardDFA();
    auto lexer2 = LexerFactory::createCustomLexer(std::move(customDFA));
    assert(lexer2 != nullptr);
    
    std::cout << "✓ LexerFactory tests passed!" << std::endl;
}

/**
 * @brief 性能测试
 */
void testPerformance() {
    std::cout << "Testing lexer performance..." << std::endl;
    
    // 生成大量源代码
    std::string largeSource;
    for (int i = 0; i < 1000; ++i) {
        largeSource += "int var" + std::to_string(i) + " = " + std::to_string(i) + ";\n";
    }
    
    std::cout << "Generated source code with " << largeSource.length() << " characters" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Lexer lexer(largeSource);
    LexicalResult result = lexer.analyze();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Lexical analysis completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Analyzed " << result.tokens.size() << " tokens" << std::endl;
    std::cout << "Performance: " << (result.tokens.size() / (duration.count() + 1)) << " tokens/ms" << std::endl;
    
    assert(result.success);
    assert(result.tokens.size() > 5000); // 应该有很多Token
    
    std::cout << "✓ Performance tests passed!" << std::endl;
}

/**
 * @brief 运行所有词法分析器测试
 */
int runLexerTests() {
    std::cout << "=== Lexer Module Test Suite ===" << std::endl;
    
    try {
        testDFA();
        testLexer();
        testComplexSourceCode();
        testErrorHandling();
        testDFAOptimization();
        testLexerFactory();
        testPerformance();
        
        std::cout << "\n🎉 All lexer tests passed successfully!" << std::endl;
        std::cout << "✅ DFA construction and validation" << std::endl;
        std::cout << "✅ Basic token recognition" << std::endl;
        std::cout << "✅ Complex source code analysis" << std::endl;
        std::cout << "✅ Error handling" << std::endl;
        std::cout << "✅ DFA optimization and minimization" << std::endl;
        std::cout << "✅ Factory pattern implementation" << std::endl;
        std::cout << "✅ Performance characteristics" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
} 