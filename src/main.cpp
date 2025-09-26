#include <QApplication>
#include "Logger.hpp"
#include "MainWindow.h"
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif
#include "DataManager.h"

// int main(int argc, char *argv[])
// {
// #if defined(_WIN32)
//     // 开启控制台中文输入输出
//     SetConsoleCP(CP_UTF8);
//     SetConsoleOutputCP(CP_UTF8);
//     _setmode(_fileno(stdin), _O_WTEXT);
// #endif
//     // 初始化日志
//     Logger::init();

//     // 开启高DPI支持
// #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
//     QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
// #endif
// #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
//     QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//     QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
// #endif

//     QApplication app(argc, argv);

//     // // 注册元对象
//     // DataManager::registerAllMetaType();

//     // 设置全局字体
//     app.setFont(getGlobalFont());

//     MainWindow w;
//     w.resize(1200, 700);
//     w.show();

//     app.exec();
//     return 0;
// }

#include "CMessageListWidget.h"

int main(int argc, char *argv[])
{
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

    QApplication a(argc, argv);
    // 设置全局字体
    a.setFont(getGlobalFont());

    QWidget window;
    QVBoxLayout *mainLayout = new QVBoxLayout(&window);

    CMessageListWidget *listView = new CMessageListWidget(&window);
    

    mainLayout->addWidget(listView);

    QLineEdit *inputLine = new QLineEdit(&window);
    QPushButton *sendButton = new QPushButton("Send", &window);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(inputLine);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);

    QObject::connect(sendButton, &QPushButton::clicked,
                     [&]()
                     {
                         QString messageText = inputLine->text();
                         if (!messageText.isEmpty())
                         {
                             CMessage msg(messageText, CMessage::USER, "://image/avatar_self.png");
                             listView->addMessage(msg);
                             inputLine->clear();
                             listView->scrollToBottom(); // Scroll to the latest message
                         }
                     });

    for (int i = 0; i < 50; ++i)
    {
        QString text = QString("%1 - 测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本测试超长文本").arg(i + 1);
        listView->addMessage(CMessage(text, CMessage::USER));
    }

    window.resize(400, 600);
    window.setWindowTitle("QListView Chat Demo");
    window.show();

    return a.exec();
}