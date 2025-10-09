#include "ToastManager.h"
#include <QPainter>
#include "global.h"
#include "Logger.hpp"
#include <QSvgRenderer>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

// Toast
Toast::Toast(Toast::Type type, const QString &message, int duration, QWidget *parent)
    : QObject(parent),
      m_type(type),
      m_message(message),
      m_duration(duration),
      m_timeoutOccurred(false),
      m_opacity(1.0),
      m_renderY(0.0)
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

// ToastManager
ToastManager *ToastManager::s_instance = nullptr;
ToastManager::ToastManager(QWidget *parent)
    : QWidget(parent),
      m_animationDuration(300),
      m_margin(12),
      m_paddingH(12),
      m_paddingV(10),
      m_spacing(12),
      m_width(400),
      m_height(200),
      m_widthIcon(24),
      m_heightIcon(24),
      m_spacingIconToText(8),
      m_spread(6),
      m_offSetX(2),
      m_offSetY(2),
      m_radius(8),
      m_baseAlpha(40),
      m_isAnimating(false)
{
    // 开启透明背景与鼠标穿透
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    hide();
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
        qreal currentDrawY = y + toast->getRenderY();
        // 计算文本最大可占用宽度
        int maxTextWidth = m_width - m_margin * 2 - m_paddingH * 2 - m_spacingIconToText - m_offSetX - m_spread * 2;
        // 计算绘制文本所需最小区域
        QRect rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toast->m_message);
        // 计算背景区域
        int w = m_paddingH + m_widthIcon + m_spacingIconToText + rectText.width() + m_paddingH;
        int h = m_paddingV + rectText.height() + m_paddingV;
        int x = (m_width - w) / 2;
        // 背景绘制区
        QRect rectBackground(x, currentDrawY, w, h);
        // 文本绘制区
        QRect rectDrawText = rectText.adjusted(x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               currentDrawY + m_paddingV,
                                               x + m_paddingH + m_widthIcon + m_spacingIconToText,
                                               currentDrawY + m_paddingV);
        // 图标绘制区
        QRect rectIcon = QRect(rectBackground.topLeft() + QPoint(m_paddingH, m_paddingV), QSize(m_widthIcon, m_heightIcon));

        // 设置画笔透明度
        painter.setOpacity(toast->getOpacity());
        // 绘制阴影 (rectBackground 已使用 currentDrawY)
        for (int j = 0; j < m_spread; ++j)
        {
            float ratio = 1.0f - float(j) / m_spread;
            int alpha = m_baseAlpha * ratio;
            QColor shadow = QColor(m_colorShadow);
            shadow.setAlpha(alpha);

            QRectF shadowRect = rectBackground.adjusted(-j + m_offSetX, -j + m_offSetY, j + m_offSetY, j + m_offSetY);
            QPainterPath path;
            path.addRoundedRect(shadowRect, m_radius + j, m_radius + j);
            painter.setPen(QPen(shadow, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }
        // 绘制背景
        painter.setBrush(QColor(m_colorBackground));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(rectBackground, m_radius, m_radius);
        // 绘制图标
        QSvgRenderer svgRenderer(toast->m_svgFilePath);
        svgRenderer.render(&painter, rectIcon);
        // 绘制文本
        painter.setPen(Qt::black);
        painter.drawText(rectDrawText, Qt::TextWordWrap, toast->m_message);

        // 更新下一条消息的堆叠基准 Y 坐标
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
    // 非第一个直接返回
    if (m_isAnimating || m_toasts.isEmpty() || m_toasts.first() != toastToExit)
        return;

    // 标记为正在动画，防止重复触发
    m_isAnimating = true;

    // 计算第一条消息的高度
    QFont font(getGlobalFont());
    QFontMetrics fontMetrics(font);
    int maxTextWidth = m_width - m_margin * 2 - m_paddingH * 2 - m_spacingIconToText - m_offSetX - m_spread * 2;
    QRect rectText = fontMetrics.boundingRect(0, 0, maxTextWidth, 0, Qt::TextWordWrap, toastToExit->m_message);
    int firstSlideDistance = m_paddingV + rectText.height() + m_paddingV;

    // 创建并行动画组（首条淡出并上移，剩余消息整体上移）
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    // 首条消息动画：m_renderY 从 0 -> -firstSlideDistance（滑出） 并且 opacity 1 -> 0 （淡出）
    QPropertyAnimation *firstSlide = new QPropertyAnimation(toastToExit, "m_renderY");
    firstSlide->setDuration(m_animationDuration - 50);
    firstSlide->setStartValue(0.0);
    firstSlide->setEndValue(-firstSlideDistance);
    firstSlide->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(firstSlide);

    QPropertyAnimation *firstOpacity = new QPropertyAnimation(toastToExit, "m_opacity");
    firstOpacity->setDuration(m_animationDuration - 50);
    firstOpacity->setStartValue(1.0);
    firstOpacity->setEndValue(0.0);
    firstOpacity->setEasingCurve(QEasingCurve::InOutSine);
    group->addAnimation(firstOpacity);

    // 其余消息整体上移：每个消息 m_renderY 从 0 -> -(firstSlideDistance + spacing)
    for (int i = 1; i < m_toasts.count(); ++i)
    {
        Toast *t = m_toasts.at(i);
        // 目标上移距离 = firstSlideDistance + m_spacing （第一条消息的高度和其下方的消息间距）
        int slideDistance = -(firstSlideDistance + m_spacing);
        QPropertyAnimation *anim = new QPropertyAnimation(t, "m_renderY");
        anim->setDuration(m_animationDuration - 100);
        anim->setStartValue(0.0);
        anim->setEndValue(slideDistance);
        anim->setEasingCurve(QEasingCurve::InOutSine);
        group->addAnimation(anim);
    }

    connect(group, &QParallelAnimationGroup::finished, this,
            [this]()
            {
                if (!m_toasts.isEmpty())
                {
                    Toast *first = m_toasts.takeFirst();
                    first->deleteLater();
                }
                // 重置所有剩余消息的 renderY 与 opacity 到初始状态
                for (Toast *t : m_toasts)
                {
                    t->setRenderY(0.0);
                    t->setOpacity(1.0);
                }
                m_isAnimating = false;
                this->update();

                // 如果新的队首已经超时，则立即开始下一次退出动画
                if (!m_toasts.isEmpty())
                {
                    Toast *newFirst = m_toasts.first();
                    if (newFirst->m_timeoutOccurred)
                    {
                        // 递归调用发起下一个动画
                        slot_onRequestExist(newFirst);
                    }
                }
            });

    group->start(QAbstractAnimation::DeleteWhenStopped);
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
    Toast *newToast = new Toast(type, message, duration, parentWidget());
    m_toasts.append(newToast);
    connect(newToast, &Toast::sig_requestExit, this, &ToastManager::slot_onRequestExist);
    connect(newToast, &Toast::sig_requestUpdate, this, &ToastManager::slot_onRequestUpdate);

    // 淡入动画
    QPropertyAnimation *pAnimationOpacity = new QPropertyAnimation(newToast, "m_opacity");
    pAnimationOpacity->setDuration(m_animationDuration);
    pAnimationOpacity->setEasingCurve(QEasingCurve::OutSine);
    pAnimationOpacity->setStartValue(0.0);
    pAnimationOpacity->setEndValue(1.0);
    pAnimationOpacity->start(QAbstractAnimation::DeleteWhenStopped);

    newToast->startTimer();
    raise();
    update();
}