#include <QApplication>
#include <QMainWindow>
#include <QTextEdit>
#include <QSplitter>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QDebug>

class TestMainWindow : public QMainWindow
{
public:
    TestMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        qDebug() << "TestMainWindow构造函数开始";
        
        try {
            // 创建中央部件
            QWidget *centralWidget = new QWidget(this);
            setCentralWidget(centralWidget);
            qDebug() << "中央部件创建成功";
            
            // 创建分割器
            QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
            qDebug() << "分割器创建成功";
            
            // 创建左侧编辑器
            QTextEdit *leftEditor = new QTextEdit(this);
            leftEditor->setText("左侧：代码编辑器区域");
            mainSplitter->addWidget(leftEditor);
            qDebug() << "左侧编辑器创建成功";
            
            // 创建右侧分析面板
            QTextEdit *rightPanel = new QTextEdit(this);
            rightPanel->setText("右侧：分析结果面板");
            mainSplitter->addWidget(rightPanel);
            qDebug() << "右侧面板创建成功";
            
            // 设置布局
            QHBoxLayout *layout = new QHBoxLayout(centralWidget);
            layout->addWidget(mainSplitter);
            layout->setContentsMargins(0, 0, 0, 0);
            qDebug() << "布局设置成功";
            
            // 创建菜单栏
            QMenuBar *mBar = this->menuBar();
            QMenu *fileMenu = mBar->addMenu("文件");
            fileMenu->addAction("新建");
            fileMenu->addAction("打开");
            qDebug() << "菜单栏创建成功";
            
            // 创建状态栏
            QStatusBar *sBar = this->statusBar();
            sBar->addWidget(new QLabel("就绪"));
            qDebug() << "状态栏创建成功";
            
            // 设置窗口属性
            setWindowTitle("CompilerFrontend - 测试版本");
            setMinimumSize(800, 600);
            resize(1200, 800);
            qDebug() << "窗口属性设置成功";
            
        } catch (const std::exception &e) {
            qDebug() << "构造函数异常:" << e.what();
        } catch (...) {
            qDebug() << "构造函数未知异常";
        }
        
        qDebug() << "TestMainWindow构造函数完成";
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qDebug() << "应用程序创建成功";
    
    TestMainWindow window;
    window.show();
    
    qDebug() << "进入事件循环";
    return app.exec();
} 