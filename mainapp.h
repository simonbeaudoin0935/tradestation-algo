#ifndef MAINAPP_H
#define MAINAPP_H

#include <QObject>
#include "pricestreamer.h"
#include "pricefetcher.h"

class QThread;
class QSettings;

class MainApp : public QObject {
    Q_OBJECT
public:
    MainApp(const QString& configFile, const QString& nasdaqCsvFile, bool mockMode = false, QObject* parent = nullptr);
    ~MainApp();

private slots:
    void onPriceUpdated(const QJsonObject& priceData);

private:
    PriceStreamer* streamer;
    QThread* thread;

    PriceFetcher* price_fetcher;
    QThread* pricefetcher_thread;

    QSettings* settings;

    QString loadAccessToken(bool mockMode);
};

#endif // MAINAPP_H
