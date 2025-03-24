QT += core network
CONFIG += console c++11 debug
TARGET = tradestation_algo
TEMPLATE = app
SOURCES += main.cpp pricestreamer.cpp mainapp.cpp \
    mockedstockprices.cpp \
    pricefetcher.cpp \
    stock.cpp
HEADERS += pricestreamer.h mainapp.h \
    mockedstockprices.h \
    pricefetcher.h \
    stock.h
