#ifndef RECV_H
#define RECV_H

#include <iostream>
#include <QSerialPort>
#include <QThread>
#include <QDebug>

class Recv : public QThread
{
public:
    Recv(QSerialPort *serial);

public slots:
    void readData();
    void worker();

private:
    QSerialPort *serial;
    QByteArray arr;
};

#endif // RECV_H
