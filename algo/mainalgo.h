#ifndef MAINALGO_H
#define MAINALGO_H

#include <QObject>
#include <QTimer>
#include <QThread>

class MainAlgo : public QObject {
    Q_OBJECT
public:
    explicit MainAlgo(QObject* parent = nullptr);
    ~MainAlgo() override;

public slots:
    void start();  // Start the algo logic
    void stop();   // Stop the algo logic

private slots:
    void process();  // Periodic trading logic (stub for now)

private:
    QTimer* timer;
    QThread* thread;
    bool running;
};

#endif // MAINALGO_H
