#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <vector>
#include <string>
#include <memory>
#include "symbol_table.h"
#include "../parser/ast.h"

/**
 * @brief 语义错误类型枚举
 */
enum class SemanticErrorType {
    UNDEFINED_VARIABLE,         // 未定义的变量
    UNDEFINED_FUNCTION,         // 未定义的函数
    REDEFINED_VARIABLE,         // 重复定义的变量
    REDEFINED_FUNCTION,         // 重复定义的函数
    TYPE_MISMATCH,              // 类型不匹配
    INVALID_ASSIGNMENT,         // 无效的赋值
    INVALID_OPERATION,          // 无效的运算
    FUNCTION_CALL_ERROR,        // 函数调用错误
    PARAMETER_COUNT_MISMATCH,   // 参数数量不匹配
    PARAMETER_TYPE_MISMATCH,    // 参数类型不匹配
    RETURN_TYPE_MISMATCH,       // 返回类型不匹配
    UNINITIALIZED_VARIABLE,     // 使用未初始化的变量
    UNREACHABLE_CODE,           // 不可达代码
    MISSING_RETURN,             // 缺少返回语句
    DIVISION_BY_ZERO,           // 除零错误
    ARRAY_INDEX_ERROR,          // 数组索引错误
    SCOPE_ERROR                 // 作用域错误
};

/**
 * @brief 语义错误信息
 */
struct SemanticError {
    SemanticErrorType type;     // 错误类型
    std::string message;        // 错误消息
    int line;                   // 错误行号
    int column;                 // 错误列号
    std::string context;        // 错误上下文
    
    // 默认构造函数
    SemanticError() : type(SemanticErrorType::SCOPE_ERROR), message(""), line(0), column(0), context("") {}
    
    SemanticError(SemanticErrorType t, const std::string& msg, 
                  int l = 0, int c = 0, const std::string& ctx = "")
        : type(t), message(msg), line(l), column(c), context(ctx) {}
    
    std::string toString() const;
    std::string getTypeString() const;
};

/**
 * @brief 语义分析结果
 */
struct SemanticAnalysisResult {
    bool success;                           // 分析是否成功
    std::vector<SemanticError> errors;      // 错误列表
    std::vector<SemanticError> warnings;    // 警告列表
    std::unique_ptr<SymbolTable> symbolTable; // 符号表
    
    // 统计信息
    int totalSymbols;                       // 总符号数
    int totalScopes;                        // 总作用域数
    int analysisTimeMs;                     // 分析时间(毫秒)
    
    SemanticAnalysisResult() 
        : success(false), symbolTable(std::make_unique<SymbolTable>()),
          totalSymbols(0), totalScopes(0), analysisTimeMs(0) {}
    
    void addError(const SemanticError& error) {
        errors.push_back(error);
        success = false;
    }
    
    void addWarning(const SemanticError& warning) {
        warnings.push_back(warning);
    }
    
    bool hasErrors() const { return !errors.empty(); }
    bool hasWarnings() const { return !warnings.empty(); }
    
    std::string getSummary() const;
};

/**
 * @brief 语义分析器配置
 */
struct SemanticAnalyzerConfig {
    bool checkUnusedVariables;      // 检查未使用的变量
    bool checkUninitializedVars;    // 检查未初始化的变量
    bool checkTypeConversion;       // 检查类型转换
    bool checkFunctionCalls;        // 检查函数调用
    bool checkArrayBounds;          // 检查数组边界
    bool warningsAsErrors;          // 将警告当作错误
    bool strictTypeChecking;        // 严格类型检查
    
    SemanticAnalyzerConfig()
        : checkUnusedVariables(true), checkUninitializedVars(true),
          checkTypeConversion(true), checkFunctionCalls(true),
          checkArrayBounds(false), warningsAsErrors(false),
          strictTypeChecking(false) {}
};

/**
 * @brief 表达式类型信息
 */
struct ExpressionType {
    DataType dataType;          // 数据类型
    bool isLValue;              // 是否为左值
    bool isConstant;            // 是否为常量
    std::string constantValue;  // 常量值（如果是常量）
    
    ExpressionType(DataType dt = DataType::UNKNOWN, bool lval = false, bool cnst = false)
        : dataType(dt), isLValue(lval), isConstant(cnst) {}
};

/**
 * @brief 语义分析器主类
 */
class SemanticAnalyzer {
private:
    std::unique_ptr<SymbolTable> symbolTable;   // 符号表
    SemanticAnalyzerConfig config;              // 配置
    std::vector<SemanticError> errors;          // 错误列表
    std::vector<SemanticError> warnings;        // 警告列表
    
