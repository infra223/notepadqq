#include "include/EditorNS/editor.h"

#include "include/notepadqq.h"
#include "include/nqqsettings.h"

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QMessageBox>
#include <QRegExp>
#include <QRegularExpression>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

namespace EditorNS
{

    QQueue<Editor*> Editor::m_editorBuffer = QQueue<Editor*>();

    Editor::Editor(QWidget* parent)
        : QWidget(parent)
        , m_textEditor(parent)
    {
        const auto& repo = ote::TextEdit::getRepository();

        QString themeName = NqqSettings::getInstance().Appearance.getColorScheme();
        auto theme = repo.theme(themeName);

        if (!theme.isValid()) {
            theme = (palette().color(QPalette::Base).lightness() < 128)
                        ? repo.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
                        : repo.defaultTheme(KSyntaxHighlighting::Repository::LightTheme);
        }

        fullConstructor(theme);
    }

    Editor::Editor(const KSyntaxHighlighting::Theme& theme, QWidget* parent)
        : QWidget(parent)
        , m_textEditor(parent)
    {
        fullConstructor(theme);
    }

    void Editor::fullConstructor(const KSyntaxHighlighting::Theme& theme) // FIXME: Should use new Theme
    {
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);
        m_layout->addWidget(&m_textEditor, 1);
        setLayout(m_layout);

        setTheme(theme);
        setLanguage(nullptr);

        connect(&m_textEditor, &ote::TextEdit::textChanged, this, &Editor::contentChanged);
        connect(&m_textEditor, &ote::TextEdit::cursorPositionChanged, this, &Editor::cursorActivity);
        connect(&m_textEditor, &ote::TextEdit::gotFocus, this, &Editor::gotFocus);

        connect(&m_textEditor, &ote::TextEdit::modificationChanged, [this](bool b) { emit cleanChanged(!b); });

