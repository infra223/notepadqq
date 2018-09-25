#include "pluginbase.h"

#include "../Highlighter/syntaxhighlighter.h"

#include <vector>

namespace ote {

PluginBase::PluginBase(TextEdit* parent)
    : QObject(parent)
{
}

TextEdit* PluginBase::getTextEdit()
{
    return dynamic_cast<TextEdit*>(parent());
}

const TextEdit* PluginBase::getTextEdit() const
{
    return dynamic_cast<TextEdit*>(parent());
}

PluginBlockData* PluginBase::getPluginBlockData(const QTextBlock& block)
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    return getTextEdit()->getHighlighter()->getPluginBlockData(block, m_typeId);
}

const PluginBlockData* PluginBase::getPluginBlockData(const QTextBlock& block) const
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    return getTextEdit()->getHighlighter()->getPluginBlockData(block, m_typeId);
}

void PluginBase::setPluginBlockData(const QTextBlock& block, std::unique_ptr<PluginBlockData> data)
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    getTextEdit()->getHighlighter()->setPluginBlockData(block, m_typeId, std::move(data));
}

void PluginBase::initializePluginId()
{
    static std::vector<const char*> typeNames;
    const char* name = metaObject()->className();

    auto it = std::find_if(typeNames.begin(), typeNames.end(), [name](const char* a) { return strcmp(a, name) == 0; });

    if (it == typeNames.end()) {
        typeNames.push_back(name);
        m_typeId = int(typeNames.size());
    } else {
        m_typeId = int(std::distance(typeNames.begin(), it)) + 1;
    }
}

int PluginBase::getTypeId() const
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    return m_typeId;
}

void PluginBase::setExtraSelections(const PluginBase::ExtraSelectionList& list)
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    getTextEdit()->setExtraSelections(TextEdit::ESPluginStart + m_typeId, list);
}

const PluginBase::ExtraSelectionList* PluginBase::getExtraSelections() const
{
    Q_ASSERT_X(m_typeId != 0, "PluginBase", "m_typeId shouldn't be 0. Did you use Q_OBJECT and initializePluginId()?");
    return getTextEdit()->getExtraSelections(TextEdit::ESPluginStart + m_typeId);
}

} // namespace ote
