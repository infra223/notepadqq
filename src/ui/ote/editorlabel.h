#ifndef EDITORLABEL_H
#define EDITORLABEL_H

#include <QObject>
#include <QTextBlock>
#include <QPainter>
#include <memory>

namespace ote {
class TextEdit;

/**
 * An EditorLabel is an object that can be attached to a positoin
 * inside the TextEdit's text. It will stay anchored to this positoin
 * even if the document contents change.
 * The EditorLabel can render a custom pixmap to the text field.
 * Some limitations apply. See below.
 * EditorLabels may be removed by the TextEdit if the text they're anchord
 * to is deleted. For this reason, do not retain pointers to EditorLabels!
 * Use WeakEditorLabelPtr instead.
 */
class EditorLabel : public QObject {
    Q_OBJECT

public:

    // Determines where the pixmap will be shown
    enum AnchorPoint {
        AnchorOnLine,       // Shown ontop of the anchor
        AnchorEndOfLine,    // Shown after the line containing the anchor
        AnchorBelowLine     //  Shown right below the line containing the anchor
    };

    virtual ~EditorLabel() {}
    // Will be called when the pixmap may have to be recreated. E.g. when
    // the available space for the pixmap changes.
    virtual void updatePixmap() = 0;

    // Called to draw the pixmap into the painter at a specific position.
    virtual void draw(QPainter& painter, const QPointF& offset);

    // Called by painting code to recalculate the available size for the pixmap.
    // If it returns true, the pixmap needs to be recreated.
    bool updateDisplayRect(qreal rightBorder = -1);
    const QRectF& getDisplayRect() { return m_displayRect; }

    // Sets/Gets the anchor point. See above for what that does.
    void setAnchorPoint(AnchorPoint p);
    AnchorPoint getAnchorPoint() const { return m_anchor; }

    // Returns the anchor point as the absolute position in the text.
    int getPosition() const { return m_absPos; }

    // EditorLabels are restricted in height. At most it's MAX_LINE_COUNT number of lines,
    // but it can be restricted with this function.
    void setHeightInLines(int lines) { m_heightInLines = std::min(lines, MAX_LINE_COUNT); }

    // When true, the pixmap will draw over any text. When false, the pixmap's size will be
    // restricted to empty space.
    void setTextOverlap(bool allow);

    // If set, the next paint event will fully recalc and redraw the label.
    void markForRedraw() { m_changed = true; m_wantRedraw = true; }

    // If set, the EditorLabel should be removed. Using this and TextEdit::RemoveMarkedEditorLabels
    // you can delete several labels at once which is more efficient than deletion one by one.
    void markForDeletion() { m_markedForDeletion = true; }

    // Returns an ID unique to each subclass of EditorLabel.
    int getTypeId() const { return m_typeId; }

protected:
    EditorLabel(TextEdit* parent, int pos, int typeId);

    QTextBlock getTextBlock() const;
    TextEdit* getTextEdit() const;
    static constexpr int getNewTypeId() { return __COUNTER__; }

    QPixmap m_pixmap;

private:
    friend class TextEdit;
    static const int MAX_LINE_COUNT = 3;

    const int m_typeId;
    int m_heightInLines = MAX_LINE_COUNT;
    int m_absPos;

    QRectF m_displayRect;

    AnchorPoint m_anchor;
    bool m_overlap = false;
    bool m_changed = true;          // If true, the label will be recalced on next update
    bool m_wantRedraw = false;      // If true, the label will be recalced and redrawn on next update
    bool m_markedForDeletion = false;
};

using EditorLabelPtr = std::shared_ptr<EditorLabel>;
using WeakEditorLabelPtr = std::weak_ptr<EditorLabel>;

} // namespace ote

#endif // EDITORLABEL_H
