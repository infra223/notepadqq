QT       += core gui widgets
CONFIG   += c++14 warn_on

DEFINES += QT_DEPRECATED_WARNINGS NO_STANDARD_PATHS

SOURCES += \
    $$PWD/abstracthighlighter.cpp \
    $$PWD/context.cpp \
    $$PWD/contextswitch.cpp \
    $$PWD/definition.cpp \
    #$$PWD/definitiondownloader.cpp \
    $$PWD/fmtrangelist.cpp \
    $$PWD/foldingregion.cpp \
    $$PWD/format.cpp \
    $$PWD/htmlhighlighter.cpp \
    $$PWD/keywordlist.cpp \
    $$PWD/repository.cpp \
    $$PWD/rule.cpp \
    $$PWD/state.cpp \
    $$PWD/syntaxhighlighter.cpp \
    $$PWD/theme.cpp \
    $$PWD/themedata.cpp \
    $$PWD/wildcardmatcher.cpp

HEADERS += \
    $$PWD/abstracthighlighter.h \
    $$PWD/abstracthighlighter_p.h \
    $$PWD/context_p.h \
    $$PWD/contextswitch_p.h \
    $$PWD/definition.h \
    $$PWD/definition_p.h \
    #$$PWD/definitiondownloader.h \
    $$PWD/definitionref_p.h \
    $$PWD/fmtrangelist.h \
    $$PWD/foldingregion.h \
    $$PWD/format.h \
    $$PWD/format_p.h \
    $$PWD/htmlhighlighter.h \
    $$PWD/keywordlist_p.h \
    $$PWD/matchresult_p.h \
    $$PWD/repository.h \
    $$PWD/repository_p.h \
    $$PWD/rule_p.h \
    $$PWD/state.h \
    $$PWD/state_p.h \
    $$PWD/syntaxhighlighter.h \
    $$PWD/textstyledata_p.h \
    $$PWD/theme.h \
    $$PWD/themedata_p.h \
    $$PWD/wildcardmatcher_p.h \
    $$PWD/xml_p.h
 
