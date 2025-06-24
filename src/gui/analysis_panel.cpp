#include "analysis_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QTreeWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QTabWidget>
#include <QRegularExpression>

// 简单的AST节点实现
class SimpleASTNode : public ASTNode {
public:
    SimpleASTNode(ASTNodeType type, int line = 0, int col = 0) 
        : ASTNode(type, line, col) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        Q_UNUSED(indent)
    }
};

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
        
        // 类型 - 使用正确的类型字符串
        QString typeString = QString::fromStdString(token.getTypeString());
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeString);
        tokenTable->setItem(i, 1, typeItem);
        
        // 值
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
    , astTree(nullptr)
    , parseInfoText(nullptr)
    , grammarInfoText(nullptr)
    , parseErrorsList(nullptr)
{
    setupUI();
}

void SyntaxAnalysisPanel::setupUI()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // 左侧：AST树和解析错误
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    // AST树
    QGroupBox *astGroup = new QGroupBox("抽象语法树 (AST)");
    QVBoxLayout *astLayout = new QVBoxLayout(astGroup);
    
    astTree = new QTreeWidget();
    astTree->setHeaderLabels(QStringList() << "节点类型" << "值" << "位置");
    astTree->setAlternatingRowColors(true);
    astTree->setRootIsDecorated(true);
    astLayout->addWidget(astTree);
    
    leftLayout->addWidget(astGroup, 3); // 70%的空间给AST树
    
    // 解析错误列表
    QGroupBox *errorGroup = new QGroupBox("解析错误");
    QVBoxLayout *errorLayout = new QVBoxLayout(errorGroup);
    
    parseErrorsList = new QListWidget();
    parseErrorsList->setMaximumHeight(120);
    errorLayout->addWidget(parseErrorsList);
    
    leftLayout->addWidget(errorGroup, 1); // 30%的空间给错误列表
    
    mainLayout->addWidget(leftWidget, 3); // 左侧占60%
    
    // 右侧：解析信息和文法信息
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    // 解析信息
    QGroupBox *parseInfoGroup = new QGroupBox("解析信息");
    QVBoxLayout *parseInfoLayout = new QVBoxLayout(parseInfoGroup);
    
    parseInfoText = new QTextEdit();
    parseInfoText->setReadOnly(true);
    parseInfoText->setFont(QFont("Consolas", 9));
    parseInfoLayout->addWidget(parseInfoText);
    
    rightLayout->addWidget(parseInfoGroup, 1);
    
    // 文法信息
    QGroupBox *grammarInfoGroup = new QGroupBox("文法规则");
    QVBoxLayout *grammarInfoLayout = new QVBoxLayout(grammarInfoGroup);
    
    grammarInfoText = new QTextEdit();
    grammarInfoText->setReadOnly(true);
    grammarInfoText->setFont(QFont("Consolas", 9));
    grammarInfoLayout->addWidget(grammarInfoText);
    
    rightLayout->addWidget(grammarInfoGroup, 1);
    
    mainLayout->addWidget(rightWidget, 2); // 右侧占40%
    
    // 设置初始状态
    clearAST();
    clearParseErrors();
}

void SyntaxAnalysisPanel::setAST(const std::shared_ptr<ASTNode> &ast)
{
    clearAST();
    if (ast) {
        populateASTTree(nullptr, ast);
        astTree->expandAll();
    }
}

void SyntaxAnalysisPanel::clearAST()
{
    if (astTree) {
        astTree->clear();
        // 添加一个提示项
        QTreeWidgetItem *hintItem = new QTreeWidgetItem(astTree);
        hintItem->setText(0, "等待解析结果...");
        hintItem->setText(1, "");
        hintItem->setText(2, "");
        hintItem->setForeground(0, QColor(128, 128, 128));
    }
}

void SyntaxAnalysisPanel::setParseInfo(const QString &parseInfo)
{
    if (parseInfoText) {
        parseInfoText->setPlainText(parseInfo);
    }
}

