#ifndef STOCK_H
#define STOCK_H

#include <QString>

class Stock
{
public:
    Stock(const QString& line); // From a CSV line

    // Static method to parse CSV and return a list of Stocks
    static QList<Stock> parseCsv(const QString& filePath);

    int index;
    QString symbol;
    QString companyName;
    double marketCap;  // In billions
    double lastSale;   // Price
    double percentChange;  // As a percentage
    double revenue;    // In billions
    QString exchange;
};

#endif // STOCK_H
