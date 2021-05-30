#include "console.h"

Console::Console( QIODevice *io )
{
    in = new QTextStream(io);
    //connect(in, &QTextStream::readAll, this, &Console::getData);

}
