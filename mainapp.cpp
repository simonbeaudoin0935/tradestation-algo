#include "mainapp.h"
#include "stock.h"
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

MainApp::MainApp(const QString &configFile, const QString &nasdaqCsvFile, bool mockMode, QObject* parent) : QObject(parent) {
    settings = new QSettings(configFile, QSettings::IniFormat, this);
    QString accessToken = loadAccessToken(mockMode);
    NASDAQ_stocks = Stock::parseCsv(nasdaqCsvFile);

    // PriceStreamer
    {
        streamer = new PriceStreamer(accessToken, "AAPL", mockMode);
        thread = new QThread(this); //TODO rename this variable because there are going to be lots of price streamer in the future

        streamer->moveToThread(thread);
        connect(thread, &QThread::started, streamer, &PriceStreamer::start);
        connect(streamer, &PriceStreamer::priceUpdated, this, &MainApp::onPriceUpdated);

        thread->start();
    }

    // PriceFetcher
    {
        price_fetcher = new PriceFetcher(accessToken, NASDAQ_stocks, mockMode, 1 /* 1 min */);
        price_fetcher_thread = new QThread(this);

        price_fetcher->moveToThread(price_fetcher_thread);
        connect(price_fetcher_thread, &QThread::started, price_fetcher, &PriceFetcher::start);
        connect(price_fetcher, &PriceFetcher::pricesFetched, this, &MainApp::onPricesFetched);

        price_fetcher_thread->start();
    }
}

MainApp::~MainApp() {
    streamer->stop();
    thread->quit();
    thread->wait();
    delete streamer;
    delete thread;

    if (price_fetcher) {
        price_fetcher->stop();
        price_fetcher_thread->quit();
        price_fetcher_thread->wait();
        delete price_fetcher;
        delete price_fetcher_thread;
    }

    delete settings;
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

void MainApp::onPricesFetched()
{
    qCritical() << Q_FUNC_INFO << "TODO act on prices fetched";
}