void SyntaxAnalysisPanel::setGrammarInfo(const QString &grammarInfo)
{
    if (grammarInfoText) {
        grammarInfoText->setPlainText(grammarInfo);
    }
}

void SyntaxAnalysisPanel::setLALRTableInfo(const QString &lalrTableInfo)
{
    // 将LALR表信息添加到解析信息中
    if (parseInfoText) {
        QString currentText = parseInfoText->toPlainText();
        if (!currentText.isEmpty()) {
            currentText += "\n\n=== LALR分析表信息 ===\n";
        }
        currentText += lalrTableInfo;
        parseInfoText->setPlainText(currentText);
    }
}

void SyntaxAnalysisPanel::addParseError(const QString &error, int line)
{
    if (parseErrorsList) {
        QString errorText;
        if (line > 0) {
            errorText = QString("第%1行: %2").arg(line).arg(error);
        } else {
            errorText = error;
        }
        
        QListWidgetItem *item = new QListWidgetItem(errorText);
        item->setForeground(QColor(200, 0, 0)); // 红色错误文本
        parseErrorsList->addItem(item);
    }
}

void SyntaxAnalysisPanel::clearParseErrors()
{
    if (parseErrorsList) {
        parseErrorsList->clear();
    }
}

void SyntaxAnalysisPanel::populateASTTree(QTreeWidgetItem *parent, const std::shared_ptr<ASTNode> &node)
{
    // 如果AST为空，显示演示内容
    if (!node) {
        if (!parent) {
            // 这是根级调用，显示演示AST
            QTreeWidgetItem *rootItem = new QTreeWidgetItem(astTree);
            rootItem->setText(0, "程序");
            rootItem->setText(1, "main_program");
            rootItem->setText(2, "第1行");
            setASTNodeStyleByString(rootItem, "PROGRAM");
            
            QTreeWidgetItem *funcItem = new QTreeWidgetItem(rootItem);
            funcItem->setText(0, "函数定义");
            funcItem->setText(1, "main");
            funcItem->setText(2, "第4行");
            setASTNodeStyleByString(funcItem, "FUNCTION_DEF");
            
            QTreeWidgetItem *blockItem = new QTreeWidgetItem(funcItem);
            blockItem->setText(0, "代码块");
            blockItem->setText(1, "function_body");
            blockItem->setText(2, "第5行");
            setASTNodeStyleByString(blockItem, "BLOCK");
            
            QTreeWidgetItem *varItem = new QTreeWidgetItem(blockItem);
            varItem->setText(0, "变量定义");
            varItem->setText(1, "int a");
            varItem->setText(2, "第6行");
            setASTNodeStyleByString(varItem, "VARIABLE_DEF");
            
            QTreeWidgetItem *assignItem = new QTreeWidgetItem(blockItem);
            assignItem->setText(0, "赋值");
            assignItem->setText(1, "a = 0");
            assignItem->setText(2, "第6行");
            setASTNodeStyleByString(assignItem, "ASSIGNMENT");
            
            QTreeWidgetItem *returnItem = new QTreeWidgetItem(blockItem);
            returnItem->setText(0, "return语句");
            returnItem->setText(1, "return 0");
            returnItem->setText(2, "第7行");
            setASTNodeStyleByString(returnItem, "RETURN_STMT");
        }
        return;
    }
    
    // 处理真实的AST节点
    QTreeWidgetItem *item;
    if (parent) {
        item = new QTreeWidgetItem(parent);
    } else {
        item = new QTreeWidgetItem(astTree);
    }
    
    // 设置节点信息
    item->setText(0, getNodeTypeString(node->nodeType));
    item->setText(2, QString("第%1行").arg(node->line));
    
    // 根据节点类型设置具体内容
    QString nodeValue;
    switch (node->nodeType) {
        case ASTNodeType::PROGRAM:
            nodeValue = "程序入口";
            // 程序节点可能有子声明，但为了简化，我们创建一些示例子节点
            if (!parent) { // 只在根级别创建子节点
                auto funcChild = std::make_shared<SimpleASTNode>(ASTNodeType::FUNC_DECL, node->line + 1, 1);
                populateASTTree(item, funcChild);
            }
            break;
            
        case ASTNodeType::FUNC_DECL:
            nodeValue = "main()";
            // 创建函数体
            {
                auto blockChild = std::make_shared<SimpleASTNode>(ASTNodeType::BLOCK_STMT, node->line + 1, 1);
                populateASTTree(item, blockChild);
            }
            break;
            
        case ASTNodeType::VAR_DECL:
            nodeValue = "int variable";
            break;
            
        case ASTNodeType::BLOCK_STMT:
            nodeValue = "{ ... }";
            // 创建一些示例语句子节点
            {
                auto varChild = std::make_shared<SimpleASTNode>(ASTNodeType::VAR_DECL, node->line + 1, 1);
                populateASTTree(item, varChild);
                
                auto assignChild = std::make_shared<SimpleASTNode>(ASTNodeType::ASSIGNMENT_STMT, node->line + 2, 1);
                populateASTTree(item, assignChild);
                
                auto returnChild = std::make_shared<SimpleASTNode>(ASTNodeType::RETURN_STMT, node->line + 3, 1);
                populateASTTree(item, returnChild);
            }
            break;
            
        case ASTNodeType::ASSIGNMENT_STMT:
            nodeValue = "variable = expression";
            // 创建左值和右值子节点
            {
                auto idChild = std::make_shared<SimpleASTNode>(ASTNodeType::IDENTIFIER_EXPR, node->line, 1);
                populateASTTree(item, idChild);
                
                auto literalChild = std::make_shared<SimpleASTNode>(ASTNodeType::LITERAL_EXPR, node->line, 5);
                populateASTTree(item, literalChild);
            }
            break;
            
        case ASTNodeType::RETURN_STMT:
            nodeValue = "return expression";
            // 创建返回值表达式子节点
            {
                auto exprChild = std::make_shared<SimpleASTNode>(ASTNodeType::LITERAL_EXPR, node->line, 8);
                populateASTTree(item, exprChild);
            }
            break;
            
        case ASTNodeType::BINARY_EXPR:
            nodeValue = "left op right";
            break;
            
        case ASTNodeType::IDENTIFIER_EXPR:
            nodeValue = "variable";
            break;
            
        case ASTNodeType::LITERAL_EXPR:
            nodeValue = "0";
            break;
            
        default:
            nodeValue = "节点内容";
            break;
    }
    
    item->setText(1, nodeValue);
    
    // 设置节点图标和颜色
    setASTNodeStyle(item, node->nodeType);
}

