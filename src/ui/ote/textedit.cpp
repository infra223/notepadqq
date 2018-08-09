#include "textedit.h"

#include "texteditgutter.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFontDatabase>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QPalette>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>
#include <QTimer>

#include <algorithm>
#include <cmath>

namespace ote {

TextEdit::TextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
    , m_sideBar(new TextEditGutter(this))
    , m_highlighter(new SyntaxHighlighter(document()))
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    // We probably shouldn't call this here.
    /*setTheme((palette().color(QPalette::Base).lightness() < 128)
                 ? getRepository().defaultTheme(Repository::DarkTheme)
                 : getRepository().defaultTheme(Repository::LightTheme));*/

    connect(this, &QPlainTextEdit::blockCountChanged, this, &TextEdit::updateSidebarGeometry);
    connect(this, &QPlainTextEdit::updateRequest, this, &TextEdit::updateSidebarArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEdit::onCursorPositionChanged); // slot
    // connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged); // signal
    connect(this, &QPlainTextEdit::selectionChanged, this, &TextEdit::onSelectionChanged);
    connect(document(), &QTextDocument::contentsChange, this, &TextEdit::onContentsChange);

    connect(m_highlighter, &ote::SyntaxHighlighter::blockChanged, this, &TextEdit::blockChanged);

    setWordWrap(false);
    setCenterOnScroll(false);

    updateSidebarGeometry();
    onCursorPositionChanged();
}

void TextEdit::setTheme(const Theme& theme)
{
    if (theme == getTheme())
        return;

    auto pal = qApp->palette();
    if (theme.isValid()) {
        pal.setColor(QPalette::Base, theme.editorColor(Theme::BackgroundColor));
        pal.setColor(QPalette::Text, theme.textColor(Theme::Normal));
        pal.setColor(QPalette::Highlight, theme.editorColor(Theme::CurrentLine));
    }
    setPalette(pal);
    viewport()->setPalette(pal);

    m_highlighter->setTheme(theme);

    onCursorPositionChanged();
    onSelectionChanged();
}

void TextEdit::highlightCurrentLine()
{
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QBrush(getTheme().editorColor(Theme::CurrentLine)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    auto& eslh = m_extraSelections[ESLineHighlight];
    eslh.clear();
    eslh << selection;
}

Repository* TextEdit::s_repository = nullptr;

void TextEdit::initRepository(const QString& path)
{
    QElapsedTimer t;
    t.start();
    s_repository = new Repository(path);

    qDebug() << "Repository directory loaded in " << t.elapsed() / 1000 / 1000 << "msec";
}

void TextEdit::setDefinition(const Definition& d)
{
    m_highlighter->setDefinition(d);
}

/*void TextEdit::setSyntaxHighlightingEnabled(bool enabled)
{
    if (enabled && m_highlighter->document() == nullptr)
        m_highlighter->setDocument(document());
    else
        m_highlighter->setDocument(nullptr);
}*/

void TextEdit::setEndOfLineMarkersVisible(bool enable)
{
    if (enable == m_showEndOfLineMarkers)
        return;

    m_showEndOfLineMarkers = enable;
    viewport()->repaint();
}

void TextEdit::setWhitespaceVisible(bool show)
{
    auto opts = document()->defaultTextOption();
    auto flags = opts.flags();

    //flags.setFlag(QTextOption::ShowTabsAndSpaces, show);
    if (show)
        opts.setFlags(flags | QTextOption::ShowTabsAndSpaces);
    else
        opts.setFlags(flags &  (~QTextOption::ShowTabsAndSpaces));

    //opts.setFlags(flags);
    document()->setDefaultTextOption(opts);
}

void TextEdit::setShowLinebreaks(bool show)
{
    if (show == m_showLinebreaks)
        return;

    m_showLinebreaks = show;
    update();
}

void TextEdit::setSmartIndent(bool enable)
{
    m_smartIndent = enable;
}

void TextEdit::setTabToSpaces(bool enable)
{
    m_tabToSpaces = enable;
}

void TextEdit::setWordWrap(bool enable)
{
    enable ? setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere) : setWordWrapMode(QTextOption::NoWrap);
}

void TextEdit::setTabWidth(int width)
{
    if (width < 0 || width == m_tabWidth)
        return;

    m_tabWidth = width;
    setFont(getFont());
}

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
void TextEdit::setTabStopDistance(qreal distance)
{
    QTextOption opt = document()->defaultTextOption();
    if (opt.tabStop() == distance || distance < 0)
        return;
    opt.setTabStop(distance);
    document()->setDefaultTextOption(opt);
}
#endif

void TextEdit::setFont(QFont font)
{
    if (!QFontInfo(font).fixedPitch()) {
        qDebug() << "Selected font is not monospace: " << font.family() << font.style();
    }

    m_fontSize = font.pointSize();
    font.setPointSize(font.pointSize() + m_zoomLevel);

    // Calculating letter width using QFrontMetrics isn't 100% accurate. Small inaccuracies
    // can accumulate over time. Instead, we can calculate a good letter spacing value and
    // make the font use it.
    // https://stackoverflow.com/a/42071875/1038629
    QFontMetricsF fm(font);
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    auto stopWidth = m_tabWidth * fm.width(' ');
#else
    auto stopWidth = m_tabWidth * fm.horizontalAdvance(' ');
#endif
    auto letterSpacing = (ceil(stopWidth) - stopWidth) / m_tabWidth;

    font.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);
    QPlainTextEdit::setFont(font);
    setTabStopDistance(ceil(stopWidth));
}

QFont TextEdit::getFont() const {
    QFont f = font();
    f.setPointSize(m_fontSize);
    return f;
}

bool TextEdit::isTabToSpaces() const
{
    return m_tabToSpaces;
}

int TextEdit::getTabWidth() const
{
    return m_tabWidth;
}

