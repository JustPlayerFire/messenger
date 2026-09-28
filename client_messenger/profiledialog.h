#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class ProfileDialog; }
QT_END_NAMESPACE

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(QWidget *parent = nullptr);
    ~ProfileDialog();

    void setProfileData(const QString &nickname, const QString &status, const QString &description, const QString &avatarBase64 = QString());
    QVector<QString>  getProfileData();
    void setReadOnly(bool readOnly);

private slots:
    void on_changeAvatarButton_clicked();


private:
    Ui::ProfileDialog *ui;
    QString id;
    QString nickname;
    QString status;
    QString description;
    QString avatarBase64;
};

#endif // PROFILEDIALOG_H