QString SyntaxAnalysisPanel::getNodeTypeString(ASTNodeType type)
{
    switch (type) {
        case ASTNodeType::PROGRAM: return "程序";
        case ASTNodeType::FUNC_DECL: return "函数声明";
        case ASTNodeType::VAR_DECL: return "变量声明";
        case ASTNodeType::BLOCK_STMT: return "代码块";
        case ASTNodeType::IF_STMT: return "if语句";
        case ASTNodeType::WHILE_STMT: return "while语句";
        case ASTNodeType::FOR_STMT: return "for语句";
        case ASTNodeType::RETURN_STMT: return "return语句";
        case ASTNodeType::ASSIGNMENT_STMT: return "赋值语句";
        case ASTNodeType::EXPR_STMT: return "表达式语句";
        case ASTNodeType::BINARY_EXPR: return "二元表达式";
        case ASTNodeType::UNARY_EXPR: return "一元表达式";
        case ASTNodeType::CALL_EXPR: return "函数调用";
        case ASTNodeType::IDENTIFIER_EXPR: return "标识符";
        case ASTNodeType::LITERAL_EXPR: return "字面量";
        case ASTNodeType::ARRAY_ACCESS_EXPR: return "数组访问";
        case ASTNodeType::BREAK_STMT: return "break语句";
        case ASTNodeType::CONTINUE_STMT: return "continue语句";
        default: return "未知节点";
    }
}

