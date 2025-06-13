#ifndef GLOBAL_H
#define GLOBAL_H

#include <QFont>
#include <QFontMetrics>
#include <QApplication>

inline QFont getGlobalFont()
{
    QFont font("Microsoft YaHei");
    font.setPointSize(10);
    font.setStyleHint(QFont::SansSerif); // 微软雅黑不存在时优先考虑无衬线字体
    return font;
}

inline int getFontWidth(const QString &str)
{
    return QFontMetrics(getGlobalFont()).horizontalAdvance(str);
}

inline int getFontHeight()
{
    return QFontMetrics(getGlobalFont()).height();
}

#endif // GLOBAL_H