#include "profiledialog.h"
#include "ui_profiledialog.h"
#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QBuffer>

ProfileDialog::ProfileDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProfileDialog)
{
    ui->setupUi(this);
    setWindowTitle("Профиль");

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProfileDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ProfileDialog::reject);

    ui->avatarLabel->setStyleSheet(R"(
          QLabel {
             border: 2px solid #0088cc;
          }
    )");
}

ProfileDialog::~ProfileDialog()
{
    delete ui;
}

void ProfileDialog::setProfileData(const QString &nickname, const QString &status, const QString &description, const QString &avatarBase64)
{
    ui->nicknameLineEdit->setText(nickname);
    ui->statusLineEdit->setText(status);
    ui->descriptionTextEdit->setPlainText(description);

    this->avatarBase64 = avatarBase64.isEmpty() ? ":/icons/default_avatar.png" : avatarBase64;
    if (!avatarBase64.isEmpty()){
        QByteArray avatarData = QByteArray::fromBase64(avatarBase64.toUtf8());
        QPixmap avatar;
        avatar.loadFromData(avatarData, "PNG");
        ui->avatarLabel->setPixmap(avatar.scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else{
        ui->avatarLabel->setPixmap(QPixmap(":icons/default_avatar.png").scaled(40,40,Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    this->nickname = nickname;
    this->status = status;
    this->description = description;
    this->avatarBase64 = avatarBase64;

}

QVector<QString> ProfileDialog::getProfileData()
{
    this->nickname = ui->nicknameLineEdit->text();
    this->status = ui->statusLineEdit->text();
    this->description = ui->descriptionTextEdit->toPlainText();
    return QVector<QString>{nickname, status, description, avatarBase64};
}

void ProfileDialog::on_changeAvatarButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Выберите аватар", "", "Images (*.png *.jpg *.jpeg)");
    if (!file.isEmpty()) {
        QString avatarPath = file;
        QPixmap pix(avatarPath);
        ui->avatarLabel->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QPixmap pixmap(avatarPath);
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");

        this->avatarBase64 = QString(byteArray.toBase64());;
    }
}


void ProfileDialog::setReadOnly(bool readOnly)
{
    ui->nicknameLineEdit->setReadOnly(readOnly);
    ui->statusLineEdit->setReadOnly(readOnly);
    ui->descriptionTextEdit->setReadOnly(readOnly);

    ui->changeAvatarButton->setVisible(!readOnly);
    ui->buttonBox->setVisible(!readOnly);
    ui->labelInfo->setVisible(!readOnly);

}