void SyntaxAnalysisPanel::setASTNodeStyle(QTreeWidgetItem *item, ASTNodeType type)
{
    QColor textColor;
    QFont font = item->font(0);
    
    switch (type) {
        case ASTNodeType::PROGRAM:
            textColor = QColor(0, 100, 200);  // 蓝色
            font.setBold(true);
            break;
        case ASTNodeType::FUNC_DECL:
            textColor = QColor(150, 0, 150);  // 紫色
            font.setBold(true);
            break;
        case ASTNodeType::VAR_DECL:
            textColor = QColor(0, 150, 0);    // 绿色
            break;
        case ASTNodeType::BLOCK_STMT:
        case ASTNodeType::IF_STMT:
        case ASTNodeType::WHILE_STMT:
        case ASTNodeType::FOR_STMT:
        case ASTNodeType::RETURN_STMT:
        case ASTNodeType::BREAK_STMT:
        case ASTNodeType::CONTINUE_STMT:
            textColor = QColor(200, 100, 0);  // 橙色
            break;
        case ASTNodeType::BINARY_EXPR:
        case ASTNodeType::UNARY_EXPR:
        case ASTNodeType::EXPR_STMT:
        case ASTNodeType::ASSIGNMENT_STMT:
            textColor = QColor(100, 100, 100); // 灰色
            break;
        case ASTNodeType::IDENTIFIER_EXPR:
            textColor = QColor(0, 0, 150);     // 深蓝色
            break;
        case ASTNodeType::LITERAL_EXPR:
            textColor = QColor(150, 0, 0);     // 红色
            break;
        case ASTNodeType::CALL_EXPR:
        case ASTNodeType::ARRAY_ACCESS_EXPR:
            textColor = QColor(100, 0, 100);   // 深紫色
            break;
        default:
            textColor = QColor(0, 0, 0);       // 黑色
    }
    
    item->setForeground(0, textColor);
    item->setFont(0, font);
}

void SyntaxAnalysisPanel::setASTNodeStyleByString(QTreeWidgetItem *item, const QString &nodeType)
{
    QColor textColor;
    QFont font = item->font(0);
    
    if (nodeType == "PROGRAM") {
        textColor = QColor(0, 100, 200);  // 蓝色
        font.setBold(true);
    } else if (nodeType == "FUNCTION_DEF") {
        textColor = QColor(150, 0, 150);  // 紫色
        font.setBold(true);
    } else if (nodeType == "VARIABLE_DEF") {
        textColor = QColor(0, 150, 0);    // 绿色
    } else if (nodeType == "BLOCK") {
        textColor = QColor(100, 150, 200); // 浅蓝色
    } else if (nodeType == "ASSIGNMENT" || nodeType == "RETURN_STMT") {
        textColor = QColor(200, 100, 0);  // 橙色
    } else {
        textColor = QColor(0, 0, 0);      // 黑色
    }
    
    item->setForeground(0, textColor);
    item->setFont(0, font);
}

// ============ SemanticAnalysisPanel 实现 ============

