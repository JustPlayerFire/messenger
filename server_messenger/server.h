#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class Server : public QTcpServer {
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QMap<int, QTcpSocket*> onlineUsers;
    QHash<QTcpSocket*, int> onlineSockets;
    QHash<QTcpSocket*, QByteArray> socketBuffers;
};

#endif // SERVER_H
