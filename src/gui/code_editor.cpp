#include "code_editor.h"
#include <QApplication>
#include <QTextBlock>
#include <QTextCursor>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>

// ============ CodeEditor 实现 ============

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , lineNumberArea(new LineNumberArea(this))
    , syntaxHighlighter(new SyntaxHighlighter(document()))
    , errorIndicator(new ErrorIndicator(this))
    , m_completer(nullptr)
    , m_isModified(false)
    , m_changeTimer(new QTimer(this))
{
    setupEditor();
    setupCompleter();
    
    // 连接信号槽
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
    connect(this, &QPlainTextEdit::textChanged, this, &CodeEditor::onTextChanged);
    connect(m_changeTimer, &QTimer::timeout, this, [this]() {
        emit textChanged();
    });
    
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

CodeEditor::~CodeEditor()
{
    delete syntaxHighlighter;
    delete errorIndicator;
}

void CodeEditor::setupEditor()
{
    // 设置字体
    QFont font("Consolas");
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);
    
    // 设置制表符宽度
    const int tabStop = 4;
    QFontMetrics metrics(font);
    setTabStopDistance(tabStop * metrics.horizontalAdvance(' '));
    
    // 设置当前行高亮格式
    m_currentLineFormat.setBackground(QColor(232, 232, 255));
    m_currentLineFormat.setProperty(QTextFormat::FullWidthSelection, true);
    
    // 设置编辑器样式
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setWordWrapMode(QTextOption::NoWrap);
    
    // 配置变更检测定时器
    m_changeTimer->setSingleShot(true);
    m_changeTimer->setInterval(500); // 500ms延迟
}

void CodeEditor::setupCompleter()
{
    // 创建基础的关键字补全
    QStringList keywords;
    keywords << "auto" << "break" << "case" << "char" << "const" << "continue"
             << "default" << "do" << "double" << "else" << "enum" << "extern"
             << "float" << "for" << "goto" << "if" << "int" << "long"
             << "register" << "return" << "short" << "signed" << "sizeof"
             << "static" << "struct" << "switch" << "typedef" << "union"
             << "unsigned" << "void" << "volatile" << "while" << "true" << "false";
    
    m_completer = new QCompleter(keywords, this);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWidget(this);
    
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    
    int space = 13 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);
    updateLineNumberAreaGeometry();
}

void CodeEditor::updateLineNumberAreaGeometry()
{
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format = m_currentLineFormat;
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
    
    // 发送光标位置变化信号
    QTextCursor cursor = textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    emit cursorPositionChanged(line, column);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), QColor(245, 245, 245));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(120, 120, 120));
            painter.drawText(0, top, lineNumberArea->width() - 3, fontMetrics().height(),
                           Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (m_completer && m_completer->popup()->isVisible()) {
        // 处理自动补全
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        default:
            break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E);
    if (!m_completer || !isShortcut)
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!m_completer || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
                      || eow.contains(e->text().right(1)))) {
        m_completer->popup()->hide();
        return;
    }

    if (completionPrefix != m_completer->completionPrefix()) {
        m_completer->setCompletionPrefix(completionPrefix);
        m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
    }
    
    QRect cr = cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0)
                + m_completer->popup()->verticalScrollBar()->sizeHint().width());
    m_completer->complete(cr);
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (m_completer)
        m_completer->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl + 滚轮缩放字体
        const int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();
    } else {
        QPlainTextEdit::wheelEvent(event);
    }
}

void CodeEditor::onTextChanged()
{
    m_isModified = true;
    emit modificationChanged(true);
    
    // 延迟触发分析
    m_changeTimer->start();
}

void CodeEditor::insertCompletion(const QString &completion)
{
    if (m_completer->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

// 文件操作实现
bool CodeEditor::openFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Code Editor"),
                           tr("Cannot read file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    m_currentFile = fileName;
    m_isModified = false;
    emit fileNameChanged(fileName);
    emit modificationChanged(false);
    
    return true;
}

bool CodeEditor::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Code Editor"),
                           tr("Cannot write file %1:\n%2.")
                           .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << toPlainText();
    QApplication::restoreOverrideCursor();

    m_currentFile = fileName;
    m_isModified = false;
    emit fileNameChanged(fileName);
    emit modificationChanged(false);
    
    return true;
}

QString CodeEditor::currentFile() const
{
    return m_currentFile;
}

bool CodeEditor::isModified() const
{
    return m_isModified;
}

void CodeEditor::gotoLine(int line)
{
    QTextCursor cursor(document()->findBlockByLineNumber(line - 1));
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::findText(const QString &text, bool caseSensitive)
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    
    if (!find(text, flags)) {
        // 如果没找到，从头开始查找
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        setTextCursor(cursor);
        find(text, flags);
    }
}

void CodeEditor::replaceText(const QString &findText, const QString &replaceText)
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection() && cursor.selectedText() == findText) {
        cursor.insertText(replaceText);
    }
}

// 错误标记实现
void CodeEditor::addErrorMark(int line, const QString &message)
{
    ErrorMark mark;
    mark.line = line;
    mark.message = message;
    mark.isWarning = false;
    m_errorMarks.append(mark);
    highlightErrorLines();
}

void CodeEditor::clearErrorMarks()
{
    m_errorMarks.clear();
    highlightErrorLines();
}

void CodeEditor::addWarningMark(int line, const QString &message)
{
    ErrorMark mark;
    mark.line = line;
    mark.message = message;
    mark.isWarning = true;
    m_errorMarks.append(mark);
    highlightErrorLines();
}

void CodeEditor::clearWarningMarks()
{
    for (auto it = m_errorMarks.begin(); it != m_errorMarks.end();) {
        if (it->isWarning) {
            it = m_errorMarks.erase(it);
        } else {
            ++it;
        }
    }
    highlightErrorLines();
}

