#include "settings_dialog.h"
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , settings(new QSettings(this))
{
    setWindowTitle("设置");
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setModal(true);
    resize(450, 350);
    
    setupUI();
    loadSettings();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建标签页
    tabWidget = new QTabWidget(this);
    setupEditorTab();
    setupAnalyzerTab();
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    okButton = new QPushButton("确定", this);
    cancelButton = new QPushButton("取消", this);
    resetButton = new QPushButton("重置", this);
    
    okButton->setDefault(true);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(okButton);
    
    mainLayout->addWidget(tabWidget);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(okButton, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::resetToDefaults);
}

void SettingsDialog::setupEditorTab()
{
    QWidget *editorTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(editorTab);
    
    // 字体设置组
    QGroupBox *fontGroup = new QGroupBox("字体设置", editorTab);
    QGridLayout *fontLayout = new QGridLayout(fontGroup);
    
    fontLayout->addWidget(new QLabel("字体:"), 0, 0);
    fontFamilyCombo = new QFontComboBox();
    fontLayout->addWidget(fontFamilyCombo, 0, 1);
    
    fontLayout->addWidget(new QLabel("大小:"), 0, 2);
    fontSizeSpinBox = new QSpinBox();
    fontSizeSpinBox->setRange(8, 72);
    fontSizeSpinBox->setValue(12);
    fontLayout->addWidget(fontSizeSpinBox, 0, 3);
    
    // 编辑器行为组
    QGroupBox *behaviorGroup = new QGroupBox("编辑器行为", editorTab);
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);
    
    showLineNumbersCheckBox = new QCheckBox("显示行号");
    showLineNumbersCheckBox->setChecked(true);
    behaviorLayout->addWidget(showLineNumbersCheckBox);
    
    highlightCurrentLineCheckBox = new QCheckBox("高亮当前行");
    highlightCurrentLineCheckBox->setChecked(true);
    behaviorLayout->addWidget(highlightCurrentLineCheckBox);
    
    autoIndentCheckBox = new QCheckBox("自动缩进");
    autoIndentCheckBox->setChecked(true);
    behaviorLayout->addWidget(autoIndentCheckBox);
    
    wordWrapCheckBox = new QCheckBox("自动换行");
    behaviorLayout->addWidget(wordWrapCheckBox);
    
    layout->addWidget(fontGroup);
    layout->addWidget(behaviorGroup);
    layout->addStretch();
    
    tabWidget->addTab(editorTab, "编辑器");
}

void SettingsDialog::setupAnalyzerTab()
{
    QWidget *analyzerTab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(analyzerTab);
    
    // 自动分析组
    QGroupBox *autoAnalysisGroup = new QGroupBox("自动分析", analyzerTab);
    QVBoxLayout *autoLayout = new QVBoxLayout(autoAnalysisGroup);
    
    autoLexicalAnalysisCheckBox = new QCheckBox("启用自动词法分析");
    autoLexicalAnalysisCheckBox->setChecked(true);
    autoLayout->addWidget(autoLexicalAnalysisCheckBox);
    
    QHBoxLayout *delayLayout = new QHBoxLayout();
    delayLayout->addWidget(new QLabel("分析延迟:"));
    analysisDelaySpinBox = new QSpinBox();
    analysisDelaySpinBox->setRange(100, 5000);
    analysisDelaySpinBox->setValue(1000);
    analysisDelaySpinBox->setSuffix(" ms");
    delayLayout->addWidget(analysisDelaySpinBox);
    delayLayout->addStretch();
    autoLayout->addLayout(delayLayout);
    
    // 语法高亮组
    QGroupBox *highlightGroup = new QGroupBox("语法高亮", analyzerTab);
    QVBoxLayout *highlightLayout = new QVBoxLayout(highlightGroup);
    
    enableSyntaxHighlightingCheckBox = new QCheckBox("启用语法高亮");
    enableSyntaxHighlightingCheckBox->setChecked(true);
    highlightLayout->addWidget(enableSyntaxHighlightingCheckBox);
    
    enableErrorHighlightingCheckBox = new QCheckBox("启用错误高亮");
    enableErrorHighlightingCheckBox->setChecked(true);
    highlightLayout->addWidget(enableErrorHighlightingCheckBox);
    
    layout->addWidget(autoAnalysisGroup);
    layout->addWidget(highlightGroup);
    layout->addStretch();
    
    tabWidget->addTab(analyzerTab, "分析器");
}

void SettingsDialog::setupInterfaceTab() {}
void SettingsDialog::setupAdvancedTab() {}

void SettingsDialog::loadSettings()
{
    // 编辑器设置
    fontFamilyCombo->setCurrentFont(QFont(getSetting("editor/fontFamily", "Consolas").toString()));
    fontSizeSpinBox->setValue(getSetting("editor/fontSize", 12).toInt());
    showLineNumbersCheckBox->setChecked(getSetting("editor/showLineNumbers", true).toBool());
    highlightCurrentLineCheckBox->setChecked(getSetting("editor/highlightCurrentLine", true).toBool());
    autoIndentCheckBox->setChecked(getSetting("editor/autoIndent", true).toBool());
    wordWrapCheckBox->setChecked(getSetting("editor/wordWrap", false).toBool());
    
    // 分析器设置
    autoLexicalAnalysisCheckBox->setChecked(getSetting("analyzer/autoLexical", true).toBool());
    analysisDelaySpinBox->setValue(getSetting("analyzer/delay", 1000).toInt());
    enableSyntaxHighlightingCheckBox->setChecked(getSetting("analyzer/syntaxHighlight", true).toBool());
    enableErrorHighlightingCheckBox->setChecked(getSetting("analyzer/errorHighlight", true).toBool());
}

void SettingsDialog::saveSettings()
{
    // 编辑器设置
    setSetting("editor/fontFamily", fontFamilyCombo->currentFont().family());
    setSetting("editor/fontSize", fontSizeSpinBox->value());
    setSetting("editor/showLineNumbers", showLineNumbersCheckBox->isChecked());
    setSetting("editor/highlightCurrentLine", highlightCurrentLineCheckBox->isChecked());
    setSetting("editor/autoIndent", autoIndentCheckBox->isChecked());
    setSetting("editor/wordWrap", wordWrapCheckBox->isChecked());
    
    // 分析器设置
    setSetting("analyzer/autoLexical", autoLexicalAnalysisCheckBox->isChecked());
    setSetting("analyzer/delay", analysisDelaySpinBox->value());
    setSetting("analyzer/syntaxHighlight", enableSyntaxHighlightingCheckBox->isChecked());
    setSetting("analyzer/errorHighlight", enableErrorHighlightingCheckBox->isChecked());
}

void SettingsDialog::accept()
{
    saveSettings();
    emit settingsChanged();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    loadSettings();
    QDialog::reject();
}

void SettingsDialog::resetToDefaults()
{
    loadSettings();
}

void SettingsDialog::onFontChanged() {}
void SettingsDialog::onThemeChanged() {}
void SettingsDialog::onColorButtonClicked() {}
void SettingsDialog::restoreDefaults() {}

QVariant SettingsDialog::getSetting(const QString &key, const QVariant &defaultValue)
{
    QSettings settings;
    return settings.value(key, defaultValue);
}

void SettingsDialog::setSetting(const QString &key, const QVariant &value)
{
    QSettings settings;
    settings.setValue(key, value);
}

void SettingsDialog::updateColorButton(QPushButton *button, const QColor &color)
{
    Q_UNUSED(button)
    Q_UNUSED(color)
}

QColor SettingsDialog::getColorFromButton(QPushButton *button)
{
    Q_UNUSED(button)
    return QColor();
}