    // 分析状态
    DataType currentFunctionReturnType;         // 当前函数返回类型
    bool inFunction;                            // 是否在函数内部
    bool hasReturnStatement;                    // 是否有返回语句
    
public:
    SemanticAnalyzer(const SemanticAnalyzerConfig& cfg = SemanticAnalyzerConfig());
    ~SemanticAnalyzer() = default;
    
    /**
     * @brief 分析AST
     */
    SemanticAnalysisResult analyze(ASTNode* root);
    
    /**
     * @brief 分析程序节点
     */
    void analyzeProgram(ProgramNode* node);
    
    /**
     * @brief 分析声明节点
     */
    void analyzeDeclaration(ASTNode* node);
    
    /**
     * @brief 分析变量声明
     */
    void analyzeVariableDecl(VariableDeclNode* node);
    
    /**
     * @brief 分析函数声明
     */
    void analyzeFunctionDecl(FunctionDeclNode* node);
    
    /**
     * @brief 分析语句节点
     */
    void analyzeStatement(StmtNode* node);
    
    /**
     * @brief 分析赋值语句
     */
    void analyzeAssignmentStmt(AssignmentStmtNode* node);
    
    /**
     * @brief 分析if语句
     */
    void analyzeIfStmt(IfStmtNode* node);
    
    /**
     * @brief 分析while语句
     */
    void analyzeWhileStmt(WhileStmtNode* node);
    
    /**
     * @brief 分析复合语句
     */
    void analyzeBlockStmt(BlockStmtNode* node);
    
    /**
     * @brief 分析返回语句
     */
    void analyzeReturnStmt(ReturnStmtNode* node);
    
    /**
     * @brief 分析表达式语句
     */
    void analyzeExpressionStmt(ExpressionStmtNode* node);
    
    /**
     * @brief 分析表达式节点
     */
    ExpressionType analyzeExpression(ExprNode* node);
    
    /**
     * @brief 分析二元表达式
     */
    ExpressionType analyzeBinaryExpression(BinaryExprNode* node);
    
    /**
     * @brief 分析一元表达式
     */
    ExpressionType analyzeUnaryExpression(UnaryExprNode* node);
    
    /**
     * @brief 分析标识符
     */
    ExpressionType analyzeIdentifier(IdentifierNode* node);
    
    /**
     * @brief 分析字面量
     */
    ExpressionType analyzeLiteral(LiteralNode* node);
    
    /**
     * @brief 类型检查
     */
    bool checkTypeCompatibility(DataType expected, DataType actual, 
                               const std::string& context, int line, int col);
    
    /**
     * @brief 添加错误
     */
    void addError(SemanticErrorType type, const std::string& message,
                  int line = 0, int col = 0, const std::string& context = "");
    
    /**
     * @brief 添加警告
     */
    void addWarning(SemanticErrorType type, const std::string& message,
                    int line = 0, int col = 0, const std::string& context = "");
    
    /**
     * @brief 检查未使用的变量
     */
    void checkUnusedVariables();
    
    /**
     * @brief 检查未初始化的变量
     */
    void checkUninitializedVariables();
    
    /**
     * @brief 获取符号表
     */
    SymbolTable* getSymbolTable() const;
    
    /**
     * @brief 获取错误列表
     */
    const std::vector<SemanticError>& getErrors() const;
    
    /**
     * @brief 获取警告列表
     */
    const std::vector<SemanticError>& getWarnings() const;
    
    /**
     * @brief 清空分析状态
     */
    void clear();
    
    /**
     * @brief 设置配置
     */
    void setConfig(const SemanticAnalyzerConfig& cfg);

private:
    /**
     * @brief 获取复合赋值运算符对应的二元运算符
     */
    TokenType getCorrespondingBinaryOp(TokenType compoundAssignOp) const;
};

/**
 * @brief 语义分析器工厂类
 */
class SemanticAnalyzerFactory {
public:
    /**
     * @brief 创建标准语义分析器
     */
    static std::unique_ptr<SemanticAnalyzer> createStandard();
    
    /**
     * @brief 创建严格语义分析器
     */
    static std::unique_ptr<SemanticAnalyzer> createStrict();
    
    /**
     * @brief 创建宽松语义分析器
     */
    static std::unique_ptr<SemanticAnalyzer> createPermissive();
};

#endif // SEMANTIC_ANALYZER_H 