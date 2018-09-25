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

#pragma once

#ifndef KSYNTAXHIGHLIGHTING_QSYNTAXHIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTING_QSYNTAXHIGHLIGHTER_H

#include "abstracthighlighter.h"

#include <QSyntaxHighlighter>

namespace ote {

class PluginBlockData {
public:
    virtual ~PluginBlockData() = default;
};

class SyntaxHighlighterPrivate;

/** A QSyntaxHighlighter implementation for use with QTextDocument.
 *  This supports partial re-highlighting during editing and
 *  tracks syntax-based code folding regions.
 *
 *  @since 5.28
 */
class SyntaxHighlighter : public QSyntaxHighlighter, public AbstractHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QObject *parent = nullptr);
    explicit SyntaxHighlighter(QTextDocument *document);
    ~SyntaxHighlighter() override;

    void setDefinition(const Definition &def) override;

    /** Returns whether there is a folding region beginning at @p startBlock.
     *  This only considers syntax-based folding regions,
     *  not indention-based ones as e.g. found in Python.
     *
     *  @see findFoldingRegionEnd
     */
    bool startsFoldingRegion(const QTextBlock &startBlock) const;

    /** Finds the end of the folding region starting at @p startBlock.
     *  If multiple folding regions begin at @p startBlock, the end of
     *  the last/innermost one is returned.
     *  This returns an invalid block if no folding region end is found,
     *  which typically indicates an unterminated region and thus folding
     *  until the document end.
     *  This method performs a sequential search starting at @p startBlock
     *  for the matching folding region end, which is a potentially expensive
     *  operation.
     *
     *  @see startsFoldingRegion
     */
    QTextBlock findFoldingRegionEnd(const QTextBlock &startBlock) const;

    /**
     * Returns whether the range [absPos,absPos+len] is inside a comment-formatted section.
     */
    bool isPositionInComment(int absPos, int len=0) const;

    /**
     * Returns whether the range [absPos,absPos+len] is inside a string-formatted section.
     */
    bool isPositionInString(int absPos, int len=0) const;

    /**
     * Starts an asynchronous rehighighting job for the whole document.
     */
    void startRehighlighting();


    void setPluginBlockData(const QTextBlock& block, int id, std::unique_ptr<PluginBlockData> data);
    PluginBlockData* getPluginBlockData(const QTextBlock& block, int id);
    const PluginBlockData* getPluginBlockData(const QTextBlock& block, int id) const;

signals:
    /**
     * Emitted for every TextBlock after highlighting has happened.
     */
    void blockChanged(const QTextBlock& block);

protected:
    void highlightBlock(const QString & text) override;
    void applyFormat(int offset, int length, const Format &format) override;
    void applyFolding(int offset, int length, FoldingRegion region) override;

private:
    Q_DECLARE_PRIVATE_D(AbstractHighlighter::d_ptr, SyntaxHighlighter)
};
}

#endif // KSYNTAXHIGHLIGHTING_QSYNTAXHIGHLIGHTER_H