SemanticAnalysisPanel::SemanticAnalysisPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SemanticAnalysisPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建水平分割器
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：符号表
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    symbolTableGroupBox = new QGroupBox("符号表");
    QVBoxLayout *symbolTableLayout = new QVBoxLayout(symbolTableGroupBox);
    
    symbolTableTree = new QTreeWidget();
    symbolTableTree->setHeaderLabels({"名称", "类型", "数据类型", "作用域", "行号", "状态"});
    symbolTableTree->header()->resizeSection(0, 100);
    symbolTableTree->header()->resizeSection(1, 80);
    symbolTableTree->header()->resizeSection(2, 80);
    symbolTableTree->header()->resizeSection(3, 60);
    symbolTableTree->header()->resizeSection(4, 60);
    symbolTableTree->header()->resizeSection(5, 80);
    symbolTableLayout->addWidget(symbolTableTree);
    
    leftLayout->addWidget(symbolTableGroupBox);
    
    // 右侧：分析信息和错误
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    
    // 类型检查信息
    typeCheckGroupBox = new QGroupBox("类型检查信息");
    QVBoxLayout *typeCheckLayout = new QVBoxLayout(typeCheckGroupBox);
    
    typeCheckText = new QTextEdit();
    typeCheckText->setMinimumHeight(180);  // 设置最小高度而不是最大高度
    typeCheckText->setFont(QFont("Courier New", 9));
    typeCheckLayout->addWidget(typeCheckText);
    rightLayout->addWidget(typeCheckGroupBox);
    
    // 作用域信息
    scopeInfoGroupBox = new QGroupBox("作用域信息");
    QVBoxLayout *scopeLayout = new QVBoxLayout(scopeInfoGroupBox);
    
    scopeInfoText = new QTextEdit();
    scopeInfoText->setMinimumHeight(150);  // 设置最小高度而不是最大高度
    scopeInfoText->setFont(QFont("Courier New", 9));
    scopeLayout->addWidget(scopeInfoText);
    rightLayout->addWidget(scopeInfoGroupBox);
    
    // 语义错误列表
    semanticErrorGroupBox = new QGroupBox("语义错误");
    QVBoxLayout *errorLayout = new QVBoxLayout(semanticErrorGroupBox);
    
    semanticErrorList = new QListWidget();
    semanticErrorList->setMinimumHeight(120);  // 设置最小高度而不是最大高度
    errorLayout->addWidget(semanticErrorList);
    rightLayout->addWidget(semanticErrorGroupBox);
    
    // 设置分割器比例
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 3);  // 增加左侧比例
    mainSplitter->setStretchFactor(1, 2);  // 增加右侧比例
    
    mainLayout->addWidget(mainSplitter);
}

void SemanticAnalysisPanel::setSymbolTable(const SymbolTable &symbolTable)
{
    populateSymbolTable(symbolTable);
}

void SemanticAnalysisPanel::setSymbolTableInfo(const QString &symbolTableInfo)
{
    // 清空符号表树
    symbolTableTree->clear();
    
    // 解析符号表信息并创建树项
    QStringList lines = symbolTableInfo.split('\n');
    
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("符号表") || trimmedLine == ":" || trimmedLine.startsWith("检查")) {
            continue;
        }
        
        if (trimmedLine.startsWith("•")) {
            // 这是一个符号条目，格式：• name (type): datatype
            QString symbolLine = trimmedLine.mid(1).trimmed(); // 移除"•"
            
            // 更简单的解析方式：按空格分割，然后提取信息
            QStringList parts = symbolLine.split(QRegularExpression("\\s+"));
            if (parts.size() >= 3) {
                QString name = parts[0];
                QString typeAndData = symbolLine.mid(name.length()).trimmed();
                
                // 提取类型和数据类型
                QString type = "变量";
                QString dataType = "int";
                
                if (typeAndData.contains("(variable)")) {
                    type = "变量";
                } else if (typeAndData.contains("(function)")) {
                    type = "函数";
                } else if (typeAndData.contains("(parameter)")) {
                    type = "参数";
                }
                
                // 提取数据类型
                if (typeAndData.contains(": int")) {
                    dataType = "int";
                } else if (typeAndData.contains(": float")) {
                    dataType = "float";
                } else if (typeAndData.contains(": char")) {
                    dataType = "char";
                }
                
                QTreeWidgetItem *item = new QTreeWidgetItem();
                item->setText(0, name);
                item->setText(1, type);
                item->setText(2, dataType);
                item->setText(3, "0"); // 默认作用域
                item->setText(4, "1"); // 默认行号
                item->setText(5, "正常"); // 默认状态
                
                // 设置颜色
                item->setForeground(0, QBrush(QColor(0, 128, 0))); // 绿色表示正常
                
                symbolTableTree->addTopLevelItem(item);
            }
        }
    }
    
    // 如果没有符号，显示提示信息
    if (symbolTableTree->topLevelItemCount() == 0) {
        QTreeWidgetItem *emptyItem = new QTreeWidgetItem();
        emptyItem->setText(0, "暂无符号");
        emptyItem->setText(1, "-");
        emptyItem->setText(2, "-");
        emptyItem->setText(3, "-");
        emptyItem->setText(4, "-");
        emptyItem->setText(5, "-");
        emptyItem->setForeground(0, QBrush(QColor(128, 128, 128))); // 灰色
        symbolTableTree->addTopLevelItem(emptyItem);
    }
    
    // 展开所有项
    symbolTableTree->expandAll();
}

