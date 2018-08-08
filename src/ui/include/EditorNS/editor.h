#ifndef EDITOR_H
#define EDITOR_H

#include <QObject>
#include <QQueue>
#include <QTextCodec>
#include <QVBoxLayout>
#include <QVariant>
#include <QWheelEvent>
#include <QUrl>
#include <QPrinter>

#include <ote/textedit.h>


class EditorTabWidget;

namespace EditorNS
{
    /**
         * @brief Provides a JavaScript CodeMirror instance.
         *
         * Communication works by sending messages to the javascript Editor using
         * the sendMessage() method. On the other side, when a javascript event
         * occurs, the messageReceived() signal will be emitted.
         *
         * In addition to messageReceived(), other signals could be emitted at the
         * same time, for example currentLineChanged(). This is simply for
         * convenience, so that the user of this class doesn't need to manually parse
         * the arguments for pre-defined messages.
         *
         */
    class Editor : public QWidget
    {
        Q_OBJECT
    public:

        explicit Editor(const ote::Theme& theme, QWidget *parent = 0);
        explicit Editor(QWidget *parent = 0);

        /**
             * @brief Efficiently returns a new Editor object from an internal buffer.
             * @return
             */
        static QSharedPointer<Editor> getNewEditor(QWidget *parent = 0);
        static Editor *getNewEditorUnmanagedPtr(QWidget *parent);

        static void invalidateEditorBuffer();

        struct Cursor {
            int line;
            int column;

            bool operator == (const Cursor &x) const {
                return line == x.line && column == x.column;
            }

            bool operator < (const Cursor &x) const {
                return std::tie(line, column) < std::tie(x.line, x.column);
            }

            bool operator <= (const Cursor &x) const {
                return *this == x || *this < x;
            }

            bool operator > (const Cursor &x) const {
                return !(*this <= x);
            }

            bool operator >= (const Cursor &x) const {
                return !(*this < x);
            }
        };

        struct Selection {
            Cursor from;
            Cursor to;
        };

        struct IndentationMode {
            bool useTabs;
            int size;
        };

        /**
             * @brief Adds a new Editor to the internal buffer used by getNewEditor().
             *        You might want to call this method e.g. as soon as the application
             *        starts (so that an Editor is ready as soon as it gets required),
             *        or when the application is idle.
             * @param howMany specifies how many Editors to add
             * @return
             */
        static void addEditorToBuffer(const int howMany = 1);

        /**
             * @brief Give focus to the editor, so that the user can start
             *        typing. Note that calling won't automatically switch to
             *        the tab where the editor is. Use EditorTabWidget::setCurrentIndex()
             *        and TopEditorContainer::setFocus() for that.
             */
        Q_INVOKABLE void setFocus();

        /**
             * @brief Remove the focus from the editor.
             *
             * @param widgetOnly only clear the focus on the actual widget
             */
        Q_INVOKABLE void clearFocus();

        /**
             * @brief Set the file name associated with this editor
             * @param filename full path of the file
             */
        Q_INVOKABLE void setFilePath(const QUrl &filename);

        /**
             * @brief Get the file name associated with this editor
             * @return
             */
        Q_INVOKABLE QUrl filePath() const;

        Q_INVOKABLE bool fileOnDiskChanged() const;
        Q_INVOKABLE void setFileOnDiskChanged(bool fileOnDiskChanged);

        enum class SelectMode {
            Before,
            After,
            Selected
        };

        void insertBanner(QWidget *banner);
        void removeBanner(QWidget *banner);
        void removeBanner(QString objectName);

        // Lower-level message wrappers:
        Q_INVOKABLE bool isClean();
        Q_INVOKABLE void markClean();
        Q_INVOKABLE void markDirty();

        /**
         * @brief Returns an integer that denotes the editor's history state. Making changes to
         *        the contents increments the integer while reverting changes decrements it again.
         */
        Q_INVOKABLE int getHistoryGeneration();

        /**
         * @brief Set the language to use for the editor.
         *        It automatically adjusts tab settings from
         *        the default configuration for the specified language.
         * @param language Language id
         */
        Q_INVOKABLE void setLanguage(ote::Definition def);
        Q_INVOKABLE void setLanguage(const QString &language);
        Q_INVOKABLE void setLanguageFromFileName(const QString& filePath);
        Q_INVOKABLE void setLanguageFromFileName();
        void detectAndSetLanguage();
        Q_INVOKABLE void setValue(const QString &value);
        Q_INVOKABLE QString value();

        /**
         * @brief Set custom indentation settings which may be different
         *        from the default tab settings associated with the current
         *        language.
         *        If this method is called, further calls to setLanguage()
         *        will NOT modify these tab settings. Use
         *        clearCustomIndentationMode() to reset to default settings.
         * @param useTabs
         * @param size Size of an indentation. If 0, keeps the current one.
         */
        void setCustomIndentationMode(const bool useTabs, const int size);
        void setCustomIndentationMode(const bool useTabs);
        void clearCustomIndentationMode();
        bool isUsingCustomIndentationMode() const;

        int getZoomLevel() const;
        void zoomOut();
        void zoomIn();
        void setZoomLevel(int level);

