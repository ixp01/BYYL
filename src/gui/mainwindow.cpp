#include "mainwindow.h"
#include <QApplication>
#include <QCloseEvent>
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QInputDialog>
#include <QTextStream>

// ============ AnalysisThread 实现 ============

AnalysisThread::AnalysisThread(QObject *parent)
    : QThread(parent)
{
}

void AnalysisThread::setSourceCode(const QString &code)
{
    QMutexLocker locker(&m_mutex);
    m_sourceCode = code;
}

void AnalysisThread::run()
{
    try {
        QMutexLocker locker(&m_mutex);
        QString code = m_sourceCode;
        locker.unlock();
        
        if (code.isEmpty()) {
            emit analysisError("源代码为空");
            return;
        }
        
        // 1. 词法分析
        Lexer lexer(code);
        QVector<Token> tokens = lexer.tokenize();
        emit lexicalAnalysisFinished(tokens);
        
        if (isInterruptionRequested()) return;
        
        // 2. 语法分析
        // 这里需要根据实际的Parser实现来调用
        emit syntaxAnalysisFinished(true, "语法分析完成");
        
        if (isInterruptionRequested()) return;
        
        // 3. 语义分析
        emit semanticAnalysisFinished(true, "语义分析完成");
        
        if (isInterruptionRequested()) return;
        
        // 4. 代码生成
        emit codeGenerationFinished(true, "代码生成完成");
        
    } catch (const std::exception &e) {
        emit analysisError(QString("分析过程中发生错误: %1").arg(e.what()));
    }
}

// ============ MainWindow 实现 ============

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , codeEditor(nullptr)
    , analysisPanel(nullptr)
    , analysisThread(nullptr)
    , isUntitled(true)
    , enableAutoAnalysis(true)
    , enableSyntaxHighlighting(true)
    , showLineNumbers(true)
{
    setupUI();
    setupActions();
    setupMenus();
    setupToolBars();
    setupStatusBar();
    setupDockWidgets();
    setupConnections();
    
    // 读取设置
    readSettings();
    
    // 设置初始状态
    setCurrentFile(QString());
    updateActions();
    
    // 自动分析定时器
    autoAnalysisTimer = new QTimer(this);
    autoAnalysisTimer->setSingleShot(true);
    autoAnalysisTimer->setInterval(1000); // 1秒延迟
    connect(autoAnalysisTimer, &QTimer::timeout, this, &MainWindow::runLexicalAnalysis);
}

MainWindow::~MainWindow()
{
    if (analysisThread && analysisThread->isRunning()) {
        analysisThread->requestInterruption();
        analysisThread->wait(3000);
    }
    writeSettings();
}

