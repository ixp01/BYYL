#include "code_generator.h"
#include <sstream>
#include <chrono>

// ============ CodeGenResult类实现 ============

std::string CodeGenResult::getSummary() const {
    std::ostringstream oss;
    oss << "Code Generation Summary:\n";
    oss << "  Result: " << (success ? "SUCCESS" : "FAILED") << "\n";
    oss << "  Errors: " << errors.size() << "\n";
    oss << "  Warnings: " << warnings.size() << "\n";
    oss << "  Instructions: " << statistics.instructionCount << "\n";
    oss << "  Basic Blocks: " << statistics.basicBlockCount << "\n";
    oss << "  Temporaries: " << statistics.temporaryCount << "\n";
    oss << "  Labels: " << statistics.labelCount << "\n";
    
    if (!errors.empty()) {
        oss << "\nErrors:\n";
        for (const auto& error : errors) {
            oss << "  - " << error << "\n";
        }
    }
    
    if (!warnings.empty()) {
        oss << "\nWarnings:\n";
        for (const auto& warning : warnings) {
            oss << "  - " << warning << "\n";
        }
    }
    
    return oss.str();
}

// ============ CodeGenerator类实现 ============

CodeGenerator::CodeGenerator(const CodeGenConfig& cfg) 
    : ir(std::make_unique<IntermediateCode>()), config(cfg) {
}

CodeGenResult CodeGenerator::generate(ASTNode* root) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 清空之前的状态
    clear();
    
    CodeGenResult result;
    
    if (!root) {
        addError("Empty AST root");
        result.success = false;
        result.errors = errors;
        return result;
    }
    
    try {
        // 根据AST节点类型进行代码生成
        if (auto programNode = dynamic_cast<ProgramNode*>(root)) {
            generateProgram(programNode);
        } else {
            addError("Invalid root node type");
        }
        
        // 执行优化
        if (config.enableOptimization) {
            if (config.enableConstantFolding) {
                ir->constantFolding();
            }
            if (config.enableDeadCodeElim) {
                ir->deadCodeElimination();
            }
        }
        
        // 构建基本块和控制流图
        ir->buildBasicBlocks();
        ir->buildControlFlowGraph();
        
        result.success = errors.empty();
        result.errors = errors;
        result.warnings = warnings;
        result.intermediateCode = std::move(ir);
        result.statistics = result.intermediateCode->getStatistics();
        
        // 重新创建ir对象供下次使用
        ir = std::make_unique<IntermediateCode>();
        
    } catch (const std::exception& e) {
        addError("Code generation exception: " + std::string(e.what()));
        result.success = false;
        result.errors = errors;
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    return result;
}

void CodeGenerator::generateProgram(ProgramNode* node) {
    if (!node) {
        addError("Null program node");
        return;
    }
    
    if (config.generateComments) {
        addComment("Program start");
    }
    
    // 生成所有声明
    for (const auto& decl : node->declarations) {
        if (decl) {
            generateDeclaration(decl.get());
        }
    }
    
    if (config.generateComments) {
        addComment("Program end");
    }
}

void CodeGenerator::generateDeclaration(ASTNode* node) {
    if (!node) {
        return;
    }
    
    switch (node->nodeType) {
        case ASTNodeType::VAR_DECL:
            generateVariableDecl(dynamic_cast<VariableDeclNode*>(node));
            break;
        case ASTNodeType::FUNC_DECL:
            generateFunctionDecl(dynamic_cast<FunctionDeclNode*>(node));
            break;
        default:
            addError("Unknown declaration type");
            break;
    }
}

void CodeGenerator::generateVariableDecl(VariableDeclNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Variable declaration: " + node->name);
    }
    
    // 如果有初始化表达式，生成赋值
    if (node->initializer) {
        ExprGenResult initResult = generateExpression(node->initializer.get());
        
        // 生成赋值指令
        auto assignInstr = InstructionUtils::createAssign(
            OperandUtils::createVariable(node->name, dataTypeToIRDataType(DataType::UNKNOWN)),
            OperandUtils::createVariable(initResult.operand, initResult.dataType),
            node->line
        );
        
        if (config.generateComments) {
            assignInstr->comment = "Variable initialization";
        }
        
        addInstruction(std::move(assignInstr));
    }
}

