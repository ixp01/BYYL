#include "find_replace_dialog.h"
#include "code_editor.h"
#include <QKeyEvent>
#include <QShowEvent>
#include <QTextCursor>
#include <QTextDocument>
#include <QRegularExpression>
#include <QApplication>

FindReplaceDialog::FindReplaceDialog(CodeEditor *editor, QWidget *parent)
    : QDialog(parent)
    , codeEditor(editor)
    , replaceMode(false)
    , findCount(0)
{
    setWindowTitle("查找和替换");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setModal(false);
    resize(400, 180);
    
    setupUI();
    setupConnections();
    updateButtons();
}

void FindReplaceDialog::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 查找替换输入区域
    QGridLayout *inputLayout = new QGridLayout();
    
    findLabel = new QLabel("查找:", this);
    findLineEdit = new QLineEdit(this);
    findLineEdit->setPlaceholderText("输入要查找的文本...");
    
    replaceLabel = new QLabel("替换:", this);
    replaceLineEdit = new QLineEdit(this);
    replaceLineEdit->setPlaceholderText("输入替换文本...");
    
    inputLayout->addWidget(findLabel, 0, 0);
    inputLayout->addWidget(findLineEdit, 0, 1);
    inputLayout->addWidget(replaceLabel, 1, 0);
    inputLayout->addWidget(replaceLineEdit, 1, 1);
    
    // 选项区域
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    
    caseSensitiveCheckBox = new QCheckBox("区分大小写", this);
    wholeWordCheckBox = new QCheckBox("全词匹配", this);
    regexCheckBox = new QCheckBox("正则表达式", this);
    
    optionsLayout->addWidget(caseSensitiveCheckBox);
    optionsLayout->addWidget(wholeWordCheckBox);
    optionsLayout->addWidget(regexCheckBox);
    optionsLayout->addStretch();
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    findNextButton = new QPushButton("查找下一个", this);
    findPreviousButton = new QPushButton("查找上一个", this);
    replaceButton = new QPushButton("替换", this);
    replaceAllButton = new QPushButton("全部替换", this);
    closeButton = new QPushButton("关闭", this);
    
    findNextButton->setDefault(true);
    
    buttonLayout->addWidget(findNextButton);
    buttonLayout->addWidget(findPreviousButton);
    buttonLayout->addWidget(replaceButton);
    buttonLayout->addWidget(replaceAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    // 状态标签
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("color: gray; font-size: 11px;");
    
    // 组装主布局
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(optionsLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(statusLabel);
    
    // 设置初始状态（仅查找模式）
    replaceLabel->setVisible(false);
    replaceLineEdit->setVisible(false);
    replaceButton->setVisible(false);
    replaceAllButton->setVisible(false);
}

void FindReplaceDialog::setupConnections()
{
    // 按钮连接
    connect(findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &FindReplaceDialog::findPrevious);
    connect(replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::replace);
    connect(replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    // 文本框连接
    connect(findLineEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::findTextChanged);
    connect(findLineEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::onFindTextReturnPressed);
    connect(replaceLineEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::onReplaceTextReturnPressed);
    
    // 选项连接
    connect(caseSensitiveCheckBox, &QCheckBox::toggled, this, &FindReplaceDialog::findTextChanged);
    connect(wholeWordCheckBox, &QCheckBox::toggled, this, &FindReplaceDialog::findTextChanged);
    connect(regexCheckBox, &QCheckBox::toggled, this, &FindReplaceDialog::findTextChanged);
    
    // 快捷键
    escapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escapeShortcut, &QShortcut::activated, this, &QDialog::close);
    
    findNextShortcut = new QShortcut(QKeySequence(Qt::Key_F3), this);
    connect(findNextShortcut, &QShortcut::activated, this, &FindReplaceDialog::findNext);
    
    findPreviousShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F3), this);
    connect(findPreviousShortcut, &QShortcut::activated, this, &FindReplaceDialog::findPrevious);
}

void FindReplaceDialog::showFind()
{
    replaceMode = false;
    
    // 隐藏替换相关组件
    replaceLabel->setVisible(false);
    replaceLineEdit->setVisible(false);
    replaceButton->setVisible(false);
    replaceAllButton->setVisible(false);
    
    setWindowTitle("查找");
    resize(400, 120);
    
    // 获取选中文本作为查找内容
    if (codeEditor && codeEditor->textCursor().hasSelection()) {
        QString selectedText = codeEditor->textCursor().selectedText();
        if (!selectedText.isEmpty() && !selectedText.contains('\n')) {
            setFindText(selectedText);
        }
    }
    
    show();
    findLineEdit->setFocus();
    findLineEdit->selectAll();
}

void FindReplaceDialog::showReplace()
{
    replaceMode = true;
    
    // 显示替换相关组件
    replaceLabel->setVisible(true);
    replaceLineEdit->setVisible(true);
    replaceButton->setVisible(true);
    replaceAllButton->setVisible(true);
    
    setWindowTitle("查找和替换");
    resize(400, 180);
    
    // 获取选中文本作为查找内容
    if (codeEditor && codeEditor->textCursor().hasSelection()) {
        QString selectedText = codeEditor->textCursor().selectedText();
        if (!selectedText.isEmpty() && !selectedText.contains('\n')) {
            setFindText(selectedText);
        }
    }
    
    show();
    findLineEdit->setFocus();
    findLineEdit->selectAll();
}

void FindReplaceDialog::setFindText(const QString &text)
{
    findLineEdit->setText(text);
    updateButtons();
}

void FindReplaceDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    
    QDialog::keyPressEvent(event);
}

void FindReplaceDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    findLineEdit->setFocus();
}

void FindReplaceDialog::findNext()
{
    findInEditor(true);
}

void FindReplaceDialog::findPrevious()
{
    findInEditor(false);
}

void FindReplaceDialog::replace()
{
    if (!codeEditor || findLineEdit->text().isEmpty()) {
        return;
    }
    
    QTextCursor cursor = codeEditor->textCursor();
    if (cursor.hasSelection()) {
        // 检查选中的文本是否匹配查找条件
        QString selectedText = cursor.selectedText();
        QString findText = findLineEdit->text();
        
        bool matches = false;
        if (regexCheckBox->isChecked()) {
            QRegularExpression regex(findText);
            if (!caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            matches = regex.match(selectedText).hasMatch();
        } else {
            Qt::CaseSensitivity cs = caseSensitiveCheckBox->isChecked() ? 
                Qt::CaseSensitive : Qt::CaseInsensitive;
            
            if (wholeWordCheckBox->isChecked()) {
                matches = (selectedText.compare(findText, cs) == 0);
            } else {
                matches = selectedText.contains(findText, cs);
            }
        }
        
        if (matches) {
            cursor.insertText(replaceLineEdit->text());
            statusLabel->setText("已替换 1 处");
            
            // 继续查找下一个
            findNext();
            return;
        }
    }
    
    // 如果没有选中匹配的文本，先查找
    if (!findInEditor(true, false)) {
        statusLabel->setText("找不到要替换的文本");
    }
}

void FindReplaceDialog::replaceAll()
{
    if (!codeEditor || findLineEdit->text().isEmpty()) {
        return;
    }
    
    int count = replaceInEditor();
    if (count > 0) {
        statusLabel->setText(QString("已替换 %1 处").arg(count));
    } else {
        statusLabel->setText("找不到要替换的文本");
    }
}

void FindReplaceDialog::findTextChanged()
{
    updateButtons();
    if (!findLineEdit->text().isEmpty() && findLineEdit->text() != lastFindText) {
        lastFindText = findLineEdit->text();
        statusLabel->clear();
    }
}

void FindReplaceDialog::onFindTextReturnPressed()
{
    findNext();
}

void FindReplaceDialog::onReplaceTextReturnPressed()
{
    if (replaceMode) {
        replace();
    } else {
        findNext();
    }
}

void FindReplaceDialog::updateButtons()
{
    bool hasText = !findLineEdit->text().isEmpty();
    findNextButton->setEnabled(hasText);
    findPreviousButton->setEnabled(hasText);
    replaceButton->setEnabled(hasText && replaceMode);
    replaceAllButton->setEnabled(hasText && replaceMode);
}

bool FindReplaceDialog::findInEditor(bool forward, bool showMessage)
{
    if (!codeEditor || findLineEdit->text().isEmpty()) {
        return false;
    }
    
    QString findText = findLineEdit->text();
    QTextDocument::FindFlags flags = QTextDocument::FindFlags();
    
    if (!forward) {
        flags |= QTextDocument::FindBackward;
    }
    
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    
    if (wholeWordCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }
    
    QTextCursor cursor = codeEditor->textCursor();
    QTextCursor foundCursor;
    
    if (regexCheckBox->isChecked()) {
        QRegularExpression regex(findText);
        if (!caseSensitiveCheckBox->isChecked()) {
            regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        foundCursor = codeEditor->document()->find(regex, cursor, flags);
    } else {
        foundCursor = codeEditor->document()->find(findText, cursor, flags);
    }
    
    if (!foundCursor.isNull()) {
        codeEditor->setTextCursor(foundCursor);
        statusLabel->setText(QString("找到匹配项"));
        return true;
    } else {
        // 从文档开头/结尾重新搜索
        cursor.movePosition(forward ? QTextCursor::Start : QTextCursor::End);
        
        if (regexCheckBox->isChecked()) {
            QRegularExpression regex(findText);
            if (!caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            foundCursor = codeEditor->document()->find(regex, cursor, flags);
        } else {
            foundCursor = codeEditor->document()->find(findText, cursor, flags);
        }
        
        if (!foundCursor.isNull()) {
            codeEditor->setTextCursor(foundCursor);
            statusLabel->setText(QString("从%1开始查找").arg(forward ? "开头" : "结尾"));
            return true;
        } else {
            if (showMessage) {
                statusLabel->setText("找不到匹配项");
            }
            return false;
        }
    }
}

int FindReplaceDialog::replaceInEditor()
{
    if (!codeEditor || findLineEdit->text().isEmpty()) {
        return 0;
    }
    
    QString findText = findLineEdit->text();
    QString replaceText = replaceLineEdit->text();
    int count = 0;
    
    QTextCursor cursor = codeEditor->textCursor();
    cursor.beginEditBlock();
    
    // 移动到文档开头
    cursor.movePosition(QTextCursor::Start);
    
    QTextDocument::FindFlags flags = QTextDocument::FindFlags();
    if (caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (wholeWordCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }
    
    while (true) {
        QTextCursor foundCursor;
        
        if (regexCheckBox->isChecked()) {
            QRegularExpression regex(findText);
            if (!caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            foundCursor = codeEditor->document()->find(regex, cursor, flags);
        } else {
            foundCursor = codeEditor->document()->find(findText, cursor, flags);
        }
        
        if (foundCursor.isNull()) {
            break;
        }
        
        foundCursor.insertText(replaceText);
        cursor = foundCursor;
        count++;
    }
    
    cursor.endEditBlock();
    return count;
} 