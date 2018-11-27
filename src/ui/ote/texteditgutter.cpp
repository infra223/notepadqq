#include "texteditgutter.h"

#include "Highlighter/syntaxhighlighter.h"
#include "Highlighter/theme.h"
#include "textedit.h"

#include <QApplication>
#include <QBitmap>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QTextBlock>

namespace ote {

QPixmap createBookmark(int size, QColor colorFill)
{
    QIcon icon(":/bookmark.svg");
    auto pm = icon.pixmap(QSize(size, size));
    auto mask = pm.createMaskFromColor(Qt::black, Qt::MaskOutColor);

    QPixmap pix = QPixmap(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(colorFill);
    painter.drawPixmap(pm.rect(), mask, mask.rect());
    return pix;
}

QPixmap createFoldingMark(int size, QColor colorFill)
{
    QPixmap pix = QPixmap(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);

    QPolygonF polygon;
    polygon << QPointF(size * 0.15, size * 0.15);
    polygon << QPointF(size * 0.15, size * 0.85);
    polygon << QPointF(size * 0.85, size * 0.5);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(colorFill);
    painter.drawPolygon(polygon);
    return pix;
}

TextEditGutter::TextEditGutter(TextEdit* editor, const Config& cfg)
    : QWidget(editor)
    , m_textEdit(editor)
{
    m_bookmarkStrip.visible = cfg.showBookmarkStrip;
    m_numberStrip.visible = cfg.showNumberStrip;
    m_foldingStrip.visible = cfg.showFoldingStrip;
    m_editStrip.visible = cfg.showEditStrip;

    setMouseTracking(true);
}

void TextEditGutter::mouseMoveEvent(QMouseEvent* event)
{
    const auto block = m_textEdit->blockAtPosition(event->y());

    if (!block.isValid())
        return;

    Q_ASSERT_X(block.isVisible(), "TextEditGutter", "Mouse-over block is invisible.");

    const auto currentBlock = block.blockNumber();
    int currentSection = -1;
    const auto evtX = event->x();

    if (m_bookmarkStrip.visible && m_bookmarkStrip.isInside(evtX))
        currentSection = 0;
    else if (m_numberStrip.visible && m_numberStrip.isInside(evtX))
        currentSection = 1;
    else if (m_foldingStrip.visible && m_foldingStrip.isInside(evtX))
        currentSection = 2;

    if (currentBlock != m_currentHoverBlock || currentSection != m_currentHoverSection) {
        switch (m_currentHoverSection) {
        case 0:
            onBookmarkStripLeave();
            break;
        case 1:
            onNumberStripLeave();
            break;
        case 2:
            onFoldingStripLeave();
            break;
        default:
            break;
        }
        switch (currentSection) {
        case 0:
            onBookmarkStripEnter(block);
            break;
        case 1:
            onNumberStripEnter(block);
            break;
        case 2:
            onFoldingStripEnter(block);
            break;
        default:
            break;
        }
        m_currentHoverSection = currentSection;
        m_currentHoverBlock = currentBlock;
    }
}

void TextEditGutter::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    auto block = m_textEdit->blockAtPosition(event->y());
    if (!block.isValid())
        return;

    switch (m_currentHoverSection) {
    case 0: {
        m_textEdit->toggleBookmark(block);
        update(); // TODO: Only update this line
        break;
    }
    case 1:
        break;
    case 2: {
        if (m_textEdit->isFoldable(block) || m_textEdit->isFolded(block))
            m_textEdit->toggleFold(block);
        break;
    }
    }

    QWidget::mouseReleaseEvent(event);
}

QSize TextEditGutter::sizeHint() const
{
    return m_gutterSize;
}

void TextEditGutter::updateSizeHint(int lineHeight)
{
    auto count = m_textEdit->getLineCount();
    int digits = 1;
    while (count >= 10) {
        ++digits;
        count /= 10;
    }

    static const QString templateString("9999999999");
    const QStringRef ref(&templateString, 0, digits);
    const auto leftMargin = lineHeight / 4;
    const auto widthOfString = m_textEdit->fontMetrics().boundingRect(ref.toString()).width();

    m_lineHeight = lineHeight;

    m_bookmarkStrip.xOffset = leftMargin;
    m_bookmarkStrip.width = m_bookmarkStrip.visible ? lineHeight : 0;

    m_numberStrip.xOffset = m_bookmarkStrip.xOffset + m_bookmarkStrip.width;
    m_numberStrip.width = m_numberStrip.visible ? widthOfString+4 : 0;

    m_foldingStrip.xOffset = m_numberStrip.xOffset + m_numberStrip.width;
    m_foldingStrip.width = m_foldingStrip.visible ? lineHeight : 0;

    m_editStrip.xOffset = m_foldingStrip.xOffset + m_foldingStrip.width;
    m_editStrip.width = m_editStrip.visible ? (lineHeight/4) : 0;

    m_gutterSize = QSize(m_editStrip.xOffset + m_editStrip.width, 0);

    m_foldingMark = createFoldingMark(lineHeight, m_textEdit->getTheme().editorColor(Theme::CodeFolding));
    m_bookmark = createBookmark(lineHeight, m_textEdit->getTheme().editorColor(Theme::MarkBookmark));
}

void TextEditGutter::setBookmarksVisible(bool visible)
{
    if (m_bookmarkStrip.visible == visible)
        return;

    m_bookmarkStrip.visible = visible;
    updateSizeHint(m_lineHeight);
}

void TextEditGutter::setNumbersVisible(bool visible)
{
    if (m_numberStrip.visible == visible)
        return;

    m_numberStrip.visible = visible;
    updateSizeHint(m_lineHeight);
}

void TextEditGutter::setFoldingVisible(bool visible)
{
    if (m_foldingStrip.visible == visible)
        return;

    m_foldingStrip.visible = visible;
    updateSizeHint(m_lineHeight);
}

void TextEditGutter::setEditsVisible(bool visible)
{
    if (m_foldingStrip.visible == visible)
        return;

    m_editStrip.visible = visible;
    updateSizeHint(m_lineHeight);
}

void TextEditGutter::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    const auto& blockList = m_textEdit->getBlocksInRect(event->rect());