void CodeGenerator::generateFunctionDecl(FunctionDeclNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Function: " + node->name);
    }
    
    // 生成函数标签
    auto funcLabel = InstructionUtils::createLabel(
        OperandUtils::createLabel(node->name), node->line);
    addInstruction(std::move(funcLabel));
    
    // 生成函数体
    if (node->body) {
        generateBlockStmt(node->body.get());
    }
    
    // 如果函数没有显式返回，添加默认返回
    auto returnInstr = InstructionUtils::createReturn(nullptr, node->line);
    if (config.generateComments) {
        returnInstr->comment = "Default return";
    }
    addInstruction(std::move(returnInstr));
}

void CodeGenerator::generateStatement(StmtNode* node) {
    if (!node) {
        return;
    }
    
    switch (node->nodeType) {
        case ASTNodeType::ASSIGNMENT_STMT:
            generateAssignmentStmt(dynamic_cast<AssignmentStmtNode*>(node));
            break;
        case ASTNodeType::IF_STMT:
            generateIfStmt(dynamic_cast<IfStmtNode*>(node));
            break;
        case ASTNodeType::WHILE_STMT:
            generateWhileStmt(dynamic_cast<WhileStmtNode*>(node));
            break;
        case ASTNodeType::FOR_STMT:
            generateForStmt(dynamic_cast<ForStmtNode*>(node));
            break;
        case ASTNodeType::DO_WHILE_STMT:
            generateDoWhileStmt(dynamic_cast<DoWhileStmtNode*>(node));
            break;
        case ASTNodeType::BREAK_STMT:
            generateBreakStmt(dynamic_cast<BreakStmtNode*>(node));
            break;
        case ASTNodeType::CONTINUE_STMT:
            generateContinueStmt(dynamic_cast<ContinueStmtNode*>(node));
            break;
        case ASTNodeType::GOTO_STMT:
            generateGotoStmt(dynamic_cast<GotoStmtNode*>(node));
            break;
        case ASTNodeType::LABEL_STMT:
            generateLabelStmt(dynamic_cast<LabelStmtNode*>(node));
            break;
        case ASTNodeType::SWITCH_STMT:
            generateSwitchStmt(dynamic_cast<SwitchStmtNode*>(node));
            break;
        case ASTNodeType::BLOCK_STMT:
            generateBlockStmt(dynamic_cast<BlockStmtNode*>(node));
            break;
        case ASTNodeType::RETURN_STMT:
            generateReturnStmt(dynamic_cast<ReturnStmtNode*>(node));
            break;
        case ASTNodeType::EXPR_STMT:
            generateExpressionStmt(dynamic_cast<ExpressionStmtNode*>(node));
            break;
        default:
            addError("Unknown statement type");
            break;
    }
}

void CodeGenerator::generateAssignmentStmt(AssignmentStmtNode* node) {
    if (!node || !node->lvalue || !node->rvalue) {
        addError("Invalid assignment statement");
        return;
    }
    
    // 生成右值表达式
    ExprGenResult rvalueResult = generateExpression(node->rvalue.get());
    
    // 生成左值表达式（应该是变量）
    ExprGenResult lvalueResult = generateExpression(node->lvalue.get());
    
    // 生成赋值指令
    auto assignInstr = InstructionUtils::createAssign(
        OperandUtils::createVariable(lvalueResult.operand, lvalueResult.dataType),
        OperandUtils::createVariable(rvalueResult.operand, rvalueResult.dataType),
        node->line
    );
    
    if (config.generateComments) {
        assignInstr->comment = "Assignment";
    }
    
    addInstruction(std::move(assignInstr));
}

