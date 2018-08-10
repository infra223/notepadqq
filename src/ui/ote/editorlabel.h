#ifndef EDITORLABEL_H
#define EDITORLABEL_H

#include <QObject>
#include <QTextBlock>
#include <QPainter>
#include <memory>

namespace ote {
class TextEdit;

class EditorLabel : public QObject {
    Q_OBJECT

public:

    enum AnchorPoint {
        AnchorOnLine,
        AnchorEndOfLine,
        AnchorBelowLine
    };

    EditorLabel(TextEdit* te, int pos);
    virtual ~EditorLabel() {}


    virtual void updatePixmap() = 0;

    virtual void draw(QPainter& painter, const QPointF& offset);
    bool updateDisplayRect(qreal rightBorder = -1);

    const QRectF& getDisplayRect() { return m_displayRect; }


    void setAnchorPoint(AnchorPoint p) { m_anchor = p; }
    AnchorPoint getAnchorPoint() const { return m_anchor; }


    void setHeightInLines(int lines) { m_heightInLines = std::min(lines, MAX_LINE_COUNT); }

    void setTextOverlap(bool allow) { m_overlap = allow; }


    void setChanged() { m_changed = true; }
    bool getChanged() const { return m_changed; }

    void markForDeletion() { m_markedForDeletion = true; }

    int getTypeId() const { return m_typeId; }

protected:
    QTextBlock getTextBlock() const;
    TextEdit* getTextEdit() const;

    QPixmap m_pixmap;

private:
    friend class TextEdit;
    static const int MAX_LINE_COUNT = 3;


    int m_typeId = -1;
    int m_heightInLines = MAX_LINE_COUNT;
    int m_absPos = 0;

    QRectF m_displayRect;

    AnchorPoint m_anchor;
    bool m_overlap = false;
    bool m_changed = true;
    bool m_markedForDeletion = false;
};


typedef std::shared_ptr<EditorLabel> EditorLabelPtr;
typedef std::weak_ptr<EditorLabel> WeakEditorLabelPtr;

} // namespace ote

#endif // EDITORLABEL_H
