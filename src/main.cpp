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
    CMessageListModel *chatModel = new CMessageListModel(&window);
    CMessageDelegate *chatDelegate = new CMessageDelegate(&window);

    listView->setModel(chatModel);
    listView->setItemDelegate(chatDelegate);
    listView->setSelectionMode(QAbstractItemView::NoSelection);                        // No selection
    listView->setUniformItemSizes(false);                                              // Crucial for variable height items
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);                      // No editing
    listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);                // Smooth scroll
    listView->setStyleSheet("QListView { background-color: #F5F5F5; border: none; }"); // Chat background

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
                             chatModel->addMessage(msg);
                             inputLine->clear();
                             listView->scrollToBottom(); // Scroll to the latest message
                         }
                     });

    // Add some initial messages
    chatModel->addMessage(CMessage("Hello there!", CMessage::ASSISTANT, "://image/avatar_robot.png")); // Replace with other avatar path
    chatModel->addMessage(CMessage("Hi! How are you?", CMessage::USER, "://image/avatar_self.png"));
    chatModel->addMessage(CMessage("I'm doing great, thank you! I have a very long message that should test the wrapping and resizing of the bubble. This message should definitely wrap to multiple lines to ensure the height calculation is correct.", CMessage::ASSISTANT, "://image/avatar_robot.png"));
    chatModel->addMessage(CMessage("That's good to hear. Here is a shorter message.", CMessage::USER, "://image/avatar_self.png"));
    chatModel->addMessage(CMessage("This is another message from the other person.", CMessage::ASSISTANT, "://image/avatar_robot.png"));

    window.resize(400, 600);
    window.setWindowTitle("QListView Chat Demo");
    window.show();

    return a.exec();
}