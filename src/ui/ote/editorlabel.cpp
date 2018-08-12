#include "editorlabel.h"

#include "textedit.h"

#include <QDebug>
#include <QTextDocument>

namespace ote {

const int EditorLabel::MAX_LINE_COUNT;

void EditorLabel::draw(QPainter& painter, const QPointF& offset)
{
    painter.save();
    painter.drawPixmap(getDisplayRect().translated(offset).topLeft(), m_pixmap);
    /*if(m_renderDebugInfo) {
        painter.setPen(Qt::red);
        painter.drawRect( getDisplayRect().translated(offset) );
    }*/
    painter.restore();
}

bool EditorLabel::updateDisplayRect(qreal rightBorder)
{
    m_changed = false;

    TextEdit* te = dynamic_cast<TextEdit*>(parent());

    const QTextBlock block = te->document()->findBlock(m_absPos);
    const QTextLayout* layout = block.layout();

    if (!layout) {
        m_displayRect = QRectF();
        return true;
    }

    const auto layoutBoundingRect = layout->boundingRect();
    const int positionInBlock = m_absPos - block.position();
    const QTextLine l = layout->lineForTextPosition(positionInBlock);

    if (!l.isValid()) {
        m_displayRect = QRectF();
        return true;
    }

    auto rectStart = m_anchor == AnchorBelowLine ? layoutBoundingRect.bottomLeft() : layoutBoundingRect.topLeft();
    if (m_anchor == AnchorEndOfLine) {
        rectStart.rx() = l.naturalTextRect().right();
    } else {
        rectStart.rx() = l.cursorToX(positionInBlock);
    }

    auto rectEnd = layoutBoundingRect.bottomLeft();
    if (rightBorder < 0) // Set right border
        rectEnd.rx() = layoutBoundingRect.right();
    else
        rectEnd.rx() = rightBorder;

    auto nb = block.next();

    int numLines = std::min(m_heightInLines, MAX_LINE_COUNT);

    if (m_anchor != AnchorBelowLine) {
        numLines -= block.lineCount();
    }

    while (nb.isValid() && nb.isVisible() && numLines > 0) {
        auto lay = nb.layout();

        if (!m_overlap && lay->lineCount() > 0) {
            auto lineWidth = lay->lineAt(0).naturalTextWidth();
            if (rectStart.x() < lineWidth)
                break;
            numLines--;
        }

        rectEnd.ry() += nb.layout()->boundingRect().height();
        nb = nb.next();
    }

    const auto newDisplayRect = QRectF(rectStart, rectEnd);
    const bool rectChanged = m_displayRect != newDisplayRect;
    m_displayRect = newDisplayRect;

    return rectChanged;
}

EditorLabel::EditorLabel(TextEdit* parent, int pos, int typeId)
    : QObject(parent)
    , m_typeId(typeId)
    , m_absPos(pos)
{
}

QTextBlock EditorLabel::getTextBlock() const
{
    return getTextEdit()->document()->findBlock(m_absPos);
}

TextEdit* EditorLabel::getTextEdit() const
{
    return dynamic_cast<TextEdit*>(parent());
}

} // namespace ote
