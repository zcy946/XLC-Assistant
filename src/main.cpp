#include <QApplication>
#include "Logger.hpp"
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // 初始化日志
    initLogger();
    // 开启高DPI支持
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    app.exec();
    return 0;
}