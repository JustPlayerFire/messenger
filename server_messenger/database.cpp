#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

bool Database::init()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("messenger.db");

    if (!db.open()) {
        qDebug() << "DB open error:" << db.lastError();
        return false;
    }

    QSqlQuery q;
    QSqlQuery q2;

    q.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "email TEXT UNIQUE,"
        "nickname TEXT,"
        "password TEXT,"
        "description TEXT,"
        "status TEXT,"
        "avatar BLOB"
        ")"
    );

    q2.exec(
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender_id INTEGER,"
        "receiver_id INTEGER,"
        "text TEXT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ")"
    );
    return true;
}
