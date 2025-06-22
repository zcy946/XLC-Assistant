#include <QApplication>
#include "Logger.hpp"
#include "MainWindow.h"
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#include "DataManager.h"

int main(int argc, char *argv[])
{
#if defined(_WIN32)
    // 开启控制台中文输入输出
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    _setmode(_fileno(stdin), _O_WTEXT);
#endif
    // 初始化日志
    Logger::init();

    // 开启高DPI支持
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication app(argc, argv);

    // // 注册元对象
    // DataManager::registerAllMetaType();

    // 设置全局字体
    app.setFont(getGlobalFont());

    MainWindow w;
    w.resize(800, 600);
    w.show();

    app.exec();
    return 0;
}