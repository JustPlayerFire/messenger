#include <QCoreApplication>
#include "server.h"
#include "database.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (!Database::init()) {
        return -1;
    }

    Server server;

    return a.exec();
}
