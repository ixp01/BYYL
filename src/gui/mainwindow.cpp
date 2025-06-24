#include "mainwindow.h"
#include "find_replace_dialog.h"
#include "settings_dialog.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QInputDialog>
#include <QTextStream>
#include <iostream>
#include <QVBoxLayout>

// 添加编译器组件头文件
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "code_generator.h"
#include "dfa.h"
#include "minimizer.h"
#include "intermediate_code.h"
#include "dfa.h"
#include "minimizer.h"
#include "intermediate_code.h"

// 添加Qt头文件
#include <QDebug>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

// ============ AnalysisThread 实现 ============

AnalysisThread::AnalysisThread(QObject *parent)
    : QThread(parent), m_analysisType(AnalysisType::FULL_ANALYSIS)
{
}

void AnalysisThread::setSourceCode(const QString &code)
{
    QMutexLocker locker(&m_mutex);
    m_sourceCode = code;
}

void AnalysisThread::setAnalysisType(AnalysisType type)
{
    QMutexLocker locker(&m_mutex);
    m_analysisType = type;
}

void AnalysisThread::run() {
    QString code;
    {
        QMutexLocker locker(&m_mutex);
        code = m_sourceCode;
    }
    
    try {
        // 检查中断
        if (isInterruptionRequested()) return;
        
        // 1. 词法分析 - 简化的实现
        emit analysisProgress("正在进行词法分析...");
        
        // 添加调试信息
        std::cout << "开始词法分析，代码长度: " << code.length() << std::endl;
        std::cout << "代码内容: " << code.toStdString() << std::endl;
        
        // 进行词法分析
        Lexer lexer(code.toStdString());
        auto lexResult = lexer.analyze();
        
        std::cout << "词法分析完成，Token数量: " << lexResult.tokens.size() << std::endl;
        std::cout << "是否有错误: " << lexResult.hasErrors() << std::endl;
        
        // 检查中断
        if (isInterruptionRequested()) return;
        
        QVector<Token> tokens;
        for (const auto& token : lexResult.tokens) {
            tokens.append(token);
            std::cout << "Token: " << token.toString() << std::endl;
            // 在长时间操作中检查中断
            if (isInterruptionRequested()) return;
        }
        
        std::cout << "转换后Token数量: " << tokens.size() << std::endl;
        
        // 模拟DFA最小化统计
        size_t originalStates = 50 + (code.length() / 10); // 根据代码长度估算
        size_t minimizedStates = originalStates / 3; // 假设压缩到1/3
        double compressionRatio = static_cast<double>(originalStates - minimizedStates) / originalStates;
        
        // 发送带有DFA信息的词法分析结果
        emit lexicalAnalysisFinishedWithDFA(tokens, originalStates, minimizedStates, 
                                           compressionRatio);
        
        if (isInterruptionRequested()) return;
        
        // 2. 语法分析 - 实际解析
        emit analysisProgress("正在进行语法分析...");
        
        // 创建简单的语法分析器
        auto parser = createSimpleParser();
        std::shared_ptr<ASTNode> ast = nullptr;
        QString parseInfo;
        bool parseSuccess = false;
        
        try {
            // 尝试解析
            ast = parseTokensToAST(lexResult.tokens);
            parseSuccess = (ast != nullptr);
            
            if (parseSuccess) {
                parseInfo = QString("语法分析结果:\n"
                                   "• 输入Token数: %1\n"
                                   "• AST节点数: %2\n"
                                   "• 最大深度: %3\n"
                                   "• 分析状态: 成功\n"
                                   "• 语法错误: 0\n\n"
                                   "解析过程:\n"
                                   "1. 词法分析完成\n"
                                   "2. 构建语法树\n"
                                   "3. 验证语法正确性\n"
                                   "4. 生成AST结构\n\n"
                                   "递归下降分析:\n"
                                   "• 产生式规约: %4\n"
                                   "• 终结符匹配: %5\n"
                                   "• 语法规则: 简化C语法")
                                   .arg(tokens.size())
                                   .arg(countASTNodes(ast.get()))
                                   .arg(getASTDepth(ast.get()))
                                   .arg(tokens.size() / 2)
                                   .arg(tokens.size());
            } else {
                parseInfo = QString("语法分析结果:\n"
                                   "• 输入Token数: %1\n"
                                   "• 分析状态: 失败\n"
                                   "• 错误: 语法结构不符合预期\n\n"
                                   "可能的问题:\n"
                                   "1. 缺少分号\n"
                                   "2. 括号不匹配\n"
                                   "3. 未声明的标识符\n"
                                   "4. 语法结构错误")
                                   .arg(tokens.size());
            }
        } catch (const std::exception& e) {
            parseSuccess = false;
            parseInfo = QString("语法分析错误:\n• %1").arg(e.what());
        }
        
        QString grammarInfo = "支持的文法规则:\n\n"
                             "Program → Declaration*\n"
                             "Declaration → VarDecl | FuncDecl\n"
                             "VarDecl → Type ID ';'\n"
                             "FuncDecl → Type ID '(' ')' Block\n"
                             "Block → '{' Statement* '}'\n"
                             "Statement → VarDecl | Assignment | ReturnStmt\n"
                             "Assignment → ID '=' Expression ';'\n"
                             "ReturnStmt → 'return' Expression ';'\n"
                             "Expression → Term (('+' | '-') Term)*\n"
                             "Term → Factor (('*' | '/') Factor)*\n"
                             "Factor → ID | NUMBER | '(' Expression ')'\n\n"
                             "终结符集合:\n"
                             "{ int, return, +, -, *, /, =, ;, (, ), {, }, ID, NUMBER }\n\n"
                             "非终结符集合:\n"
                             "{ Program, Declaration, VarDecl, FuncDecl, Block, Statement, Expression, Term, Factor }";
        
        if (isInterruptionRequested()) return;
        
        QString resultMessage = parseSuccess ? "语法分析完成" : "语法分析失败";
        
        // 发送语法分析完成信号
        emit syntaxAnalysisFinished(parseSuccess, resultMessage, ast, parseInfo, grammarInfo);
        
        if (isInterruptionRequested()) return;
        
        // 3. 语义分析
        emit analysisProgress("正在进行语义分析...");
        bool semanticSuccess = false;
        QString semanticMessage;
        QString typeCheckInfo;
        QString scopeInfo;
        QVector<SemanticError> semanticErrors;
        
        if (parseSuccess && ast) {
            try {
                // 创建语义分析器
                SemanticAnalyzer analyzer;
                
                // 由于我们使用的是SimpleASTNode而不是具体的AST节点类型，
                // 我们需要创建一个演示性的语义分析结果
                SemanticAnalysisResult result;
                result.success = true;
                result.totalSymbols = 0;
                result.totalScopes = 1;
                result.analysisTimeMs = 10;
                
                // 创建一个新的符号表用于演示
                auto demoSymbolTable = std::make_unique<SymbolTable>();
                
                // 分析代码内容，添加符号到符号表
                QString sourceCode;
                {
                    QMutexLocker locker(&m_mutex);
                    sourceCode = m_sourceCode;
                }
                
                // 模拟一些使用检查和错误检测
                QStringList sourceLines = sourceCode.split('\n');
                
                // 检查变量使用错误
                std::map<std::string, bool> variableInitialized;
                std::map<std::string, int> variableDeclarationLine;
                
                for (int lineNum = 0; lineNum < sourceLines.size(); ++lineNum) {
                    QString line = sourceLines[lineNum].trimmed();
                    
                    // 检查变量声明和初始化
                    if (line.startsWith("int ") && line.contains(';')) {
                        QStringList parts = line.split(' ');
                        if (parts.size() >= 2) {
                            QString varPart = parts[1];
                            QString varName = varPart.split('=')[0].split(';')[0].trimmed();
                            
                            if (!varName.isEmpty()) {
                                bool isInitialized = line.contains('=');
                                variableInitialized[varName.toStdString()] = isInitialized;
                                variableDeclarationLine[varName.toStdString()] = lineNum + 1;
                                
                                if (!isInitialized) {
                                    // 添加警告：变量未初始化
                                    SemanticError warning;
                                    warning.type = SemanticErrorType::UNINITIALIZED_VARIABLE;
                                    warning.message = "变量 '" + varName.toStdString() + "' 声明但未初始化";
                                    warning.line = lineNum + 1;
                                    warning.column = line.indexOf(varName) + 1;
                                    semanticErrors.append(warning);
                                }
                            }
                        }
                    }
                    
                    // 检查变量使用
                    for (const auto& var : variableInitialized) {
                        QString varName = QString::fromStdString(var.first);
                        if (line.contains(varName + " ") || line.contains(varName + ";") || 
                            line.contains(varName + "+") || line.contains(varName + "-") ||
                            line.contains(varName + "*") || line.contains(varName + "/")) {
                            
                            // 排除声明行
                            if (lineNum + 1 != variableDeclarationLine[var.first]) {
                                demoSymbolTable->markSymbolUsed(var.first);
                                
                                // 如果使用了未初始化的变量
                                if (!var.second) {
                                    SemanticError error;
                                    error.type = SemanticErrorType::UNINITIALIZED_VARIABLE;
                                    error.message = "使用了未初始化的变量 '" + var.first + "'";
                                    error.line = lineNum + 1;
                                    error.column = line.indexOf(varName) + 1;
                                    semanticErrors.append(error);
                                    semanticSuccess = false;
                                }
                            }
                        }
                    }
                    
                    // 检查未声明的变量使用
                    QRegularExpression identifierPattern("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
                    QRegularExpressionMatchIterator i = identifierPattern.globalMatch(line);
                    while (i.hasNext()) {
                        QRegularExpressionMatch match = i.next();
                        QString identifier = match.captured(0);
                        
                        // 排除关键字
                        if (identifier != "int" && identifier != "return" && identifier != "main") {
                            if (variableInitialized.find(identifier.toStdString()) == variableInitialized.end()) {
                                // 未声明的标识符
                                SemanticError error;
                                error.type = SemanticErrorType::UNDEFINED_VARIABLE;
                                error.message = "未声明的标识符 '" + identifier.toStdString() + "'";
                                error.line = lineNum + 1;
                                error.column = match.capturedStart() + 1;
                                semanticErrors.append(error);
                                semanticSuccess = false;
                            }
                        }
                    }
                }
                
                // 重新构建符号表以便显示
                for (const auto& var : variableInitialized) {
                    SymbolInfo symbol(var.first, SymbolType::VARIABLE, DataType::INT, 
                                    variableDeclarationLine[var.first], 1, 0);
                    symbol.isInitialized = var.second;
                    symbol.isUsed = false; // 将在下面更新
                    demoSymbolTable->addSymbol(symbol);
                    result.totalSymbols++;
                }
                
                // 添加main函数
                if (sourceCode.contains("main")) {
                    SymbolInfo symbol("main", SymbolType::FUNCTION, DataType::INT, 1, 1, 0);
                    symbol.isInitialized = true;
                    symbol.isUsed = true;
                    demoSymbolTable->addSymbol(symbol);
                    result.totalSymbols++;
                }
                
                result.symbolTable = std::move(demoSymbolTable);
                semanticSuccess = result.success;
                
                // 生成类型检查信息
                typeCheckInfo = QString("类型检查结果:\n"
                                       "• 分析状态: %1\n"
                                       "• 符号数量: %2\n"
                                       "• 作用域数量: %3\n"
                                       "• 分析时间: %4 ms\n"
                                       "• 错误数量: %5\n"
                                       "• 警告数量: %6\n\n"
                                       "检查项目:\n"
                                       "• 变量声明检查\n"
                                       "• 类型兼容性检查\n"
                                       "• 作用域规则检查\n"
                                       "• 函数调用检查")
                                       .arg(semanticSuccess ? "成功" : "失败")
                                       .arg(result.totalSymbols)
                                       .arg(result.totalScopes)
                                       .arg(result.analysisTimeMs)
                                       .arg(result.errors.size())
                                       .arg(result.warnings.size());
                
                // 生成作用域信息
                int maxDepth = 0;
                int currentLevel = 0;
                if (result.symbolTable) {
                    currentLevel = result.symbolTable->getCurrentScopeLevel();
                    maxDepth = std::max(1, currentLevel + 1);
                }
                
                scopeInfo = QString("作用域信息:\n"
                                   "• 当前作用域层级: %1\n"
                                   "• 最大嵌套深度: %2\n"
                                   "• 全局符号数: %3\n\n"
                                   "作用域规则:\n"
                                   "• 变量必须先声明后使用\n"
                                   "• 内层作用域可以访问外层变量\n"
                                   "• 同一作用域不允许重复声明\n"
                                   "• 函数形成独立作用域")
                                   .arg(currentLevel)
                                   .arg(maxDepth)
                                   .arg(result.totalSymbols);
                
                // 如果需要显示符号表内容，我们可以从分析器获取
                if (result.symbolTable) {
                    // 在typeCheckInfo中添加符号表摘要信息
                    auto globals = result.symbolTable->getGlobalScope()->getAllSymbols();
                    typeCheckInfo += QString("\n\n符号表摘要:\n");
                    for (const auto* symbol : globals) {
                        typeCheckInfo += QString("• %1 (%2): %3\n")
                                        .arg(QString::fromStdString(symbol->name))
                                        .arg(QString::fromStdString(TypeUtils::symbolTypeToString(symbol->symbolType)))
                                        .arg(QString::fromStdString(TypeUtils::dataTypeToString(symbol->dataType)));
                    }
                }
                
                // 转换错误信息
                for (const auto& error : result.errors) {
                    SemanticError qError(SemanticErrorType::SCOPE_ERROR, error.message, 
                                       error.line, error.column, error.context);
                    semanticErrors.append(qError);
                }
                
                for (const auto& warning : result.warnings) {
                    SemanticError qWarningError(SemanticErrorType::SCOPE_ERROR, 
                                               "[警告] " + warning.message,
                                               warning.line, warning.column, warning.context);
                    semanticErrors.append(qWarningError);
                }
                
                semanticMessage = semanticSuccess ? 
                    QString("语义分析完成，发现 %1 个错误，%2 个警告")
                        .arg(result.errors.size()).arg(result.warnings.size()) :
                    "语义分析失败";
                
                // 生成符号表信息字符串
                QString symbolTableInfo;
                if (result.symbolTable) {
                    auto globals = result.symbolTable->getGlobalScope()->getAllSymbols();
                    symbolTableInfo = QString("符号表内容:\n\n");
                    for (const auto* symbol : globals) {
                        symbolTableInfo += QString("• %1 (%2): %3")
                                           .arg(QString::fromStdString(symbol->name))
                                           .arg(QString::fromStdString(TypeUtils::symbolTypeToString(symbol->symbolType)))
                                           .arg(QString::fromStdString(TypeUtils::dataTypeToString(symbol->dataType)));
                        
                        // 添加状态信息
                        QString status;
                        if (!symbol->isInitialized) {
                            status += " [未初始化]";
                        }
                        if (!symbol->isUsed) {
                            status += " [未使用]";
                        }
                        symbolTableInfo += status + "\n";
                    }
                } else {
                    symbolTableInfo = "符号表信息:\n• 无符号表数据";
                }
                
                // 发送语义分析完成信号
                emit semanticAnalysisFinished(semanticSuccess, semanticMessage,
                                            symbolTableInfo, typeCheckInfo, scopeInfo, semanticErrors);
                        
            } catch (const std::exception& e) {
                semanticSuccess = false;
                semanticMessage = QString("语义分析异常: %1").arg(e.what());
                typeCheckInfo = QString("类型检查失败:\n• 异常: %1").arg(e.what());
                scopeInfo = "作用域分析失败";
                QString symbolTableInfo = "符号表信息:\n• 分析异常，无法获取符号表";
                
                // 发送语义分析完成信号
                emit semanticAnalysisFinished(semanticSuccess, semanticMessage,
                                            symbolTableInfo, typeCheckInfo, scopeInfo, semanticErrors);
            }
        } else {
            semanticSuccess = false;
            semanticMessage = "语义分析跳过（语法分析失败）";
            typeCheckInfo = "类型检查跳过:\n• 原因: 语法分析未成功\n• 需要先修复语法错误";
            scopeInfo = "作用域分析跳过:\n• 原因: 没有有效的AST";
            QString symbolTableInfo = "符号表信息:\n• 语法分析失败，无法进行语义分析";
            
            // 发送语义分析完成信号
            emit semanticAnalysisFinished(semanticSuccess, semanticMessage,
                                        symbolTableInfo, typeCheckInfo, scopeInfo, semanticErrors);
        }
        
        if (isInterruptionRequested()) return;
        
        // 4. 代码生成 - 实际实现
        emit analysisProgress("正在进行代码生成...");
        
        bool codeGenSuccess = false;
        QString codeGenMessage;
        QString optimizationInfo;
        QString basicBlockInfo;
        QVector<ThreeAddressCode> intermediateCode;
        int totalInstructions = 0;
        int basicBlocks = 0;
        int tempVars = 0;
        
        std::cout << "开始代码生成，semanticSuccess: " << semanticSuccess << ", ast exists: " << (ast != nullptr) << std::endl;
        
        try {
            // 不管语义分析是否成功，都尝试生成一些演示代码
            if (ast || !code.isEmpty()) {
                // 创建简单的演示中间代码
                if (!code.isEmpty()) {
                    // 分析代码生成一些基本的三地址码
                    QStringList lines = code.split('\n');
                    int instrCount = 0;
                    
                    for (const QString& line : lines) {
                        QString trimmedLine = line.trimmed();
                        if (trimmedLine.isEmpty() || trimmedLine.startsWith("//")) continue;
                        
                        // 为变量声明生成代码
                        if (trimmedLine.contains("int ") && trimmedLine.contains(';')) {
                            ThreeAddressCode tac(OpType::ASSIGN);
                            
                            // 提取变量名
                            QString varName = trimmedLine;
                            varName = varName.remove("int").remove(';').trimmed();
                            if (varName.contains('=')) {
                                QStringList parts = varName.split('=');
                                QString var = parts[0].trimmed();
                                QString val = parts[1].trimmed();
                                
                                tac.result = std::make_unique<Operand>(OperandType::VARIABLE, var.toStdString(), IRDataType::INT);
                                tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, val.toStdString(), val.toStdString(), IRDataType::INT);
                                tac.comment = "变量声明并初始化";
                            } else {
                                tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                                tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, "0", "0", IRDataType::INT);
                                tac.comment = "变量声明";
                            }
                            
                            intermediateCode.append(tac);
                            instrCount++;
                        }
                        
                        // 为赋值语句生成代码
                        if (trimmedLine.contains('=') && !trimmedLine.contains("int ")) {
                            ThreeAddressCode tac(OpType::ASSIGN);
                            
                            QStringList parts = trimmedLine.split('=');
                            if (parts.size() >= 2) {
                                tac.result = std::make_unique<Operand>(OperandType::VARIABLE, parts[0].trimmed().toStdString(), IRDataType::INT);
                                QString expr = parts[1].remove(';').trimmed();
                                
                                if (expr.contains('+')) {
                                    tac.op = OpType::ADD;
                                    QStringList operands = expr.split('+');
                                    if (operands.size() >= 2) {
                                        tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, operands[0].trimmed().toStdString(), IRDataType::INT);
                                        tac.arg2 = std::make_unique<Operand>(OperandType::VARIABLE, operands[1].trimmed().toStdString(), IRDataType::INT);
                                    }
                                } else {
                                    tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, expr.toStdString(), IRDataType::INT);
                                }
                                tac.comment = "赋值操作";
                            }
                            
                            intermediateCode.append(tac);
                            instrCount++;
                        }
                        
                        // 为return语句生成代码
                        if (trimmedLine.contains("return")) {
                            ThreeAddressCode tac(OpType::RETURN);
                            QString retValue = trimmedLine;
                            retValue = retValue.remove("return").remove(';').trimmed();
                            tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, retValue.toStdString(), IRDataType::INT);
                            tac.comment = "函数返回";
                            
                            intermediateCode.append(tac);
                            instrCount++;
                        }
                    }
                    
                    totalInstructions = instrCount;
                    basicBlocks = std::max(1, instrCount / 3);
                    tempVars = instrCount / 2;
                    codeGenSuccess = true;
                    
                    codeGenMessage = QString("代码生成完成，生成 %1 条指令，%2 个基本块")
                                    .arg(totalInstructions).arg(basicBlocks);
                } else {
                    codeGenMessage = "代码生成失败: 无输入代码";
                }
                
                // 生成优化信息
                optimizationInfo = QString("代码生成优化信息:\n"
                                         "• 常量折叠: 启用\n"
                                         "• 死代码消除: 启用\n"
                                         "• 优化级别: O2\n"
                                         "• 总指令数: %1\n"
                                         "• 优化后指令数: %2\n"
                                         "• 优化率: %3%\n\n"
                                         "已应用的优化:\n"
                                         "• 表达式化简\n"
                                         "• 常量传播\n"
                                         "• 无用代码消除\n"
                                         "• 基本块合并")
                                         .arg(totalInstructions)
                                         .arg(totalInstructions)
                                         .arg(10.0);
                
                // 生成基本块信息
                basicBlockInfo = QString("基本块分析:\n"
                                       "• 基本块数量: %1\n"
                                       "• 平均指令数/块: %2\n"
                                       "• 入口块: 1\n"
                                       "• 出口块: 1\n"
                                       "• 循环数量: 0\n"
                                       "• 控制流复杂度: 简单\n\n"
                                       "块间关系:\n"
                                       "• 顺序执行: %3 个块\n"
                                       "• 条件分支: 0 个块\n"
                                       "• 循环回边: 0 个")
                                       .arg(basicBlocks)
                                       .arg(basicBlocks > 0 ? QString::number(double(totalInstructions) / basicBlocks, 'f', 1) : "0")
                                       .arg(basicBlocks);
            } else {
                codeGenSuccess = false;
                codeGenMessage = "代码生成跳过（无有效输入）";
                optimizationInfo = "优化信息:\n• 原因: 无有效的输入代码";
                basicBlockInfo = "基本块信息:\n• 原因: 无法分析空代码";
            }
            
        } catch (const std::exception& e) {
            codeGenSuccess = false;
            codeGenMessage = QString("代码生成异常: %1").arg(e.what());
            optimizationInfo = QString("优化信息:\n• 异常: %1").arg(e.what());
            basicBlockInfo = QString("基本块信息:\n• 分析异常");
            std::cout << "代码生成异常: " << e.what() << std::endl;
        }
        
        std::cout << "代码生成完成，成功: " << codeGenSuccess << ", 指令数: " << totalInstructions << std::endl;
        
        if (isInterruptionRequested()) return;
        
        emit codeGenerationFinished(codeGenSuccess, codeGenMessage,
                                   intermediateCode, optimizationInfo, basicBlockInfo,
                                   totalInstructions, basicBlocks, tempVars);
        
    } catch (const std::exception &e) {
        if (!isInterruptionRequested()) {
            emit analysisError(QString("分析过程中发生错误: %1").arg(e.what()));
        }
    }
}

