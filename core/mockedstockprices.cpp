#include "mockedstockprices.h"
#include <QRandomGenerator>
#include <QDebug>

MockedStockPrices::MockedStockPrices(const QList<Stock>& stocks, QObject* parent)
    : QObject(parent), running(false) {
    // Initialize timer and thread
    updateTimer = new QTimer(this);
    thread = new QThread(this);
    thread->setObjectName("MockedStock");

    // Move this object to its own thread
    moveToThread(thread);
    connect(thread, &QThread::started, this, &MockedStockPrices::start);
    connect(updateTimer, &QTimer::timeout, this, &MockedStockPrices::updatePrices);

    // Populate initial prices from stocks
    initializePrices(stocks);

    // Start the thread
    thread->start();
}

MockedStockPrices::~MockedStockPrices() {
    stop();
    thread->quit();
    thread->wait();
}

void MockedStockPrices::initializePrices(const QList<Stock>& stocks) {
    QMutexLocker locker(&pricesMutex);
    for (const Stock& stock : stocks) {
        prices[stock.symbol] = stock.lastSale;  // Start with CSV price
    }
    qDebug() << "Initialized mock prices for" << prices.size() << "stocks";
}

void MockedStockPrices::start() {
    QMutexLocker locker(&pricesMutex);
    running = true;
    updateTimer->start(1000);  // Update every 1 second
}

void MockedStockPrices::stop() {
    QMutexLocker locker(&pricesMutex);
    running = false;
    updateTimer->stop();
}

double MockedStockPrices::getPrice(const QString& symbol) const {
    QMutexLocker locker(&pricesMutex);
    return prices.value(symbol, 0.0);  // Return 0.0 if symbol not found
}

QMap<QString, double> MockedStockPrices::getAllPrices() const {
    QMutexLocker locker(&pricesMutex);
    return prices;  // Returns a copy, thread-safe due to mutex
}

void MockedStockPrices::updatePrices() {
    QMutexLocker locker(&pricesMutex);
    if (!running) return;

    // Simulate realistic price movements (e.g., trending)
    for (auto it = prices.begin(); it != prices.end(); ++it) {
        double& price = it.value();
        // Random trend direction (up/down) with momentum
        static QMap<QString, double> trends;  // Persistent trend per stock
        double trend = trends.value(it.key(), 0.0);

        // Small chance to change trend (e.g., 5%)
        if (QRandomGenerator::global()->bounded(100) < 5) {
            trend = (QRandomGenerator::global()->bounded(2) == 0 ? -0.01 : 0.01);  // ±0.01
            trends[it.key()] = trend;
        }

        // Apply trend with some noise
        double change = trend + (QRandomGenerator::global()->bounded(10) - 5) * 0.005;  // ±0.025 noise
        price += change;

        // Ensure price stays positive
        if (price < 0.1) price = 0.1;

        // Round to 2 decimal places
        price = QString::number(price, 'f', 2).toDouble();
    }

    // Uncomment for debugging
    // qDebug() << "Updated mock prices:" << prices;
}