QString TextEdit::getCurrentWord() const
{
    auto c = textCursor();
    c.select(QTextCursor::WordUnderCursor);
    return c.selectedText();
}

int TextEdit::getLineCount() const
{
    return blockCount();
}

int TextEdit::getCharCount() const
{
    return document()->characterCount();
}

void TextEdit::setCursorPosition(int line, int column)
{
    setAbsoluteCursorPosition(cursorPosToAbsolutePos({line, column}));
}

void TextEdit::setCursorPosition(const TextEdit::CursorPos& pos)
{
    setCursorPosition(pos.line, pos.column);
}

TextEdit::CursorPos TextEdit::getCursorPosition() const
{
    const auto& c = textCursor();
    return {c.blockNumber(), c.positionInBlock()};
}

void TextEdit::setAbsoluteCursorPosition(int pos)
{
    auto c = textCursor();
    c.setPosition(pos);
    setTextCursor(c);
}

int TextEdit::getAbsoluteCursorPosition() const
{
    return textCursor().position();
}

QString TextEdit::getSelectedText() const
{
    return textCursor().selectedText();
}

TextEdit::Selection TextEdit::getSelection() const
{
    Selection s;
    auto cursor = textCursor();
    auto c = cursor;
    c.setPosition(cursor.selectionStart());
    s.start = {c.blockNumber(), c.positionInBlock()};
    c.setPosition(cursor.selectionEnd());
    s.end = {c.blockNumber(), c.positionInBlock()};

    return s;
}

void TextEdit::setSelection(const TextEdit::Selection& sel)
{
    auto cur = textCursor();
    cur.setPosition(cursorPosToAbsolutePos(sel.start));
    cur.setPosition(cursorPosToAbsolutePos(sel.end), QTextCursor::KeepAnchor);
    setTextCursor(cur);
}

void TextEdit::setTextInSelection(const QString& text, bool keepSelection)
{
    auto c = textCursor();

    c.insertText(text);

    if (keepSelection && !text.isEmpty()) {
        c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, text.length());
        setTextCursor(c);
    }

    // textCursor().insertText(text);
}

QPoint TextEdit::getScrollPosition() const
{
    return {horizontalScrollBar()->sliderPosition(), verticalScrollBar()->sliderPosition()};
}

void TextEdit::setScrollPosition(const QPoint& p)
{
    horizontalScrollBar()->setSliderPosition(p.x());
    verticalScrollBar()->setSliderPosition(p.y());
}

bool TextEdit::findTentative(const QString& term, TextEdit::FindFlags flags)
{
    auto c = textCursor();
    c.setPosition(c.selectionStart());
    setTextCursor(c);
    return find(term, flags);
}

bool TextEdit::find(const QString& term, TextEdit::FindFlags flags)
{
    return find(term, 0, -1, flags);
}

bool TextEdit::find(const QString& term, int regionStart, int regionEnd, TextEdit::FindFlags flags, bool wrapAround)
{
    auto curr = textCursor();
    if (regionEnd < 0)
        regionEnd = document()->characterCount() - 1;

    if (curr.position() < regionStart)
        curr.setPosition(regionStart);
    else if (curr.position() > regionEnd)
        curr.setPosition(regionEnd);

    const bool fwd = !flags.testFlag(QTextDocument::FindBackward);
    int pos = curr.position();

    if (flags & QTextDocument::FindBackward)
        pos = curr.selectionStart() - 1;
    else
        pos = curr.selectionEnd();

    auto c = document()->find(QRegularExpression(term), pos, flags);

    // If c moved past the search range or failed to find a match we may want to loop around and retry.
    if (wrapAround) {
        if ((fwd && c.isNull()) || c.selectionEnd() > regionEnd)
            c = document()->find(term, regionStart, flags);
        else if ((!fwd && c.isNull()) || c.selectionStart() < regionStart)
            c = document()->find(term, regionEnd, flags);
    }

    if (!c.isNull() && c.selectionEnd() <= regionEnd && c.selectionStart() >= regionStart) {
        setTextCursor(c);
        m_findTermSelected = true;
        return true;
    }

    return false;
}

void TextEdit::resetZoom()
{
    setZoomTo(0);
}

void TextEdit::setZoomTo(int value)
{
    // Clamp maximum font size to [4,40]. That should be sensible.
    const int MIN_FONT_SIZE = 4;
    const int MAX_FONT_SIZE = 40;

    if (m_fontSize + value < MIN_FONT_SIZE)
        value = MIN_FONT_SIZE - m_fontSize;
    else if (m_fontSize + value > MAX_FONT_SIZE)
        value = MAX_FONT_SIZE - m_fontSize;

    m_zoomLevel = value;

    QFont f = font();
    f.setPointSize(m_fontSize + m_zoomLevel);
    QPlainTextEdit::setFont(f);

    updateSidebarGeometry();
}

void TextEdit::zoomIn()
{
    setZoomTo(m_zoomLevel + 1);
}

void TextEdit::zoomOut()
{
    setZoomTo(m_zoomLevel - 1);
}

int TextEdit::getZoomLevel() const
{
    return m_zoomLevel;
}

void TextEdit::clearHistory()
{
    document()->clearUndoRedoStacks();
}

int TextEdit::getModificationRevision() const
{
    return document()->revision();
}

bool TextEdit::isModified() const
{
    return document()->isModified();
}

void TextEdit::setModified(bool modified)
{
    document()->setModified(modified);
}

