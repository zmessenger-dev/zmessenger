#ifndef ZCHANNEL_H
#define ZCHANNEL_H

#include <QListWidgetItem>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextBrowser>

#include "zperson.h"

class ZChatInput;
class ZPersonDb;
class ZSettings;

enum ZChannelFlags
{
    TYPE_ADHOC   = 0x01,
    TYPE_CONSOLE = 0x02,
    TYPE_PUBLIC  = 0x04,
    TYPE_PM      = 0x08,
    MODE_ADS     = 0x10,
    MODE_CHAT    = 0x20,

    TYPE_ANY     = (TYPE_ADHOC | TYPE_CONSOLE | TYPE_PM | TYPE_PUBLIC)
};

enum ZAction
{
    ACTION_CHANNEL_DIALOG,
    ACTION_FULL_PROFILE,
    ACTION_INTERESTED,
    ACTION_JOIN,
    ACTION_NOT_INTERESTED,
    ACTION_PM,
    ACTION_RECENT,
};

/* A ZChannel handles I/O and state for one channel. */
class ZChannel: public QObject
{
    Q_OBJECT

public:
    ZChannel(ZPersonDb *, int, QString, QString = "", ZSettings * = nullptr);
    ~ZChannel();
    QWidget *getDisplayWidget() { return displayWidget; }
    QString getName() { return name; }
    QString getTitle() { return title; }
    int getType() { return flags & TYPE_ANY; }
    QListWidgetItem *getPanelItem() { return panelItem; }
    void setMode(int);
    int getNumUnread() { return numUnread; }
    void setPanelItem(QListWidgetItem *item) { panelItem = item; }
    void setIsCurrent(bool b) { isCurrent = b; numUnread = 0; }

    void initializeOpbase(QHash<QString, int> o) { opbase = QHash<QString, int>(o); }
    void initializeInfo(QList<ZChannelInfo *>);
    void pushMessage(QString);
    void pushAndLogMessage(QString);

    void addInfo(ZChannelInfo *);
    void deleteInfo(ZChannelInfo *);
    void removeInfo(ZChannelInfo *);
    void promoteInfo(ZChannelInfo *, QString);
    void demoteInfo(ZChannelInfo *, QString);

    void doContextMenu(QString);

signals:
    void closeChannel(ZChannel *);
    void runAction(ZAction, QString);
    void socketMessage(QString);
    void socketFakeInput(QString);
    void updateItemText(QListWidgetItem *, QString, int);

private slots:
    void onEnterPressed();
    void onCloseClicked();
    void onPostAdClicked();

    void onBlockerDialog();
    void onChannelDialog();

    void onContextFullProfile();
    void onContextInterested();
    void onContextNotInterested();
    void onContextPm();
    void onAnchorClicked(QUrl);
    void onBrowserContextMenu(const QPoint &);
    void onSidebarContextMenu(const QPoint &);

    void onTypingClear();
    void onTypingStartStop(bool);

private:
    QWidget *buildChannelBar();
    void parseCommand(QString);
    QListWidgetItem *itemFor(ZChannelInfo *);

    bool isCurrent;
    bool sendingAd;
    int flags;
    int numUnread;
    QString name;
    QString title;

    QString contextString;

    QFile *logFile;
    ZPersonDb *persondb;
    QList<ZChannelInfo *> channelPeople;
    QHash<QString, int> opbase;

    QPushButton *adButton;
    QListWidgetItem *panelItem;
    QTextBrowser *chatBrowser;
    ZChatInput *inputEdit;
    QListWidget *characterSidebar;

    QMenu *dialogMenu;
    ZSettings *settings;
    QMenu *globalMenu;

    QWidget *displayWidget;
};

#endif
