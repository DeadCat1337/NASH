#ifndef CONSOLE_H
#define CONSOLE_H

#include <QTextStream>
#include <QThread>


class Console: public QThread
{

public:
    Console(QIODevice *io);

public slots:
    void getData();

private:
    QTextStream *in;

};

#endif // CONSOLE_H
