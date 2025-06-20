#ifndef CODE_EDITOR_H
#define CODE_EDITOR_H

#include <QPlainTextEdit>
#include <QTextDocument>
#include <QPainter>
#include <QTextBlock>
#include <QScrollBar>
#include <QCompleter>
#include <QTimer>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

// 前向声明
class LineNumberArea;
class SyntaxHighlighter;
class ErrorIndicator;

/**
 * @brief 代码编辑器主类
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);
    ~CodeEditor();

    // 行号区域相关
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    
    // 错误标记
    void addErrorMark(int line, const QString &message);
    void clearErrorMarks();
    void addWarningMark(int line, const QString &message);
    void clearWarningMarks();
    
    // 语法高亮
    void enableSyntaxHighlighting(bool enable = true);
    
    // 自动补全
    void setCompleter(QCompleter *completer);
    QCompleter *completer() const;
    
    // 文件操作
    bool openFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    QString currentFile() const;
    bool isModified() const;
    
    // 编辑功能
    void gotoLine(int line);
    void findText(const QString &text, bool caseSensitive = false);
    void replaceText(const QString &findText, const QString &replaceText);
    
protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onTextChanged();
    void insertCompletion(const QString &completion);

signals:
    void textChanged();
    void modificationChanged(bool changed);
    void fileNameChanged(const QString &fileName);
    void cursorPositionChanged(int line, int column);

private:
    // UI组件
    LineNumberArea *lineNumberArea;
    SyntaxHighlighter *syntaxHighlighter;
    ErrorIndicator *errorIndicator;
    QCompleter *m_completer;
    
    // 文件信息
    QString m_currentFile;
    bool m_isModified;
    
    // 编辑状态
    QTimer *m_changeTimer;
    QTextCharFormat m_currentLineFormat;
    
    // 错误和警告标记
    struct ErrorMark {
        int line;
        QString message;
        bool isWarning;
    };
    QList<ErrorMark> m_errorMarks;
    
    // 初始化函数
    void setupEditor();
    void setupCompleter();
    void updateLineNumberAreaGeometry();
    void highlightErrorLines();
    
    // 工具函数
    QString textUnderCursor() const;
    QTextBlock getLineBlock(int line) const;
};

/**
 * @brief 行号显示区域
 */
class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberArea(CodeEditor *editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *codeEditor;
};

/**
 * @brief 语法高亮器
 */
class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    // 不同类型的格式
    QTextCharFormat keywordFormat;
    QTextCharFormat typeFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat identifierFormat;
    
    void setupHighlightingRules();
};

/**
 * @brief 错误指示器
 */
class ErrorIndicator : public QObject
{
    Q_OBJECT

public:
    explicit ErrorIndicator(CodeEditor *editor);
    
    void addError(int line, const QString &message);
    void addWarning(int line, const QString &message);
    void clearAll();
    void clearErrors();
    void clearWarnings();

private:
    CodeEditor *m_editor;
    
    struct Indicator {
        int line;
        QString message;
        bool isError; // true for error, false for warning
    };
    QList<Indicator> m_indicators;
    
    void updateIndicators();
};

#endif // CODE_EDITOR_H 