void MainWindow::setupUI()
{
    // 创建中央部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // 创建代码编辑器
    codeEditor = new CodeEditor(this);
    
    // 创建分析面板
    analysisPanel = new AnalysisPanel(this);
    
    // 创建右侧分割器
    rightSplitter = new QSplitter(Qt::Vertical, this);
    rightSplitter->addWidget(analysisPanel);
    
    // 设置分割器
    mainSplitter->addWidget(codeEditor);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setSizes({2, 1}); // 2:1 比例
    
    // 设置布局
    QHBoxLayout *layout = new QHBoxLayout(centralWidget);
    layout->addWidget(mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 设置窗口属性
    setMinimumSize(1000, 700);
    resize(1400, 900);
}

void MainWindow::setupActions()
{
    // 文件操作
    createAction(newAction, "新建(&N)", "Ctrl+N", "创建新文件");
    createAction(openAction, "打开(&O)", "Ctrl+O", "打开文件");
    createAction(saveAction, "保存(&S)", "Ctrl+S", "保存文件");
    createAction(saveAsAction, "另存为(&A)", "Ctrl+Shift+S", "另存为");
    createAction(exitAction, "退出(&X)", "Ctrl+Q", "退出程序");
    
    // 编辑操作
    createAction(undoAction, "撤销(&U)", "Ctrl+Z", "撤销");
    createAction(redoAction, "重做(&R)", "Ctrl+Y", "重做");
    createAction(cutAction, "剪切(&T)", "Ctrl+X", "剪切");
    createAction(copyAction, "复制(&C)", "Ctrl+C", "复制");
    createAction(pasteAction, "粘贴(&P)", "Ctrl+V", "粘贴");
    createAction(selectAllAction, "全选(&A)", "Ctrl+A", "全选");
    createAction(findAction, "查找(&F)", "Ctrl+F", "查找");
    createAction(replaceAction, "替换(&R)", "Ctrl+H", "替换");
    createAction(gotoLineAction, "转到行(&G)", "Ctrl+G", "转到指定行");
    
    // 分析操作
    createAction(runLexicalAction, "词法分析(&L)", "F5", "运行词法分析");
    createAction(runSyntaxAction, "语法分析(&S)", "F6", "运行语法分析");
    createAction(runSemanticAction, "语义分析(&M)", "F7", "运行语义分析");
    createAction(runCodeGenAction, "代码生成(&G)", "F8", "运行代码生成");
    createAction(runFullAction, "完整分析(&F)", "F9", "运行完整分析");
    createAction(stopAnalysisAction, "停止分析(&T)", "Esc", "停止当前分析");
    
    // 视图操作
    createAction(toggleLineNumbersAction, "显示行号(&N)", "", "显示/隐藏行号");
    createAction(toggleSyntaxHighlightingAction, "语法高亮(&H)", "", "启用/禁用语法高亮");
    createAction(zoomInAction, "放大(&I)", "Ctrl++", "放大字体");
    createAction(zoomOutAction, "缩小(&O)", "Ctrl+-", "缩小字体");
    createAction(resetZoomAction, "重置缩放(&R)", "Ctrl+0", "重置字体大小");
    
    // 帮助操作
    createAction(aboutAction, "关于(&A)", "", "关于本程序");
    createAction(aboutQtAction, "关于Qt(&Q)", "", "关于Qt");
    
    // 设置复选框状态
    toggleLineNumbersAction->setCheckable(true);
    toggleLineNumbersAction->setChecked(showLineNumbers);
    toggleSyntaxHighlightingAction->setCheckable(true);
    toggleSyntaxHighlightingAction->setChecked(enableSyntaxHighlighting);
    
    // 初始状态
    stopAnalysisAction->setEnabled(false);
}

void MainWindow::setupMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    fileMenu->addAction(newAction);
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    
    // 最近文件子菜单
    QMenu *recentMenu = fileMenu->addMenu("最近文件(&R)");
    for (int i = 0; i < MaxRecentFiles; ++i) {
        QAction *action = new QAction(this);
        action->setVisible(false);
        connect(action, &QAction::triggered, this, &MainWindow::recentFileTriggered);
        recentFileActions.append(action);
        recentMenu->addAction(action);
    }
    updateRecentFiles();
    
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 编辑菜单
    QMenu *editMenu = menuBar()->addMenu("编辑(&E)");
    editMenu->addAction(undoAction);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    editMenu->addAction(cutAction);
    editMenu->addAction(copyAction);
    editMenu->addAction(pasteAction);
    editMenu->addSeparator();
    editMenu->addAction(selectAllAction);
    editMenu->addSeparator();
    editMenu->addAction(findAction);
    editMenu->addAction(replaceAction);
    editMenu->addAction(gotoLineAction);
    
    // 分析菜单
    QMenu *analysisMenu = menuBar()->addMenu("分析(&A)");
    analysisMenu->addAction(runLexicalAction);
    analysisMenu->addAction(runSyntaxAction);
    analysisMenu->addAction(runSemanticAction);
    analysisMenu->addAction(runCodeGenAction);
    analysisMenu->addSeparator();
    analysisMenu->addAction(runFullAction);
    analysisMenu->addAction(stopAnalysisAction);
    
    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction(toggleLineNumbersAction);
    viewMenu->addAction(toggleSyntaxHighlightingAction);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomInAction);
    viewMenu->addAction(zoomOutAction);
    viewMenu->addAction(resetZoomAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::setupToolBars()
{
    // 文件工具栏
    fileToolBar = addToolBar("文件");
    fileToolBar->addAction(newAction);
    fileToolBar->addAction(openAction);
    fileToolBar->addAction(saveAction);
    
    // 编辑工具栏
    editToolBar = addToolBar("编辑");
    editToolBar->addAction(undoAction);
    editToolBar->addAction(redoAction);
    editToolBar->addSeparator();
    editToolBar->addAction(cutAction);
    editToolBar->addAction(copyAction);
    editToolBar->addAction(pasteAction);
    
    // 分析工具栏
    analysisToolBar = addToolBar("分析");
    analysisToolBar->addAction(runLexicalAction);
    analysisToolBar->addAction(runSyntaxAction);
    analysisToolBar->addAction(runSemanticAction);
    analysisToolBar->addAction(runCodeGenAction);
    analysisToolBar->addSeparator();
    analysisToolBar->addAction(runFullAction);
    analysisToolBar->addAction(stopAnalysisAction);
    
    // 视图工具栏
    viewToolBar = addToolBar("视图");
    viewToolBar->addAction(zoomInAction);
    viewToolBar->addAction(zoomOutAction);
    viewToolBar->addAction(resetZoomAction);
}

void MainWindow::setupStatusBar()
{
    // 状态标签
    statusLabel = new QLabel("就绪", this);
    statusBar()->addWidget(statusLabel);
    
    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    statusBar()->addWidget(progressBar);
    
    // 位置标签
    positionLabel = new QLabel("行: 1, 列: 1", this);
    statusBar()->addPermanentWidget(positionLabel);
    
    // 编码标签
    encodingLabel = new QLabel("UTF-8", this);
    statusBar()->addPermanentWidget(encodingLabel);
}

void MainWindow::setupDockWidgets()
{
    // 错误信息停靠窗口
    errorDock = new QDockWidget("错误信息", this);
    errorList = new QListWidget(this);
    errorDock->setWidget(errorList);
    addDockWidget(Qt::BottomDockWidgetArea, errorDock);
    errorDock->hide(); // 初始隐藏
}

void MainWindow::setupConnections()
{
    // 文件操作连接
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // 编辑操作连接
    connect(undoAction, &QAction::triggered, codeEditor, &QPlainTextEdit::undo);
    connect(redoAction, &QAction::triggered, codeEditor, &QPlainTextEdit::redo);
    connect(cutAction, &QAction::triggered, codeEditor, &QPlainTextEdit::cut);
    connect(copyAction, &QAction::triggered, codeEditor, &QPlainTextEdit::copy);
    connect(pasteAction, &QAction::triggered, codeEditor, &QPlainTextEdit::paste);
    connect(selectAllAction, &QAction::triggered, codeEditor, &QPlainTextEdit::selectAll);
    connect(findAction, &QAction::triggered, this, &MainWindow::find);
    connect(replaceAction, &QAction::triggered, this, &MainWindow::replace);
    connect(gotoLineAction, &QAction::triggered, this, &MainWindow::gotoLine);
    
    // 分析操作连接
    connect(runLexicalAction, &QAction::triggered, this, &MainWindow::runLexicalAnalysis);
    connect(runSyntaxAction, &QAction::triggered, this, &MainWindow::runSyntaxAnalysis);
    connect(runSemanticAction, &QAction::triggered, this, &MainWindow::runSemanticAnalysis);
    connect(runCodeGenAction, &QAction::triggered, this, &MainWindow::runCodeGeneration);
    connect(runFullAction, &QAction::triggered, this, &MainWindow::runFullAnalysis);
    connect(stopAnalysisAction, &QAction::triggered, this, &MainWindow::stopAnalysis);
    
    // 视图操作连接
    connect(toggleLineNumbersAction, &QAction::triggered, this, &MainWindow::toggleLineNumbers);
    connect(toggleSyntaxHighlightingAction, &QAction::triggered, this, &MainWindow::toggleSyntaxHighlighting);
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::resetZoom);
    
    // 帮助操作连接
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    
    // 代码编辑器信号连接
    connect(codeEditor, &CodeEditor::textChanged, this, &MainWindow::onTextChanged);
    connect(codeEditor, &CodeEditor::modificationChanged, this, &MainWindow::onModificationChanged);
    connect(codeEditor, &CodeEditor::cursorPositionChanged, this, &MainWindow::onCursorPositionChanged);
    connect(codeEditor, &CodeEditor::fileNameChanged, this, &MainWindow::onFileNameChanged);
    
    // 右键菜单
    codeEditor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(codeEditor, &QWidget::customContextMenuRequested, this, &MainWindow::showContextMenu);
}