    if (blockList.empty())
        return;

    painter.fillRect(event->rect(), m_theme.editorColor(Theme::CurrentLine));

    if (m_bookmarkStrip.visible)
        paintBookmarkStrip(painter, blockList);
    if (m_numberStrip.visible)
        paintNumberStrip(painter, blockList);
    if (m_foldingStrip.visible)
        paintFoldingStrip(painter, blockList);
    if (m_editStrip.visible)
        paintEditStrip(painter, blockList);

    if (m_foldingStrip.visible)
        paintFoldingMarks(painter, blockList);
}

void TextEditGutter::setTheme(const Theme& theme)
{
    if (m_theme == theme)
        return;

    m_theme = theme;
    m_foldingMark = createFoldingMark(m_foldingStrip.width, m_textEdit->getTheme().editorColor(Theme::CodeFolding));
    m_bookmark = createBookmark(m_bookmarkStrip.width, m_textEdit->getTheme().editorColor(Theme::MarkBookmark));
}

void TextEditGutter::paintBookmarkStrip(QPainter& p, const TextEdit::BlockList& bl)
{
    for (const auto& blockData : bl) {
        const auto& block = blockData.block;
        const auto& geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        if (m_textEdit->isBookmarked(block))
            p.drawPixmap(m_bookmarkStrip.xOffset, geom.top(), m_bookmark);
    }
}

void TextEditGutter::paintNumberStrip(QPainter& p, const TextEdit::BlockList& bl)
{
    const int currentBlockNumber = m_textEdit->textCursor().blockNumber();
    p.setFont(m_textEdit->font());

    for (const auto& blockData : bl) {
        const auto& block = blockData.block;
        const auto& geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        const auto blockNumber = block.blockNumber();
        const auto color = (blockNumber == currentBlockNumber) ? m_theme.editorColor(Theme::CurrentLineNumber)
                                                               : m_theme.editorColor(Theme::LineNumbers);

        p.setPen(color);
        p.drawText(m_numberStrip.xOffset,
            geom.top(),
            m_numberStrip.width,
            geom.height(),
            Qt::AlignRight,
            QString::number(blockNumber + 1));
    }
}

