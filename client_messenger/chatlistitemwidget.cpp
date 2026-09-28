#include "chatlistitemwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

ChatListItemWidget::ChatListItemWidget(QWidget *parent)
    : QWidget(parent)
{
    avatar = new QLabel(this);
    avatar->setFixedSize(40, 40);
    avatar->setScaledContents(false);

    QPixmap pix(":/icons/default_avatar.png");
    avatar->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    titleLabel = new QLabel;
    titleLabel->setStyleSheet("font-weight: bold;");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    previewLabel = new QLabel;
    previewLabel->setStyleSheet("color: #777;");
    previewLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    previewLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    timeLabel = new QLabel;
    timeLabel->setStyleSheet("color: #999; font-size: 10px;");
    timeLabel->setAlignment(Qt::AlignRight | Qt::AlignTop);
    timeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->addWidget(titleLabel);
    textLayout->addWidget(previewLabel);
    textLayout->setSpacing(2);
    textLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(avatar, 0, Qt::AlignVCenter);
    mainLayout->addLayout(textLayout, 1);
    mainLayout->addWidget(timeLabel, 0, Qt::AlignTop);
    mainLayout->setContentsMargins(6, 8, 8, 8);
    mainLayout->setSpacing(8);
}



void ChatListItemWidget::setData(const QString &title, const QString &preview, const QString &time, QPixmap *avatar)
{
    titleLabel->setText(title);
    previewLabel->setText(preview);
    timeLabel->setText(time);
    if (avatar->isNull()){
        QPixmap pix(":/icons/default_avatar.png");
        this->avatar->setPixmap(pix.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        this->avatar->setPixmap(avatar->scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void ChatListItemWidget::setColor(const QString &color){
    if (color != "default"){
        this->previewLabel->setStyleSheet("color: #777; background-color:" + color +"; border-radius: 6px; padding: 6px;");
        return;
    }
    this->previewLabel->setStyleSheet("color: #777;");
}
