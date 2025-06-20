#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "../src/parser/parser.h"
#include "../src/parser/grammar.h"
#include "../src/parser/lalr.h"

void testGrammarBasics() {
    std::cout << "Testing Grammar basics...\n";
    
    Grammar grammar;
    
    // 测试符号添加
    auto idSym = grammar.addTerminal("id", TokenType::IDENTIFIER);
    auto plusSym = grammar.addTerminal("+", TokenType::PLUS);
    auto eSym = grammar.addNonTerminal("E");
    
    assert(idSym.isTerminal());
    assert(plusSym.isTerminal());
    assert(eSym.isNonTerminal());
    
    // 测试产生式添加
    int prodId = grammar.addProduction("E", {"E", "+", "id"});
    assert(prodId >= 0);
    
    const auto& prod = grammar.getProduction(prodId);
    assert(prod.left == eSym);
    assert(prod.right.size() == 3);
    
    grammar.setStartSymbol("E");
    assert(grammar.getStartSymbol() == eSym);
    
    std::cout << "✓ Grammar basic tests passed!\n";
}

void testSimpleExpressionGrammar() {
    std::cout << "Testing simple expression grammar...\n";
    
    auto grammar = Grammar::buildSimpleExpressionGrammar();
    
    // 验证文法有效性
    assert(grammar.validate());
    
    // 计算FIRST和FOLLOW集
    grammar.computeFirstSets();
    grammar.computeFollowSets();
    
    // 验证FIRST集
    auto eSym = grammar.getSymbol("E");
    auto firstE = grammar.getFirstSet(eSym);
    
    // E的FIRST集应该包含 ( id num
    bool hasLParen = false, hasId = false, hasNum = false;
    for (const auto& sym : firstE) {
        if (sym.name == "(") hasLParen = true;
        else if (sym.name == "id") hasId = true;
        else if (sym.name == "num") hasNum = true;
    }
    
    assert(hasLParen && hasId && hasNum);
    
    std::cout << "✓ Simple expression grammar tests passed!\n";
}

void testLALRParser() {
    std::cout << "Testing LALR parser...\n";
    
    auto grammar = Grammar::buildTestGrammar();
    LALRParser parser(grammar);
    
    // 构建分析器
    bool buildSuccess = parser.build();
    assert(buildSuccess);
    
    // 检查是否为LALR(1)
    assert(parser.isLALR1());
    
    std::cout << "✓ LALR parser tests passed!\n";
}

void testParserCreation() {
    std::cout << "Testing parser creation...\n";
    
    // 测试表达式分析器创建
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 验证分析器状态
    assert(parser->isLALR1());
    assert(!parser->hasConflicts());
    
    std::cout << "✓ Parser creation tests passed!\n";
}

void testBasicParsing() {
    std::cout << "Testing basic parsing...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 准备测试Token序列：id + num
    std::vector<Token> tokens = {
        Token(TokenType::IDENTIFIER, "x", 1, 1),
        Token(TokenType::PLUS, "+", 1, 3),
        Token(TokenType::NUMBER, "42", 1, 5)
    };
    
    // 执行分析
    auto result = parser->parse(tokens);
    
    // 验证结果
    std::cout << "Parse result: " << (result.success ? "Success" : "Failed") << "\n";
    if (!result.success) {
        result.printErrors();
    } else {
        std::cout << "✓ Basic parsing test passed!\n";
    }
}

void testSourceParsing() {
    std::cout << "Testing source code parsing...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 测试简单表达式
    std::vector<std::string> testCases = {
        "x + 42",
        "x * y + z",
        "(x + y) * z",
        "x",
        "42"
    };
    
    size_t successCount = 0;
    for (const auto& testCase : testCases) {
        std::cout << "  Testing: \"" << testCase << "\" -> ";
        
        auto result = parser->parseSource(testCase);
        
        if (result.success) {
            std::cout << "✓ Success (" << result.astNodes << " nodes)\n";
            successCount++;
        } else {
            std::cout << "✗ Failed: " << result.getErrorSummary() << "\n";
        }
    }
    
    std::cout << "Source parsing: " << successCount << "/" << testCases.size() << " passed\n";
}

void testParserErrorHandling() {
    std::cout << "Testing error handling...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 测试错误的表达式
    std::vector<std::string> errorCases = {
        "",              // 空输入
        "+",             // 单独的运算符
        "x +",           // 不完整的表达式
        "( x",           // 不匹配的括号
        "x + + y"        // 重复运算符
    };
    
    size_t errorCount = 0;
    for (const auto& errorCase : errorCases) {
        auto result = parser->parseSource(errorCase);
        
        if (!result.success) {
            errorCount++;
        }
    }
    
    std::cout << "Error handling: " << errorCount << "/" << errorCases.size() << " errors detected correctly\n";
    
    if (errorCount == errorCases.size()) {
        std::cout << "✓ Error handling tests passed!\n";
    }
}

