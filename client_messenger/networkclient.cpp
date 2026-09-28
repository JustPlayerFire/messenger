#include "networkclient.h"
#include <QJsonDocument>
#include <QDebug>
#include <QSettings>
#include <QPixmap>
#include <QBuffer>

NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);
    tcpBuffer = new QByteArray();
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void NetworkClient::connectToServer(const QString &host, quint16 port)
{
    socket->connectToHost(host, port);
}

void NetworkClient::registerUser(const QString &email, const QString &nickname, const QString &password)
{
    QJsonObject req;
    req["type"] = "register";
    req["email"] = email;
    req["nickname"] = nickname;
    req["password"] = password;

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact)  + "\n");
}

void NetworkClient::loginUser(const QString &email, const QString &password)
{
    QJsonObject req;
    req["type"] = "login";
    req["email"] = email;
    req["password"] = password;

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::checkUserAndCreateChat(const QString &nickname)
{
    QJsonObject req;
    req["type"] = "isUserExists";
    req["nickname"] = nickname;


    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::sendMessage(const int &sender_id, const int &reciever_id, const QString &text)
{
    QJsonObject req;
    req["type"] = "sendMessage";
    req["sender_id"] = sender_id;
    req["reciever_id"] = reciever_id;
    req["text"] = text;

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::requestChats(const QString &user_id){
    QJsonObject req;
    req["type"] = "requestChats";
    req["user_id"] = user_id.toInt();

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::requestChatMessages(const QString &user_id, const QString &reciever_id){
    QJsonObject req;
    req["type"] = "requestChatMessages";
    req["sender_id"] = user_id.toInt();
    req["reciever_id"] = reciever_id.toInt();

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::changeProfile(const QString &user_id, const QString &nickname,
                                  const QString &status, const QString &description,
                                  const QString &avatarBase64){
    QJsonObject req;
    req["type"] = "changeProfile";
    req["id"] = user_id.toInt();
    req["nickname"] = nickname;
    req["status"] = status;
    req["description"] = description;
    req["avatar"] = avatarBase64;

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}


void NetworkClient::getProfileInfo(const QString &user_id, const bool &isForClient){
    QJsonObject req;
    req["type"] = "getProfileInfo";
    req["id"] = user_id.toInt();
    req["isForClient"] = isForClient;

    socket->write(QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkClient::onConnected()
{
    qDebug() << "Connected to server";
    emit connected();
}

void NetworkClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void NetworkClient::onReadyRead()
{
    *tcpBuffer += socket->readAll();

    while (tcpBuffer->contains('\n')) {
        int idx = tcpBuffer->indexOf('\n');
        QByteArray line = tcpBuffer->left(idx);
        *tcpBuffer = tcpBuffer->mid(idx + 1);

        if (line.isEmpty()){
            continue;
        }

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            qDebug() << "Invalid JSON:" << line;
            continue;
        }

        QJsonObject obj = doc.object();
        QString type = obj.value("type").toString();

        if (type == "login") {

            bool ok = obj.value("status").toString() == "ok";
            int userId = obj.value("userId").toInt(-1);
            QString nickname = obj.value("nickname").toString();
            QString message = obj.value("message").toString();
            QString avatar = obj.value("avatar").toString();

            emit loginResult(ok, message, userId, nickname, avatar);
        }

        else if (type == "register") {

            bool ok = obj.value("status").toString() == "ok";
            QString message = obj.value("message").toString();

            emit registerResult(ok, message);
        }

        else if (type == "isUserExists") {
            emit checkUserResult(obj.value("id").toString(), obj.value("nickname").toString());
        }

        else if (type == "sendMessage") {
            emit messageSent(obj);
        }
        else if (type == "recieveMessage") {
            emit messageReceived(obj);
        }
        else if (type == "requestChats"){
            emit gotChats(obj);
        }
        else if (type == "requestChatMessages"){
            emit gotChatMessages(obj);
        }
        else if (type == "getProfileInfo"){
            emit gotProfileInfo(obj);
        }
    }
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString err = socket->errorString();
    qDebug() << "Socket error:" << err;
    emit errorOccurred(err);
}

QAbstractSocket::SocketState NetworkClient::getState(){
    return socket->state();
}


void NetworkClient::abort(){
    socket->abort();
}


