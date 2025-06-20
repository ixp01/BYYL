#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QGroupBox>
#include <QStatusBar>

#include "src/lexer/lexer.h"

class SimpleLexerGUI : public QMainWindow
{
    Q_OBJECT

public:
    SimpleLexerGUI(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        setupUI();
        setupConnections();
    }

private slots:
    void runLexicalAnalysis()
    {
        QString sourceCode = codeEditor->toPlainText();
        if (sourceCode.isEmpty()) {
            QMessageBox::information(this, "提示", "请先输入源代码");
            return;
        }

        try {
            // 进行词法分析（QString转换为std::string）
            Lexer lexer(sourceCode.toStdString());
            LexicalResult result = lexer.analyze();
            
            if (result.hasErrors()) {
                QString errorMsg = "词法分析发现错误:\n";
                for (const auto& error : result.errors) {
                    errorMsg += QString("行%1列%2: %3\n")
                               .arg(error.line)
                               .arg(error.column)
                               .arg(QString::fromStdString(error.message));
                }
                QMessageBox::warning(this, "词法分析错误", errorMsg);
            }
            
            // 显示结果
            displayTokens(result.tokens);
            statusLabel->setText(QString("词法分析完成，生成 %1 个Token").arg(result.tokens.size()));
            
        } catch (const std::exception &e) {
            QMessageBox::warning(this, "错误", QString("词法分析失败: %1").arg(e.what()));
            statusLabel->setText("词法分析失败");
        }
    }

private:
    QPlainTextEdit *codeEditor;
    QTableWidget *tokenTable;
    QPushButton *analyzeButton;
    QLabel *statusLabel;

    void setupUI()
    {
        setWindowTitle("编译器前端 - 词法分析器演示");
        setMinimumSize(1000, 600);
        
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // 创建主布局
        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
        
        // 创建分割器
        QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
        
        // 左侧 - 代码编辑区
        QWidget *leftWidget = new QWidget(this);
        QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
        
        QGroupBox *editorGroup = new QGroupBox("源代码输入", this);
        QVBoxLayout *editorLayout = new QVBoxLayout(editorGroup);
        
        codeEditor = new QPlainTextEdit(this);
        codeEditor->setFont(QFont("Consolas", 10));
        codeEditor->setPlainText(
            "int main() {\n"
            "    int x = 42;\n"
            "    float y = 3.14;\n"
            "    if (x > 0) {\n"
            "        return x + y;\n"
            "    }\n"
            "    return 0;\n"
            "}"
        );
        editorLayout->addWidget(codeEditor);
        
        analyzeButton = new QPushButton("开始词法分析", this);
        analyzeButton->setMinimumHeight(30);
        
        leftLayout->addWidget(editorGroup);
        leftLayout->addWidget(analyzeButton);
        
        // 右侧 - Token表格
        QGroupBox *resultGroup = new QGroupBox("词法分析结果", this);
        QVBoxLayout *resultLayout = new QVBoxLayout(resultGroup);
        
        tokenTable = new QTableWidget(this);
        tokenTable->setColumnCount(5);
        QStringList headers;
        headers << "序号" << "类型" << "值" << "行号" << "列号";
        tokenTable->setHorizontalHeaderLabels(headers);
        tokenTable->setAlternatingRowColors(true);
        tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        
        // 设置列宽
        QHeaderView *header = tokenTable->horizontalHeader();
        header->resizeSection(0, 60);
        header->resizeSection(1, 120);
        header->resizeSection(2, 150);
        header->resizeSection(3, 60);
        header->resizeSection(4, 60);
        header->setStretchLastSection(true);
        
        resultLayout->addWidget(tokenTable);
        
        // 添加到分割器
        splitter->addWidget(leftWidget);
        splitter->addWidget(resultGroup);
        splitter->setSizes({1, 1});
        
        mainLayout->addWidget(splitter);
        
        // 状态栏
        statusLabel = new QLabel("就绪", this);
        statusBar()->addWidget(statusLabel);
    }
    
    void setupConnections()
    {
        connect(analyzeButton, &QPushButton::clicked, this, &SimpleLexerGUI::runLexicalAnalysis);
    }
    
    void displayTokens(const std::vector<Token> &tokens)
    {
        tokenTable->setRowCount(static_cast<int>(tokens.size()));
        
        for (int i = 0; i < static_cast<int>(tokens.size()); ++i) {
            const Token &token = tokens[i];
            
            // 序号
            QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i + 1));
            indexItem->setTextAlignment(Qt::AlignCenter);
            tokenTable->setItem(i, 0, indexItem);
            
            // 类型
            QTableWidgetItem *typeItem = new QTableWidgetItem(QString::fromStdString(token.toString()));
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
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("Compiler Frontend - Lexer Demo");
    app.setApplicationVersion("1.0");
    
    SimpleLexerGUI window;
    window.show();
    
    return app.exec();
}

#include "main_simple.moc" 