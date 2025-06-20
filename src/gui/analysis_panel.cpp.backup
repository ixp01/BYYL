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
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 设置Token表格
    setupTokenTable();
    
    // DFA信息组
    dfaGroupBox = new QGroupBox("DFA 信息", this);
    QVBoxLayout *dfaLayout = new QVBoxLayout(dfaGroupBox);
    
    dfaInfoText = new QTextEdit(this);
    dfaInfoText->setReadOnly(true);
    dfaInfoText->setFont(QFont("Consolas", 9));
    
    minimizedDFAInfoText = new QTextEdit(this);
    minimizedDFAInfoText->setReadOnly(true);
    minimizedDFAInfoText->setFont(QFont("Consolas", 9));
    
    QSplitter *dfaSplitter = new QSplitter(Qt::Vertical, this);
    dfaSplitter->addWidget(dfaInfoText);
    dfaSplitter->addWidget(minimizedDFAInfoText);
    dfaSplitter->setSizes({1, 1});
    
    dfaLayout->addWidget(dfaSplitter);
    
    // 统计信息组
    statisticsGroupBox = new QGroupBox("统计信息", this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statisticsGroupBox);
    
    statisticsLabel = new QLabel("等待分析...", this);
    statisticsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statisticsLabel->setWordWrap(true);
    statsLayout->addWidget(statisticsLabel);
    
    // 组装右侧面板
    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->addWidget(dfaGroupBox, 3);
    rightLayout->addWidget(statisticsGroupBox, 1);
    
    // 添加到主分割器
    mainSplitter->addWidget(tokenGroupBox);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setSizes({2, 1});
    
    mainLayout->addWidget(mainSplitter);
}

void LexicalAnalysisPanel::setupTokenTable()
{
    tokenGroupBox = new QGroupBox("Token 列表", this);
    QVBoxLayout *tokenLayout = new QVBoxLayout(tokenGroupBox);
    
    tokenTable = new QTableWidget(this);
    tokenTable->setColumnCount(5);
    
    QStringList headers;
    headers << "序号" << "类型" << "值" << "行号" << "列号";
    tokenTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    tokenTable->setAlternatingRowColors(true);
    tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokenTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tokenTable->setSortingEnabled(false);
    
    // 设置列宽
    QHeaderView *header = tokenTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(0, 60);  // 序号
    header->resizeSection(1, 120); // 类型
    header->resizeSection(2, 150); // 值
    header->resizeSection(3, 60);  // 行号
    header->resizeSection(4, 60);  // 列号
    
    tokenLayout->addWidget(tokenTable);
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
        
        // 类型
        QTableWidgetItem *typeItem = new QTableWidgetItem(token.toString());
        tokenTable->setItem(i, 1, typeItem);
        
        // 值
        QTableWidgetItem *valueItem = new QTableWidgetItem(token.getValue());
        tokenTable->setItem(i, 2, valueItem);
        
        // 行号
        QTableWidgetItem *lineItem = new QTableWidgetItem(QString::number(token.getLine()));
        lineItem->setTextAlignment(Qt::AlignCenter);
        tokenTable->setItem(i, 3, lineItem);
        
        // 列号
        QTableWidgetItem *columnItem = new QTableWidgetItem(QString::number(token.getColumn()));
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
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // AST树组
    astGroupBox = new QGroupBox("抽象语法树 (AST)", this);
    QVBoxLayout *astLayout = new QVBoxLayout(astGroupBox);
    
    astTree = new QTreeWidget(this);
    astTree->setHeaderLabel("语法树结构");
    astTree->setFont(QFont("Consolas", 9));
    astLayout->addWidget(astTree);
    
    // 右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    
    // 语法分析信息组
    parseInfoGroupBox = new QGroupBox("分析过程", this);
    QVBoxLayout *parseInfoLayout = new QVBoxLayout(parseInfoGroupBox);
    
    parseInfoText = new QTextEdit(this);
    parseInfoText->setReadOnly(true);
    parseInfoText->setFont(QFont("Consolas", 8));
    parseInfoLayout->addWidget(parseInfoText);
    
    // 文法信息组
    grammarGroupBox = new QGroupBox("文法规则", this);
    QVBoxLayout *grammarLayout = new QVBoxLayout(grammarGroupBox);
    
    grammarInfoText = new QTextEdit(this);
    grammarInfoText->setReadOnly(true);
    grammarInfoText->setFont(QFont("Consolas", 8));
    grammarLayout->addWidget(grammarInfoText);
    
    // LALR表组
    lalrTableGroupBox = new QGroupBox("LALR分析表", this);
    QVBoxLayout *lalrLayout = new QVBoxLayout(lalrTableGroupBox);
    
    lalrTableText = new QTextEdit(this);
    lalrTableText->setReadOnly(true);
    lalrTableText->setFont(QFont("Consolas", 8));
    lalrLayout->addWidget(lalrTableText);
    
    // 错误列表组
    errorGroupBox = new QGroupBox("语法错误", this);
    QVBoxLayout *errorLayout = new QVBoxLayout(errorGroupBox);
    
    errorList = new QListWidget(this);
    errorLayout->addWidget(errorList);
    
    // 组装右侧面板
    rightSplitter->addWidget(parseInfoGroupBox);
    rightSplitter->addWidget(grammarGroupBox);
    rightSplitter->addWidget(lalrTableGroupBox);
    rightSplitter->addWidget(errorGroupBox);
    rightSplitter->setSizes({1, 1, 1, 1});
    
    // 添加到主分割器
    mainSplitter->addWidget(astGroupBox);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1});
    
    mainLayout->addWidget(mainSplitter);
}

