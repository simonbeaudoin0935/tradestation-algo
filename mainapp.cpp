#include "mainapp.h"
#include "stock/stock.h"
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

MainApp::MainApp(const QString &configFile, const QString &nasdaqCsvFile, bool mockMode, QObject* parent) : QObject(parent) {
    settings = new QSettings(configFile, QSettings::IniFormat, this);
    QString accessToken = loadAccessToken(mockMode);
    QList<Stock> NASDAQ_stocks = Stock::parseCsv(nasdaqCsvFile);


    //----------------------------------------------------------------------------------------------------------------
    // PriceStreamer
    {
        streamer = new PriceStreamer(accessToken, "AAPL", mockMode);
        thread = new QThread(this);

        streamer->moveToThread(thread);
        connect(thread, &QThread::started, streamer, &PriceStreamer::start);
        connect(streamer, &PriceStreamer::priceUpdated, this, &MainApp::onPriceUpdated);

        thread->start();
    }

    // PriceFetcher
    {
        price_fetcher = new PriceFetcher(accessToken, NASDAQ_stocks, mockMode, 1 /* 1 min */);
        pricefetcher_thread = new QThread(this);

        price_fetcher->moveToThread(pricefetcher_thread);

        //TODO take care of the connections

        thread->start();
    }
}

MainApp::~MainApp() {
    streamer->stop();
    thread->quit();
    thread->wait();

    //TODO clean up after pricefetcher just like price streamer
    delete streamer;
}

QString MainApp::loadAccessToken(bool mockMode) {

    QString accessToken = settings->value("TradeStation/AccessToken").toString();

    if (accessToken.isEmpty() && !mockMode) { // Allow empty token in mock mode
        qFatal() << "No access token found in config.ini. Exiting...";
    }

    return accessToken;
}

void MainApp::onPriceUpdated(const QJsonObject& priceData) {
    qDebug() << "Main Thread - Price Updated:" << priceData;
}
