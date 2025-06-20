#include "analysis_panel.h"
#include <QApplication>
#include <QHeaderView>
#include <QFont>

// ============ LexicalAnalysisPanel 实现 ============

LexicalAnalysisPanel::LexicalAnalysisPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void LexicalAnalysisPanel::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Token表格组
    tokenGroupBox = new QGroupBox("Token序列", this);
    QVBoxLayout *tokenLayout = new QVBoxLayout(tokenGroupBox);
    
    setupTokenTable();
    tokenLayout->addWidget(tokenTable);
    
    // 右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    
    // DFA信息组
    dfaGroupBox = new QGroupBox("DFA信息", this);
    QVBoxLayout *dfaLayout = new QVBoxLayout(dfaGroupBox);
    
    dfaInfoText = new QTextEdit(this);
    dfaInfoText->setReadOnly(true);
    dfaInfoText->setFont(QFont("Consolas", 8));
    dfaLayout->addWidget(dfaInfoText);
    
    // 最小化DFA信息组
    minimizedDfaGroupBox = new QGroupBox("最小化DFA", this);
    QVBoxLayout *minimizedDfaLayout = new QVBoxLayout(minimizedDfaGroupBox);
    
    minimizedDFAInfoText = new QTextEdit(this);
    minimizedDFAInfoText->setReadOnly(true);
    minimizedDFAInfoText->setFont(QFont("Consolas", 8));
    minimizedDfaLayout->addWidget(minimizedDFAInfoText);
    
    // 统计信息组
    statisticsGroupBox = new QGroupBox("统计信息", this);
    QVBoxLayout *statisticsLayout = new QVBoxLayout(statisticsGroupBox);
    
    statisticsLabel = new QLabel("等待分析...", this);
    statisticsLabel->setFont(QFont("Consolas", 9));
    statisticsLabel->setAlignment(Qt::AlignTop);
    statisticsLayout->addWidget(statisticsLabel);
    
    // 组装右侧面板
    rightSplitter->addWidget(dfaGroupBox);
    rightSplitter->addWidget(minimizedDfaGroupBox);
    rightSplitter->addWidget(statisticsGroupBox);
    rightSplitter->setSizes({1, 1, 1});
    
    // 添加到主分割器
    mainSplitter->addWidget(tokenGroupBox);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1});
    
    mainLayout->addWidget(mainSplitter);
}

void LexicalAnalysisPanel::setupTokenTable()
{
    tokenTable = new QTableWidget(this);
    tokenTable->setColumnCount(5);
    
    QStringList headers;
    headers << "序号" << "类型" << "值" << "行号" << "列号";
    tokenTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    tokenTable->setAlternatingRowColors(true);
    tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokenTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenTable->setSortingEnabled(false);
    
    // 设置列宽
    tokenTable->horizontalHeader()->setStretchLastSection(true);
    tokenTable->setColumnWidth(0, 60);  // 序号
    tokenTable->setColumnWidth(1, 120); // 类型
    tokenTable->setColumnWidth(2, 150); // 值
    tokenTable->setColumnWidth(3, 60);  // 行号
    tokenTable->setColumnWidth(4, 60);  // 列号
}

void LexicalAnalysisPanel::setTokens(const QVector<Token> &tokens)
{
    tokenTable->setRowCount(tokens.size());
    
    for (int i = 0; i < tokens.size(); ++i) {
        const Token &token = tokens[i];
        
        // 序号
        QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        tokenTable->setItem(i, 0, indexItem);
        
        // 类型 - 简化实现，避免API问题
        QTableWidgetItem *typeItem = new QTableWidgetItem(QString::fromStdString(token.value));
        tokenTable->setItem(i, 1, typeItem);
        
        // 值 - 使用value
        QTableWidgetItem *valueItem = new QTableWidgetItem(QString::fromStdString(token.value));
        tokenTable->setItem(i, 2, valueItem);
        
        // 行号
        QTableWidgetItem *lineItem = new QTableWidgetItem(QString::number(token.line));
        lineItem->setTextAlignment(Qt::AlignCenter);
        tokenTable->setItem(i, 3, lineItem);
        
        // 列号
        QTableWidgetItem *columnItem = new QTableWidgetItem(QString::number(token.column));
        columnItem->setTextAlignment(Qt::AlignCenter);
        tokenTable->setItem(i, 4, columnItem);
    }
    
    tokenTable->resizeRowsToContents();
}