void SyntaxAnalysisPanel::setAST(const std::shared_ptr<ASTNode> &ast)
{
    astTree->clear();
    if (ast) {
        QTreeWidgetItem *rootItem = new QTreeWidgetItem(astTree);
        rootItem->setText(0, getNodeTypeString(ast->getType()));
        populateASTTree(rootItem, ast);
        astTree->expandAll();
    }
}

void SyntaxAnalysisPanel::clearAST()
{
    astTree->clear();
}

void SyntaxAnalysisPanel::setParseInfo(const QString &parseInfo)
{
    parseInfoText->setText(parseInfo);
}

void SyntaxAnalysisPanel::setGrammarInfo(const QString &grammarInfo)
{
    grammarInfoText->setText(grammarInfo);
}

void SyntaxAnalysisPanel::setLALRTableInfo(const QString &lalrTableInfo)
{
    lalrTableText->setText(lalrTableInfo);
}

void SyntaxAnalysisPanel::addParseError(const QString &error, int line)
{
    QString errorText = error;
    if (line >= 0) {
        errorText = QString("行 %1: %2").arg(line).arg(error);
    }
    errorList->addItem(errorText);
}

void SyntaxAnalysisPanel::clearParseErrors()
{
    errorList->clear();
}

void SyntaxAnalysisPanel::populateASTTree(QTreeWidgetItem *parent, const std::shared_ptr<ASTNode> &node)
{
    if (!node) return;
    
    // 添加节点详细信息
    if (auto expr = std::dynamic_pointer_cast<ExpressionNode>(node)) {
        if (auto binary = std::dynamic_pointer_cast<BinaryOpNode>(expr)) {
            parent->setText(0, QString("%1 (%2)").arg(getNodeTypeString(node->getType())).arg(binary->getOperator()));
        } else if (auto unary = std::dynamic_pointer_cast<UnaryOpNode>(expr)) {
            parent->setText(0, QString("%1 (%2)").arg(getNodeTypeString(node->getType())).arg(unary->getOperator()));
        } else if (auto literal = std::dynamic_pointer_cast<LiteralNode>(expr)) {
            parent->setText(0, QString("%1 (%2)").arg(getNodeTypeString(node->getType())).arg(literal->getValue()));
        } else if (auto id = std::dynamic_pointer_cast<IdentifierNode>(expr)) {
            parent->setText(0, QString("%1 (%2)").arg(getNodeTypeString(node->getType())).arg(id->getName()));
        }
    }
    
    // 递归添加子节点
    for (const auto &child : node->getChildren()) {
        QTreeWidgetItem *childItem = new QTreeWidgetItem(parent);
        childItem->setText(0, getNodeTypeString(child->getType()));
        populateASTTree(childItem, child);
    }
}

