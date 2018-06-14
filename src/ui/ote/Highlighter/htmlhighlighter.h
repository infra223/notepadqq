/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>
    Modified 2018 Julian Bansen <https://github.com/JuBan1>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KSYNTAXHIGHLIGHTING_HTMLHIGHLIGHTER_H
#define KSYNTAXHIGHLIGHTING_HTMLHIGHLIGHTER_H


#include "abstracthighlighter.h"

#include <QString>

#include <memory>

class QFile;
class QTextStream;

namespace KSyntaxHighlighting {

class HtmlHighlighterPrivate;

class  HtmlHighlighter : public AbstractHighlighter
{
public:
    HtmlHighlighter();
    ~HtmlHighlighter() override;

    void highlightFile(const QString &fileName);

    void setOutputFile(const QString &fileName);
    void setOutputFile(FILE *fileHandle);

protected:
    void applyFormat(int offset, int length, const Format &format) override;

private:
    std::unique_ptr<HtmlHighlighterPrivate> d;
};
}

#endif // KSYNTAXHIGHLIGHTING_HTMLHIGHLIGHTER_H
