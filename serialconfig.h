#ifndef SERIALCONFIG_H
#define SERIALCONFIG_H

#include <qtypes.h>
#include <QSerialPort>
#include <QString>

struct SerialConfig
{
    qint32 baudrate;
    QSerialPort::DataBits databits;
    QSerialPort::StopBits stopbits;
    QSerialPort::Parity parity;
    QSerialPort::FlowControl flowControl;
    QString name;
};

#endif // SERIALCONFIG_H