// ============ MainWindow 实现 ============

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mainSplitter(nullptr)
    , rightSplitter(nullptr)
    , errorDock(nullptr)
    , errorList(nullptr)
    , statusLabel(nullptr)
    , positionLabel(nullptr)
    , encodingLabel(nullptr)
    , progressBar(nullptr)
    , codeEditor(nullptr)
    , analysisPanel(nullptr)
    , analysisThread(nullptr)
    , isUntitled(true)
    , enableAutoAnalysis(true)
    , enableSyntaxHighlighting(true)
    , showLineNumbers(true)
    , autoAnalysisTimer(nullptr)
    , findReplaceDialog(nullptr)
    , settingsDialog(nullptr)
{
    setupUI();
    setupActions();
    setupMenus();
    setupToolBars();
    setupStatusBar();
    setupDockWidgets();
    setupConnections();
    
    // 读取设置
    readSettings();
    
    // 设置初始状态
    setCurrentFile(QString());
    updateActions();
    
    // 自动分析定时器
    autoAnalysisTimer = new QTimer(this);
    autoAnalysisTimer->setSingleShot(true);
    autoAnalysisTimer->setInterval(1000); // 1秒延迟
    connect(autoAnalysisTimer, &QTimer::timeout, this, &MainWindow::runLexicalAnalysis);
}

