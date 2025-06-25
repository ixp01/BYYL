#include "performance_panel.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QGroupBox>
#include <QSplitter>
#include <QDateTime>
#include <QTextStream>
#include <QDebug>

PerformancePanel::PerformancePanel(QWidget* parent)
    : QWidget(parent)
    , isTestingInProgress_(false)
    , startTime_(0)
    , currentTestIndex_(0)
    , totalTestCount_(0)
    , autoUpdateCharts_(true)
    , showDetailedLogs_(false)
{
    // 初始化测试管理器
    testManager_ = std::make_unique<PerformanceTestManager>(this);
    
    // 设置UI
    setupUI();
    
    // 连接信号槽
    connect(testManager_.get(), &PerformanceTestManager::testStarted,
            this, &PerformancePanel::onTestStarted);
    connect(testManager_.get(), &PerformanceTestManager::testCompleted,
            this, &PerformancePanel::onTestCompleted);
    connect(testManager_.get(), &PerformanceTestManager::testFailed,
            this, &PerformancePanel::onTestFailed);
    connect(testManager_.get(), &PerformanceTestManager::allTestsCompleted,
            this, &PerformancePanel::onAllTestsCompleted);
    connect(testManager_.get(), &PerformanceTestManager::progressChanged,
            this, &PerformancePanel::onProgressChanged);
    connect(testManager_.get(), &PerformanceTestManager::statusChanged,
            this, &PerformancePanel::onStatusChanged);
    
    // 初始化定时器
    uiUpdateTimer_ = new QTimer(this);
    connect(uiUpdateTimer_, &QTimer::timeout, this, &PerformancePanel::updateProgressDisplay);
    
    elapsedTimer_ = new QTimer(this);
    connect(elapsedTimer_, &QTimer::timeout, this, &PerformancePanel::updateProgressDisplay);
    
    // 加载测试用例
    loadTestCases();
}

PerformancePanel::~PerformancePanel()
{
    if (isTestingInProgress_) {
        testManager_->stopTests();
    }
}

void PerformancePanel::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // 创建主分割器
    auto* mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧控制面板
    auto* leftWidget = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftWidget);
    
    createControlPanel();
    createConfigurationPanel();
    createProgressPanel();
    
    leftLayout->addWidget(controlWidget_);
    leftLayout->addWidget(configWidget_);
    leftLayout->addWidget(progressWidget_);
    leftLayout->addStretch();
    
    leftWidget->setMaximumWidth(350);
    leftWidget->setMinimumWidth(300);
    
    // 右侧结果面板
    auto* rightWidget = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightWidget);
    
    createResultsTable();
    createStatisticsPanel();
    
    auto* resultsSplitter = new QSplitter(Qt::Vertical);
    resultsSplitter->addWidget(resultsWidget_);
    resultsSplitter->addWidget(statisticsWidget_);
    resultsSplitter->setStretchFactor(0, 3);
    resultsSplitter->setStretchFactor(1, 1);
    
    rightLayout->addWidget(resultsSplitter);
    
    // 添加到主分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 0);
    mainSplitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(mainSplitter);
}

