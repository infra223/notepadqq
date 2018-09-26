#ifndef LATEXPLUGIN_H
#define LATEXPLUGIN_H

#include "pluginbase.h"

namespace ote {
/**
 * This plugin creates EditorLabels for all Latex text pieces it can find.
 * These pieces will be run through the JKQTMath library to create a
 * pixmap of the pretty-formatted latex formula.
 * Text in strigs will be ignored.
 */
class LatexPlugin : public PluginBase
{
public:
    LatexPlugin(TextEdit* te);

private:
    void onBlockChanged(const QTextBlock& block);
};

} // namespace

#endif // LATEXPLUGIN_H
