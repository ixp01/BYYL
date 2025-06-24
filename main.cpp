#include "src/gui/mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QDebug>
#include <QMessageBox>
#include <QMetaType>

// 注册自定义类型
#include "token.h"
#include "ast.h"
#include "symbol_table.h"
#include "intermediate_code.h"
#include "semantic_analyzer.h"

int main(int argc, char *argv[])
{
    // 创建应用程序
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Compiler Frontend");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("CS Course Design");
    app.setOrganizationDomain("cs.edu.cn");
    
    // Qt6线程兼容性设置
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6特定设置
    qDebug() << "运行在Qt6环境:" << qVersion();
#else
    // Qt5设置
    qDebug() << "运行在Qt5环境:" << qVersion();
#endif
    
    // 注册自定义类型用于信号槽
    qRegisterMetaType<QVector<Token>>("QVector<Token>");
    qRegisterMetaType<std::shared_ptr<ASTNode>>("std::shared_ptr<ASTNode>");
    // qRegisterMetaType<SymbolTable>("SymbolTable");  // 暂时注释，有复制构造问题
    qRegisterMetaType<QVector<SemanticError>>("QVector<SemanticError>");
    qRegisterMetaType<SemanticError>("SemanticError");
    qRegisterMetaType<QVector<ThreeAddressCode>>("QVector<ThreeAddressCode>");
    
    qDebug() << "自定义类型注册完成";
    
    try {
        // 创建主窗口
        qDebug() << "准备创建主窗口";
        MainWindow window;
        
        qDebug() << "主窗口创建成功，准备显示";
        window.show();
        
        qDebug() << "进入Qt事件循环";
        return app.exec();
        
    } catch (const std::exception &e) {
        qDebug() << "主程序异常:" << e.what();
        QMessageBox::critical(nullptr, "错误", 
                             QString("程序启动失败: %1").arg(e.what()));
        return -1;
    } catch (...) {
        qDebug() << "主程序未知异常";
        QMessageBox::critical(nullptr, "错误", "程序启动失败: 未知异常");
        return -1;
    }
}