void PerformancePanel::createControlPanel()
{
    controlWidget_ = new QGroupBox("测试控制", this);
    auto* layout = new QGridLayout(controlWidget_);
    
    // 创建按钮
    startButton_ = new QPushButton("开始测试", this);
    stopButton_ = new QPushButton("停止测试", this);
    pauseButton_ = new QPushButton("暂停测试", this);
    clearButton_ = new QPushButton("清除结果", this);
    exportReportButton_ = new QPushButton("导出报告", this);
    exportDataButton_ = new QPushButton("导出数据", this);
    
    // 设置按钮样式
    startButton_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    stopButton_->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    pauseButton_->setStyleSheet("QPushButton { background-color: #ff9800; color: white; font-weight: bold; }");
    
    // 初始状态
    stopButton_->setEnabled(false);
    pauseButton_->setEnabled(false);
    
    // 连接信号
    connect(startButton_, &QPushButton::clicked, this, &PerformancePanel::onStartTests);
    connect(stopButton_, &QPushButton::clicked, this, &PerformancePanel::onStopTests);
    connect(pauseButton_, &QPushButton::clicked, this, &PerformancePanel::onPauseTests);
    connect(clearButton_, &QPushButton::clicked, this, &PerformancePanel::onClearResults);
    connect(exportReportButton_, &QPushButton::clicked, this, &PerformancePanel::onExportReport);
    connect(exportDataButton_, &QPushButton::clicked, this, &PerformancePanel::onExportData);
    
    // 布局
    layout->addWidget(startButton_, 0, 0);
    layout->addWidget(stopButton_, 0, 1);
    layout->addWidget(pauseButton_, 1, 0);
    layout->addWidget(clearButton_, 1, 1);
    layout->addWidget(exportReportButton_, 2, 0);
    layout->addWidget(exportDataButton_, 2, 1);
}

void PerformancePanel::createConfigurationPanel()
{
    configWidget_ = new QGroupBox("测试配置", this);
    auto* layout = new QVBoxLayout(configWidget_);
    
    // 测试级别选择
    auto* levelLayout = new QHBoxLayout;
    levelLayout->addWidget(new QLabel("测试级别:", this));
    
    testLevelCombo_ = new QComboBox(this);
    testLevelCombo_->addItem("快速测试", static_cast<int>(PerformanceTestManager::TestLevel::QUICK));
    testLevelCombo_->addItem("标准测试", static_cast<int>(PerformanceTestManager::TestLevel::STANDARD));
    testLevelCombo_->addItem("全面测试", static_cast<int>(PerformanceTestManager::TestLevel::COMPREHENSIVE));
    testLevelCombo_->setCurrentIndex(1); // 默认标准测试
    
    connect(testLevelCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PerformancePanel::onTestLevelChanged);
    
    levelLayout->addWidget(testLevelCombo_);
    layout->addLayout(levelLayout);
    
    // 测试用例选择
    testCasesGroup_ = new QGroupBox("测试用例", this);
    testCasesLayout_ = new QVBoxLayout(testCasesGroup_);
    
    layout->addWidget(testCasesGroup_);
}

void PerformancePanel::createProgressPanel()
{
    progressWidget_ = new QGroupBox("测试进度", this);
    auto* layout = new QVBoxLayout(progressWidget_);
    
    // 整体进度
    layout->addWidget(new QLabel("整体进度:", this));
    overallProgressBar_ = new QProgressBar(this);
    overallProgressBar_->setTextVisible(true);
    layout->addWidget(overallProgressBar_);
    
    // 当前测试进度
    layout->addWidget(new QLabel("当前测试:", this));
    currentTestProgressBar_ = new QProgressBar(this);
    currentTestProgressBar_->setTextVisible(true);
    layout->addWidget(currentTestProgressBar_);
    
    // 状态信息
    statusLabel_ = new QLabel("就绪", this);
    statusLabel_->setWordWrap(true);
    layout->addWidget(statusLabel_);
    
    currentTestLabel_ = new QLabel("无", this);
    currentTestLabel_->setWordWrap(true);
    layout->addWidget(currentTestLabel_);
    
    timeElapsedLabel_ = new QLabel("已用时间: 00:00", this);
    layout->addWidget(timeElapsedLabel_);
    
    timeRemainingLabel_ = new QLabel("剩余时间: --:--", this);
    layout->addWidget(timeRemainingLabel_);
}

void PerformancePanel::createResultsTable()
{
    resultsWidget_ = new QGroupBox("测试结果", this);
    auto* layout = new QVBoxLayout(resultsWidget_);
    
    resultsTable_ = new QTableWidget(this);
    setupResultsTableHeaders();
    
    // 设置表格属性
    resultsTable_->setAlternatingRowColors(true);
    resultsTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    resultsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    resultsTable_->setSortingEnabled(true);
    
    // 调整列宽
    resultsTable_->horizontalHeader()->setStretchLastSection(true);
    resultsTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    layout->addWidget(resultsTable_);
}

