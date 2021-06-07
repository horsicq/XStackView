INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/xstackview.h

SOURCES += \
    $$PWD/xstackview.cpp

!contains(XCONFIG, xspecdebugger) {
    XCONFIG += xspecdebugger
    include($$PWD/../XSpecDebugger/xspecdebugger.pri)
}

!contains(XCONFIG, xabstracttableview) {
    XCONFIG += xabstracttableview
    include($$PWD/../Controls/xabstracttableview.pri)
}
