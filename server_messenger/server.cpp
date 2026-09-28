#include "server.h"
#include "userrepository.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QTime>

Server::Server(QObject *parent) : QTcpServer(parent)
{
    listen(QHostAddress::Any, 5555);
    qDebug() << "Server started on port 5555";
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead,
            this, &Server::onReadyRead);
    connect(socket, &QTcpSocket::disconnected,
            this, &Server::onDisconnected);

    qDebug() << QTime::currentTime().toString() << " - Client connected " << socket->peerAddress().toString();
}

void Server::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    socketBuffers[socket] += socket->readAll();
    QByteArray &buffer = socketBuffers[socket];

    while (buffer.contains('\n')) {
        int idx = buffer.indexOf('\n');
        QByteArray line = buffer.left(idx);
        buffer = buffer.mid(idx + 1);

        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            qDebug() << "Invalid JSON:" << line;
            continue;
        }

        QJsonObject req = doc.object();
        QJsonObject reply;

        QString type = req["type"].toString();
        reply["type"] = type;

        if(type == "sendMessage") {
            bool ok = UserRepository::sendMessage(
                    req["sender_id"].toInt(),
                    req["reciever_id"].toInt(),
                    req["text"].toString()
                    );
            reply["status"] = ok;

            reply["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();

            if (ok){
                if (onlineUsers.contains(req["reciever_id"].toInt())){
                    QJsonObject msg;
                    msg["type"] = "recieveMessage";
                    msg["sender_id"] = req["sender_id"].toInt();
                    msg["reciever_id"] = req["reciever_id"].toInt();
                    msg["text"] = req["text"].toString();
                    msg["sender_nickname"] = UserRepository::getNickname(req["sender_id"].toInt());
                    msg["timestamp"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
                    if (onlineUsers.contains(req["reciever_id"].toInt())) {
                        QTcpSocket* target = onlineUsers[req["reciever_id"].toInt()];

                        target->write(
                            QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n"
                        );
                    }
                }
            }
        }
        else if (type == "register") {
            QString error;
            bool ok = UserRepository::registerUser(
                        req["email"].toString(),
                    req["nickname"].toString(),
                    req["password"].toString(),
                    error
                    );

            reply["status"] = ok ? "ok" : "error";
            if (!ok) {
                reply["message"] = error;
                qDebug() <<  QTime::currentTime().toString()
                          << " - Failed to register user " << req["nickname"].toString() << ": " << error;
            } else {
                qDebug() <<  QTime::currentTime().toString()
                          << " - Registered user " << req["nickname"].toString();
            }
        }
        else if (type == "login") {
            QString error;
            int userId = -1;
            QString res = UserRepository::loginUser(req["email"].toString(), req["password"].toString(), userId, error);
            if (res != "error"){
                reply["status"] = "ok";
                reply["userId"] = userId;
                reply["nickname"] = UserRepository::getNickname(userId);
                reply["avatar"] = res;

                onlineUsers[userId] = socket;
                onlineSockets[socket] = userId;

                qDebug() << QTime::currentTime().toString()
                         << " - User login ID: "
                         << onlineUsers[userId];
            } else {
                reply["status"] = "error";
                reply["message"] = error;
            }
        }
        else if (type == "isUserExists"){
            QJsonArray res = UserRepository::isUserExists(req["nickname"].toString());
            if (res[0] == "-1"){
                reply["status"] = "error";
            } else {
                reply["status"] = "ok";
                reply["id"] = res[0];
                reply["nickname"] = res[1];
            }

        }
        else if(type == "requestChats"){
            reply["chats"] = UserRepository::getChats(req["user_id"].toInt());
            if (!reply["chats"].toArray().empty()){
                reply["status"] = "ok";
            } else reply["status"] = "error";
        }
        else if(type == "requestChatMessages"){
            reply["messages"] = UserRepository::getChatMessages(req["sender_id"].toInt(), req["reciever_id"].toInt());
            if (!reply["messages"].toArray().empty()){
                reply["status"] = "ok";
            } else reply["status"] = "error";
        }
        else if(type == "changeProfile"){
            bool res = UserRepository::changeProfile(req["id"].toInt(), req["nickname"].toString(),
                    req["status"].toString(), req["description"].toString(), req["avatar"].toString());
            if (res){
                reply["status"] = "ok";
            }
            else{
                reply["status"] = "error";
            }
        }
        else if(type == "getProfileInfo"){
            QJsonArray res = UserRepository::getProfileInfo(req["id"].toInt());
            if (res.isEmpty()){
                reply["status"] = "error";
            }else{
                reply["status"] = "ok";
            }
            reply["info"] = res;
            reply["isForClient"] = req["isForClient"];
        }
        else {
            reply["status"] = "error";
            reply["message"] = "Unknown request type";
        }

        socket->write(QJsonDocument(reply).toJson(QJsonDocument::Compact) + "\n");
    }
}

void Server::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (onlineSockets.contains(socket)) {
        int userId = onlineSockets[socket];
        qDebug() << QTime::currentTime().toString()
                 << " - User disconnected ID: "
                 << onlineUsers[userId];
        onlineUsers.remove(userId);
        onlineSockets.remove(socket);
    }
    socketBuffers.remove(socket);
}
