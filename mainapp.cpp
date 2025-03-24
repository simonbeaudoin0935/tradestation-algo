#include "mainapp.h"
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QCoreApplication>
#include <QFile>

MainApp::MainApp(const QString &configFile, const QString &nasdaqCsvFile, bool mockMode, QObject* parent) : QObject(parent) {
    settings = new QSettings(configFile, QSettings::IniFormat, this);
    QString accessToken = loadAccessToken();

    if (accessToken.isEmpty() && !mockMode) { // Allow empty token in mock mode
        qWarning() << "No access token found in config.ini. Exiting...";
        QCoreApplication::quit();
        return;
    }

    QFile file(nasdaqCsvFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: Could not open NASDAQ CSV file:" << nasdaqCsvFile;
        return;
    }


    //----------------------------------------------------------------------------------------------------------------

    streamer = new PriceStreamer(accessToken, "AAPL", mockMode);
    thread = new QThread(this);

    streamer->moveToThread(thread);
    connect(thread, &QThread::started, streamer, &PriceStreamer::start);
    connect(streamer, &PriceStreamer::priceUpdated, this, &MainApp::onPriceUpdated);

    thread->start();
}

MainApp::~MainApp() {
    streamer->stop();
    thread->quit();
    thread->wait();
    delete streamer;
}

QString MainApp::loadAccessToken() {
    return settings->value("TradeStation/AccessToken").toString();
}

void MainApp::onPriceUpdated(const QJsonObject& priceData) {
    qDebug() << "Main Thread - Price Updated:" << priceData;
}
