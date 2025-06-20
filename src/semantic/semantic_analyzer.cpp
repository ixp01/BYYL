#include "semantic_analyzer.h"
#include <chrono>
#include <sstream>
#include <algorithm>

// ============ SemanticError类实现 ============

std::string SemanticError::toString() const {
    std::ostringstream oss;
    oss << "Semantic Error [Line " << line << ":" << column << "] ";
    oss << getTypeString() << ": " << message;
    if (!context.empty()) {
        oss << " (in " << context << ")";
    }
    return oss.str();
}

std::string SemanticError::getTypeString() const {
    switch (type) {
        case SemanticErrorType::UNDEFINED_VARIABLE: return "Undefined Variable";
        case SemanticErrorType::UNDEFINED_FUNCTION: return "Undefined Function";
        case SemanticErrorType::REDEFINED_VARIABLE: return "Redefined Variable";
        case SemanticErrorType::REDEFINED_FUNCTION: return "Redefined Function";
        case SemanticErrorType::TYPE_MISMATCH: return "Type Mismatch";
        case SemanticErrorType::INVALID_ASSIGNMENT: return "Invalid Assignment";
        case SemanticErrorType::INVALID_OPERATION: return "Invalid Operation";
        case SemanticErrorType::FUNCTION_CALL_ERROR: return "Function Call Error";
        case SemanticErrorType::PARAMETER_COUNT_MISMATCH: return "Parameter Count Mismatch";
        case SemanticErrorType::PARAMETER_TYPE_MISMATCH: return "Parameter Type Mismatch";
        case SemanticErrorType::RETURN_TYPE_MISMATCH: return "Return Type Mismatch";
        case SemanticErrorType::UNINITIALIZED_VARIABLE: return "Uninitialized Variable";
        case SemanticErrorType::UNREACHABLE_CODE: return "Unreachable Code";
        case SemanticErrorType::MISSING_RETURN: return "Missing Return Statement";
        case SemanticErrorType::DIVISION_BY_ZERO: return "Division by Zero";
        case SemanticErrorType::ARRAY_INDEX_ERROR: return "Array Index Error";
        case SemanticErrorType::SCOPE_ERROR: return "Scope Error";
        default: return "Unknown Error";
    }
}

// ============ SemanticAnalysisResult类实现 ============

std::string SemanticAnalysisResult::getSummary() const {
    std::ostringstream oss;
    oss << "Semantic Analysis Summary:\n";
    oss << "  Result: " << (success ? "SUCCESS" : "FAILED") << "\n";
    oss << "  Errors: " << errors.size() << "\n";
    oss << "  Warnings: " << warnings.size() << "\n";
    oss << "  Total Symbols: " << totalSymbols << "\n";
    oss << "  Total Scopes: " << totalScopes << "\n";
    oss << "  Analysis Time: " << analysisTimeMs << " ms\n";
    
    if (!errors.empty()) {
        oss << "\nErrors:\n";
        for (const auto& error : errors) {
            oss << "  - " << error.toString() << "\n";
        }
    }
    
    if (!warnings.empty()) {
        oss << "\nWarnings:\n";
        for (const auto& warning : warnings) {
            oss << "  - " << warning.toString() << "\n";
        }
    }
    
    return oss.str();
}

// ============ SemanticAnalyzer类实现 ============

SemanticAnalyzer::SemanticAnalyzer(const SemanticAnalyzerConfig& cfg)
    : symbolTable(std::make_unique<SymbolTable>()), config(cfg),
      currentFunctionReturnType(DataType::VOID), inFunction(false), hasReturnStatement(false) {
}

