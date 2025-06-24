#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QProgressBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QTextEdit>
#include <QListWidget>
#include <QDockWidget>

// Qt6 兼容性
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

#include "code_editor.h"
#include "analysis_panel.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"
#include "../semantic/semantic_analyzer.h"
#include "../semantic/symbol_table.h"
#include "../codegen/code_generator.h"
#include "../codegen/intermediate_code.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// 前向声明
class AnalysisPanel;

// 简单的AST节点实现（具体类，可以实例化）
class SimpleASTNode : public ASTNode {
public:
    SimpleASTNode(ASTNodeType type, int line = 0, int col = 0) 
        : ASTNode(type, line, col) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        // 简化实现，不依赖iostream
        Q_UNUSED(indent)
    }
};

/**
 * @brief 编译分析线程
 */
class AnalysisThread : public QThread
{
    Q_OBJECT

public:
    enum class AnalysisType {
        LEXICAL_ONLY,
        SYNTAX_ONLY, 
        SEMANTIC_ONLY,
        CODEGEN_ONLY,
        FULL_ANALYSIS
    };

    explicit AnalysisThread(QObject *parent = nullptr);
    void setSourceCode(const QString &code);
    void setAnalysisType(AnalysisType type);

protected:
    void run() override;

signals:
    void lexicalAnalysisFinished(const QVector<Token> &tokens);
    void lexicalAnalysisFinishedWithDFA(const QVector<Token> &tokens, 
                                       size_t originalStates, size_t minimizedStates,
                                       double compressionRatio);
    void syntaxAnalysisFinished(bool success, const QString &message, 
                               const std::shared_ptr<ASTNode> &ast,
                               const QString &parseInfo,
                               const QString &grammarInfo);
    void semanticAnalysisFinished(bool success, const QString &message,
                                 const QString &symbolTableInfo,
                                 const QString &typeCheckInfo,
                                 const QString &scopeInfo,
                                 const QVector<SemanticError> &errors);
    void codeGenerationFinished(bool success, const QString &message,
                               const QVector<ThreeAddressCode> &codes,
                               const QString &optimizationInfo,
                               const QString &basicBlockInfo,
                               int totalInstructions, int basicBlocks, int tempVars);
    void analysisError(const QString &error);
    void analysisProgress(const QString &stage);

private:
    QString m_sourceCode;
    QMutex m_mutex;
    AnalysisType m_analysisType;
    
    // 辅助方法
    std::shared_ptr<ASTNode> createSimpleParser();
    std::shared_ptr<ASTNode> parseTokensToAST(const std::vector<Token>& tokens);
    int countASTNodes(ASTNode* node);
    int getASTDepth(ASTNode* node);
};

/**
 * @brief 主窗口类
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void tabChanged(int index);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void recentFileTriggered();
    
    // 编辑操作
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void selectAll();
    void find();
    void replace();
    void gotoLine();
    
    // 分析操作
    void runLexicalAnalysis();
    void runSyntaxAnalysis();
    void runSemanticAnalysis();
    void runCodeGeneration();
    void runFullAnalysis();
    void stopAnalysis();
    
    // 视图操作
    void toggleLineNumbers();
    void toggleSyntaxHighlighting();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    
    // 代码编辑器信号
    void onTextChanged();
    void onModificationChanged(bool changed);
    void onCursorPositionChanged(int line, int column);
    void onFileNameChanged(const QString &fileName);
    
    // 分析结果信号
    void onLexicalAnalysisFinished(const QVector<Token> &tokens);
    void onLexicalAnalysisFinishedWithDFA(const QVector<Token> &tokens, 
                                         size_t originalStates, size_t minimizedStates,
                                         double compressionRatio);
    void onSyntaxAnalysisFinished(bool success, const QString &message, 
                                 const std::shared_ptr<ASTNode> &ast,
                                 const QString &parseInfo,
                                 const QString &grammarInfo);
    void onSemanticAnalysisFinished(bool success, const QString &message,
                                    const QString &symbolTableInfo,
                                    const QString &typeCheckInfo,
                                    const QString &scopeInfo,
                                    const QVector<SemanticError> &errors);
    void onCodeGenerationFinished(bool success, const QString &message,
                                 const QVector<ThreeAddressCode> &codes,
                                 const QString &optimizationInfo,
                                 const QString &basicBlockInfo,
                                 int totalInstructions, int basicBlocks, int tempVars);
    void onAnalysisError(const QString &error);
    
    // 错误显示
    void showErrorInEditor(int line, const QString &message);
    void clearAllErrors();
    
    // 其他槽函数
    void about();
    void updateRecentFiles();
    void showContextMenu(const QPoint &pos);
    void onTabChanged(int index);
    void showSettings();

private:
    // UI组件
    CodeEditor *codeEditor;
    AnalysisPanel *analysisPanel;
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
    // 对话框
    class FindReplaceDialog *findReplaceDialog;
    class SettingsDialog *settingsDialog;
    
    // 错误信息面板
    QDockWidget *errorDock;
    QListWidget *errorList;
    
    // 状态栏组件
    QLabel *statusLabel;
    QLabel *positionLabel;
    QLabel *encodingLabel;
    QProgressBar *progressBar;
    
    // 菜单和工具栏
    QMenuBar *menuBar;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *analysisToolBar;
    QToolBar *viewToolBar;
    
    // 操作
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;
    
    QAction *undoAction;
    QAction *redoAction;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *selectAllAction;
    QAction *findAction;
    QAction *replaceAction;
    QAction *gotoLineAction;
    
    QAction *runLexicalAction;
    QAction *runSyntaxAction;
    QAction *runSemanticAction;
    QAction *runCodeGenAction;
    QAction *runFullAction;
    QAction *stopAnalysisAction;
    
    QAction *toggleLineNumbersAction;
    QAction *toggleSyntaxHighlightingAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *resetZoomAction;
    
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *settingsAction;
    
    // 最近文件列表
    QList<QAction*> recentFileActions;
    static const int MaxRecentFiles = 5;
    
    // 分析线程
    AnalysisThread *analysisThread;
    
    // 当前文件信息
    QString currentFile;
    bool isUntitled;
    
    // 设置相关
    QTimer *autoAnalysisTimer;
    bool enableAutoAnalysis;
    bool enableSyntaxHighlighting;
    bool showLineNumbers;
    
    // 初始化函数
    void setupUI();
    void setupMenus();
    void setupToolBars();
    void setupStatusBar();
    void setupActions();
    void setupConnections();
    void setupDockWidgets();
    
    // 工具函数
    void createAction(QAction *&action, const QString &text, 
                     const QString &shortcut = QString(), 
                     const QString &tooltip = QString(),
                     const QString &icon = QString());
    void updateWindowTitle();
    void updateActions();
    void addToRecentFiles(const QString &fileName);
    QStringList getRecentFiles() const;
    void setRecentFiles(const QStringList &files);
    
    // 文件操作辅助
    bool maybeSave();
    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    void readSettings();
    void writeSettings();
    
    // 分析操作辅助
    void startAnalysis();
    void finishAnalysis();
    void updateAnalysisProgress(const QString &stage);
    void displayAnalysisResults();
    void clearAnalysisResults();
};

#endif // MAINWINDOW_H