void TextEdit::moveSelectedBlocksUp()
{
    bool success;

    // Selects the line above the selected blocks.
    auto lineCursor = textCursor();
    lineCursor.setPosition(lineCursor.selectionStart()); // Remove selection
    success = lineCursor.movePosition(QTextCursor::PreviousBlock);
    success &= lineCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

    if (!success)
        return;

    auto insertCursor = textCursor();
    insertCursor.setPosition(insertCursor.selectionEnd());
    success = insertCursor.movePosition(QTextCursor::NextBlock);

    // If the cursor is at the last block the above move action can fail. In that case a new
    // block needs to be added or the insert operation screws up.
    if (!success) {
        insertCursor.movePosition(QTextCursor::EndOfBlock);
        insertCursor.insertBlock();
    }

    lineCursor.beginEditBlock();

    auto text = lineCursor.selectedText();
    lineCursor.removeSelectedText();
    insertCursor.insertText(text);

    lineCursor.endEditBlock();
}

void TextEdit::moveSelectedBlocksDown()
{
    auto c = textCursor();

    bool success;
    c.setPosition(c.selectionEnd()); // Remove selection
    success = c.movePosition(QTextCursor::NextBlock);
    success &= c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

    if (!success)
        return;

    c.beginEditBlock();
    auto text = c.selectedText();
    c.removeSelectedText();

    c = textCursor();
    c.setPosition(c.selectionStart());
    c.movePosition(QTextCursor::StartOfBlock);
    c.insertText(text);

    c.endEditBlock();
}

void TextEdit::duplicateSelectedBlocks()
{
    auto c = textCursor();
    auto blockCursor = c;

    blockCursor.setPosition(c.selectionStart());
    blockCursor.movePosition(QTextCursor::StartOfBlock);
    blockCursor.setPosition(c.selectionEnd(), QTextCursor::KeepAnchor);
    auto success = blockCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);

    // The previous call fails when we're at the end of the document. In that case we insert a new
    // Block and remove it later.
    if (!success) {
        auto v = blockCursor;
        v.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        v.insertBlock();
        blockCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    }

    c.beginEditBlock();
    auto text = blockCursor.selectedText();

    c = textCursor();
    c.setPosition(c.selectionStart());
    c.movePosition(QTextCursor::StartOfBlock);
    c.insertText(text);

    if (!success)
        c.deletePreviousChar();

    c.endEditBlock();
}

void TextEdit::deleteSelectedBlocks()
{
    auto c = textCursor();
    auto ce = c;

    ce.beginEditBlock();
    ce.setPosition(c.selectionStart());
    ce.movePosition(QTextCursor::StartOfBlock);
    ce.setPosition(c.selectionEnd(), QTextCursor::KeepAnchor);
    auto success = ce.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    if (!success)
        ce.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    ce.removeSelectedText();
    if (!success)
        ce.deletePreviousChar();
    ce.endEditBlock();
}

// pair.first = number of ws characters found, pair.second = number of spaces needed
QPair<int, int> getLeadingWSLength(const QStringRef& ref, int tabWidth)
{
    auto ws = 0;

    for (int i = 0; i < ref.size(); ++i) {
        switch (ref.at(i).toLatin1()) {
        case ' ':
            ws += 1;
            break;
        case '\t': {
            const auto distToNextCol = tabWidth - (ws % tabWidth);
            ws += distToNextCol;
            break;
        }
        default:
            return {i, ws};
        }
    }

    return {ref.size(), ws};
}

void TextEdit::convertLeadingWhitespaceToTabs()
{
    // TODO: Might be more efficient using a block-based approach?
    QString plaintext = toPlainText();
    auto lines = plaintext.splitRef('\n');
    QString final;
    final.reserve(plaintext.length());

    for (auto& line : lines) {
        auto pair = getLeadingWSLength(line, m_tabWidth);
        auto idx = pair.first;
        auto ws = pair.second;

        final += QString(ws / m_tabWidth, '\t') + QString(ws % m_tabWidth, ' ') ;
        final += line.mid(idx);
        final += '\n';
    }

    if (!final.isEmpty())
        final.chop(1); // Remove last '\n' since it creates an extra line at the end

    auto c = textCursor();
    auto p = getCursorPosition();
    c.beginEditBlock();
    c.select(QTextCursor::Document);
    c.insertText(final);
    setCursorPosition(p);
    c.endEditBlock();
}

void TextEdit::convertLeadingWhitespaceToSpaces()
{
    // TODO: Might be more efficient using a block-based approach?
    QString plaintext = toPlainText();
    auto lines = plaintext.splitRef('\n');
    QString final;
    final.reserve(plaintext.length());

    for (auto& line : lines) {
        auto pair = getLeadingWSLength(line, m_tabWidth);
        auto idx = pair.first;
        auto ws = pair.second;

        final += QString(ws, ' ');
        final += line.mid(idx);
        final += '\n';
    }

    if (!final.isEmpty())
        final.chop(1); // Remove last '\n' since it creates an extra line at the end

    auto c = textCursor();
    auto p = getCursorPosition();
    c.beginEditBlock();
    c.select(QTextCursor::Document);
    c.insertText(final);
    setCursorPosition(p);
    c.endEditBlock();
}

int findFirstNonWS(const QStringRef& ref)
{
    for (int i = 0; i < ref.size(); ++i)
        if (!ref.at(i).isSpace())
            return i;

    return ref.size();
}

int findLastNonWS(const QStringRef& ref)
{
    for (int i = ref.size() - 1; i >= 0; i--)
        if (!ref.at(i).isSpace())
            return i;

    return 0;
}

void TextEdit::trimWhitespace(bool leading, bool trailing)
{
    if (!leading && !trailing)
        return;

    const QString& original = toPlainText();
    const auto lines = original.splitRef('\n');
    QString final;
    final.reserve(original.length());

    for (auto& line : lines) {
        const int start = !leading ? 0 : findFirstNonWS(line);
        const int end = !trailing ? -1 : findLastNonWS(line) - start + 1;

        final += line.mid(start, end);
        final += '\n';
    }

    if (!final.isEmpty())
        final.chop(1); // Remove last '\n' since it creates an extra line at the end

    auto c = textCursor();
    auto p = getCursorPosition();
    c.beginEditBlock();
    c.select(QTextCursor::Document);
    c.insertText(final);
    setCursorPosition(p);
    c.endEditBlock();
}