void CodeGenerator::generateIfStmt(IfStmtNode* node) {
    if (!node) {
        return;
    }
    
    std::string elseLabel = newLabel();
    std::string endLabel = newLabel();
    
    if (config.generateComments) {
        addComment("If statement");
    }
    
    // 生成条件表达式
    ExprGenResult condResult = generateExpression(node->condition.get());
    
    // 生成条件跳转（如果条件为假，跳转到else或end）
    std::string jumpTarget = node->elseStmt ? elseLabel : endLabel;
    auto condJump = InstructionUtils::createConditionalJump(
        OpType::IF_FALSE,
        OperandUtils::createVariable(condResult.operand, condResult.dataType),
        OperandUtils::createLabel(jumpTarget),
        node->line
    );
    addInstruction(std::move(condJump));
    
    // 生成then语句
    if (node->thenStmt) {
        generateStatement(node->thenStmt.get());
    }
    
    // 如果有else分支，生成跳转到end的指令
    if (node->elseStmt) {
        auto gotoEnd = InstructionUtils::createGoto(
            OperandUtils::createLabel(endLabel), node->line);
        addInstruction(std::move(gotoEnd));
        
        // 生成else标签
        auto elseLabelInstr = InstructionUtils::createLabel(
            OperandUtils::createLabel(elseLabel), node->line);
        addInstruction(std::move(elseLabelInstr));
        
        // 生成else语句
        generateStatement(node->elseStmt.get());
    }
    
    // 生成end标签
    auto endLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(endLabel), node->line);
    addInstruction(std::move(endLabelInstr));
}

void CodeGenerator::generateWhileStmt(WhileStmtNode* node) {
    if (!node) {
        return;
    }
    
    std::string loopLabel = newLabel();
    std::string endLabel = newLabel();
    
    // 保存之前的break/continue标签
    std::string oldBreakLabel = breakLabel;
    std::string oldContinueLabel = continueLabel;
    
    breakLabel = endLabel;
    continueLabel = loopLabel;
    
    if (config.generateComments) {
        addComment("While loop");
    }
    
    // 生成循环开始标签
    auto loopLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(loopLabel), node->line);
    addInstruction(std::move(loopLabelInstr));
    
    // 生成条件表达式
    ExprGenResult condResult = generateExpression(node->condition.get());
    
    // 生成条件跳转（如果条件为假，跳出循环）
    auto condJump = InstructionUtils::createConditionalJump(
        OpType::IF_FALSE,
        OperandUtils::createVariable(condResult.operand, condResult.dataType),
        OperandUtils::createLabel(endLabel),
        node->line
    );
    addInstruction(std::move(condJump));
    
    // 生成循环体
    if (node->body) {
        generateStatement(node->body.get());
    }
    
    // 生成跳转回循环开始
    auto gotoLoop = InstructionUtils::createGoto(
        OperandUtils::createLabel(loopLabel), node->line);
    addInstruction(std::move(gotoLoop));
    
    // 生成循环结束标签
    auto endLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(endLabel), node->line);
    addInstruction(std::move(endLabelInstr));
    
    // 恢复之前的标签
    breakLabel = oldBreakLabel;
    continueLabel = oldContinueLabel;
}

void CodeGenerator::generateBlockStmt(BlockStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Block start");
    }
    
    // 生成所有语句
    for (const auto& stmt : node->statements) {
        if (stmt) {
            generateStatement(stmt.get());
        }
    }
    
    if (config.generateComments) {
        addComment("Block end");
    }
}

void CodeGenerator::generateReturnStmt(ReturnStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Return statement");
    }
    
    std::unique_ptr<ThreeAddressCode> returnInstr;
    
    if (node->expression) {
        // 生成返回值表达式
        ExprGenResult exprResult = generateExpression(node->expression.get());
        
        returnInstr = InstructionUtils::createReturn(
            OperandUtils::createVariable(exprResult.operand, exprResult.dataType),
            node->line
        );
    } else {
        returnInstr = InstructionUtils::createReturn(nullptr, node->line);
    }
    
    addInstruction(std::move(returnInstr));
}

void CodeGenerator::generateExpressionStmt(ExpressionStmtNode* node) {
    if (!node || !node->expression) {
        return;
    }
    
    // 生成表达式（可能有副作用）
    generateExpression(node->expression.get());
}

