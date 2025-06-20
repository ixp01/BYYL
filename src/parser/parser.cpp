#include "parser.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>

// ParseResult类实现
void ParseResult::printErrors() const {
    std::cout << "Parse Errors (" << errors.size() << "):\n";
    for (size_t i = 0; i < errors.size(); ++i) {
        std::cout << "  " << (i + 1) << ": " << errors[i] << "\n";
    }
}

void ParseResult::printStatistics() const {
    std::cout << "Parse Statistics:\n";
    std::cout << "  Success: " << (success ? "Yes" : "No") << "\n";
    std::cout << "  Tokens: " << numTokens << "\n";
    std::cout << "  Parse Time: " << parseTime << " ms\n";
    std::cout << "  AST Nodes: " << astNodes << "\n";
    if (parseTime > 0) {
        std::cout << "  Speed: " << (numTokens * 1000 / parseTime) << " tokens/second\n";
    }
}

std::string ParseResult::getErrorSummary() const {
    if (errors.empty()) {
        return "No errors";
    }
    
    std::ostringstream oss;
    oss << errors.size() << " error(s): ";
    for (size_t i = 0; i < std::min(errors.size(), size_t(3)); ++i) {
        if (i > 0) oss << "; ";
        oss << errors[i];
    }
    if (errors.size() > 3) {
        oss << "; ...";
    }
    return oss.str();
}

// Parser类实现
Parser::Parser(std::unique_ptr<Grammar> g, const ParserConfig& cfg)
    : grammar(std::move(g)), config(cfg), totalTokensParsed(0), 
      totalParseTime(0), successfulParses(0), failedParses(0) {
}

bool Parser::build() {
    if (!grammar) {
        logError("No grammar provided");
        return false;
    }
    
    // 验证文法
    if (!validateGrammar()) {
        logError("Grammar validation failed");
        return false;
    }
    
    if (config.printGrammar) {
        printGrammar();
    }
    
    // 创建LALR分析器
    lalrParser = std::make_unique<LALRParser>(*grammar);
    
    // 构建分析器
    if (!lalrParser->build()) {
        logError("Failed to build LALR parser");
        if (lalrParser->hasErrors()) {
            auto errors = lalrParser->getErrors();
            for (const auto& error : errors) {
                logError("  " + error);
            }
        }
        return false;
    }
    
    if (config.printParsingTable) {
        printParsingTable();
    }
    
    if (config.printAutomaton) {
        printAutomaton();
    }
    
    logInfo("Parser built successfully");
    return true;
}

ParseResult Parser::parse(const std::vector<Token>& tokens) {
    auto start = std::chrono::high_resolution_clock::now();
    
    ParseResult result;
    result.numTokens = tokens.size();
    
    if (!lalrParser) {
        result.addError("Parser not built");
        return result;
    }
    
    try {
        // 执行分析
        result.ast = lalrParser->parse(tokens);
        
        if (result.ast) {
            result.success = true;
            result.astNodes = countASTNodes(result.ast.get());
            successfulParses++;
        } else {
            result.success = false;
            failedParses++;
            
            // 收集错误信息
            if (lalrParser->hasErrors()) {
                auto errors = lalrParser->getErrors();
                for (const auto& error : errors) {
                    result.addError(error);
                }
            } else {
                result.addError("Unknown parsing error");
            }
        }
    } catch (const std::exception& e) {
        result.addError("Exception during parsing: " + std::string(e.what()));
        failedParses++;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    result.parseTime = duration.count();
    
    updateStatistics(result);
    
    if (config.verboseLogging) {
        std::cout << "Parse completed: " << (result.success ? "Success" : "Failed") 
                  << " (" << result.parseTime << "ms)\n";
    }
    
    return result;
}

ParseResult Parser::parseSource(const std::string& source) {
    auto lexer = createLexer();
    lexer->setSource(source);
    
    auto lexResult = lexer->analyze();
    
    ParseResult result;
    
    if (lexResult.hasErrors()) {
        for (const auto& error : lexResult.errors) {
            result.addError("Lexical error: " + error.toString());
        }
        return result;
    }
    
    return parse(lexResult.tokens);
}

ParseResult Parser::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        ParseResult result;
        result.addError("Cannot open file: " + filename);
        return result;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    return parseSource(source);
}

bool Parser::validateGrammar() const {
    if (!grammar) {
        return false;
    }
    
    return grammar->validate();
}

std::vector<std::string> Parser::getGrammarErrors() const {
    if (!grammar) {
        return {"No grammar provided"};
    }
    
    return grammar->getValidationErrors();
}

