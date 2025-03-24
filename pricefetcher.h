#ifndef PRICEFETCHER_H
#define PRICEFETCHER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QMutex>
#include "mockedstockprices.h"
#include "stock.h"

class QNetworkReply;

class PriceFetcher : public QObject {
    Q_OBJECT
public:
    PriceFetcher(const QString& accessToken, QList<Stock>& stocks, bool mockMode = false, int intervalMinutes = 1, MockedStockPrices* mockPrices = nullptr, QObject* parent = nullptr);
    ~PriceFetcher();

    void start();
    void stop();

signals:
    void pricesFetched();

private slots:
    void fetchPrices();
    void onQuotesReceived();

private:
    QNetworkAccessManager* network_manager;
    QString accessToken;
    QList<Stock>& stocks;
    QList<QNetworkReply*> pendingReplies;
    QMutex stocksMutex; // TODO wont need this probably
    MockedStockPrices* mockPrices;  // Reference to mock data source
    bool running;
    bool mockMode;         // Added mock mode
    QTimer* fetch_timer;

    static const int BATCH_SIZE = 500;
};

#endif // PRICEFETCHER_H
