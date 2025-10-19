#include "ListViewStyleHelper.h"
#include <QListView>
#include "ColorRepository.h"

void ListViewStyleHelper::drawBorder(QPainter *painter, QRect rect)
{
    painter->save();
    QPen borderPen;
    borderPen.setColor(ColorRepository::listViewBorderColor());
    borderPen.setWidth(WIDTH_BORDER);
    painter->setPen(borderPen);
    painter->drawRoundedRect(rect.adjusted(WIDTH_BORDER, WIDTH_BORDER, -WIDTH_BORDER, -WIDTH_BORDER), RADIUS, RADIUS);
    painter->restore();
}