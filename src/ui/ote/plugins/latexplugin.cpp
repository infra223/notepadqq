#include "latexplugin.h"

#include "../textedit.h"

#include <QApplication>
#include <QDebug>

namespace ote {

TeXLabel::TeXLabel(TextEdit* te, int pos)
    : EditorLabel(te, pos)
{
    QFont f;
    QString name = f.family() + " " + f.styleName();

    QFontMetrics fm(f);
    // qDebug() << fm.inFont(QChar(0x03B3).unicode()) << fm.inFontUcs4(0x03B3);

    jkMath.useASANA();

    jkMath.set_fontColor(Qt::white);
    jkMath.set_fontSize(15);
}

TeXLabel::~TeXLabel() {}

void TeXLabel::updatePixmap()
{
    const auto& displayRect = getDisplayRect();
    const auto& block = getTextBlock();

    if (displayRect.isEmpty()) {
        m_pixmap = QPixmap();
        return;
    }

    const auto m_displayRectSize = displayRect.size();
    const auto currentSize = m_pixmap.size();
    const bool largeEnough = !currentSize.isEmpty() && m_displayRectSize.height() >= currentSize.height() &&
                             m_displayRectSize.width() >= currentSize.height();

    if (!m_pixmapIsSquished && largeEnough && !m_needToRecalc) {
        qDebug() << "Don't update pixmap";
        return;
    }

    m_needToRecalc = false;
    jkMath.parse(mathFormula);

    QPainter painter;
    auto size = jkMath.getSize(painter).toSize();

    QRectF drawRect(QPointF(0, 0), size);
    auto lineHeight = block.layout()->boundingRect().height() / block.layout()->lineCount();
    auto scale = drawRect.height() / displayRect.height();
    drawRect.setWidth(drawRect.width() + 12);
    if (scale < 1)
        drawRect.setHeight(drawRect.height() + (lineHeight - ::fmod(drawRect.height(), lineHeight)));

    if (m_renderDebugInfo)
        qDebug() << "Drawing pixmap";

    m_pixmap = QPixmap(drawRect.size().toSize()); // W x H

    if (m_renderDebugInfo)
        m_pixmap.fill(Qt::blue);
    else
        m_pixmap.fill(qApp->palette().base().color());

    painter.begin(&m_pixmap);

    if (m_renderDebugInfo) {
        auto centerRect = QRectF(QPointF(0, 0), size);
        centerRect.translate((drawRect.width() - size.width()) / 2, (drawRect.height() - size.height()) / 2);
        painter.setPen(Qt::yellow);
        painter.drawRect(centerRect);
    }

    jkMath.draw(painter, Qt::AlignLeft | Qt::AlignVCenter, drawRect, false);

    painter.end();

    if (scale > 1)
        m_pixmap = m_pixmap.scaledToHeight(displayRect.height(), Qt::SmoothTransformation);
    m_pixmapIsSquished = scale > 1;
}

void TeXLabel::setLatexString(const QString& text)
{
    if (mathFormula == text)
        return;

    m_needToRecalc = true;
    setChanged();
    mathFormula = text;
}

LatexPlugin::LatexPlugin(TextEdit* te)
    : PluginBase(te)
{
    connect(te, &TextEdit::blockChanged, this, &LatexPlugin::onBlockChanged);
}

void LatexPlugin::onBlockChanged(const QTextBlock& block)
{
    static QRegularExpression regex("\\$(.*)\\$");
    regex.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    const auto blockStart = block.position();
    const auto& text = block.text();

    // QTextCharFormat fmt;
    // fmt.setTextOutline(QPen(Qt::yellow));

    auto te = getTextEdit();

    auto it = regex.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();

        if (m.captured(1).isEmpty())
            continue;

        const QString string = m.captured(1);
        const auto pos = blockStart + m.capturedStart() + 1;
        const auto wp = te->getEditorLabelAtPos(pos);

        if (wp.expired()) {
            auto l = std::make_shared<TeXLabel>(te, pos);
            l->setAnchorPoint(EditorLabel::AnchorBelowLine);
            l->setLatexString(string);
            te->addEditorLabel(l);
        } else {
            auto ptr = wp.lock();
            auto p = dynamic_cast<TeXLabel*>(ptr.get());
            p->setLatexString(string);
        }

        // setFormat(m.capturedStart(), m.capturedLength(), fmt);
    }
}

} // namespace ote
