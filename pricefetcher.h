#ifndef PRICEFETCHER_H
#define PRICEFETCHER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QMutex>
#include "stock/stock.h"

class QNetworkReply;

class PriceFetcher : public QObject {
    Q_OBJECT
public:
    PriceFetcher(const QString& accessToken, QList<Stock>& stocks, bool mockMode = false, int intervalMinutes = 1, QObject* parent = nullptr);
    ~PriceFetcher();

    void start();
    void stop();

signals:
    void pricesUpdated();

private slots:
    void fetchPrices();
    void onQuotesReceived();

private:
    QString accessToken;
    QList<Stock>& stocks;
    bool mockMode;         // Added mock mode
    QMutex stocksMutex;
    QTimer* timer;
    QNetworkAccessManager* manager;
    QList<QNetworkReply*> pendingReplies;

    static const int BATCH_SIZE = 500;
};

#endif // PRICEFETCHER_H
