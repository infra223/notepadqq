#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTimer>

#include <vector>

#include "Highlighter/syntaxhighlighter.h"
#include "Highlighter/repository.h"
#include "Highlighter/theme.h"
#include "Highlighter/definition.h"

#include "editorlabel.h"

namespace ote {

class TextEditGutter;

class TextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget* parent);

    void setTheme(const Theme& theme);
    Theme getTheme() const { return m_highlighter->theme(); }

    void setDefinition(const Definition& d);
    Definition getDefinition() const { return m_highlighter->definition(); }

    SyntaxHighlighter* getHighlighter() { return m_highlighter; }
    const SyntaxHighlighter* getHighlighter() const { return m_highlighter; }

    // Various text formatting and display options
    void setEndOfLineMarkersVisible(bool enable);
    void setWhitespaceVisible(bool show);
    void setShowLinebreaks(bool show);
    void setSmartIndent(bool enable);
    void setTabToSpaces(bool enable);
    void setWordWrap(bool enable);
    void setTabWidth(int width);
    
    void setFont(QFont font);
    QFont getFont() const;

    // Returns whether the editor automatically turns tabs to spaces on user input.
    bool isTabToSpaces() const;
    
    // Returns the width of a tab as the number of spaces it takes to reach the same width.
    int getTabWidth() const;

    QString getCurrentWord() const;
    int getLineCount() const;
    int getCharCount() const;

    // Cursor Positioning
    // Note that these functions take input starting from 0.

    using CursorPos = int;
    std::pair<int,int> getLineColumnForCursorPos(const CursorPos& p);
    CursorPos getCursorPosForLineColumn(int line, int column);

    struct Selection {
        Selection() = default;
        Selection(int start, int end) : start(start), end(end) {}

        CursorPos start = 0;
        CursorPos end = 0;

        bool isValid() const { return start >= 0 && end >= start; }
        bool length() const { return end-start; }
        bool hasSelection() const { return length() > 0; }
    };

    void setCursorPosition(int line, int column);
    void setCursorPosition(const CursorPos& pos);
    CursorPos getCursorPosition() const;

    // Selection
    QStringList getSelectedTexts() const;

    QString getSelectedText() const;
    Selection getSelection() const;
    std::vector<Selection> getSelections() const;
    void setSelection(const Selection& sel);
    void setSelections(const std::vector<Selection>& selections);
    void setTextInSelection(const QString& text, bool keepSelection=true);
    void setTextInSelections(const QStringList& texts, bool keepSelection=true);

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
    std::vector<Selection> findAll(const QString& term, int startPos=0, int endPos=-1, FindFlags flags = FindFlags());

    //bool find(const QRegExp &exp, QTextDocument::FindFlags options = QTextDocument::FindFlags()) = delete;

    // Returns whether the currently selected text is the one selected by the most recent call of find(...)
    bool isSearchTermSelected() const { return m_findTermSelected; }

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
    
    // Returns a static instance of ote::Repository. Use this to get themes or definitions
    static Repository& getRepository() {
        return *s_repository;
    }
    
    // This needs to be called before any TextEdits are created
    static void initRepository(const QString& path);

    bool isFoldable(const QTextBlock& block) const;
    bool isFolded(const QTextBlock& block) const;
    void toggleFold(const QTextBlock& startBlock);

    // Editor labels
    typedef std::vector<EditorLabelPtr>::iterator EditorLabelIterator;
    typedef std::pair<EditorLabelIterator, EditorLabelIterator> EditorLabelRange;

    EditorLabelRange getEditorLabelsInRange(int begin, int end);
    EditorLabelRange getEditorLabelsInBlock(const QTextBlock& block);

    void deleteMarkedEditorLabelsInRange(const EditorLabelRange& range);

    WeakEditorLabelPtr getEditorLabelAtPos(int pos);
    void removeEditorLabel(WeakEditorLabelPtr label);
    WeakEditorLabelPtr addEditorLabel(EditorLabelPtr label);

signals:
    //void cursorPositionChanged();
    void blockChanged(const QTextBlock& block);
    void contentsChanged();
    void mouseWheelUsed(QWheelEvent *ev);
    void gotFocus();
    void urlsDropped(const QList<QUrl>& urls);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void wheelEvent(QWheelEvent* event) override;
    void dropEvent(QDropEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *) override;

    void focusInEvent(QFocusEvent* event) override;
    void paintEvent(QPaintEvent* e) override;

    void contextMenuEvent(QContextMenuEvent* event) override;
    
private:
    friend class TextEditGutter;
    friend class PluginBase;

    // Multiple-cursor selection
    bool mcsMoveOperation(QKeyEvent *evt);
    void mcsInsertText(const QString& text);
    bool mcsAddCursor(const QTextCursor& c);
    void mcsEnsureUniqueCursors();
    void mcsUpdateSelectionHighlights();
    void mcsClearAllCursors(bool updateViewport=true);

    void mcsPaste(const QStringList& list);
    void mcsPaste(const QString& text);

    std::vector<QTextCursor> m_cursors;
    QTimer m_cursorTimer;
    bool m_drawCursorsOn = true;
    void onCursorRepaint();
    void singleCursorKeyPressEvent(QKeyEvent* e);
    
    QTextBlock blockAtPosition(int y) const;
    void updateSidebarGeometry();
    void updateSidebarArea(const QRect& rect, int dy);

    void onCursorPositionChanged();
    void onSelectionChanged();
    void onContentsChange(int position, int removed, int added);

    void ensureSelectionUnfolded(const Selection& sel);

    struct BlockData {
        QTextBlock block;
        QRect translatedRect;
    };

    enum ESType {
        ESLineHighlight = 0,
        ESSameItems = 1,
        ESCursorSelection = 2,
        ESPluginStart = 3
    };

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
	void setTabStopDistance(qreal distance);
#endif

    typedef std::vector<BlockData> BlockList;
    typedef QList<QTextEdit::ExtraSelection> ExtraSelectionList;
    typedef QMap<int /*ESType*/, ExtraSelectionList> ExtraSelectionMap;

    bool m_extraSelectionsModified = false;
    const ExtraSelectionList* getExtraSelections(int /*ESType*/ type) const;
    void setExtraSelections(int /*ESType*/ type, ExtraSelectionList list);

    // mutable int m_blockListCounter = 0;

    BlockList getBlocksInViewport() const;
    BlockList getBlocksInRect(QRect rect) const;

    void resizeEvent(QResizeEvent* event) override;
    QTextBlock findClosingBlock(const QTextBlock& startBlock) const;
    void paintLineBreaks(QPainter& painter, const BlockList& blockList) const;
    void paintSearchBlock(QPainter& painter, const QRect& eventRect, const QTextBlock& block);
    void paintLineSuffixes(QPainter& painter, const BlockList& blockList) const;
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
    int m_fontSize = 0; // Will be set by setFont(...) in constructor call
    int m_zoomLevel = 0;

    ExtraSelectionMap m_extraSelections;

    TextEditGutter* m_sideBar;
    SyntaxHighlighter* m_highlighter;

    std::vector<EditorLabelPtr> m_editorLabels;
    void highlightCurrentLine();

    static Repository* s_repository;
};

} // namespace ote

#endif // TEXTEDITOR_H
