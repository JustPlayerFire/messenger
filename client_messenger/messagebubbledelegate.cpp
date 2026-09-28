#include "messagebubbledelegate.h"
#include "messagelistmodel.h"
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

MessageBubbleDelegate::MessageBubbleDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void MessageBubbleDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    int type = index.data(MessageListModel::TypeRole).toInt();

    if (type == static_cast<int>(MessageItemType::Message))
    {
        QString text = index.data(MessageListModel::TextRole).toString();
        QDateTime time = index.data(MessageListModel::TimeRole).toDateTime();
        bool isFromSender = index.data(MessageListModel::SenderRole).toBool();

        QString timeStr = time.toString("HH:mm");

        const int borderRadius = 12;
        const int maxBubbleWidth = 400;
        const int minBubbleWidth = 50;
        const int margin = 8;
        const int textMargin = 10;
        const int timeHeight = 16;
        const int spacing = 2;

        QFontMetrics fm(option.font);
        int textWidth = fm.boundingRect(QRect(0, 0, maxBubbleWidth - 2 * textMargin, 0),
                                        Qt::TextWordWrap, text).width();

        int bubbleWidth = qMax(textWidth + 2 * textMargin, minBubbleWidth);
        bubbleWidth = qMin(bubbleWidth, maxBubbleWidth);


        int textHeight = fm.boundingRect(QRect(0, 0, bubbleWidth - 2 * textMargin, 0),
                                         Qt::TextWordWrap, text).height();

        int bubbleHeight = textHeight + timeHeight + spacing + 2 * textMargin;


        int x = option.rect.left() + margin;
        if (isFromSender)
            x = option.rect.right() - bubbleWidth - margin;

        QRect bubbleRect(x, option.rect.top() + margin, bubbleWidth, bubbleHeight);


        QColor bubbleColor = isFromSender ? QColor("#87CEFA") : QColor("#E0FFFF");
        painter->setBrush(bubbleColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(bubbleRect, borderRadius, borderRadius);


        QRect textRect(bubbleRect.left() + textMargin,
                       bubbleRect.top() + textMargin,
                       bubbleWidth - 2 * textMargin,
                       textHeight);
        painter->setPen(Qt::black);
        painter->drawText(textRect, Qt::TextWordWrap, text);


        QRect timeRect(bubbleRect.left() + textMargin,
                       bubbleRect.bottom() - textMargin - timeHeight,
                       bubbleWidth - 2 * textMargin,
                       timeHeight);
        painter->setPen(isFromSender ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 120));
        painter->drawText(timeRect, Qt::AlignRight | Qt::AlignVCenter, timeStr);
    }
    else if (type == static_cast<int>(MessageItemType::DateSeparator))
    {
        QString dateText = index.data(MessageListModel::DateTextRole).toString();


        QFontMetrics fm(option.font);
        int textWidth = fm.width(dateText) + 20;
        int textHeight = fm.height() + 6;

        int x = option.rect.left() + (option.rect.width() - textWidth) / 2;
        int y = option.rect.top() + (option.rect.height() - textHeight) / 2;

        QRect separatorRect(x, y, textWidth, textHeight);


        painter->setBrush(QColor("#e0e0e0"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(separatorRect, 10, 10);

        painter->setPen(Qt::darkGray);
        painter->drawText(separatorRect, Qt::AlignCenter, dateText);
    }

    painter->restore();
}

QSize MessageBubbleDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    int type = index.data(MessageListModel::TypeRole).toInt();

    if (type == static_cast<int>(MessageItemType::Message))
    {
        QString text = index.data(MessageListModel::TextRole).toString();
        QFontMetrics fm(option.font);

        const int maxBubbleWidth = 400;
        const int minBubbleWidth = 80;
        const int textMargin = 10;
        const int timeHeight = 16;
        const int spacing = 2;
        const int margin = 8;

        int textWidth = fm.boundingRect(QRect(0, 0, maxBubbleWidth - 2 * textMargin, 0),
                                        Qt::TextWordWrap, text).width();

        int bubbleWidth = qMax(textWidth + 2 * textMargin, minBubbleWidth);
        bubbleWidth = qMin(bubbleWidth, maxBubbleWidth);

        int textHeight = fm.boundingRect(QRect(0, 0, bubbleWidth - 2 * textMargin, 0),
                                         Qt::TextWordWrap, text).height();
        int bubbleHeight = textHeight + timeHeight + spacing + 2 * textMargin + 2 * margin;

        return QSize(option.rect.width(), bubbleHeight);
    }
    else
    {
        return QSize(option.rect.width(), 30);
    }
}