void TextEdit::updateSidebarGeometry()
{
    const auto firstBlock = firstVisibleBlock();
    const qreal lineHeight =
        firstBlock.isValid() ? blockBoundingRect(firstBlock).height() / firstBlock.lineCount() : 17; // Decent default

    m_sideBar->updateSizeHint(lineHeight);
    auto gutterWidth = m_sideBar->sizeHint().width();

    setViewportMargins(gutterWidth, 0, 0, 0);
    const auto r = contentsRect();
    m_sideBar->setGeometry(QRect(r.left(), r.top(), gutterWidth, r.height()));

    auto g = horizontalScrollBar()->geometry();
    g.setLeft(gutterWidth);
    horizontalScrollBar()->setGeometry(g);
}

void TextEdit::updateSidebarArea(const QRect& rect, int dy)
{
    if (dy)
        m_sideBar->scroll(0, dy);
    else
        m_sideBar->update(0, rect.y(), m_sideBar->width(), rect.height());
}

void TextEdit::onCursorPositionChanged()
{
    // Highlight current line
    highlightCurrentLine();
/*
    auto pos = textCursor().position();
    if (m_highlighter->isPositionInComment(pos))
        qDebug() << "Is inside comment";
    else
        qDebug() << "Is not inside comment";*/


    // Try to match brackets
    m_extraSelections[ESMatchingBrackets].clear();

    /*auto pair = m_highlighter->m_bracketMatcher->findMatchingBracketPosition(textCursor());

    if(pair.first != -1 && pair.second != -1) {
        createParenthesisSelection(pair.first);
        createParenthesisSelection(pair.second);
    }*/

    m_findTermSelected = false;
}

void TextEdit::onSelectionChanged()
{
    const auto& cursor = textCursor();
    const auto& text = cursor.selectedText();

    if (text.length() < 2 || text.trimmed().isEmpty()) {
        m_extraSelections[ESSameItems].clear();
        return;
    }

    const auto& fullText = toPlainText();
    int j = 0;

    ExtraSelectionList list;
    QTextEdit::ExtraSelection selection;
    selection.format.setForeground(QBrush(getTheme().textColor(Theme::Keyword)));
    selection.format.setBackground(
        QBrush(getTheme().editorColor(Theme::SearchHighlight)));

    while ((j = fullText.indexOf(text, j)) != -1) {
        selection.cursor = cursor;
        selection.cursor.setPosition(j);
        selection.cursor.setPosition(j + text.length(), QTextCursor::KeepAnchor);

        list.append(selection);
        ++j;
    }

    m_extraSelections[ESSameItems] = list;
}

void TextEdit::onContentsChange(int position, int removed, int added)
{
    auto& lbls = m_editorLabels;
    auto it = std::remove_if(lbls.begin(), lbls.end(), [this, position, added, removed](EditorLabelPtr& ptr) {
        if (ptr->m_absPos >= position && ptr->m_absPos <= position + removed) {
            return true;
        }

        auto b = document()->findBlock(position).previous().previous();
        if (!b.isValid())
            b = document()->findBlockByNumber(0);

        auto newPos = b.position();

        if (ptr->m_absPos >= position) {
            ptr->m_changed = true;
            ptr->m_absPos += added - removed;
        } else if (ptr->m_absPos >= newPos) {
            ptr->m_changed = true;
        }

        return false;
    });

    if (it != lbls.end()) {
        lbls.erase(it, lbls.end());
        QTimer::singleShot(0, [this]() mutable { viewport()->repaint(); });
    }
}

void TextEdit::createParenthesisSelection(int pos)
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    QTextCharFormat f;
    f.setForeground(QBrush(getTheme().editorColor(Theme::BracketMatching)));

    m_extraSelections[ESMatchingBrackets] << QTextEdit::ExtraSelection{cursor, f};
}

void TextEdit::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Tab && m_tabToSpaces) {
        auto cursor = textCursor();
        auto numSpaces = m_tabWidth - cursor.positionInBlock() % m_tabWidth;
        if (numSpaces == 0)
            numSpaces = m_tabWidth;
        cursor.insertText(QString(numSpaces, ' '));
        return;
    }

    if (e->key() == Qt::Key_Return && m_smartIndent) {
        auto cursor = textCursor();
        cursor.beginEditBlock();
        QPlainTextEdit::keyPressEvent(e);
        auto txt = textCursor().block().previous().text();

        int txtPos = 0;
        while (txt[txtPos] == ' ' || txt[txtPos] == '\t')
            ++txtPos;

        textCursor().insertText(txt.mid(0, txtPos));
        cursor.endEditBlock();
        return;
    }

    if (e->key() == Qt::Key_Backspace && m_tabToSpaces) {
        auto txt = textCursor().block().text();

        if (txt.isEmpty())
            goto processEventNormally;

        if (txt.endsWith(QString(m_tabWidth, ' ')) && (txt.length() % m_tabWidth) == 0) {
            auto c = textCursor();

            c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, m_tabWidth);
            c.removeSelectedText();
            return;
        }
    }

    if (e->key() == Qt::Key_Insert)
        setOverwriteMode(!overwriteMode());

processEventNormally:
    QPlainTextEdit::keyPressEvent(e);
}

void TextEdit::wheelEvent(QWheelEvent* event)
{
    emit mouseWheelUsed(event);
    QPlainTextEdit::wheelEvent(event);
}

void TextEdit::dropEvent(QDropEvent* event)
{
    if (event->mimeData() && event->mimeData()->hasUrls())
        emit urlsDropped(event->mimeData()->urls());
}

void TextEdit::focusInEvent(QFocusEvent* event)
{
    emit gotFocus();
    QPlainTextEdit::focusInEvent(event);
}

