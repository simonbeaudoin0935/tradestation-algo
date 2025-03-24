#include "pricestreamer.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QEventLoop>
#include <QThread>
#include <QQueue>
#include <QRandomGenerator>
#include <QDebug>
#include <cstring>


// Custom QNetworkReply for mock mode
class MockNetworkReply : public QNetworkReply {
    Q_OBJECT
public:
    MockNetworkReply(QObject* parent = nullptr) : QNetworkReply(parent) {
        open(QIODevice::ReadOnly);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200); // Simulate success
    }

    void setData(const QByteArray& data) {
        QByteArray *array = new QByteArray(data);
        buffer.enqueue(array);

        emit readyRead(); // Trigger reading of mock data
        //emit finished();
    }

    qint64 bytesAvailable() const override {
        if (buffer.isEmpty()){
            return 0;
        } else {
            return buffer.head()->size();
        }
    }

    void abort() override {}

protected:
    qint64 readData(char* data, qint64 maxSize) override {

        if (buffer.isEmpty()){
            return 0;
        }

        QByteArray *array = buffer.dequeue();

        qint64 size = array->size();

        if(size > maxSize){
            qFatal() << "fuck";
        }

        memcpy(data, array->constData(), array->size());

        delete array;

        return size;
    }

private:
    QQueue<QByteArray*> buffer;
};


PriceStreamer::PriceStreamer(const QString& accessToken, const QString& symbol, bool mockMode, QObject* parent)
    : QObject(parent), accessToken(accessToken), symbol(symbol), running(true) {
    manager = new QNetworkAccessManager(this);
    this->mockMode = mockMode;
    if (mockMode) {
        mockTimer = new QTimer(this);
        connect(mockTimer, &QTimer::timeout, this, &PriceStreamer::generateMockData);
    }
}

PriceStreamer::~PriceStreamer() {
    stop();
}

void PriceStreamer::start() {
    streamPrices();
}

void PriceStreamer::stop() {
    running = false;
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
    }
    if (mockMode && mockTimer) {
        mockTimer->stop();
    }
}

void PriceStreamer::streamPrices() {
    while (running) {
        if (mockMode) {
            currentReply = new MockNetworkReply(this);
            mockTimer->start(2000); // Generate mock data every 2 seconds
        } else {
            QNetworkRequest request;
            request.setUrl(QUrl("https://api.tradestation.com/v3/marketdata/stream/quotes/" + symbol));
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            request.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());

            currentReply = manager->get(request);
        }

        connect(currentReply, &QNetworkReply::readyRead, this, &PriceStreamer::onReadyRead);
        connect(currentReply, &QNetworkReply::finished, this, &PriceStreamer::onFinished);

        // Block until the reply finishes or is aborted
        QEventLoop loop;
        connect(currentReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (running) {
            qDebug() << "Stream disconnected. Reconnecting in 5 seconds...";
            QThread::sleep(5); // Wait before retrying
        }
    }
}

void PriceStreamer::generateMockData() {
    if (!mockMode || !currentReply) return;

    // Generate fake price data
    static double lastPrice = 150.0;
    lastPrice += (QRandomGenerator::global()->bounded(10) - 5) * 0.01;
    QJsonObject mockData;
    mockData["Last"] = lastPrice;
    mockData["Bid"] = lastPrice - 0.02;
    mockData["Ask"] = lastPrice + 0.02;
    mockData["Symbol"] = symbol;
    mockData["Time"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QByteArray jsonData = QJsonDocument(mockData).toJson();
    static_cast<MockNetworkReply*>(currentReply)->setData(jsonData);
}

void PriceStreamer::onReadyRead() {
    QByteArray data = currentReply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject priceData = doc.object();
        emit priceUpdated(priceData);

        // Example: Print price data
        if (priceData.contains("Last")) {
            qDebug() << "Last Price:" << priceData["Last"].toDouble();
        }
    } else {
        qDebug() << "Failed to parse JSON:" << data;
    }
}

void PriceStreamer::onFinished() {
    if (currentReply->error() != QNetworkReply::NoError) {
        qDebug() << "Network error:" << currentReply->errorString();
    }
    currentReply->deleteLater();
    currentReply = nullptr;
}

#include "pricestreamer.moc"
