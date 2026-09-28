#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScrollBar>
#include <QKeyEvent>
#include <QInputDialog>
#include <QUuid>
#include <chatlistitemwidget.h>
#include "profiledialog.h"
#include "networkclient.h"
#include "logindialog.h"
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>
#include <QTime>
#include <QTimer>
#include "messagelistmodel.h"
#include "messagebubbledelegate.h"
#include <QSettings>

MainWindow::MainWindow(int id, QString nickname, QString avatarBase64, NetworkClient *cl, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    if (cl != nullptr){
        client = cl;
    } else {
        client = new NetworkClient(this);
        client->connectToServer("127.0.0.1", 5555);
    }
    setWindowTitle("Мессенджер");

    isExiting = false;
    isMuted = false;

    connect(client, &NetworkClient::messageReceived,
            this, &MainWindow::onMessageReceived);

    connect(client, &NetworkClient::messageSent,
            this, &MainWindow::onMessageSent);

    connect(client, &NetworkClient::checkUserResult, this, [this](const QString &id, const QString &nickname){
        QVector<Message> emptyMessages;
        createChat(id, nickname, &emptyMessages);
    });

    connect(client, &NetworkClient::errorOccurred, this, [](const QString &err){
        qDebug() << "Network error:" << err;
    });
    connect(client, &NetworkClient::gotProfileInfo, this, &MainWindow::onGotProfileInfo);

    ui->chatListWidget->setStyleSheet(R"(
        QListWidget {
            background-color: #ffffff;
            border: none;
            outline: 0;
        }

        QListWidget::item {
            background-color: transparent;
            border: none;
        }

        QListWidget::item:hover {
            background-color: #f5f5f5;
        }

        QListWidget::item:selected {
            background-color: #e6f0fa;
        }

        QListWidget::item:selected:hover {
            background-color: #d9e8f5;
        }
    )");


    qApp->setStyleSheet(R"(
        QMainWindow {
            background-color: #f2f2f2;
        }

        QScrollArea {
            border: none;
            background: transparent;
        }

        QLineEdit {
            border: 1px solid #ccc;
            border-radius: 12px;
            padding: 6px 10px;
        }

        QPushButton {
            background-color: #0088cc;
            color: white;
            border-radius: 12px;
            padding: 6px 14px;
            font-weight: bold;
        }

        QPushButton:hover {
            background-color: #007ab8;
        }

        QPushButton:pressed {
            background-color: #006aa3;
        }
    )");

    ui->ownerNicknameLabel->setStyleSheet(R"(
        font-size: 14pt;
        font-weight: bold;
        color: #333333;
        padding-left: 4px;
    )");

    ui->nicknameLabel->setStyleSheet(R"(
        font-size: 13pt;
        font-weight: 600;
        color: #222222;
        padding-left: 4px;
    )");

    ui->ownerAvatarLabel->setStyleSheet(R"(
         QLabel {
             border: 2px solid #0088cc;
         }
    )");

    ui->avatarLabel->setStyleSheet(R"(
          QLabel {
             border: 2px solid #0088cc;
          }
    )");

    messageModel = new MessageListModel(this);

    ui->chatListView->setModel(messageModel);
    ui->chatListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->chatListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->chatListView->setResizeMode(QListView::Adjust);
    ui->chatListView->setWordWrap(true);
    ui->chatListView->setSpacing(6);

    MessageBubbleDelegate *delegate = new MessageBubbleDelegate(this);
    ui->chatListView->setItemDelegate(delegate);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icons/message.png"));
    trayIcon->show();
    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, MainWindow::onTrayMessageClicked);

    connect(trayIcon, &QSystemTrayIcon::activated,
            this, [this](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger)
        {
            this->show();
            this->raise();
            this->activateWindow();

        }
    });


    QMenu *menu = new QMenu(this);

    QAction *openAction = new QAction("Открыть", this);
    QAction *quitAction = new QAction("Закрыть приложение", this);
    QAction *muteAction = new QAction("Выключить уведомления", this);
    muteAction->setCheckable(true);

    connect(openAction, &QAction::triggered, this, &MainWindow::show);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    connect(muteAction, &QAction::toggled, this, [this](bool checked){
        isMuted = checked;
    });

    menu->addAction(openAction);

    menu->addAction(muteAction);
    menu->addAction(quitAction);

    trayIcon->setContextMenu(menu);


    connect(ui->chatListWidget,&QListWidget::itemClicked,this,&MainWindow::onChatSelected);

    currentUser.id = QString::number(id);
    currentUser.nickname = nickname;
    currentUser.status = "Онлайн";
    currentUser.description = "Описание профиля";
    currentUser.avatarBase64 = avatarBase64;

    ui->ownerNicknameLabel->setText(currentUser.nickname);
    if (!avatarBase64.isEmpty()){
        QByteArray avatarData = QByteArray::fromBase64(avatarBase64.toUtf8());
        QPixmap avatar;
        avatar.loadFromData(avatarData, "PNG");
        ui->ownerAvatarLabel->setPixmap(avatar.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else{
        ui->ownerAvatarLabel->setPixmap(QPixmap(":icons/default_avatar.png").scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    ui->chatHeaderWidget->installEventFilter(this);
    ui->ownerProfileWidget->installEventFilter(this);

    connect(client, &NetworkClient::gotChats, this, &MainWindow::onGotChats);
    connect(client, &NetworkClient::gotChatMessages, this, &MainWindow::onGotChatMessages);

    client->requestChats(currentUser.id);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_sendMessageButton_clicked()
{

    client->sendMessage(currentUser.id.toInt(), currentChatId.toInt(), ui->messageLineEdit->text());

}


void MainWindow::onMessageSent(const QJsonObject &msg)
{
    QString txt = ui->messageLineEdit->text().trimmed();
    if (txt.isEmpty()){
        return;
    }

    QDateTime msgTime = QDateTime::currentDateTime();

    Message m;
    m.text = txt;
    m.time = msgTime;
    m.isFromSender = true;

    chatMessages[currentChatId].append(m);


    MessageItem item;
    item.type = MessageItemType::Message;
    item.text = m.text;
    item.time = m.time;
    item.isFromSender = m.isFromSender;
    messageModel->addMessage(item);

    ui->chatListView->scrollToBottom();

    updateChatPreview(currentChatId);

    ui->messageLineEdit->clear();
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
            on_sendMessageButton_clicked();
            event->accept();
            return;
    }
}


void MainWindow::on_addChatButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(
        this,
        "Новый чат",
        "Название чата:",
        QLineEdit::Normal,
        "",
        &ok
    );

    if (!ok || name.trimmed().isEmpty()){
        return;
    }

    client->checkUserAndCreateChat(name);
}


void MainWindow::createChat(const QString &id, const QString &nickname, QVector<Message> *messages, const QString &avatarBase64) {
    if (chats.contains(id)) {
            return;
        }
    if (id == "-1") {
        QMessageBox::warning(this, "Ошибка", "Неправильный никнейм или пользователь не найден.");
        return;
    } else if (currentUser.id == id) {
        QMessageBox::warning(this, "Ошибка", "Нельзя начать чат с самим собой.");
        return;
    }

    Chat newChat;
    newChat.chatId = id;
    newChat.isGroup = false;
    newChat.name = nickname;
    QByteArray avatarData = QByteArray::fromBase64(avatarBase64.trimmed().toUtf8());
    QPixmap avatar;
    avatar.loadFromData(avatarData, "PNG");
    newChat.avatar = avatar;

    chats[newChat.chatId] = newChat;


    QListWidgetItem *item = new QListWidgetItem;
    item->setData(Qt::UserRole, newChat.chatId);
    item->setSizeHint(QSize(0, 64));


    ChatListItemWidget *widget = new ChatListItemWidget;
    QString preview = messages && !messages->isEmpty() ? messages->last().text : "";
    QString timeStr = messages && !messages->isEmpty() ? messages->last().time.toString("dd-MM-yyyy hh:mm") : "";
    widget->setData(newChat.name, preview, timeStr, &avatar);

    ui->chatListWidget->addItem(item);
    ui->chatListWidget->setItemWidget(item, widget);
}


void MainWindow::loadChat(const QString &chatId)
{
    messageModel->clear();

    auto &messages = chatMessages[chatId];
    std::sort(messages.begin(), messages.end(),
              [](const Message &a, const Message &b) {
                  return a.time < b.time;
              });

    for (const Message &m : messages) {
        MessageItem item;
        item.type = MessageItemType::Message;
        item.text = m.text;
        item.time = m.time;
        item.isFromSender = m.isFromSender;
        messageModel->addMessage(item);
    }


    QTimer::singleShot(100, this, [this]() {
        ui->chatListView->scrollToBottom();
    });
}


void MainWindow::onChatSelected(QListWidgetItem *item)
{
    currentChatId = item->data(Qt::UserRole).toString();

    for (int i = 0; i < ui->chatListWidget->count(); ++i) {
        QListWidgetItem *item = ui->chatListWidget->item(i);
        item->setSizeHint(QSize(0, 64));
        if (item->data(Qt::UserRole).toString() == currentChatId) {

            auto *widget = qobject_cast<ChatListItemWidget*>(
                ui->chatListWidget->itemWidget(item)
            );

            widget->setColor();

            break;
        }
    }

    ui->nicknameLabel->setText(chats[currentChatId].name);
    messageModel->clear();

    if (!chats[currentChatId].avatar.isNull()) {

        ui->avatarLabel->setPixmap(
            chats[currentChatId].avatar.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    } else {
        ui->avatarLabel->setPixmap(
            QPixmap(":icons/default_avatar.png").scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
        );
    }

    client->requestChatMessages(currentUser.id, currentChatId);
}



void MainWindow::updateChatPreview(const QString &chatId)
{
    const auto &msgs = chatMessages[chatId];
    if (msgs.isEmpty()){
        return;
    }
    const Message &last = msgs.last();

    QString preview = last.text;
    if (preview.length() > 30)
        preview = preview.left(30) + "...";

    QString time = last.time.toString("dd-MM-yyyy hh:mm");

    for (int i = 0; i < ui->chatListWidget->count(); ++i) {
        QListWidgetItem *item = ui->chatListWidget->item(i);

        if (item->data(Qt::UserRole).toString() == chatId) {

            auto *widget = qobject_cast<ChatListItemWidget*>(
                ui->chatListWidget->itemWidget(item)
            );

            if (widget) {
                widget->setData(
                    chats[chatId].name,
                    preview,
                    time,
                    &chats[chatId].avatar
                );
            }

            break;
        }
    }
}



bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->chatHeaderWidget && event->type() == QEvent::MouseButtonRelease) {
        client->getProfileInfo(currentChatId, 0);
        return true;

    }
    if (obj == ui->ownerProfileWidget && event->type() == QEvent::MouseButtonRelease) {
        client->getProfileInfo(currentUser.id, 1);
        return true;
    }

    return QMainWindow::eventFilter(obj, event);
}


void MainWindow::onMessageReceived(const QJsonObject &msg)
{
    if (msg["type"] != "recieveMessage"){
        return;
    }

    int senderId = msg["sender_id"].toInt();
    QString text = msg["text"].toString();
    QString chatId = QString::number(senderId);

    QDateTime msgTime = QDateTime::currentDateTime();

    Message m;
    m.text = text;
    m.time = msgTime;
    m.isFromSender = false;
    if (!chats.contains(chatId)) {
        QVector<Message> messages;
        messages.append(m);
        createChat(chatId, msg["sender_nickname"].toString(), &messages);
        chatMessages[chatId] = messages;
    } else {
        chatMessages[chatId].append(m);
    }

    updateChatPreview(chatId);

    if (currentChatId == chatId) {
        MessageItem item;
        item.type = MessageItemType::Message;
        item.text = m.text;
        item.time = m.time;
        item.isFromSender = m.isFromSender;
        messageModel->addMessage(item);
        ui->chatListView->scrollToBottom();
    }

    if (!this->isVisible() || this->isMinimized()){
        trayIcon->setIcon(QIcon(":icons/message_new.png"));
        if (!isMuted){
            trayIcon->showMessage(chats[chatId].name, m.text, QSystemTrayIcon::Information, 4000);
        }

    }
    for (int i = 0; i < ui->chatListWidget->count(); ++i) {
        QListWidgetItem *item = ui->chatListWidget->item(i);

        if (item->data(Qt::UserRole).toString() == chatId) {
            auto *widget = qobject_cast<ChatListItemWidget*>(
                ui->chatListWidget->itemWidget(item)
            );
            if (currentChatId != chatId){
                widget->setColor("#e3b2ac");
            }
            break;
        }
    }
}


void MainWindow::onGotChats(const QJsonObject &chats) {

    if (chats["status"] == "ok") {
        QJsonArray chatsArray = chats["chats"].toArray();
        for (int i = 0; i < chatsArray.size(); ++i) {
            QJsonObject chatObj = chatsArray[i].toObject();
            QString otherUserId = QString::number(chatObj["other_user_id"].toInt());
            QString lastText = chatObj["last_message_text"].toString();
            QString nick = chatObj["other_user_nickname"].toString();
            QDateTime lastTime = QDateTime::fromString(chatObj["last_message_timestamp"].toString(), Qt::ISODate);
            QString avatarBase64 = chatObj["other_user_avatar"].toString();

            QVector<Message> messages;
            Message lm;
            lm.text = lastText;
            lm.time = lastTime;
            messages.append(lm);

            createChat(otherUserId, nick, &messages, avatarBase64);
        }
    }
}


void MainWindow::onGotChatMessages(const QJsonObject &msgs) {
    if (msgs["status"].toString() == "ok") {
        QJsonArray messagesArray = msgs["messages"].toArray();
        QVector<Message> ms = {};
        for (int i = 0 ; i < messagesArray.size(); i++){
            QJsonObject mObj = messagesArray[i].toObject();
            Message m;
            m.text = mObj["text"].toString();
            m.time = QDateTime::fromString(mObj["timestamp"].toString(), Qt::ISODate);
            m.isFromSender = (QString::number(mObj["sender_id"].toInt()) == currentUser.id);
            ms.append(m);
        }
        chatMessages[msgs["receiver_id"].toString()] = ms;
        loadChat(msgs["receiver_id"].toString());
    }
}

void MainWindow::onGotProfileInfo(const QJsonObject &array)
{

    if (array["status"].toString() == "ok") {

        QJsonArray info = array["info"].toArray();

        ProfileDialog dlg(this);

        dlg.setProfileData(
            info.at(0).toString(),
            info.at(1).toString(),
            info.at(2).toString(),
            info.at(3).toString()
        );

        if (array["isForClient"].toBool()) {
            dlg.setReadOnly(false);
        } else {
            dlg.setReadOnly(true);
            chats[currentChatId].name = info.at(0).toString();
            ui->nicknameLabel->setText(info.at(0).toString());
            if (!info.at(3).toString().isEmpty()){
                QByteArray avatarData = QByteArray::fromBase64(info.at(3).toString().toUtf8());
                QPixmap avatar;
                avatar.loadFromData(avatarData, "PNG");
                chats[currentChatId].avatar = avatar;
                ui->avatarLabel->setPixmap(
                    avatar.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
            }else {
                ui->avatarLabel->setPixmap(
                    QPixmap(":icons/default_avatar.png").scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
            }
            updateChatPreview(currentChatId);
        }

        if (dlg.exec() == QDialog::Accepted) {

            QVector<QString> res = dlg.getProfileData();

            ui->ownerNicknameLabel->setText(res[0]);

            if (!res[3].isEmpty()) {
                QByteArray avatarData = QByteArray::fromBase64(res[3].toUtf8());
                QPixmap avatar;
                avatar.loadFromData(avatarData, "PNG");

                ui->ownerAvatarLabel->setPixmap(
                    avatar.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
            } else {
                ui->ownerAvatarLabel->setPixmap(
                    QPixmap(":icons/default_avatar.png").scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation)
                );
            }

            currentUser.nickname = res[0];
            currentUser.status = res[1];
            currentUser.description = res[2];
            currentUser.avatarBase64 = res[3];

            QSettings settings("Messenger", "Client");
            if (settings.value("nickname").isValid()){
                settings.setValue("nickname", currentUser.nickname);
                settings.setValue("avatarBase64", currentUser.avatarBase64);
            }

            client->changeProfile(
                currentUser.id,
                currentUser.nickname,
                currentUser.status,
                currentUser.description,
                currentUser.avatarBase64
            );
        }
    }
}



void MainWindow::on_exitButton_clicked()
{
    isExiting = true;
    QSettings settings("Messenger", "Client");
    settings.clear();
    settings.sync();
    client->disconnect();
    this->close();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (isExiting) {
        event->accept();
    } else {
        this->hide();
        event->ignore();
    }
}


void MainWindow::onTrayMessageClicked()
{
    this->show();
    this->raise();
    this->activateWindow();

    trayIcon->setIcon(QIcon(":icons/message.png"));
}