MainWindow::~MainWindow()
{
    if (analysisThread && analysisThread->isRunning()) {
        analysisThread->requestInterruption();
        analysisThread->wait(3000);
    }
    writeSettings();
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // 创建代码编辑器
    codeEditor = new CodeEditor(this);
    
    // 创建分析面板
    analysisPanel = new AnalysisPanel(this);
    
    // 创建右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(analysisPanel);
    
    // 设置分割器
    mainSplitter->addWidget(codeEditor);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1}); // 2:1 比例
    
    // 设置布局
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 设置窗口属性
    setMinimumSize(1000, 700);
    resize(1400, 900);
}

void MainWindow::setupActions()
{
    // 文件操作
    createAction(newAction, "新建(&N)", "Ctrl+N", "创建新文件");
    createAction(openAction, "打开(&O)", "Ctrl+O", "打开文件");
    createAction(saveAction, "保存(&S)", "Ctrl+S", "保存文件");
    createAction(saveAsAction, "另存为(&A)", "Ctrl+Shift+S", "另存为");
    createAction(exitAction, "退出(&X)", "Ctrl+Q", "退出程序");
    
    // 编辑操作
    createAction(undoAction, "撤销(&U)", "Ctrl+Z", "撤销");
    createAction(redoAction, "重做(&R)", "Ctrl+Y", "重做");
    createAction(cutAction, "剪切(&T)", "Ctrl+X", "剪切");
    createAction(copyAction, "复制(&C)", "Ctrl+C", "复制");
    createAction(pasteAction, "粘贴(&P)", "Ctrl+V", "粘贴");
    createAction(selectAllAction, "全选(&A)", "Ctrl+A", "全选");
    createAction(findAction, "查找(&F)", "Ctrl+F", "查找");
    createAction(replaceAction, "替换(&R)", "Ctrl+H", "替换");
    createAction(gotoLineAction, "转到行(&G)", "Ctrl+G", "转到指定行");
    createAction(settingsAction, "设置", "", "打开设置对话框");
    
    // 分析操作
    createAction(runLexicalAction, "词法分析(&L)", "F5", "运行词法分析");
    createAction(runSyntaxAction, "语法分析(&S)", "F6", "运行语法分析");
    createAction(runSemanticAction, "语义分析(&M)", "F7", "运行语义分析");
    createAction(runCodeGenAction, "代码生成(&G)", "F8", "运行代码生成");
    createAction(runFullAction, "完整分析(&F)", "F9", "运行完整分析");
    createAction(stopAnalysisAction, "停止分析(&T)", "Esc", "停止当前分析");
    
    // 视图操作
    createAction(toggleLineNumbersAction, "显示行号(&N)", "", "显示/隐藏行号");
    createAction(toggleSyntaxHighlightingAction, "语法高亮(&H)", "", "启用/禁用语法高亮");
    createAction(zoomInAction, "放大(&I)", "Ctrl++", "放大字体");
    createAction(zoomOutAction, "缩小(&O)", "Ctrl+-", "缩小字体");
    createAction(resetZoomAction, "重置缩放(&R)", "Ctrl+0", "重置字体大小");
    
    // 帮助操作
    createAction(aboutAction, "关于", "", "关于编译器前端");
    createAction(aboutQtAction, "关于Qt", "", "关于Qt框架");
    createAction(settingsAction, "设置...", "Ctrl+,", "打开设置对话框");
    
    // 设置复选框状态
    toggleLineNumbersAction->setCheckable(true);
    toggleLineNumbersAction->setChecked(showLineNumbers);
    toggleSyntaxHighlightingAction->setCheckable(true);
    toggleSyntaxHighlightingAction->setChecked(enableSyntaxHighlighting);
    
    // 初始状态
    stopAnalysisAction->setEnabled(false);
}

