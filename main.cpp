#include <QCoreApplication>
#ifdef GUI_ENABLED
#include <QApplication>
#include "gui/guifrontend.h"
#else
#include "core/terminalfrontend.h"
#endif
#include <QCommandLineParser>
#include <QDebug>
#include "core/mainapp.h"

void processCLIArgs(const QStringList &args, bool &mockMode, QString &configFile, QString &criteriaFile, QString &nasdaqCsvFile){
    QCommandLineParser parser;
    parser.setApplicationDescription("TradeStation algorithmic trading application");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption mockOption("mock", "Run in mock mode without real API calls");
    parser.addOption(mockOption);

    QCommandLineOption configOption("config", "Path to the configuration file (e.g., config.ini)", "file");
    parser.addOption(configOption);

    QCommandLineOption criteriaOption("criteria", "Path to the criteria configuration file (e.g., criteria.ini)", "file");
    parser.addOption(criteriaOption);

    QCommandLineOption nasdaqCsvOption("nasdaq-csv", "Path to CSV file containing NASDAQ symbols (e.g., nasdaq-csv.csv)", "file");
    parser.addOption(nasdaqCsvOption);

    // Process command-line arguments
    parser.process(args);

    mockMode = parser.isSet(mockOption);

    configFile = parser.value(configOption);
    if(configFile.isEmpty()){
        qFatal() << "No config file specified. Usage: ./tradestation_algo --config <config_file>";
    }

    criteriaFile = parser.value(criteriaOption);
    if(criteriaFile.isEmpty()){
        qFatal() << "No criteriaFile file specified. Usage: ./tradestation_algo --criteria <criteria_file>";
    }

    nasdaqCsvFile = parser.value(nasdaqCsvOption);
    if(nasdaqCsvFile.isEmpty()){
        qFatal() << "No nasdaq-csv file specified. Usage: ./tradestation_algo --nasdaq-csv <nasdaq_csv_file>";
    }
}

int main(int argc, char* argv[]) {
#ifdef GUI_ENABLED
    QApplication app(argc, argv);
#else
    QCoreApplication app(argc, argv);
#endif
    bool mockMode;
    QString configFile;
    QString criteriaFile;
    QString nasdaqCsvFile;

    QCoreApplication::setApplicationName("TradeStation Algo");
    QCoreApplication::setApplicationVersion("1.0");

    processCLIArgs(app.arguments(), mockMode, configFile, criteriaFile, nasdaqCsvFile);


    MainApp mainApp(configFile, nasdaqCsvFile, mockMode);

#ifdef GUI_ENABLED
    mainApp.setFrontend(new GuiFrontend(&mainApp));
#else
    mainApp.setFrontend(new TerminalFrontend(&mainApp));
#endif

    return app.exec();
}