ExprGenResult CodeGenerator::generateExpression(ExprNode* node) {
    if (!node) {
        return ExprGenResult("", IRDataType::UNKNOWN);
    }
    
    switch (node->nodeType) {
        case ASTNodeType::BINARY_EXPR:
            return generateBinaryExpression(dynamic_cast<BinaryExprNode*>(node));
        case ASTNodeType::UNARY_EXPR:
            return generateUnaryExpression(dynamic_cast<UnaryExprNode*>(node));
        case ASTNodeType::IDENTIFIER_EXPR:
            return generateIdentifier(dynamic_cast<IdentifierNode*>(node));
        case ASTNodeType::LITERAL_EXPR:
            return generateLiteral(dynamic_cast<LiteralNode*>(node));
        default:
            addError("Unknown expression type");
            return ExprGenResult("", IRDataType::UNKNOWN);
    }
}

ExprGenResult CodeGenerator::generateBinaryExpression(BinaryExprNode* node) {
    if (!node || !node->left || !node->right) {
        addError("Invalid binary expression");
        return ExprGenResult("", IRDataType::UNKNOWN);
    }
    
    // 生成左右操作数
    ExprGenResult leftResult = generateExpression(node->left.get());
    ExprGenResult rightResult = generateExpression(node->right.get());
    
    // 生成临时变量存储结果
    std::string tempVar = newTemp();
    IRDataType resultType = leftResult.dataType; // 简化：假设结果类型与左操作数相同
    
    // 转换TokenType到OpType
    OpType opType = tokenTypeToOpType(node->operator_);
    
    // 生成二元运算指令
    auto binaryInstr = InstructionUtils::createBinaryOp(
        opType,
        OperandUtils::createTemporary(tempVar, resultType),
        OperandUtils::createVariable(leftResult.operand, leftResult.dataType),
        OperandUtils::createVariable(rightResult.operand, rightResult.dataType),
        node->line
    );
    
    if (config.generateComments) {
        binaryInstr->comment = "Binary operation";
    }
    
    addInstruction(std::move(binaryInstr));
    
    return ExprGenResult(tempVar, resultType, true);
}

ExprGenResult CodeGenerator::generateUnaryExpression(UnaryExprNode* node) {
    if (!node || !node->operand) {
        addError("Invalid unary expression");
        return ExprGenResult("", IRDataType::UNKNOWN);
    }
    
    // 生成操作数
    ExprGenResult operandResult = generateExpression(node->operand.get());
    
    // 生成临时变量存储结果
    std::string tempVar = newTemp();
    IRDataType resultType = operandResult.dataType;
    
    // 转换TokenType到OpType
    OpType opType = tokenTypeToOpType(node->operator_);
    
    // 生成一元运算指令
    auto unaryInstr = InstructionUtils::createUnaryOp(
        opType,
        OperandUtils::createTemporary(tempVar, resultType),
        OperandUtils::createVariable(operandResult.operand, operandResult.dataType),
        node->line
    );
    
    if (config.generateComments) {
        unaryInstr->comment = "Unary operation";
    }
    
    addInstruction(std::move(unaryInstr));
    
    return ExprGenResult(tempVar, resultType, true);
}

ExprGenResult CodeGenerator::generateIdentifier(IdentifierNode* node) {
    if (!node) {
        return ExprGenResult("", IRDataType::UNKNOWN);
    }
    
    return ExprGenResult(node->name, IRDataType::UNKNOWN, false);
}

ExprGenResult CodeGenerator::generateLiteral(LiteralNode* node) {
    if (!node) {
        return ExprGenResult("", IRDataType::UNKNOWN);
    }
    
    IRDataType dataType = IRDataType::UNKNOWN;
    
    // 根据TokenType确定数据类型
    switch (node->valueType) {
        case TokenType::NUMBER:
            dataType = IRDataType::INT;
            break;
        case TokenType::REAL:
            dataType = IRDataType::FLOAT;
            break;
        case TokenType::STRING:
            dataType = IRDataType::STRING;
            break;
        case TokenType::TRUE:
        case TokenType::FALSE:
            dataType = IRDataType::BOOL;
            break;
        default:
            dataType = IRDataType::UNKNOWN;
            break;
    }
    
    return ExprGenResult(node->value, dataType, false);
}