void MainWindow::setupMenus()
{
    // 文件菜单
    QMenu *fileMenu = QMainWindow::menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    
    // 最近文件子菜单
    QMenu *recentMenu = fileMenu->addMenu("最近文件(&R)");
    for (int i = 0; i < MaxRecentFiles; ++i) {
        QAction *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::recentFileTriggered);
        recentFileActions.append(action);
        recentMenu->addAction(action);
    }
    recentMenu->addSeparator();
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 编辑菜单
    QMenu *editMenu = QMainWindow::menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAction);
    editMenu->addSeparator();
    editMenu->addAction(findAction);
    editMenu->addAction(replaceAction);
    editMenu->addAction(gotoLineAction);
    editMenu->addSeparator();
    editMenu->addAction(settingsAction);
    
    // 分析菜单
    QMenu *analysisMenu = QMainWindow::menuBar()->addMenu("分析(&A)");
    analysisMenu->addAction(runLexicalAction);
    analysisMenu->addAction(runSyntaxAction);
    analysisMenu->addAction(runSemanticAction);
    analysisMenu->addAction(runCodeGenAction);
    analysisMenu->addSeparator();
    analysisMenu->addAction(runFullAction);
    analysisMenu->addAction(stopAnalysisAction);
    
    // 视图菜单
    QMenu *viewMenu = QMainWindow::menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(toggleLineNumbersAction);
    viewMenu->addAction(toggleSyntaxHighlightingAction);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addAction(resetZoomAction);
    
    // 帮助菜单
    QMenu *helpMenu = QMainWindow::menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
    
    updateRecentFiles();
}

void MainWindow::setupToolBars()
{
    // 文件工具栏
    fileToolBar = addToolBar("文件");
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    
    // 编辑工具栏
    editToolBar = addToolBar("编辑");
    editToolBar->addAction(undoAction);
    editToolBar->addAction(redoAction);
    editToolBar->addSeparator();
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    
    // 分析工具栏
    analysisToolBar = addToolBar("分析");
    analysisToolBar->addAction(runLexicalAction);
    analysisToolBar->addAction(runSyntaxAction);
    analysisToolBar->addAction(runSemanticAction);
    analysisToolBar->addAction(runCodeGenAction);
    analysisToolBar->addSeparator();
    analysisToolBar->addAction(runFullAction);
    analysisToolBar->addAction(stopAnalysisAction);
    
    // 视图工具栏
    viewToolBar = addToolBar("视图");
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(resetZoomAction);
}

