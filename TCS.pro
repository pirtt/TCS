#-------------------------------------------------
#
# Project created by QtCreator 2021-07-10T21:54:09
#
#-------------------------------------------------

QT       += core gui network charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
RC_ICONS = skin/tcs.ico

TARGET = TCS
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
    TCSWin.cpp \
    ChatTask.cpp \
    MsrpChatTask.cpp \
    HttpChatTask.cpp \
    ChatMsg.cpp

HEADERS += \
    TCSWin.h \
    ChatTask.h \
    MsrpChatTask.h \
    HttpChatTask.h \
    ChatMsg.h \
    Log.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    msg.ui \
    chat.ui

RESOURCES += \
    skin/tcs.qrc
