#ifndef COLORLABELSPLUGIN_H
#define COLORLABELSPLUGIN_H

#include <memory>
#include "../editorlabel.h"

#include "pluginbase.h"

namespace ote {

/**
 * This plugin looks at the word under the current cursor and,
 * if this word is a color code (such as #AABBCC) it'll show
 * a small square to the right of it with the color in question.
 */
class ColorLabelsPlugin : public PluginBase
{
public:
    ColorLabelsPlugin(TextEdit* te);

private:
    void onCursorPositionChanged();

    // Contains the current EditorLabel that is used to show the
    // color label.
    WeakEditorLabelPtr m_ptr;
};

} // namespace ote

#endif // COLORLABELSPLUGIN_H
