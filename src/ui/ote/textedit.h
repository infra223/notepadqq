#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <vector>

#include <QPlainTextEdit>
#include <QTextBlock>

#include <QElapsedTimer>
//#include "Highlighting/compositehighlighter.h"
//#include "Themes/theme.h"
//#include "Syntax/syntaxdefinition.h"

#include "Highlighter/syntaxhighlighter.h"
#include "Highlighter/repository.h"

#include "Highlighter/theme.h"
#include "Highlighter/definition.h"


namespace ote {

class TextEditGutter;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget* parent);

    void setTheme(const KSyntaxHighlighting::Theme& theme);
    KSyntaxHighlighting::Theme getTheme() const { return m_highlighter->theme(); }

    void setDefinition(const KSyntaxHighlighting::Definition& d);
    KSyntaxHighlighting::Definition getDefinition() const { return m_highlighter->definition(); }

    void setSyntaxHighlightingEnabled(bool enabled);

    void setEndOfLineMarkersVisible(bool enable);
    void setWhitespaceVisible(bool show);
    void setShowLinebreaks(bool show);
    void setSmartIndent(bool enable);
    void setTabToSpaces(bool enable);
    void setWordWrap(bool enable);
    void setTabWidth(int tabWidth);
    void setFont(QFont font);

    bool isTabToSpaces() const;
    int getTabWidth() const;

    QString getCurrentWord() const;
    int getLineCount() const;
    int getCharCount() const;

    // Cursor Positioning
    // Note that these functions take input starting from 0. First line, first character is {0,0}
    struct CursorPos {
        int line = 0;
        int column = 0;
    };

    struct Selection {
        CursorPos start;
        CursorPos end;
    };

    void setCursorPosition(int line, int column);
    void setCursorPosition(const CursorPos& pos);
    CursorPos getCursorPosition() const;

    void setAbsoluteCursorPosition(int pos);
    int getAbsoluteCursorPosition() const;

    // Selection
    QString getSelectedText() const;
    Selection getSelection() const;
    void setSelection(const Selection& sel);
    void setTextInSelection(const QString& text, bool keepSelection=true);

    // Scrolling
    QPoint getScrollPosition() const;
    void setScrollPosition(const QPoint& p);

    // Finding and replacing
    // TODO: Add overloads for QRegularExpression
    using FindFlags = QTextDocument::FindFlags;
    using FindFlag = QTextDocument::FindFlag;

    bool findTentative(const QString& term, FindFlags flags = FindFlags());
    bool find(const QString& term, FindFlags flags = FindFlags());
    bool find(const QString& term, int startPos, int endPos=-1, FindFlags flags = FindFlags(), bool wrapAround=true);

    //bool find(const QRegExp &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags()) = delete;

    bool isSearchTermSelected() const { return m_findTermSelected; }

    QElapsedTimer m_t;

    // Zoom
    void resetZoom();
    void setZoomTo(int value);
    void zoomIn();
    void zoomOut();
    int getZoomLevel() const;

    // Modification status
    void clearHistory();
    int getModificationRevision() const;
    bool isModified() const;
    void setModified(bool modified);

    // Text editing
    void moveSelectedBlocksUp();
    void moveSelectedBlocksDown();
    void duplicateSelectedBlocks();
    void deleteSelectedBlocks();

    // Whitespace management
    void convertLeadingWhitespaceToTabs();
    void convertLeadingWhitespaceToSpaces();
    void trimWhitespace(bool leading, bool trailing);

signals:
    void cursorPositionChanged();
    void contentsChanged();
    void mouseWheelUsed(QWheelEvent *ev);
    void gotFocus();
    void urlsDropped(const QList<QUrl>& urls);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent* event) override;
    void dropEvent(QDropEvent *event) override;

    void focusInEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* e) override;

    void contextMenuEvent(QContextMenuEvent* event) override;
private:
    friend class TextEditGutter;

    QTextBlock blockAtPosition(int y) const;
    void updateSidebarGeometry();
    void updateSidebarArea(const QRect& rect, int dy);

    void onCursorPositionChanged();
    void onSelectionChanged();

private:
    struct BlockData {
        QTextBlock block;
        QRect translatedRect;
    };

    enum ESType {
        ESLineHighlight = 0,
        ESSameItems,
        ESMatchingBrackets,
    };

    typedef std::vector<BlockData> BlockList;
    typedef QList<QTextEdit::ExtraSelection> ExtraSelectionList;
    typedef QMap<ESType, ExtraSelectionList> ExtraSelectionMap;

    mutable int m_blockListCounter = 0;

    BlockList getBlocksInViewport() const;
    BlockList getBlocksInRect(QRect rect) const;

    int cursorPosToAbsolutePos(const CursorPos& pos) const;

    void resizeEvent(QResizeEvent* event) override;
    QTextBlock findClosingBlock(const QTextBlock& startBlock) const;
    void paintLineBreaks(QPainter& painter, const BlockList& blockList) const;
    void paintSearchBlock(QPainter& painter, const QRect& eventRect, const QTextBlock& block);
    void paintEndOfLineMarkers(QPainter& painter, const BlockList& blockList) const;
    void compositeExtraSelections();

    bool m_showEndOfLineMarkers = false;
    bool m_showLinebreaks = false;
    bool m_smartIndent = false;
    bool m_tabToSpaces = false;

    bool m_findActive = false;
    bool m_findTermSelected = false;
    QString m_findTerm;
    QPair<int,int> m_findRange;
    QTextDocument::FindFlags m_findFlags;

    int m_tabWidth = 4;
    int m_pointZoom = 0;

    ExtraSelectionMap m_extraSelections;

    //CompositeHighlighter* m_highlighter;
    TextEditGutter* m_sideBar;

    //Theme m_currentTheme;
    //SyntaxDefinition m_currentSyntaxDefinition;


    //static KSyntaxHighlighting::Repository m_repository;
    KSyntaxHighlighting::SyntaxHighlighter* m_highlighter;

    void createParenthesisSelection(int pos);

    void highlightCurrentLine();

public:
    static KSyntaxHighlighting::Repository& getRepository() {
        static KSyntaxHighlighting::Repository repo;
        return repo;
    }
    static void initRepository();

    bool isFoldable(const QTextBlock& block) const;
    bool isFolded(const QTextBlock& block) const;
    void toggleFold(const QTextBlock& startBlock);
};

} // namespace ote

#endif // TEXTEDITOR_H