void CodeEditor::highlightErrorLines()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    // 添加当前行高亮
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format = m_currentLineFormat;
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    // 添加错误行高亮
    for (const auto &mark : m_errorMarks) {
        QTextEdit::ExtraSelection selection;
        QTextCharFormat format;
        
        if (mark.isWarning) {
            format.setBackground(QColor(255, 255, 200, 100)); // 淡黄色背景
            format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            format.setUnderlineColor(QColor(255, 165, 0)); // 橙色波浪线
        } else {
            format.setBackground(QColor(255, 200, 200, 100)); // 淡红色背景
            format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            format.setUnderlineColor(Qt::red); // 红色波浪线
        }
        
        selection.format = format;
        selection.cursor = QTextCursor(document()->findBlockByLineNumber(mark.line - 1));
        selection.cursor.select(QTextCursor::LineUnderCursor);
        extraSelections.append(selection);
    }
    
    setExtraSelections(extraSelections);
}

void CodeEditor::enableSyntaxHighlighting(bool enable)
{
    if (enable) {
        syntaxHighlighter->setDocument(document());
    } else {
        syntaxHighlighter->setDocument(nullptr);
    }
}

void CodeEditor::setCompleter(QCompleter *completer)
{
    if (m_completer)
        m_completer->disconnect(this);
    
    m_completer = completer;
    
    if (!m_completer)
        return;
    
    m_completer->setWidget(this);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);
}

QCompleter *CodeEditor::completer() const
{
    return m_completer;
}

QTextBlock CodeEditor::getLineBlock(int line) const
{
    return document()->findBlockByLineNumber(line - 1);
}

// ============ LineNumberArea 实现 ============

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->lineNumberAreaPaintEvent(event);
}

// ============ SyntaxHighlighter 实现 ============

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    setupHighlightingRules();
}

void SyntaxHighlighter::setupHighlightingRules()
{
    HighlightingRule rule;

    // 关键字格式
    keywordFormat.setColor(QColor(0, 0, 255));
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bauto\\b" << "\\bbreak\\b" << "\\bcase\\b" << "\\bchar\\b"
                    << "\\bconst\\b" << "\\bcontinue\\b" << "\\bdefault\\b" << "\\bdo\\b"
                    << "\\bdouble\\b" << "\\belse\\b" << "\\benum\\b" << "\\bextern\\b"
                    << "\\bfloat\\b" << "\\bfor\\b" << "\\bgoto\\b" << "\\bif\\b"
                    << "\\bint\\b" << "\\blong\\b" << "\\breturn\\b" << "\\bshort\\b"
                    << "\\bsigned\\b" << "\\bsizeof\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\bswitch\\b" << "\\btypedef\\b" << "\\bunion\\b" << "\\bunsigned\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bwhile\\b" << "\\btrue\\b" << "\\bfalse\\b";
    
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 数字格式
    numberFormat.setColor(QColor(255, 0, 255));
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 字符串格式
    stringFormat.setColor(QColor(0, 128, 0));
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // 字符格式
    rule.pattern = QRegularExpression("'.*'");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // 单行注释格式
    commentFormat.setColor(QColor(128, 128, 128));
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//.*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // 运算符格式
    operatorFormat.setColor(QColor(128, 0, 128));
    operatorFormat.setFontWeight(QFont::Bold);
    QStringList operatorPatterns;
    operatorPatterns << "\\+" << "-" << "\\*" << "/" << "%" << "=" << "==" << "!="
                     << "<" << ">" << "<=" << ">=" << "&&" << "\\|\\|" << "!" << "\\+\\+"
                     << "--" << "\\+=" << "-=" << "\\*=" << "/=" << "%=";
    
    foreach (const QString &pattern, operatorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 多行注释处理
    setCurrentBlockState(0);
    QRegularExpression startExpression("/\\*");
    QRegularExpression endExpression("\\*/");

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = endExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, commentFormat);
        startIndex = text.indexOf(startExpression, startIndex + commentLength);
    }
}

// ============ ErrorIndicator 实现 ============

ErrorIndicator::ErrorIndicator(CodeEditor *editor)
    : QObject(editor), m_editor(editor)
{
}

void ErrorIndicator::addError(int line, const QString &message)
{
    Indicator indicator;
    indicator.line = line;
    indicator.message = message;
    indicator.isError = true;
    m_indicators.append(indicator);
    updateIndicators();
}

void ErrorIndicator::addWarning(int line, const QString &message)
{
    Indicator indicator;
    indicator.line = line;
    indicator.message = message;
    indicator.isError = false;
    m_indicators.append(indicator);
    updateIndicators();
}

void ErrorIndicator::clearAll()
{
    m_indicators.clear();
    updateIndicators();
}

void ErrorIndicator::clearErrors()
{
    for (auto it = m_indicators.begin(); it != m_indicators.end();) {
        if (it->isError) {
            it = m_indicators.erase(it);
        } else {
            ++it;
        }
    }
    updateIndicators();
}

void ErrorIndicator::clearWarnings()
{
    for (auto it = m_indicators.begin(); it != m_indicators.end();) {
        if (!it->isError) {
            it = m_indicators.erase(it);
        } else {
            ++it;
        }
    }
    updateIndicators();
}

void ErrorIndicator::updateIndicators()
{
    // 清除之前的标记
    m_editor->clearErrorMarks();
    m_editor->clearWarningMarks();
    
    // 添加新的标记
    for (const auto &indicator : m_indicators) {
        if (indicator.isError) {
            m_editor->addErrorMark(indicator.line, indicator.message);
        } else {
            m_editor->addWarningMark(indicator.line, indicator.message);
        }
    }
}

#include "code_editor.moc" 