#include "include/frmabout.h"

#include "include/iconprovider.h"
#include "include/notepadqq.h"
#include "ui_frmabout.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>

frmAbout::frmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    auto size = std::max(ui->lblIcon->width(), ui->lblIcon->height());
    ui->lblIcon->setPixmap(IconProvider::fromTheme("notepadqq").pixmap(size, size));

    auto makeLink = [](const QString& txt, const QString& link) {
        const QString linkStyle = "text-decoration: none; color:#606060;";
        return QString("<a href=\"%1\"><span style=\"" + linkStyle + "\">%2</span></a>").arg(link).arg(txt);
    };

    QString text;
    text += "<h1>Notepadqq</h1>";
    text += "<p><b>v" + QApplication::applicationVersion() + "</b></p>";
    text += Notepadqq::copyright();
    text += "<p>" + tr("This program makes use of the following software libraries:");
    text +=
        "<ul><li>" + makeLink("KSyntaxHighlighting, KDE e.V.", "https://github.com/KDE/syntax-highlighting") + "</li>";
    text += "<li>" + makeLink("JKQTPlotter, Jan W. Krieger", "https://github.com/jkriege2/JKQtPlotter") + "</li>";
    text += "</ul></p>";
    text += tr("Contributors:") + " " + makeLink(tr("GitHub Contributors"), Notepadqq::contributorsUrl) + "<br/>";
    text += tr("Website:") + " " + makeLink(Notepadqq::website, Notepadqq::website);

    ui->lblText->setText(text);

    setFixedSize(this->width(), this->height());
    setWindowFlags( (windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMaximizeButtonHint);
}

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::on_pushButton_clicked()
{
    this->close();
}

void frmAbout::on_btnLicense_clicked()
{
    QMessageBox license;
    license.setText(R"DELIM(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
                    <html><head><style type="text/css">
                    p, li { white-space: pre-wrap; }
                    </style></head><body>
                    <p>This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.</p>
                    <p>This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</p>
                    <p>You should have received a copy of the GNU General Public License along with this program. If not, see &lt;http://www.gnu.org/licenses/&gt;.</p>
                    </body></html>)DELIM");
    license.exec();
}