// ============ 私有方法实现 ============

void CodeGenerator::addError(const std::string& message) {
    errors.push_back(message);
}

void CodeGenerator::addWarning(const std::string& message) {
    warnings.push_back(message);
}

void CodeGenerator::clear() {
    errors.clear();
    warnings.clear();
    ir->clear();
    breakLabel.clear();
    continueLabel.clear();
}

OpType CodeGenerator::tokenTypeToOpType(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::PLUS: return OpType::ADD;
        case TokenType::MINUS: return OpType::SUB;
        case TokenType::MULTIPLY: return OpType::MUL;
        case TokenType::DIVIDE: return OpType::DIV;
        case TokenType::MODULO: return OpType::MOD;
        case TokenType::AND: return OpType::AND;
        case TokenType::OR: return OpType::OR;
        case TokenType::NOT: return OpType::NOT;
        case TokenType::EQ: return OpType::EQ;
        case TokenType::NE: return OpType::NE;
        case TokenType::LT: return OpType::LT;
        case TokenType::LE: return OpType::LE;
        case TokenType::GT: return OpType::GT;
        case TokenType::GE: return OpType::GE;
        case TokenType::ASSIGN: return OpType::ASSIGN;
        default: return OpType::NOP;
    }
}

IRDataType CodeGenerator::dataTypeToIRDataType(DataType dataType) {
    switch (dataType) {
        case DataType::VOID: return IRDataType::VOID;
        case DataType::INT: return IRDataType::INT;
        case DataType::FLOAT: return IRDataType::FLOAT;
        case DataType::BOOL: return IRDataType::BOOL;
        case DataType::CHAR: return IRDataType::CHAR;
        case DataType::STRING: return IRDataType::STRING;
        case DataType::POINTER: return IRDataType::POINTER;
        default: return IRDataType::UNKNOWN;
    }
}

std::string CodeGenerator::newTemp() {
    return ir->newTemp();
}

std::string CodeGenerator::newLabel() {
    return ir->newLabel();
}

void CodeGenerator::addInstruction(std::unique_ptr<ThreeAddressCode> instr) {
    ir->addInstruction(std::move(instr));
}

void CodeGenerator::addComment(const std::string& comment) {
    // 创建一个NOP指令作为注释
    auto commentInstr = std::make_unique<ThreeAddressCode>(OpType::NOP);
    commentInstr->comment = comment;
    addInstruction(std::move(commentInstr));
}

// ============ 新增语句生成方法实现 ============

void CodeGenerator::generateForStmt(ForStmtNode* node) {
    if (!node) {
        return;
    }
    
    std::string loopLabel = newLabel();
    std::string updateLabel = newLabel();
    std::string endLabel = newLabel();
    
    // 保存之前的break/continue标签
    std::string oldBreakLabel = breakLabel;
    std::string oldContinueLabel = continueLabel;
    
    breakLabel = endLabel;
    continueLabel = updateLabel;
    
    if (config.generateComments) {
        addComment("For loop");
    }
    
    // 生成初始化语句
    if (node->init) {
        generateStatement(node->init.get());
    }
    
    // 生成循环开始标签
    auto loopLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(loopLabel), node->line);
    addInstruction(std::move(loopLabelInstr));
    
    // 生成条件检查
    if (node->condition) {
        ExprGenResult condResult = generateExpression(node->condition.get());
        
        // 生成条件跳转（如果条件为假，跳出循环）
        auto condJump = InstructionUtils::createConditionalJump(
            OpType::IF_FALSE,
            OperandUtils::createVariable(condResult.operand, condResult.dataType),
            OperandUtils::createLabel(endLabel),
            node->line
        );
        addInstruction(std::move(condJump));
    }
    
    // 生成循环体
    if (node->body) {
        generateStatement(node->body.get());
    }
    
    // 生成更新标签（continue跳转目标）
    auto updateLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(updateLabel), node->line);
    addInstruction(std::move(updateLabelInstr));
    
    // 生成更新表达式
    if (node->update) {
        generateExpression(node->update.get());
    }
    
    // 生成跳转回循环开始
    auto gotoLoop = InstructionUtils::createGoto(
        OperandUtils::createLabel(loopLabel), node->line);
    addInstruction(std::move(gotoLoop));
    
    // 生成循环结束标签
    auto endLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(endLabel), node->line);
    addInstruction(std::move(endLabelInstr));
    
    // 恢复之前的标签
    breakLabel = oldBreakLabel;
    continueLabel = oldContinueLabel;
}

