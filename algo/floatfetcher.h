#ifndef FLOATFETCHER_H
#define FLOATFETCHER_H

#include <QJsonObject>

class FloatFetcher
{
public:
    FloatFetcher();

    void fetchTickerToCik();

    qint64 fetchFloatFromTicker(QString &ticker);

private:
    static QJsonObject parseTickerToCikMap(const QByteArray& data);
    static QJsonObject loadTickerToCikMap();
    static qint64 fetchFloat(const QString& cik);

    QJsonObject tickerToCikMap;
};

#endif // FLOATFETCHER_H
