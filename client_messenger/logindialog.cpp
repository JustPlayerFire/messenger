#include "logindialog.h"
#include "ui_logindialog.h"
#include "networkclient.h"
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    client = new NetworkClient(this);
    client->connectToServer("127.0.0.1", 5555);


    connect(client, &NetworkClient::loginResult,
            this, LoginDialog::onLoginResult);

    connect(client, &NetworkClient::registerResult,
            this, LoginDialog::onRegisterResult);


    qApp->setStyleSheet(
        "QDialog { background-color: #f2f2f2; }"
        "QLabel#titleLabel {"
            "font-size: 22px;"
            "font-weight: bold;"
            "color: #333;"
            "qproperty-alignment: 'AlignHCenter';"
            "margin-bottom: 20px;"
        "}"
        "QLineEdit {"
            "border: 1px solid #ccc;"
            "border-radius: 12px;"
            "padding: 8px 12px;"
            "background-color: white;"
            "font-size: 14px;"
        "}"
        "QLineEdit:focus {"
            "border: 2px solid #0088cc;"
        "}"
        "QPushButton {"
            "background-color: #0088cc;"
            "color: white;"
            "border-radius: 12px;"
            "padding: 8px 14px;"
            "font-weight: bold;"
            "font-size: 14px;"
        "}"
        "QPushButton:hover {"
            "background-color: #007ab8;"
        "}"
        "QPushButton:pressed {"
            "background-color: #006aa3;"
        "}"
    );


    setWindowTitle("Вход в мессенджер");

    switchToRegister(false);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}


void LoginDialog::switchToRegister(bool regMode)
{
    ui->nicknameLabel->setVisible(regMode);
    ui->nicknameLineEdit->setVisible(regMode);
    ui->passwordRepeatLabel->setVisible(regMode);
    ui->passwordRepeatLineEdit->setVisible(regMode);

    ui->loginButton->setVisible(!regMode);
    ui->registerButton->setVisible(!regMode);
    ui->registerConfirmButton->setVisible(regMode);
    ui->backButton->setVisible(regMode);

    ui->rememberCheckBox->setVisible(!regMode);
}


bool LoginDialog::validateInput(bool isRegister)
{
    email = ui->emailLineEdit->text().trimmed();
    password = ui->passwordLineEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля.");
        return false;
    }

    if (isRegister) {
        nickname = ui->nicknameLineEdit->text().trimmed();
        QString passwordRepeat = ui->passwordRepeatLineEdit->text();

        if (nickname.isEmpty() || passwordRepeat.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Заполните все поля.");
            return false;
        }

        if (password != passwordRepeat) {
            QMessageBox::warning(this, "Ошибка", "Пароли не совпадают.");
            return false;
        }
    }

    return true;
}


void LoginDialog::on_loginButton_clicked()
{
    if (!validateInput(false))
        return;

    client->loginUser(email, password);
}


void LoginDialog::on_registerButton_clicked()
{
    switchToRegister(true);
}

void LoginDialog::on_registerConfirmButton_clicked()
{
    if (!validateInput(true))
        return;

    client->registerUser(email, nickname, password);
}


void LoginDialog::on_backButton_clicked()
{
    switchToRegister(false);
}

int LoginDialog::getId() const { return id; }
QString LoginDialog::getEmail() const { return email; }
QString LoginDialog::getNickname() const { return nickname; }
QString LoginDialog::getPassword() const { return password; }
QString LoginDialog::getAvatarBase64() const { return avatarBase64; }


void LoginDialog::onLoginResult(bool ok, const QString &error, int &userId, const QString &nickname, const QString &avatarBase64)
{
    if (ok) {
        qDebug() << "Login OK, userId:" << userId;
        qDebug() << "nickname:" << nickname;
        this->id = userId;
        this->nickname = nickname;
        this->avatarBase64 = avatarBase64;
        if (ui->rememberCheckBox->isChecked()){
            QSettings settings("Messenger", "Client");
            settings.setValue("id", this->id);
            settings.setValue("nickname", this->nickname);
            settings.setValue("avatarBase64", this->avatarBase64);
        }
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка входа", error);
    }
}

void LoginDialog::onRegisterResult(bool ok, const QString &error)
{
    if (ok) {
        QMessageBox::information(this, "Успех", "Регистрация завершена");
        switchToRegister(false);
    } else {
        QMessageBox::warning(this, "Ошибка регистрации", error);
    }
}

