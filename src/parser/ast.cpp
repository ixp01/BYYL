#include "ast.h"
#include <iostream>

// ASTNode基类实现
std::string ASTNode::getNodeTypeString() const {
    switch (nodeType) {
        case ASTNodeType::BINARY_EXPR: return "BinaryExpr";
        case ASTNodeType::UNARY_EXPR: return "UnaryExpr";
        case ASTNodeType::IDENTIFIER_EXPR: return "Identifier";
        case ASTNodeType::LITERAL_EXPR: return "Literal";
        case ASTNodeType::ASSIGNMENT_STMT: return "AssignmentStmt";
        case ASTNodeType::IF_STMT: return "IfStmt";
        case ASTNodeType::WHILE_STMT: return "WhileStmt";
        case ASTNodeType::BLOCK_STMT: return "BlockStmt";
        case ASTNodeType::RETURN_STMT: return "ReturnStmt";
        case ASTNodeType::EXPR_STMT: return "ExpressionStmt";
        case ASTNodeType::VAR_DECL: return "VariableDecl";
        case ASTNodeType::FUNC_DECL: return "FunctionDecl";
        case ASTNodeType::PROGRAM: return "Program";
        default: return "Unknown";
    }
}

void ASTNode::printIndent(int indent) const {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

// BinaryExprNode二元表达式节点实现
BinaryExprNode::BinaryExprNode(std::unique_ptr<ExprNode> l, TokenType op, 
                               std::unique_ptr<ExprNode> r, int line, int col)
    : ExprNode(ASTNodeType::BINARY_EXPR, line, col), left(std::move(l)), 
      right(std::move(r)), operator_(op) {}

void BinaryExprNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryExpr(" << Token::getTypeString(operator_) << ")\n";
    
    if (left) {
        printIndent(indent + 1);
        std::cout << "Left:\n";
        left->print(indent + 2);
    }
    
    if (right) {
        printIndent(indent + 1);
        std::cout << "Right:\n";
        right->print(indent + 2);
    }
}

// UnaryExprNode一元表达式节点实现
UnaryExprNode::UnaryExprNode(TokenType op, std::unique_ptr<ExprNode> operand, int line, int col)
    : ExprNode(ASTNodeType::UNARY_EXPR, line, col), operand(std::move(operand)), operator_(op) {}

void UnaryExprNode::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryExpr(" << Token::getTypeString(operator_) << ")\n";
    
    if (operand) {
        printIndent(indent + 1);
        std::cout << "Operand:\n";
        operand->print(indent + 2);
    }
}

// IdentifierNode标识符节点实现
IdentifierNode::IdentifierNode(const std::string& n, int line, int col)
    : ExprNode(ASTNodeType::IDENTIFIER_EXPR, line, col), name(n) {}

void IdentifierNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Identifier(" << name << ")\n";
}

// LiteralNode字面量节点实现
LiteralNode::LiteralNode(TokenType t, const std::string& v, int line, int col)
    : ExprNode(ASTNodeType::LITERAL_EXPR, line, col), valueType(t), value(v) {}

void LiteralNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Literal(" << Token::getTypeString(valueType) << ", " << value << ")\n";
}

// AssignmentStmtNode赋值语句节点实现
AssignmentStmtNode::AssignmentStmtNode(std::unique_ptr<ExprNode> lval, 
                                       std::unique_ptr<ExprNode> rval, int line, int col)
    : StmtNode(ASTNodeType::ASSIGNMENT_STMT, line, col), 
      lvalue(std::move(lval)), rvalue(std::move(rval)) {}

void AssignmentStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "AssignmentStmt\n";
    
    if (lvalue) {
        printIndent(indent + 1);
        std::cout << "Target:\n";
        lvalue->print(indent + 2);
    }
    
    if (rvalue) {
        printIndent(indent + 1);
        std::cout << "Value:\n";
        rvalue->print(indent + 2);
    }
}

// IfStmtNode条件语句节点实现
IfStmtNode::IfStmtNode(std::unique_ptr<ExprNode> cond, 
                       std::unique_ptr<StmtNode> then_stmt,
                       std::unique_ptr<StmtNode> else_stmt, int line, int col)
    : StmtNode(ASTNodeType::IF_STMT, line, col), 
      condition(std::move(cond)), thenStmt(std::move(then_stmt)), 
      elseStmt(std::move(else_stmt)) {}

void IfStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "IfStmt\n";
    
    if (condition) {
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        condition->print(indent + 2);
    }
    
    if (thenStmt) {
        printIndent(indent + 1);
        std::cout << "Then:\n";
        thenStmt->print(indent + 2);
    }
    
    if (elseStmt) {
        printIndent(indent + 1);
        std::cout << "Else:\n";
        elseStmt->print(indent + 2);
    }
}

