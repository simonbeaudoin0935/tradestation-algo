#ifndef MOCKEDSTOCKPRICES_H
#define MOCKEDSTOCKPRICES_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include "stock.h"

class MockedStockPrices : public QObject {
    Q_OBJECT
public:
    explicit MockedStockPrices(const QList<Stock>& stocks, QObject* parent = nullptr);
    ~MockedStockPrices();

    // Fetch the latest mock price for a symbol
    double getPrice(const QString& symbol) const;

    // Fetch all current mock prices
    QMap<QString, double> getAllPrices() const;

public slots:
    void start();  // Start price updates
    void stop();   // Stop price updates

private slots:
    void updatePrices();  // Background price update logic

private:
    QMap<QString, double> prices;  // Symbol -> Current Price
    mutable QMutex pricesMutex;            // Thread-safe access
    QTimer* updateTimer;           // Periodic updates
    QThread* thread;               // Own thread
    bool running;

    void initializePrices(const QList<Stock>& stocks);
};

#endif // MOCKEDSTOCKPRICES_H