void TextEdit::contextMenuEvent(QContextMenuEvent* event)
{
    QPlainTextEdit::contextMenuEvent(event);
    /*auto menu = createStandardContextMenu(event->pos());
    menu->exec(event->globalPos());
    delete menu;*/
}

void TextEdit::paintLineSuffixes(QPainter& painter, const BlockList& blockList) const
{
    // Paints EOL symbols and the '((...))' labels for folded regions.
    const QFontMetrics& metrics = fontMetrics();
    const QChar visualArrow = ushort(0x21A4);
    const qreal arrowWidth = metrics.width(visualArrow);
    const QChar cont[]{0x2E28, 0x22EF, 0x22EF, 0x2E29, 0}; // ((...))
    const QString contStr(cont);
    const qreal constWidth = metrics.width(contStr);
    const qreal spaceWidth = metrics.width(' ') * 2;

    const QColor textColor(getTheme().textColor(Theme::Normal));
    const QBrush regionBrush(getTheme().textColor(Theme::RegionMarker));

    painter.save();
    painter.setPen(textColor);

    for (const auto& blockData : blockList) {
        const auto block = blockData.block;
        const auto geom = blockData.translatedRect;

        if (!block.isVisible())
            continue;

        const bool folded = isFolded(block);

        if (!m_showEndOfLineMarkers && !folded)
            continue;

        const QTextLayout* layout = block.layout();
        const int lineCount = layout->lineCount();
        const QTextLine line = layout->lineAt(lineCount - 1);
        const QRectF lineRect = line.naturalTextRect().translated(contentOffset().x(), geom.top());

        if (m_showEndOfLineMarkers)
            painter.drawText(QPointF(lineRect.right() + 2, lineRect.top() + line.ascent()), visualArrow);

        if (folded) {
            qreal offset = spaceWidth + (m_showEndOfLineMarkers ? arrowWidth : 0);
            painter.save();
            painter.setPen(getTheme().textColor(Theme::RegionMarker));

            QRectF rect;
            rect.setTopLeft(QPointF(lineRect.right() + offset, lineRect.top()));
            rect.setHeight(lineRect.height() - 1); // normal height() is a little taller than the line highlight
            rect.setWidth(constWidth);

            // painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(rect, 3.0, 3.0);
            painter.fillPath(path, regionBrush);
            painter.drawPath(path);

            painter.setPen(textColor);
            painter.drawText(QPointF(lineRect.right() + offset, lineRect.top() + line.ascent()), contStr);
            painter.restore();
        }
    }

    painter.restore();
}

void TextEdit::paintLineBreaks(QPainter& painter, const BlockList& blockList) const
{
    if (!m_showLinebreaks)
        return;

    const QChar visualArrow = ushort(0x21B5);
    const auto arrowWidth = fontMetrics().boundingRect(visualArrow).width();

    painter.save();
    painter.setPen(getTheme().textColor(Theme::Normal));

    for (const auto& blockData : blockList) {
        const auto block = blockData.block;
        const auto geom = blockData.translatedRect;

        const QTextLayout* layout = block.layout();
        const int lineCount = layout->lineCount();
        const int arrowX = geom.width() - contentsMargins().right() - arrowWidth;

        if (lineCount <= 1)
            continue;

        for (int i = 0; i < lineCount - 1; ++i) {
            const QTextLine line = layout->lineAt(i);
            const QRectF lineRect = line.naturalTextRect().translated(contentOffset().x(), geom.top());

            painter.drawText(QPointF(arrowX, lineRect.top() + line.ascent()), visualArrow);
        }
    }

    painter.restore();
}