QString SyntaxAnalysisPanel::getNodeTypeString(ASTNodeType type)
{
    switch (type) {
        case ASTNodeType::PROGRAM: return "程序";
        case ASTNodeType::FUNCTION_DECL: return "函数声明";
        case ASTNodeType::VARIABLE_DECL: return "变量声明";
        case ASTNodeType::PARAMETER: return "参数";
        case ASTNodeType::COMPOUND_STMT: return "复合语句";
        case ASTNodeType::IF_STMT: return "if语句";
        case ASTNodeType::WHILE_STMT: return "while语句";
        case ASTNodeType::FOR_STMT: return "for语句";
        case ASTNodeType::RETURN_STMT: return "return语句";
        case ASTNodeType::EXPRESSION_STMT: return "表达式语句";
        case ASTNodeType::BINARY_OP: return "二元运算";
        case ASTNodeType::UNARY_OP: return "一元运算";
        case ASTNodeType::FUNCTION_CALL: return "函数调用";
        case ASTNodeType::ARRAY_ACCESS: return "数组访问";
        case ASTNodeType::MEMBER_ACCESS: return "成员访问";
        case ASTNodeType::ASSIGNMENT: return "赋值";
        case ASTNodeType::LITERAL: return "字面量";
        case ASTNodeType::IDENTIFIER: return "标识符";
        default: return "未知";
    }
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
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 符号表组
    symbolTableGroupBox = new QGroupBox("符号表", this);
    QVBoxLayout *symbolLayout = new QVBoxLayout(symbolTableGroupBox);
    
    symbolTableTree = new QTreeWidget(this);
    QStringList symbolHeaders;
    symbolHeaders << "名称" << "类型" << "数据类型" << "作用域" << "行号";
    symbolTableTree->setHeaderLabels(symbolHeaders);
    symbolTableTree->setFont(QFont("Consolas", 9));
    symbolLayout->addWidget(symbolTableTree);
    
    // 右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    
    // 类型检查信息组
    typeCheckGroupBox = new QGroupBox("类型检查", this);
    QVBoxLayout *typeCheckLayout = new QVBoxLayout(typeCheckGroupBox);
    
    typeCheckText = new QTextEdit(this);
    typeCheckText->setReadOnly(true);
    typeCheckText->setFont(QFont("Consolas", 8));
    typeCheckLayout->addWidget(typeCheckText);
    
    // 作用域信息组
    scopeInfoGroupBox = new QGroupBox("作用域分析", this);
    QVBoxLayout *scopeLayout = new QVBoxLayout(scopeInfoGroupBox);
    
    scopeInfoText = new QTextEdit(this);
    scopeInfoText->setReadOnly(true);
    scopeInfoText->setFont(QFont("Consolas", 8));
    scopeLayout->addWidget(scopeInfoText);
    
    // 语义错误列表组
    semanticErrorGroupBox = new QGroupBox("语义错误", this);
    QVBoxLayout *semanticErrorLayout = new QVBoxLayout(semanticErrorGroupBox);
    
    semanticErrorList = new QListWidget(this);
    semanticErrorLayout->addWidget(semanticErrorList);
    
    // 组装右侧面板
    rightSplitter->addWidget(typeCheckGroupBox);
    rightSplitter->addWidget(scopeInfoGroupBox);
    rightSplitter->addWidget(semanticErrorGroupBox);
    rightSplitter->setSizes({1, 1, 1});
    
    // 添加到主分割器
    mainSplitter->addWidget(symbolTableGroupBox);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1});
    
    mainLayout->addWidget(mainSplitter);
}

void SemanticAnalysisPanel::setSymbolTable(const SymbolTable &symbolTable)
{
    symbolTableTree->clear();
    populateSymbolTable(symbolTable);
    symbolTableTree->expandAll();
}

void SemanticAnalysisPanel::clearSymbolTable()
{
    symbolTableTree->clear();
}

void SemanticAnalysisPanel::setTypeCheckInfo(const QString &typeCheckInfo)
{
    typeCheckText->setText(typeCheckInfo);
}

void SemanticAnalysisPanel::setScopeInfo(const QString &scopeInfo)
{
    scopeInfoText->setText(scopeInfo);
}

void SemanticAnalysisPanel::addSemanticError(const QString &error, int line)
{
    QString errorText = error;
    if (line >= 0) {
        errorText = QString("行 %1: %2").arg(line).arg(error);
    }
    semanticErrorList->addItem(errorText);
}

void SemanticAnalysisPanel::clearSemanticErrors()
{
    semanticErrorList->clear();
}

void SemanticAnalysisPanel::populateSymbolTable(const SymbolTable &symbolTable)
{
    // 这里需要根据SymbolTable的实际实现来填充
    // 暂时创建一个示例结构
    QTreeWidgetItem *globalScope = new QTreeWidgetItem(symbolTableTree);
    globalScope->setText(0, "全局作用域");
    globalScope->setText(1, "作用域");
    globalScope->setText(2, "-");
    globalScope->setText(3, "0");
    globalScope->setText(4, "-");
    
    // 这里应该遍历实际的符号表内容
    // 示例代码：
    /*
    for (const auto &symbol : symbolTable.getAllSymbols()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(globalScope);
        item->setText(0, symbol.name);
        item->setText(1, getSymbolTypeString(symbol.type));
        item->setText(2, getDataTypeString(symbol.dataType));
        item->setText(3, QString::number(symbol.scopeLevel));
        item->setText(4, QString::number(symbol.lineNumber));
    }
    */
}

