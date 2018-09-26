#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <QObject>

#include "../editorlabel.h"
#include "../textedit.h"

namespace ote {

class TextEdit;
class PluginBlockData;

/**
 * When subclassing PluginBase, be sure to use the O_OBJECT macro and call initializePluginId()
 * in the constructor. It provides the class with a unique typeId which allows it to interact
 * with the TextEdit and SyntaxHighlighter classes.
 * You should only ever attach one instance of a Plugin to a TextEdit.
 *
 */

class PluginBase : public QObject
{
    Q_OBJECT

public:
    PluginBase(TextEdit* parent);

    // Returns a unique integer >0 for the type of plugin.
    // This value isn't guaranteed to be stable across multiple runs.
    int getTypeId() const;

protected:
    typedef QList<QTextEdit::ExtraSelection> ExtraSelectionList;

    // Gets/Sets this plugin's extraSelections added to the TextEdit.
    void setExtraSelections(const ExtraSelectionList& list);
    const ExtraSelectionList* getExtraSelections() const;

    // Returns the TextEdit this Plugin is attached to
    TextEdit* getTextEdit();
    const TextEdit* getTextEdit() const;

    // Gets/Sets this Plugin's data for a specific block. This can be used to save information
    // Inside a block for later use. Subclass PluginBlockData if you want to do that.
    // That data will be deleted when the text block is deleted.
    PluginBlockData* getPluginBlockData(const QTextBlock& block);
    const PluginBlockData* getPluginBlockData(const QTextBlock& block) const;
    void setPluginBlockData(const QTextBlock& block, std::unique_ptr<PluginBlockData> data);

    // Call this function in the constructor of all Plugin-subclasses. Otherwise many of the
    // functions above will fail, and you'll be told about it. Very clearly. With asserts.
    void initializePluginId();

private:
    int m_typeId = 0;
};

} // namespace ote

#endif // PLUGINBASE_H
