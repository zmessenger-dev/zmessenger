#ifndef ZLOGINDIALOG_H
#define ZLOGINDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ZLoginDialog: public QDialog
{
    Q_OBJECT

public:
    ZLoginDialog();
    QString getAccount() { return account; }
    QString getTicket() { return ticket; }
    QString getCharacter() { return character; }

private slots:
    void loginActivated();
    void setCharacterAndFinish();
    void ticketReply(QNetworkReply *);

private:
    QString ticket;
    QString account;
    QString character;
    QNetworkAccessManager *netManager;
    QLineEdit *userLine;
    QLineEdit *passLine;
    QComboBox *charCombo;
    QPushButton *connButton;
    QPushButton *loginButton;
};

#endif