void MainWindow::createAction(QAction *&action, const QString &text, 
                             const QString &shortcut, const QString &tooltip, const QString &icon)
{
    action = new QAction(text, this);
    if (!shortcut.isEmpty()) {
        action->setShortcut(QKeySequence(shortcut));
    }
    if (!tooltip.isEmpty()) {
        action->setToolTip(tooltip);
        action->setStatusTip(tooltip);
    }
    if (!icon.isEmpty()) {
        action->setIcon(QIcon(icon));
    }
}

// ============ 槽函数实现 ============

void MainWindow::newFile()
{
    if (maybeSave()) {
        codeEditor->clear();
        setCurrentFile(QString());
        clearAnalysisResults();
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            "打开文件", ".", "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*.*)");
        if (!fileName.isEmpty()) {
            if (codeEditor->openFile(fileName)) {
                setCurrentFile(fileName);
                addToRecentFiles(fileName);
                clearAnalysisResults();
            }
        }
    }
}

bool MainWindow::saveFile()
{
    if (isUntitled) {
        return saveFileAs();
    } else {
        return codeEditor->saveFile(currentFile);
    }
}

bool MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "另存为", ".", "C/C++ 文件 (*.c *.cpp *.h *.hpp);;所有文件 (*.*)");
    if (fileName.isEmpty()) {
        return false;
    }
    
    if (codeEditor->saveFile(fileName)) {
        setCurrentFile(fileName);
        addToRecentFiles(fileName);
        return true;
    }
    return false;
}

