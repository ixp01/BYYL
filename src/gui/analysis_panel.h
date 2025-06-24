#ifndef ANALYSIS_PANEL_H
#define ANALYSIS_PANEL_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSplitter>
#include <QGroupBox>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>

// 前向声明
#include "../lexer/token.h"
#include "../parser/ast.h"
#include "../semantic/symbol_table.h"
#include "../codegen/intermediate_code.h"

// GUI专用分析结果结构体定义（避免与核心模块冲突）
struct LexicalResultGUI {
    std::vector<Token> tokens;
    int originalDfaStates;
    int originalAcceptStates;
    int originalTransitions;
    int minimizedDfaStates;
    int minimizedAcceptStates;
    int minimizedTransitions;
    double compressionRatio;
    double analysisTime;
    size_t memoryUsage;
};

struct ASTInfo {
    std::string rootNodeType;
    int nodeCount;
    int depth;
};

struct ParseInfo {
    int reductions;
    int shifts;
    int conflicts;
};

struct GrammarInfo {
    int productions;
    int nonterminals;
    int terminals;
};

struct SyntaxResult {
    bool success;
    std::string error;
    ASTInfo astInfo;
    ParseInfo parseInfo;
    GrammarInfo grammarInfo;
};

struct SymbolInfoGUI {
    std::string name;
    std::string type;
    int scope;
    int line;
};

struct ScopeInfo {
    int maxDepth;
    int currentLevel;
};

struct SemanticErrorGUI {
    int line;
    std::string message;
};

struct SemanticResultGUI {
    bool success;
    std::vector<SymbolInfoGUI> symbols;
    ScopeInfo scopeInfo;
    std::vector<SemanticErrorGUI> errors;
};

struct OptimizationInfo {
    int passes;
    int instructionsOptimized;
    int constantsFolded;
};

struct BlockInfo {
    int blockCount;
    int maxBlockSize;
    int edges;
};

struct CodeGenStatistics {
    int instructionCount;
    int temporaryCount;
    double generationTime;
};

struct CodeGenResultGUI {
    std::string intermediateCode;
    OptimizationInfo optimizationInfo;
    BlockInfo blockInfo;
    CodeGenStatistics statistics;
};

/**
 * @brief 词法分析结果面板
 */
class LexicalAnalysisPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LexicalAnalysisPanel(QWidget *parent = nullptr);
    
    // 设置Token列表
    void setTokens(const QVector<Token> &tokens);
    void clearTokens();
    
    // 设置DFA信息
    void setDFAInfo(const QString &dfaInfo);
    void setMinimizedDFAInfo(const QString &minimizedDFAInfo);
    
    // 更新统计信息
    void updateStatistics(int totalTokens, int totalLines, int totalChars);

private:
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
    // Token表格
    QTableWidget *tokenTable;
    QGroupBox *tokenGroupBox;
    
    // DFA信息
    QTextEdit *dfaInfoText;
    QGroupBox *dfaGroupBox;
    
    // 最小化DFA信息
    QTextEdit *minimizedDFAInfoText;
    QGroupBox *minimizedDfaGroupBox;
    
    // 统计信息
    QLabel *statisticsLabel;
    QGroupBox *statisticsGroupBox;
    
    void setupUI();
    void setupTokenTable();
};

/**
 * @brief 语法分析结果面板
 */
class SyntaxAnalysisPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SyntaxAnalysisPanel(QWidget *parent = nullptr);
    
    // 设置AST树
    void setAST(const std::shared_ptr<ASTNode> &ast);
    void clearAST();
    
    // 设置语法分析信息
    void setParseInfo(const QString &parseInfo);
    void setGrammarInfo(const QString &grammarInfo);
    void setLALRTableInfo(const QString &lalrTableInfo);
    
    // 错误信息
    void addParseError(const QString &error, int line = -1);
    void clearParseErrors();

private:
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
    // AST树显示
    QTreeWidget *astTree;
    QGroupBox *astGroupBox;
    
    // 解析信息
    QTextEdit *parseInfoText;
    QGroupBox *parseInfoGroupBox;
    
    // 文法信息
    QTextEdit *grammarInfoText;
    QGroupBox *grammarGroupBox;
    
    // LALR分析表
    QTextEdit *lalrTableText;
    QGroupBox *lalrTableGroupBox;
    
    // 解析错误列表
    QListWidget *parseErrorsList;
    QGroupBox *parseErrorsGroupBox;
    
    void setupUI();
    void populateASTTree(QTreeWidgetItem *parent, const std::shared_ptr<ASTNode> &node);
    QString getNodeTypeString(ASTNodeType type);
    void setASTNodeStyle(QTreeWidgetItem *item, ASTNodeType type);
    void setASTNodeStyleByString(QTreeWidgetItem *item, const QString &nodeType);
};