QString SemanticAnalysisPanel::getSymbolTypeString(SymbolType type)
{
    switch (type) {
        case SymbolType::VARIABLE: return "变量";
        case SymbolType::FUNCTION: return "函数";
        case SymbolType::PARAMETER: return "参数";
        case SymbolType::ARRAY: return "数组";
        case SymbolType::STRUCT: return "结构体";
        default: return "未知";
    }
}

QString SemanticAnalysisPanel::getDataTypeString(DataType type)
{
    switch (type) {
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::DOUBLE: return "double";
        case DataType::CHAR: return "char";
        case DataType::VOID: return "void";
        case DataType::BOOL: return "bool";
        case DataType::STRING: return "string";
        case DataType::ARRAY: return "array";
        case DataType::STRUCT: return "struct";
        case DataType::FUNCTION: return "function";
        default: return "未知";
    }
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
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 设置中间代码表格
    setupCodeTable();
    
    // 右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    
    // 基本块信息组
    basicBlockGroupBox = new QGroupBox("基本块信息", this);
    QVBoxLayout *blockLayout = new QVBoxLayout(basicBlockGroupBox);
    
    basicBlockText = new QTextEdit(this);
    basicBlockText->setReadOnly(true);
    basicBlockText->setFont(QFont("Consolas", 8));
    blockLayout->addWidget(basicBlockText);
    
    // 优化信息组
    optimizationGroupBox = new QGroupBox("代码优化", this);
    QVBoxLayout *optimizationLayout = new QVBoxLayout(optimizationGroupBox);
    
    optimizationText = new QTextEdit(this);
    optimizationText->setReadOnly(true);
    optimizationText->setFont(QFont("Consolas", 8));
    optimizationLayout->addWidget(optimizationText);
    
    // 统计信息组
    statisticsGroupBox = new QGroupBox("生成统计", this);
    QVBoxLayout *statsLayout = new QVBoxLayout(statisticsGroupBox);
    
    statisticsLabel = new QLabel("等待代码生成...", this);
    statisticsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    statisticsLabel->setWordWrap(true);
    statsLayout->addWidget(statisticsLabel);
    
    // 组装右侧面板
    rightSplitter->addWidget(basicBlockGroupBox);
    rightSplitter->addWidget(optimizationGroupBox);
    rightSplitter->addWidget(statisticsGroupBox);
    rightSplitter->setSizes({2, 2, 1});
    
    // 添加到主分割器
    mainSplitter->addWidget(codeGroupBox);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1});
    
    mainLayout->addWidget(mainSplitter);
}

void CodeGenerationPanel::setupCodeTable()
{
    codeGroupBox = new QGroupBox("中间代码", this);
    QVBoxLayout *codeLayout = new QVBoxLayout(codeGroupBox);
    
    codeTable = new QTableWidget(this);
    codeTable->setColumnCount(5);
    
    QStringList headers;
    headers << "序号" << "操作" << "操作数1" << "操作数2" << "结果";
    codeTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    codeTable->setAlternatingRowColors(true);
    codeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    codeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    codeTable->setSortingEnabled(false);
    codeTable->setFont(QFont("Consolas", 9));
    
    // 设置列宽
    QHeaderView *header = codeTable->horizontalHeader();
    header->setStretchLastSection(true);
    header->resizeSection(0, 60);  // 序号
    header->resizeSection(1, 80);  // 操作
    header->resizeSection(2, 100); // 操作数1
    header->resizeSection(3, 100); // 操作数2
    header->resizeSection(4, 100); // 结果
    
    codeLayout->addWidget(codeTable);
}

void CodeGenerationPanel::setIntermediateCode(const QVector<ThreeAddressCode> &codes)
{
    codeTable->setRowCount(codes.size());
    
    for (int i = 0; i < codes.size(); ++i) {
        const ThreeAddressCode &code = codes[i];
        
        // 序号
        QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setTextAlignment(Qt::AlignCenter);
        codeTable->setItem(i, 0, indexItem);
        
        // 操作
        QTableWidgetItem *opItem = new QTableWidgetItem(getOpTypeString(code.op));
        codeTable->setItem(i, 1, opItem);
        
        // 操作数1
        QTableWidgetItem *arg1Item = new QTableWidgetItem(getOperandString(code.arg1));
        codeTable->setItem(i, 2, arg1Item);
        
        // 操作数2
        QTableWidgetItem *arg2Item = new QTableWidgetItem(getOperandString(code.arg2));
        codeTable->setItem(i, 3, arg2Item);
        
        // 结果
        QTableWidgetItem *resultItem = new QTableWidgetItem(getOperandString(code.result));
        codeTable->setItem(i, 4, resultItem);
    }
    
    codeTable->resizeRowsToContents();
}

