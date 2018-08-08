#ifndef COLORLABELSPLUGIN_H
#define COLORLABELSPLUGIN_H

#include <memory>
#include "../editorlabel.h"

#include "pluginbase.h"

namespace ote {

class ColorLabel : public EditorLabel {
public:
    ColorLabel(TextEdit* te, int pos) : EditorLabel(te, pos) {}

    void setColor(QColor c);


public:
    void updatePixmap();

private:
    QColor m_color;
    QString m_displayName;
};

class ColorLabelsPlugin : public PluginBase
{
public:
    ColorLabelsPlugin(TextEdit* te);

private:
    void onCursorPositionChanged();

    WeakEditorLabelPtr m_ptr;
};

} // namespace ote

#endif // COLORLABELSPLUGIN_H
