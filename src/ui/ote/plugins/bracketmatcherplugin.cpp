#include "bracketmatcherplugin.h"

#include "../Highlighter/syntaxhighlighter.h"
#include "../textedit.h"

#include <QDebug>

namespace ote {

bool isLeftBracket(char c)
{
    return c == '(' || c == '{' || c == '[';
}

bool isRightBracket(char c)
{
    return c == ')' || c == '}' || c == ']';
}

bool isBracket(char c)
{
    return isLeftBracket(c) || isRightBracket(c);
}

char opposingBracket(char c)
{
    switch (c) {
    case '(':
        return ')';
    case '{':
        return '}';
    case '[':
        return ']';
    case ')':
        return '(';
    case '}':
        return '{';
    case ']':
        return '[';
    default:
        return '\0';
    }
}

struct ParenPos {
    char paren;
    int pos;
};

class BracketBlockData : public PluginBlockData {
public:
    BracketBlockData() = default;
    ~BracketBlockData() override = default;

    std::vector<ParenPos> brackets;
};

ote::BracketMatcherPlugin::BracketMatcherPlugin(ote::TextEdit* parent)
    : PluginBase(parent)
{
    initializePluginId();

    connect(parent, &TextEdit::cursorPositionChanged, this, &BracketMatcherPlugin::onCursorPositionChanged);
    connect(parent, &TextEdit::blockHighlighted, this, &BracketMatcherPlugin::onBlockChanged);
}

void ote::BracketMatcherPlugin::onCursorPositionChanged()
{
    const auto cursor = getTextEdit()->textCursor();
    const QTextBlock curBlock = cursor.block();
    BracketBlockData* data = reinterpret_cast<BracketBlockData*>(getPluginBlockData(curBlock));
    if (!data) {
        return;
    }

    if (cursor.hasSelection()) {
        clearSelections();
        return;
    }

    const std::vector<ParenPos>& parens = data->brackets;

    const int cursorPos = cursor.position();
    const int blockPos = curBlock.position();
    const int relativePos = cursorPos - blockPos;

    for (size_t i = 0; i < parens.size(); ++i) {
        const auto& pp = parens[i];

        if (pp.pos > relativePos)
            break;

        if (pp.pos != relativePos - 1 && pp.pos != relativePos)
            continue;

        const char c = pp.paren;

        int otherParenPos = -1;
        if (isLeftBracket(c))
            otherParenPos = findRightBracket(curBlock, c, opposingBracket(c), int(i + 1), 0);
        else if (isRightBracket(c))
            otherParenPos = findLeftBracket(curBlock, c, opposingBracket(c), int(parens.size() - i), 0);

        createSelections(blockPos + pp.pos, otherParenPos);

        return;
    }

    clearSelections();
}

void ote::BracketMatcherPlugin::onBlockChanged(const QTextBlock& b)
{
    BracketBlockData* data = reinterpret_cast<BracketBlockData*>(getPluginBlockData(b));
    if (!data) {
        data = new BracketBlockData();
        setPluginBlockData(b, std::unique_ptr<PluginBlockData>(data));
    } else
        data->brackets.clear();

    const auto* hl = getTextEdit()->getHighlighter();
    const auto& text = b.text();
    const auto textSize = text.size();
    const auto blockPos = b.position();

    for (int i = 0; i < textSize; ++i) {
        const char c = text.at(i).toLatin1();
        if ((isLeftBracket(c) || isRightBracket(c)) && !hl->isPositionInString(blockPos + i) &&
            !hl->isPositionInComment(blockPos + i)) {
            data->brackets.push_back({c, i});
        }
    }
}

int BracketMatcherPlugin::findRightBracket(QTextBlock b, char orig, char other, int pos, int depth) const
{
    const BracketBlockData* data = reinterpret_cast<const BracketBlockData*>(getPluginBlockData(b));
    if (!data)
        return -1;

    const std::vector<ParenPos>& parens = data->brackets;

    for (auto it = parens.begin() + pos; it != parens.end(); ++it) {
        const ParenPos& pp = *it;

        if (pp.paren == orig)
            depth++;
        else if (pp.paren == other) {
            if (depth == 0)
                return b.position() + pp.pos;
            depth--;
        }
    }

    b = b.next();

    if (b.isValid())
        return findRightBracket(b, orig, other, 0, depth);
    else
        return -1;
}

int BracketMatcherPlugin::findLeftBracket(QTextBlock b, char orig, char other, int pos, int depth) const
{
    const BracketBlockData* data = reinterpret_cast<const BracketBlockData*>(getPluginBlockData(b));
    if (!data)
        return -1;

    const std::vector<ParenPos>& parens = data->brackets;

    auto it = parens.rbegin();
    while (it != parens.rend() && pos-- > 0)
        ++it;

    for (; it != parens.rend(); ++it) {
        const ParenPos& pp = *it;

        if (pp.paren == orig)
            depth++;
        else if (pp.paren == other) {
            if (depth == 0)
                return b.position() + pp.pos;
            depth--;
        }
    }

    b = b.previous();

    if (b.isValid())
        return findLeftBracket(b, orig, other, 0, depth);
    else
        return -1;
}

void BracketMatcherPlugin::createSelections(int pos1, int pos2)
{
    if (pos1 == -1 || pos2 == -1) {
        clearSelections();
        return;
    }

    ExtraSelectionList sels;
    QTextCursor c = getTextEdit()->textCursor();

    QTextCharFormat f;
    f.setBackground(QBrush(getTextEdit()->getTheme().editorColor(Theme::BracketMatching)));

    c.setPosition(pos1);
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    sels << QTextEdit::ExtraSelection{c, f};

    c.setPosition(pos2);
    c.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    sels << QTextEdit::ExtraSelection{c, f};

    setExtraSelections(sels);
}

void BracketMatcherPlugin::clearSelections()
{
    const auto* sels = getExtraSelections();
    if (sels && !sels->isEmpty())
        setExtraSelections({});
}

} // namespace ote
