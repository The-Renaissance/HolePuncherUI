#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include "serialconfig.h"

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);

public slots:
    void setFile(const QString& filename);
    void receiveConfig(const SerialConfig& config);
    void start();

signals:
    void finished();
    // void serialError(QSerialPort::SerialPortError err);
    void error(const QString& err);

private:
    QString m_filename;
    QSerialPort m_port;
    struct
    {
        struct
        {
            double width, height;
        } size;
        struct hole
        {
            hole(double x, double y) : x(x), y(y) {}
            double x, y;
        };
        QVector<hole> holes;
    } m_svgCanvas;
    const int m_timeout_ms = 10000; // Serial timeout is 10s.

    bool parseFile(const QString& filename);
    template <size_t N>
    bool waitForLine(char (&buf)[N]);
    template <size_t N>
    bool waitForOK(char (&buf)[N]);
    template <size_t N>
    bool punchHoles(double cx, double cy, char (&buf)[N]);
};

#endif // SERIALWORKER_H
