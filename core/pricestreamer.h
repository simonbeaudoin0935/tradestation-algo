#ifndef PRICESTREAMER_H
#define PRICESTREAMER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QTimer>
#include "mockedstockprices.h"

class PriceStreamer : public QObject {
    Q_OBJECT
public:
    PriceStreamer(const QString& accessToken, const QString& symbol, bool mockMode = false, MockedStockPrices* mockPrices = nullptr, QObject* parent = nullptr);
    ~PriceStreamer();
    void start();
    void stop();

signals:
    void priceUpdated(const QJsonObject& priceData);

private:
    QNetworkAccessManager* network_manager;
    QString accessToken;
    QString symbol;
    QNetworkReply* currentReply = nullptr;
    bool running;
    bool mockMode;
    MockedStockPrices* mockPrices;  // Reference to mock data source
    QTimer* mockTimer = nullptr; // For mock data generation

    void streamPrices();
    void generateMockData();     // New method for mock mode

private slots:
    void onReadyRead();
    void onFinished();
};

#endif // PRICESTREAMER_H