void CodeGenerationPanel::clearIntermediateCode()
{
    codeTable->setRowCount(0);
}

void CodeGenerationPanel::setBasicBlockInfo(const QString &blockInfo)
{
    basicBlockText->setText(blockInfo);
}

void CodeGenerationPanel::setOptimizationInfo(const QString &optimizationInfo)
{
    optimizationText->setText(optimizationInfo);
}

void CodeGenerationPanel::updateCodeGenStatistics(int totalInstructions, int basicBlocks, int tempVars)
{
    QString stats = QString(
        "代码生成统计:\n"
        "• 总指令数: %1\n"
        "• 基本块数: %2\n"
        "• 临时变量数: %3\n"
        "• 平均指令/块: %4\n"
    ).arg(totalInstructions)
     .arg(basicBlocks)
     .arg(tempVars)
     .arg(basicBlocks > 0 ? QString::number(double(totalInstructions) / basicBlocks, 'f', 2) : "0");
    
    statisticsLabel->setText(stats);
}

QString CodeGenerationPanel::getOpTypeString(OpType type)
{
    switch (type) {
        case OpType::ADD: return "ADD";
        case OpType::SUB: return "SUB";
        case OpType::MUL: return "MUL";
        case OpType::DIV: return "DIV";
        case OpType::MOD: return "MOD";
        case OpType::ASSIGN: return "ASSIGN";
        case OpType::LOAD: return "LOAD";
        case OpType::STORE: return "STORE";
        case OpType::JMP: return "JMP";
        case OpType::JZ: return "JZ";
        case OpType::JNZ: return "JNZ";
        case OpType::LABEL: return "LABEL";
        case OpType::CALL: return "CALL";
        case OpType::RET: return "RET";
        case OpType::PUSH: return "PUSH";
        case OpType::POP: return "POP";
        case OpType::CMP: return "CMP";
        case OpType::EQ: return "EQ";
        case OpType::NE: return "NE";
        case OpType::LT: return "LT";
        case OpType::LE: return "LE";
        case OpType::GT: return "GT";
        case OpType::GE: return "GE";
        case OpType::AND: return "AND";
        case OpType::OR: return "OR";
        case OpType::NOT: return "NOT";
        default: return "UNKNOWN";
    }
}

QString CodeGenerationPanel::getOperandString(const Operand &operand)
{
    switch (operand.type) {
        case OperandType::IMMEDIATE:
            return QString::number(operand.value);
        case OperandType::VARIABLE:
            return operand.name;
        case OperandType::TEMPORARY:
            return QString("t%1").arg(operand.tempId);
        case OperandType::LABEL:
            return QString("L%1").arg(operand.labelId);
        case OperandType::NONE:
            return "-";
        default:
            return "?";
    }
}

// ============ AnalysisPanel 实现 ============

AnalysisPanel::AnalysisPanel(QWidget *parent)
    : QTabWidget(parent)
{
    setupTabs();
    connect(this, &QTabWidget::currentChanged, this, &AnalysisPanel::onTabChanged);
}

void AnalysisPanel::setupTabs()
{
    // 创建各个面板
    lexicalPanel = new LexicalAnalysisPanel(this);
    
    // 暂时只实现词法分析面板，其他面板将在后续实现
    syntaxPanel = nullptr;
    semanticPanel = nullptr;
    codeGenPanel = nullptr;
    
    // 添加标签页
    addTab(lexicalPanel, "词法分析");
    addTab(new QWidget(this), "语法分析");
    addTab(new QWidget(this), "语义分析");
    addTab(new QWidget(this), "代码生成");
}

void AnalysisPanel::switchToLexicalTab()
{
    setCurrentIndex(0);
}

void AnalysisPanel::switchToSyntaxTab()
{
    setCurrentIndex(1);
}

void AnalysisPanel::switchToSemanticTab()
{
    setCurrentIndex(2);
}

void AnalysisPanel::switchToCodeGenTab()
{
    setCurrentIndex(3);
}

void AnalysisPanel::clearAllResults()
{
    lexicalPanel->clearTokens();
    // 其他面板的清理将在后续实现
}

void AnalysisPanel::onTabChanged(int index)
{
    emit tabChanged(index);
}

#include "analysis_panel.moc" 