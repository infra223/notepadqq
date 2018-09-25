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
 *
 */

class PluginBase : public QObject
{
    Q_OBJECT

public:
    PluginBase(TextEdit* parent);

    int getTypeId() const;

protected:
    typedef QList<QTextEdit::ExtraSelection> ExtraSelectionList;
    void setExtraSelections(const ExtraSelectionList& list);
    const ExtraSelectionList* getExtraSelections() const;

    TextEdit* getTextEdit();
    const TextEdit* getTextEdit() const;

    PluginBlockData* getPluginBlockData(const QTextBlock& block);
    const PluginBlockData* getPluginBlockData(const QTextBlock& block) const;
    void setPluginBlockData(const QTextBlock& block, std::unique_ptr<PluginBlockData> data);

    void initializePluginId();

private:
    int m_typeId = 0;
};

} // namespace ote

#endif // PLUGINBASE_H