void LexicalAnalysisPanel::clearTokens()
{
    tokenTable->setRowCount(0);
}

void LexicalAnalysisPanel::setDFAInfo(const QString &dfaInfo)
{
    dfaInfoText->setText(dfaInfo);
}

void LexicalAnalysisPanel::setMinimizedDFAInfo(const QString &minimizedDFAInfo)
{
    minimizedDFAInfoText->setText(minimizedDFAInfo);
}

void LexicalAnalysisPanel::updateStatistics(int totalTokens, int totalLines, int totalChars)
{
    QString stats = QString(
        "词法分析统计:\n"
        "• 总Token数: %1\n"
        "• 总行数: %2\n"
        "• 总字符数: %3\n"
        "• 平均Token/行: %4\n"
    ).arg(totalTokens)
     .arg(totalLines)
     .arg(totalChars)
     .arg(totalLines > 0 ? QString::number(double(totalTokens) / totalLines, 'f', 2) : "0");
    
    statisticsLabel->setText(stats);
}

// ============ SyntaxAnalysisPanel 实现 ============

SyntaxAnalysisPanel::SyntaxAnalysisPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SyntaxAnalysisPanel::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 创建简化的语法分析面板
    QLabel *label = new QLabel("语法分析面板 (演示模式)\n\n数据绑定功能开发中...", this);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(QFont("Arial", 12));
    
    mainLayout->addWidget(label);
}

void SyntaxAnalysisPanel::setAST(const std::shared_ptr<ASTNode> &ast)
{
    Q_UNUSED(ast)
    // 简化实现，暂时跳过复杂的AST处理
}

void SyntaxAnalysisPanel::clearAST()
{
    // 简化实现
}

void SyntaxAnalysisPanel::setParseInfo(const QString &parseInfo)
{
    Q_UNUSED(parseInfo)
    // 简化实现
}

void SyntaxAnalysisPanel::setGrammarInfo(const QString &grammarInfo)
{
    Q_UNUSED(grammarInfo)
    // 简化实现
}

void SyntaxAnalysisPanel::setLALRTableInfo(const QString &lalrTableInfo)
{
    Q_UNUSED(lalrTableInfo)
    // 简化实现
}

void SyntaxAnalysisPanel::addParseError(const QString &error, int line)
{
    Q_UNUSED(error)
    Q_UNUSED(line)
    // 简化实现
}

void SyntaxAnalysisPanel::clearParseErrors()
{
    // 简化实现
}

void SyntaxAnalysisPanel::populateASTTree(QTreeWidgetItem *parent, const std::shared_ptr<ASTNode> &node)
{
    Q_UNUSED(parent)
    Q_UNUSED(node)
    // 简化实现，避免复杂的AST API
}

QString SyntaxAnalysisPanel::getNodeTypeString(ASTNodeType type)
{
    Q_UNUSED(type)
    return "节点";
}

// ============ SemanticAnalysisPanel 实现 ============

SemanticAnalysisPanel::SemanticAnalysisPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SemanticAnalysisPanel::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 创建简化的语义分析面板
    QLabel *label = new QLabel("语义分析面板 (演示模式)\n\n数据绑定功能开发中...", this);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(QFont("Arial", 12));
    
    mainLayout->addWidget(label);
}

void SemanticAnalysisPanel::setSymbolTable(const SymbolTable &symbolTable)
{
    Q_UNUSED(symbolTable)
    // 简化实现
}

void SemanticAnalysisPanel::clearSymbolTable()
{
    // 简化实现
}

void SemanticAnalysisPanel::setTypeCheckInfo(const QString &typeCheckInfo)
{
    Q_UNUSED(typeCheckInfo)
    // 简化实现
}

void SemanticAnalysisPanel::setScopeInfo(const QString &scopeInfo)
{
    Q_UNUSED(scopeInfo)
    // 简化实现
}

