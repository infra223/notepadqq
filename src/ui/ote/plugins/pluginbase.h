#ifndef PLUGINBASE_H
#define PLUGINBASE_H

#include <QObject>

#include "../editorlabel.h"

namespace ote {

class TextEdit;

class PluginBase : public QObject
{
    Q_OBJECT

public:
    PluginBase(TextEdit* parent);

protected:
    TextEdit* getTextEdit();
};

} // namespace ote

#endif // PLUGINBASE_H
