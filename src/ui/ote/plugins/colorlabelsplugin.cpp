#include "colorlabelsplugin.h"

#include "../textedit.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QStylePainter>

namespace ote {

class ColorLabel : public EditorLabel {
public:
    ColorLabel(TextEdit* te, int pos)
        : EditorLabel(te, pos, TYPE_ID)
    {
    }
    void setColor(QColor c);
    void updatePixmap() override;

    static const int TYPE_ID = getNewTypeId();

private:
    QColor m_color;
    QString m_displayName;
};

void ColorLabel::setColor(QColor c)
{
    m_color = c;

    /*m_displayName = c.name();
    for (const auto& cn : QColor::colorNames()) {
        if (QColor(cn) == c)
            m_displayName = cn;
    }*/
}

void ColorLabel::updatePixmap()
{
    const auto size = getDisplayRect().size();
    const auto smallerSide = std::min(size.height(), size.width());
    const auto square = QSizeF(smallerSide, smallerSide);

    m_pixmap = QPixmap(square.toSize());

    QPainter p(&m_pixmap);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(getTextEdit()->getTheme().editorColor(Theme::IconBorder));
    p.setPen(pen);
    p.setBrush(QBrush(m_color));
    p.drawRect(QRectF(QPointF(0, 0), square));
}

ColorLabelsPlugin::ColorLabelsPlugin(TextEdit* te)
    : PluginBase(te)
{
    connect(te, &TextEdit::cursorPositionChanged, this, &ColorLabelsPlugin::onCursorPositionChanged);
}

void ColorLabelsPlugin::onCursorPositionChanged()
{
    auto te = getTextEdit();

    if (!m_ptr.expired()) {
        te->removeEditorLabel(m_ptr);
    }

    const auto c = te->textCursor();
    const auto b = c.block();
    const auto text = b.text();

    if (text.isEmpty())
        return;

    int count = 6;
    int pos = c.positionInBlock();

    while (--pos >= 0 && count > 0) {
        if (text[pos] == '#')
            break;
        count--;
    }

    if (pos < 0 || text[pos] != '#')
        return;
    if (pos + 6 >= text.length())
        return;
    const int startPos = pos;
    const int endPos = startPos + 6;

    count = 0;
    while (++pos <= endPos) {
        const auto c = text[pos];
        if (c.isDigit() || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
            count++;
        else
            return;
    }

    auto l = std::make_shared<ColorLabel>(te, b.position() + endPos + 1);
    l->setColor(QColor(text.mid(startPos, 1 + count)));
    l->setTextOverlap(true);
    l->setAnchorPoint(EditorLabel::AnchorOnLine);
    l->setHeightInLines(1);

    m_ptr = te->addEditorLabel(l);
}

} // namespace ote