void TextEdit::paintSearchBlock(QPainter& painter, const QRect& /*eventRect*/, const QTextBlock& block)
{
    return;
    QTextLayout* layout = block.layout();
    QString text = block.text();

    QTextLine line = layout->lineForTextPosition(0);

    QRectF rr = line.naturalTextRect();
    QRectF r = blockBoundingGeometry(block).translated(contentOffset());

    rr.moveTop(rr.top() + r.top());

    painter.fillRect(rr, QColor("#5000FF00"));

    QColor lineCol = QColor("red");
    QPen pen = painter.pen();
    painter.setPen(lineCol);
    // if (block == d->m_findScopeStart.block())
    painter.drawLine(rr.topLeft(), rr.topRight());
    // if (block == d->m_findScopeEnd.block())
    painter.drawLine(rr.bottomLeft(), rr.bottomRight());
    painter.drawLine(rr.topLeft(), rr.bottomLeft());
    painter.drawLine(rr.topRight(), rr.bottomRight());
    painter.setPen(pen);

    /*qreal spacew = QFontMetricsF(font()).width(QLatin1Char(' '));

    int offset = 0;
    int relativePos  =  ts.positionAtColumn(text,
                                            d->m_findScopeVerticalBlockSelectionFirstColumn,
                                            &offset);

    qreal x = line.cursorToX(relativePos) + offset * spacew;

    int eoffset = 0;
    int erelativePos  =  ts.positionAtColumn(text,
                                             d->m_findScopeVerticalBlockSelectionLastColumn,
                                             &eoffset);
    QTextLine eline = layout->lineForTextPosition(erelativePos);
    qreal ex = eline.cursorToX(erelativePos) + eoffset * spacew;


    rr.moveTop(rr.top() + r.top());
    rr.setLeft(r.left() + x);
    if (line.lineNumber() == eline.lineNumber())
        rr.setRight(r.left() + ex);


    QColor lineCol = QColor("red");
    QPen pen = painter.pen();
    painter.setPen(lineCol);
    if (blockFS == d->m_findScopeStart.block())
        painter.drawLine(rr.topLeft(), rr.topRight());
    if (blockFS == d->m_findScopeEnd.block())
        painter.drawLine(rr.bottomLeft(), rr.bottomRight());
    painter.drawLine(rr.topLeft(), rr.bottomLeft());
    painter.drawLine(rr.topRight(), rr.bottomRight());
    painter.setPen(pen);
*/

    /*  auto er = eventRect;

    if (!m_findActive) {
        QTextBlock blockFS = block;
        QPointF offsetFS = contentOffset();
        while (blockFS.isValid()) {

            QRectF r = blockBoundingRect(blockFS).translated(offsetFS);

            if (r.bottom() >= er.top() && r.top() <= er.bottom()) {

                if (blockFS.position() >= d->m_findScopeStart.block().position()
                        && blockFS.position() <= d->m_findScopeEnd.block().position()) {
                    QTextLayout *layout = blockFS.layout();
                    QString text = blockFS.text();
                    const TabSettings &ts = d->m_document->tabSettings();
                    qreal spacew = QFontMetricsF(font()).width(QLatin1Char(' '));

                    int offset = 0;
                    int relativePos  =  ts.positionAtColumn(text,
                                                            d->m_findScopeVerticalBlockSelectionFirstColumn,
                                                            &offset);
                    QTextLine line = layout->lineForTextPosition(relativePos);
                    qreal x = line.cursorToX(relativePos) + offset * spacew;

                    int eoffset = 0;
                    int erelativePos  =  ts.positionAtColumn(text,
                                                             d->m_findScopeVerticalBlockSelectionLastColumn,
                                                             &eoffset);
                    QTextLine eline = layout->lineForTextPosition(erelativePos);
                    qreal ex = eline.cursorToX(erelativePos) + eoffset * spacew;

                    QRectF rr = line.naturalTextRect();
                    rr.moveTop(rr.top() + r.top());
                    rr.setLeft(r.left() + x);
                    if (line.lineNumber() == eline.lineNumber())
                        rr.setRight(r.left() + ex);
                    painter.fillRect(rr, QColor("yellow"));

                    QColor lineCol = QColor("red");
                    QPen pen = painter.pen();
                    painter.setPen(lineCol);
                    if (blockFS == d->m_findScopeStart.block())
                        painter.drawLine(rr.topLeft(), rr.topRight());
                    if (blockFS == d->m_findScopeEnd.block())
                        painter.drawLine(rr.bottomLeft(), rr.bottomRight());
                    painter.drawLine(rr.topLeft(), rr.bottomLeft());
                    painter.drawLine(rr.topRight(), rr.bottomRight());
                    painter.setPen(pen);
                }
            }
            offsetFS.ry() += r.height();

            if (offsetFS.y() > viewport()->rect().height())
                break;

            blockFS = blockFS.next();
            if (!blockFS.isVisible()) {
                // invisible blocks do have zero line count
                blockFS = document()->findBlockByLineNumber(blockFS.firstLineNumber()); //findBlockByNumber?
            }

        }
    }*/
}

void TextEdit::compositeExtraSelections()
{
    ExtraSelectionList fullList;

    for (const auto& list : m_extraSelections)
        fullList << list;

    setExtraSelections(fullList);
    // setExtraSelections( m_extraSelections[ESSearchRange] );
}

static void fillBackground(QPainter* p, const QRectF& rect, QBrush brush, const QRectF& gradientRect = QRectF())
{
    p->save();
    if (brush.style() >= Qt::LinearGradientPattern && brush.style() <= Qt::ConicalGradientPattern) {
        if (!gradientRect.isNull()) {
            QTransform m = QTransform::fromTranslate(gradientRect.left(), gradientRect.top());
            m.scale(gradientRect.width(), gradientRect.height());
            brush.setTransform(m);
            const_cast<QGradient*>(brush.gradient())->setCoordinateMode(QGradient::LogicalMode);
        }
    } else {
        p->setBrushOrigin(rect.topLeft());
    }
    p->fillRect(rect, brush);
    p->restore();
}

