#include "guifrontend.h"
#include "ui_guifrontend.h"
#include <QJsonDocument>

GuiFrontend::GuiFrontend(QObject* parent) : AppFrontend(parent) {
    ui = new Ui::GuiFrontend();
    ui->setupUi(new QMainWindow());
    //ui->centralwidget->setParent(this);
    static_cast<QMainWindow*>(ui->centralwidget->parent())->show();
}

GuiFrontend::~GuiFrontend() {
    delete ui;
}

void GuiFrontend::onPriceUpdated(const QJsonObject& priceData) {
    ui->logDisplay->append(QString("Price Updated: %1").arg(QString(QJsonDocument(priceData).toJson(QJsonDocument::Compact))));
}

void GuiFrontend::onPricesFetched() {
    ui->logDisplay->append("Prices Fetched - TODO act on prices");
}