void SemanticAnalysisPanel::addSemanticError(const QString &error, int line)
{
    Q_UNUSED(error)
    Q_UNUSED(line)
    // 简化实现
}

void SemanticAnalysisPanel::clearSemanticErrors()
{
    // 简化实现
}

void SemanticAnalysisPanel::populateSymbolTable(const SymbolTable &symbolTable)
{
    Q_UNUSED(symbolTable)
    // 简化实现
}

QString SemanticAnalysisPanel::getSymbolTypeString(SymbolType type)
{
    Q_UNUSED(type)
    return "符号";
}

QString SemanticAnalysisPanel::getDataTypeString(DataType type)
{
    Q_UNUSED(type)
    return "类型";
}

// ============ CodeGenerationPanel 实现 ============

CodeGenerationPanel::CodeGenerationPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void CodeGenerationPanel::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 创建简化的代码生成面板
    QLabel *label = new QLabel("代码生成面板 (演示模式)\n\n数据绑定功能开发中...", this);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(QFont("Arial", 12));
    
    mainLayout->addWidget(label);
}

void CodeGenerationPanel::setIntermediateCode(const QVector<ThreeAddressCode> &codes)
{
    Q_UNUSED(codes)
    // 简化实现，避免ThreeAddressCode的复制问题
}

void CodeGenerationPanel::clearIntermediateCode()
{
    // 简化实现
}

void CodeGenerationPanel::setBasicBlockInfo(const QString &blockInfo)
{
    Q_UNUSED(blockInfo)
    // 简化实现
}

void CodeGenerationPanel::setOptimizationInfo(const QString &optimizationInfo)
{
    Q_UNUSED(optimizationInfo)
    // 简化实现
}

void CodeGenerationPanel::updateCodeGenStatistics(int totalInstructions, int basicBlocks, int tempVars)
{
    Q_UNUSED(totalInstructions)
    Q_UNUSED(basicBlocks)
    Q_UNUSED(tempVars)
    // 简化实现
}

void CodeGenerationPanel::setupCodeTable()
{
    // 简化实现
}

QString CodeGenerationPanel::getOpTypeString(OpType type)
{
    Q_UNUSED(type)
    return "操作";
}

QString CodeGenerationPanel::getOperandString(const Operand &operand)
{
    Q_UNUSED(operand)
    return "操作数";
}

// ============ AnalysisPanel 实现 ============

AnalysisPanel::AnalysisPanel(QWidget *parent)
    : QTabWidget(parent)
{
    setupTabs();
}

void AnalysisPanel::setupTabs()
{
    // 创建各个面板
    lexicalPanel = new LexicalAnalysisPanel(this);
    syntaxPanel = new SyntaxAnalysisPanel(this);
    semanticPanel = new SemanticAnalysisPanel(this);
    codeGenPanel = new CodeGenerationPanel(this);
    
    // 添加标签页
    addTab(lexicalPanel, "词法分析");
    addTab(syntaxPanel, "语法分析");
    addTab(semanticPanel, "语义分析");
    addTab(codeGenPanel, "代码生成");
    
    // 连接信号
    connect(this, QOverload<int>::of(&QTabWidget::currentChanged),
            this, &AnalysisPanel::onTabChanged);
}

void AnalysisPanel::switchToLexicalTab()
{
    setCurrentWidget(lexicalPanel);
}

void AnalysisPanel::switchToSyntaxTab()
{
    setCurrentWidget(syntaxPanel);
}

void AnalysisPanel::switchToSemanticTab()
{
    setCurrentWidget(semanticPanel);
}

void AnalysisPanel::switchToCodeGenTab()
{
    setCurrentWidget(codeGenPanel);
}

void AnalysisPanel::clearAllResults()
{
    lexicalPanel->clearTokens();
    syntaxPanel->clearAST();
    semanticPanel->clearSymbolTable();
    codeGenPanel->clearIntermediateCode();
}

void AnalysisPanel::onTabChanged(int index)
{
    emit tabChanged(index);
}

