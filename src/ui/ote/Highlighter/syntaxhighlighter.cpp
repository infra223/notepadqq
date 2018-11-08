/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "syntaxhighlighter.h"

#include "abstracthighlighter_p.h"
#include "definition.h"
#include "format.h"
#include "theme.h"
#include "state.h"
#include "fmtrangelist.h"
#include "foldingregion.h"
#include "../util/scopeguard.h"

#include <QDebug>
#include <QTextDocument>

Q_DECLARE_METATYPE(QTextBlock)

using namespace ote;

namespace ote {

class TextBlockUserData : public QTextBlockUserData {
public:
    State state;
    QVector<FoldingRegion> foldingRegions;
    FmtRangeList fmtList;
    bool bookmarked = false;
    bool forceRehighlighting = false;

    std::map<int, std::unique_ptr<PluginBlockData>> extraData;
};

class SyntaxHighlighterPrivate : public AbstractHighlighterPrivate {
public:
    static FoldingRegion foldingRegion(const QTextBlock& startBlock);
    QVector<FoldingRegion> foldingRegions;
    FmtRangeList fmtList;
};

FoldingRegion SyntaxHighlighterPrivate::foldingRegion(const QTextBlock& startBlock)
{
    const auto data = dynamic_cast<TextBlockUserData*>(startBlock.userData());
    if (!data)
        return FoldingRegion();
    for (int i = data->foldingRegions.size() - 1; i >= 0; --i) {
        if (data->foldingRegions.at(i).type() == FoldingRegion::Begin)
            return data->foldingRegions.at(i);
    }
    return FoldingRegion();
}

unsigned int SyntaxHighlighter::s_continuousIterations = 0;

SyntaxHighlighter::SyntaxHighlighter(QObject* parent)
    : QSyntaxHighlighter(parent)
    , AbstractHighlighter(new SyntaxHighlighterPrivate)
{
    qRegisterMetaType<QTextBlock>();
}

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* document)
    : QSyntaxHighlighter(document)
    , AbstractHighlighter(new SyntaxHighlighterPrivate)
{
    qRegisterMetaType<QTextBlock>();
}

SyntaxHighlighter::~SyntaxHighlighter() {}

void SyntaxHighlighter::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;

    m_enabled = enabled;
    if (m_enabled) startRehighlighting();
}

void SyntaxHighlighter::setDefinition(const Definition& def)
{
    if (definition() == def)
        return;

    AbstractHighlighter::setDefinition(def);
    startRehighlighting();
}

bool SyntaxHighlighter::startsFoldingRegion(const QTextBlock& startBlock) const
{
    return SyntaxHighlighterPrivate::foldingRegion(startBlock).type() == FoldingRegion::Begin;
}

QTextBlock SyntaxHighlighter::findFoldingRegionEnd(const QTextBlock& startBlock) const
{
    const auto region = SyntaxHighlighterPrivate::foldingRegion(startBlock);

    auto block = startBlock;
    int depth = 1;
    while (block.isValid()) {
        block = block.next();
        const auto data = dynamic_cast<TextBlockUserData*>(block.userData());
        if (!data)
            continue;
        for (auto it = data->foldingRegions.constBegin(); it != data->foldingRegions.constEnd(); ++it) {
            if ((*it).id() != region.id())
                continue;
            if ((*it).type() == FoldingRegion::End)
                --depth;
            else if ((*it).type() == FoldingRegion::Begin)
                ++depth;
            if (depth == 0)
                return block;
        }
    }

    return QTextBlock();
}

bool SyntaxHighlighter::isBookmarked(const QTextBlock& block) const
{
    auto data = dynamic_cast<TextBlockUserData*>(block.userData());
    if (!data)
        return false;

    return data->bookmarked;
}

void SyntaxHighlighter::setBookmark(QTextBlock block, bool bookmarked)
{
    auto data = dynamic_cast<TextBlockUserData*>(block.userData());
    if (!data) {
        data = new TextBlockUserData();
        block.setUserData(data);
    }
    data->bookmarked = bookmarked;
}

void SyntaxHighlighter::toggleBookmark(QTextBlock block)
{
    auto data = dynamic_cast<TextBlockUserData*>(block.userData());
    if (!data) {
        data = new TextBlockUserData();
        block.setUserData(data);
    }
    data->bookmarked = !data->bookmarked;
}

bool SyntaxHighlighter::isPositionInComment(int absPos, int len) const
{
    const auto& block = document()->findBlock(absPos);
    const auto t = block.text();
    auto data = dynamic_cast<TextBlockUserData*>(block.userData());
    if (!data)
        return false;

    const auto start = absPos - block.position();
    return data->fmtList.isFormat(start, start + len, 'c');
}

bool SyntaxHighlighter::isPositionInString(int absPos, int len) const
{
    const auto& block = document()->findBlock(absPos);
    auto data = dynamic_cast<TextBlockUserData*>(block.userData());
    if (!data)
        return false;

    const auto start = absPos - block.position();
    return data->fmtList.isFormat(start, start + len, 's');
}

void SyntaxHighlighter::startRehighlighting()
{
    const auto firstBlock = document()->firstBlock();
    auto data = dynamic_cast<TextBlockUserData*>(firstBlock.userData());
    if (data)
        data->forceRehighlighting = true;

    if (firstBlock.isValid())
        QMetaObject::invokeMethod(this, "rehighlightBlock", Qt::QueuedConnection, Q_ARG(QTextBlock, firstBlock));
}

void SyntaxHighlighter::setPluginBlockData(const QTextBlock& block, int id, std::unique_ptr<PluginBlockData> data)
{
    auto blockData = reinterpret_cast<TextBlockUserData*>(block.userData());
    Q_ASSERT_X(blockData, "SyntaxHighlighter", "blockData must not be null");

    blockData->extraData[id] = std::move(data);
}