        Q_INVOKABLE void setSmartIndent(bool enabled);
        Q_INVOKABLE void setSelectionsText(const QStringList &texts, SelectMode mode);
        Q_INVOKABLE void setSelectionsText(const QStringList &texts);
        ote::Definition getLanguage() { return m_textEditor.getDefinition(); }
        Q_INVOKABLE void setLineWrap(const bool wrap);
        Q_INVOKABLE void setEOLVisible(const bool showeol);
        Q_INVOKABLE void setWhitespaceVisible(const bool showspace);
        //Q_INVOKABLE void setMathEnabled(const bool enabled);

        void setFont(const QFont& font);

        /**
         * @brief Get the current cursor position
         * @return a <line, column> pair.
         */
        QPair<int, int> cursorPosition();
        QPair<int, int> cursorPositionP();
        void setCursorPosition(const int line, const int column);
        void setCursorPosition(const QPair<int, int> &position);
        void setCursorPosition(const Cursor &cursor);

        /**
         * @brief Get the current scroll position
         * @return a <left, top> pair.
         */
        QPair<int, int> scrollPosition();
        void setScrollPosition(const int left, const int top);
        void setScrollPosition(const QPair<int, int> &position);
        QString endOfLineSequence() const;
        void setEndOfLineSequence(const QString &endOfLineSequence);

        QTextCodec *codec() const;

        /**
         * @brief Set the codec for this Editor.
         *        This method does not change the in-memory or on-screen
         *        representation of the document (which is always Unicode).
         *        It serves solely as a way to keep track of the encoding
         *        that needs to be used when the document gets saved.
         * @param codec
         */
        void setCodec(QTextCodec *codec);

        bool bom() const;
        void setBom(bool bom);

        void setTheme(const ote::Theme& theme);
        void setTheme(const QString& themeName);

        ote::TextEdit& textEditor() { return m_textEditor; }

        QList<Selection> selections();

        /**
         * @brief Returns the currently selected texts.
         * @return
         */
        Q_INVOKABLE QStringList selectedTexts();

        void setOverwrite(bool overwrite);

        /**
         * @brief Detect the indentation mode used within the current document.
         * @return a pair whose first element is the document indentation, that is
         *         significative only if the second element ("found") is true.
         */
        std::pair<IndentationMode, bool> detectDocumentIndentation();
        Editor::IndentationMode indentationMode();

        QString getCurrentWord();

        void setSelection(int fromLine, int fromCol, int toLine, int toCol);

        int lineCount();

        int characterCount() const;

    private:
        friend class ::EditorTabWidget;

        // These functions should only be used by EditorTabWidget to manage the tab's title. This works around
        // KDE's habit to automatically modify QTabWidget's tab titles to insert shortcut sequences (like &1).
        QString tabName() const;
        void setTabName(const QString& name);

        static QQueue<Editor*> m_editorBuffer;
        QVBoxLayout *m_layout;

        ote::TextEdit m_textEditor;


        QUrl m_filePath = QUrl();
        QString m_tabName;
        bool m_fileOnDiskChanged = false;
        bool m_loaded = false;
        QString m_endOfLineSequence = "\n";
        QTextCodec *m_codec = QTextCodec::codecForName("UTF-8");
        bool m_bom = false;
        bool m_customIndentationMode = false;

        void fullConstructor(const ote::Theme& theme);

        void setIndentationMode(const bool useTabs, const int size);
        void setIndentationMode(const ote::Definition& def);

    signals:
        void messageReceived(QString msg, QVariant data);
        void asyncReplyReceived(unsigned int id, QString msg, QVariant data);
        void gotFocus();
        void mouseWheel(QWheelEvent *ev);
        void urlsDropped(QList<QUrl> urls);
        void bannerRemoved(QWidget *banner);

        // Pre-interpreted messages:
        void contentChanged();
        void cursorActivity();
        void cleanChanged(bool isClean);
        void fileNameChanged(const QUrl &oldFileName, const QUrl &newFileName);

        /**
             * @brief The editor finished loading. There should be
             *        no need to use this signal outside this class.
             */
        void editorReady();

        void currentLanguageChanged(QString name);

    public slots:
        //void sendMessage(const QString &msg, const QVariant &data);
        //void sendMessage(const QString &msg);

        /**
         * @brief asyncSendMessageWithResult
         * @param msg
         * @param data
         * @param callback When set, the result is returned asynchronously via the provided function.
         *                 If set, you should NOT use the return value of this method.
         * @return
         */
        //QVariant asyncSendMessageWithResultP(const QString &msg, const QVariant &data);
        //QVariant asyncSendMessageWithResultP(const QString &msg);

        /*std::shared_future<QVariant> asyncSendMessageWithResult(const QString &msg, const QVariant &data, std::function<void(QVariant)> callback = 0);
        std::shared_future<QVariant> asyncSendMessageWithResult(const QString &msg, std::function<void(QVariant)> callback = 0);*/

        /**
         * @brief Print the editor. As of Qt 5.11, it produces low-quality, non-vector graphics with big dimension.
         * @param printer
         */
        void print(std::shared_ptr<QPrinter> printer);

        /**
         * @brief Returns the content of the editor layed out in a pdf file that can be directly saved to disk.
         *        This method produces light, vector graphics.
         * @param pageLayout
         * @return
         */
        QPromise<QByteArray> printToPdf(const QPageLayout &pageLayout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF()));
    };

}

#endif // EDITOR_H
