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

#include "code_editor.h"
#include "analysis_panel.h"
#include "../lexer/lexer.h"
#include "../parser/parser.h"
#include "../semantic/semantic_analyzer.h"
#include "../codegen/code_generator.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @brief 编译分析线程
 */
class AnalysisThread : public QThread
{
    Q_OBJECT

public:
    explicit AnalysisThread(QObject *parent = nullptr);
    void setSourceCode(const QString &code);

protected:
    void run() override;

signals:
    void lexicalAnalysisFinished(const QVector<Token> &tokens);
    void syntaxAnalysisFinished(bool success, const QString &message);
    void semanticAnalysisFinished(bool success, const QString &message);
    void codeGenerationFinished(bool success, const QString &message);
    void analysisError(const QString &error);

private:
    QString m_sourceCode;
    QMutex m_mutex;
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
    void onSyntaxAnalysisFinished(bool success, const QString &message);
    void onSemanticAnalysisFinished(bool success, const QString &message);
    void onCodeGenerationFinished(bool success, const QString &message);
    void onAnalysisError(const QString &error);
    
    // 其他槽函数
    void about();
    void updateRecentFiles();
    void showContextMenu(const QPoint &pos);

private:
    // UI组件
    CodeEditor *codeEditor;
    AnalysisPanel *analysisPanel;
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;
    
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