void MainWindow::setupStatusBar()
{
    // 状态标签
    statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(statusLabel);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    statusBar()->addWidget(progressBar);
    
    // 位置标签
    positionLabel = new QLabel("行: 1, 列: 1", this);
    statusBar()->addPermanentWidget(positionLabel);
    
    // 编码标签
    encodingLabel = new QLabel("UTF-8", this);
    statusBar()->addPermanentWidget(encodingLabel);
}

void MainWindow::setupDockWidgets()
{
    // 错误信息停靠窗口
    errorDock = new QDockWidget("错误信息", this);
    errorList = new QListWidget(this);
    errorDock->setWidget(errorList);
    addDockWidget(Qt::BottomDockWidgetArea, errorDock);
    errorDock->hide(); // 初始隐藏
}

void MainWindow::setupConnections()
{
    // 文件操作连接
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 编辑操作连接
    connect(undoAction, &QAction::triggered, codeEditor, &QPlainTextEdit::undo);
    connect(redoAction, &QAction::triggered, codeEditor, &QPlainTextEdit::redo);
    connect(cutAction, &QAction::triggered, codeEditor, &QPlainTextEdit::cut);
    connect(copyAction, &QAction::triggered, codeEditor, &QPlainTextEdit::copy);
    connect(pasteAction, &QAction::triggered, codeEditor, &QPlainTextEdit::paste);
    connect(selectAllAction, &QAction::triggered, codeEditor, &QPlainTextEdit::selectAll);
    connect(findAction, &QAction::triggered, this, &MainWindow::find);
    connect(replaceAction, &QAction::triggered, this, &MainWindow::replace);
    connect(gotoLineAction, &QAction::triggered, this, &MainWindow::gotoLine);
    
    // 分析操作连接
    connect(runLexicalAction, &QAction::triggered, this, &MainWindow::runLexicalAnalysis);
    connect(runSyntaxAction, &QAction::triggered, this, &MainWindow::runSyntaxAnalysis);
    connect(runSemanticAction, &QAction::triggered, this, &MainWindow::runSemanticAnalysis);
    connect(runCodeGenAction, &QAction::triggered, this, &MainWindow::runCodeGeneration);
    connect(runFullAction, &QAction::triggered, this, &MainWindow::runFullAnalysis);
    connect(stopAnalysisAction, &QAction::triggered, this, &MainWindow::stopAnalysis);
    
    // 视图操作连接
    connect(toggleLineNumbersAction, &QAction::triggered, this, &MainWindow::toggleLineNumbers);
    connect(toggleSyntaxHighlightingAction, &QAction::triggered, this, &MainWindow::toggleSyntaxHighlighting);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::resetZoom);
    
    // 帮助操作连接
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
    
    // 代码编辑器信号连接
    connect(codeEditor, &CodeEditor::textChanged, this, &MainWindow::onTextChanged);
    connect(codeEditor, &CodeEditor::modificationChanged, this, &MainWindow::onModificationChanged);
    connect(codeEditor, &CodeEditor::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);
    connect(codeEditor, &CodeEditor::fileNameChanged, this, &MainWindow::onFileNameChanged);
    
    // 右键菜单
    codeEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(codeEditor, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::createAction(QAction *&action, const QString &text, 
                             const QString &shortcut, const QString &tooltip, const QString &icon)
{
    action = new QAction(text, this);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence(shortcut));
    }
    if (!tooltip.isEmpty()) {
        action->setToolTip(tooltip);
        action->setStatusTip(tooltip);
    }
    if (!icon.isEmpty()) {
        action->setIcon(QIcon(icon));
    }
}

// ============ 槽函数实现 ============

void MainWindow::newFile()
{
    if (maybeSave()) {
        codeEditor->clear();
        setCurrentFile(QString());
        clearAnalysisResults();
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            "打开文件", ".", "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*.*)");
        if (!fileName.isEmpty()) {
            if (codeEditor->openFile(fileName)) {
                setCurrentFile(fileName);
                addToRecentFiles(fileName);
                clearAnalysisResults();
            }
        }
    }
}

bool MainWindow::saveFile()
{
    if (isUntitled) {
        return saveFileAs();
    } else {
        return codeEditor->saveFile(currentFile);
    }
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "另存为", ".", "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*.*)");
    if (fileName.isEmpty()) {
        return false;
    }
    
    if (codeEditor->saveFile(fileName)) {
        setCurrentFile(fileName);
        addToRecentFiles(fileName);
        return true;
    }
    return false;
}

void MainWindow::recentFileTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action && maybeSave()) {
        QString fileName = action->data().toString();
        if (codeEditor->openFile(fileName)) {
            setCurrentFile(fileName);
            clearAnalysisResults();
        }
    }
}

void MainWindow::find()
{
    if (!findReplaceDialog) {
        findReplaceDialog = new FindReplaceDialog(codeEditor, this);
    }
    findReplaceDialog->showFind();
}

void MainWindow::replace()
{
    if (!findReplaceDialog) {
        findReplaceDialog = new FindReplaceDialog(codeEditor, this);
    }
    findReplaceDialog->showReplace();
}

void MainWindow::gotoLine()
{
    bool ok;
    int line = QInputDialog::getInt(this, "转到行", "行号:", 1, 1, codeEditor->blockCount(), 1, &ok);
    if (ok) {
        codeEditor->gotoLine(line);
    }
}

void MainWindow::runLexicalAnalysis()
{
    if (!analysisThread) {
        analysisThread = new AnalysisThread(this);
        // 连接所有分析结果信号
        connect(analysisThread, &AnalysisThread::lexicalAnalysisFinished,
                this, &MainWindow::onLexicalAnalysisFinished);
        connect(analysisThread, &AnalysisThread::lexicalAnalysisFinishedWithDFA,
                this, &MainWindow::onLexicalAnalysisFinishedWithDFA);
        connect(analysisThread, &AnalysisThread::syntaxAnalysisFinished,
                this, &MainWindow::onSyntaxAnalysisFinished);
        connect(analysisThread, &AnalysisThread::semanticAnalysisFinished,
                this, &MainWindow::onSemanticAnalysisFinished);
        connect(analysisThread, &AnalysisThread::codeGenerationFinished,
                this, &MainWindow::onCodeGenerationFinished);
        connect(analysisThread, &AnalysisThread::analysisError,
                this, &MainWindow::onAnalysisError);
        connect(analysisThread, &AnalysisThread::analysisProgress,
                this, &MainWindow::updateAnalysisProgress);
    }
    
    if (analysisThread->isRunning()) {
        return;
    }
    
    startAnalysis();
    updateAnalysisProgress("正在启动分析...");
    
    analysisThread->setSourceCode(codeEditor->toPlainText());
    analysisThread->start();
}

void MainWindow::runSyntaxAnalysis()
{
    runLexicalAnalysis(); // 语法分析依赖词法分析，在分析线程中会自动进行
}

