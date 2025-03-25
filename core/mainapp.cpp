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
    mainAlgo = new MainAlgo(this);
    frontend = nullptr;  // Set via setFrontend later

    // Initialize mock prices if in mock mode
    mocked_stock_prices = mockMode ? new MockedStockPrices(NASDAQ_stocks) : nullptr;

    // PriceStreamer
    // TODO This will change later because there will be more than one price streamer.
    //      They will be held and managed in some structure, created and deleted dynamically
    {
        streamer = new PriceStreamer(accessToken, "AAPL", mockMode, mocked_stock_prices);
        thread = new QThread(this); //TODO rename this variable because there are going to be lots of price streamer in the future
        thread->setObjectName("PriceStreamer");

        streamer->moveToThread(thread);
        connect(thread, &QThread::started, streamer, &PriceStreamer::start);
        connect(streamer, &PriceStreamer::priceUpdated, this, &MainApp::onPriceUpdated);

        thread->start();
    }

    // PriceFetcher
    {
        price_fetcher = new PriceFetcher(accessToken, NASDAQ_stocks, mockMode, 1 /* 1 min */, mocked_stock_prices);
        price_fetcher_thread = new QThread(this);
        price_fetcher_thread->setObjectName("PriceFetcher");

        price_fetcher->moveToThread(price_fetcher_thread);
        connect(price_fetcher_thread, &QThread::started, price_fetcher, &PriceFetcher::start);
        connect(price_fetcher, &PriceFetcher::pricesFetched, this, &MainApp::onPricesFetched);

        price_fetcher_thread->start();
    }
}

MainApp::~MainApp() {

    mainAlgo->stop();
    delete mainAlgo;

    streamer->stop();
    thread->quit();
    thread->wait();
    delete thread;

    price_fetcher->stop();
    price_fetcher_thread->quit();
    price_fetcher_thread->wait();
    delete price_fetcher_thread;

    delete streamer;
    delete price_fetcher;
    delete mocked_stock_prices;
    delete settings;
}

QString MainApp::loadAccessToken(bool mockMode) {

    QString accessToken = settings->value("TradeStation/AccessToken").toString();

    if (accessToken.isEmpty() && !mockMode) { // Allow empty token in mock mode
        qFatal() << "No access token found in config.ini. Exiting...";
    }

    return accessToken;
}

void MainApp::onPriceUpdated(const QJsonObject& priceData)
{
    if (frontend != nullptr){
        frontend->onPriceUpdated(priceData);
    }
}

void MainApp::onPricesFetched()
{
    if (frontend != nullptr){
        frontend->onPricesFetched();
    }
}

void MainApp::setFrontend(AppFrontend* frontend) {
   this->frontend = frontend;
}
