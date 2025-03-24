QT += core network
CONFIG += console c++11
TARGET = tradestation_algo
TEMPLATE = app
SOURCES += main.cpp pricestreamer.cpp mainapp.cpp
HEADERS += pricestreamer.h mainapp.h