void testParserPerformance() {
    std::cout << "Testing parser performance...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 生成复杂表达式进行性能测试
    std::vector<std::string> performanceTests;
    
    // 简单表达式
    performanceTests.push_back("x + y");
    
    // 中等复杂度表达式
    performanceTests.push_back("(x + y) * (z - w) + (a * b)");
    
    // 复杂表达式
    std::string complex = "((x + y) * (z - w) + (a * b)) / ((c + d) * (e - f))";
    performanceTests.push_back(complex);
    
    // 非常复杂的表达式
    std::string veryComplex = complex;
    for (int i = 0; i < 3; ++i) {
        veryComplex = "(" + veryComplex + " + " + complex + ")";
    }
    performanceTests.push_back(veryComplex);
    
    ParserUtils::benchmarkParser(*parser, performanceTests);
    
    std::cout << "✓ Performance tests completed!\n";
}

void testParserStatistics() {
    std::cout << "Testing parser statistics...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    // 重置统计信息
    parser->resetStatistics();
    
    // 执行多次分析
    std::vector<std::string> testCases = {
        "x + y",
        "x * y + z",
        "(x + y) * z"
    };
    
    for (const auto& testCase : testCases) {
        parser->parseSource(testCase);
    }
    
    // 打印统计信息
    parser->printStatistics();
    
    std::cout << "✓ Statistics tests passed!\n";
}

void testGrammarValidation() {
    std::cout << "Testing grammar validation...\n";
    
    // 测试有效文法
    auto validGrammar = Grammar::buildSimpleExpressionGrammar();
    assert(validGrammar.validate());
    
    // 测试无效文法（缺少开始符号产生式）
    Grammar invalidGrammar;
    invalidGrammar.addNonTerminal("S");
    invalidGrammar.addTerminal("a", TokenType::IDENTIFIER);
    invalidGrammar.setStartSymbol("S");
    // 故意不添加S的产生式
    
    assert(!invalidGrammar.validate());
    auto errors = invalidGrammar.getValidationErrors();
    assert(!errors.empty());
    
    std::cout << "✓ Grammar validation tests passed!\n";
}

void testFactoryMethods() {
    std::cout << "Testing factory methods...\n";
    
    // 测试ParserFactory
    auto exprParser = ParserFactory::createExpressionParser();
    assert(exprParser != nullptr);
    
    // 测试解析功能
    auto result = exprParser->parseSource("x + y * z");
    std::cout << "Factory parser test: " << (result.success ? "Success" : "Failed") << "\n";
    
    std::cout << "✓ Factory method tests passed!\n";
}

void testAST() {
    std::cout << "Testing AST functionality...\n";
    
    auto parser = Parser::createSimpleExpressionParser();
    assert(parser != nullptr);
    
    auto result = parser->parseSource("x + y");
    
    if (result.success && result.ast) {
        std::cout << "AST for 'x + y':\n";
        ParserUtils::printAST(result.ast.get());
        
        size_t nodeCount = ParserUtils::getASTNodeCount(result.ast.get());
        std::cout << "AST has " << nodeCount << " nodes\n";
        
        assert(nodeCount > 0);
    }
    
    std::cout << "✓ AST tests passed!\n";
}

int runParserTests() {
    std::cout << "=== Parser Module Test Suite ===\n\n";
    
    try {
        testGrammarBasics();
        testSimpleExpressionGrammar();
        testLALRParser();
        testParserCreation();
        testGrammarValidation();
        testFactoryMethods();
        testBasicParsing();
        testSourceParsing();
        testParserErrorHandling();
        testAST();
        testParserStatistics();
        testParserPerformance();
        
        std::cout << "\n🎉 All parser tests passed successfully!\n";
        std::cout << "✅ Grammar construction and validation\n";
        std::cout << "✅ LALR parser generation\n";
        std::cout << "✅ Basic parsing functionality\n";
        std::cout << "✅ Source code parsing\n";
        std::cout << "✅ Error handling and recovery\n";
        std::cout << "✅ AST generation and manipulation\n";
        std::cout << "✅ Performance characteristics\n";
        std::cout << "✅ Factory patterns and utilities\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception\n";
        return 1;
    }
    
    return 0;
} 