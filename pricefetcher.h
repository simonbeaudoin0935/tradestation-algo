#ifndef PRICEFETCHER_H
#define PRICEFETCHER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QMutex>
#include "stock.h"

class QNetworkReply;

class PriceFetcher : public QObject {
    Q_OBJECT
public:
    PriceFetcher(const QString& accessToken, QList<Stock>& stocks, bool mockMode = false, int intervalMinutes = 1, QObject* parent = nullptr);
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
    QMutex stocksMutex;
    bool running;
    bool mockMode;         // Added mock mode
    QTimer* fetch_timer;

    static const int BATCH_SIZE = 500;
};

#endif // PRICEFETCHER_H
