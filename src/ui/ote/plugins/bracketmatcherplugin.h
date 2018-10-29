#ifndef BRACKETMATCHERPLUGIN_H
#define BRACKETMATCHERPLUGIN_H

#include "pluginbase.h"

namespace ote {

class TextEdit;

/**
 * This plugin tracks the use of brackets of types
 * '()', '[]', '{}' in the document and adds highlights
 * of a bracket and its matching partner when the user
 * clicks on them.
 */
class BracketMatcherPlugin : public PluginBase
{
    Q_OBJECT
public:
    BracketMatcherPlugin(TextEdit* parent);

private slots:
    void onCursorPositionChanged();
    void onBlockHighlighted(const QTextBlock& b);

private:
    // Returns the (absolute) position of a matching bracket to the left/right of the
    // starting position. 'orig' is the starting bracket. 'other' the match we're looking for.
    // Keep 'depth' at 0. It's used during recursion.
    int findRightBracket(QTextBlock b, char orig, char other, int pos, int depth=0) const;
    int findLeftBracket(QTextBlock b, char orig, char other, int pos, int depth=0) const;
    // Creates two highlights at positions pos1 and pos2, provided neither of them is -1.
    void createSelections(int pos1, int pos2);
    // Removes any bracket highlights.
    void clearSelections();
};

} // namespace ote

#endif // BRACKETMATCHERPLUGIN_H
