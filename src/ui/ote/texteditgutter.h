#ifndef TEXTEDITGUTTER_H
#define TEXTEDITGUTTER_H

#include <QPlainTextEdit>
#include "textedit.h"

namespace ote {

class TextEditGutter : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditGutter(TextEdit *editor);
    QSize sizeHint() const override;

    void updateSizeHint(qreal lineHeight);

signals:
    void foldingMarkClicked(const QTextBlock& block);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    friend class TextEdit;

    void paintFoldingMarks(QPainter& painter, const TextEdit::BlockList& blockList);
    void paintGutter(QPaintEvent* event, QPainter& painter, const TextEdit::BlockList& blockList);

    void leaveEvent(QEvent* e) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    TextEdit* m_textEdit;

    int m_foldingStartBlock = -1;
    int m_foldingEndBlock = -1;

    QSize m_gutterSize;
    qreal m_foldingBarWidth = 0;
};

} // namespace ote

#endif // TEXTEDITGUTTER_H