bool Parser::isLALR1() const {
    return lalrParser && lalrParser->isLALR1();
}

bool Parser::hasConflicts() const {
    if (!lalrParser) {
        return true;
    }
    
    const auto* table = lalrParser->getParsingTable();
    return table && table->hasConflicts();
}

std::vector<std::string> Parser::getConflicts() const {
    if (!lalrParser) {
        return {"Parser not built"};
    }
    
    const auto* table = lalrParser->getParsingTable();
    return table ? table->getConflicts() : std::vector<std::string>{};
}

void Parser::printGrammar() const {
    if (grammar) {
        grammar->print();
    }
}

void Parser::printFirstSets() const {
    if (grammar) {
        grammar->printFirstSets();
    }
}

void Parser::printFollowSets() const {
    if (grammar) {
        grammar->printFollowSets();
    }
}

void Parser::printParsingTable() const {
    if (lalrParser) {
        lalrParser->printParsingTable();
    }
}

void Parser::printAutomaton() const {
    if (lalrParser) {
        lalrParser->printAutomaton();
    }
}

void Parser::printStatistics() const {
    std::cout << "Parser Statistics:\n";
    std::cout << "  Total parses: " << (successfulParses + failedParses) << "\n";
    std::cout << "  Successful: " << successfulParses << "\n";
    std::cout << "  Failed: " << failedParses << "\n";
    std::cout << "  Success rate: " << 
        (successfulParses + failedParses > 0 ? 
         (100.0 * successfulParses / (successfulParses + failedParses)) : 0.0) << "%\n";
    std::cout << "  Total tokens: " << totalTokensParsed << "\n";
    std::cout << "  Total time: " << totalParseTime << " ms\n";
    
    if (totalParseTime > 0) {
        std::cout << "  Average speed: " << 
            (totalTokensParsed * 1000 / totalParseTime) << " tokens/second\n";
    }
}

void Parser::resetStatistics() {
    totalTokensParsed = 0;
    totalParseTime = 0;
    successfulParses = 0;
    failedParses = 0;
}

std::unique_ptr<Parser> Parser::createSimpleExpressionParser(const ParserConfig& config) {
    auto grammar = std::make_unique<Grammar>(Grammar::buildSimpleExpressionGrammar());
    auto parser = std::make_unique<Parser>(std::move(grammar), config);
    
    if (!parser->build()) {
        return nullptr;
    }
    
    return parser;
}

std::unique_ptr<Lexer> Parser::createLexer() const {
    return LexerFactory::createStandardLexer();
}

void Parser::updateStatistics(const ParseResult& result) const {
    totalTokensParsed += result.numTokens;
    totalParseTime += result.parseTime;
}

size_t Parser::countASTNodes(const ASTNode* node) const {
    return ParserUtils::getASTNodeCount(node);
}

void Parser::logInfo(const std::string& message) const {
    if (config.verboseLogging) {
        std::cout << "[INFO] " << message << "\n";
    }
}

void Parser::logError(const std::string& message) const {
    std::cerr << "[ERROR] " << message << "\n";
}

// ParserFactory类实现
std::unique_ptr<Parser> ParserFactory::createExpressionParser() {
    auto grammar = std::make_unique<Grammar>(buildExpressionGrammar());
    return createFromGrammar(std::move(grammar));
}

std::unique_ptr<Parser> ParserFactory::createFromGrammar(std::unique_ptr<Grammar> grammar) {
    auto parser = std::make_unique<Parser>(std::move(grammar));
    
    if (!parser->build()) {
        return nullptr;
    }
    
    return parser;
}

Grammar ParserFactory::buildExpressionGrammar() {
    return Grammar::buildSimpleExpressionGrammar();
}

Grammar ParserFactory::buildArithmeticGrammar() {
    Grammar grammar;
    
    // 构建算术表达式文法
    // E -> E + T | E - T | T
    // T -> T * F | T / F | F
    // F -> (E) | number | id
    
    grammar.addTerminal("+", TokenType::PLUS);
    grammar.addTerminal("-", TokenType::MINUS);
    grammar.addTerminal("*", TokenType::MULTIPLY);
    grammar.addTerminal("/", TokenType::DIVIDE);
    grammar.addTerminal("(", TokenType::LPAREN);
    grammar.addTerminal(")", TokenType::RPAREN);
    grammar.addTerminal("number", TokenType::NUMBER);
    grammar.addTerminal("id", TokenType::IDENTIFIER);
    
    grammar.addNonTerminal("E");
    grammar.addNonTerminal("T");
    grammar.addNonTerminal("F");
    
    grammar.addProduction("E", {"E", "+", "T"});
    grammar.addProduction("E", {"E", "-", "T"});
    grammar.addProduction("E", {"T"});
    
    grammar.addProduction("T", {"T", "*", "F"});
    grammar.addProduction("T", {"T", "/", "F"});
    grammar.addProduction("T", {"F"});
    
    grammar.addProduction("F", {"(", "E", ")"});
    grammar.addProduction("F", {"number"});
    grammar.addProduction("F", {"id"});
    
    grammar.setStartSymbol("E");
    
    return grammar;
}

