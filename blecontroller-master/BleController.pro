QT       += core gui bluetooth
QT       += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11


include($$PWD/bluetooth/bluetooth.pri)

CONFIG += warn_off
CONFIG += console

msvc {
QMAKE_CFLAGS += /utf-8
QMAKE_CXXFLAGS += /utf-8
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    PythonInit.cpp \
    bluechat.cpp \
    btdevcell.cpp \
    draw.cpp \
    lsl_rx_data.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    PyThreadStateLock.h \
    bluechat.h \
    btdevcell.h \
    draw.h \
    lsl_rx_data.h \
    widget.h

FORMS += \
    bluechat.ui \
    btdevcell.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    lsl.py


win32:CONFIG(release, debug|release): LIBS += -LC:/Users/gly19/AppData/Local/Programs/Python/Python38/libs/ -lpython38
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/Users/gly19/AppData/Local/Programs/Python/Python38/libs/ -lpython38d
else:unix: LIBS += -LC:/Users/gly19/AppData/Local/Programs/Python/Python38/libs/ -lpython38

INCLUDEPATH += C:/Users/gly19/AppData/Local/Programs/Python/Python38/include
DEPENDPATH += C:/Users/gly19/AppData/Local/Programs/Python/Python38/include

