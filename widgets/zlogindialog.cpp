#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>
#include <QMessageBox>

#include "zdefines.h"
#include "zlogindialog.h"

ZLoginDialog::ZLoginDialog():
    QDialog(),
    ticket(),
    character()
{
    netManager = new QNetworkAccessManager;
    QGridLayout *grid = new QGridLayout;

    QLabel *userLabel = new QLabel("Username:");
    userLine = new QLineEdit;

    QLabel *passLabel = new QLabel("Password:");
    passLine = new QLineEdit;
    passLine->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton(QIcon(":/images/tick.png"), "Login");
    connect(loginButton, SIGNAL(clicked()), this, SLOT(loginActivated()));

    QLabel *charLabel = new QLabel("Character:");
    charCombo = new QComboBox;
    charCombo->setEnabled(false);

    connButton = new QPushButton(QIcon(":/images/plug-connect.png"), "Connect");
    connect(connButton, SIGNAL(clicked()), this, SLOT(setCharacterAndFinish()));
    connButton->setEnabled(false);

    grid->addWidget(userLabel,   1, 0);
    grid->addWidget(userLine,    1, 1, 1, 2);
    grid->addWidget(passLabel,   2, 0);
    grid->addWidget(passLine,    2, 1, 1, 2);
    grid->addWidget(loginButton, 3, 1);
    grid->addWidget(charLabel,   4, 0);
    grid->addWidget(charCombo,   4, 1, 1, 2);
    grid->addWidget(connButton,  5, 1);

    connect(netManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(ticketReply(QNetworkReply *)));

    setLayout(grid);
}

void ZLoginDialog::setCharacterAndFinish()
{
    character = charCombo->currentText();
    account = userLine->text();
    accept();
}

void ZLoginDialog::ticketReply(QNetworkReply *reply)
{
    reply->deleteLater();
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject o = doc.object();
    bool netDown = reply->error() != QNetworkReply::NoError;
    QString error = o.value("error").toString();

    if (netDown || error.isEmpty() == false) {
        if (error.isEmpty() == false) {
            QMessageBox::warning(this, "Error", error);
        }
        userLine->setEnabled(true);
        passLine->setEnabled(true);
        loginButton->setEnabled(true);
        return;
    }

    charCombo->setEnabled(true);
    connButton->setEnabled(true);
    QJsonArray characterArray = o.value("characters").toArray();
    QString defaultCharacter = o.value("default_character").toString();
    int index = 0;

    for (int i = 0; i < characterArray.size(); i++) {
        QString s = characterArray.at(i).toString();
        if (s == defaultCharacter) {
            index = i;
        }
        charCombo->addItem(s);
    }

    charCombo->setCurrentIndex(index);
    charCombo->setFocus();
    ticket = o.value("ticket").toString();
}

void ZLoginDialog::loginActivated()
{
    QByteArray query = QByteArray("secure=yes");
    query += "&account=";
    query += QUrl::toPercentEncoding(userLine->text());
    query += "&password=";
    query += QUrl::toPercentEncoding(passLine->text());

    QNetworkRequest request(QUrl(SITE_BASE_HTTPS TICKET_URL));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    userLine->setEnabled(false);
    passLine->setEnabled(false);
    loginButton->setEnabled(false);
    netManager->post(request, query);
}