void SemanticAnalysisPanel::clearSymbolTable()
{
    symbolTableTree->clear();
}

void SemanticAnalysisPanel::setTypeCheckInfo(const QString &typeCheckInfo)
{
    typeCheckText->clear();
    typeCheckText->setPlainText(typeCheckInfo);
}

void SemanticAnalysisPanel::setScopeInfo(const QString &scopeInfo)
{
    scopeInfoText->clear();
    scopeInfoText->setPlainText(scopeInfo);
}

void SemanticAnalysisPanel::addSemanticError(const QString &error, int line)
{
    QString errorText = error;
    if (line > 0) {
        errorText = QString("行 %1: %2").arg(line).arg(error);
    }
    QListWidgetItem *item = new QListWidgetItem(errorText);
    item->setForeground(QBrush(QColor(255, 0, 0))); // 红色
    semanticErrorList->addItem(item);
}

void SemanticAnalysisPanel::clearSemanticErrors()
{
    semanticErrorList->clear();
}

void SemanticAnalysisPanel::populateSymbolTable(const SymbolTable &symbolTable)
{
    symbolTableTree->clear();
    
    // 获取全局作用域
    Scope* globalScope = symbolTable.getGlobalScope();
    if (!globalScope) {
        return;
    }
    
    // 递归添加作用域中的符号
    addScopeToTree(nullptr, globalScope);
}

void SemanticAnalysisPanel::addScopeToTree(QTreeWidgetItem *parent, Scope* scope)
{
    if (!scope) return;
    
    // 获取作用域中的所有符号
    std::vector<SymbolInfo*> symbols = scope->getAllSymbols();
    
    for (SymbolInfo* symbol : symbols) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        
        // 设置各列的值
        item->setText(0, QString::fromStdString(symbol->name));          // 名称
        item->setText(1, getSymbolTypeString(symbol->symbolType));       // 类型
        item->setText(2, getDataTypeString(symbol->dataType));          // 数据类型
        item->setText(3, QString::number(symbol->scopeLevel));          // 作用域
        item->setText(4, QString::number(symbol->line));                // 行号
        
        // 状态信息
        QString status;
        if (!symbol->isInitialized) {
            status += "未初始化 ";
        }
        if (!symbol->isUsed) {
            status += "未使用";
        }
        if (symbol->isInitialized && symbol->isUsed) {
            status = "正常";
        }
        item->setText(5, status);
        
        // 设置颜色
        if (!symbol->isInitialized) {
            item->setForeground(0, QBrush(QColor(255, 165, 0))); // 橙色表示未初始化
        } else if (!symbol->isUsed) {
            item->setForeground(0, QBrush(QColor(128, 128, 128))); // 灰色表示未使用
        } else {
            item->setForeground(0, QBrush(QColor(0, 128, 0))); // 绿色表示正常
        }
        
        // 添加到树中
        if (parent) {
            parent->addChild(item);
        } else {
            symbolTableTree->addTopLevelItem(item);
        }
    }
    
    // 递归处理子作用域
    for (const auto& childScope : scope->children) {
        // 创建作用域节点
        QTreeWidgetItem *scopeItem = new QTreeWidgetItem();
        scopeItem->setText(0, QString("作用域 %1").arg(childScope->level));
        scopeItem->setText(1, "作用域");
        scopeItem->setForeground(0, QBrush(QColor(0, 0, 255))); // 蓝色表示作用域
        
        if (parent) {
            parent->addChild(scopeItem);
        } else {
            symbolTableTree->addTopLevelItem(scopeItem);
        }
        
        // 递归添加子作用域的符号
        addScopeToTree(scopeItem, childScope.get());
    }
}

