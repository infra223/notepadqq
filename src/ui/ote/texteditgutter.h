#ifndef TEXTEDITGUTTER_H
#define TEXTEDITGUTTER_H

#include <QPlainTextEdit>
#include "textedit.h"

namespace ote {

class TextEditGutter : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditGutter(TextEdit *editor, const Config& cfg);
    QSize sizeHint() const override;

    void updateSizeHint(int lineHeight);

    void setBookmarksVisible(bool visible);
    void setNumbersVisible(bool visible);
    void setFoldingVisible(bool visible);
    void setEditsVisible(bool visible);

// signals:
//    void foldingMarkClicked(const QTextBlock& block);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent* e) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    friend class TextEdit;

    void setTheme(const Theme& theme);

    void paintBookmarkStrip(QPainter& p, const TextEdit::BlockList& bl);
    void paintNumberStrip(QPainter& p, const TextEdit::BlockList& bl);
    void paintFoldingStrip(QPainter& p, const TextEdit::BlockList& bl);
    void paintFoldingMarks(QPainter& p, const TextEdit::BlockList& bl);
    void paintEditStrip(QPainter& p, const TextEdit::BlockList& bl);

    void onBookmarkStripEnter(const QTextBlock& block);
    void onNumberStripEnter(const QTextBlock& block);
    void onFoldingStripEnter(const QTextBlock& block);

    void onBookmarkStripLeave();
    void onNumberStripLeave();
    void onFoldingStripLeave();


    TextEdit* m_textEdit;
    Theme m_theme;

    int m_currentHoverBlock = -1;
    int m_currentHoverSection = -1;

    QPixmap m_foldingMark;
    QPixmap m_bookmark;

    int m_foldingStartBlock = -1;
    int m_foldingEndBlock = -1;

    QSize m_gutterSize;
    int m_lineHeight = 0;

    struct StripInfo {
        int xOffset = 0;
        int width = 0;
        bool visible = true;
        bool isInside(qreal x) const;
    };
    StripInfo m_bookmarkStrip;
    StripInfo m_numberStrip;
    StripInfo m_foldingStrip;
    StripInfo m_editStrip;
};

} // namespace ote

#endif // TEXTEDITGUTTER_H
