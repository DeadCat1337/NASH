#include "recv.h"

Recv::Recv(QSerialPort *serial)
{
    this->serial = serial;
    this->moveToThread(this);
    connect(serial, &QSerialPort::readyRead, this, &Recv::readData);
    //connect(this, &QThread::start, this, &Recv::worker);
}


using namespace std;
void Recv::readData() {
    arr = serial->readAll();
    arr.replace("\r", "\n");
    cout << arr.constData();
}

using namespace std;
void Recv::worker(){
    //cout << arr.constData();
}