void PerformancePanel::createStatisticsPanel()
{
    statisticsWidget_ = new QGroupBox("统计信息", this);
    auto* layout = new QGridLayout(statisticsWidget_);
    
    // 创建统计标签
    totalTestsLabel_ = new QLabel("0", this);
    successfulTestsLabel_ = new QLabel("0", this);
    failedTestsLabel_ = new QLabel("0", this);
    successRateLabel_ = new QLabel("0%", this);
    avgTimeLabel_ = new QLabel("0 ms", this);
    maxTimeLabel_ = new QLabel("0 ms", this);
    minTimeLabel_ = new QLabel("0 ms", this);
    avgThroughputLabel_ = new QLabel("0 lines/s", this);
    avgMemoryLabel_ = new QLabel("0 KB", this);
    bottleneckLabel_ = new QLabel("无", this);
    
    // 布局统计信息
    int row = 0;
    layout->addWidget(new QLabel("总测试数:"), row, 0);
    layout->addWidget(totalTestsLabel_, row++, 1);
    
    layout->addWidget(new QLabel("成功测试:"), row, 0);
    layout->addWidget(successfulTestsLabel_, row++, 1);
    
    layout->addWidget(new QLabel("失败测试:"), row, 0);
    layout->addWidget(failedTestsLabel_, row++, 1);
    
    layout->addWidget(new QLabel("成功率:"), row, 0);
    layout->addWidget(successRateLabel_, row++, 1);
    
    layout->addWidget(new QLabel("平均时间:"), row, 0);
    layout->addWidget(avgTimeLabel_, row++, 1);
    
    layout->addWidget(new QLabel("最大时间:"), row, 0);
    layout->addWidget(maxTimeLabel_, row++, 1);
    
    layout->addWidget(new QLabel("最小时间:"), row, 0);
    layout->addWidget(minTimeLabel_, row++, 1);
    
    layout->addWidget(new QLabel("平均吞吐量:"), row, 0);
    layout->addWidget(avgThroughputLabel_, row++, 1);
    
    layout->addWidget(new QLabel("平均内存:"), row, 0);
    layout->addWidget(avgMemoryLabel_, row++, 1);
    
    layout->addWidget(new QLabel("性能瓶颈:"), row, 0);
    layout->addWidget(bottleneckLabel_, row++, 1);
}

void PerformancePanel::setupResultsTableHeaders()
{
    QStringList headers = {
        "测试用例", "规模", "状态", "总时间(ms)",
        "词法(ms)", "语法(ms)", "语义(ms)", "代码生成(ms)",
        "内存(KB)", "吞吐量(lines/s)", "Token数", "AST节点"
    };
    
    resultsTable_->setColumnCount(headers.size());
    resultsTable_->setHorizontalHeaderLabels(headers);
}

void PerformancePanel::loadTestCases()
{
    // 清除现有的测试用例
    availableTestCases_.clear();
    
    // 生成所有测试用例
    availableTestCases_ = TestCaseGenerator::generateAllTestCases();
    
    // 刷新UI
    refreshTestCaseList();
    
    qDebug() << "Loaded" << availableTestCases_.size() << "test cases";
}

void PerformancePanel::refreshTestCaseList()
{
    // 清除现有的复选框
    for (auto* checkBox : testCaseCheckBoxes_) {
        testCasesLayout_->removeWidget(checkBox);
        delete checkBox;
    }
    testCaseCheckBoxes_.clear();
    
    // 根据当前级别过滤测试用例
    auto level = static_cast<PerformanceTestManager::TestLevel>(
        testLevelCombo_->currentData().toInt());
    
    for (const auto& testCase : availableTestCases_) {
        bool shouldShow = false;
        
        switch (level) {
            case PerformanceTestManager::TestLevel::QUICK:
                shouldShow = (testCase.category == "小规模");
                break;
            case PerformanceTestManager::TestLevel::STANDARD:
                shouldShow = (testCase.category == "小规模" || testCase.category == "中规模");
                break;
            case PerformanceTestManager::TestLevel::COMPREHENSIVE:
                shouldShow = true;
                break;
        }
        
        if (shouldShow) {
            auto* checkBox = new QCheckBox(
                QString("%1 (%2)").arg(testCase.description, testCase.category), this);
            checkBox->setChecked(testCase.isEnabled);
            checkBox->setObjectName(testCase.name);
            
            connect(checkBox, &QCheckBox::toggled, this, &PerformancePanel::onTestCaseToggled);
            
            testCasesLayout_->addWidget(checkBox);
            testCaseCheckBoxes_.append(checkBox);
        }
    }
}

