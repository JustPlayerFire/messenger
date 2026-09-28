#include "userrepository.h"
#include <QSqlQuery>
#include <QVariant>
#include <QTime>
#include <QJsonObject>
#include <QDebug>
#include <QSqlError>
#include <QUuid>
#include <QDir>
#include <QImage>
#include <QBuffer>

bool UserRepository::registerUser(const QString& email,
                                  const QString& nickname,
                                  const QString& password,
                                  QString& error)
{
    QSqlQuery q;
    q.prepare("INSERT INTO users (email, nickname, password) VALUES (?, ?, ?)");
    q.addBindValue(QVariant(email));
    q.addBindValue(QVariant(nickname));
    q.addBindValue(QVariant(password));

    if (!q.exec()) {
        error = "User already exists";
        return false;
    }

    return true;
}

QString UserRepository::loginUser(const QString& email,
                               const QString& password,
                               int& userId,
                               QString& error)
{
    QSqlQuery q;
    q.prepare("SELECT id, avatar FROM users WHERE email=? AND password=?");
    q.addBindValue(QVariant(email));
    q.addBindValue(QVariant(password));
    q.exec();

    if (q.next()) {
        userId = q.value(0).toInt();
        QString avatar;

        QString avatarPath = q.value(1).toString();
        if (!avatarPath.isEmpty()) {
            QFile file(avatarPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray avatarData = file.readAll();
                avatar = QString(avatarData.toBase64());
            } else {
                qDebug() << "Cannot open avatar file:" << avatarPath;
                avatar = QString();
            }
        } else {
            avatar = QString();
        }
        return avatar;
    }

    error = "Invalid email or password";
    return "error";
}


bool UserRepository::sendMessage(const int& sender_id, const int& receiver_id, const QString& text)
{
    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender_id, receiver_id, text, timestamp) VALUES (?, ?, ?, ?)");
    q.addBindValue(sender_id);
    q.addBindValue(receiver_id);
    q.addBindValue(text);
    q.addBindValue(QDateTime::currentDateTime());
    if (!q.exec()) {
        return false;
    }
    return true;
}


QJsonArray UserRepository::isUserExists(const QString& nickname){
    QSqlQuery q;
    q.prepare("SELECT id, nickname FROM users WHERE nickname=?");
    q.addBindValue(QVariant(nickname));
    q.exec();
    if (!q.next()) {
        return QJsonArray({"-1"});
    }else{
        return QJsonArray({q.value(0).toString(), q.value(1).toString()});
    }

}


QString UserRepository::getNickname(int id){
    QSqlQuery q;
    q.prepare("SELECT nickname FROM users WHERE id=?");
    q.addBindValue(QVariant(id));
    if (!q.exec()) {
        return "-1";
    }

    if (q.next()) {
            return q.value(0).toString();
        }

}

QJsonArray UserRepository::getChats(const int &user_id) {
    QSqlQuery q;
    QString query =
        "SELECT "
            "CASE WHEN m.sender_id = :uid THEN m.receiver_id ELSE m.sender_id END AS other_user_id, "
            "u.nickname AS other_user_nickname, "
            "u.avatar AS other_user_avatar, "
            "m.text AS last_message_text, "
            "m.timestamp AS last_message_timestamp, "
            "m.sender_id AS last_message_sender_id "
        "FROM messages m "
        "JOIN users u ON u.id = CASE WHEN m.sender_id = :uid THEN m.receiver_id ELSE m.sender_id END "
        "WHERE (m.sender_id = :uid OR m.receiver_id = :uid) "
          "AND m.timestamp = ("
              "SELECT MAX(m2.timestamp) "
              "FROM messages m2 "
              "WHERE (m2.sender_id = m.sender_id AND m2.receiver_id = m.receiver_id) "
                 "OR (m2.sender_id = m.receiver_id AND m2.receiver_id = m.sender_id)"
          ") "
        "ORDER BY m.timestamp DESC";

    if (!q.prepare(query)) {
        qDebug() << "Prepare error:" << q.lastError();
        return QJsonArray();
    }

    q.bindValue(":uid", user_id);

    QJsonArray chatsArray;

    if (!q.exec()) {
        qDebug() << "Exec error:" << q.lastError();
        return chatsArray;
    }

    while (q.next()) {
        QJsonObject chat;
        chat["other_user_id"] = q.value("other_user_id").toInt();
        chat["other_user_nickname"] = q.value("other_user_nickname").toString();

        QString avatarPath = q.value("other_user_avatar").toString();
        if (!avatarPath.isEmpty()) {
            QFile file(avatarPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray avatarData = file.readAll();
                chat["other_user_avatar"] = QString(avatarData.toBase64());
            } else {
                qDebug() << "Cannot open avatar file:" << avatarPath;
                chat["other_user_avatar"] = QString();
            }
        } else {
            chat["other_user_avatar"] = QString();
        }

        chat["last_message_text"] = q.value("last_message_text").toString();
        chat["last_message_timestamp"] = q.value("last_message_timestamp").toDateTime().toString(Qt::ISODate);
        chat["last_message_sender_id"] = q.value("last_message_sender_id").toInt();
        chatsArray.append(chat);
    }

    return chatsArray;
}



