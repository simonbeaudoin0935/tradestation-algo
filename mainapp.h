#ifndef MAINAPP_H
#define MAINAPP_H

#include <QObject>
#include "pricestreamer.h"

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
    QSettings* settings;

    QString loadAccessToken();
};

#endif // MAINAPP_H
