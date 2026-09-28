#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H
#include <QString>
#include <QJsonArray>

class UserRepository {

public:
    static bool registerUser(const QString& email, const QString& nickname, const QString& password, QString& error);
    static QString loginUser(const QString& email, const QString& password, int& userId, QString& error);
    static bool sendMessage(const int& sender_id, const int& receiver_id, const QString& text);
    static QJsonArray isUserExists(const QString& nickname);
    static QJsonArray getChats(const int& user_id);
    static QJsonArray getChatMessages(const int &sender_id, const int &reciever_id);
    static QString getNickname(int id);
    static bool changeProfile(const int &user_id, const QString &nickname, const QString &status, const QString &description, const QString &avatarBase64);
    static QJsonArray getProfileInfo(const int &sender_id);

};

#endif // USERREPOSITORY_H