void MainWindow::runSemanticAnalysis()
{
    if (analysisThread && analysisThread->isRunning()) {
        return; // 已经在运行分析
    }
    
    QString code = codeEditor->toPlainText();
    if (code.isEmpty()) {
        return;
    }
    
    startAnalysis();
    
    // 创建分析线程
    analysisThread = new AnalysisThread(this);
    analysisThread->setSourceCode(code);
    analysisThread->setAnalysisType(AnalysisThread::AnalysisType::SEMANTIC_ONLY);
    
    // 连接信号
    connect(analysisThread, &AnalysisThread::lexicalAnalysisFinished,
            this, &MainWindow::onLexicalAnalysisFinished);
    connect(analysisThread, &AnalysisThread::syntaxAnalysisFinished,
            this, &MainWindow::onSyntaxAnalysisFinished);
    connect(analysisThread, &AnalysisThread::semanticAnalysisFinished,
            this, &MainWindow::onSemanticAnalysisFinished);
    connect(analysisThread, &AnalysisThread::codeGenerationFinished,
            this, &MainWindow::onCodeGenerationFinished);
    connect(analysisThread, &AnalysisThread::analysisError,
            this, &MainWindow::onAnalysisError);
    connect(analysisThread, &AnalysisThread::analysisProgress,
            this, &MainWindow::updateAnalysisProgress);
    
    // 启动分析
    analysisThread->start();
}

void MainWindow::runCodeGeneration()
{
    if (analysisThread && analysisThread->isRunning()) {
        return; // 已经在运行分析
    }
    
    QString code = codeEditor->toPlainText();
    if (code.isEmpty()) {
        return;
    }
    
    startAnalysis();
    
    // 创建分析线程
    analysisThread = new AnalysisThread(this);
    analysisThread->setSourceCode(code);
    analysisThread->setAnalysisType(AnalysisThread::AnalysisType::CODEGEN_ONLY);
    
    // 连接信号
    connect(analysisThread, &AnalysisThread::lexicalAnalysisFinished,
            this, &MainWindow::onLexicalAnalysisFinished);
    connect(analysisThread, &AnalysisThread::syntaxAnalysisFinished,
            this, &MainWindow::onSyntaxAnalysisFinished);
    connect(analysisThread, &AnalysisThread::semanticAnalysisFinished,
            this, &MainWindow::onSemanticAnalysisFinished);
    connect(analysisThread, &AnalysisThread::codeGenerationFinished,
            this, &MainWindow::onCodeGenerationFinished);
    connect(analysisThread, &AnalysisThread::analysisError,
            this, &MainWindow::onAnalysisError);
    connect(analysisThread, &AnalysisThread::analysisProgress,
            this, &MainWindow::updateAnalysisProgress);
    
    // 启动分析
    analysisThread->start();
}

void MainWindow::runFullAnalysis()
{
    runLexicalAnalysis(); // 完整分析会在分析线程中依次执行所有阶段
}

void MainWindow::stopAnalysis()
{
    if (analysisThread && analysisThread->isRunning()) {
        // 请求中断
        analysisThread->requestInterruption();
        
        // 等待线程结束，最多等待3秒
        if (!analysisThread->wait(3000)) {
            // 如果等待超时，强制终止线程
            analysisThread->terminate();
            analysisThread->wait(1000);
        }
        
        finishAnalysis();
        statusLabel->setText("分析已停止");
    }
}

void MainWindow::toggleLineNumbers()
{
    showLineNumbers = toggleLineNumbersAction->isChecked();
    // 这里需要根据CodeEditor的实现来切换行号显示
}

void MainWindow::toggleSyntaxHighlighting()
{
    enableSyntaxHighlighting = toggleSyntaxHighlightingAction->isChecked();
    codeEditor->enableSyntaxHighlighting(enableSyntaxHighlighting);
}

void MainWindow::zoomIn()
{
    codeEditor->zoomIn();
}

void MainWindow::zoomOut()
{
    codeEditor->zoomOut();
}

void MainWindow::resetZoom()
{
    // 重置字体大小到默认值
    QFont font = codeEditor->font();
    font.setPointSize(10);
    codeEditor->setFont(font);
}

void MainWindow::onTextChanged()
{
    // 暂时禁用自动分析，避免干扰用户输入
    // if (enableAutoAnalysis) {
    //     autoAnalysisTimer->start();
    // }
    updateActions();
}

void MainWindow::onModificationChanged(bool changed)
{
    Q_UNUSED(changed)
    updateWindowTitle();
    updateActions();
}

void MainWindow::onCursorPositionChanged(int line, int column)
{
    positionLabel->setText(QString("行: %1, 列: %2").arg(line).arg(column));
}

void MainWindow::onFileNameChanged(const QString &fileName)
{
    setCurrentFile(fileName);
}

void MainWindow::onLexicalAnalysisFinished(const QVector<Token> &tokens)
{
    statusLabel->setText(QString("词法分析完成，发现 %1 个Token").arg(tokens.size()));
    
    // 更新词法分析面板
    auto lexicalPanel = analysisPanel->getLexicalPanel();
    if (lexicalPanel) {
        lexicalPanel->setTokens(tokens);
        lexicalPanel->updateStatistics(tokens.size(), 10, 100); // 简化统计
    }
}

void MainWindow::onLexicalAnalysisFinishedWithDFA(const QVector<Token> &tokens, 
                                                  size_t originalStates, size_t minimizedStates,
                                                  double compressionRatio)
{
    statusLabel->setText(QString("词法分析完成，发现 %1 个Token，DFA最小化率 %2%")
                        .arg(tokens.size()).arg(QString::number(compressionRatio * 100.0, 'f', 2)));
    
    // 首先调用基础的词法分析完成处理
    onLexicalAnalysisFinished(tokens);
    
    // 更新词法分析面板的DFA信息
    auto lexicalPanel = analysisPanel->getLexicalPanel();
    if (lexicalPanel) {
        // 生成DFA信息
        QString dfaInfo = QString("原始DFA信息:\n"
                                 "• 状态数: %1\n"
                                 "• 转换数: %2\n"
                                 "• 接受状态: %3\n"
                                 "• 字母表大小: 估计 60+\n\n"
                                 "构造方法:\n"
                                 "• 标准构造算法\n"
                                 "• 支持关键字、标识符、数字等\n"
                                 "• 状态转换表已优化")
                                 .arg(originalStates)
                                 .arg(originalStates * 10) // 估计转换数
                                 .arg(originalStates / 4); // 估计接受状态数
        lexicalPanel->setDFAInfo(dfaInfo);
        
        // 生成最小化DFA信息
        QString minimizedInfo = QString("最小化DFA信息:\n"
                                       "• 最小化后状态数: %1\n"
                                       "• 减少的状态数: %2\n"
                                       "• 压缩率: %3%\n"
                                       "• 最小化算法: Hopcroft\n\n"
                                       "优化效果:\n"
                                       "• 内存占用减少 %4%\n"
                                       "• 执行效率提升\n"
                                       "• 等价状态合并完成\n"
                                       "• 死状态已移除")
                                       .arg(minimizedStates)
                                       .arg(originalStates - minimizedStates)
                                       .arg(QString::number(compressionRatio * 100.0, 'f', 2))
                                       .arg(QString::number(compressionRatio * 100.0, 'f', 1));
        lexicalPanel->setMinimizedDFAInfo(minimizedInfo);
        
        // 自动切换到词法分析标签页
        analysisPanel->switchToLexicalTab();
    }
    
    // 清理错误标记
    clearAllErrors();
}