// ParserUtils类实现
void ParserUtils::printAST(const ASTNode* root, int indent) {
    if (!root) {
        return;
    }
    
    // 打印缩进
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
    
    // 打印节点信息
    std::cout << root->getNodeTypeString();
    
    // 根据节点类型打印额外信息
    if (auto identifier = dynamic_cast<const IdentifierNode*>(root)) {
        std::cout << " (" << identifier->name << ")";
    } else if (auto literal = dynamic_cast<const LiteralNode*>(root)) {
        std::cout << " (" << literal->value << ")";
    }
    
    std::cout << "\n";
    
    // 递归打印子节点
    root->print(indent);
}

std::string ParserUtils::astToString(const ASTNode* root) {
    std::ostringstream oss;
    // 简化实现，实际需要根据节点类型来转换
    if (auto identifier = dynamic_cast<const IdentifierNode*>(root)) {
        oss << identifier->name;
    } else if (auto literal = dynamic_cast<const LiteralNode*>(root)) {
        oss << literal->value;
    } else {
        oss << "Node";
    }
    return oss.str();
}

size_t ParserUtils::getASTNodeCount(const ASTNode* root) {
    if (!root) {
        return 0;
    }
    
    size_t count = 1; // 当前节点
    
    // 根据节点类型递归计算子节点
    if (auto binary = dynamic_cast<const BinaryExprNode*>(root)) {
        count += getASTNodeCount(binary->left.get());
        count += getASTNodeCount(binary->right.get());
    } else if (auto unary = dynamic_cast<const UnaryExprNode*>(root)) {
        count += getASTNodeCount(unary->operand.get());
    } else if (auto ifStmt = dynamic_cast<const IfStmtNode*>(root)) {
        count += getASTNodeCount(ifStmt->condition.get());
        count += getASTNodeCount(ifStmt->thenStmt.get());
        if (ifStmt->elseStmt) {
            count += getASTNodeCount(ifStmt->elseStmt.get());
        }
    } else if (auto whileStmt = dynamic_cast<const WhileStmtNode*>(root)) {
        count += getASTNodeCount(whileStmt->condition.get());
        count += getASTNodeCount(whileStmt->body.get());
    } else if (auto block = dynamic_cast<const BlockStmtNode*>(root)) {
        for (const auto& stmt : block->statements) {
            count += getASTNodeCount(stmt.get());
        }
    }
    
    return count;
}

std::vector<Token> ParserUtils::tokenizeString(const std::string& source) {
    auto lexer = LexerFactory::createStandardLexer();
    lexer->setSource(source);
    auto result = lexer->analyze();
    return result.tokens;
}

void ParserUtils::benchmarkParser(const Parser& parser, const std::vector<std::string>& testCases) {
    std::cout << "Parser Benchmark Results:\n";
    std::cout << "========================\n";
    
    size_t totalTime = 0;
    size_t totalTokens = 0;
    size_t successCount = 0;
    
    for (size_t i = 0; i < testCases.size(); ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = const_cast<Parser&>(parser).parseSource(testCases[i]);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Test " << (i + 1) << ": ";
        if (result.success) {
            std::cout << "✓ " << duration.count() << "μs (" << result.numTokens << " tokens)\n";
            successCount++;
        } else {
            std::cout << "✗ " << duration.count() << "μs (FAILED)\n";
        }
        
        totalTime += duration.count();
        totalTokens += result.numTokens;
    }
    
    std::cout << "\nSummary:\n";
    std::cout << "  Total tests: " << testCases.size() << "\n";
    std::cout << "  Successful: " << successCount << " (" << 
        (100.0 * successCount / testCases.size()) << "%)\n";
    std::cout << "  Total time: " << totalTime << " μs\n";
    std::cout << "  Average time: " << (totalTime / testCases.size()) << " μs/test\n";
    
    if (totalTime > 0) {
        std::cout << "  Throughput: " << (totalTokens * 1000000 / totalTime) << " tokens/second\n";
    }
} 