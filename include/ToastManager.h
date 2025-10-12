#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QPaintEvent>

class Toast : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal m_opacity READ getOpacity WRITE setOpacity)
    Q_PROPERTY(qreal m_renderY READ getRenderY WRITE setRenderY)

Q_SIGNALS:
    void sig_requestExit(Toast *message);
    void sig_requestUpdate();

public:
    enum Type
    {
        Info,
        Success,
        Warning,
        Error
    };

public:
    Toast(Toast::Type type, const QString &message, int duration, QWidget *parent);
    ~Toast() = default;
    // 开始倒计时
    void startTimer();
    qreal getOpacity();
    void setOpacity(qreal opacity);
    qreal getRenderY();
    void setRenderY(qreal renderY);

public:
    Type m_type;            // 消息类型
    QString m_message;      // 消息体
    QString m_svgFilePath;  // 图标文件路径
    int m_duration;         // 显示时间
    bool m_timeoutOccurred; // 是否已经超时
    qreal m_opacity = 1.0;  // 透明度
    qreal m_renderY = 0.0;  // Y轴绘制偏移量
    QTimer *m_timer;
};
Q_DECLARE_METATYPE(Toast::Type)

class ToastManager : public QWidget
{
    Q_OBJECT

public Q_SLOTS:
    // 显示消息
    void slot_showMessage(Toast::Type type, const QString &message, int duration);

private Q_SLOTS:
    void slot_onRequestExist(Toast *message);
    void slot_onRequestUpdate();

public:
    static ToastManager *getInstance();
    ~ToastManager() = default;
    // 初始化
    void init(QWidget *parent);
    static void showMessage(Toast::Type type, const QString &message, int duration = 3000);

private:
    explicit ToastManager(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static ToastManager *s_instance;
    QList<Toast *> m_toasts;
    int m_width;                           // 宽度
    int m_height;                          // 高度
    int m_margin = 10;                     // 外边距
    int m_spacing = 10;                    // 消息间距
    int m_paddingH = 18;                   // 内水平边距
    int m_paddingV = 10;                   // 内垂直边距
    int m_radius = 5;                      // 圆角半径
    int m_widthIcon = 20;                  // 图标宽度
    int m_heightIcon = 20;                 // 图标高度
    int m_spacingIconToText = 10;          // 图标到文本的间距
    int m_spread = 4;                      // 阴影扩散层数
    int m_baseAlpha = 50;                  // 阴影起始透明度
    QString m_colorBackground = "#FFFFFF"; // 背景颜色
    QString m_colorShadow = "#000000";     // 阴影颜色
    int m_offSetX = 0;                     // 阴影水平偏移
    int m_offSetY = 2;                     // 阴影垂直偏移
    int m_animationDuration = 350;         // 动画持续时间
    bool m_isAnimating = false;            // 是否正在进行退出动画
};

#endif // TOASTMANAGER_H