SemanticAnalysisResult SemanticAnalyzer::analyze(ASTNode* root) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 清空之前的分析状态
    clear();
    
    SemanticAnalysisResult result;
    
    if (!root) {
        addError(SemanticErrorType::SCOPE_ERROR, "Empty AST root");
        result.success = false;
        result.errors = errors;
        return result;
    }
    
    // 根据AST节点类型进行分析
    if (auto programNode = dynamic_cast<ProgramNode*>(root)) {
        analyzeProgram(programNode);
    } else {
        addError(SemanticErrorType::SCOPE_ERROR, "Invalid root node type");
    }
    
    // 执行后期检查
    if (config.checkUnusedVariables) {
        checkUnusedVariables();
    }
    
    if (config.checkUninitializedVars) {
        checkUninitializedVariables();
    }
    
    // 计算分析时间
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 填充结果
    result.success = errors.empty() && (!config.warningsAsErrors || warnings.empty());
    result.errors = errors;
    result.warnings = warnings;
    result.symbolTable = std::move(symbolTable);
    result.analysisTimeMs = static_cast<int>(duration.count());
    
    // 计算统计信息
    result.totalSymbols = 0;
    result.totalScopes = 1; // 至少有全局作用域
    
    return result;
}

ExpressionType SemanticAnalyzer::analyzeExpression(ExprNode* node) {
    if (!node) {
        return ExpressionType(DataType::UNKNOWN);
    }
    
    switch (node->nodeType) {
        case ASTNodeType::BINARY_EXPR:
            return analyzeBinaryExpression(dynamic_cast<BinaryExprNode*>(node));
        case ASTNodeType::UNARY_EXPR:
            return analyzeUnaryExpression(dynamic_cast<UnaryExprNode*>(node));
        case ASTNodeType::IDENTIFIER_EXPR:
            return analyzeIdentifier(dynamic_cast<IdentifierNode*>(node));
        case ASTNodeType::LITERAL_EXPR:
            return analyzeLiteral(dynamic_cast<LiteralNode*>(node));
        default:
            addError(SemanticErrorType::INVALID_OPERATION, 
                    "Unknown expression type", node->line, node->column);
            return ExpressionType(DataType::UNKNOWN);
    }
}

