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

qreal Toast::getRenderY()
{
    return m_renderY;
}

void Toast::setRenderY(qreal renderY)
{
    m_renderY = renderY;
    Q_EMIT sig_requestUpdate();
}

bool Toast::isAnimatingExit()
{
    return m_isAnimatingExit;
}

void Toast::setAnimatingExit(bool state)
{
    m_isAnimatingExit = state;
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
                // showMessage(Toast::Type::Success, "测试消息");
                // showMessage(Toast::Type::Warning, "测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息");
                // showMessage(Toast::Type::Error, "测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息测试消息");
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

        // 计算当前 Toast 的实际绘制 Y 坐标
        // y 是堆叠位置，m_renderY 是动画带来的偏移量
        qreal currentDrawY = y + toast->m_renderY;

        // 计算文本最大可占用宽度
        int maxTextWidth = m_width - m_margin * 2 - m_paddingH * 2 - m_spacingIconToText - m_offSetX - m_spread * 2;
        // 计算绘制文本所需最小区域
        QRect rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toast->m_message);
        // 计算背景区域
        int w = m_paddingH + m_widthIcon + m_spacingIconToText + rectText.width() + m_paddingH;
        int h = m_paddingV + rectText.height() + m_paddingV;
        int x = (m_width - w) / 2;

        // 用 currentDrawY 定义背景区域
        QRect rectBackground(x, currentDrawY, w, h);

        // 使用 currentDrawY 定义文本绘制区域的 Y 坐标
        QRect rectDrawText = rectText.adjusted(x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               currentDrawY + m_paddingV,
                                               x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               currentDrawY + m_paddingV);

        // rectIcon 由于使用了 rectBackground.topLeft()，会自动应用 currentDrawY
        QRect rectIcon = QRect(rectBackground.topLeft() + QPoint(m_paddingH, m_paddingV), QSize(m_widthIcon, m_heightIcon));

        // 设置画笔透明度
        painter.setOpacity(toast->m_opacity);

        // 绘制阴影 (rectBackground 已使用 currentDrawY)
        for (int j = 0; j < m_spread; ++j)
        {
            float ratio = 1.0f - float(j) / m_spread;
            int alpha = m_baseAlpha * ratio;
            QColor shadow = QColor(m_colorShadow);
            shadow.setAlpha(alpha);

            // 阴影矩形向外扩展 j 像素
            // 注意：rectBackground 已经是 QRect，需要转换为 QRectF 进行 adjusted 操作，但 Qt 会自动转换
            QRectF shadowRect = rectBackground.adjusted(-j + m_offSetX, -j + m_offSetY, j + m_offSetY, j + m_offSetY);
            QPainterPath path;
            // 阴影圆角半径随 j 增加
            path.addRoundedRect(shadowRect, m_radius + j, m_radius + j);
            // 绘制阴影层
            painter.setPen(QPen(shadow, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }

        // 绘制背景 (rectBackground 已使用 currentDrawY)
        painter.setBrush(QColor("#FFFFFF"));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rectBackground, m_radius, m_radius);

        // 绘制图标 (rectIcon 已使用 currentDrawY)
        QSvgRenderer svgRenderer(toast->m_svgFilePath);
        svgRenderer.render(&painter, rectIcon);

        // 绘制文本 (rectDrawText 已使用 currentDrawY)
        painter.setPen(Qt::black);
        painter.drawText(rectDrawText, Qt::AlignLeft | Qt::AlignVCenter, toast->m_message);

        // 核心：更新下一条消息的堆叠基准 Y 坐标（不受当前动画偏移影响）
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

void ToastManager::slot_onRequestExist(Toast *toastToExit)
{
    // 1. 检查是否正在动画中或队列中第一个不是它
    if (toastToExit->isAnimatingExit() || m_toasts.isEmpty() || m_toasts.first() != toastToExit)
        return;

    // 标记为正在动画，防止重复触发
    toastToExit->setAnimatingExit(true);

    QFont font(getGlobalFont());
    QFontMetrics fontMetrics(font);
    int maxTextWidth = m_width - m_margin * 2 - m_paddingH * 2 - m_spacingIconToText - m_offSetX - m_spread * 2;
    // 计算绘制文本所需最小区域
    QRect rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toastToExit->m_message);
    const qreal SLIDE_DISTANCE = -(m_paddingV + rectText.height() + m_paddingV + m_spacing); // 上移像素

    // 2. 创建并行动画组
    QParallelAnimationGroup *group = new QParallelAnimationGroup(toastToExit);

    // a) Y 轴滑动动画 ("renderY")
    QPropertyAnimation *slideAnim = new QPropertyAnimation(toastToExit, "m_renderY");
    slideAnim->setDuration(m_animationDuration);
    slideAnim->setStartValue(0.0);          // 从无偏移开始
    slideAnim->setEndValue(SLIDE_DISTANCE); // 向上移动
    slideAnim->setEasingCurve(QEasingCurve::OutSine);
    group->addAnimation(slideAnim);

    // b) 透明度淡出动画 ("opacity")
    QPropertyAnimation *opacityAnim = new QPropertyAnimation(toastToExit, "m_opacity");
    opacityAnim->setDuration(m_animationDuration);
    opacityAnim->setStartValue(toastToExit->getOpacity()); // 从当前不透明度开始 (1.0)
    opacityAnim->setEndValue(0.0);                         // 淡出至 0.0
    opacityAnim->setEasingCurve(QEasingCurve::OutSine);
    group->addAnimation(opacityAnim);

    // c) 整体上移
    for (int i = 1; i < m_toasts.count(); ++i)
    {
        Toast *toast = m_toasts[i];
        // 计算绘制文本所需最小区域
        rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toast->m_message);
        int slideDistance = -(m_paddingV + rectText.height() + m_paddingV + m_spacing);
        QPropertyAnimation *anim = new QPropertyAnimation(toast, "m_renderY");
        anim->setDuration(m_animationDuration - 100);
        anim->setEasingCurve(QEasingCurve::InOutSine);
        anim->setStartValue(0.0);
        anim->setEndValue(slideDistance);
        group->addAnimation(anim);
    }

    // 3. 连接动画完成信号到清理槽
    connect(group, &QAbstractAnimation::finished, this,
            [this, toastToExit, group]()
            {
                // 动画完成后，执行清理工作
                removeToastAfterAnimation(toastToExit);
                group->deleteLater(); // 清理动画组
            });

    // 4. 启动动画
    group->start();
}

void ToastManager::removeToastAfterAnimation(Toast *toastToRemove)
{
    // 查找并移除 Toast (使用 removeOne 更安全)
    if (m_toasts.contains(toastToRemove))
    {
        m_toasts.removeOne(toastToRemove);
        toastToRemove->deleteLater();
        XLC_LOG_DEBUG("Remove toast after animation success");
    }

    // 动画完成后，需要重新绘制，让所有剩余的 Toast 向上移动
    this->update();

    // **核心：处理队列中已超时的下一条消息的级联移除**
    // 恢复原来的 while 循环逻辑
    while (true)
    {
        if (m_toasts.isEmpty())
            return;
        Toast *nextMessage = m_toasts.first();
        if (!nextMessage->m_timeoutOccurred)
            break;

        // 如果下一条消息已经超时，立即移除（这里可以考虑也给它一个快速动画，但现在按照原逻辑直接移除）
        m_toasts.removeFirst();
        nextMessage->deleteLater();
        this->update();
        XLC_LOG_DEBUG("Remove cascaded toast success");
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
