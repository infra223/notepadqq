#include "texteditgutter.h"

#include "Highlighter/theme.h"
#include "textedit.h"

#include <QApplication>
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

TextEditGutter::TextEditGutter(TextEdit* editor)
    : QWidget(editor)
    , m_textEdit(editor)
{
    setMouseTracking(true);
}

void TextEditGutter::mouseMoveEvent(QMouseEvent* event)
{
    const auto block = m_textEdit->blockAtPosition(event->y());

    if (!block.isValid() || !block.isVisible())
        return;

    const auto blockNum = block.blockNumber();

    if (blockNum == m_foldingStartBlock)
        return;

    int hoverBlock = -1;

    if (event->x() < this->geometry().right() - m_foldingBarWidth) {
        hoverBlock = -1;
    } else if (m_textEdit->m_highlighter->startsFoldingRegion(block)) {
        hoverBlock = blockNum;
    }

    const bool needsRepaint = hoverBlock != m_foldingStartBlock;
    const bool isHoveringOverFoldingBlock = hoverBlock == blockNum;

    if (isHoveringOverFoldingBlock) {
        m_foldingStartBlock = blockNum;
        m_foldingEndBlock = m_textEdit->findClosingBlock(block).blockNumber();
    } else {
        m_foldingStartBlock = -1;
        m_foldingEndBlock = -1;
    }

    if (needsRepaint) {
        if (isHoveringOverFoldingBlock)
            setCursor(QCursor(Qt::PointingHandCursor));
        else
            setCursor(QCursor(Qt::ArrowCursor));

        repaint(); // TODO: Only needed to repaint the area between start and endblock
    }
}

QSize TextEditGutter::sizeHint() const
{
    return m_gutterSize;
}

void TextEditGutter::updateSizeHint(qreal lineHeight)
{
    auto count = m_textEdit->getLineCount();
    int digits = 1;
    while (count >= 10) {
        ++digits;
        count /= 10;
    }

    qDebug() << lineHeight;

    static const QString templateString("9999999999");
    const QStringRef ref(&templateString, 0, digits);
    const auto leftMargin = lineHeight / 4;
    const auto widthOfString = m_textEdit->fontMetrics().boundingRect(ref.toString()).width();
    const auto foldingMarkerSize = lineHeight;

    m_foldingBarWidth = foldingMarkerSize;
    m_gutterSize = QSize(leftMargin + widthOfString + foldingMarkerSize, 0);
}

void TextEditGutter::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    const auto& blockList = m_textEdit->getBlocksInRect(event->rect());

    paintGutter(event, painter, blockList);
    paintFoldingMarks(painter, blockList);
}

void TextEditGutter::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->x() >= width() - m_textEdit->fontMetrics().lineSpacing()) {
        auto block = m_textEdit->blockAtPosition(event->y());
        if (!block.isValid() || !m_textEdit->isFoldable(block))
            return;
        m_textEdit->toggleFold(block);
    }
    QWidget::mouseReleaseEvent(event);
}

/*QPixmap createFoldingMark(qreal size, QColor color, QColor color2) {
    QPixmap pix = QPixmap(size, size);
    QPainter *paint = new QPainter(&pix);
    paint->setPen(color);
    paint->drawRect(15,15,100,100);


    pix.fill(color2);

    QPolygonF polygon;
    polygon << QPointF(size * 0.4, size * 0.25);
    polygon << QPointF(size * 0.4, size * 0.75);
    polygon << QPointF(size * 0.8, size * 0.5);

    paint->setRenderHint(QPainter::Antialiasing);
    //paint->setPen(Qt::NoPen);
    paint->setBrush(QColor("red"));
    paint->drawPolygon(polygon);

    paint->end();
    return pix;
}*/

