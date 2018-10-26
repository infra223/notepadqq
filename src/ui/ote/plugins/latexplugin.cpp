#include "latexplugin.h"

#include "../Highlighter/syntaxhighlighter.h"
#include "../JKQTMath/jkqtmathtext.h"
#include "../textedit.h"

#include <QApplication>
#include <QDebug>

namespace ote {

class TeXLabel : public EditorLabel {
public:
    TeXLabel(TextEdit* te, int pos)
        : EditorLabel(te, pos, TYPE_ID)
    {
        jkMath.useASANA();
    }

    void updatePixmap() override;

    void setLatexString(const QString& text);

    static const int TYPE_ID = getNewTypeId();

private:
    QString mathFormula;
    JKQTmathText jkMath;
    bool m_pixmapIsSquished = false;
};

void TeXLabel::updatePixmap()
{
    const auto& displayRect = getDisplayRect();
    const auto& block = getTextBlock();

    if (displayRect.isEmpty()) {
        m_pixmap = QPixmap();
        return;
    }

    const bool largeEnough = displayRect.contains(m_pixmap.rect());
    if (!m_pixmapIsSquished && largeEnough) {
        return;
    }

    const auto* te = getTextEdit();
    jkMath.set_fontColor(te->getTheme().textColor(Theme::Normal));
    jkMath.set_fontSize(te->font().pointSize());

    QPainter painter;
    auto size = jkMath.getSize(painter).toSize();

    QRectF drawRect(QPointF(0, 0), size);
    auto lineHeight = block.layout()->boundingRect().height() / block.layout()->lineCount();
    auto scale = drawRect.height() / displayRect.height();
    drawRect.setWidth(drawRect.width() + 12);
    if (scale < 1)
        drawRect.setHeight(drawRect.height() + (lineHeight - ::fmod(drawRect.height(), lineHeight)));

    m_pixmap = QPixmap(drawRect.size().toSize());

    painter.begin(&m_pixmap);
    painter.setBrush(QBrush(te->getTheme().editorColor(Theme::BackgroundColor)));
    QPen pen(te->getTheme().editorColor(Theme::IconBorder));
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(m_pixmap.rect());
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

    mathFormula = text;
    jkMath.parse(mathFormula);
    markForRedraw();
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

    const auto& text = block.text();
    const int blockStartPos = block.position();

    auto te = getTextEdit();
    const auto hl = te->getHighlighter();
    std::vector<QRegularExpressionMatch> matches;

    auto it = regex.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();

        if (!m.captured(1).isEmpty() &&
            hl->isPositionInComment(blockStartPos + m.capturedStart(1), m.capturedLength(1))) {
            matches.push_back(std::move(m));
        }
    }

    const auto& range = te->getEditorLabelsInBlock(block);

    if (range.first == range.second && matches.empty())
        return;

    for (auto it = range.first; it != range.second; ++it) {
        const auto pos = it->get()->getPosition() - blockStartPos;

        auto match = std::find_if(matches.begin(), matches.end(), [pos](const QRegularExpressionMatch& m) {
            return m.capturedStart(1) == pos;
        });

        if (match != matches.end()) {
            auto p = dynamic_cast<TeXLabel*>(it->get());
            p->setLatexString(match->captured(1));
            matches.erase(match);
        } else {
            it->get()->markForDeletion();
        }
    }

    te->deleteMarkedEditorLabelsInRange(range);

    for (const auto& match : matches) {
        auto l = std::make_shared<TeXLabel>(te, blockStartPos + match.capturedStart(1));
        l->setAnchorPoint(EditorLabel::AnchorBelowLine);
        l->setLatexString(match.captured(1));
        te->addEditorLabel(l);
    }
}

} // namespace ote