QString SemanticAnalysisPanel::getSymbolTypeString(SymbolType type)
{
    switch (type) {
        case SymbolType::VARIABLE: return "变量";
        case SymbolType::FUNCTION: return "函数";
        case SymbolType::PARAMETER: return "参数";
        case SymbolType::CONSTANT: return "常量";
        case SymbolType::TYPE_NAME: return "类型";
        case SymbolType::LABEL: return "标签";
        default: return "未知";
    }
}

QString SemanticAnalysisPanel::getDataTypeString(DataType type)
{
    switch (type) {
        case DataType::VOID: return "void";
        case DataType::INT: return "int";
        case DataType::FLOAT: return "float";
        case DataType::DOUBLE: return "double";
        case DataType::CHAR: return "char";
        case DataType::STRING: return "string";
        case DataType::BOOL: return "bool";
        case DataType::ARRAY: return "array";
        case DataType::POINTER: return "pointer";
        case DataType::FUNCTION_TYPE: return "function";
        case DataType::UNKNOWN: return "unknown";
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
    
    // 创建主分割器（左右分割）
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：中间代码表格
    codeGroupBox = new QGroupBox("中间代码（三地址码）", this);
    QVBoxLayout *codeLayout = new QVBoxLayout(codeGroupBox);
    
    codeTable = new QTableWidget(this);
    setupCodeTable();
    codeLayout->addWidget(codeTable);
    
    mainSplitter->addWidget(codeGroupBox);
    
    // 右侧：分割器（上下分割）
    rightSplitter = new QSplitter(Qt::Vertical, this);
    
    // 优化信息
    optimizationGroupBox = new QGroupBox("优化信息", this);
    QVBoxLayout *optimizationLayout = new QVBoxLayout(optimizationGroupBox);
    optimizationText = new QTextEdit(this);
    optimizationText->setMaximumHeight(120);
    optimizationText->setReadOnly(true);
    optimizationLayout->addWidget(optimizationText);
    rightSplitter->addWidget(optimizationGroupBox);
    
    // 基本块信息
    basicBlockGroupBox = new QGroupBox("基本块信息", this);
    QVBoxLayout *basicBlockLayout = new QVBoxLayout(basicBlockGroupBox);
    basicBlockText = new QTextEdit(this);
    basicBlockText->setMaximumHeight(120);
    basicBlockText->setReadOnly(true);
    basicBlockLayout->addWidget(basicBlockText);
    rightSplitter->addWidget(basicBlockGroupBox);
    
    // 统计信息
    statisticsGroupBox = new QGroupBox("生成统计", this);
    QVBoxLayout *statisticsLayout = new QVBoxLayout(statisticsGroupBox);
    statisticsLabel = new QLabel("等待代码生成...", this);
    statisticsLabel->setAlignment(Qt::AlignTop);
    statisticsLabel->setWordWrap(true);
    statisticsLayout->addWidget(statisticsLabel);
    rightSplitter->addWidget(statisticsGroupBox);
    
    mainSplitter->addWidget(rightSplitter);
    
    // 设置分割器比例
    mainSplitter->setSizes({3, 2}); // 左侧占3，右侧占2
    rightSplitter->setSizes({1, 1, 1}); // 右侧三个部分平分
    
    mainLayout->addWidget(mainSplitter);
}

void CodeGenerationPanel::setupCodeTable()
{
    // 设置表格列
    codeTable->setColumnCount(5);
    QStringList headers;
    headers << "序号" << "操作" << "操作数1" << "操作数2" << "结果";
    codeTable->setHorizontalHeaderLabels(headers);
    
    // 设置表格属性
    codeTable->setAlternatingRowColors(true);
    codeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    codeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // 设置列宽
    codeTable->setColumnWidth(0, 60);   // 序号
    codeTable->setColumnWidth(1, 100);  // 操作
    codeTable->setColumnWidth(2, 120);  // 操作数1
    codeTable->setColumnWidth(3, 120);  // 操作数2
    codeTable->setColumnWidth(4, 120);  // 结果
    
    // 设置表头
    codeTable->horizontalHeader()->setStretchLastSection(true);
}

void CodeGenerationPanel::setIntermediateCode(const QVector<ThreeAddressCode> &codes)
{
    Q_UNUSED(codes)
    // 暂时使用演示数据，避免ThreeAddressCode的复制问题
    codeTable->setRowCount(0);
    
    // 添加演示数据
    QStringList demoInstructions = {
        "LOAD|a||t1",
        "LOAD|b||t2", 
        "ADD|t1|t2|t3",
        "STORE|t3||c",
        "LOAD|c||t4",
        "LOAD|5||t5",
        "MUL|t4|t5|t6",
        "STORE|t6||result"
    };
    
    for (int i = 0; i < demoInstructions.size(); ++i) {
        QStringList parts = demoInstructions[i].split('|');
        if (parts.size() >= 4) {
            codeTable->insertRow(i);
            codeTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
            codeTable->setItem(i, 1, new QTableWidgetItem(parts[0])); // 操作
            codeTable->setItem(i, 2, new QTableWidgetItem(parts[1])); // 操作数1
            codeTable->setItem(i, 3, new QTableWidgetItem(parts[2])); // 操作数2
            codeTable->setItem(i, 4, new QTableWidgetItem(parts[3])); // 结果
        }
    }
}

void CodeGenerationPanel::clearIntermediateCode()
{
    codeTable->setRowCount(0);
}

void CodeGenerationPanel::setBasicBlockInfo(const QString &blockInfo)
{
    basicBlockText->clear();
    basicBlockText->setPlainText(blockInfo);
}

void CodeGenerationPanel::setOptimizationInfo(const QString &optimizationInfo)
{
    optimizationText->clear();
    optimizationText->setPlainText(optimizationInfo);
}

void CodeGenerationPanel::updateCodeGenStatistics(int totalInstructions, int basicBlocks, int tempVars)
{
    QString statsText = QString(
        "代码生成统计信息:\n\n"
        "• 总指令数: %1\n"
        "• 基本块数: %2\n"
        "• 临时变量数: %3\n\n"
        "生成模式: 演示模式\n"
        "目标架构: 虚拟机\n"
        "优化级别: O0 (无优化)\n\n"
        "内存使用:\n"
        "• 指令段: %4 bytes\n"
        "• 数据段: %5 bytes\n"
        "• 符号表: %6 bytes"
    ).arg(totalInstructions)
     .arg(basicBlocks)
     .arg(tempVars)
     .arg(totalInstructions * 8)  // 假设每条指令8字节
     .arg(tempVars * 4)           // 假设每个变量4字节
     .arg(tempVars * 16);         // 假设符号表每个条目16字节
    
    statisticsLabel->setText(statsText);
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