ExpressionType SemanticAnalyzer::analyzeBinaryExpression(BinaryExprNode* node) {
    if (!node || !node->left || !node->right) {
        addError(SemanticErrorType::INVALID_OPERATION, 
                "Invalid binary expression", node ? node->line : 0, node ? node->column : 0);
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 分析左右操作数
    ExpressionType leftType = analyzeExpression(node->left.get());
    ExpressionType rightType = analyzeExpression(node->right.get());
    
    // 检查操作数类型
    if (leftType.dataType == DataType::UNKNOWN || rightType.dataType == DataType::UNKNOWN) {
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 检查特殊情况：除零
    if ((node->operator_ == TokenType::DIVIDE || node->operator_ == TokenType::MODULO) && 
        rightType.isConstant && rightType.constantValue == "0") {
        addError(SemanticErrorType::DIVISION_BY_ZERO, 
                "Division by zero", node->line, node->column);
    }
    
    // 获取运算结果类型
    DataType resultType = TypeUtils::getBinaryOperationResultType(
        leftType.dataType, rightType.dataType, node->operator_);
    
    if (resultType == DataType::UNKNOWN) {
        addError(SemanticErrorType::TYPE_MISMATCH, 
                "Incompatible types for binary operation: " + 
                TypeUtils::dataTypeToString(leftType.dataType) + " " + 
                Token::getTypeString(node->operator_) + " " +
                TypeUtils::dataTypeToString(rightType.dataType),
                node->line, node->column);
    }
    
    // 常量折叠（简单实现）
    bool isConstant = leftType.isConstant && rightType.isConstant;
    
    return ExpressionType(resultType, false, isConstant);
}

ExpressionType SemanticAnalyzer::analyzeUnaryExpression(UnaryExprNode* node) {
    if (!node || !node->operand) {
        addError(SemanticErrorType::INVALID_OPERATION, 
                "Invalid unary expression", node ? node->line : 0, node ? node->column : 0);
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 分析操作数
    ExpressionType operandType = analyzeExpression(node->operand.get());
    
    if (operandType.dataType == DataType::UNKNOWN) {
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 获取运算结果类型
    DataType resultType = TypeUtils::getUnaryOperationResultType(
        operandType.dataType, node->operator_);
    
    if (resultType == DataType::UNKNOWN) {
        addError(SemanticErrorType::TYPE_MISMATCH, 
                "Invalid unary operation: " + Token::getTypeString(node->operator_) + 
                " applied to " + TypeUtils::dataTypeToString(operandType.dataType),
                node->line, node->column);
    }
    
    // 检查左值要求（暂时跳过，因为TokenType中没有INCREMENT/DECREMENT）
    // bool needsLValue = (node->operator_ == TokenType::INCREMENT || 
    //                    node->operator_ == TokenType::DECREMENT);
    // 
    // if (needsLValue && !operandType.isLValue) {
    //     addError(SemanticErrorType::INVALID_ASSIGNMENT, 
    //             "Operator " + Token::getTypeString(node->operator_) + 
    //             " requires lvalue", node->line, node->column);
    // }
    
    return ExpressionType(resultType, false, operandType.isConstant);
}

ExpressionType SemanticAnalyzer::analyzeIdentifier(IdentifierNode* node) {
    if (!node) {
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 查找符号
    SymbolInfo* symbol = symbolTable->findSymbol(node->name);
    
    if (!symbol) {
        addError(SemanticErrorType::UNDEFINED_VARIABLE, 
                "Undefined variable: " + node->name, node->line, node->column);
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 标记为已使用
    symbolTable->markSymbolUsed(node->name);
    
    // 检查是否已初始化
    if (config.checkUninitializedVars && !symbol->isInitialized && 
        symbol->symbolType == SymbolType::VARIABLE) {
        addWarning(SemanticErrorType::UNINITIALIZED_VARIABLE, 
                  "Variable '" + node->name + "' used before initialization",
                  node->line, node->column);
    }
    
    // 返回类型信息
    bool isLValue = (symbol->symbolType == SymbolType::VARIABLE);
    bool isConstant = (symbol->symbolType == SymbolType::CONSTANT);
    
    return ExpressionType(symbol->dataType, isLValue, isConstant);
}

ExpressionType SemanticAnalyzer::analyzeLiteral(LiteralNode* node) {
    if (!node) {
        return ExpressionType(DataType::UNKNOWN);
    }
    
    // 根据字面量类型确定数据类型
    DataType dataType = TypeUtils::tokenTypeToDataType(node->valueType);
    
    ExpressionType result(dataType, false, true);
    result.constantValue = node->value;
    
    return result;
}

bool SemanticAnalyzer::checkTypeCompatibility(DataType expected, DataType actual, 
                                             const std::string& context, int line, int col) {
    if (expected == actual) {
        return true;
    }
    
    // 检查隐式类型转换
    if (TypeUtils::canImplicitlyConvert(actual, expected)) {
        if (config.checkTypeConversion) {
            addWarning(SemanticErrorType::TYPE_MISMATCH,
                      "Implicit conversion from " + TypeUtils::dataTypeToString(actual) + 
                      " to " + TypeUtils::dataTypeToString(expected) + " in " + context,
                      line, col);
        }
        return true;
    }
    
    // 检查类型兼容性
    if (TypeUtils::areTypesCompatible(expected, actual)) {
        if (config.strictTypeChecking) {
            addError(SemanticErrorType::TYPE_MISMATCH,
                    "Type mismatch in " + context + ": expected " + 
                    TypeUtils::dataTypeToString(expected) + ", got " + 
                    TypeUtils::dataTypeToString(actual), line, col);
            return false;
        } else {
            addWarning(SemanticErrorType::TYPE_MISMATCH,
                      "Type compatibility warning in " + context, line, col);
        }
        return true;
    }
    
    // 完全不兼容
    addError(SemanticErrorType::TYPE_MISMATCH,
            "Incompatible types in " + context + ": expected " + 
            TypeUtils::dataTypeToString(expected) + ", got " + 
            TypeUtils::dataTypeToString(actual), line, col);
    return false;
}

void SemanticAnalyzer::addError(SemanticErrorType type, const std::string& message,
                               int line, int col, const std::string& context) {
    errors.emplace_back(type, message, line, col, context);
}

void SemanticAnalyzer::addWarning(SemanticErrorType type, const std::string& message,
                                 int line, int col, const std::string& context) {
    warnings.emplace_back(type, message, line, col, context);
}

SymbolTable* SemanticAnalyzer::getSymbolTable() const {
    return symbolTable.get();
}

const std::vector<SemanticError>& SemanticAnalyzer::getErrors() const {
    return errors;
}

const std::vector<SemanticError>& SemanticAnalyzer::getWarnings() const {
    return warnings;
}

void SemanticAnalyzer::clear() {
    errors.clear();
    warnings.clear();
    symbolTable->clear();
    currentFunctionReturnType = DataType::VOID;
    inFunction = false;
    hasReturnStatement = false;
}

void SemanticAnalyzer::setConfig(const SemanticAnalyzerConfig& cfg) {
    config = cfg;
}

// ============ 高级分析功能（第二部分）============

void SemanticAnalyzer::analyzeProgram(ProgramNode* node) {
    if (!node) {
        addError(SemanticErrorType::SCOPE_ERROR, "Null program node");
        return;
    }
    
    // 分析所有声明
    for (const auto& decl : node->declarations) {
        if (decl) {
            analyzeDeclaration(decl.get());
        }
    }
}

void SemanticAnalyzer::analyzeDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }
    
    switch (node->nodeType) {
        case ASTNodeType::VAR_DECL:
            analyzeVariableDecl(dynamic_cast<VariableDeclNode*>(node));
            break;
        case ASTNodeType::FUNC_DECL:
            analyzeFunctionDecl(dynamic_cast<FunctionDeclNode*>(node));
            break;
        default:
            addError(SemanticErrorType::SCOPE_ERROR, 
                    "Unknown declaration type", node->line, node->column);
            break;
    }
}

void SemanticAnalyzer::analyzeVariableDecl(VariableDeclNode* node) {
    if (!node) {
        return;
    }
    
    // 检查是否在当前作用域中重复定义
    if (symbolTable->isLocalDefined(node->name)) {
        addError(SemanticErrorType::REDEFINED_VARIABLE, 
                "Variable '" + node->name + "' already defined in current scope",
                node->line, node->column);
        return;
    }
    
    // 创建符号信息
    DataType dataType = TypeUtils::tokenTypeToDataType(node->varType);
    SymbolInfo symbol(node->name, SymbolType::VARIABLE, dataType, 
                     node->line, node->column, symbolTable->getCurrentScopeLevel());
    
    // 如果有初始化表达式，分析它
    bool isInitialized = false;
    if (node->initializer) {
        ExpressionType initType = analyzeExpression(node->initializer.get());
        
        // 检查初始化类型兼容性
        if (initType.dataType != DataType::UNKNOWN) {
            if (checkTypeCompatibility(dataType, initType.dataType, 
                                     "variable initialization", node->line, node->column)) {
                isInitialized = true;
            }
        }
    }
    
    symbol.isInitialized = isInitialized;
    
    // 添加到符号表
    if (!symbolTable->addSymbol(symbol)) {
        addError(SemanticErrorType::REDEFINED_VARIABLE, 
                "Failed to add variable '" + node->name + "' to symbol table",
                node->line, node->column);
    }
}

void SemanticAnalyzer::analyzeFunctionDecl(FunctionDeclNode* node) {
    if (!node) {
        return;
    }
    
    // 检查函数是否重复定义
    if (symbolTable->isLocalDefined(node->name)) {
        addError(SemanticErrorType::REDEFINED_FUNCTION, 
                "Function '" + node->name + "' already defined",
                node->line, node->column);
        return;
    }
    
    // 创建函数符号信息
    SymbolInfo funcSymbol(node->name, SymbolType::FUNCTION, DataType::FUNCTION_TYPE, 
                         node->line, node->column, symbolTable->getCurrentScopeLevel());
    
    funcSymbol.returnType = TypeUtils::tokenTypeToDataType(node->returnType);
    
    // 收集参数类型
    for (const auto& param : node->parameters) {
        if (param) {
            funcSymbol.paramTypes.push_back(TypeUtils::tokenTypeToDataType(param->varType));
        }
    }
    
    // 添加函数到符号表
    if (!symbolTable->addSymbol(funcSymbol)) {
        addError(SemanticErrorType::REDEFINED_FUNCTION, 
                "Failed to add function '" + node->name + "' to symbol table",
                node->line, node->column);
        return;
    }
    
    // 进入函数作用域
    symbolTable->enterScope();
    
    // 设置函数分析状态
    DataType previousReturnType = currentFunctionReturnType;
    bool wasInFunction = inFunction;
    bool hadReturn = hasReturnStatement;
    
    currentFunctionReturnType = TypeUtils::tokenTypeToDataType(node->returnType);
    inFunction = true;
    hasReturnStatement = false;
    
    // 添加参数到作用域
            for (const auto& param : node->parameters) {
        if (param) {
            DataType paramDataType = TypeUtils::tokenTypeToDataType(param->varType);
            SymbolInfo paramSymbol(param->name, SymbolType::PARAMETER, paramDataType,
                                 param->line, param->column, symbolTable->getCurrentScopeLevel());
            paramSymbol.isInitialized = true; // 参数默认已初始化
            
            if (!symbolTable->addSymbol(paramSymbol)) {
                addError(SemanticErrorType::REDEFINED_VARIABLE, 
                        "Parameter '" + param->name + "' already defined",
                        param->line, param->column);
            }
        }
    }
    
    // 分析函数体
    if (node->body) {
        analyzeBlockStmt(node->body.get());
    }
    
    // 检查返回语句
    if (TypeUtils::tokenTypeToDataType(node->returnType) != DataType::VOID && !hasReturnStatement) {
        addError(SemanticErrorType::MISSING_RETURN, 
                "Function '" + node->name + "' missing return statement",
                node->line, node->column);
    }
    
    // 恢复之前的函数分析状态
    currentFunctionReturnType = previousReturnType;
    inFunction = wasInFunction;
    hasReturnStatement = hadReturn;
    
    // 退出函数作用域
    symbolTable->exitScope();
}

void SemanticAnalyzer::analyzeStatement(StmtNode* node) {
    if (!node) {
        return;
    }
    
    switch (node->nodeType) {
        case ASTNodeType::ASSIGNMENT_STMT:
            analyzeAssignmentStmt(dynamic_cast<AssignmentStmtNode*>(node));
            break;
        case ASTNodeType::IF_STMT:
            analyzeIfStmt(dynamic_cast<IfStmtNode*>(node));
            break;
        case ASTNodeType::WHILE_STMT:
            analyzeWhileStmt(dynamic_cast<WhileStmtNode*>(node));
            break;
        case ASTNodeType::BLOCK_STMT:
            analyzeBlockStmt(dynamic_cast<BlockStmtNode*>(node));
            break;
        case ASTNodeType::RETURN_STMT:
            analyzeReturnStmt(dynamic_cast<ReturnStmtNode*>(node));
            break;
        case ASTNodeType::EXPR_STMT:
            analyzeExpressionStmt(dynamic_cast<ExpressionStmtNode*>(node));
            break;
        case ASTNodeType::VAR_DECL:
            // 局部变量声明（转换为VarDeclNode）
            if (auto varDecl = dynamic_cast<VarDeclNode*>(node)) {
                DataType varDataType = TypeUtils::tokenTypeToDataType(varDecl->type);
                SymbolInfo symbol(varDecl->name, SymbolType::VARIABLE, varDataType,
                                varDecl->line, varDecl->column, symbolTable->getCurrentScopeLevel());
                
                bool isInitialized = false;
                if (varDecl->initializer) {
                    ExpressionType initType = analyzeExpression(varDecl->initializer.get());
                    if (checkTypeCompatibility(varDataType, initType.dataType,
                                             "variable initialization", varDecl->line, varDecl->column)) {
                        isInitialized = true;
                    }
                }
                
                symbol.isInitialized = isInitialized;
                symbolTable->addSymbol(symbol);
            }
            break;
        default:
            addError(SemanticErrorType::INVALID_OPERATION, 
                    "Unknown statement type", node->line, node->column);
            break;
    }
}

void SemanticAnalyzer::analyzeAssignmentStmt(AssignmentStmtNode* node) {
    if (!node || !node->lvalue || !node->rvalue) {
        addError(SemanticErrorType::INVALID_ASSIGNMENT, 
                "Invalid assignment statement", 
                node ? node->line : 0, node ? node->column : 0);
        return;
    }
    
    // 分析左值和右值
    ExpressionType lvalueType = analyzeExpression(node->lvalue.get());
    ExpressionType rvalueType = analyzeExpression(node->rvalue.get());
    
    // 检查左值
    if (!lvalueType.isLValue) {
        addError(SemanticErrorType::INVALID_ASSIGNMENT, 
                "Left side of assignment is not an lvalue", node->line, node->column);
        return;
    }
    
    // 检查类型兼容性
    if (lvalueType.dataType != DataType::UNKNOWN && rvalueType.dataType != DataType::UNKNOWN) {
        checkTypeCompatibility(lvalueType.dataType, rvalueType.dataType, 
                             "assignment", node->line, node->column);
    }
    
    // 标记左值变量为已初始化
    if (auto identNode = dynamic_cast<IdentifierNode*>(node->lvalue.get())) {
        symbolTable->markSymbolInitialized(identNode->name);
    }
}

void SemanticAnalyzer::analyzeIfStmt(IfStmtNode* node) {
    if (!node) {
        return;
    }
    
    // 分析条件表达式
    if (node->condition) {
        ExpressionType condType = analyzeExpression(node->condition.get());
        
        // 检查条件是否为布尔类型
        if (condType.dataType != DataType::UNKNOWN && condType.dataType != DataType::BOOL) {
            if (!TypeUtils::canImplicitlyConvert(condType.dataType, DataType::BOOL)) {
                addError(SemanticErrorType::TYPE_MISMATCH, 
                        "Condition in if statement must be boolean",
                        node->line, node->column);
            }
        }
    }
    
    // 分析then语句
    if (node->thenStmt) {
        analyzeStatement(node->thenStmt.get());
    }
    
    // 分析else语句
    if (node->elseStmt) {
        analyzeStatement(node->elseStmt.get());
    }
}

void SemanticAnalyzer::analyzeWhileStmt(WhileStmtNode* node) {
    if (!node) {
        return;
    }
    
    // 分析条件表达式
    if (node->condition) {
        ExpressionType condType = analyzeExpression(node->condition.get());
        
        // 检查条件是否为布尔类型
        if (condType.dataType != DataType::UNKNOWN && condType.dataType != DataType::BOOL) {
            if (!TypeUtils::canImplicitlyConvert(condType.dataType, DataType::BOOL)) {
                addError(SemanticErrorType::TYPE_MISMATCH, 
                        "Condition in while statement must be boolean",
                        node->line, node->column);
            }
        }
    }
    
    // 分析循环体
    if (node->body) {
        analyzeStatement(node->body.get());
    }
}

void SemanticAnalyzer::analyzeBlockStmt(BlockStmtNode* node) {
    if (!node) {
        return;
    }
    
    // 进入新作用域
    symbolTable->enterScope();
    
    // 分析所有语句
    for (const auto& stmt : node->statements) {
        if (stmt) {
            analyzeStatement(stmt.get());
        }
    }
    
    // 退出作用域
    symbolTable->exitScope();
}

void SemanticAnalyzer::analyzeReturnStmt(ReturnStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (!inFunction) {
        addError(SemanticErrorType::SCOPE_ERROR, 
                "Return statement outside function", node->line, node->column);
        return;
    }
    
    hasReturnStatement = true;
    
    // 检查返回值类型
    if (node->expression) {
        ExpressionType returnType = analyzeExpression(node->expression.get());
        
        if (currentFunctionReturnType == DataType::VOID) {
            addError(SemanticErrorType::RETURN_TYPE_MISMATCH, 
                    "Void function should not return value", node->line, node->column);
        } else if (returnType.dataType != DataType::UNKNOWN) {
            checkTypeCompatibility(currentFunctionReturnType, returnType.dataType,
                                 "return statement", node->line, node->column);
        }
    } else {
        if (currentFunctionReturnType != DataType::VOID) {
            addError(SemanticErrorType::RETURN_TYPE_MISMATCH, 
                    "Non-void function must return value", node->line, node->column);
        }
    }
}

void SemanticAnalyzer::analyzeExpressionStmt(ExpressionStmtNode* node) {
    if (!node || !node->expression) {
        return;
    }
    
    // 分析表达式
    analyzeExpression(node->expression.get());
}

void SemanticAnalyzer::checkUnusedVariables() {
    auto unusedVars = symbolTable->getUnusedVariables();
    
    for (SymbolInfo* symbol : unusedVars) {
        addWarning(SemanticErrorType::UNINITIALIZED_VARIABLE, 
                  "Variable '" + symbol->name + "' declared but never used",
                  symbol->line, symbol->column);
    }
}

void SemanticAnalyzer::checkUninitializedVariables() {
    auto uninitVars = symbolTable->getUninitializedVariables();
    
    for (SymbolInfo* symbol : uninitVars) {
        if (symbol->isUsed) {
            addWarning(SemanticErrorType::UNINITIALIZED_VARIABLE, 
                      "Variable '" + symbol->name + "' used before initialization",
                      symbol->line, symbol->column);
        }
    }
}

// ============ SemanticAnalyzerFactory类实现 ============

std::unique_ptr<SemanticAnalyzer> SemanticAnalyzerFactory::createStandard() {
    SemanticAnalyzerConfig config;
    config.checkUnusedVariables = true;
    config.checkUninitializedVars = true;
    config.checkTypeConversion = true;
    config.checkFunctionCalls = true;
    config.checkArrayBounds = false;
    config.warningsAsErrors = false;
    config.strictTypeChecking = false;
    
    return std::make_unique<SemanticAnalyzer>(config);
}

std::unique_ptr<SemanticAnalyzer> SemanticAnalyzerFactory::createStrict() {
    SemanticAnalyzerConfig config;
    config.checkUnusedVariables = true;
    config.checkUninitializedVars = true;
    config.checkTypeConversion = true;
    config.checkFunctionCalls = true;
    config.checkArrayBounds = true;
    config.warningsAsErrors = true;
    config.strictTypeChecking = true;
    
    return std::make_unique<SemanticAnalyzer>(config);
}

std::unique_ptr<SemanticAnalyzer> SemanticAnalyzerFactory::createPermissive() {
    SemanticAnalyzerConfig config;
    config.checkUnusedVariables = false;
    config.checkUninitializedVars = false;
    config.checkTypeConversion = false;
    config.checkFunctionCalls = true;
    config.checkArrayBounds = false;
    config.warningsAsErrors = false;
    config.strictTypeChecking = false;
    
    return std::make_unique<SemanticAnalyzer>(config);
} 