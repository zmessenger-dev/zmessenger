#ifndef ZMAINWINDOW_H
#define ZMAINWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>
#include <QTextEdit>
#include <QWebSocket>

class ZChannel;
class ZChannelDialog;
class ZPerson;
class ZPersonDb;
class ZSettings;

/* ZMainWindow handles session state and channel switching. */
class ZMainWindow: public QMainWindow
{
    Q_OBJECT

public:
    ZMainWindow();
    ~ZMainWindow();

    void setRecord(bool b) { record = b; }
    void setTesting(bool b) { testing = b; }
    bool start();

private slots:
    void onUpdateItemText(QListWidgetItem *, QString, int);
    void onTextMessageReceived(const QString &);
    void onPickerItemChanged(QListWidgetItem *, QListWidgetItem *);
    void onPickerWidgetClicked();
    void onSocketMessage(QString);
    void onSocketConnected();
    void onSocketDisconnected();
    void onCloseChannel(ZChannel *);
    void onAction(int, QString);
    void onRecentContextMenu();
    void onClose();
    void onPageDown();
    void onPageUp();

private:
    QString buildMessageFrom(ZPerson *, QString, int);
    QString buildPoseFrom(ZPerson *, QString);
    QString buildStampedMessage(QString);
    QString linkFor(ZPerson *);

    void openPm(QString);
    void startConnect(QString, QString);
    void setupChannel(ZChannel *, bool = false);
    void sendOnOffMessage(ZPerson *, bool);
    QString statusString(int);

    QWebSocket *socket;
    QString connectString;

    bool testing;
    bool record;
    int unreadTotal;
    QString recentName;
    QTextEdit *recentEdit;
    QListWidget *panelPicker;
    QStackedWidget *panelStackWidget;

    ZChannelDialog *channelDialog;

    ZSettings *settings;
    ZChannel *currentPanel;
    /* The console. Always available. */
    ZChannel *console;
    /* Public and adhoc channels. Kept apart to prevent name collisions */
    QHash<QString, ZChannel *> publicChannels;
    /* PM channels only. */
    QHash<QString, ZChannel *> privateChannels;
    /* Transforms 3-letter commands into enums to switch upon. */
    QHash<QString, int> commandMapping;
    /* People online, and flags for online and off. */
    ZPersonDb *persondb;
    /* Turns a gender string to an int. */
    QHash<QString, int> genderMapping;
    /* Status to an int. */
    QHash<QString, int> statusMapping;
    /* COL information, used in the next JCH. */
    QHash<QString, int> pendingOpbase;

    void processBanOrKick(QJsonObject, QString, QString);
    void processADL(QJsonObject);
    void processAOP(QJsonObject);
    void processBRO(QJsonObject);
    void processCBU(QJsonObject);
    void processCDS(QJsonObject);
    void processCHA(QJsonObject);
    void processCIU(QJsonObject);
    void processCKU(QJsonObject);
    void processCOA(QJsonObject);
    void processCOL(QJsonObject);
    void processCON(QJsonObject);
    void processCOR(QJsonObject);
    void processCTU(QJsonObject);
    void processDOP(QJsonObject);
    void processERR(QJsonObject);
    void processFLN(QJsonObject);
    void processFRL(QJsonObject);
    void processHLO(QJsonObject);
    void processICH(QJsonObject);
    void processIDN(QJsonObject);
    void processIGN(QJsonObject);
    void processJCH(QJsonObject);
    void processLCH(QJsonObject);
    void processLIS(QJsonObject);
    void processLRP(QJsonObject);
    void processMSG(QJsonObject);
    void processNLN(QJsonObject);
    void processORS(QJsonObject);
    void processPIN();
    void processPRI(QJsonObject);
    void processRMO(QJsonObject);
    void processRTB(QJsonObject);
    void processSTA(QJsonObject);
    void processSYS(QJsonObject);
    void processTPN(QJsonObject);
    void processVAR(QJsonObject);

private:
    ZChannel *findPrivateChannel(QString name) { return privateChannels.value(name, nullptr); }
    ZChannel *findPublicChannel(QString name) { return publicChannels.value(name, nullptr); }
};

#endif
