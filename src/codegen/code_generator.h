#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "intermediate_code.h"
#include "../parser/ast.h"
#include "../semantic/symbol_table.h"
#include <memory>
#include <string>
#include <unordered_map>

/**
 * @brief 代码生成器配置
 */
struct CodeGenConfig {
    bool enableOptimization;        // 启用优化
    bool generateComments;          // 生成注释
    bool enableConstantFolding;     // 启用常量折叠
    bool enableDeadCodeElim;        // 启用死代码消除
    
    CodeGenConfig() 
        : enableOptimization(true), generateComments(true),
          enableConstantFolding(true), enableDeadCodeElim(true) {}
};

/**
 * @brief 代码生成结果
 */
struct CodeGenResult {
    std::unique_ptr<IntermediateCode> intermediateCode;
    bool success;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    IntermediateCode::Statistics statistics;
    
    CodeGenResult() : success(false) {}
    
    std::string getSummary() const;
};

/**
 * @brief 表达式生成结果
 */
struct ExprGenResult {
    std::string operand;        // 结果操作数名称
    IRDataType dataType;        // 数据类型
    bool isTemporary;           // 是否为临时变量
    
    ExprGenResult(const std::string& op, IRDataType type, bool temp = false)
        : operand(op), dataType(type), isTemporary(temp) {}
};

/**
 * @brief 代码生成器
 */
class CodeGenerator {
private:
    std::unique_ptr<IntermediateCode> ir;      // 中间代码
    CodeGenConfig config;                       // 配置
    std::vector<std::string> errors;           // 错误列表
    std::vector<std::string> warnings;         // 警告列表
    
    // 标签管理
    std::string breakLabel;                     // break标签
    std::string continueLabel;                  // continue标签
    
public:
    explicit CodeGenerator(const CodeGenConfig& cfg = CodeGenConfig());
    ~CodeGenerator() = default;
    
    /**
     * @brief 生成中间代码
     */
    CodeGenResult generate(ASTNode* root);
    
    /**
     * @brief 生成程序
     */
    void generateProgram(ProgramNode* node);
    
    /**
     * @brief 生成声明
     */
    void generateDeclaration(ASTNode* node);
    
    /**
     * @brief 生成变量声明
     */
    void generateVariableDecl(VariableDeclNode* node);
    
    /**
     * @brief 生成函数声明
     */
    void generateFunctionDecl(FunctionDeclNode* node);
    
    /**
     * @brief 生成语句
     */
    void generateStatement(StmtNode* node);
    
    /**
     * @brief 生成赋值语句
     */
    void generateAssignmentStmt(AssignmentStmtNode* node);
    
    /**
     * @brief 生成if语句
     */
    void generateIfStmt(IfStmtNode* node);
    
    /**
     * @brief 生成while语句
     */
    void generateWhileStmt(WhileStmtNode* node);
    
    /**
     * @brief 生成复合语句
     */
    void generateBlockStmt(BlockStmtNode* node);
    
    /**
     * @brief 生成返回语句
     */
    void generateReturnStmt(ReturnStmtNode* node);
    
    /**
     * @brief 生成表达式语句
     */
    void generateExpressionStmt(ExpressionStmtNode* node);
    
    /**
     * @brief 生成表达式
     */
    ExprGenResult generateExpression(ExprNode* node);
    
    /**
     * @brief 生成二元表达式
     */
    ExprGenResult generateBinaryExpression(BinaryExprNode* node);
    
    /**
     * @brief 生成一元表达式
     */
    ExprGenResult generateUnaryExpression(UnaryExprNode* node);
    
    /**
     * @brief 生成标识符
     */
    ExprGenResult generateIdentifier(IdentifierNode* node);
    
    /**
     * @brief 生成字面量
     */
    ExprGenResult generateLiteral(LiteralNode* node);
    
    /**
     * @brief 获取中间代码
     */
    IntermediateCode* getIntermediateCode() const { return ir.get(); }
    
    /**
     * @brief 设置配置
     */
    void setConfig(const CodeGenConfig& cfg) { config = cfg; }
    
    /**
     * @brief 获取错误列表
     */
    const std::vector<std::string>& getErrors() const { return errors; }
    
    /**
     * @brief 获取警告列表
     */
    const std::vector<std::string>& getWarnings() const { return warnings; }
    
private:
    /**
     * @brief 添加错误
     */
    void addError(const std::string& message);
    
    /**
     * @brief 添加警告
     */
    void addWarning(const std::string& message);
    
    /**
     * @brief 清理状态
     */
    void clear();
    
    /**
     * @brief TokenType转换为OpType
     */
    OpType tokenTypeToOpType(TokenType tokenType);
    
    /**
     * @brief DataType转换为IRDataType
     */
    IRDataType dataTypeToIRDataType(DataType dataType);
    
    /**
     * @brief 生成临时变量
     */
    std::string newTemp();
    
    /**
     * @brief 生成标签
     */
    std::string newLabel();
    
    /**
     * @brief 添加指令
     */
    void addInstruction(std::unique_ptr<ThreeAddressCode> instr);
    
    /**
     * @brief 添加注释
     */
    void addComment(const std::string& comment);
};

/**
 * @brief 代码生成器工厂
 */
class CodeGeneratorFactory {
public:
    /**
     * @brief 创建标准代码生成器
     */
    static std::unique_ptr<CodeGenerator> createStandard();
    
    /**
     * @brief 创建优化代码生成器
     */
    static std::unique_ptr<CodeGenerator> createOptimized();
    
    /**
     * @brief 创建调试代码生成器
     */
    static std::unique_ptr<CodeGenerator> createDebug();
};

#endif // CODE_GENERATOR_H 