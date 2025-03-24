# Base Qt modules (always included for headless)
QT += core network
CONFIG += console c++11
TARGET = tradestation_algo
TEMPLATE = app

# Common files (always included)
SOURCES += \
    core/main.cpp \
    core/pricestreamer.cpp \
    core/mainapp.cpp \
    core/mockedstockprices.cpp \
    core/pricefetcher.cpp \
    core/stock.cpp \

HEADERS += \
    core/pricestreamer.h \
    core/mainapp.h \
    core/mockedstockprices.h \
    core/pricefetcher.h \
    core/stock.h \

# GUI-specific files and module
gui {
    QT += widgets           # Adds QtWidgets (and implicitly QtGui)
    DEFINES += GUI_ENABLED  # For conditional compilation in code
    SOURCES += \
        gui/guifrontend.cpp
    HEADERS += \
        gui/guifrontend.h
    FORMS += \
        gui/mainwindow.ui

}

