#include <QCoreApplication>
#include "mainapp.h"
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("TradeStation Algo");
    QCoreApplication::setApplicationVersion("1.0");


    // Set up command-line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("TradeStation algorithmic trading application");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption mockOption("mock", "Run in mock mode without real API calls");
    parser.addOption(mockOption);

    QCommandLineOption configOption("config", "Path to the configuration file (e.g., config.ini)", "file");
    parser.addOption(configOption);

    QCommandLineOption nasdaqCsvOption("nasdaq-csv", "Path to CSV file containing NASDAQ symbols (e.g., nasdaq-csv.csv)", "file");
    parser.addOption(nasdaqCsvOption);

    // Process command-line arguments
    parser.process(app);


    bool mockMode = parser.isSet(mockOption);

    QString configFile = parser.value(configOption);
    if(configFile.isEmpty()){
        qFatal() << "No config file specified. Usage: ./tradestation_algo --config <config_file>";
    }

    QString nasdaqCsvFile = parser.value(nasdaqCsvOption);
    if(nasdaqCsvFile.isEmpty()){
        qFatal() << "No nasdaq-csv file specified. Usage: ./tradestation_algo --nasdaq-csv <nasdaq-csv>";
    }

    MainApp mainApp(configFile, nasdaqCsvFile, mockMode);

    return app.exec();
}
