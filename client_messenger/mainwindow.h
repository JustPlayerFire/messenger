#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QDate>
#include <QMap>
#include <QListWidgetItem>
#include <QTimer>
#include "networkclient.h"
#include "messagelistmodel.h"
#include <QPixmap>


struct Message {
    QString text;
    QDateTime time;
    bool isFromSender;
};

struct Chat {
    QString chatId;
    QString name;
    bool isGroup;
    QPixmap avatar;
};

struct UserProfile {
    QString id;
    QString nickname;
    QString status;
    QString description;
    QString avatarBase64;
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int id, QString nickname, QString avatarBase64, NetworkClient *cl = nullptr, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *) override;

private slots:
    void on_sendMessageButton_clicked();
    void on_addChatButton_clicked();
    void onChatSelected(QListWidgetItem *item);
    void loadChat(const QString &chatId);
    void updateChatPreview(const QString &chatId);
    void createChat(const QString &id, const QString &nickname, QVector<Message> *messages = nullptr, const QString &avatarBase64 = "");
    bool eventFilter(QObject *obj, QEvent *event) override;
    void onMessageReceived(const QJsonObject &msg);
    void onMessageSent(const QJsonObject &msg);
    void onGotChats(const QJsonObject &chats);
    void onGotChatMessages(const QJsonObject &msgs);
    void onGotProfileInfo(const QJsonObject &array);
    void onTrayMessageClicked();
    void on_exitButton_clicked();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMap<QString, QVector<Message>> chatMessages;
    QMap<QString, Chat> chats;
    QString currentChatId;
    QString notificationChatId;
    UserProfile currentUser;
    NetworkClient *client;
    MessageListModel *messageModel;
    bool isExiting;
    bool isMuted;
};

#endif // MAINWINDOW_H
