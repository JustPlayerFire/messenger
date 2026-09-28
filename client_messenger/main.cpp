#include "logindialog.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSettings settings("Messenger", "Client");

    if (!settings.value("nickname").toString().isEmpty()){
        MainWindow w(settings.value("id").toInt(), settings.value("nickname").toString(), settings.value("avatarBase64").toString());
        w.show();
        return a.exec();
    } else {
        LoginDialog login;
        if (login.exec() == QDialog::Accepted) {
            qDebug() << login.getId() << " " << login.getNickname();
            MainWindow w(login.getId(), login.getNickname(), login.getAvatarBase64(), login.client);
            login.close();
            w.show();
            return a.exec();
        }
    }

    return 0;
}
