#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFontComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QColorDialog>
#include <QSettings>
#include <QApplication>

/**
 * @brief 设置对话框
 * 
 * 提供编译器前端的各种配置选项：
 * - 编辑器设置（字体、主题、行为）
 * - 分析器设置（自动分析、性能选项）
 * - 界面设置（主题、布局）
 * - 高级设置（调试选项、日志级别）
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    // 获取设置值的便捷方法
    static QVariant getSetting(const QString &key, const QVariant &defaultValue = QVariant());
    static void setSetting(const QString &key, const QVariant &value);

signals:
    void settingsChanged();

public slots:
    void accept() override;
    void reject() override;

private slots:
    void resetToDefaults();
    void onFontChanged();
    void onThemeChanged();
    void onColorButtonClicked();
    void restoreDefaults();

private:
    void setupUI();
    void setupEditorTab();
    void setupAnalyzerTab();
    void setupInterfaceTab();
    void setupAdvancedTab();
    void loadSettings();
    void saveSettings();
    void applySettings();
    
    // 主要组件
    QTabWidget *tabWidget;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QPushButton *resetButton;
    QPushButton *defaultsButton;
    
    // 编辑器设置
    QFontComboBox *fontFamilyCombo;
    QSpinBox *fontSizeSpinBox;
    QCheckBox *fontBoldCheckBox;
    QCheckBox *fontItalicCheckBox;
    QComboBox *themeComboBox;
    QCheckBox *showLineNumbersCheckBox;
    QCheckBox *highlightCurrentLineCheckBox;
    QCheckBox *enableCodeFoldingCheckBox;
    QCheckBox *autoIndentCheckBox;
    QSpinBox *tabSizeSpinBox;
    QCheckBox *spacesForTabsCheckBox;
    QCheckBox *showWhitespaceCheckBox;
    QCheckBox *wordWrapCheckBox;
    
    // 分析器设置
    QCheckBox *autoLexicalAnalysisCheckBox;
    QSpinBox *analysisDelaySpinBox;
    QCheckBox *enableSyntaxHighlightingCheckBox;
    QCheckBox *enableErrorHighlightingCheckBox;
    QCheckBox *enableAutoCompletionCheckBox;
    QCheckBox *enablePerformanceMonitoringCheckBox;
    QSpinBox *maxTokensSpinBox;
    QDoubleSpinBox *timeoutSpinBox;
    
    // 界面设置
    QComboBox *uiThemeComboBox;
    QCheckBox *showToolbarCheckBox;
    QCheckBox *showStatusBarCheckBox;
    QCheckBox *showSidebarCheckBox;
    QComboBox *layoutModeComboBox;
    QCheckBox *enableAnimationsCheckBox;
    QSpinBox *windowOpacitySpinBox;
    
    // 高级设置
    QCheckBox *enableDebugModeCheckBox;
    QComboBox *logLevelComboBox;
    QLineEdit *logFilePathEdit;
    QPushButton *browseLogFileButton;
    QCheckBox *enableProfiling;
    QCheckBox *enableMemoryMonitoring;
    QSpinBox *threadPoolSizeSpinBox;
    QSpinBox *cacheMaxSizeSpinBox;
    
    // 颜色按钮
    QPushButton *backgroundColorButton;
    QPushButton *textColorButton;
    QPushButton *keywordColorButton;
    QPushButton *commentColorButton;
    QPushButton *stringColorButton;
    QPushButton *numberColorButton;
    QPushButton *errorColorButton;
    
    // 设置存储
    QSettings *settings;
    
    // 颜色存储
    QColor backgroundColor;
    QColor textColor;
    QColor keywordColor;
    QColor commentColor;
    QColor stringColor;
    QColor numberColor;
    QColor errorColor;
    
    void updateColorButton(QPushButton *button, const QColor &color);
    QColor getColorFromButton(QPushButton *button);
};

#endif // SETTINGS_DIALOG_H 