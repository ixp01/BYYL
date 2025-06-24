#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QDebug>
#include <QTextEdit>

class SimpleMainWindow : public QMainWindow
{
public:
    SimpleMainWindow(QWidget *parent = nullptr) : QMainWindow(parent)
    {
        qDebug() << "SimpleMainWindow构造函数开始";
        
        QTextEdit *editor = new QTextEdit(this);
        editor->setText("Hello CompilerFrontend!");
        setCentralWidget(editor);
        
        setWindowTitle("CompilerFrontend - 调试版本");
        resize(800, 600);
        
        qDebug() << "SimpleMainWindow构造函数完成";
    }
};

int main(int argc, char *argv[])
{
    qDebug() << "程序开始";
    
    QApplication app(argc, argv);
    
    qDebug() << "QApplication创建完成";
    
    app.setApplicationName("Compiler Frontend Debug");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("CS Course Design");
    
    qDebug() << "准备创建窗口";
    
    SimpleMainWindow window;
    
    qDebug() << "窗口创建完成，准备显示";
    
    window.show();
    
    qDebug() << "窗口显示完成，进入事件循环";
    
    return app.exec();
} 