#include "pluginbase.h"

#include "../textedit.h"

namespace ote {

PluginBase::PluginBase(TextEdit* parent)
    : QObject(parent)
{
}

TextEdit* PluginBase::getTextEdit()
{
    return dynamic_cast<TextEdit*>(parent());
}

} // namespace ote
