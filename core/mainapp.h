#ifndef MAINAPP_H
#define MAINAPP_H

#include <QObject>
#include "pricestreamer.h"
#include "pricefetcher.h"
#include "mockedstockprices.h"

class QThread;
class QSettings;

class MainApp : public QObject {
    Q_OBJECT
public:
    MainApp(const QString& configFile, const QString& nasdaqCsvFile, bool mockMode = false, QObject* parent = nullptr);
    ~MainApp();

private slots:
    // Called when a new price from the single stock stream is received
    void onPriceUpdated(const QJsonObject& priceData);

    // Called when all stock prices from the whole list are fetched
    void onPricesFetched();

private:
    PriceStreamer* streamer;
    QThread* thread;

    PriceFetcher* price_fetcher;
    QThread* price_fetcher_thread;

    MockedStockPrices* mocked_stock_prices;

    QSettings* settings;

    QList<Stock> NASDAQ_stocks;

    QString loadAccessToken(bool mockMode);
};

#endif // MAINAPP_H