void AnalysisPanel::updateLexicalAnalysisResult(const LexicalResultGUI& result) {
    // 转换为QVector并使用现有API
    QVector<Token> tokens;
    for (const auto& token : result.tokens) {
        tokens.append(token);
    }
    
    // 使用子面板的方法
    lexicalPanel->setTokens(tokens);
    
    // 更新DFA信息
    QString dfaInfo = QString("原始DFA信息:\n状态数: %1\n接收状态: %2\n转换数: %3")
        .arg(result.originalDfaStates)
        .arg(result.originalAcceptStates)
        .arg(result.originalTransitions);
    lexicalPanel->setDFAInfo(dfaInfo);
    
    // 更新最小化DFA信息
    QString minimizedInfo = QString("最小化DFA信息:\n状态数: %1\n接收状态: %2\n转换数: %3\n压缩率: %4%")
        .arg(result.minimizedDfaStates)
        .arg(result.minimizedAcceptStates)
        .arg(result.minimizedTransitions)
        .arg(QString::number(result.compressionRatio * 100.0, 'f', 2));
    lexicalPanel->setMinimizedDFAInfo(minimizedInfo);
    
    // 更新统计信息
    lexicalPanel->updateStatistics(
        static_cast<int>(result.tokens.size()),
        10,  // 假设行数
        static_cast<int>(result.memoryUsage)
    );
}

void AnalysisPanel::updateSyntaxAnalysisResult(const SyntaxResult& result) {
    // 演示模式：使用现有的子面板API
    if (result.success) {
        QString parseInfo = QString("语法分析成功\n"
                                   "规约次数: %1\n"
                                   "移进次数: %2\n"
                                   "冲突数: %3")
            .arg(result.parseInfo.reductions)
            .arg(result.parseInfo.shifts)
            .arg(result.parseInfo.conflicts);
        syntaxPanel->setParseInfo(parseInfo);
        
        QString grammarInfo = QString("产生式数: %1\n非终结符: %2\n终结符: %3")
            .arg(result.grammarInfo.productions)
            .arg(result.grammarInfo.nonterminals)
            .arg(result.grammarInfo.terminals);
        syntaxPanel->setGrammarInfo(grammarInfo);
    } else {
        syntaxPanel->addParseError(QString::fromStdString(result.error));
    }
}

void AnalysisPanel::updateSemanticAnalysisResult(const SemanticResultGUI& result) {
    // 演示模式：使用现有的子面板API
    QString typeCheckInfo = result.success ? "类型检查通过" : 
        QString("类型检查失败: %1个错误").arg(result.errors.size());
    semanticPanel->setTypeCheckInfo(typeCheckInfo);
    
    QString scopeInfo = QString("作用域层数: %1\n当前作用域: %2")
        .arg(result.scopeInfo.maxDepth)
        .arg(result.scopeInfo.currentLevel);
    semanticPanel->setScopeInfo(scopeInfo);
    
    // 添加错误信息
    for (const auto& error : result.errors) {
        semanticPanel->addSemanticError(
            QString::fromStdString(error.message), 
            error.line
        );
    }
}

void AnalysisPanel::updateCodeGenerationResult(const CodeGenResultGUI& result) {
    // 演示模式：使用现有的子面板API
    
    // 更新优化信息
    QString optimizationInfo = QString("优化通道: %1\n指令优化: %2\n常量折叠: %3")
        .arg(result.optimizationInfo.passes)
        .arg(result.optimizationInfo.instructionsOptimized)
        .arg(result.optimizationInfo.constantsFolded);
    codeGenPanel->setOptimizationInfo(optimizationInfo);
    
    // 更新基本块信息
    QString basicBlockInfo = QString("基本块数: %1\n最大块大小: %2\n控制流边: %3")
        .arg(result.blockInfo.blockCount)
        .arg(result.blockInfo.maxBlockSize)
        .arg(result.blockInfo.edges);
    codeGenPanel->setBasicBlockInfo(basicBlockInfo);
    
    // 更新统计数据
    codeGenPanel->updateCodeGenStatistics(
        result.statistics.instructionCount,
        result.blockInfo.blockCount,
        result.statistics.temporaryCount
    );
}

