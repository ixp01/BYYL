#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <vector>
#include <string>
#include "grammar.h"
#include "lalr.h"
#include "ast.h"
#include "../lexer/token.h"
#include "../lexer/lexer.h"

/**
 * @brief 语法分析结果类
 */
class ParseResult {
public:
    std::unique_ptr<ASTNode> ast;           // 语法树
    std::vector<std::string> errors;        // 错误信息
    bool success;                           // 是否成功
    
    // 分析统计信息
    size_t numTokens;                       // Token数量
    size_t parseTime;                       // 分析耗时(ms)
    size_t astNodes;                        // AST节点数量
    
    ParseResult() : success(false), numTokens(0), parseTime(0), astNodes(0) {}
    
    bool hasErrors() const { return !errors.empty(); }
    void addError(const std::string& error) { 
        errors.push_back(error); 
        success = false; 
    }
    
    void printErrors() const;
    void printStatistics() const;
    std::string getErrorSummary() const;
};

/**
 * @brief 语法分析器配置
 */
struct ParserConfig {
    bool enableErrorRecovery = true;        // 启用错误恢复
    bool generateFullAST = true;            // 生成完整AST
    bool enableOptimizations = true;        // 启用优化
    bool verboseLogging = false;            // 详细日志
    size_t maxErrors = 10;                  // 最大错误数
    
    // 调试选项
    bool printGrammar = false;              // 打印文法
    bool printParsingTable = false;         // 打印分析表
    bool printAutomaton = false;            // 打印自动机
};

/**
 * @brief 主语法分析器类
 */
class Parser {
private:
    std::unique_ptr<Grammar> grammar;       // 文法
    std::unique_ptr<LALRParser> lalrParser; // LALR分析器
    ParserConfig config;                    // 配置
    
    // 统计信息
    mutable size_t totalTokensParsed;
    mutable size_t totalParseTime;
    mutable size_t successfulParses;
    mutable size_t failedParses;

public:
    explicit Parser(std::unique_ptr<Grammar> g, const ParserConfig& cfg = ParserConfig());
    ~Parser() = default;
    
    // 禁用拷贝和赋值
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    
    // 构建分析器
    bool build();
    
    // 分析Token序列
    ParseResult parse(const std::vector<Token>& tokens);
    
    // 分析源代码字符串（集成词法分析）
    ParseResult parseSource(const std::string& source);
    
    // 分析源代码文件
    ParseResult parseFile(const std::string& filename);
    
    // 文法验证
    bool validateGrammar() const;
    std::vector<std::string> getGrammarErrors() const;
    
    // 检查文法类型
    bool isLALR1() const;
    bool hasConflicts() const;
    std::vector<std::string> getConflicts() const;
    
    // 配置管理
    void setConfig(const ParserConfig& cfg) { config = cfg; }
    const ParserConfig& getConfig() const { return config; }
    
    // 调试功能
    void printGrammar() const;
    void printFirstSets() const;
    void printFollowSets() const;
    void printParsingTable() const;
    void printAutomaton() const;
    
    // 统计信息
    void printStatistics() const;
    void resetStatistics();
    
    // 获取内部组件
    const Grammar* getGrammar() const { return grammar.get(); }
    const LALRParser* getLALRParser() const { return lalrParser.get(); }
    
    // 工厂方法
    static std::unique_ptr<Parser> createSimpleExpressionParser(const ParserConfig& config = ParserConfig());
    static std::unique_ptr<Parser> createStandardCParser(const ParserConfig& config = ParserConfig());
    static std::unique_ptr<Parser> createFromGrammarFile(const std::string& filename, const ParserConfig& config = ParserConfig());
    
private:
    // 内部辅助方法
    std::unique_ptr<Lexer> createLexer() const;
    void updateStatistics(const ParseResult& result) const;
    size_t countASTNodes(const ASTNode* node) const;
    void logInfo(const std::string& message) const;
    void logError(const std::string& message) const;
};

/**
 * @brief 语法分析器工厂类
 */
class ParserFactory {
public:
    // 预定义分析器创建
    static std::unique_ptr<Parser> createExpressionParser();
    static std::unique_ptr<Parser> createArithmeticParser();
    static std::unique_ptr<Parser> createSimpleLanguageParser();
    
    // 从文法定义创建
    static std::unique_ptr<Parser> createFromGrammar(std::unique_ptr<Grammar> grammar);
    static std::unique_ptr<Parser> createFromGrammarSpec(const std::string& grammarSpec);
    
    // 验证和测试
    static bool validateParser(const Parser& parser, const std::vector<std::string>& testCases);
    static void runParserTests(const Parser& parser);
    
private:
    static Grammar buildExpressionGrammar();
    static Grammar buildArithmeticGrammar();
    static Grammar buildSimpleLanguageGrammar();
};

/**
 * @brief 语法分析器辅助工具
 */
class ParserUtils {
public:
    // AST工具
    static void printAST(const ASTNode* root, int indent = 0);
    static std::string astToString(const ASTNode* root);
    static size_t getASTDepth(const ASTNode* root);
    static size_t getASTNodeCount(const ASTNode* root);
    
    // Token工具
    static std::vector<Token> tokenizeString(const std::string& source);
    static std::string tokensToString(const std::vector<Token>& tokens);
    
    // 文法工具
    static bool isLeftRecursive(const Grammar& grammar);
    static Grammar eliminateLeftRecursion(const Grammar& grammar);
    static Grammar leftFactor(const Grammar& grammar);
    
    // 分析工具
    static std::vector<std::string> getParseTrace(const Parser& parser, const std::vector<Token>& tokens);
    static void compareASTs(const ASTNode* ast1, const ASTNode* ast2);
    
    // 性能测试
    static void benchmarkParser(const Parser& parser, const std::vector<std::string>& testCases);
    static void profileMemoryUsage(const Parser& parser, const std::string& source);
};

/**
 * @brief 错误恢复策略
 */
class ErrorRecoveryStrategy {
public:
    virtual ~ErrorRecoveryStrategy() = default;
    
    // 恢复策略接口
    virtual bool recover(Parser& parser, const std::string& error) = 0;
    virtual void reset() = 0;
    
    // 预定义策略
    static std::unique_ptr<ErrorRecoveryStrategy> createPanicMode();
    static std::unique_ptr<ErrorRecoveryStrategy> createSynchronization();
    static std::unique_ptr<ErrorRecoveryStrategy> createErrorProduction();
};

/**
 * @brief 语法分析器性能监控
 */
class ParserProfiler {
private:
    struct ProfileData {
        size_t parseCount = 0;
        size_t totalTime = 0;
        size_t totalTokens = 0;
        size_t totalNodes = 0;
        size_t maxTime = 0;
        size_t minTime = SIZE_MAX;
    };
    
    ProfileData data;
    bool enabled;

public:
    ParserProfiler() : enabled(false) {}
    
    void enable() { enabled = true; }
    void disable() { enabled = false; }
    void reset() { data = ProfileData(); }
    
    void recordParse(size_t time, size_t tokens, size_t nodes);
    void printReport() const;
    
    double getAverageTime() const;
    double getTokensPerSecond() const;
    double getNodesPerSecond() const;
};

#endif // PARSER_H 