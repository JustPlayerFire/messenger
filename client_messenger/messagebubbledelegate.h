#ifndef MESSAGEBUBBLEDELEGATE_H
#define MESSAGEBUBBLEDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QDateTime>

class MessageBubbleDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageBubbleDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
};

#endif // MESSAGEBUBBLEDELEGATE_H