/**
 * @brief 语义分析结果面板
 */
class SemanticAnalysisPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SemanticAnalysisPanel(QWidget *parent = nullptr);
    
    // 设置符号表
    void setSymbolTable(const SymbolTable &symbolTable);
    void setSymbolTableInfo(const QString &symbolTableInfo);
    void clearSymbolTable();
    
    // 设置类型检查信息
    void setTypeCheckInfo(const QString &typeCheckInfo);
    
    // 作用域信息
    void setScopeInfo(const QString &scopeInfo);
    
    // 语义错误
    void addSemanticError(const QString &error, int line = -1);
    void clearSemanticErrors();

private:
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
    // 符号表显示
    QTreeWidget *symbolTableTree;
    QGroupBox *symbolTableGroupBox;
    
    // 类型检查信息
    QTextEdit *typeCheckText;
    QGroupBox *typeCheckGroupBox;
    
    // 作用域信息
    QTextEdit *scopeInfoText;
    QGroupBox *scopeInfoGroupBox;
    
    // 语义错误列表
    QListWidget *semanticErrorList;
    QGroupBox *semanticErrorGroupBox;
    
    void setupUI();
    void populateSymbolTable(const SymbolTable &symbolTable);
    void addScopeToTree(QTreeWidgetItem *parent, Scope* scope);
    QString getSymbolTypeString(SymbolType type);
    QString getDataTypeString(DataType type);
};

/**
 * @brief 代码生成结果面板
 */
class CodeGenerationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CodeGenerationPanel(QWidget *parent = nullptr);
    
    // 设置中间代码
    void setIntermediateCode(const QVector<ThreeAddressCode> &codes);
    void clearIntermediateCode();
    
    // 设置基本块信息
    void setBasicBlockInfo(const QString &blockInfo);
    
    // 设置优化信息
    void setOptimizationInfo(const QString &optimizationInfo);
    
    // 设置生成统计
    void updateCodeGenStatistics(int totalInstructions, int basicBlocks, int tempVars);

private:
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
    // 中间代码表格
    QTableWidget *codeTable;
    QGroupBox *codeGroupBox;
    
    // 基本块信息
    QTextEdit *basicBlockText;
    QGroupBox *basicBlockGroupBox;
    
    // 优化信息
    QTextEdit *optimizationText;
    QGroupBox *optimizationGroupBox;
    
    // 统计信息
    QLabel *statisticsLabel;
    QGroupBox *statisticsGroupBox;
    
    void setupUI();
    void setupCodeTable();
    QString getOpTypeString(OpType type);
    QString getOperandString(const Operand &operand);
};

/**
 * @brief 分析结果主面板
 */
class AnalysisPanel : public QTabWidget
{
    Q_OBJECT

public:
    explicit AnalysisPanel(QWidget *parent = nullptr);
    
    // 获取各个子面板
    LexicalAnalysisPanel* getLexicalPanel() const { return lexicalPanel; }
    SyntaxAnalysisPanel* getSyntaxPanel() const { return syntaxPanel; }
    SemanticAnalysisPanel* getSemanticPanel() const { return semanticPanel; }
    CodeGenerationPanel* getCodeGenPanel() const { return codeGenPanel; }
    
    // 便捷方法
    void switchToLexicalTab();
    void switchToSyntaxTab();
    void switchToSemanticTab();
    void switchToCodeGenTab();
    
    // 清除所有结果
    void clearAllResults();
    
    // 数据绑定方法
    void updateLexicalAnalysisResult(const LexicalResultGUI& result);
    void updateSyntaxAnalysisResult(const SyntaxResult& result);
    void updateSemanticAnalysisResult(const SemanticResultGUI& result);
    void updateCodeGenerationResult(const CodeGenResultGUI& result);

signals:
    void tabChanged(int index);

private slots:
    void onTabChanged(int index);

private:
    LexicalAnalysisPanel *lexicalPanel;
    SyntaxAnalysisPanel *syntaxPanel;
    SemanticAnalysisPanel *semanticPanel;
    CodeGenerationPanel *codeGenPanel;
    
    void setupTabs();
};

#endif // ANALYSIS_PANEL_H 
