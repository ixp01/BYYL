#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 显示Qt版本信息
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Runtime Qt Version:" << qVersion();
    
    QMainWindow window;
    QLabel *label = new QLabel("Qt6兼容性测试成功！", &window);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px; padding: 50px;");
    
    window.setCentralWidget(label);
    window.setWindowTitle("CompilerFrontend Qt6兼容性测试");
    window.resize(600, 200);
    window.show();
    
    return app.exec();
} 