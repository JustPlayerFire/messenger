#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>

class NetworkClient : public QObject
{
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent = 0);
    void connectToServer(const QString &host, quint16 port);
    void registerUser(const QString &email, const QString &nickname, const QString &password);
    void loginUser(const QString &email, const QString &password);
    void sendMessage(const int &sender_id, const int &reciever_id, const QString &text);
    void checkUserAndCreateChat(const QString &nickname);
    void requestChats(const QString &user_id);
    void requestChatMessages(const QString &sender_id, const QString &reciever_id);
    void changeProfile(const QString &user_id, const QString &nickname, const QString &status, const QString &description, const QString &avatarBase64);
    void getProfileInfo(const QString &user_id, const bool &isForClient);
    QAbstractSocket::SocketState getState();
    void abort();

signals:
    void connected();
    void disconnected();
    void loginResult(bool ok, const QString &message, int &userId, const QString &nickname, const QString &avatarBase64);
    void registerResult(bool ok, const QString &message);
    void checkUserResult(const QString &id, const QString &nickname);
    void errorOccurred(const QString &error);
    void messageSent(const QJsonObject &msg);
    void messageReceived(const QJsonObject &msg);
    void gotChats(const QJsonObject &chats);
    void gotChatMessages(const QJsonObject &chats);
    void gotProfileInfo(const QJsonObject &array);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *socket;
    QByteArray *tcpBuffer;
};

#endif // NETWORKCLIENT_H
