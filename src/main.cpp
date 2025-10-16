#include <QApplication>
#include "Logger.hpp"
#include "MainWindow.h"
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#include "DataManager.h"
#include "ToastManager.h"
#include "XlcStyle.h"
#include "ColorRepository.h"
#include <QScreen>

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

    qputenv("QT_LOGGING_RULES", "qt.qpa.backingstore.debug=true");

    QApplication app(argc, argv);

    // 注册自定义类型
    qRegisterMetaType<Toast::Type>("Toast::Type");

    // 设置自定义样式
    app.setStyle(new XlcStyle());
    app.setPalette(ColorRepository::standardPalette());

    // 设置全局字体
    app.setFont(getGlobalFont());

    MainWindow w;
    // w.resize(1200, 700);
    // 获取屏幕可用区域
    QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();

    // 设置合理的窗口尺寸约束
    w.setMinimumSize(400, 300);
    w.setMaximumSize(screenGeometry.width(), screenGeometry.height());

    // 确保初始尺寸合理
    int initWidth = qMin(1200, screenGeometry.width() - 100);
    int initHeight = qMin(700, screenGeometry.height() - 100);
    w.resize(initWidth, initHeight);
    w.show();

    app.exec();
    return 0;
}