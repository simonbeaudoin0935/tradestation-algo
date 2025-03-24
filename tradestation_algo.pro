# Base Qt modules (always included for headless)
QT += core network
CONFIG += console c++11
TARGET = tradestation_algo
TEMPLATE = app

# Common files (always included)
SOURCES += \
    core/appfrontend.cpp \
    core/main.cpp \
    core/pricestreamer.cpp \
    core/mainapp.cpp \
    core/mockedstockprices.cpp \
    core/pricefetcher.cpp \
    core/stock.cpp \
    core/terminalfrontend.cpp

HEADERS += \
    core/appfrontend.h \
    core/pricestreamer.h \
    core/mainapp.h \
    core/mockedstockprices.h \
    core/pricefetcher.h \
    core/stock.h \
    core/terminalfrontend.h

!gui {
    SOURCES += \
        core/terminalfrontend.cpp
    HEADERS += \
        core/terminalguifrontend.h
}
# GUI-specific files and module
gui {
    QT += widgets           # Adds QtWidgets (and implicitly QtGui)
    DEFINES += GUI_ENABLED  # For conditional compilation in code
    SOURCES += \
        gui/guifrontend.cpp
    HEADERS += \
        gui/guifrontend.h
    FORMS += \
    gui/guifrontend.ui

}