void TextEdit::paintEvent(QPaintEvent* e)
{
    compositeExtraSelections();

    // What follows is a copy of QPlainTextEdit::paintEvent with some modifications to allow different
    // line-highlighting.
    QPainter painter(viewport());
    Q_ASSERT(qobject_cast<QPlainTextDocumentLayout*>(document()->documentLayout()));

    QPointF offset(contentOffset());

    QRect er = e->rect();
    QRect viewportRect = viewport()->rect();

    bool editable = !isReadOnly();

    QTextBlock block = firstVisibleBlock();
    qreal maximumWidth = document()->documentLayout()->documentSize().width();

    // Set a brush origin so that the WaveUnderline knows where the wave started
    painter.setBrushOrigin(offset);

    // keep right margin clean from full-width selection
    int maxX = offset.x() + qMax((qreal)viewportRect.width(), maximumWidth) - document()->documentMargin();
    er.setRight(qMin(er.right(), maxX));
    painter.setClipRect(er);

    QAbstractTextDocumentLayout::PaintContext context = getPaintContext();

    while (block.isValid()) {
        QRectF r = blockBoundingRect(block).translated(offset);

        if (r.bottom() >= er.top())
            break;

        offset.ry() += r.height();
        if (offset.y() > viewportRect.height())
            break;

        block = block.next();
    }

    QTextBlock beginBlock = block;

    while (block.isValid()) {
        QRectF r = blockBoundingRect(block).translated(offset);
        QTextLayout* layout = block.layout();

        if (!block.isVisible()) {
            offset.ry() += r.height();
            block = block.next();
            continue;
        }

        if (r.top() > er.bottom())
            break;

        QTextBlockFormat blockFormat = block.blockFormat();

        QBrush bg = blockFormat.background();
        // QBrush bg( getTheme().editorColor(Theme::BackgroundColor) );
        if (bg != Qt::NoBrush) {
            QRectF contentsRect = r;
            contentsRect.setWidth(qMax(r.width(), maximumWidth));
            fillBackground(&painter, contentsRect, bg);
        }

        // The last selection is the text selection added by Qt. Since the original one removes
        // foregroundColor we've got to intercept it.
        if (context.selections.size() > 0 && textCursor().hasSelection()) {
            auto& s = context.selections.last();

            s.format.clearForeground();
            s.format.setBackground(QBrush(getTheme().editorColor(Theme::TextSelection)));

            // context.selections.pop_back();

            /*QAbstractTextDocumentLayout::Selection selection;
            selection.cursor = textCursor();

            //QPalette::ColorGroup cg = d->hasFocus ? QPalette::Active : QPalette::Inactive;
            selection.format.setBackground(
            QBrush(getTheme().editorColor(Theme::CurrentLine)) );
            //selection.format.setBackground(m_currentTheme.getColor(Theme::TextEditActiveBackground));
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            context.selections.push_back(selection);*/
        }

        QVector<QTextLayout::FormatRange> selections;
        int blpos = block.position();
        int bllen = block.length();
        for (int i = 0; i < context.selections.size(); ++i) {
            const QAbstractTextDocumentLayout::Selection& range = context.selections.at(i);
            const int selStart = range.cursor.selectionStart() - blpos;
            const int selEnd = range.cursor.selectionEnd() - blpos;
            if (selStart < bllen && selEnd > 0 && selEnd > selStart) {
                QTextLayout::FormatRange o;
                o.start = selStart;
                o.length = selEnd - selStart;
                o.format = range.format;
                selections.append(o);
            } else if (!range.cursor.hasSelection() && range.format.hasProperty(QTextFormat::FullWidthSelection) &&
                       block.contains(range.cursor.position())) {
                // for full width selections we don't require an actual selection, just
                // a position to specify the line. that's more convenience in usage.
                QTextLayout::FormatRange o;
                QTextLine l = layout->lineForTextPosition(range.cursor.position() - blpos);
                o.start = l.textStart();
                o.length = l.textLength();
                if (o.start + o.length == bllen - 1)
                    ++o.length; // include newline
                o.format = range.format;
                selections.append(o);
            }
        }

        // context.cursorPosition = getAbsoluteCursorPosition();
        bool drawCursor = ((editable || (textInteractionFlags() & Qt::TextSelectableByKeyboard)) &&
                           context.cursorPosition >= blpos && context.cursorPosition < blpos + bllen);

        // int curPos = getAbsoluteCursorPosition();
        // drawCursor = curPos >= blpos && curPos < blpos + bllen;

        // drawCursor = drawCursor && m_t.elapsed() % 1000 < 500;

        bool drawCursorAsBlock = drawCursor && overwriteMode();

        if (drawCursorAsBlock) {
            if (context.cursorPosition == blpos + bllen - 1) {
                drawCursorAsBlock = false;
            } else {
                QTextLayout::FormatRange o;
                o.start = context.cursorPosition - blpos;
                o.length = 1;
                o.format.setForeground(palette().base());
                o.format.setBackground(palette().text());
                selections.append(o);
            }
        }

        if (!placeholderText().isEmpty() && document()->isEmpty() && layout->preeditAreaText().isEmpty()) {
            QColor col = palette().text().color();
            col.setAlpha(128);
            painter.setPen(col);
            const int margin = int(document()->documentMargin());
            painter.drawText(r.adjusted(margin, 0, 0, 0), Qt::AlignTop | Qt::TextWordWrap, placeholderText());
        } else {
            layout->draw(&painter, offset, selections, er);
        }
        if ((drawCursor && !drawCursorAsBlock) ||
            (editable && context.cursorPosition < -1 && !layout->preeditAreaText().isEmpty())) {
            int cpos = context.cursorPosition;
            if (cpos < -1)
                cpos = layout->preeditAreaPosition() - (cpos + 2);
            else
                cpos -= blpos;
            layout->drawCursor(&painter, offset, cpos, cursorWidth());
        }

        offset.ry() += r.height();
        if (offset.y() > viewportRect.height())
            break;
        block = block.next();
    }

    const QTextBlock& endBlock = block;
    block = beginBlock;
    auto curr = std::upper_bound(
        m_editorLabels.begin(), m_editorLabels.end(), block.position(), [](int val, const EditorLabelPtr& ptr) {
            return val < ptr->m_absPos;
        });

    while (beginBlock.isValid()) {
        if (!block.isVisible())
            continue;

        auto blockEndPos = block.position() + block.length();
        for (; curr != m_editorLabels.end() && curr->get()->m_absPos <= blockEndPos; ++curr) {
            // draw curr
        }
    }

    if (beginBlock.isValid()) {
        QTextBlock db = beginBlock;
        int bc = 1;

        while (bc-- > 0) {
            auto prev = db.previous();
            if (!prev.isValid())
                break;
            db = prev;
        }

        int startingPos = db.position();
        auto lower = std::upper_bound(
            m_editorLabels.begin(), m_editorLabels.end(), startingPos, [](int val, const EditorLabelPtr& ptr) {
                return val < ptr->m_absPos;
            });
        auto upper = m_editorLabels.end(); // FIXME

        bool wantRepaint = false;

        for (; lower != upper; ++lower) {
            auto ptr = *lower;

            if (ptr->m_changed) {
                if (ptr->updateDisplayRect()) {
                    ptr->updatePixmap();
                    wantRepaint = true;
                }
                ptr->m_changed = false;
            }

            QTextBlock b = document()->findBlock(ptr->m_absPos);
            auto op = blockBoundingGeometry(b).translated(contentOffset()).topLeft();

            ptr->draw(painter, op);
        }

        if (wantRepaint)
            QTimer::singleShot(0, [this]() mutable { viewport()->repaint(); });
    }

    if (backgroundVisible() && !block.isValid() && offset.y() <= er.bottom() &&
        (centerOnScroll() || verticalScrollBar()->maximum() == verticalScrollBar()->minimum())) {
        painter.fillRect(QRect(QPoint((int)er.left(), (int)offset.y()), er.bottomRight()), palette().background());
    }

    auto bl = getBlocksInRect(e->rect());
    paintLineBreaks(painter, bl);
    paintLineSuffixes(painter, bl);
}

