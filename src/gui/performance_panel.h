#ifndef PERFORMANCE_PANEL_H
#define PERFORMANCE_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>

#include "../performance/performance_test_manager.h"
#include "../performance/test_case_generator.h"

using namespace QtCharts;

/**
 * @brief 性能测试面板类
 */
class PerformancePanel : public QWidget {
    Q_OBJECT
    
public:
    explicit PerformancePanel(QWidget* parent = nullptr);
    ~PerformancePanel();
    
    // 公共接口
    void resetPanel();
    void loadTestCases();
    void updateResults();
    
signals:
    void testStartRequested();
    void testStopRequested();
    void reportExportRequested(const QString& filePath);
    
private slots:
    // 测试控制
    void onStartTests();
    void onStopTests();
    void onPauseTests();
    void onClearResults();
    void onExportReport();
    void onExportData();
    
    // 测试级别选择
    void onTestLevelChanged();
    void onTestCaseToggled();
    
    // 测试进度回调
    void onTestStarted(const QString& testName);
    void onTestCompleted(const PerformanceResult& result);
    void onTestFailed(const QString& testName, const QString& error);
    void onAllTestsCompleted(const QVector<PerformanceResult>& results);
    void onProgressChanged(int current, int total, double percentage);
    void onStatusChanged(const QString& status);
    
    // UI更新
    void updateProgressDisplay();
    void updateStatistics();
    void updateCharts();
    void refreshTestCaseList();
    
private:
    // UI创建方法
    void setupUI();
    void createControlPanel();
    void createConfigurationPanel();
    void createProgressPanel();
    void createResultsTable();
    void createStatisticsPanel();
    void createChartsPanel();
    void createDetailPanel();
    
    // 表格管理
    void setupResultsTableHeaders();
    void addResultToTable(const PerformanceResult& result);
    void clearResultsTable();
    void updateTableRow(int row, const PerformanceResult& result);
    
    // 图表管理
    void createTimeDistributionChart();
    void createPerformanceTrendChart();
    void createMemoryUsageChart();
    void createThroughputChart();
    void updateChartData();
    
    // 统计计算
    void calculateAndDisplayStatistics();
    QString formatTime(double timeMs) const;
    QString formatMemory(size_t memoryKB) const;
    QString formatThroughput(double throughput) const;
    
    // 数据导出
    bool exportToCsv(const QString& filePath);
    bool exportToHtml(const QString& filePath);
    bool exportToPdf(const QString& filePath);
    
    // 控件组
    QWidget* controlWidget_;
    QWidget* configWidget_;
    QWidget* progressWidget_;
    QWidget* resultsWidget_;
    QWidget* statisticsWidget_;
    QWidget* chartsWidget_;
    QWidget* detailWidget_;
    
    // 控制面板控件
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* pauseButton_;
    QPushButton* clearButton_;
    QPushButton* exportReportButton_;
    QPushButton* exportDataButton_;
    
    // 配置面板控件
    QComboBox* testLevelCombo_;
    QGroupBox* testCasesGroup_;
    QVBoxLayout* testCasesLayout_;
    QVector<QCheckBox*> testCaseCheckBoxes_;
    
    // 进度面板控件
    QProgressBar* overallProgressBar_;
    QProgressBar* currentTestProgressBar_;
    QLabel* statusLabel_;
    QLabel* currentTestLabel_;
    QLabel* timeElapsedLabel_;
    QLabel* timeRemainingLabel_;
    
    // 结果表格
    QTableWidget* resultsTable_;
    
    // 统计面板控件
    QLabel* totalTestsLabel_;
    QLabel* successfulTestsLabel_;
    QLabel* failedTestsLabel_;
    QLabel* successRateLabel_;
    QLabel* avgTimeLabel_;
    QLabel* maxTimeLabel_;
    QLabel* minTimeLabel_;
    QLabel* avgThroughputLabel_;
    QLabel* avgMemoryLabel_;
    QLabel* bottleneckLabel_;
    
    // 图表控件
    QChartView* timeDistributionChartView_;
    QChartView* performanceTrendChartView_;
    QChartView* memoryUsageChartView_;
    QChartView* throughputChartView_;
    
    QChart* timeDistributionChart_;
    QChart* performanceTrendChart_;
    QChart* memoryUsageChart_;
    QChart* throughputChart_;
    
    // 详细信息面板
    QTextEdit* detailTextEdit_;
    
    // 数据和状态
    std::unique_ptr<PerformanceTestManager> testManager_;
    QVector<TestCase> availableTestCases_;
    QVector<PerformanceResult> currentResults_;
    
    QTimer* uiUpdateTimer_;
    QTimer* elapsedTimer_;
    
    bool isTestingInProgress_;
    int startTime_;
    int currentTestIndex_;
    int totalTestCount_;
    
    // 配置
    QString lastExportPath_;
    bool autoUpdateCharts_;
    bool showDetailedLogs_;
};

#endif // PERFORMANCE_PANEL_H 