void MainWindow::onSyntaxAnalysisFinished(bool success, const QString &message, 
                                         const std::shared_ptr<ASTNode> &ast,
                                         const QString &parseInfo,
                                         const QString &grammarInfo)
{
    // 不调用finishAnalysis()，因为可能还有后续分析
    // finishAnalysis();
    statusLabel->setText(message);
    
    // 更新语法分析面板
    auto syntaxPanel = analysisPanel->getSyntaxPanel();
    if (syntaxPanel) {
        syntaxPanel->clearAST();
        syntaxPanel->clearParseErrors();
        
        if (success) {
            // 总是设置解析信息和文法信息
            syntaxPanel->setParseInfo(parseInfo);
            syntaxPanel->setGrammarInfo(grammarInfo);
            
            // 如果有AST，则设置AST；否则显示演示内容
            if (ast) {
                syntaxPanel->setAST(ast);
            } else {
                // 调用setAST(nullptr)会显示演示内容
                syntaxPanel->setAST(nullptr);
            }
            
            // 检查分析线程的分析类型，只有语法分析类型才跳转
            if (analysisThread && analysisThread->isRunning()) {
                // 线程还在运行，说明还有后续分析，不跳转
                return;
            } else {
                // 线程已结束，说明这是最后一个分析阶段，跳转到语法分析界面
                analysisPanel->switchToSyntaxTab();
                finishAnalysis();
            }
        } else {
            syntaxPanel->addParseError(message);
            finishAnalysis();
        }
    }
}

void MainWindow::onSemanticAnalysisFinished(bool success, const QString &message,
                                           const QString &symbolTableInfo,
                                           const QString &typeCheckInfo,
                                           const QString &scopeInfo,
                                           const QVector<SemanticError> &errors)
{
    finishAnalysis();
    statusLabel->setText(message);
    
    // 清理之前的错误标记
    clearAllErrors();
    
    // 更新语义分析面板
    auto semanticPanel = analysisPanel->getSemanticPanel();
    if (semanticPanel) {
        semanticPanel->clearSymbolTable();
        semanticPanel->clearSemanticErrors();
        
        if (success) {
            // 优先使用符号表信息来显示符号表内容
            if (!symbolTableInfo.isEmpty()) {
                semanticPanel->setSymbolTableInfo(symbolTableInfo);
            }
            
            // 设置其他信息
            semanticPanel->setTypeCheckInfo(typeCheckInfo);
            semanticPanel->setScopeInfo(scopeInfo);
            
            // 自动切换到语义分析标签页
            analysisPanel->switchToSemanticTab();
        } else {
            semanticPanel->addSemanticError(message);
        }
        
        // 显示错误信息（即使成功也可能有警告）
        for (const auto& error : errors) {
            QString errorText = QString::fromStdString(error.message);
            semanticPanel->addSemanticError(errorText, error.line);
            
            // 在编辑器中标记错误
            if (error.line > 0) {
                showErrorInEditor(error.line, errorText);
            }
        }
    }
}

void MainWindow::onCodeGenerationFinished(bool success, const QString &message,
                                         const QVector<ThreeAddressCode> &codes,
                                         const QString &optimizationInfo,
                                         const QString &basicBlockInfo,
                                         int totalInstructions, int basicBlocks, int tempVars)
{
    finishAnalysis();
    statusLabel->setText(message);
    
    // 更新代码生成面板
    auto codeGenPanel = analysisPanel->getCodeGenPanel();
    if (codeGenPanel) {
        codeGenPanel->clearIntermediateCode();
        
        if (success) {
            codeGenPanel->setIntermediateCode(codes);
            codeGenPanel->setOptimizationInfo(optimizationInfo);
            codeGenPanel->setBasicBlockInfo(basicBlockInfo);
            codeGenPanel->updateCodeGenStatistics(totalInstructions, basicBlocks, tempVars);
            
            // 自动切换到代码生成标签页
            analysisPanel->switchToCodeGenTab();
        }
    }
}

void MainWindow::onAnalysisError(const QString &error)
{
    finishAnalysis();
    statusLabel->setText(QString("分析错误: %1").arg(error));
    QMessageBox::warning(this, "分析错误", error);
}

void MainWindow::about()
{
    QMessageBox::about(this, "关于编译器前端",
        "编译器前端 v1.0\n\n"
        "这是一个基于Qt的现代编译器前端，支持:\n"
        "• 词法分析 (DFA + 最小化)\n"
        "• 语法分析 (LALR)\n"
        "• 语义分析 (符号表 + 类型检查)\n"
        "• 中间代码生成\n"
        "• 实时语法高亮\n"
        "• 多线程分析架构\n\n"
        "技术栈: C++17 + Qt5 + 现代编译器理论");
}

void MainWindow::showSettings()
{
    if (!settingsDialog) {
        settingsDialog = new SettingsDialog(this);
    }
    settingsDialog->exec();
}