        connect(&m_textEditor, &ote::TextEdit::mouseWheelUsed, this, &Editor::mouseWheel);
        connect(&m_textEditor, &ote::TextEdit::urlsDropped, this, &Editor::urlsDropped);
    }

    QSharedPointer<Editor> Editor::getNewEditor(QWidget *parent)
    {
        return QSharedPointer<Editor>(getNewEditorUnmanagedPtr(parent), &Editor::deleteLater);
    }

    Editor *Editor::getNewEditorUnmanagedPtr(QWidget *parent)
    {
        Editor *out;

        if (m_editorBuffer.length() == 0) {
            m_editorBuffer.enqueue(new Editor());
            out = new Editor();
        } else if (m_editorBuffer.length() == 1) {
            m_editorBuffer.enqueue(new Editor());
            out = m_editorBuffer.dequeue();
        } else {
            out = m_editorBuffer.dequeue();
        }

        out->setParent(parent);
        return out;
    }

    void Editor::addEditorToBuffer(const int howMany)
    {
        for (int i = 0; i < howMany; i++)
            m_editorBuffer.enqueue(new Editor());
    }

    void Editor::invalidateEditorBuffer()
    {
        m_editorBuffer.clear();
    }

    void Editor::setFocus()
    {
        m_textEditor.setFocus();
    }

    void Editor::clearFocus()
    {
        m_textEditor.clearFocus();
    }

    /**
     * Automatically converts local relative file names to absolute ones.
     */
    void Editor::setFilePath(const QUrl &filename)
    {
        QUrl old = m_filePath;
        QUrl newUrl = filename;

        if (newUrl.isLocalFile())
            newUrl = QUrl::fromLocalFile(QFileInfo(filename.toLocalFile()).absoluteFilePath());

        m_filePath = newUrl;
        emit fileNameChanged(old, newUrl);
    }

    /**
     * Always returns an absolute url.
     */
    QUrl Editor::filePath() const
    {
        return m_filePath;
    }

    QString Editor::tabName() const
    {
        return m_tabName;
    }

    void Editor::setTabName(const QString& name)
    {
        m_tabName = name;
    }

    bool Editor::isClean()
    {
<<<<<<< HEAD
        QVariant data(0); // avoid crash on Mac OS X, see issue #702
        return asyncSendMessageWithResult("C_FUN_IS_CLEAN", data).get().toBool();
=======
        return !m_textEditor.isModified();
>>>>>>> Clang syntax.
    }

    void Editor::markClean()
    {
        m_textEditor.setModified(false);
    }

    void Editor::markDirty()
    {
        m_textEditor.setModified(true);
    }

    int Editor::getHistoryGeneration()
    {
        return m_textEditor.document()->revision();
    }

    void Editor::setLanguage(KSyntaxHighlighting::Definition def)
    {
        qDebug() << "Setting language to " << def.name();

        if (!def.isValid()) {
            def = m_textEditor.getRepository().definitionForFileName(".txt");
        }

        // We use setLanguage() also when changing indentation modes, so this needs to be up here.
        if (!m_customIndentationMode)
            setIndentationMode(def);

        if (m_textEditor.getDefinition() == def)
            return;

        m_textEditor.setDefinition(def);

        // FIXME
        emit currentLanguageChanged(def.name());
    }

    void Editor::setLanguage(const QString& language)
    {
        setLanguage(m_textEditor.getRepository().definitionForName(language));
    }

    void Editor::setLanguageFromFileName(const QString& fileName)
    {
  /*auto& cache = LanguageService::getInstance();
        auto lang = cache.lookupByFileName(fileName);
        if (lang != nullptr) {
            setLanguage(lang);
            return;
        }
        lang = cache.lookupByExtension(fileName);
        if (lang != nullptr) {
            setLanguage(lang);
        }*/
        setLanguage(m_textEditor.getRepository().definitionForFileName(fileName));
    }

    void Editor::setLanguageFromFileName()
    {
        setLanguageFromFileName(filePath().toString());
        setLanguageFromFilePath(filePath().toString());
    }

    void EditorNS::Editor::detectAndSetLanguage()
    {
        // First try to find a definition for the file's extension
        // FIXME: Second try to find a definition for the file's name
        // Third try to find a definition for the content
        auto def = m_textEditor.getRepository().definitionForFileName(m_filePath.toLocalFile());
        if (def.isValid()) {
            setLanguage(def);
            return;
        }

        def = m_textEditor.getRepository().definitionForContent(value());
        if (def.isValid()) {
            setLanguage(def);
            return;
        }

        setLanguage(KSyntaxHighlighting::Definition());
    }

    void Editor::setIndentationMode(const KSyntaxHighlighting::Definition& def)
    {
        const auto& s = NqqSettings::getInstance().Languages;
        const bool useDefaults = s.getUseDefaultSettings(def.name());
        const auto& langId = useDefaults ? "default" : def.name();

        return setIndentationMode(!s.getIndentWithSpaces(langId), s.getTabSize(langId));
    }

    void Editor::setIndentationMode(const bool useTabs, const int size)
    {
        m_textEditor.setTabWidth(size);
        m_textEditor.setTabToSpaces(!useTabs);
    }

    Editor::IndentationMode Editor::indentationMode()
    {
        auto tabWidth = m_textEditor.getTabWidth();
        auto usingSpaces = m_textEditor.isTabToSpaces();
        return Editor::IndentationMode{!usingSpaces, tabWidth};
    }

    void Editor::setCustomIndentationMode(const bool useTabs, const int size)
    {
        m_customIndentationMode = true;
        setIndentationMode(useTabs, size);
    }

    void Editor::setCustomIndentationMode(const bool useTabs)
    {
        m_customIndentationMode = true;
        setIndentationMode(useTabs, 0);
    }

    void Editor::clearCustomIndentationMode()
    {
        m_customIndentationMode = false;
        // setIndentationMode(getLanguage()); // FIXME
    }

    bool Editor::isUsingCustomIndentationMode() const
    {
        return m_customIndentationMode;
    }

    void EditorNS::Editor::setZoomLevel(int level)
    {
        m_textEditor.setZoomTo(level);
    }

    void EditorNS::Editor::zoomIn()
    {
        m_textEditor.zoomIn();
    }

    void EditorNS::Editor::zoomOut()
    {
        m_textEditor.zoomOut();
    }

    int EditorNS::Editor::getZoomLevel() const
    {
        return m_textEditor.getZoomLevel();
    }

    void Editor::setSmartIndent(bool enabled)
    {
        m_textEditor.setSmartIndent(enabled);
    }

    void Editor::setValue(const QString& value)
    {        
        m_textEditor.setPlainText(value);
    }

    QString Editor::value()
    {
        return m_textEditor.toPlainText();
    }

    bool Editor::fileOnDiskChanged() const
    {
        return m_fileOnDiskChanged;
    }

    void Editor::setFileOnDiskChanged(bool fileOnDiskChanged)
    {
        m_fileOnDiskChanged = fileOnDiskChanged;
    }

    void Editor::setSelectionsText(const QStringList &texts, SelectMode mode)
    {
        /*QVariantMap data {{"text", texts}};
        switch (mode) {
            case SelectMode::After:
                data.insert("select", "after"); break;
            case SelectMode::Before:
                data.insert("select", "before"); break;
            default:
                data.insert("select", "selected"); break;
        }*/
        // FIXME - can only select one selection, also doesn't care about other select modes
        m_textEditor.setTextInSelection(texts.first(), mode == SelectMode::Selected);
    }

    void Editor::setSelectionsText(const QStringList &texts)
    {
        setSelectionsText(texts, SelectMode::After);
    }

    void Editor::insertBanner(QWidget *banner)
    {
        m_layout->insertWidget(0, banner);
    }

    void Editor::removeBanner(QWidget *banner)
    {
        if (banner != &m_textEditor && m_layout->indexOf(banner) >= 0) {
            m_layout->removeWidget(banner);
            emit bannerRemoved(banner);
        }
    }

    void Editor::removeBanner(QString objectName)
    {
        for (auto&& banner : findChildren<QWidget *>(objectName)) {
            removeBanner(banner);
        }
    }

    void Editor::setLineWrap(const bool wrap)
    {
        m_textEditor.setWordWrap(wrap);
        // m_textEditor.setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
        // asyncSendMessageWithResultP("C_CMD_SET_LINE_WRAP", wrap);
    }

    void Editor::setEOLVisible(const bool showeol)
    {
        m_textEditor.setEndOfLineMarkersVisible(showeol);
        // asyncSendMessageWithResultP("C_CMD_SHOW_END_OF_LINE", showeol);
    }

    void Editor::setWhitespaceVisible(const bool showspace)
    {
        m_textEditor.setWhitespaceVisible(showspace);
        // asyncSendMessageWithResultP("C_CMD_SHOW_WHITESPACE", showspace);
    }

    void Editor::setMathEnabled(const bool enabled)
    {
        // FIXME - Math functionality unfixable at the moment
        // asyncSendMessageWithResultP("C_CMD_ENABLE_MATH", enabled);
    }

    QPair<int, int> Editor::cursorPosition()
    {
        auto p = m_textEditor.getCursorPosition();
        return {p.line, p.column};
        // QList<QVariant> cursor = asyncSendMessageWithResult("C_FUN_GET_CURSOR").get().toList();
        // return {cursor[0].toInt(), cursor[1].toInt()};
    }

    void Editor::setCursorPosition(const int line, const int column)
    {
        m_textEditor.setCursorPosition(line, column);
        // asyncSendMessageWithResultP("C_CMD_SET_CURSOR", QList<QVariant>{line, column});
    }

    void Editor::setCursorPosition(const QPair<int, int> &position)
    {
        setCursorPosition(position.first, position.second);
    }

    void Editor::setCursorPosition(const Cursor &cursor)
    {
        setCursorPosition(cursor.line, cursor.column);
    }

    void Editor::setSelection(int fromLine, int fromCol, int toLine, int toCol)
    {
        // asyncSendMessageWithResultP("C_CMD_SET_SELECTION", QVariant(arg));
        m_textEditor.setSelection({{fromLine, fromCol}, {toLine, toCol}});
    }

    QPair<int, int> Editor::scrollPosition()
    {
        auto p = m_textEditor.getScrollPosition();
        return {p.x(), p.y()};
        // QVariantList scroll = asyncSendMessageWithResult("C_FUN_GET_SCROLL_POS").get().toList();
        // return {scroll[0].toInt(), scroll[1].toInt()};
    }

    void Editor::setScrollPosition(const int left, const int top)
    {
        m_textEditor.setScrollPosition(QPoint{left, top});
        // asyncSendMessageWithResultP("C_CMD_SET_SCROLL_POS", QVariantList{left, top});
    }

    void Editor::setScrollPosition(const QPair<int, int> &position)
    {
        setScrollPosition(position.first, position.second);
    }

    QString Editor::endOfLineSequence() const
    {
        return m_endOfLineSequence;
    }

    void Editor::setEndOfLineSequence(const QString &newLineSequence)
    {
        m_endOfLineSequence = newLineSequence;
    }

    void Editor::setFont(const QFont& font)
    {
        m_textEditor.setFont(font);
    }

    void Editor::setFont(QString fontFamily, int fontSize)
    {
        QFont f = font();

        if (!fontFamily.isEmpty())
            f.setFamily(fontFamily);
        else
            f.setFamily("Monospace");

        if (fontSize > 0)
            f.setPointSize(fontSize);

        m_textEditor.setFont(f);
    }

    QTextCodec *Editor::codec() const
    {
        return m_codec;
    }

    void Editor::setCodec(QTextCodec *codec)
    {
        m_codec = codec;
    }

    bool Editor::bom() const
    {
        return m_bom;
    }

    void Editor::setBom(bool bom)
    {
        m_bom = bom;
    }

    void Editor::setTheme(const KSyntaxHighlighting::Theme& theme)
    {
        qDebug() << "Setting theme to " << theme.name();
        m_textEditor.setTheme(theme);
    }

    void Editor::setTheme(const QString& themeName)
    {
        const auto& theme = ote::TextEdit::getRepository().theme(themeName);
        m_textEditor.setTheme(theme);
    }

    QList<Editor::Selection> Editor::selections()
    {
        // FIXME returns only one selection
        const auto& pos = m_textEditor.getSelection();
        return QList<Selection>() << Selection{{pos.start.line, pos.start.column}, {pos.end.line, pos.end.column}};
    }

    QStringList Editor::selectedTexts()
    {
        // FIXME
        return QStringList() << m_textEditor.getSelectedText();

        /*return asyncSendMessageWithResultP("C_FUN_GET_SELECTIONS_TEXT")
                .then([](QVariant text){ return text.toStringList(); });*/
    }

    void Editor::setOverwrite(bool overwrite)
    {
        m_textEditor.setOverwriteMode(overwrite);
    }

    std::pair<Editor::IndentationMode, bool> Editor::detectDocumentIndentation()
    {
        // This is basically a copy from App.js DETECT_INDENTATION function.
        QRegularExpression reg("([ ]{2,}|[\t]+)[^ \t]+?");
        QStringList lines = m_textEditor.toPlainText().split('\n', QString::SkipEmptyParts);
        int maxLines = 50;

        for (const auto& line : lines) {
            const auto it = reg.match(line);

            if (maxLines-- <= 0)
                break;

            if (it.hasMatch()) {
                const auto match = it.captured(1);

                if (match[0] == '\t')
                    return std::make_pair(Editor::IndentationMode{true, 0}, true);
                else {
                    int length = match.length();
                    if (length == 2 || length == 4 || length == 8)
                        return std::make_pair(Editor::IndentationMode{false, length}, true);
                    else
                        return std::make_pair(Editor::IndentationMode{false, 0}, false);
                }
            }
        }

        return std::make_pair(Editor::IndentationMode{0, 0}, false);
    }

    void Editor::print(std::shared_ptr<QPrinter> printer)
    {
        // 1. Set theme to default because dark themes would force the printer to color the entire
        //    document in the background color. Default theme has white background.
        // 2. Set WebView's bg-color to white to prevent visual artifacts when printing less than one page.
        // 3. Set C_CMD_DISPLAY_PRINT_STYLE to hide UI elements like the gutter.

#if QT_VERSION >= QT_VERSION_CHECK(5,8,0)
        QColor prevBackgroundColor = m_webView->page()->backgroundColor();
        QString prevStylesheet = m_webView->styleSheet();

        this->setLineWrap(true);
        setTheme(themeFromName("default"));
        m_webView->page()->setBackgroundColor(Qt::transparent);
        m_webView->setStyleSheet("background-color: white");
        sendMessage("C_CMD_DISPLAY_PRINT_STYLE");
        m_webView->page()->print(printer.get(), [=](bool /*success*/) {
            // Note: it is important to capture "printer" in order to keep the shared_ptr alive.
            sendMessage("C_CMD_DISPLAY_NORMAL_STYLE");
            m_webView->setStyleSheet(prevStylesheet);
            m_webView->page()->setBackgroundColor(prevBackgroundColor);
            setTheme(themeFromName(NqqSettings::getInstance().Appearance.getColorScheme()));
            this->setLineWrap(NqqSettings::getInstance().General.getWordWrap());
        });
#endif
    }

    QPromise<QByteArray> Editor::printToPdf(const QPageLayout& pageLayout)
    {
        // 1. Set theme to default because dark themes would force the printer to color the entire
        //    document in the background color. Default theme has white background.
        // 2. Set WebView's bg-color to white to prevent visual artifacts when printing less than one page.
        // 3. Set C_CMD_DISPLAY_PRINT_STYLE to hide UI elements like the gutter.

        return QPromise<QByteArray>(
            [&](const QPromiseResolve<QByteArray>& resolve, const QPromiseReject<QByteArray>& reject) {

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
                QColor prevBackgroundColor = m_webView->page()->backgroundColor();
                QString prevStylesheet = m_webView->styleSheet();

                this->setLineWrap(true);
                setTheme(themeFromName("default"));
                m_webView->page()->setBackgroundColor(Qt::transparent);
                m_webView->setStyleSheet("background-color: white");
                asyncSendMessageWithResultP("C_CMD_DISPLAY_PRINT_STYLE").wait();

                m_webView->page()->printToPdf(
                    [=](const QByteArray& data) {
                        QTimer::singleShot(0, [=]() {
                            asyncSendMessageWithResultP("C_CMD_DISPLAY_NORMAL_STYLE").wait();
                            m_webView->setStyleSheet(prevStylesheet);
                            m_webView->page()->setBackgroundColor(prevBackgroundColor);
                            setTheme(themeFromName(NqqSettings::getInstance().Appearance.getColorScheme()));
                            this->setLineWrap(NqqSettings::getInstance().General.getWordWrap());
                        });

                        if (data.isEmpty() || data.isNull()) {
                            reject(QByteArray());
                        } else {
                            resolve(data);
                        }
                    },
                    pageLayout);

#else
                reject(QByteArray());
#endif
            });
    }

    QString Editor::getCurrentWord()
    {
        return m_textEditor.getCurrentWord();
        // return asyncSendMessageWithResultP("C_FUN_GET_CURRENT_WORD")
        //        .then([](QVariant v){ return v.toString(); });
    }

    int Editor::characterCount() const
    {
        return m_textEditor.getCharCount();
    }

    int Editor::lineCount()
    {
        return m_textEditor.getLineCount();
        // return asyncSendMessageWithResultP("C_FUN_GET_LINE_COUNT")
        //        .then([](QVariant v){ return v.toInt(); });
    }
}