QJsonArray UserRepository::getChatMessages(const int &sender_id, const int &reciever_id){
    QSqlQuery q;
    QString query = "SELECT sender_id, receiver_id, text, timestamp FROM messages WHERE (sender_id = ? AND receiver_id = ? )"
            "OR (sender_id = ? AND receiver_id = ? ) ORDER BY timestamp";

    if (!q.prepare(query)) {
        qDebug() << "Prepare error:" << q.lastError();
        return QJsonArray();
    }

    q.addBindValue(sender_id);
    q.addBindValue(reciever_id);
    q.addBindValue(reciever_id);
    q.addBindValue(sender_id);

    QJsonArray messagesArray;

    if (!q.exec()) {
        qDebug() << "Exec error:" << q.lastError();
        return messagesArray;
    }

    while (q.next()) {
        QJsonObject messages;
        messages["sender_id"] = q.value("sender_id").toInt();
        messages["receiver_id"] = q.value("receiver_id").toInt();
        messages["text"] = q.value("text").toString();
        messages["timestamp"] = q.value("timestamp").toDateTime().toString(Qt::ISODate);
        messagesArray.append(messages);
    }

    return messagesArray;
}

bool UserRepository::changeProfile(const int &user_id, const QString &nickname, const QString &status, const QString &description, const QString &avatarBase64)
{
    QString cleanBase64 = avatarBase64;
    QString avatarPath = "";
    if (!cleanBase64.isEmpty()){

        if (cleanBase64.startsWith("data:image/")) {
            int comma = cleanBase64.indexOf(',');
            if (comma != -1) {
                cleanBase64 = cleanBase64.mid(comma + 1);
            }
        }

        cleanBase64 = cleanBase64.trimmed()
                          .replace("\n", "")
                          .replace("\r", "")
                          .replace(" ", "");

        QByteArray avatarData = QByteArray::fromBase64(cleanBase64.toUtf8());

        if (avatarData.size() < 100) {
            qDebug() << "ERROR: Avatar data too small or empty!";
            return false;
        }

        QImage avatar;
        if (!avatar.loadFromData(avatarData)) {
            qDebug() << "Failed to load avatar (wrong format or corrupted)";
            return false;
        }

        avatarPath = QString("avatars/%1.png").arg(user_id);

        if (!QDir("avatars").exists()) {
            QDir().mkpath("avatars");
        }

        if (!avatar.save(avatarPath, "PNG", 90)) {
            qDebug() << "Failed to save avatar:" << avatarPath;
            return false;
        }
    }

    QSqlQuery query;
    query.prepare("UPDATE users SET nickname = ?, status = ?, description = ?, avatar = ? WHERE id = ?");
    query.addBindValue(nickname);
    query.addBindValue(status);
    query.addBindValue(description);
    query.addBindValue(avatarPath);
    query.addBindValue(user_id);

    if (!query.exec()) {
        qDebug() << "DB error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Profile updated successfully for user" << user_id;
    return true;
}


QJsonArray UserRepository::getProfileInfo(const int &user_id) {

    QSqlQuery q;
    QString query = "SELECT nickname, status, description, avatar FROM users WHERE id=?";
    if (!q.prepare(query)) {
        qDebug() << "Prepare error:" << q.lastError();
        return QJsonArray();
    }

    q.addBindValue(user_id);

    if (!q.exec()) {
        qDebug() << "Exec error:" << q.lastError();
        return QJsonArray();
    }

    QJsonArray resArray;

    if (q.next()) {
        resArray.append(q.value("nickname").toString());
        resArray.append(q.value("status").toString());
        resArray.append(q.value("description").toString());

        QString avatarPath = q.value("avatar").toString();
        QString avatarBase64;
        if (!avatarPath.isEmpty()) {
            QFile file(avatarPath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray avatarData = file.readAll();
                avatarBase64 = QString(avatarData.toBase64());
            } else {
                qDebug() << "Cannot open avatar file:" << avatarPath;
                avatarBase64 = QString();
            }
        } else {
            avatarBase64 = QString();
        }


        resArray.append(avatarBase64);

    } else {
        qDebug() << "User not found";
    }

    return resArray;
}
