#include "mainalgo.h"
#include <QDebug>

MainAlgo::MainAlgo(QObject* parent) : QObject(parent), running(false) {
    timer = new QTimer(this);
    thread = new QThread(this);

    // Move this object to its own thread
    moveToThread(thread);
    connect(thread, &QThread::started, this, &MainAlgo::start);
    connect(timer, &QTimer::timeout, this, &MainAlgo::process);

    // Start the thread
    thread->start();
}

MainAlgo::~MainAlgo() {
    stop();
    thread->quit();
    thread->wait();
    // No need to delete timer/thread explicitly—handled by parent relationship
}

void MainAlgo::start() {
    running = true;
    timer->start(5000);  // Run every 5 seconds (adjust as needed)
    qDebug() << "MainAlgo started in thread:" << QThread::currentThreadId();
}

void MainAlgo::stop() {
    running = false;
    timer->stop();
    qDebug() << "MainAlgo stopped";
}

void MainAlgo::process() {
    if (!running) return;
    qDebug() << "MainAlgo processing...";  // Stub—add trading logic here
}
