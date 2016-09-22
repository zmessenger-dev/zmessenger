TEMPLATE = app
TARGET = zmessenger
INCLUDEPATH += .

# Input
HEADERS += \
    zdefines.h \
    zpersondb.h \
\
    zcache.h \
    zchannel.h \
    zmainwindow.h \
    zparser.h \
    zperson.h \
    zsettings.h \
    widgets/zblockerdialog.h \
    widgets/zchanneldialog.h \
    widgets/zlogindialog.h \
    widgets/zchatinput.h \

SOURCES += \
    main.cpp \
\
    zcache.cpp \
    zchannel.cpp \
    zmainwindow.cpp \
    zparser.cpp \
    zperson.cpp \
    zsettings.cpp \
    widgets/zblockerdialog.cpp \
    widgets/zchanneldialog.cpp \
    widgets/zlogindialog.cpp \
    widgets/zchatinput.cpp \

RESOURCES = zmessenger.qrc

QT += core gui multimedia network websockets widgets
