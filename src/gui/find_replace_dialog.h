#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QMessageBox>
#include <QKeySequence>
#include <QShortcut>

class CodeEditor;

/**
 * @brief 查找替换对话框
 * 
 * 提供文本查找和替换功能，支持：
 * - 普通查找和替换
 * - 区分大小写
 * - 全词匹配
 * - 正则表达式
 * - 查找全部和替换全部
 */
class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(CodeEditor *editor, QWidget *parent = nullptr);
    
    // 显示对话框的便捷方法
    void showFind();
    void showReplace();
    
    // 设置查找文本
    void setFindText(const QString &text);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void findNext();
    void findPrevious();
    void replace();
    void replaceAll();
    void findTextChanged();
    void onFindTextReturnPressed();
    void onReplaceTextReturnPressed();
    
private:
    void setupUI();
    void setupConnections();
    void updateButtons();
    bool findInEditor(bool forward = true, bool showMessage = true);
    int replaceInEditor();
    void highlightAll();
    void clearHighlights();
    
    // UI组件
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
    QPushButton *findNextButton;
    QPushButton *findPreviousButton;
    QPushButton *replaceButton;
    QPushButton *replaceAllButton;
    QPushButton *closeButton;
    
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordCheckBox;
    QCheckBox *regexCheckBox;
    
    QLabel *findLabel;
    QLabel *replaceLabel;
    QLabel *statusLabel;
    
    // 关联的编辑器
    CodeEditor *codeEditor;
    
    // 状态
    bool replaceMode;
    QString lastFindText;
    int findCount;
    
    // 快捷键
    QShortcut *escapeShortcut;
    QShortcut *findNextShortcut;
    QShortcut *findPreviousShortcut;
};

#endif // FIND_REPLACE_DIALOG_H 