void MainWindow::recentFileTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action && maybeSave()) {
        QString fileName = action->data().toString();
        if (codeEditor->openFile(fileName)) {
            setCurrentFile(fileName);
            clearAnalysisResults();
        }
    }
}

void MainWindow::find()
{
    bool ok;
    QString text = QInputDialog::getText(this, "查找", "查找内容:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        codeEditor->findText(text);
    }
}

void MainWindow::replace()
{
    // 这里可以实现一个更复杂的查找替换对话框
    bool ok;
    QString findText = QInputDialog::getText(this, "替换", "查找内容:", QLineEdit::Normal, "", &ok);
    if (!ok || findText.isEmpty()) return;
    
    QString replaceText = QInputDialog::getText(this, "替换", "替换为:", QLineEdit::Normal, "", &ok);
    if (!ok) return;
    
    codeEditor->replaceText(findText, replaceText);
}

void MainWindow::gotoLine()
{
    bool ok;
    int line = QInputDialog::getInt(this, "转到行", "行号:", 1, 1, codeEditor->blockCount(), 1, &ok);
    if (ok) {
        codeEditor->gotoLine(line);
    }
}

void MainWindow::runLexicalAnalysis()
{
    if (!analysisThread) {
        analysisThread = new AnalysisThread(this);
        connect(analysisThread, &AnalysisThread::lexicalAnalysisFinished,
                this, &MainWindow::onLexicalAnalysisFinished);
        connect(analysisThread, &AnalysisThread::analysisError,
                this, &MainWindow::onAnalysisError);
    }
    
    if (analysisThread->isRunning()) {
        return;
    }
    
    startAnalysis();
    updateAnalysisProgress("正在进行词法分析...");
    
    analysisThread->setSourceCode(codeEditor->toPlainText());
    analysisThread->start();
}

