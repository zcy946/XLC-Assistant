#include <QApplication>
#include "Logger.hpp"
#include "MainWindow.h"
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdin), _O_WTEXT); // wide character input mode
#endif
    // 初始化日志
    initLogger();
    // 开启高DPI支持
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    MainWindow w;
    w.resize(600, 400);
    w.show();
    app.exec();
    return 0;
}