void CodeGenerator::generateDoWhileStmt(DoWhileStmtNode* node) {
    if (!node) {
        return;
    }
    
    std::string loopLabel = newLabel();
    std::string condLabel = newLabel();
    std::string endLabel = newLabel();
    
    // 保存之前的break/continue标签
    std::string oldBreakLabel = breakLabel;
    std::string oldContinueLabel = continueLabel;
    
    breakLabel = endLabel;
    continueLabel = condLabel;
    
    if (config.generateComments) {
        addComment("Do-while loop");
    }
    
    // 生成循环开始标签
    auto loopLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(loopLabel), node->line);
    addInstruction(std::move(loopLabelInstr));
    
    // 生成循环体
    if (node->body) {
        generateStatement(node->body.get());
    }
    
    // 生成条件检查标签（continue跳转目标）
    auto condLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(condLabel), node->line);
    addInstruction(std::move(condLabelInstr));
    
    // 生成条件表达式
    if (node->condition) {
        ExprGenResult condResult = generateExpression(node->condition.get());
        
        // 生成条件跳转（如果条件为真，继续循环）
        auto condJump = InstructionUtils::createConditionalJump(
            OpType::IF_TRUE,
            OperandUtils::createVariable(condResult.operand, condResult.dataType),
            OperandUtils::createLabel(loopLabel),
            node->line
        );
        addInstruction(std::move(condJump));
    }
    
    // 生成循环结束标签
    auto endLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(endLabel), node->line);
    addInstruction(std::move(endLabelInstr));
    
    // 恢复之前的标签
    breakLabel = oldBreakLabel;
    continueLabel = oldContinueLabel;
}

void CodeGenerator::generateBreakStmt(BreakStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Break statement");
    }
    
    if (breakLabel.empty()) {
        addError("Break statement outside of loop or switch");
        return;
    }
    
    // 生成跳转到break标签
    auto breakJump = InstructionUtils::createGoto(
        OperandUtils::createLabel(breakLabel), node->line);
    addInstruction(std::move(breakJump));
}

void CodeGenerator::generateContinueStmt(ContinueStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Continue statement");
    }
    
    if (continueLabel.empty()) {
        addError("Continue statement outside of loop");
        return;
    }
    
    // 生成跳转到continue标签
    auto continueJump = InstructionUtils::createGoto(
        OperandUtils::createLabel(continueLabel), node->line);
    addInstruction(std::move(continueJump));
}

void CodeGenerator::generateGotoStmt(GotoStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Goto statement to label: " + node->label);
    }
    
    // 生成跳转到指定标签
    auto gotoJump = InstructionUtils::createGoto(
        OperandUtils::createLabel(node->label), node->line);
    addInstruction(std::move(gotoJump));
}

void CodeGenerator::generateLabelStmt(LabelStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Label: " + node->label);
    }
    
    // 生成标签
    auto labelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(node->label), node->line);
    addInstruction(std::move(labelInstr));
    
    // 如果有关联语句，生成它
    if (node->statement) {
        generateStatement(node->statement.get());
    }
}