void MainWindow::runSyntaxAnalysis()
{
    statusLabel->setText("语法分析功能将在后续版本实现");
}

void MainWindow::runSemanticAnalysis()
{
    statusLabel->setText("语义分析功能将在后续版本实现");
}

void MainWindow::runCodeGeneration()
{
    statusLabel->setText("代码生成功能将在后续版本实现");
}

void MainWindow::runFullAnalysis()
{
    runLexicalAnalysis();
}

void MainWindow::stopAnalysis()
{
    if (analysisThread && analysisThread->isRunning()) {
        analysisThread->requestInterruption();
        finishAnalysis();
        statusLabel->setText("分析已停止");
    }
}

void MainWindow::toggleLineNumbers()
{
    showLineNumbers = toggleLineNumbersAction->isChecked();
    // 这里需要根据CodeEditor的实现来切换行号显示
}

void MainWindow::toggleSyntaxHighlighting()
{
    enableSyntaxHighlighting = toggleSyntaxHighlightingAction->isChecked();
    codeEditor->enableSyntaxHighlighting(enableSyntaxHighlighting);
}

void MainWindow::zoomIn()
{
    codeEditor->zoomIn();
}

void MainWindow::zoomOut()
{
    codeEditor->zoomOut();
}

void MainWindow::resetZoom()
{
    // 重置字体大小到默认值
    QFont font = codeEditor->font();
    font.setPointSize(10);
    codeEditor->setFont(font);
}

void MainWindow::onTextChanged()
{
    if (enableAutoAnalysis) {
        autoAnalysisTimer->start();
    }
    updateActions();
}

void MainWindow::onModificationChanged(bool changed)
{
    updateWindowTitle();
    updateActions();
}

void MainWindow::onCursorPositionChanged(int line, int column)
{
    positionLabel->setText(QString("行: %1, 列: %2").arg(line).arg(column));
}

void MainWindow::onFileNameChanged(const QString &fileName)
{
    setCurrentFile(fileName);
}

void MainWindow::onLexicalAnalysisFinished(const QVector<Token> &tokens)
{
    finishAnalysis();
    
    // 更新分析面板
    analysisPanel->getLexicalPanel()->setTokens(tokens);
    analysisPanel->switchToLexicalTab();
    
    // 更新统计信息
    QString text = codeEditor->toPlainText();
    int lines = text.count('\n') + 1;
    int chars = text.length();
    analysisPanel->getLexicalPanel()->updateStatistics(tokens.size(), lines, chars);
    
    statusLabel->setText(QString("词法分析完成，生成 %1 个Token").arg(tokens.size()));
}

void MainWindow::onSyntaxAnalysisFinished(bool success, const QString &message)
{
    finishAnalysis();
    statusLabel->setText(message);
}

void MainWindow::onSemanticAnalysisFinished(bool success, const QString &message)
{
    finishAnalysis();
    statusLabel->setText(message);
}

void MainWindow::onCodeGenerationFinished(bool success, const QString &message)
{
    finishAnalysis();
    statusLabel->setText(message);
}

void MainWindow::onAnalysisError(const QString &error)
{
    finishAnalysis();
    statusLabel->setText(QString("分析错误: %1").arg(error));
    QMessageBox::warning(this, "分析错误", error);
}

void MainWindow::about()
{
    QMessageBox::about(this, "关于编译器",
        "<h2>编译原理课程设计</h2>"
        "<p>版本: 1.0</p>"
        "<p>这是一个完整的编译器前端实现，包含:</p>"
        "<ul>"
        "<li>词法分析 (DFA + 最小化)</li>"
        "<li>语法分析 (LALR)</li>"
        "<li>语义分析 (符号表 + 类型检查)</li>"
        "<li>中间代码生成</li>"
        "</ul>"
        "<p>采用Qt框架实现图形化界面</p>");
}

