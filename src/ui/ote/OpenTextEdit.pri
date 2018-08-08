#-------------------------------------------------
#
# Project created by QtCreator 2018-01-26T14:05:30
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG   += c++14 warn_on

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    $$PWD/Highlighter/abstracthighlighter.cpp \
    $$PWD/Highlighter/context.cpp \
    $$PWD/Highlighter/contextswitch.cpp \
    $$PWD/Highlighter/definition.cpp \
    $$PWD/Highlighter/foldingregion.cpp \
    $$PWD/Highlighter/format.cpp \
    $$PWD/Highlighter/htmlhighlighter.cpp \
    $$PWD/Highlighter/keywordlist.cpp \
    $$PWD/Highlighter/matchresult.cpp \
    $$PWD/Highlighter/repository.cpp \
    $$PWD/Highlighter/rule.cpp \
    $$PWD/Highlighter/state.cpp \
    $$PWD/Highlighter/syntaxhighlighter.cpp \
    $$PWD/Highlighter/theme.cpp \
    $$PWD/Highlighter/themedata.cpp \
    $$PWD/Highlighter/wildcardmatcher.cpp \
    $$PWD/textedit.cpp \
    $$PWD/texteditgutter.cpp \
    $$PWD/Highlighter/fmtrangelist.cpp \
    $$PWD/editorlabel.cpp \
    $$PWD/plugins/colorlabelsplugin.cpp \
    $$PWD/plugins/latexplugin.cpp \
    $$PWD/plugins/pluginbase.cpp \
    $$PWD/JKQTMath/jkqtmathtext.cpp \
    $$PWD/JKQTMath/jkqtphighrestimer.cpp \
    $$PWD/JKQTMath/jkqtptools.cpp

HEADERS += \
    $$PWD/Highlighter/abstracthighlighter.h \
    $$PWD/Highlighter/abstracthighlighter_p.h \
    $$PWD/Highlighter/context_p.h \
    $$PWD/Highlighter/contextswitch_p.h \
    $$PWD/Highlighter/definition.h \
    $$PWD/Highlighter/definition_p.h \
    $$PWD/Highlighter/definitionref_p.h \
    $$PWD/Highlighter/foldingregion.h \
    $$PWD/Highlighter/format.h \
    $$PWD/Highlighter/format_p.h \
    $$PWD/Highlighter/htmlhighlighter.h \
    $$PWD/Highlighter/keywordlist_p.h \
    $$PWD/Highlighter/matchresult_p.h \
    $$PWD/Highlighter/repository.h \
    $$PWD/Highlighter/repository_p.h \
    $$PWD/Highlighter/rule_p.h \
    $$PWD/Highlighter/state.h \
    $$PWD/Highlighter/state_p.h \
    $$PWD/Highlighter/syntaxhighlighter.h \
    $$PWD/Highlighter/textstyledata_p.h \
    $$PWD/Highlighter/theme.h \
    $$PWD/Highlighter/themedata_p.h \
    $$PWD/Highlighter/wildcardmatcher_p.h \
    $$PWD/Highlighter/xml_p.h \
    $$PWD/textedit.h \
    $$PWD/texteditgutter.h \
    $$PWD/Highlighter/fmtrangelist.h \
    $$PWD/editorlabel.h \
    $$PWD/plugins/colorlabelsplugin.h \
    $$PWD/plugins/latexplugin.h \
    $$PWD/plugins/pluginbase.h \
    $$PWD/JKQTMath/jkqtmathtext.h \
    $$PWD/JKQTMath/jkqtp_imexport.h \
    $$PWD/JKQTMath/jkqtphighrestimer.h \
    $$PWD/JKQTMath/jkqtptools.h
 
RESOURCES += $$PWD/JKQTMath/asana.qrc
DEFINES += AUTOLOAD_Asana_FONTS