// WhileStmtNode循环语句节点实现
WhileStmtNode::WhileStmtNode(std::unique_ptr<ExprNode> cond, 
                             std::unique_ptr<StmtNode> body, int line, int col)
    : StmtNode(ASTNodeType::WHILE_STMT, line, col), 
      condition(std::move(cond)), body(std::move(body)) {}

void WhileStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "WhileStmt\n";
    
    if (condition) {
        printIndent(indent + 1);
        std::cout << "Condition:\n";
        condition->print(indent + 2);
    }
    
    if (body) {
        printIndent(indent + 1);
        std::cout << "Body:\n";
        body->print(indent + 2);
    }
}

// BlockStmtNode复合语句节点实现
BlockStmtNode::BlockStmtNode(int line, int col)
    : StmtNode(ASTNodeType::BLOCK_STMT, line, col) {}

void BlockStmtNode::addStatement(std::unique_ptr<StmtNode> stmt) {
    statements.push_back(std::move(stmt));
}

void BlockStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BlockStmt (" << statements.size() << " statements)\n";
    
    for (const auto& stmt : statements) {
        if (stmt) {
            stmt->print(indent + 1);
        }
    }
}

// ReturnStmtNode返回语句节点实现
ReturnStmtNode::ReturnStmtNode(std::unique_ptr<ExprNode> expr, int line, int col)
    : StmtNode(ASTNodeType::RETURN_STMT, line, col), expression(std::move(expr)) {}

void ReturnStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "ReturnStmt\n";
    
    if (expression) {
        printIndent(indent + 1);
        std::cout << "Expression:\n";
        expression->print(indent + 2);
    }
}

// ExpressionStmtNode表达式语句节点实现
ExpressionStmtNode::ExpressionStmtNode(std::unique_ptr<ExprNode> expr, int line, int col)
    : StmtNode(ASTNodeType::EXPR_STMT, line, col), expression(std::move(expr)) {}

void ExpressionStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "ExpressionStmt\n";
    
    if (expression) {
        printIndent(indent + 1);
        std::cout << "Expression:\n";
        expression->print(indent + 2);
    }
}

// VariableDeclNode变量声明节点实现
VariableDeclNode::VariableDeclNode(const std::string& n, TokenType t, 
                                   std::unique_ptr<ExprNode> init, int line, int col)
    : DeclNode(ASTNodeType::VAR_DECL, line, col), 
      name(n), varType(t), initializer(std::move(init)) {}

void VariableDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "VariableDecl(" << Token::getTypeString(varType) << " " << name << ")\n";
    
    if (initializer) {
        printIndent(indent + 1);
        std::cout << "Initializer:\n";
        initializer->print(indent + 2);
    }
}

// FunctionDeclNode函数声明节点实现
FunctionDeclNode::FunctionDeclNode(const std::string& n, TokenType retType,
                                   std::vector<std::unique_ptr<VariableDeclNode>> params,
                                   std::unique_ptr<BlockStmtNode> body, int line, int col)
    : DeclNode(ASTNodeType::FUNC_DECL, line, col),
      name(n), returnType(retType), parameters(std::move(params)), body(std::move(body)) {}

void FunctionDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "FunctionDecl(" << Token::getTypeString(returnType) << " " << name << ")\n";
    
    if (!parameters.empty()) {
        printIndent(indent + 1);
        std::cout << "Parameters:\n";
        for (const auto& param : parameters) {
            if (param) {
                param->print(indent + 2);
            }
        }
    }
    
    if (body) {
        printIndent(indent + 1);
        std::cout << "Body:\n";
        body->print(indent + 2);
    }
}

// VarDeclNode变量声明节点实现（简化版）
VarDeclNode::VarDeclNode(TokenType var_type, const std::string& var_name,
                         std::unique_ptr<ExprNode> init, int line, int col)
    : StmtNode(ASTNodeType::VAR_DECL, line, col), 
      type(var_type), name(var_name), initializer(std::move(init)) {}

void VarDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "VarDecl(" << Token::getTypeString(type) << " " << name << ")\n";
    
    if (initializer) {
        printIndent(indent + 1);
        std::cout << "Initializer:\n";
        initializer->print(indent + 2);
    }
}

// ProgramNode程序根节点实现
ProgramNode::ProgramNode() : ASTNode(ASTNodeType::PROGRAM) {}

void ProgramNode::addDeclaration(std::unique_ptr<DeclNode> decl) {
    declarations.push_back(std::move(decl));
}

void ProgramNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Program (" << declarations.size() << " declarations)\n";
    
    for (const auto& decl : declarations) {
        if (decl) {
            decl->print(indent + 1);
        }
    }
} 