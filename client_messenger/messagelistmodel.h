#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QDateTime>

enum class MessageItemType {
    Message,
    DateSeparator
};

struct MessageItem {
    MessageItemType type;

    QString text;
    QDateTime time;
    bool isFromSender;

    QString dateText;
};

class MessageListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TypeRole = Qt::UserRole + 1,
        TextRole,
        TimeRole,
        SenderRole,
        DateTextRole
    };

    explicit MessageListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setMessages(const QVector<MessageItem> &msgs);
    void addMessage(const MessageItem &msg);
    void clear();
    const MessageItem& getMessage(int row) const;

private:
    QVector<MessageItem> messages;
};
