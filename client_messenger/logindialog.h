#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "networkclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    int getId() const;
    QString getEmail() const;
    QString getNickname() const;
    QString getPassword() const;
    QString getAvatarBase64() const;
    NetworkClient *client;

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_registerConfirmButton_clicked();
    void on_backButton_clicked();
    void onLoginResult(bool ok, const QString &error, int &id, const QString &nickname, const QString &avatarBase64);
    void onRegisterResult(bool ok, const QString &error);

private:
    Ui::LoginDialog *ui;

    int id;
    QString email;
    QString nickname;
    QString password;
    QString avatarBase64;

    QTimer *reconnectTimer;

    void switchToRegister(bool regMode);
    bool validateInput(bool isRegister);


};

#endif // LOGINDIALOG_H