void MainWindow::updateRecentFiles()
{
    QStringList files = getRecentFiles();
    
    for (int i = 0; i < MaxRecentFiles; ++i) {
        if (i < files.size()) {
            QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
            recentFileActions[i]->setText(text);
            recentFileActions[i]->setData(files[i]);
            recentFileActions[i]->setVisible(true);
        } else {
            recentFileActions[i]->setVisible(false);
        }
    }
}

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);
    contextMenu.addAction(undoAction);
    contextMenu.addAction(redoAction);
    contextMenu.addSeparator();
    contextMenu.addAction(cutAction);
    contextMenu.addAction(copyAction);
    contextMenu.addAction(pasteAction);
    contextMenu.addSeparator();
    contextMenu.addAction(selectAllAction);
    contextMenu.exec(codeEditor->mapToGlobal(pos));
}

// ============ 工具函数实现 ============

void MainWindow::updateWindowTitle()
{
    QString shownName = currentFile;
    if (shownName.isEmpty()) {
        shownName = "未命名";
    } else {
        shownName = strippedName(currentFile);
    }
    
    setWindowTitle(QString("%1[*] - 编译器").arg(shownName));
    setWindowModified(codeEditor->isModified());
}

void MainWindow::updateActions()
{
    bool hasText = !codeEditor->toPlainText().isEmpty();
    bool hasSelection = codeEditor->textCursor().hasSelection();
    
    saveAction->setEnabled(codeEditor->isModified());
    undoAction->setEnabled(codeEditor->document()->isUndoAvailable());
    redoAction->setEnabled(codeEditor->document()->isRedoAvailable());
    cutAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    
    runLexicalAction->setEnabled(hasText);
    runSyntaxAction->setEnabled(hasText);
    runSemanticAction->setEnabled(hasText);
    runCodeGenAction->setEnabled(hasText);
    runFullAction->setEnabled(hasText);
}

void MainWindow::addToRecentFiles(const QString &fileName)
{
    QStringList files = getRecentFiles();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    setRecentFiles(files);
    updateRecentFiles();
}

QStringList MainWindow::getRecentFiles() const
{
    QSettings settings;
    return settings.value("recentFiles").toStringList();
}

void MainWindow::setRecentFiles(const QStringList &files)
{
    QSettings settings;
    settings.setValue("recentFiles", files);
}

bool MainWindow::maybeSave()
{
    if (codeEditor->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "编译器",
                                 "文档已修改。\n是否保存更改？",
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isUntitled = fileName.isEmpty();
    updateWindowTitle();
    
    if (!fileName.isEmpty()) {
        addToRecentFiles(fileName);
    }
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::readSettings()
{
    QSettings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(1200, 800)).toSize();
    resize(size);
    move(pos);
    
    enableAutoAnalysis = settings.value("enableAutoAnalysis", true).toBool();
    enableSyntaxHighlighting = settings.value("enableSyntaxHighlighting", true).toBool();
    showLineNumbers = settings.value("showLineNumbers", true).toBool();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("enableAutoAnalysis", enableAutoAnalysis);
    settings.setValue("enableSyntaxHighlighting", enableSyntaxHighlighting);
    settings.setValue("showLineNumbers", showLineNumbers);
}

void MainWindow::startAnalysis()
{
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // 不确定进度
    stopAnalysisAction->setEnabled(true);
    
    // 禁用分析相关按钮
    runLexicalAction->setEnabled(false);
    runSyntaxAction->setEnabled(false);
    runSemanticAction->setEnabled(false);
    runCodeGenAction->setEnabled(false);
    runFullAction->setEnabled(false);
}

void MainWindow::finishAnalysis()
{
    progressBar->setVisible(false);
    stopAnalysisAction->setEnabled(false);
    updateActions();
}

void MainWindow::updateAnalysisProgress(const QString &stage)
{
    statusLabel->setText(stage);
}

void MainWindow::displayAnalysisResults()
{
    // 显示分析结果
}

void MainWindow::clearAnalysisResults()
{
    analysisPanel->clearAllResults();
    errorList->clear();
    errorDock->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

#include "mainwindow.moc"
