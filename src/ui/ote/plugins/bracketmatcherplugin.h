#ifndef BRACKETMATCHERPLUGIN_H
#define BRACKETMATCHERPLUGIN_H

#include "pluginbase.h"

namespace ote {

class TextEdit;

class BracketMatcherPlugin : public PluginBase
{
    Q_OBJECT
public:
    BracketMatcherPlugin(TextEdit* parent);

private slots:
    void onCursorPositionChanged();
    void onBlockChanged(const QTextBlock& b);

private:
    int findRightBracket(QTextBlock b, char orig, char other, int pos, int depth) const;
    int findLeftBracket(QTextBlock b, char orig, char other, int pos, int depth) const;
    void createSelections(int pos1, int pos2);
    void clearSelections();
};

} // namespace ote

#endif // BRACKETMATCHERPLUGIN_H
