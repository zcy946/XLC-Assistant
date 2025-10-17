#include "ListViewStyleHelper.h"
#include <QListView>
#include "ColorRepository.h"

void ListViewStyleHelper::drawBorder(QPainter *painter, QRect rect)
{
    painter->save();
    QPen borderPen;
    borderPen.setColor(ColorRepository::listViewBorderColor());
    borderPen.setWidth(BORDER_WIDTH);
    painter->setPen(borderPen);
    painter->drawRoundedRect(rect.adjusted(BORDER_WIDTH, BORDER_WIDTH, -BORDER_WIDTH, -BORDER_WIDTH), RADIUS, RADIUS);
    painter->restore();
}