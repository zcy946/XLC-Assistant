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
#include "XlcNavigationBar.h"

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

    // 注册自定义类型
    qRegisterMetaType<Toast::Type>("Toast::Type");

    // 设置自定义样式
    app.setStyle(new XlcStyle());
    app.setPalette(ColorRepository::standardPalette());

    // 设置全局字体
    app.setFont(getGlobalFont());

    // MainWindow w;
    // w.resize(1200, 700);
    // w.show();

    XlcNavigationBar *xlcNavigationBar = new XlcNavigationBar();
    xlcNavigationBar->addNavigationNode("超长文本超长文本超长文本测试1", QChar(0xf0f0), "target_测试1");
    xlcNavigationBar->addNavigationNode("测试2", QChar(0xf0f0));
    xlcNavigationBar->addNavigationNode("测试2-1", QChar(0xf0f0), "target_测试2-1", "测试2");
    xlcNavigationBar->addNavigationNode("超长文本超长文本超长文本超长文本超长文本超长文本超长文本超长文本测试2-2", QChar(0xf0f0), QString(), "测试2");
    xlcNavigationBar->addNavigationNode("测试2-2-1", QChar(0xf0f0), "target_测试2-2-1", "超长文本超长文本超长文本超长文本超长文本超长文本超长文本超长文本测试2-2");
    QObject::connect(xlcNavigationBar, &XlcNavigationBar::sig_currentItemChanged,
            [](const QString &targetId)
            {
                XLC_LOG_DEBUG("jump to {}", targetId);
            });
    xlcNavigationBar->resize(300, 800);
    xlcNavigationBar->show();

    app.exec();
    return 0;
}