#ifndef CHATLISTITEMWIDGET_H
#define CHATLISTITEMWIDGET_H

#include <QWidget>
#include <QLabel>

class ChatListItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatListItemWidget(QWidget *parent = nullptr);

    void setData(const QString &title, const QString &preview, const QString &time, QPixmap *avatar = nullptr);
    void setColor(const QString &color = "default");

private:
    QLabel *avatar;
    QLabel *titleLabel;
    QLabel *previewLabel;
    QLabel *timeLabel;
};

#endif
