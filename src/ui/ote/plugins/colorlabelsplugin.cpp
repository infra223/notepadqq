#include "colorlabelsplugin.h"

#include "../textedit.h"

#include <QApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QStylePainter>

namespace ote {

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
    p.save();

    QPen pen;
    pen.setWidth(2);
    pen.setColor(qApp->palette().window().color());
    p.setPen(pen);
    p.setBrush(QBrush(m_color));
    p.drawRect(QRectF(QPointF(0, 0), square));

    p.restore();
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

    auto c = te->textCursor();
    auto b = c.block();
    auto text = b.text();

    static QRegularExpression regex("#[0-9a-fA-F]{6}");

    auto it = regex.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();

        if (m.captured(0).isEmpty())
            continue;

        if (m.capturedStart() >= c.positionInBlock() || c.positionInBlock() >= m.capturedEnd())
            return;

        const QString string = m.captured(0);

        auto l = std::make_shared<ColorLabel>(te, b.position() + m.capturedEnd());
        l->setColor(QColor(string));
        l->setTextOverlap(true);
        l->setAnchorPoint(EditorLabel::AnchorOnLine);
        l->setHeightInLines(1);

        m_ptr = te->addEditorLabel(l);
    }
}

} // namespace ote