void CodeGenerator::generateSwitchStmt(SwitchStmtNode* node) {
    if (!node) {
        return;
    }
    
    if (config.generateComments) {
        addComment("Switch statement");
    }
    
    std::string endLabel = newLabel();
    std::vector<std::string> caseLabels;
    std::string defaultLabel;
    
    // 保存之前的break标签
    std::string oldBreakLabel = breakLabel;
    breakLabel = endLabel;
    
    // 生成switch表达式
    ExprGenResult switchResult = generateExpression(node->expression.get());
    
    // 为每个case生成标签
    for (size_t i = 0; i < node->cases.size(); ++i) {
        caseLabels.push_back(newLabel());
    }
    
    // 如果有default，生成default标签
    if (node->defaultCase) {
        defaultLabel = newLabel();
    }
    
    // 生成case比较和跳转
    for (size_t i = 0; i < node->cases.size(); ++i) {
        const auto& caseStmt = node->cases[i];
        if (caseStmt && caseStmt->value) {
            // 生成case值表达式
            ExprGenResult caseResult = generateExpression(caseStmt->value.get());
            
            // 生成比较临时变量
            std::string tempVar = newTemp();
            auto compareInstr = InstructionUtils::createBinaryOp(
                OpType::EQ,
                OperandUtils::createTemporary(tempVar, switchResult.dataType),
                OperandUtils::createVariable(switchResult.operand, switchResult.dataType),
                OperandUtils::createVariable(caseResult.operand, caseResult.dataType),
                node->line
            );
            addInstruction(std::move(compareInstr));
            
            // 生成条件跳转到case标签
            auto caseJump = InstructionUtils::createConditionalJump(
                OpType::IF_TRUE,
                OperandUtils::createTemporary(tempVar, switchResult.dataType),
                OperandUtils::createLabel(caseLabels[i]),
                node->line
            );
            addInstruction(std::move(caseJump));
        }
    }
    
    // 如果没有匹配的case，跳转到default或end
    std::string finalTarget = node->defaultCase ? defaultLabel : endLabel;
    auto defaultJump = InstructionUtils::createGoto(
        OperandUtils::createLabel(finalTarget), node->line);
    addInstruction(std::move(defaultJump));
    
    // 生成各个case的代码
    for (size_t i = 0; i < node->cases.size(); ++i) {
        const auto& caseStmt = node->cases[i];
        if (caseStmt) {
            // 生成case标签
            auto caseLabelInstr = InstructionUtils::createLabel(
                OperandUtils::createLabel(caseLabels[i]), caseStmt->line);
            addInstruction(std::move(caseLabelInstr));
            
            // 生成case语句
            for (const auto& stmt : caseStmt->statements) {
                if (stmt) {
                    generateStatement(stmt.get());
                }
            }
        }
    }
    
    // 生成default case（如果有）
    if (node->defaultCase) {
        auto defaultLabelInstr = InstructionUtils::createLabel(
            OperandUtils::createLabel(defaultLabel), node->line);
        addInstruction(std::move(defaultLabelInstr));
        
        generateStatement(node->defaultCase.get());
    }
    
    // 生成switch结束标签
    auto endLabelInstr = InstructionUtils::createLabel(
        OperandUtils::createLabel(endLabel), node->line);
    addInstruction(std::move(endLabelInstr));
    
    // 恢复之前的break标签
    breakLabel = oldBreakLabel;
}

// ============ CodeGeneratorFactory类实现 ============

std::unique_ptr<CodeGenerator> CodeGeneratorFactory::createStandard() {
    CodeGenConfig config;
    config.enableOptimization = true;
    config.generateComments = true;
    config.enableConstantFolding = true;
    config.enableDeadCodeElim = false;
    
    return std::make_unique<CodeGenerator>(config);
}

std::unique_ptr<CodeGenerator> CodeGeneratorFactory::createOptimized() {
    CodeGenConfig config;
    config.enableOptimization = true;
    config.generateComments = false;
    config.enableConstantFolding = true;
    config.enableDeadCodeElim = true;
    
    return std::make_unique<CodeGenerator>(config);
}

std::unique_ptr<CodeGenerator> CodeGeneratorFactory::createDebug() {
    CodeGenConfig config;
    config.enableOptimization = false;
    config.generateComments = true;
    config.enableConstantFolding = false;
    config.enableDeadCodeElim = false;
    
    return std::make_unique<CodeGenerator>(config);
} 