#include "ToastManager.h"
#include <QPainter>
#include "global.h"
#include <QPushButton>
#include "Logger.hpp"
#include <QSvgRenderer>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// Toast
Toast::Toast(Toast::Type type, const QString &message, int duration, QWidget *parent)
    : QObject(parent), m_type(type), m_message(message), m_duration(duration), m_timeoutOccurred(false)
{
    switch (m_type)
    {
    case Toast::Info:
        m_svgFilePath = QString("://image/info.svg");
        break;
    case Toast::Success:
        m_svgFilePath = QString("://image/success.svg");
        break;
    case Toast::Error:
        m_svgFilePath = QString("://image/error.svg");
        break;
    case Toast::Warning:
        m_svgFilePath = QString("://image/warning.svg");
        break;
    default:
        m_svgFilePath = QString("://image/info.svg");
        break;
    }

    // 初始化计时器
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(m_duration);
    connect(m_timer, &QTimer::timeout, this,
            [this]()
            {
                m_timeoutOccurred = true;
                Q_EMIT sig_requestExit(this);
            });
}

void Toast::startTimer()
{
    m_timer->start();
}

qreal Toast::getOpacity()
{
    return m_opacity;
}

void Toast::setOpacity(qreal opacity)
{
    m_opacity = opacity;
    Q_EMIT sig_requestUpdate();
}

// ToastManager
ToastManager *ToastManager::s_instance = nullptr;
ToastManager::ToastManager(QWidget *parent)
    : QWidget(parent)
{
    // 开启透明背景与鼠标穿透
    setAttribute(Qt::WA_TranslucentBackground);
    // setAttribute(Qt::WA_TransparentForMouseEvents);

    // 测试代码
    QPushButton *btn = new QPushButton("new message", this);
    btn->move(0, 0);
    connect(btn, &QPushButton::clicked, this,
            [this]()
            {
                showMessage(Toast::Info, "测试消息测试消息测试消息测试消息测试消息测试消息");
                showMessage(Toast::Type::Success, "测试消息");
                showMessage(Toast::Type::Warning, "测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息");
                showMessage(Toast::Type::Error, "测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息");
            });
    show();
}

ToastManager *ToastManager::getInstance()
{
    if (!s_instance)
    {
        s_instance = new ToastManager();
    }
    return s_instance;
}

void ToastManager::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font(getGlobalFont());
    QFontMetrics fontMetrics(font);

    int y = m_margin;
    for (int i = 0; i < m_toasts.size(); ++i)
    {
        Toast *toast = m_toasts.at(i);
        // 计算文本最大可占用宽度
        int maxTextWidth = m_width - m_margin * 2 - m_paddingH * 2 - m_spacingIconToText - m_offSetX - m_spread * 2;
        // 计算绘制文本所需最小区域
        QRect rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toast->m_message);
        // 计算背景区域
        int w = m_paddingH + m_widthIcon + m_spacingIconToText + rectText.width() + m_paddingH;
        int h = m_paddingV + rectText.height() + m_paddingV;
        int x = (m_width - w) / 2;
        QRect rectBackground(x, y, w, h);
        // 计算文本绘制区域
        QRect rectDrawText = rectText.adjusted(x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               y + m_paddingV,
                                               x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               y + m_paddingV);
        // 计算图标区域
        QRect rectIcon = QRect(rectBackground.topLeft() + QPoint(m_paddingH, m_paddingV), QSize(m_widthIcon, m_heightIcon));

        // 设置画笔透明度
        painter.setOpacity(toast->m_opacity);
        // 绘制阴影
        for (int j = 0; j < m_spread; ++j)
        {
            float ratio = 1.0f - float(j) / m_spread;
            int alpha = m_baseAlpha * ratio;
            QColor shadow = QColor(m_colorShadow);
            shadow.setAlpha(alpha);
            // 阴影矩形向外扩展 j 像素
            QRectF shadowRect = rectBackground.adjusted(-j + m_offSetX, -j + m_offSetY, j + m_offSetY, j + m_offSetY);
            QPainterPath path;
            // 阴影圆角半径随 j 增加
            path.addRoundedRect(shadowRect, m_radius + j, m_radius + j);
            // 绘制阴影层
            painter.setPen(QPen(shadow, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
        // 绘制背景
        painter.setBrush(QColor("#FFFFFF"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rectBackground, m_radius, m_radius);
        // 绘制图标
        QSvgRenderer svgRenderer(toast->m_svgFilePath);
        svgRenderer.render(&painter, rectIcon);
        // 绘制文本
        painter.setPen(Qt::black);
        painter.drawText(rectDrawText, toast->m_message);

        // 更新y坐标
        y += h + m_spacing;
    }
}

bool ToastManager::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget())
    {
        if (event->type() == QEvent::Resize)
        {
            m_width = parentWidget()->width() / 3;
            m_height = parentWidget()->height();
            int x = (parentWidget()->width() - m_width) / 2;
            int y = 0;
            setGeometry(x, y, m_width, m_height);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ToastManager::slot_onRequestExist(Toast *message)
{
    // 移除第一条消息
    if (m_toasts.isEmpty() || m_toasts.first() != message)
        return;
    Toast *exitMessage = m_toasts.first();

    QParallelAnimationGroup *pAnimationGroupFadeOut = new QParallelAnimationGroup(this);
    QPropertyAnimation *pAnimationSlideOut = new QPropertyAnimation();

    m_toasts.removeFirst();
    exitMessage->deleteLater();
    update();

    // 检查下一条消息是否需要被移除
    while (true)
    {
        if (m_toasts.isEmpty())
            return;
        Toast *nextMessage = m_toasts.first();
        if (!nextMessage->m_timeoutOccurred)
            break;
        m_toasts.removeFirst();
        nextMessage->deleteLater();
        update();
        XLC_LOG_DEBUG("Remove toast success");
    }
}

void ToastManager::slot_onRequestUpdate()
{
    update();
}

void ToastManager::init(QWidget *parent)
{
    if (!parent)
        return;
    setParent(parent);
    parent->installEventFilter(this);
    // 在主窗口中心位置占总宽度的三分之一
    m_width = parentWidget()->width() / 3;
    m_height = parentWidget()->height();
    int x = (parentWidget()->width() - m_width) / 2;
    int y = m_margin;
    setGeometry(x, y, m_width, m_height);
    show();
}

void ToastManager::showMessage(Toast::Type type, const QString &message, int duration)
{
    Toast *newMessage = new Toast(type, message, duration, parentWidget());
    m_toasts.append(newMessage);
    connect(newMessage, &Toast::sig_requestExit, this, &ToastManager::slot_onRequestExist);
    connect(newMessage, &Toast::sig_requestUpdate, this, &ToastManager::slot_onRequestUpdate);

    QPropertyAnimation *pAnimationOpacity = new QPropertyAnimation(newMessage, "m_opacity");
    pAnimationOpacity->setDuration(m_animationDuration);
    pAnimationOpacity->setEasingCurve(QEasingCurve::OutSine);
    pAnimationOpacity->setStartValue(0.0);
    pAnimationOpacity->setEndValue(1.0);
    pAnimationOpacity->start(QAbstractAnimation::DeleteWhenStopped);

    newMessage->startTimer();
    update();
}
