#include "floatfetcher.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QEventLoop>

FloatFetcher::FloatFetcher() {}

void FloatFetcher::fetchTickerToCik() {
    tickerToCikMap = loadTickerToCikMap();

    if (tickerToCikMap.isEmpty()) {
        qFatal() << "Error while loading the Ticker to Cik database";
    }

    qDebug() << "Loaded tickers:" << tickerToCikMap.keys().size();
}

qint64 FloatFetcher::fetchFloatFromTicker(QString &ticker) {
    if (tickerToCikMap.isEmpty()) {
        qFatal() << "Error Ticker to Cik database isnt fetched";
    }

    QString cik = tickerToCikMap[ticker].toObject()["cik_str"].toString();

    if (cik.isEmpty()) {
        qFatal() << "Cannot find cik number in database for ticker " << ticker;
    }

    return fetchFloat(cik);
}

// Parse JSON into ticker map with padded CIK
QJsonObject FloatFetcher::parseTickerToCikMap(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject map;
    QJsonObject obj = doc.object();
    for (const QString& key : obj.keys()) {
        QJsonObject entry = obj[key].toObject();
        QString paddedCik = QString("%1").arg(entry["cik_str"].toInt(), 10, 10, QChar('0'));
        entry["cik_str"] = paddedCik;
        map[entry["ticker"].toString().toUpper()] = entry;
    }
    return map;
}

// Load or download ticker map
QJsonObject FloatFetcher::loadTickerToCikMap() {
    QByteArray data;
    QFile file("company_tickers_.json");

    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
        qDebug() << "company_tickers.json loaded locally";
    } else {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl("https://www.sec.gov/files/company_tickers.json"));
        request.setHeader(QNetworkRequest::UserAgentHeader, "MyStockApp/1.0 (your.email@example.com)"); // Replace with your app/email
        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error downloading ticker map:" << reply->errorString();
            qDebug() << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            qDebug() << "Response:" << reply->readAll();
            reply->deleteLater();
            return QJsonObject();
        }

        data = reply->readAll();
        if (file.open(QIODevice::WriteOnly)) {
            file.write(data);
            file.close();
        }

        reply->deleteLater();

        qDebug() << "company_tickers.json had to be downloaded";
    }

    return FloatFetcher::parseTickerToCikMap(data);
}

qint64 FloatFetcher::fetchFloat(const QString& cik) {
    QNetworkAccessManager manager;
    QString url = "https://data.sec.gov/api/xbrl/companyfacts/CIK" + cik + ".json";
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::UserAgentHeader, "MyStockApp/1.0 (your.email@example.com)");

    QNetworkReply* reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error fetching float for CIK" << cik << ":" << reply->errorString();
        qDebug() << "HTTP Status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "Response:" << reply->readAll();
        reply->deleteLater();
        return -1;
    }

    QByteArray data = reply->readAll();
    if (data.isEmpty()) {
        qDebug() << "Empty response for CIK" << cik;
        reply->deleteLater();
        return -1;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (doc.isNull()) {
        qDebug() << "JSON parse error for CIK" << cik << ":";
        qDebug() << "Error:" << parseError.errorString() << "Offset:" << parseError.offset;
        qDebug() << "Data:" << data;
        reply->deleteLater();
        return -1;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("facts") || !obj["facts"].toObject().contains("us-gaap")) {
        qDebug() << "Missing facts or us-gaap for CIK" << cik;
        reply->deleteLater();
        return -1;
    }

    QJsonObject usGaap = obj["facts"].toObject()["us-gaap"].toObject();
    QStringList possibleFields = {"SharesOutstanding", "CommonStockSharesOutstanding", "EntityCommonStockSharesOutstanding"};
    for (const QString& field : possibleFields) {
        if (usGaap.contains(field)) {
            QJsonObject sharesObj = usGaap[field].toObject();
            if (sharesObj.contains("units") && sharesObj["units"].toObject().contains("shares")) {
                QJsonArray floatData = sharesObj["units"].toObject()["shares"].toArray();
                if (!floatData.isEmpty()) {
                    qint64 floatShares = floatData.last().toObject()["val"].toVariant().toLongLong();
                    qDebug() << "Found float under" << field << "for CIK" << cik << ":" << floatShares;
                    reply->deleteLater();
                    return floatShares;
                }
            }
        }
    }

    qDebug() << "No matching SharesOutstanding field for CIK" << cik;
    qDebug() << "Available us-gaap keys:" << usGaap.keys();
    reply->deleteLater();
    return -1;
}