// 主要槽函数实现
void PerformancePanel::onStartTests()
{
    if (isTestingInProgress_) {
        QMessageBox::warning(this, "警告", "测试正在进行中，请先停止当前测试。");
        return;
    }
    
    // 检查是否有选中的测试用例
    bool hasEnabledTests = false;
    for (auto* checkBox : testCaseCheckBoxes_) {
        if (checkBox->isChecked()) {
            hasEnabledTests = true;
            break;
        }
    }
    
    if (!hasEnabledTests) {
        QMessageBox::warning(this, "警告", "请至少选择一个测试用例。");
        return;
    }
    
    // 更新UI状态
    isTestingInProgress_ = true;
    startButton_->setEnabled(false);
    stopButton_->setEnabled(true);
    pauseButton_->setEnabled(true);
    
    // 清除之前的结果
    currentResults_.clear();
    clearResultsTable();
    
    // 重置进度
    currentTestIndex_ = 0;
    overallProgressBar_->setValue(0);
    currentTestProgressBar_->setValue(0);
    
    // 启动定时器
    startTime_ = QDateTime::currentMSecsSinceEpoch();
    elapsedTimer_->start(1000); // 每秒更新一次
    uiUpdateTimer_->start(100); // 每100ms更新一次UI
    
    // 开始测试
    auto level = static_cast<PerformanceTestManager::TestLevel>(
        testLevelCombo_->currentData().toInt());
    testManager_->runPerformanceTests(level);
    
    emit testStartRequested();
}

void PerformancePanel::onStopTests()
{
    if (!isTestingInProgress_) {
        return;
    }
    
    testManager_->stopTests();
    
    // 更新UI状态
    isTestingInProgress_ = false;
    startButton_->setEnabled(true);
    stopButton_->setEnabled(false);
    pauseButton_->setEnabled(false);
    
    statusLabel_->setText("测试已停止");
    
    // 停止定时器
    elapsedTimer_->stop();
    uiUpdateTimer_->stop();
    
    emit testStopRequested();
}

void PerformancePanel::onTestStarted(const QString& testName)
{
    currentTestLabel_->setText(QString("当前: %1").arg(testName));
    statusLabel_->setText(QString("正在执行: %1").arg(testName));
    currentTestProgressBar_->setValue(0);
}

void PerformancePanel::onTestCompleted(const PerformanceResult& result)
{
    currentResults_.append(result);
    addResultToTable(result);
    updateStatistics();
    
    currentTestIndex_++;
    currentTestProgressBar_->setValue(100);
}

void PerformancePanel::onAllTestsCompleted(const QVector<PerformanceResult>& results)
{
    Q_UNUSED(results)
    
    // 更新UI状态
    isTestingInProgress_ = false;
    startButton_->setEnabled(true);
    stopButton_->setEnabled(false);
    pauseButton_->setEnabled(false);
    
    statusLabel_->setText("所有测试完成");
    overallProgressBar_->setValue(100);
    currentTestProgressBar_->setValue(100);
    
    // 停止定时器
    elapsedTimer_->stop();
    uiUpdateTimer_->stop();
    
    // 更新统计信息
    updateStatistics();
    
    QMessageBox::information(this, "完成", 
        QString("性能测试完成！\n"
                "总测试数: %1\n"
                "成功: %2\n"
                "失败: %3")
        .arg(results.size())
        .arg(results.size() - std::count_if(results.begin(), results.end(), 
                [](const PerformanceResult& r) { return r.status != "Success"; }))
        .arg(std::count_if(results.begin(), results.end(), 
                [](const PerformanceResult& r) { return r.status != "Success"; })));
}