void TextEditGutter::paintFoldingStrip(QPainter& p, const TextEdit::BlockList& bl)
{
    if (m_foldingStartBlock == -1)
        return;

    // Don't draw the folding range lines when the hovered block is already folded
    if (m_textEdit->isFolded(m_textEdit->document()->findBlockByNumber(m_foldingStartBlock)))
        return;

    QPen pen;
    pen.setColor(m_textEdit->getTheme().textColor(Theme::Normal));
    pen.setWidth(static_cast<int>(m_lineHeight / 8));
    p.setPen(pen);

    const auto foldingMarkerSize = m_lineHeight;
    const int xPos = int(m_foldingStrip.xOffset + m_foldingStrip.width/2 - (m_lineHeight / 16));

    for (const auto& blockData : bl) {
        const auto& block = blockData.block;
        const auto& geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        const auto blockNumber = block.blockNumber();

        if (blockNumber < m_foldingStartBlock)
            continue;

        if (blockNumber > m_foldingEndBlock)
            break;

        if (blockNumber == m_foldingStartBlock) {
            const QPointF start(xPos, geom.top() + foldingMarkerSize * 0.5);
            const QPointF end(xPos, geom.bottom());
            p.drawLine(start, end);
        } else if (blockNumber == m_foldingEndBlock) {
            const QPointF start(xPos, geom.top());
            const QPointF mid(xPos, geom.top() + foldingMarkerSize * 0.5);
            const QPointF end(width(), geom.top() + foldingMarkerSize * 0.5);
            p.drawLine(start, mid);
            p.drawLine(mid, end);
        } else { // middle piece
            const QPointF start(xPos, geom.top());
            const QPointF end(xPos, geom.bottom());
            p.drawLine(start, end);
        }
    }
}

void TextEditGutter::paintFoldingMarks(QPainter& p, const TextEdit::BlockList& bl)
{
    const auto foldingMarkerSize = m_lineHeight;

    for (const auto& blockData : bl) {
        const auto& block = blockData.block;
        const auto& geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        bool folded = m_textEdit->isFolded(block);

        if (!m_textEdit->isFoldable(block) && !folded)
            continue;

        p.save();
        p.translate(m_foldingStrip.xOffset, geom.top());
        if (folded) {
            p.rotate(90);
            p.translate(0, -foldingMarkerSize);
        }
        p.drawPixmap(0, 0, m_foldingMark);
        p.restore();
    }
}

void TextEditGutter::paintEditStrip(QPainter& p, const TextEdit::BlockList& bl)
{
    p.setPen(Qt::NoPen);
    const auto unsavedChanges = QBrush(m_theme.editorColor(Theme::ModifiedLines));
    const auto savedChanges = QBrush(m_theme.editorColor(Theme::SavedLines));

    const int savedRevision = m_textEdit->m_lastSavedRevision;
    const int initialRevision = m_textEdit->m_initialRevision;

    for (const auto& blockData : bl) {
        const auto& block = blockData.block;
        const auto& geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        const auto rev = block.revision();
        if(rev <= initialRevision)
            continue;
        // Show unsavedChanges not just when rev>savedRevision because a user could CTRL+Z back to a revision before
        // the last save. We want to show that as unsaved too.
        if (rev != savedRevision) {
            p.setBrush(unsavedChanges);
            p.drawRect(m_editStrip.xOffset, geom.top(), m_editStrip.width, geom.height());
        } else {
            p.setBrush(savedChanges);
            p.drawRect(m_editStrip.xOffset, geom.top(), m_editStrip.width, geom.height());
        }
    }
}

void TextEditGutter::onBookmarkStripEnter(const QTextBlock& /*block*/)
{
    setCursor(QCursor(Qt::PointingHandCursor));
}

void TextEditGutter::onNumberStripEnter(const QTextBlock& /*block*/) {}

void TextEditGutter::onFoldingStripEnter(const QTextBlock& block)
{
    if (!m_textEdit->isFoldable(block))
        return;

    const auto blockNum = block.blockNumber();

    m_foldingStartBlock = blockNum;
    m_foldingEndBlock = m_textEdit->findClosingBlock(block).blockNumber();

    setCursor(QCursor(Qt::PointingHandCursor));
    update(); // TODO: Only needed to repaint the area between start and endblock
}

void TextEditGutter::onBookmarkStripLeave()
{
    setCursor(QCursor(Qt::ArrowCursor));
}

void TextEditGutter::onNumberStripLeave() {}

void TextEditGutter::onFoldingStripLeave()
{
    if (m_foldingStartBlock == -1)
        return;

    setCursor(QCursor(Qt::ArrowCursor));

    m_foldingStartBlock = -1;
    m_foldingEndBlock = -1;
    update(); // TODO: Only needed to repaint the area between start and endblock
}

void TextEditGutter::leaveEvent(QEvent* /*e*/)
{
    m_currentHoverBlock = -1;
    m_currentHoverSection = -1;
    if (m_foldingStartBlock != -1) {
        m_foldingStartBlock = -1;
        m_textEdit->update();
    }
}

bool TextEditGutter::StripInfo::isInside(qreal x) const
{
    const auto localX = x - xOffset;
    return 0 < localX && localX < width;
}

} // namespace ote
