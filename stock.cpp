#include "stock.h"
#include <QStringList>
#include <QDebug>
#include <QFile>

Stock::Stock(const QString &line)
{
    QStringList fields = line.split(",", Qt::KeepEmptyParts);
    if (fields.size() >= 7) {
        index = fields[0].toInt();
        symbol = fields[1];
        companyName = fields[2].trimmed().remove('"');  // Remove quotes if present
        marketCap = fields[3].remove('B').toDouble();
        lastSale = fields[4].toDouble();
        percentChange = fields[5].remove('%').toDouble();
        revenue = fields[6].remove('B').toDouble();
        exchange = "NASDAQ";
    } else {
        qFatal() << "Invalid CSV line:" << line;
    }
}


QList<Stock> Stock::parseCsv(const QString& filePath) {
    QList<Stock> stocks;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qFatal() << "Failed to open CSV file:" << filePath;
        return stocks;  // Return empty list on failure
    }

    QTextStream in(&file);
    bool isHeader = true;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (isHeader) {
            isHeader = false;  // Skip header
            continue;
        }
        if (!line.isEmpty()) {
            stocks.append(Stock(line));
        }
    }
    file.close();

    qDebug() << "Parsed" << stocks.size() << "stocks from" << filePath;
    return stocks;
}
