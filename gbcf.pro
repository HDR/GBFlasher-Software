TEMPLATE = app
TARGET = gbcflsh
DEPENDPATH += .
INCLUDEPATH += .
#DESTDIR = build

QT += widgets

QMAKE_LFLAGS += -static

RESOURCES += qdarkstyle/style.qrc

# Input
HEADERS += src/About.h \
           src/AbstractPort.h \
           src/Console.h \
           src/const.h \
           src/EraseThread.h \
           src/Gui.h \
           src/Logic.h \
           src/ReadFlashThread.h \
           src/ReadRamThread.h \
           src/Settings.h \
           src/WriteFlashThread.h \
           src/WriteRamThread.h \
           src/about.xpm \
           src/ftd2xx.h \
           src/ftdi.h \
           src/icon.xpm \
           src/termios.h
SOURCES += src/About.cpp \
           src/EraseThread.cpp \
           src/gbcflsh.cpp \
           src/Gui.cpp \
           src/Logic.cpp \
           src/ReadFlashThread.cpp \
           src/ReadRamThread.cpp \
           src/Settings.cpp \
           src/WriteFlashThread.cpp \
           src/WriteRamThread.cpp
RC_FILE = src/res.rc
win32 {
SOURCES += src/SerialPortWin.cpp \
           src/USBPortWin.cpp
HEADERS += src/SerialPortWin.h \
           src/USBPortWin.h
LIBS += -L"D:\OneDrive\Dokumenter\GitHub\GBFlasher-Software-QT5\lib" "D:\OneDrive\Dokumenter\GitHub\GBFlasher-Software-QT5\lib\ftd2xx.lib" \
}
unix {
SOURCES += src/SerialPort.cpp \
           src/USBPort.cpp
HEADERS += src/SerialPort.h \
           src/USBPort.h
LIBS += -lftdi
exec.path = /usr/bin
exec.files = gbcflsh
INSTALLS += exec config
}

DISTFILES += \
    lib/ftd2xx.lib \
    src/icon.xpm