// 其他实现方法...
void PerformancePanel::addResultToTable(const PerformanceResult& result)
{
    int row = resultsTable_->rowCount();
    resultsTable_->insertRow(row);
    
    resultsTable_->setItem(row, 0, new QTableWidgetItem(result.testCaseName));
    resultsTable_->setItem(row, 1, new QTableWidgetItem(result.category));
    resultsTable_->setItem(row, 2, new QTableWidgetItem(result.status));
    resultsTable_->setItem(row, 3, new QTableWidgetItem(formatTime(result.totalTime)));
    resultsTable_->setItem(row, 4, new QTableWidgetItem(formatTime(result.lexicalTime)));
    resultsTable_->setItem(row, 5, new QTableWidgetItem(formatTime(result.syntaxTime)));
    resultsTable_->setItem(row, 6, new QTableWidgetItem(formatTime(result.semanticTime)));
    resultsTable_->setItem(row, 7, new QTableWidgetItem(formatTime(result.codegenTime)));
    resultsTable_->setItem(row, 8, new QTableWidgetItem(formatMemory(result.memoryUsage)));
    resultsTable_->setItem(row, 9, new QTableWidgetItem(formatThroughput(result.linesPerSecond)));
    resultsTable_->setItem(row, 10, new QTableWidgetItem(QString::number(result.tokenCount)));
    resultsTable_->setItem(row, 11, new QTableWidgetItem(QString::number(result.astNodes)));
    
    // 设置状态颜色
    if (result.status == "Success") {
        resultsTable_->item(row, 2)->setBackground(QColor(200, 255, 200));
    } else {
        resultsTable_->item(row, 2)->setBackground(QColor(255, 200, 200));
    }
}

QString PerformancePanel::formatTime(double timeMs) const
{
    return QString::number(timeMs, 'f', 3);
}

QString PerformancePanel::formatMemory(size_t memoryKB) const
{
    if (memoryKB < 1024) {
        return QString::number(memoryKB) + " KB";
    } else {
        return QString::number(memoryKB / 1024.0, 'f', 1) + " MB";
    }
}

QString PerformancePanel::formatThroughput(double throughput) const
{
    return QString::number(throughput, 'f', 1);
}

// 其他槽函数的简化实现...
void PerformancePanel::onPauseTests() { /* 暂时为空 */ }
void PerformancePanel::onClearResults() { 
    currentResults_.clear();
    clearResultsTable();
    updateStatistics();
}
void PerformancePanel::onExportReport() { /* 后续实现 */ }
void PerformancePanel::onExportData() { /* 后续实现 */ }
void PerformancePanel::onTestLevelChanged() { refreshTestCaseList(); }
void PerformancePanel::onTestCaseToggled() { /* 后续实现 */ }
void PerformancePanel::onTestFailed(const QString& testName, const QString& error) {
    Q_UNUSED(testName); Q_UNUSED(error);
}
void PerformancePanel::onProgressChanged(int current, int total, double percentage) {
    overallProgressBar_->setValue(static_cast<int>(percentage));
    totalTestCount_ = total;
}
void PerformancePanel::onStatusChanged(const QString& status) {
    statusLabel_->setText(status);
}
void PerformancePanel::updateProgressDisplay() { /* 后续实现 */ }
void PerformancePanel::updateStatistics() { /* 后续实现 */ }
void PerformancePanel::updateCharts() { /* 后续实现 */ }
void PerformancePanel::clearResultsTable() {
    resultsTable_->setRowCount(0);
}

// 其他方法的默认实现...
void PerformancePanel::resetPanel() {}
void PerformancePanel::updateResults() {}

#include "performance_panel.moc" 