PluginBlockData* SyntaxHighlighter::getPluginBlockData(const QTextBlock& block, int id)
{
    auto blockData = reinterpret_cast<TextBlockUserData*>(block.userData());
    Q_ASSERT_X(blockData, "SyntaxHighlighter", "blockData must not be null");

    auto it = blockData->extraData.find(id);

    if (it == blockData->extraData.end())
        return nullptr;

    return it->second.get();
}

const PluginBlockData* SyntaxHighlighter::getPluginBlockData(const QTextBlock& block, int id) const
{
    auto blockData = reinterpret_cast<TextBlockUserData*>(block.userData());
    Q_ASSERT_X(blockData, "SyntaxHighlighter", "blockData must not be null");

    auto it = blockData->extraData.find(id);

    if (it == blockData->extraData.end())
        return nullptr;

    return it->second.get();
}

void SyntaxHighlighter::highlightBlock(const QString& text)
{
    Q_D(SyntaxHighlighter);

    auto userData = dynamic_cast<TextBlockUserData*>(currentBlockUserData());
    const bool newUserData = !userData;
    if (newUserData) {
        // TODO: All blocks get their TextBlockuserData objects created in one batch at
        // the start. This blocks the main thread and leads to a short delay (~0.5s for 100k lines)
        // Possible improvements:
        //  * move this part outside the highlighter+ensure it'll be run before highlightBlock()
        //  * Allocate a big chunk of memory and put all UserData objects in there.
        userData = new TextBlockUserData();
        setCurrentBlockUserData(userData);
    }

    if (!m_enabled)
        return;

    QTextBlock currBlock = currentBlock();
    State state;
    if (currBlock.position() > 0) {
        const auto prevBlock = currBlock.previous();
        const auto prevData = dynamic_cast<TextBlockUserData*>(prevBlock.userData());
        Q_ASSERT_X(prevData, "SyntaxHighlighter", "prevData should not be null");
        state = prevData->state;
    }
    d->foldingRegions.clear();
    d->fmtList.clear();
    state = highlightLine(text, state);

    // Note: This lambda is executed at function exit
    DEFER { emit blockHighlighted(currBlock); };

    if (newUserData) { // first time we highlight this
        userData->state = state;
        userData->foldingRegions = d->foldingRegions;
        userData->fmtList = std::move(d->fmtList);
        return;
    }
    const bool forceRehighlighted = userData->forceRehighlighting;
    userData->forceRehighlighting = false;
    userData->fmtList = std::move(d->fmtList);

    // we ended up in the same state, so we are done here
    if (!forceRehighlighted && userData->state == state && userData->foldingRegions == d->foldingRegions) {
        return;
    }
    userData->state = state;
    userData->foldingRegions = d->foldingRegions;

    const auto nextBlock = currBlock.next();
    if (!nextBlock.isValid()) {
        return;
    }

    if (forceRehighlighted) { // Proliferate rehighlighting to next block if needed
        auto nextData = dynamic_cast<TextBlockUserData*>(nextBlock.userData());
        Q_ASSERT_X(nextData, "SyntaxHighlighter", "nextData should not be null");
        nextData->forceRehighlighting = true;
    }

    // The highlighter will automagically keep highlighting text blocks for as long as the block's user state
    // has changed during highlighting. However, this runs on the main thread so we shouldn't let it execute too
    // often in a row. Every some iterations we instead queue up the highlighting to continue on next tick.
    // It's good to immediately queue once in order to not block the main thread during program creation.
    if (s_continuousIterations++ % 256 == 0) {
        QMetaObject::invokeMethod(this, "rehighlightBlock", Qt::QueuedConnection, Q_ARG(QTextBlock, nextBlock));
    }
    else
        currBlock.setUserState(currBlock.userState() + 1);
}

void SyntaxHighlighter::applyFormat(int offset, int length, const Format& format)
{
    if (format.isDefaultTextStyle(theme()) || length == 0)
        return;

    QTextCharFormat tf;
    if (format.hasTextColor(theme()))
        tf.setForeground(format.textColor(theme()));
    if (format.hasBackgroundColor(theme()))
        tf.setBackground(format.backgroundColor(theme()));

    if (format.isBold(theme()))
        tf.setFontWeight(QFont::Bold);
    if (format.isItalic(theme()))
        tf.setFontItalic(true);
    if (format.isUnderline(theme()))
        tf.setFontUnderline(true);
    if (format.isStrikeThrough(theme()))
        tf.setFontStrikeOut(true);

    if (format.isComment()) {
        Q_D(SyntaxHighlighter);
        d->fmtList.append(offset, offset + length, 'c');
    } else if (format.isString()) {
        Q_D(SyntaxHighlighter);
        d->fmtList.append(offset, offset + length, 's');
    }

    QSyntaxHighlighter::setFormat(offset, length, tf);
}

void SyntaxHighlighter::applyFolding(int offset, int length, FoldingRegion region)
{
    Q_UNUSED(offset);
    Q_UNUSED(length);
    Q_D(SyntaxHighlighter);

    if (region.type() == FoldingRegion::Begin)
        d->foldingRegions.push_back(region);

    if (region.type() == FoldingRegion::End) {
        for (int i = d->foldingRegions.size() - 1; i >= 0; --i) {
            if (d->foldingRegions.at(i).id() != region.id() || d->foldingRegions.at(i).type() != FoldingRegion::Begin)
                continue;
            d->foldingRegions.remove(i);
            return;
        }
        d->foldingRegions.push_back(region);
    }
}

} // namespace ote
