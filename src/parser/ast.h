#ifndef AST_H
#define AST_H

#include <memory>
#include <vector>
#include <string>
#include "../lexer/token.h"

/**
 * @brief AST节点类型枚举
 */
enum class ASTNodeType {
    // 表达式节点
    BINARY_EXPR,        // 二元表达式
    UNARY_EXPR,         // 一元表达式
    IDENTIFIER_EXPR,    // 标识符表达式
    LITERAL_EXPR,       // 字面量表达式
    CALL_EXPR,          // 函数调用表达式
    ARRAY_ACCESS_EXPR,  // 数组访问表达式
    
    // 语句节点
    ASSIGNMENT_STMT,    // 赋值语句
    IF_STMT,            // if语句
    WHILE_STMT,         // while语句
    FOR_STMT,           // for语句
    BLOCK_STMT,         // 复合语句
    EXPR_STMT,          // 表达式语句
    RETURN_STMT,        // return语句
    BREAK_STMT,         // break语句
    CONTINUE_STMT,      // continue语句
    
    // 声明节点
    VAR_DECL,           // 变量声明
    FUNC_DECL,          // 函数声明
    
    // 程序节点
    PROGRAM             // 程序根节点
};

/**
 * @brief AST节点基类
 */
class ASTNode {
public:
    ASTNodeType nodeType;
    int line;       // 源代码行号
    int column;     // 源代码列号
    
    ASTNode(ASTNodeType type, int l = 0, int c = 0) 
        : nodeType(type), line(l), column(c) {}
    
    virtual ~ASTNode() = default;
    
    /**
     * @brief 获取节点类型的字符串表示
     */
    virtual std::string getNodeTypeString() const;
    
    /**
     * @brief 打印AST树（用于调试）
     */
    virtual void print(int indent = 0) const = 0;
    
protected:
    void printIndent(int indent) const;
};

/**
 * @brief 表达式节点基类
 */
class ExprNode : public ASTNode {
public:
    ExprNode(ASTNodeType type, int l = 0, int c = 0) 
        : ASTNode(type, l, c) {}
};

/**
 * @brief 语句节点基类
 */
class StmtNode : public ASTNode {
public:
    StmtNode(ASTNodeType type, int l = 0, int c = 0) 
        : ASTNode(type, l, c) {}
};

/**
 * @brief 二元表达式节点
 */
class BinaryExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> left;
    std::unique_ptr<ExprNode> right;
    TokenType operator_;
    
    BinaryExprNode(std::unique_ptr<ExprNode> l, 
                   TokenType op, 
                   std::unique_ptr<ExprNode> r,
                   int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 一元表达式节点
 */
class UnaryExprNode : public ExprNode {
public:
    std::unique_ptr<ExprNode> operand;
    TokenType operator_;
    
    UnaryExprNode(TokenType op, 
                  std::unique_ptr<ExprNode> operand,
                  int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 标识符节点
 */
class IdentifierNode : public ExprNode {
public:
    std::string name;
    
    IdentifierNode(const std::string& name, int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 字面量节点
 */
class LiteralNode : public ExprNode {
public:
    TokenType valueType;
    std::string value;
    
    LiteralNode(TokenType type, const std::string& val, int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 赋值语句节点
 */
class AssignmentStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> lvalue;
    std::unique_ptr<ExprNode> rvalue;
    
    AssignmentStmtNode(std::unique_ptr<ExprNode> lval,
                       std::unique_ptr<ExprNode> rval,
                       int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief if语句节点
 */
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> thenStmt;
    std::unique_ptr<StmtNode> elseStmt;  // 可为nullptr
    
    IfStmtNode(std::unique_ptr<ExprNode> cond,
               std::unique_ptr<StmtNode> then_stmt,
               std::unique_ptr<StmtNode> else_stmt = nullptr,
               int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief while语句节点
 */
class WhileStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> body;
    
    WhileStmtNode(std::unique_ptr<ExprNode> cond,
                  std::unique_ptr<StmtNode> body,
                  int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 复合语句节点
 */
class BlockStmtNode : public StmtNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;
    
    BlockStmtNode(int line = 0, int col = 0);
    
    void addStatement(std::unique_ptr<StmtNode> stmt);
    void print(int indent = 0) const override;
};

/**
 * @brief 变量声明节点
 */
class VarDeclNode : public StmtNode {
public:
    TokenType type;
    std::string name;
    std::unique_ptr<ExprNode> initializer;  // 可为nullptr
    
    VarDeclNode(TokenType var_type, 
                const std::string& var_name,
                std::unique_ptr<ExprNode> init = nullptr,
                int line = 0, int col = 0);
    
    void print(int indent = 0) const override;
};

/**
 * @brief 程序根节点
 */
class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<StmtNode>> statements;
    
    ProgramNode();
    
    void addStatement(std::unique_ptr<StmtNode> stmt);
    void print(int indent = 0) const override;
};

// 智能指针类型别名
using ASTNodePtr = std::unique_ptr<ASTNode>;
using ExprNodePtr = std::unique_ptr<ExprNode>;
using StmtNodePtr = std::unique_ptr<StmtNode>;

#endif // AST_H 