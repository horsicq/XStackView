INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xstackview.h \
    $$PWD/xstackviewoptionswidget.h

SOURCES += \
    $$PWD/xstackview.cpp \
    $$PWD/xstackviewoptionswidget.cpp

!contains(XCONFIG, xspecdebugger) {
    XCONFIG += xspecdebugger
    include($$PWD/../XSpecDebugger/xspecdebugger.pri)
}

!contains(XCONFIG, xabstracttableview) {
    XCONFIG += xabstracttableview
    include($$PWD/../Controls/xabstracttableview.pri)
}

FORMS += \
    $$PWD/xstackviewoptionswidget.ui
