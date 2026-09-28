#include "messagelistmodel.h"

MessageListModel::MessageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }
    return messages.size();
}

QVariant MessageListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= messages.size())
        return QVariant();

    const MessageItem &item = messages[index.row()];

    switch (role) {
    case TypeRole:
        return static_cast<int>(item.type);
    case TextRole:
        return item.text;
    case TimeRole:
        return item.time;
    case SenderRole:
        return item.isFromSender;
    case DateTextRole:
        return item.dateText;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MessageListModel::roleNames() const
{
    return {
        {TypeRole, "type"},
        {TextRole, "text"},
        {TimeRole, "time"},
        {SenderRole, "isSender"},
        {DateTextRole, "dateText"}
    };
}

void MessageListModel::setMessages(const QVector<MessageItem> &msgs)
{
    beginResetModel();
    messages = msgs;
    endResetModel();
}

void MessageListModel::addMessage(const MessageItem &msg)
{
    bool needSeparator = false;

    if (messages.isEmpty()) {
        needSeparator = true;
    } else {
        QDate lastMsgDate;
        for (int i = messages.size() - 1; i >= 0; --i) {
            if (messages[i].type == MessageItemType::Message) {
                lastMsgDate = messages[i].time.date();
                break;
            }
        }
        if (lastMsgDate.isValid() && lastMsgDate != msg.time.date())
            needSeparator = true;
    }

    if (needSeparator) {
        MessageItem sep;
        sep.type = MessageItemType::DateSeparator;
        QDate d = msg.time.date();
        if (d == QDate::currentDate())
            sep.dateText = "Сегодня";
        else if (d == QDate::currentDate().addDays(-1))
            sep.dateText = "Вчера";
        else
            sep.dateText = d.toString("dd.MM.yyyy");

        beginInsertRows(QModelIndex(), messages.size(), messages.size());
        messages.append(sep);
        endInsertRows();
    }

    beginInsertRows(QModelIndex(), messages.size(), messages.size());
    messages.append(msg);
    endInsertRows();
}

void MessageListModel::clear()
{
    beginResetModel();
    messages.clear();
    endResetModel();
}

const MessageItem& MessageListModel::getMessage(int row) const
{
    return messages[row];
}
