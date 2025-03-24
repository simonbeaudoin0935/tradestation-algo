#include "pricefetcher.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <QRandomGenerator>
#include <QDebug>

PriceFetcher::PriceFetcher(const QString& accessToken, QList<Stock>& stocks, bool mockMode, int intervalMinutes, MockedStockPrices* mockPrices, QObject* parent) :
    QObject(parent),
    network_manager(new QNetworkAccessManager(this)),
    accessToken(accessToken),
    stocks(stocks),
    mockPrices(mockPrices),
    running(true),
    mockMode(mockMode),
    fetch_timer(new QTimer(this))
{
    fetch_timer->setInterval(intervalMinutes * 60 * 1000);  // e.g., 1 min = 60,000 ms
    connect(fetch_timer, &QTimer::timeout, this, &PriceFetcher::fetchPrices);
}

PriceFetcher::~PriceFetcher() {
    stop();
}

void PriceFetcher::start() {
    fetchPrices();  // Fetch immediately
    fetch_timer->start();  // Start periodic fetching
}

void PriceFetcher::stop() {
    running = false;

    fetch_timer->stop();
    for (QNetworkReply* reply : pendingReplies) {
        reply->abort();
        reply->deleteLater();
    }
    pendingReplies.clear();
}

void PriceFetcher::fetchPrices() {
    QMutexLocker locker(&stocksMutex);
    if (stocks.isEmpty()) {
        qFatal() << "No stocks to fetch prices for";
        return;
    }

    if (mockMode) {
       if (!mockPrices) {
           qFatal() << "Mock mode enabled but no MockedStockPrices provided";
        }
        // Fetch all prices from MockedStockPrices
        QMap<QString, double> mockPricesMap = mockPrices->getAllPrices();
        for (Stock& stock : stocks) {
            stock.lastSale = mockPricesMap.value(stock.symbol, stock.lastSale);  // Update or keep original if not found
        }
        locker.unlock();  // Unlock before emitting
        qDebug() << "Mock updated prices for" << stocks.size() << "stocks";
        emit pricesFetched();
        return;
    }

    pendingReplies.clear();
    QStringList symbols;
    for (const Stock& stock : stocks) {
        symbols.append(stock.symbol);
    }
    locker.unlock();  // Unlock before network requests

    for (int i = 0; i < symbols.size(); i += BATCH_SIZE) {
        QStringList batch = symbols.mid(i, BATCH_SIZE);
        QString symbolBatch = batch.join(",");
        QNetworkRequest request(QUrl("https://api.tradestation.com/v3/marketdata/quotes/" + symbolBatch));
        request.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());
        QNetworkReply* reply = network_manager->get(request);
        connect(reply, &QNetworkReply::finished, this, &PriceFetcher::onQuotesReceived);
        pendingReplies.append(reply);

        if (i + BATCH_SIZE < symbols.size()) {
            QThread::msleep(500);  // Rate limit delay
        }
    }
}

void PriceFetcher::onQuotesReceived() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!pendingReplies.contains(reply)) return;

    QMutexLocker locker(&stocksMutex);
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray quotes = doc.array();
        for (const QJsonValue& quote : quotes) {
            QJsonObject obj = quote.toObject();
            QString symbol = obj["Symbol"].toString();
            double price = obj["Last"].toDouble();
            for (Stock& stock : stocks) {
                if (stock.symbol == symbol) {
                    stock.lastSale = price;
                    break;
                }
            }
        }
    } else {
        qDebug() << "Error fetching quotes:" << reply->errorString();
    }

    pendingReplies.removeOne(reply);
    reply->deleteLater();

    if (pendingReplies.isEmpty()) {
        qDebug() << "Updated prices for" << stocks.size() << "stocks";
        emit pricesFetched();
    }
}