void TextEditGutter::paintFoldingMarks(QPainter& painter, const TextEdit::BlockList& blockList)
{
    if (blockList.empty())
        return;

    // const KSyntaxHighlighting::Theme& currentTheme = m_textEdit->m_highlighter->theme();
    const auto foldingMarkerSize = blockList.front().translatedRect.height();

    for (const auto& blockData : blockList) {
        const auto block = blockData.block;
        const auto geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        if (!m_textEdit->m_highlighter->startsFoldingRegion(block))
            continue;

        bool folded = m_textEdit->isFolded(block);

        const auto blockNum = block.blockNumber();

        // TODO: Bad
        QPolygonF polygon;
        polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.25);
        polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.75);
        polygon << QPointF(foldingMarkerSize * 0.8, foldingMarkerSize * 0.5);
        polygon.translate(-foldingMarkerSize * 0.6, -foldingMarkerSize * 0.5);

        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(
            QColor(m_textEdit->m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding)));

        painter.translate(width() - foldingMarkerSize, geom.top());
        painter.translate(foldingMarkerSize * 0.6, foldingMarkerSize * 0.5);
        if (folded)
            painter.rotate(90);
        painter.drawPolygon(polygon);
        painter.restore();
    }
}

void TextEditGutter::paintGutter(QPaintEvent* event, QPainter& painter, const TextEdit::BlockList& blockList)
{
    if (blockList.empty())
        return;

    const KSyntaxHighlighting::Theme& currentTheme = m_textEdit->m_highlighter->theme();
    const int currentBlockNumber = m_textEdit->textCursor().blockNumber();
    const auto foldingMarkerSize = blockList.front().translatedRect.height();

    painter.fillRect(event->rect(), currentTheme.editorColor(KSyntaxHighlighting::Theme::CurrentLine));
    // painter.fillRect(event->rect(), QColor(QColor::colorNames()[rand() % 10]));

    for (const auto& blockData : blockList) {
        const auto block = blockData.block;
        const auto geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        const auto blockNumber = block.blockNumber();
        const auto color = (blockNumber == currentBlockNumber)
                               ? currentTheme.editorColor(KSyntaxHighlighting::Theme::CurrentLineNumber)
                               : currentTheme.editorColor(KSyntaxHighlighting::Theme::LineNumbers);

        // painter.fillRect(0, geom.top(), width()-foldingMarkerSize, geom.height(), QColor(QColor::colorNames()[rand()
        // % 10]));

        painter.setFont(m_textEdit->font());
        painter.setPen(color);
        painter.drawText(0,
            geom.top(),
            width() - foldingMarkerSize,
            geom.height(),
            Qt::AlignRight,
            QString::number(blockNumber + 1));
    }

    // Paint folding range
    painter.save();
    QPen p;
    p.setColor(m_textEdit->m_highlighter->theme().textColor(KSyntaxHighlighting::Theme::Normal));
    p.setWidth(static_cast<int>(m_foldingBarWidth / 8));
    painter.setPen(p);

    for (const auto& blockData : blockList) {
        const auto block = blockData.block;
        const auto geom = blockData.translatedRect;

        if (m_foldingStartBlock == -1)
            break;

        if (!block.isVisible())
            continue;

        const auto blockNumber = block.blockNumber();

        if (blockNumber < m_foldingStartBlock)
            continue;

        if (blockNumber > m_foldingEndBlock)
            break;

        if (blockNumber == m_foldingStartBlock) {
            const QPointF start(width() - foldingMarkerSize * 0.5, geom.top() + foldingMarkerSize * 0.8);
            const QPointF end(width() - foldingMarkerSize * 0.5, geom.bottom());
            painter.drawLine(start, end);
        } else if (blockNumber == m_foldingEndBlock) {
            const QPointF start(width() - foldingMarkerSize * 0.5, geom.top());
            const QPointF mid(width() - foldingMarkerSize * 0.5, geom.top() + foldingMarkerSize * 0.5);
            const QPointF end(width(), geom.top() + foldingMarkerSize * 0.5);
            painter.drawLine(start, mid);
            painter.drawLine(mid, end);
        } else { // middle piece
            const QPointF start(width() - foldingMarkerSize * 0.5, geom.top());
            const QPointF end(width() - foldingMarkerSize * 0.5, geom.bottom());
            painter.drawLine(start, end);
        }
    }

    painter.restore();
}

void TextEditGutter::leaveEvent(QEvent* /*e*/)
{
    m_foldingStartBlock = -1;
    m_textEdit->repaint();
}

} // namespace ote
