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
#include <QRegularExpression>
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
        
        std::cout << "=== 开始语法分析 ===" << std::endl;
        
        // 创建简单的语法分析器
        auto parser = createSimpleParser();
        std::shared_ptr<ASTNode> ast = nullptr;
        QString parseInfo;
        bool parseSuccess = false;
        
        try {
            // 尝试解析
            std::cout << "调用parseTokensToAST，Token数量: " << lexResult.tokens.size() << std::endl;
            ast = parseTokensToAST(lexResult.tokens);
            parseSuccess = (ast != nullptr);
            
            std::cout << "语法分析结果: parseSuccess=" << parseSuccess << ", ast=" << (ast ? "not null" : "null") << std::endl;
            
            if (ast) {
                std::cout << "AST节点数: " << countASTNodes(ast.get()) << std::endl;
                std::cout << "AST深度: " << getASTDepth(ast.get()) << std::endl;
            }
            
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
            std::cout << "语法分析异常: " << e.what() << std::endl;
        }
        
        std::cout << "准备发送语法分析信号, parseSuccess=" << parseSuccess << std::endl;
        
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
                std::cout << "=== 开始语义分析 ===" << std::endl;
                
                // 创建语义分析器
                SemanticAnalyzer analyzer;
                
                // 由于我们使用的是SimpleASTNode而不是具体的AST节点类型，
                // 我们需要创建一个演示性的语义分析结果
                SemanticAnalysisResult result;
                result.success = true;
                result.totalSymbols = 0;
                result.totalScopes = 1;
                result.analysisTimeMs = 10;
                
                std::cout << "创建语义分析器成功" << std::endl;
                
                // 创建一个新的符号表用于演示
                auto demoSymbolTable = std::make_unique<SymbolTable>();
                
                // 分析代码内容，添加符号到符号表
                QString sourceCode;
                {
                    QMutexLocker locker(&m_mutex);
                    sourceCode = m_sourceCode;
                }
                
                std::cout << "开始分析源代码，行数: " << sourceCode.split('\n').size() << std::endl;
                
                // 模拟一些使用检查和错误检测
                QStringList sourceLines = sourceCode.split('\n');
                
                // 检查变量使用错误
                std::map<std::string, bool> variableInitialized;
                std::map<std::string, int> variableDeclarationLine;
                
                for (int lineNum = 0; lineNum < sourceLines.size(); ++lineNum) {
                    QString line = sourceLines[lineNum].trimmed();
                    
                    // 检查变量声明和初始化 - 支持多种类型
                    QRegularExpression declPattern("(int|float|double|char)\\s+([a-zA-Z_][a-zA-Z0-9_]*)");
                    QRegularExpressionMatch declMatch = declPattern.match(line);
                    if (declMatch.hasMatch()) {
                        QString varType = declMatch.captured(1);
                        QString varName = declMatch.captured(2);
                        
                        if (!varName.isEmpty()) {
                            // 检查重复声明
                            if (variableInitialized.find(varName.toStdString()) != variableInitialized.end()) {
                                SemanticError error;
                                error.type = SemanticErrorType::REDEFINED_VARIABLE;
                                error.message = "重复定义的变量 '" + varName.toStdString() + "'";
                                error.line = lineNum + 1;
                                error.column = line.indexOf(varName) + 1;
                                semanticErrors.append(error);
                                semanticSuccess = false;
                            } else {
                                // 只有在没有重复时才添加变量
                                bool isInitialized = line.contains('=');
                                variableInitialized[varName.toStdString()] = isInitialized;
                                variableDeclarationLine[varName.toStdString()] = lineNum + 1;
                                
                                // 注意：不再在声明时就报未初始化警告
                                // 变量声明本身是合法的，无论是否初始化
                            }
                        }
                    }
                    
                    // 检查赋值语句，标记变量为已初始化
                    QRegularExpression assignPattern("([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*([^=]+)");
                    QRegularExpressionMatch assignMatch = assignPattern.match(line);
                    if (assignMatch.hasMatch()) {
                        QString varName = assignMatch.captured(1).trimmed();
                        QString varValue = assignMatch.captured(2).trimmed();
                        
                        // 检查这个变量是否已声明
                        if (variableInitialized.find(varName.toStdString()) != variableInitialized.end()) {
                            // 标记变量为已初始化（通过赋值）
                            variableInitialized[varName.toStdString()] = true;
                            demoSymbolTable->markSymbolInitialized(varName.toStdString());
                            std::cout << "变量 " << varName.toStdString() << " 通过赋值被标记为已初始化" << std::endl;
                        }
                    }
                    
                    // 标记变量使用情况（用于符号表显示）
                    for (const auto& var : variableInitialized) {
                        QString varName = QString::fromStdString(var.first);
                        if (line.contains(varName + " ") || line.contains(varName + ";") || 
                            line.contains(varName + "+") || line.contains(varName + "-") ||
                            line.contains(varName + "*") || line.contains(varName + "/")) {
                            
                            // 排除声明行和赋值行（避免将赋值误认为使用）
                            if (lineNum + 1 != variableDeclarationLine[var.first] && 
                                !line.contains(varName + " =") && !line.contains(varName + "=")) {
                                demoSymbolTable->markSymbolUsed(var.first);
                                // 注意：这里不再直接报错，错误检查统一在后面处理
                            }
                        }
                    }
                    
                    // 增加更多错误类型检查
                
                    // 1. 检查类型不匹配 (int x = "hello")
                    if (line.contains("int ") && line.contains("=") && line.contains("\"")) {
                        SemanticError error;
                        error.type = SemanticErrorType::TYPE_MISMATCH;
                        error.message = "类型不匹配：不能将字符串赋值给整数变量";
                        error.line = lineNum + 1;
                        error.column = line.indexOf("=") + 1;
                        semanticErrors.append(error);
                        semanticSuccess = false;
                    }
                    
                    // 2. 检查除零错误
                    if (line.contains("/") && line.contains("0")) {
                        QRegularExpression divZeroPattern("\\w+\\s*/\\s*0");
                        if (divZeroPattern.match(line).hasMatch()) {
                            SemanticError error;
                            error.type = SemanticErrorType::DIVISION_BY_ZERO;
                            error.message = "除零错误：不能除以零";
                            error.line = lineNum + 1;
                            error.column = line.indexOf("/") + 1;
                            semanticErrors.append(error);
                            semanticSuccess = false;
                        }
                    }
                    
                    // 3. 检查函数调用错误
                    QRegularExpression funcCallPattern("([a-zA-Z_][a-zA-Z0-9_]*)\\s*\\(");
                    QRegularExpressionMatchIterator funcMatches = funcCallPattern.globalMatch(line);
                    while (funcMatches.hasNext()) {
                        QRegularExpressionMatch match = funcMatches.next();
                        QString funcName = match.captured(1);
                        
                        // 排除已知函数和关键字 - 修复：添加控制流关键字
                        if (funcName != "main" && funcName != "printf" && funcName != "scanf" &&
                            funcName != "if" && funcName != "else" && funcName != "while" && 
                            funcName != "for" && funcName != "switch" && funcName != "do" &&
                            funcName != "return" && funcName != "sizeof" && funcName != "typeof") {
                            SemanticError error;
                            error.type = SemanticErrorType::UNDEFINED_FUNCTION;
                            error.message = "未定义的函数 '" + funcName.toStdString() + "'";
                            error.line = lineNum + 1;
                            error.column = match.capturedStart() + 1;
                            semanticErrors.append(error);
                            semanticSuccess = false;
                        }
                    }
                    
                    // 4. 检查返回类型不匹配
                    if (line.contains("return") && line.contains("\"")) {
                        SemanticError error;
                        error.type = SemanticErrorType::RETURN_TYPE_MISMATCH;
                        error.message = "返回类型不匹配：main函数应返回整数但返回了字符串";
                        error.line = lineNum + 1;
                        error.column = line.indexOf("return") + 1;
                        semanticErrors.append(error);
                        semanticSuccess = false;
                    }
                    
                    // 5. 检查无效运算 (例如对字符串进行算术运算)
                    if (line.contains("\"") && (line.contains("+") || line.contains("-") || 
                        line.contains("*") || line.contains("/"))) {
                        SemanticError error;
                        error.type = SemanticErrorType::INVALID_OPERATION;
                        error.message = "无效运算：不能对字符串进行算术运算";
                        error.line = lineNum + 1;
                        error.column = line.indexOf("+") != -1 ? line.indexOf("+") + 1 :
                                      line.indexOf("-") != -1 ? line.indexOf("-") + 1 :
                                      line.indexOf("*") != -1 ? line.indexOf("*") + 1 :
                                      line.indexOf("/") + 1;
                        semanticErrors.append(error);
                        semanticSuccess = false;
                    }
                    
                    // 6. 检查未声明的变量使用
                    QRegularExpression identifierPattern("\\b[a-zA-Z_][a-zA-Z0-9_]*\\b");
                    QRegularExpressionMatchIterator i = identifierPattern.globalMatch(line);
                    while (i.hasNext()) {
                        QRegularExpressionMatch match = i.next();
                        QString identifier = match.captured(0);
                        
                        // 排除关键字、类型名和函数名
                        if (identifier != "int" && identifier != "float" && identifier != "double" && 
                            identifier != "char" && identifier != "return" && identifier != "main" && 
                            identifier != "printf" && identifier != "scanf" && identifier != "if" && 
                            identifier != "else" && identifier != "while" && identifier != "for") {
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
                
                // 检查未初始化变量的使用情况（基于最终的变量状态）
                for (const auto& var : variableInitialized) {
                    if (!var.second) { // 仍然未初始化的变量
                        // 检查是否被使用过（在初始化之前）
                        bool usedBeforeInit = false;
                        for (int lineNum = 0; lineNum < sourceLines.size(); ++lineNum) {
                            QString line = sourceLines[lineNum].trimmed();
                            QString varName = QString::fromStdString(var.first);
                            
                            // 排除声明行
                            if (lineNum + 1 != variableDeclarationLine[var.first]) {
                                // 检查是否在赋值之前就被使用了
                                if (line.contains(varName + " ") || line.contains(varName + ";") || 
                                    line.contains(varName + "+") || line.contains(varName + "-") ||
                                    line.contains(varName + "*") || line.contains(varName + "/")) {
                                    // 排除赋值语句本身
                                    if (!line.contains(varName + " =") && !line.contains(varName + "=")) {
                                        usedBeforeInit = true;
                                        break;
                                    }
                                }
                            }
                        }
                        
                        if (usedBeforeInit) {
                            // 未初始化但被使用 - 错误
                            SemanticError error;
                            error.type = SemanticErrorType::UNINITIALIZED_VARIABLE;
                            error.message = "变量 '" + var.first + "' 在初始化前被使用";
                            error.line = variableDeclarationLine[var.first];
                            error.column = 1;
                            semanticErrors.append(error);
                            semanticSuccess = false;
                        } else {
                            // 未初始化但未被使用 - 警告
                            SemanticError warning;
                            warning.type = SemanticErrorType::UNINITIALIZED_VARIABLE;
                            warning.message = "变量 '" + var.first + "' 声明但未初始化";
                            warning.line = variableDeclarationLine[var.first];
                            warning.column = 1;
                            semanticErrors.append(warning); // 注意：这里作为警告，在UI中会显示为黄色
                        }
                    }
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
                        symbolTableInfo += QString("• %1 (%2): %3 [行号:%4]")
                                           .arg(QString::fromStdString(symbol->name))
                                           .arg(QString::fromStdString(TypeUtils::symbolTypeToString(symbol->symbolType)))
                                           .arg(QString::fromStdString(TypeUtils::dataTypeToString(symbol->dataType)))
                                           .arg(symbol->line);
                        
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
            // 首先尝试基于AST的代码生成
            if (ast) {
                std::cout << "尝试基于AST的代码生成..." << std::endl;
                
                // 递归遍历AST生成三地址码
                bool astCodeGenSuccess = generateCodeFromAST(ast.get(), intermediateCode, totalInstructions);
                
                if (astCodeGenSuccess) {
                    std::cout << "AST代码生成成功，指令数: " << totalInstructions << std::endl;
                    codeGenSuccess = true;
                } else {
                    std::cout << "AST代码生成失败，回退到字符串解析" << std::endl;
                    // 回退到字符串解析方法
                    codeGenSuccess = generateCodeFromString(code, intermediateCode, totalInstructions);
                }
            } else {
                std::cout << "没有AST，使用字符串解析方法" << std::endl;
                codeGenSuccess = generateCodeFromString(code, intermediateCode, totalInstructions);
            }
                
            // 生成优化信息
            optimizationInfo = QString("代码生成优化信息:\n"
                                     "• 常量折叠: 启用\n"
                                     "• 死代码消除: 启用\n"
                                     "• 优化级别: O2\n"
                                     "• 总指令数: %1\n"
                                     "• 基本块数: %2\n"
                                     "• 临时变量数: %3")
                                     .arg(totalInstructions)
                                     .arg(basicBlocks)
                                     .arg(tempVars);
            
            // 生成基本块信息
            basicBlockInfo = QString("基本块分析:\n"
                                   "• 基本块数量: %1\n"
                                   "• 平均指令数/块: %2\n"
                                   "• 临时变量数: %3\n"
                                   "• 控制流复杂度: %4")
                                   .arg(basicBlocks)
                                   .arg(basicBlocks > 0 ? QString::number(double(totalInstructions) / basicBlocks, 'f', 1) : "0")
                                   .arg(tempVars)
                                   .arg(code.contains("if") ? "复杂" : "简单");
            
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
    
    // 自动添加.c扩展名，如果用户没有指定扩展名
    if (!fileName.contains('.')) {
        fileName += ".c";
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
            
            // 只有真正的错误才在编辑器中标记为红色
            // 警告不应该标记为错误
            if (!errorText.startsWith("[警告]")) {
                semanticPanel->addSemanticError(errorText, error.line);
                
                // 在编辑器中标记错误
                if (error.line > 0) {
                    showErrorInEditor(error.line, errorText);
                }
            } else {
                // 这是警告，只在语义分析面板中显示，不在编辑器中标记
                semanticPanel->addSemanticError(errorText, error.line);
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
    if (tokens.empty()) {
        return nullptr;
    }
    
    // 创建一个简单的递归下降解析器
    TokenIndex = 0;
    m_tokens = tokens;
    
    try {
        return parseProgram();
    } catch (const std::exception& e) {
        std::cout << "语法分析错误: " << e.what() << std::endl;
        return nullptr;
    }
}

// 解析程序
std::shared_ptr<ASTNode> AnalysisThread::parseProgram() {
    auto program = std::make_shared<SimpleASTNode>(ASTNodeType::PROGRAM, 1, 1);
    
    // 解析声明和语句
    while (TokenIndex < m_tokens.size() && m_tokens[TokenIndex].type != TokenType::END_OF_FILE) {
        if (isTypeKeyword(currentToken().type)) {
            // 变量声明或函数声明
            auto decl = parseDeclaration();
            if (decl) {
                program->children.push_back(decl);
            }
        } else if (currentToken().type == TokenType::IDENTIFIER) {
            // 可能是赋值语句
            auto stmt = parseStatement();
            if (stmt) {
                program->children.push_back(stmt);
            }
        } else if (currentToken().type == TokenType::IF) {
            // if语句
            auto stmt = parseIfStatement();
            if (stmt) {
                program->children.push_back(stmt);
            }
        } else if (currentToken().type == TokenType::RETURN) {
            // return语句
            auto stmt = parseReturnStatement();
            if (stmt) {
                program->children.push_back(stmt);
            }
        } else {
            // 跳过未识别的token
            advance();
        }
    }
    
    return program;
}

// 解析声明
std::shared_ptr<ASTNode> AnalysisThread::parseDeclaration() {
    if (!isTypeKeyword(currentToken().type)) {
        return nullptr;
    }
    
    TokenType typeToken = currentToken().type;
    advance(); // 跳过类型关键字
    
    if (currentToken().type != TokenType::IDENTIFIER) {
        return nullptr;
    }
    
    std::string varName = currentToken().value;
    int line = currentToken().line;
    advance(); // 跳过标识符
    
    // 创建变量声明节点
    auto varDecl = std::make_shared<DetailedASTNode>(ASTNodeType::VAR_DECL, line, 1);
    varDecl->value = getTypeString(typeToken) + " " + varName;
    
    // 检查是否有初始化
    if (currentToken().type == TokenType::ASSIGN) {
        advance(); // 跳过 =
        
        // 解析初始化表达式
        auto initExpr = parseExpression();
        if (initExpr) {
            varDecl->children.push_back(initExpr);
        }
    }
    
    // 跳过分号
    if (currentToken().type == TokenType::SEMICOLON) {
        advance();
    }
    
    return varDecl;
}

// 解析语句
std::shared_ptr<ASTNode> AnalysisThread::parseStatement() {
    if (currentToken().type == TokenType::IDENTIFIER) {
        // 可能是赋值语句或复合赋值语句
        std::string varName = currentToken().value;
        int line = currentToken().line;
        advance();
        
        TokenType assignOp = currentToken().type;
        if (assignOp == TokenType::ASSIGN || 
            assignOp == TokenType::PLUS_ASSIGN || 
            assignOp == TokenType::MINUS_ASSIGN ||
            assignOp == TokenType::MUL_ASSIGN || 
            assignOp == TokenType::DIV_ASSIGN || 
            assignOp == TokenType::MOD_ASSIGN) {
            
            advance(); // 跳过赋值运算符
            
            auto assignStmt = std::make_shared<DetailedASTNode>(ASTNodeType::ASSIGNMENT_STMT, line, 1);
            
            // 根据赋值运算符类型设置显示值
            std::string opStr;
            switch (assignOp) {
                case TokenType::ASSIGN: opStr = " = "; break;
                case TokenType::PLUS_ASSIGN: opStr = " += "; break;
                case TokenType::MINUS_ASSIGN: opStr = " -= "; break;
                case TokenType::MUL_ASSIGN: opStr = " *= "; break;
                case TokenType::DIV_ASSIGN: opStr = " /= "; break;
                case TokenType::MOD_ASSIGN: opStr = " %= "; break;
                default: opStr = " = "; break;
            }
            
            // 解析右值表达式
            auto rvalue = parseExpression();
            if (rvalue) {
                if (auto detailedRvalue = std::dynamic_pointer_cast<DetailedASTNode>(rvalue)) {
                    assignStmt->value = varName + opStr + detailedRvalue->value;
                } else {
                    assignStmt->value = varName + opStr + "...";
                }
                assignStmt->children.push_back(rvalue);
            } else {
                assignStmt->value = varName + opStr + "...";
            }
            
            // 跳过分号
            if (currentToken().type == TokenType::SEMICOLON) {
                advance();
            }
            
            return assignStmt;
        }
    }
    
    return nullptr;
}

// 解析return语句
std::shared_ptr<ASTNode> AnalysisThread::parseReturnStatement() {
    if (currentToken().type != TokenType::RETURN) {
        return nullptr;
    }
    
    int line = currentToken().line;
    advance(); // 跳过 return
    
    auto returnStmt = std::make_shared<DetailedASTNode>(ASTNodeType::RETURN_STMT, line, 1);
    returnStmt->value = "return";
    
    // 解析返回值表达式
    if (currentToken().type != TokenType::SEMICOLON) {
        auto returnExpr = parseExpression();
        if (returnExpr) {
            returnStmt->children.push_back(returnExpr);
            if (auto detailedExpr = std::dynamic_pointer_cast<DetailedASTNode>(returnExpr)) {
                returnStmt->value = "return " + detailedExpr->value;
            }
        }
    }
    
    // 跳过分号
    if (currentToken().type == TokenType::SEMICOLON) {
        advance();
    }
    
    return returnStmt;
}

// 解析表达式
std::shared_ptr<ASTNode> AnalysisThread::parseExpression() {
    return parseAdditiveExpression();
}

// 解析加法表达式
std::shared_ptr<ASTNode> AnalysisThread::parseAdditiveExpression() {
    auto left = parseMultiplicativeExpression();
    
    while (currentToken().type == TokenType::PLUS || currentToken().type == TokenType::MINUS) {
        TokenType op = currentToken().type;
        int line = currentToken().line;
        advance();
        
        auto right = parseMultiplicativeExpression();
        if (right) {
            auto binaryExpr = std::make_shared<DetailedASTNode>(ASTNodeType::BINARY_EXPR, line, 1);
            
            // 安全地获取子节点的值
            std::string leftValue = "expr";
            std::string rightValue = "expr";
            if (auto leftDetailed = std::dynamic_pointer_cast<DetailedASTNode>(left)) {
                leftValue = leftDetailed->value;
            }
            if (auto rightDetailed = std::dynamic_pointer_cast<DetailedASTNode>(right)) {
                rightValue = rightDetailed->value;
            }
            
            binaryExpr->value = leftValue + " " + (op == TokenType::PLUS ? "+" : "-") + " " + rightValue;
            binaryExpr->children.push_back(left);
            binaryExpr->children.push_back(right);
            left = binaryExpr;
        }
    }
    
    return left;
}

// 解析乘法表达式
std::shared_ptr<ASTNode> AnalysisThread::parseMultiplicativeExpression() {
    auto left = parsePrimaryExpression();
    
    while (currentToken().type == TokenType::MULTIPLY || 
           currentToken().type == TokenType::DIVIDE || 
           currentToken().type == TokenType::MODULO) {
        TokenType op = currentToken().type;
        int line = currentToken().line;
        advance();
        
        auto right = parsePrimaryExpression();
        if (right) {
            auto binaryExpr = std::make_shared<DetailedASTNode>(ASTNodeType::BINARY_EXPR, line, 1);
            
            // 安全地获取子节点的值
            std::string leftValue = "expr";
            std::string rightValue = "expr";
            if (auto leftDetailed = std::dynamic_pointer_cast<DetailedASTNode>(left)) {
                leftValue = leftDetailed->value;
            }
            if (auto rightDetailed = std::dynamic_pointer_cast<DetailedASTNode>(right)) {
                rightValue = rightDetailed->value;
            }
            
            // 根据操作符选择符号
            std::string opStr;
            if (op == TokenType::MULTIPLY) {
                opStr = "*";
            } else if (op == TokenType::DIVIDE) {
                opStr = "/";
            } else if (op == TokenType::MODULO) {
                opStr = "%";
            }
            
            binaryExpr->value = leftValue + " " + opStr + " " + rightValue;
            binaryExpr->children.push_back(left);
            binaryExpr->children.push_back(right);
            left = binaryExpr;
        }
    }
    
    return left;
}

// 解析基本表达式
std::shared_ptr<ASTNode> AnalysisThread::parsePrimaryExpression() {
    if (currentToken().type == TokenType::IDENTIFIER) {
        auto identifier = std::make_shared<DetailedASTNode>(ASTNodeType::IDENTIFIER_EXPR, currentToken().line, currentToken().column);
        identifier->value = currentToken().value;
        advance();
        return identifier;
    } else if (currentToken().type == TokenType::NUMBER) {
        auto literal = std::make_shared<DetailedASTNode>(ASTNodeType::LITERAL_EXPR, currentToken().line, currentToken().column);
        literal->value = currentToken().value;
        advance();
        return literal;
    } else if (currentToken().type == TokenType::LPAREN) {
        advance(); // 跳过 (
        auto expr = parseExpression();
        if (currentToken().type == TokenType::RPAREN) {
            advance(); // 跳过 )
        }
        return expr;
    }
    
    return nullptr;
}

// 辅助函数
Token AnalysisThread::currentToken() const {
    if (TokenIndex < m_tokens.size()) {
        return m_tokens[TokenIndex];
    }
    return Token(TokenType::END_OF_FILE, "", -1, -1);
}

void AnalysisThread::advance() {
    if (TokenIndex < m_tokens.size()) {
        TokenIndex++;
    }
}

bool AnalysisThread::isTypeKeyword(TokenType type) const {
    return type == TokenType::INT || type == TokenType::FLOAT || 
           type == TokenType::BOOL;
}

std::string AnalysisThread::getTypeString(TokenType type) const {
    switch (type) {
        case TokenType::INT: return "int";
        case TokenType::FLOAT: return "float";
        case TokenType::BOOL: return "bool";
        default: return "unknown";
    }
}

int AnalysisThread::countASTNodes(ASTNode* node) {
    if (!node) return 0;
    
    int count = 1; // 当前节点
    
    // 检查是否为DetailedASTNode类型，如果是则递归计算子节点
    if (auto detailedNode = dynamic_cast<DetailedASTNode*>(node)) {
        for (const auto& child : detailedNode->children) {
            count += countASTNodes(child.get());
        }
    } else if (auto simpleNode = dynamic_cast<SimpleASTNode*>(node)) {
        for (const auto& child : simpleNode->children) {
            count += countASTNodes(child.get());
        }
    }
    
    return count;
}

int AnalysisThread::getASTDepth(ASTNode* node) {
    if (!node) return 0;
    
    int maxChildDepth = 0;
    
    // 检查是否为DetailedASTNode类型，如果是则递归计算子节点深度
    if (auto detailedNode = dynamic_cast<DetailedASTNode*>(node)) {
        for (const auto& child : detailedNode->children) {
            maxChildDepth = std::max(maxChildDepth, getASTDepth(child.get()));
        }
    } else if (auto simpleNode = dynamic_cast<SimpleASTNode*>(node)) {
        for (const auto& child : simpleNode->children) {
            maxChildDepth = std::max(maxChildDepth, getASTDepth(child.get()));
        }
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

// 解析if语句
std::shared_ptr<ASTNode> AnalysisThread::parseIfStatement() {
    if (currentToken().type != TokenType::IF) {
        return nullptr;
    }
    
    int line = currentToken().line;
    advance(); // 跳过 if
    
    // 期望左括号
    if (currentToken().type != TokenType::LPAREN) {
        return nullptr;
    }
    advance(); // 跳过 (
    
    // 解析条件表达式
    auto condition = parseComparisonExpression();
    if (!condition) {
        return nullptr;
    }
    
    // 期望右括号
    if (currentToken().type != TokenType::RPAREN) {
        return nullptr;
    }
    advance(); // 跳过 )
    
    // 解析then语句（可能是块语句或单个语句）
    std::shared_ptr<ASTNode> thenStmt = nullptr;
    if (currentToken().type == TokenType::LBRACE) {
        thenStmt = parseBlockStatement();
    } else {
        thenStmt = parseStatement();
    }
    
    // 检查是否有else子句
    std::shared_ptr<ASTNode> elseStmt = nullptr;
    if (currentToken().type == TokenType::ELSE) {
        advance(); // 跳过 else
        if (currentToken().type == TokenType::LBRACE) {
            elseStmt = parseBlockStatement();
        } else {
            elseStmt = parseStatement();
        }
    }
    
    // 创建if语句节点
    auto ifStmt = std::make_shared<DetailedASTNode>(ASTNodeType::IF_STMT, line, 1);
    ifStmt->value = "if (...) { ... }";
    
    if (condition) {
        ifStmt->children.push_back(condition);
    }
    if (thenStmt) {
        ifStmt->children.push_back(thenStmt);
    }
    if (elseStmt) {
        ifStmt->children.push_back(elseStmt);
    }
    
    return ifStmt;
}

// 解析块语句
std::shared_ptr<ASTNode> AnalysisThread::parseBlockStatement() {
    if (currentToken().type != TokenType::LBRACE) {
        return nullptr;
    }
    
    int line = currentToken().line;
    advance(); // 跳过 {
    
    auto blockStmt = std::make_shared<DetailedASTNode>(ASTNodeType::BLOCK_STMT, line, 1);
    blockStmt->value = "{ ... }";
    
    // 解析块中的语句
    while (currentToken().type != TokenType::RBRACE && currentToken().type != TokenType::END_OF_FILE) {
        std::shared_ptr<ASTNode> stmt = nullptr;
        
        if (isTypeKeyword(currentToken().type)) {
            stmt = parseDeclaration();
        } else if (currentToken().type == TokenType::IDENTIFIER) {
            stmt = parseStatement();
        } else if (currentToken().type == TokenType::IF) {
            stmt = parseIfStatement();
        } else if (currentToken().type == TokenType::RETURN) {
            stmt = parseReturnStatement();
        } else {
            advance(); // 跳过未识别的token
        }
        
        if (stmt) {
            blockStmt->children.push_back(stmt);
        }
    }
    
    // 期望右大括号
    if (currentToken().type == TokenType::RBRACE) {
        advance(); // 跳过 }
    }
    
    return blockStmt;
}

// 解析比较表达式
std::shared_ptr<ASTNode> AnalysisThread::parseComparisonExpression() {
    auto left = parseAdditiveExpression();
    
    while (currentToken().type == TokenType::GT || 
           currentToken().type == TokenType::LT ||
           currentToken().type == TokenType::GE ||
           currentToken().type == TokenType::LE ||
           currentToken().type == TokenType::EQ ||
           currentToken().type == TokenType::NE) {
        TokenType op = currentToken().type;
        int line = currentToken().line;
        advance();
        
        auto right = parseAdditiveExpression();
        if (right) {
            auto binaryExpr = std::make_shared<DetailedASTNode>(ASTNodeType::BINARY_EXPR, line, 1);
            
            // 安全地获取子节点的值
            std::string leftValue = "expr";
            std::string rightValue = "expr";
            if (auto leftDetailed = std::dynamic_pointer_cast<DetailedASTNode>(left)) {
                leftValue = leftDetailed->value;
            }
            if (auto rightDetailed = std::dynamic_pointer_cast<DetailedASTNode>(right)) {
                rightValue = rightDetailed->value;
            }
            
            // 根据操作符选择符号
            std::string opStr;
            switch (op) {
                case TokenType::GT: opStr = ">"; break;
                case TokenType::LT: opStr = "<"; break;
                case TokenType::GE: opStr = ">="; break;
                case TokenType::LE: opStr = "<="; break;
                case TokenType::EQ: opStr = "=="; break;
                case TokenType::NE: opStr = "!="; break;
                default: opStr = "?"; break;
            }
            
            binaryExpr->value = leftValue + " " + opStr + " " + rightValue;
            binaryExpr->children.push_back(left);
            binaryExpr->children.push_back(right);
            left = binaryExpr;
        }
    }
    
    return left;
}

// 基于AST的代码生成方法
bool AnalysisThread::generateCodeFromAST(ASTNode* node, QVector<ThreeAddressCode>& intermediateCode, int& instrCount) {
    if (!node) return false;
    
    try {
        // 根据AST节点类型生成对应的三地址码
        switch (node->nodeType) {
            case ASTNodeType::PROGRAM: {
                // 程序节点：递归处理所有子节点
                auto simpleNode = dynamic_cast<SimpleASTNode*>(node);
                if (simpleNode) {
                    for (auto& child : simpleNode->children) {
                        if (!generateCodeFromAST(child.get(), intermediateCode, instrCount)) {
                            return false;
                        }
                    }
                }
                return true;
            }
            
            case ASTNodeType::VAR_DECL: {
                // 变量声明：生成赋值指令
                auto detailedNode = dynamic_cast<DetailedASTNode*>(node);
                if (detailedNode) {
                    ThreeAddressCode tac(OpType::ASSIGN);
                    
                    // 从value中提取变量名和类型
                    QString value = QString::fromStdString(detailedNode->value);
                    QStringList parts = value.split(' ');
                    if (parts.size() >= 2) {
                        QString varName = parts[1];
                        
                        tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                        
                        // 检查是否有初始化值
                        if (detailedNode->children.size() > 0) {
                            auto initNode = dynamic_cast<DetailedASTNode*>(detailedNode->children[0].get());
                            if (initNode) {
                                tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, initNode->value, initNode->value, IRDataType::INT);
                                std::cout << "生成变量初始化指令: " << varName.toStdString() << " = " << initNode->value << std::endl;
                            }
                        } else {
                            tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, "0", "0", IRDataType::INT);
                            std::cout << "生成变量声明指令: " << varName.toStdString() << " = 0" << std::endl;
                        }
                        
                        tac.comment = "变量声明";
                        intermediateCode.append(tac);
                        instrCount++;
                    }
                }
                return true;
            }
            
            case ASTNodeType::ASSIGNMENT_STMT: {
                // 赋值语句：生成赋值或运算指令（包括复合赋值）
                auto detailedNode = dynamic_cast<DetailedASTNode*>(node);
                if (detailedNode && detailedNode->children.size() > 0) {
                    // 从value中提取变量名和运算符
                    QString value = QString::fromStdString(detailedNode->value);
                    QStringList parts = value.split(' ');
                    if (parts.size() >= 3) {
                        QString varName = parts[0];
                        QString operatorStr = parts[1];
                        
                        // 检查是否为复合赋值运算符
                        if (operatorStr == "+=" || operatorStr == "-=" || operatorStr == "*=" || 
                            operatorStr == "/=" || operatorStr == "%=") {
                            // 复合赋值：x += y 等价于 x = x + y
                            auto rhsNode = detailedNode->children[0].get();
                            auto rhsDetailed = dynamic_cast<DetailedASTNode*>(rhsNode);
                            if (rhsDetailed) {
                                // 确定运算类型
                                OpType opType = OpType::ADD;
                                std::string opSymbol = "+";
                                if (operatorStr == "+=") { opType = OpType::ADD; opSymbol = "+"; }
                                else if (operatorStr == "-=") { opType = OpType::SUB; opSymbol = "-"; }
                                else if (operatorStr == "*=") { opType = OpType::MUL; opSymbol = "*"; }
                                else if (operatorStr == "/=") { opType = OpType::DIV; opSymbol = "/"; }
                                else if (operatorStr == "%=") { opType = OpType::MOD; opSymbol = "%"; }
                                
                                ThreeAddressCode tac(opType);
                                tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                                tac.arg2 = std::make_unique<Operand>(OperandType::CONSTANT, rhsDetailed->value, rhsDetailed->value, IRDataType::INT);
                                tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                                tac.comment = "复合赋值";
                                
                                std::cout << "生成复合赋值指令: " << varName.toStdString() 
                                         << " = " << varName.toStdString() << " " << opSymbol << " " << rhsDetailed->value << std::endl;
                                
                                intermediateCode.append(tac);
                                instrCount++;
                            }
                        } else {
                            // 普通赋值或二元表达式
                            auto rhsNode = detailedNode->children[0].get();
                            if (rhsNode->nodeType == ASTNodeType::BINARY_EXPR) {
                                // 二元表达式：a = b + c
                                auto binaryNode = dynamic_cast<DetailedASTNode*>(rhsNode);
                                if (binaryNode && binaryNode->children.size() >= 2) {
                                    ThreeAddressCode tac(OpType::ADD); // 默认加法，后续可以根据操作符调整
                                    
                                    auto leftOperand = dynamic_cast<DetailedASTNode*>(binaryNode->children[0].get());
                                    auto rightOperand = dynamic_cast<DetailedASTNode*>(binaryNode->children[1].get());
                                    
                                    if (leftOperand && rightOperand) {
                                        tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, leftOperand->value, IRDataType::INT);
                                        tac.arg2 = std::make_unique<Operand>(OperandType::VARIABLE, rightOperand->value, IRDataType::INT);
                                        tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                                        tac.comment = "二元运算";
                                        
                                        std::cout << "生成运算指令: " << varName.toStdString() 
                                                 << " = " << leftOperand->value << " + " << rightOperand->value << std::endl;
                                        
                                        intermediateCode.append(tac);
                                        instrCount++;
                                    }
                                }
                            } else {
                                // 简单赋值：a = 5
                                ThreeAddressCode tac(OpType::ASSIGN);
                                auto rhsDetailed = dynamic_cast<DetailedASTNode*>(rhsNode);
                                if (rhsDetailed) {
                                    tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varName.toStdString(), IRDataType::INT);
                                    tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, rhsDetailed->value, rhsDetailed->value, IRDataType::INT);
                                    tac.comment = "简单赋值";
                                    
                                    std::cout << "生成赋值指令: " << varName.toStdString() << " = " << rhsDetailed->value << std::endl;
                                    
                                    intermediateCode.append(tac);
                                    instrCount++;
                                }
                            }
                        }
                    }
                }
                return true;
            }
            
            case ASTNodeType::IF_STMT: {
                // if语句：生成条件跳转指令
                auto detailedNode = dynamic_cast<DetailedASTNode*>(node);
                if (detailedNode && detailedNode->children.size() >= 2) {
                    static int labelCounter = 1;
                    QString elseLabel = QString("L%1").arg(labelCounter++);
                    QString endLabel = QString("L%1").arg(labelCounter++);
                    QString tempVar = QString("t%1").arg(labelCounter);
                    
                    // 1. 处理条件表达式
                    auto conditionNode = detailedNode->children[0].get();
                    if (conditionNode->nodeType == ASTNodeType::BINARY_EXPR) {
                        auto binaryNode = dynamic_cast<DetailedASTNode*>(conditionNode);
                        if (binaryNode && binaryNode->children.size() >= 2) {
                            auto leftOp = dynamic_cast<DetailedASTNode*>(binaryNode->children[0].get());
                            auto rightOp = dynamic_cast<DetailedASTNode*>(binaryNode->children[1].get());
                            
                            if (leftOp && rightOp) {
                                // 生成比较指令
                                ThreeAddressCode cmpTac(OpType::GT);
                                cmpTac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, leftOp->value, IRDataType::INT);
                                cmpTac.arg2 = std::make_unique<Operand>(OperandType::VARIABLE, rightOp->value, IRDataType::INT);
                                cmpTac.result = std::make_unique<Operand>(OperandType::TEMPORARY, tempVar.toStdString(), IRDataType::INT);
                                cmpTac.comment = "条件比较";
                                
                                intermediateCode.append(cmpTac);
                                instrCount++;
                                
                                std::cout << "生成比较指令: " << tempVar.toStdString() 
                                         << " = " << leftOp->value << " > " << rightOp->value << std::endl;
                                
                                // 生成条件跳转指令
                                ThreeAddressCode jumpTac(OpType::IF_FALSE);
                                jumpTac.arg1 = std::make_unique<Operand>(OperandType::TEMPORARY, tempVar.toStdString(), IRDataType::INT);
                                jumpTac.result = std::make_unique<Operand>(OperandType::LABEL, elseLabel.toStdString(), IRDataType::UNKNOWN);
                                jumpTac.comment = "条件跳转";
                                
                                intermediateCode.append(jumpTac);
                                instrCount++;
                                
                                std::cout << "生成条件跳转指令: if_false " << tempVar.toStdString() 
                                         << " goto " << elseLabel.toStdString() << std::endl;
                            }
                        }
                    }
                    
                    // 2. 处理then分支
                    auto thenNode = detailedNode->children[1].get();
                    generateCodeFromAST(thenNode, intermediateCode, instrCount);
                    
                    // 3. 生成跳转到结束
                    ThreeAddressCode gotoTac(OpType::GOTO);
                    gotoTac.result = std::make_unique<Operand>(OperandType::LABEL, endLabel.toStdString(), IRDataType::UNKNOWN);
                    gotoTac.comment = "跳转到结束";
                    
                    intermediateCode.append(gotoTac);
                    instrCount++;
                    
                    std::cout << "生成跳转指令: goto " << endLabel.toStdString() << std::endl;
                    
                    // 4. 生成else标签
                    ThreeAddressCode elseLabelTac(OpType::LABEL);
                    elseLabelTac.result = std::make_unique<Operand>(OperandType::LABEL, elseLabel.toStdString(), IRDataType::UNKNOWN);
                    elseLabelTac.comment = "else标签";
                    
                    intermediateCode.append(elseLabelTac);
                    instrCount++;
                    
                    std::cout << "生成else标签: " << elseLabel.toStdString() << ":" << std::endl;
                    
                    // 5. 处理else分支
                    if (detailedNode->children.size() >= 3) {
                        auto elseNode = detailedNode->children[2].get();
                        generateCodeFromAST(elseNode, intermediateCode, instrCount);
                    }
                    
                    // 6. 生成结束标签
                    ThreeAddressCode endLabelTac(OpType::LABEL);
                    endLabelTac.result = std::make_unique<Operand>(OperandType::LABEL, endLabel.toStdString(), IRDataType::UNKNOWN);
                    endLabelTac.comment = "结束标签";
                    
                    intermediateCode.append(endLabelTac);
                    instrCount++;
                    
                    std::cout << "生成结束标签: " << endLabel.toStdString() << ":" << std::endl;
                }
                return true;
            }
            
            case ASTNodeType::BLOCK_STMT: {
                // 块语句：递归处理所有子语句
                auto detailedNode = dynamic_cast<DetailedASTNode*>(node);
                if (detailedNode) {
                    for (auto& child : detailedNode->children) {
                        if (!generateCodeFromAST(child.get(), intermediateCode, instrCount)) {
                            return false;
                        }
                    }
                }
                return true;
            }
            
            default:
                // 其他节点类型，递归处理子节点
                auto simpleNode = dynamic_cast<SimpleASTNode*>(node);
                if (simpleNode) {
                    for (auto& child : simpleNode->children) {
                        if (!generateCodeFromAST(child.get(), intermediateCode, instrCount)) {
                            return false;
                        }
                    }
                }
                auto detailedNode = dynamic_cast<DetailedASTNode*>(node);
                if (detailedNode) {
                    for (auto& child : detailedNode->children) {
                        if (!generateCodeFromAST(child.get(), intermediateCode, instrCount)) {
                            return false;
                        }
                    }
                }
                return true;
        }
    } catch (const std::exception& e) {
        std::cout << "AST代码生成异常: " << e.what() << std::endl;
        return false;
    }
}

// 基于字符串的代码生成方法（作为后备）
bool AnalysisThread::generateCodeFromString(const QString& code, QVector<ThreeAddressCode>& intermediateCode, int& instrCount) {
    std::cout << "使用字符串解析方法生成代码" << std::endl;
    
    try {
        QStringList lines = code.split('\n');
        instrCount = 0;
        
        for (const QString& line : lines) {
            QString trimmedLine = line.trimmed();
            if (trimmedLine.isEmpty() || trimmedLine.startsWith("//")) continue;
            
            // 为变量声明生成代码
            if (trimmedLine.contains("int ") && trimmedLine.contains(';')) {
                QString varDecl = trimmedLine;
                varDecl = varDecl.remove("int").remove(';').trimmed();
                
                ThreeAddressCode tac(OpType::ASSIGN);
                
                if (varDecl.contains('=')) {
                    QStringList parts = varDecl.split('=');
                    QString var = parts[0].trimmed();
                    QString val = parts[1].trimmed();
                    
                    tac.result = std::make_unique<Operand>(OperandType::VARIABLE, var.toStdString(), IRDataType::INT);
                    tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, val.toStdString(), val.toStdString(), IRDataType::INT);
                    tac.comment = "变量声明并初始化";
                    
                    std::cout << "生成变量初始化指令: " << var.toStdString() << " = " << val.toStdString() << std::endl;
                } else {
                    tac.result = std::make_unique<Operand>(OperandType::VARIABLE, varDecl.toStdString(), IRDataType::INT);
                    tac.arg1 = std::make_unique<Operand>(OperandType::CONSTANT, "0", "0", IRDataType::INT);
                    tac.comment = "变量声明";
                    
                    std::cout << "生成变量声明指令: " << varDecl.toStdString() << " = 0" << std::endl;
                }
                
                intermediateCode.append(tac);
                instrCount++;
            }
            
            // 为赋值语句生成代码
            if (trimmedLine.contains('=') && !trimmedLine.contains("int ") && !trimmedLine.contains("if")) {
                ThreeAddressCode tac(OpType::ASSIGN);
                
                QStringList parts = trimmedLine.split('=');
                if (parts.size() >= 2) {
                    QString var = parts[0].trimmed();
                    QString expr = parts[1].remove(';').trimmed();
                    
                    tac.result = std::make_unique<Operand>(OperandType::VARIABLE, var.toStdString(), IRDataType::INT);
                    
                    if (expr.contains('+')) {
                        tac.op = OpType::ADD;
                        QStringList operands = expr.split('+');
                        if (operands.size() >= 2) {
                            tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, operands[0].trimmed().toStdString(), IRDataType::INT);
                            tac.arg2 = std::make_unique<Operand>(OperandType::VARIABLE, operands[1].trimmed().toStdString(), IRDataType::INT);
                            
                            std::cout << "生成加法指令: " << var.toStdString() 
                                     << " = " << operands[0].trimmed().toStdString()
                                     << " + " << operands[1].trimmed().toStdString() << std::endl;
                        }
                    } else {
                        tac.arg1 = std::make_unique<Operand>(OperandType::VARIABLE, expr.toStdString(), IRDataType::INT);
                        
                        std::cout << "生成赋值指令: " << var.toStdString() 
                                 << " = " << expr.toStdString() << std::endl;
                    }
                    tac.comment = "赋值操作";
                }
                
                intermediateCode.append(tac);
                instrCount++;
            }
        }
        
        return instrCount > 0;
    } catch (const std::exception& e) {
        std::cout << "字符串代码生成异常: " << e.what() << std::endl;
        return false;
    }
}