TextEdit::BlockList TextEdit::getBlocksInViewport() const
{
    return getBlocksInRect(viewport()->rect());
}

TextEdit::BlockList TextEdit::getBlocksInRect(QRect rect) const
{
    BlockList bl;
    auto block = firstVisibleBlock();
    auto contentOff = contentOffset();

    while (block.isValid()) {
        const auto geom = blockBoundingGeometry(block).translated(contentOff).toRect();

        if (geom.bottom() >= rect.top()) {
            bl.push_back(BlockData{block, geom});

            if (geom.top() > rect.bottom())
                break;
        }

        block = block.next();
    }

    // m_blockListCounter += bl.size();
    // qDebug() << m_blockListCounter << bl.size(); // << rect << contentOff << viewport()->height();
    return bl;
}

int TextEdit::cursorPosToAbsolutePos(const TextEdit::CursorPos& pos) const
{
    const auto& block = document()->findBlockByNumber(pos.line);
    const auto col = std::max(0, std::min(pos.column, block.length() - 1));
    return block.position() + col;
}

QTextBlock TextEdit::blockAtPosition(int y) const
{
    auto block = firstVisibleBlock();
    if (!block.isValid())
        return QTextBlock();

    const auto geom = blockBoundingGeometry(block).translated(contentOffset()).toRect();
    int top = geom.top();
    int bottom = top + geom.height();

    do {
        if (top <= y && y <= bottom)
            return block;
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    } while (block.isValid());
    return QTextBlock();
}

void TextEdit::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    updateSidebarGeometry();
}

QTextBlock TextEdit::findClosingBlock(const QTextBlock& startBlock) const
{
    return m_highlighter->findFoldingRegionEnd(startBlock);

    /*int depth = 1;
    auto currBlock = startBlock.next();

    while (currBlock.isValid()) {
        if (currBlock.text().startsWith("}"))
            depth--;
        else if (currBlock.text().startsWith("{"))
            depth++;

        if (depth == 0)
            return currBlock;

        currBlock = currBlock.next();
    }

    return QTextBlock();*/
}

bool TextEdit::isFoldable(const QTextBlock& block) const
{
    return m_highlighter->startsFoldingRegion(block);
}

bool TextEdit::isFolded(const QTextBlock& block) const
{
    if (!block.isValid())
        return false;
    const auto nextBlock = block.next();
    if (!nextBlock.isValid())
        return false;
    return !nextBlock.isVisible();
}

void TextEdit::toggleFold(const QTextBlock& startBlock)
{
    // we also want to fold the last line of the region, therefore the ".next()"
    const auto endBlock = findClosingBlock(startBlock).next();

    if (isFolded(startBlock)) {
        // unfold
        auto block = startBlock.next();
        while (block.isValid() && !block.isVisible()) {
            block.setVisible(true);
            block.setLineCount(block.layout()->lineCount());
            block = block.next();
        }

    } else {
        // fold
        auto block = startBlock.next();
        while (block.isValid() && block != endBlock) {
            block.setVisible(false);
            block.setLineCount(0);
            block = block.next();
        }
    }

    // redraw document
    document()->markContentsDirty(startBlock.position(), endBlock.position() - startBlock.position() + 1);

    // update scrollbars
    emit document()->documentLayout()->documentSizeChanged(document()->documentLayout()->documentSize());
}

std::pair<TextEdit::EditorLabelIterator, TextEdit::EditorLabelIterator> TextEdit::getEditorLabelsInRange(
    int begin, int end)
{
    const auto lower =
        std::upper_bound(m_editorLabels.begin(), m_editorLabels.end(), begin, [](int val, const EditorLabelPtr& ptr) {
            return val < ptr->m_absPos;
        });

    if (lower == m_editorLabels.end())
        return {lower, lower};

    const auto upper =
        std::find_if(lower, m_editorLabels.end(), [end](const EditorLabelPtr& ptr) { return ptr->m_absPos > end; });
    return {lower, upper};
}

std::pair<TextEdit::EditorLabelIterator, TextEdit::EditorLabelIterator> TextEdit::getEditorLabelsInBlock(
    const QTextBlock& block)
{
    const auto begin = block.position();
    const auto end = begin + block.length();
    return getEditorLabelsInRange(begin, end);
}

WeakEditorLabelPtr TextEdit::getEditorLabelAtPos(int pos)
{
    auto it = std::find_if(m_editorLabels.begin(), m_editorLabels.end(), [pos](const EditorLabelPtr& ptr) {
        return pos == ptr->m_absPos;
    });

    if (it != m_editorLabels.end())
        return *it;
    else
        return WeakEditorLabelPtr();
}

void TextEdit::removeEditorLabel(WeakEditorLabelPtr label)
{
    if (label.expired())
        return;

    auto it = std::find(m_editorLabels.begin(), m_editorLabels.end(), label.lock());

    if (it != m_editorLabels.end()) {
        m_editorLabels.erase(it);
        QTimer::singleShot(0, [this]() mutable { viewport()->repaint(); });
    }
}

WeakEditorLabelPtr TextEdit::addEditorLabel(EditorLabelPtr label)
{
    const int pos = label->m_absPos;
    auto it =
        std::lower_bound(m_editorLabels.begin(), m_editorLabels.end(), pos, [](const EditorLabelPtr& ptr, int pos) {
            return ptr->m_absPos < pos;
        });
    return *m_editorLabels.insert(it, label);
}

} // namespace ote