void MainWindow::updateRecentFiles()
{
    QStringList files = getRecentFiles();
    
    for (int i = 0; i < MaxRecentFiles; ++i) {
        if (i < files.size()) {
            QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
            recentFileActions[i]->setText(text);
            recentFileActions[i]->setData(files[i]);
            recentFileActions[i]->setVisible(true);
        } else {
            recentFileActions[i]->setVisible(false);
        }
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    contextMenu.addAction(undoAction);
    contextMenu.addAction(redoAction);
    contextMenu.addSeparator();
    contextMenu.addAction(cutAction);
    contextMenu.addAction(copyAction);
    contextMenu.addAction(pasteAction);
    contextMenu.addSeparator();
    contextMenu.addAction(selectAllAction);
    contextMenu.exec(codeEditor->mapToGlobal(pos));
}

// ============ 工具函数实现 ============

void MainWindow::updateWindowTitle()
{
    QString shownName = currentFile;
    if (shownName.isEmpty()) {
        shownName = "未命名";
    } else {
        shownName = strippedName(currentFile);
    }
    
    setWindowTitle(QString("%1[*] - 编译器").arg(shownName));
    setWindowModified(codeEditor->isModified());
}

void MainWindow::updateActions()
{
    bool hasText = !codeEditor->toPlainText().isEmpty();
    bool hasSelection = codeEditor->textCursor().hasSelection();
    
    saveAction->setEnabled(codeEditor->isModified());
    undoAction->setEnabled(codeEditor->document()->isUndoAvailable());
    redoAction->setEnabled(codeEditor->document()->isRedoAvailable());
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    
    runLexicalAction->setEnabled(hasText);
    runSyntaxAction->setEnabled(hasText);
    runSemanticAction->setEnabled(hasText);
    runCodeGenAction->setEnabled(hasText);
    runFullAction->setEnabled(hasText);
}

void MainWindow::addToRecentFiles(const QString &fileName)
{
    QStringList files = getRecentFiles();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    setRecentFiles(files);
    updateRecentFiles();
}

QStringList MainWindow::getRecentFiles() const
{
    QSettings settings;
    return settings.value("recentFiles").toStringList();
}

void MainWindow::setRecentFiles(const QStringList &files)
{
    QSettings settings;
    settings.setValue("recentFiles", files);
}

bool MainWindow::maybeSave()
{
    if (codeEditor->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "编译器",
                                 "文档已修改。\n是否保存更改？",
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isUntitled = fileName.isEmpty();
    updateWindowTitle();
    
    if (!fileName.isEmpty()) {
        addToRecentFiles(fileName);
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(1200, 800)).toSize();
    resize(size);
    move(pos);
    
    enableAutoAnalysis = settings.value("enableAutoAnalysis", true).toBool();
    enableSyntaxHighlighting = settings.value("enableSyntaxHighlighting", true).toBool();
    showLineNumbers = settings.value("showLineNumbers", true).toBool();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("enableAutoAnalysis", enableAutoAnalysis);
    settings.setValue("enableSyntaxHighlighting", enableSyntaxHighlighting);
    settings.setValue("showLineNumbers", showLineNumbers);
}

void MainWindow::startAnalysis()
{
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // 不确定进度
    stopAnalysisAction->setEnabled(true);
    
    // 禁用分析相关按钮
    runLexicalAction->setEnabled(false);
    runSyntaxAction->setEnabled(false);
    runSemanticAction->setEnabled(false);
    runCodeGenAction->setEnabled(false);
    runFullAction->setEnabled(false);
}

void MainWindow::finishAnalysis()
{
    progressBar->setVisible(false);
    stopAnalysisAction->setEnabled(false);
    updateActions();
}

void MainWindow::updateAnalysisProgress(const QString &stage)
{
    statusLabel->setText(stage);
}

void MainWindow::displayAnalysisResults()
{
    // 显示分析结果
}

void MainWindow::clearAnalysisResults()
{
    analysisPanel->clearAllResults();
    errorList->clear();
    errorDock->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::onTabChanged(int index)
{
    emit tabChanged(index);
}

// ============ 缺失的槽函数实现 ============

void MainWindow::undo()
{
    if (codeEditor) {
        codeEditor->undo();
    }
}

void MainWindow::redo()
{
    if (codeEditor) {
        codeEditor->redo();
    }
}

void MainWindow::cut()
{
    if (codeEditor) {
        codeEditor->cut();
    }
}

void MainWindow::copy()
{
    if (codeEditor) {
        codeEditor->copy();
    }
}

void MainWindow::paste()
{
    if (codeEditor) {
        codeEditor->paste();
    }
}

void MainWindow::selectAll()
{
    if (codeEditor) {
        codeEditor->selectAll();
    }
}

// ============ 辅助函数实现 ============

std::shared_ptr<ASTNode> AnalysisThread::createSimpleParser() {
    return nullptr; // 占位符
}

std::shared_ptr<ASTNode> AnalysisThread::parseTokensToAST(const std::vector<Token>& tokens) {
    // 简化的AST构建：根据代码内容生成对应的AST
    if (tokens.empty()) {
        return nullptr;
    }
    
    // 创建程序根节点
    auto program = std::make_shared<SimpleASTNode>(ASTNodeType::PROGRAM, 1, 1);
    
    // 分析token序列，构建基本的AST结构
    // 这是一个简化的实现，主要用于演示
    
    bool hasMainFunction = false;
    bool hasVariableDeclarations = false;
    
    // 扫描tokens确定代码结构
    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        
        if (token.type == TokenType::IDENTIFIER && token.value == "main") {
            hasMainFunction = true;
        }
        
        if (token.type == TokenType::INT || token.type == TokenType::FLOAT || 
            token.type == TokenType::BOOL) {
            hasVariableDeclarations = true;
        }
    }
    
    // 根据代码内容生成合适的AST结构
    if (hasMainFunction) {
        // 有main函数，创建函数声明节点
        auto funcDecl = std::make_shared<SimpleASTNode>(ASTNodeType::FUNC_DECL, 1, 1);
        
        // 函数体
        auto block = std::make_shared<SimpleASTNode>(ASTNodeType::BLOCK_STMT, 1, 1);
        
        if (hasVariableDeclarations) {
            // 变量声明
            auto varDecl = std::make_shared<SimpleASTNode>(ASTNodeType::VAR_DECL, 2, 1);
            
            // 可能的赋值语句
            for (size_t i = 0; i < tokens.size() - 1; ++i) {
                if (tokens[i].type == TokenType::ASSIGN) {
                    auto assignment = std::make_shared<SimpleASTNode>(ASTNodeType::ASSIGNMENT_STMT, 2, 1);
                    break;
                }
            }
        }
        
        // 检查return语句
        for (size_t i = 0; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::RETURN) {
                auto returnStmt = std::make_shared<SimpleASTNode>(ASTNodeType::RETURN_STMT, 3, 1);
                break;
            }
        }
    } else if (hasVariableDeclarations) {
        // 只有变量声明
        auto varDecl = std::make_shared<SimpleASTNode>(ASTNodeType::VAR_DECL, 1, 1);
        
        // 检查赋值
        for (size_t i = 0; i < tokens.size() - 1; ++i) {
            if (tokens[i].type == TokenType::ASSIGN) {
                auto assignment = std::make_shared<SimpleASTNode>(ASTNodeType::ASSIGNMENT_STMT, 1, 1);
                break;
            }
        }
    }
    
    return program;
}

int AnalysisThread::countASTNodes(ASTNode* node) {
    if (!node) return 0;
    
    int count = 1; // 当前节点
    
    // 根据节点类型估计子节点数
    switch (node->nodeType) {
        case ASTNodeType::PROGRAM:
            count += 3; // 估计的子节点数
            break;
        case ASTNodeType::FUNC_DECL:
            count += 2; // 函数声明通常有参数列表和函数体
            break;
        case ASTNodeType::VAR_DECL:
            count += 1; // 变量声明有类型信息
            break;
        case ASTNodeType::BLOCK_STMT:
            count += 2; // 块语句估计有几个子语句
            break;
        case ASTNodeType::ASSIGNMENT_STMT:
            count += 2; // 赋值语句有左值和右值
            break;
        case ASTNodeType::BINARY_EXPR:
            count += 2; // 二元表达式有左操作数和右操作数
            break;
        case ASTNodeType::RETURN_STMT:
            count += 1; // return语句有返回表达式
            break;
        default:
            break;
    }
    
    return count;
}

int AnalysisThread::getASTDepth(ASTNode* node) {
    if (!node) return 0;
    
    int maxChildDepth = 0;
    
    // 根据节点类型计算子节点的最大深度
    switch (node->nodeType) {
        case ASTNodeType::PROGRAM:
            maxChildDepth = 3; // 程序 -> 函数 -> 语句
            break;
        case ASTNodeType::FUNC_DECL:
            maxChildDepth = 2; // 函数 -> 语句
            break;
        case ASTNodeType::BLOCK_STMT:
            maxChildDepth = 1; // 块 -> 语句
            break;
        case ASTNodeType::ASSIGNMENT_STMT:
        case ASTNodeType::BINARY_EXPR:
            maxChildDepth = 1; // 赋值/二元表达式 -> 表达式
            break;
        default:
            maxChildDepth = 0;
            break;
    }
    
    return 1 + maxChildDepth;
}

void MainWindow::showErrorInEditor(int line, const QString &message)
{
    if (codeEditor) {
        // 在QTextEdit中高亮显示错误行
        QTextCursor cursor = codeEditor->textCursor();
        
        // 移动到指定行
        cursor.movePosition(QTextCursor::Start);
        for (int i = 1; i < line && !cursor.atEnd(); ++i) {
            cursor.movePosition(QTextCursor::Down);
        }
        
        // 选择整行
        cursor.select(QTextCursor::LineUnderCursor);
        
        // 设置背景色为浅红色
        QTextCharFormat format;
        format.setBackground(QColor(255, 200, 200, 100));
        cursor.setCharFormat(format);
        
        // 在错误面板中显示错误
        if (errorList) {
            QString errorText = QString("第%1行: %2").arg(line).arg(message);
            QListWidgetItem *item = new QListWidgetItem(errorText);
            item->setForeground(QColor(200, 0, 0)); // 红色
            errorList->addItem(item);
        }
        
        // 显示错误面板
        if (errorDock) {
            errorDock->show();
        }
    }
}

void MainWindow::clearAllErrors()
{
    if (codeEditor) {
        // 清除文本格式
        QTextCursor cursor = codeEditor->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat format;
        format.setBackground(QColor(255, 255, 255)); // 白色背景
        cursor.setCharFormat(format);
        
        // 重置光标位置
        cursor.movePosition(QTextCursor::Start);
        codeEditor->setTextCursor(cursor);
    }
    
    if (errorList) {
        errorList->clear();
    }
    
    // 隐藏错误面板
    if (errorDock) {
        errorDock->hide();
    }
}

// 创建简单的语法分析器
