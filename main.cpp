#include "src/gui/mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    a.setApplicationName("Compiler Frontend");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("CS Course Design");
    
    MainWindow w;
    w.show();
    
    return a.exec();
}
