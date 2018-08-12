#ifndef LATEXPLUGIN_H
#define LATEXPLUGIN_H

#include "pluginbase.h"
#include "../JKQTMath/jkqtmathtext.h"

namespace ote {

class TeXLabel : public EditorLabel {
public:
    TeXLabel(TextEdit* te, int pos);
    virtual ~TeXLabel() override;

    void updatePixmap() override;

    void setLatexString(const QString& text);

    static const int TYPE_ID = getNewTypeId();

private:
    bool m_pixmapIsSquished = false;
    JKQTmathText jkMath;
    QString mathFormula;
};

class LatexPlugin : public PluginBase
{
public:
    LatexPlugin(TextEdit* te);

private:
    void onBlockChanged(const QTextBlock& block);
};

} // namespace

#endif // LATEXPLUGIN_H
