#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QMenu>
#include <QSound>
#include <QSplitter>
#include <QVBoxLayout>

#include "widgets/zblockerdialog.h"
#include "widgets/zchatinput.h"

#include "zcache.h"
#include "zchannel.h"
#include "zpersondb.h"

#define MAKE_2(command, field1, value1, field2, value2) \
QString("%1 {\"%2\":\"%3\",\"%4\":\"%5\"}") \
.arg(command, field1, value1, field2, value2)

ZChannel::ZChannel(ZPersonDb *db, int flags_, QString name_, QString title_,
    ZSettings *settings_):

    flags(flags_),
    name(name_)
{
    panelItem = nullptr;
    channelPeople = QList<ZChannelInfo *>();
    logFile = nullptr;
    persondb = db;
    opbase = QHash<QString, int>();
    isCurrent = false;
    sendingAd = false;
    dialogMenu = nullptr;
    settings = settings_;
    numUnread = 0;

    if (title_.isEmpty())
        title = name_;
    else
        title = title_;

    QSplitter *split = new QSplitter(Qt::Vertical);
    QVBoxLayout *chatLayout = new QVBoxLayout;

    chatBrowser = new QTextBrowser;
    chatBrowser->setOpenLinks(false);
    chatBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    chatBrowser->setProperty("widgetname", "chat");

    connect(chatBrowser, &QWidget::customContextMenuRequested,
    this, &ZChannel::onBrowserContextMenu);
    connect(chatBrowser, &QTextBrowser::anchorClicked,
    this, &ZChannel::onAnchorClicked);

    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->addWidget(chatBrowser);
    chatLayout->addWidget(buildChannelBar());

    QWidget *w = new QWidget;
    w->setLayout(chatLayout);

    split->addWidget(w);

    int type = flags & TYPE_ANY;

    inputEdit = new ZChatInput;
    inputEdit->setProperty("widgetname", "input");
    inputEdit->setMaximumHeight(75);

    connect(inputEdit, &ZChatInput::enterPressed,
    this, &ZChannel::onEnterPressed);

    if (type == TYPE_PM) {
        inputEdit->startTimer();

        connect(inputEdit, &ZChatInput::typingStartStop,
        this, &ZChannel::onTypingStartStop);

        connect(inputEdit, &ZChatInput::typingClear,
        this, &ZChannel::onTypingClear);
    }

    split->addWidget(inputEdit);

    QSplitter *topSplit = new QSplitter;
    topSplit->addWidget(split);

    /* Always make the sidebar, so the layout is consistent. */
    characterSidebar = new QListWidget;
    characterSidebar->setContextMenuPolicy(Qt::CustomContextMenu);
    characterSidebar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    characterSidebar->setProperty("widgetname", "sidebar");
    characterSidebar->setMaximumWidth(200);
    topSplit->addWidget(characterSidebar);

    if (type == TYPE_PUBLIC || type == TYPE_ADHOC) {
        connect(characterSidebar, &QWidget::customContextMenuRequested,
        this, &ZChannel::onSidebarContextMenu);
    } else {
        if (type == TYPE_PM) {
            QString logPath = QString("logs/%1_%2.txt")
                .arg(db->getSelfName())
                .arg(name);

            QString fullPath = QString("%1/%2")
                .arg(QApplication::applicationDirPath())
                .arg(logPath);

            logFile = new QFile(fullPath);
            /* Unbuffered prevents logs buffering in case of force quit/crash. */
            if (logFile->open(QIODevice::Unbuffered | QIODevice::Append | QIODevice::Text) == false) {
                chatBrowser->append("Warning: Unable to create log file.");
                logFile = nullptr;
            }
            else {
                QFile f(fullPath);
                if (f.open(QIODevice::ReadOnly | QIODevice::Text) == true) {
                    QByteArray ba = f.readAll();
                    chatBrowser->setHtml(ba);
                    f.close();
                }

                QDate d = QDate::currentDate();
                QString m = QString("ZMessenger Log for %1 on %2.<br>\n")
                    .arg(name)
                    .arg(d.toString("dddd, MMMM d, yyyy"));

                logFile->write(m.toUtf8());
            }
        }
    }

    displayWidget = topSplit;
}

ZChannel::~ZChannel()
{
    if (logFile) {
        logFile->close();
        delete logFile;
    }

    foreach (ZChannelInfo *info, channelPeople) {
        delete info;
    }
}

/* ---Misc--- */

void ZChannel::pushMessage(QString message)
{
    chatBrowser->append(message);
    int type = getType();

    if ((type & (TYPE_ADHOC | TYPE_PUBLIC)) &&
        chatBrowser->document()->blockCount() > 75) {

        /* The cursor starts at the beginning of the document. */
        QTextCursor cursor(chatBrowser->document());

        /* For these channels, the first post (should) always be the intro.
         * Don't delete it, because it may have channel rules/helpful tips.
         * Instead, delete the first block after it. */
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);

        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
    else if (type == TYPE_PM && isCurrent == false) {
        numUnread++;
        QString title = QString("%1 (%2)").arg(name).arg(numUnread);
        emit updateItemText(panelItem, title, 1);
    }
}

void ZChannel::pushAndLogMessage(QString message)
{
    if (logFile) {
        logFile->write(message.toUtf8());
        logFile->write("<br>\n");
    }

    pushMessage(message);
}

QWidget *ZChannel::buildChannelBar()
{
    int type = getType();
    QWidget *bar = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    adButton = new QPushButton("Post as ad");
    QPushButton *closeButton = new QPushButton(QIcon(":/img/close.png"), "");

    layout->addWidget(adButton);

    if (type == TYPE_CONSOLE) {
        QPushButton *dialogButton = new QPushButton(QIcon(":/img/dialogs.png"), "");
        layout->addWidget(dialogButton);

        dialogMenu = new QMenu;
        dialogMenu->addAction("Dialogs")->setEnabled(false);
        dialogMenu->addSeparator();
        dialogMenu->addAction("Channels",
        this, &ZChannel::onChannelDialog);

        dialogButton->setMenu(dialogMenu);

        QPushButton *globalButton = new QPushButton(QIcon(":/img/prefs-global.png"), "");
        layout->addWidget(globalButton);

        globalMenu = new QMenu;
        globalMenu->addAction("Global Preferences")->setEnabled(false);
        globalMenu->addSeparator();
        globalMenu->addAction("Channels",
        this, &ZChannel::onBlockerDialog);

        globalButton->setMenu(globalMenu);

        closeButton->setEnabled(false);
    }

    layout->addStretch();
    layout->addWidget(closeButton);

    connect(adButton, &QAbstractButton::clicked,
    this, &ZChannel::onPostAdClicked);
    connect(closeButton, &QAbstractButton::clicked,
    this, &ZChannel::onCloseClicked);

    if ((flags & MODE_ADS) == false)
        adButton->setEnabled(false);

    layout->setContentsMargins(0, 0, 0, 0);
    bar->setLayout(layout);
    return bar;
}

void ZChannel::onBlockerDialog()
{
    ZBlockerDialog *d = new ZBlockerDialog(settings);
    d->setModal(true);
    d->exec();
}

void ZChannel::onChannelDialog()
{
    emit runAction(ACTION_CHANNEL_DIALOG, "");
}

void ZChannel::onCloseClicked()
{
    emit closeChannel(this);
}

void ZChannel::onPostAdClicked()
{
    sendingAd = true;
    onEnterPressed();
    sendingAd = false;
}

void ZChannel::onAnchorClicked(QUrl url)
{
    QString urlString = url.toString();

    if (urlString.startsWith("http://") ||
        urlString.startsWith("https://"))
        QDesktopServices::openUrl(url);
    else if (urlString.startsWith("session://")) {
        QString channel = urlString.mid(10);
        emit runAction(ACTION_JOIN, channel);
    }
}

void ZChannel::onTypingClear()
{
    emit socketMessage(MAKE_2("TPN", "character", name, "status", "clear"));
}

void ZChannel::onTypingStartStop(bool start)
{
    QString status;
    if (start)
        status = "typing";
    else
        status = "paused";

    emit socketMessage(MAKE_2("TPN", "character", name, "status", status));
}

/* ---Context Menus--- */

void ZChannel::onContextInterested()
{
    emit runAction(ACTION_INTERESTED, contextString);
}

void ZChannel::onContextNotInterested()
{
    emit runAction(ACTION_NOT_INTERESTED, contextString);
}

void ZChannel::onContextPm()
{
    emit runAction(ACTION_PM, contextString);
}

void ZChannel::onContextFullProfile()
{
    emit runAction(ACTION_FULL_PROFILE, contextString);
}

void ZChannel::doContextMenu(QString context)
{
    contextString = context;
    QMenu menu(nullptr);

    /* So there's a nice titlebar for the name. */
    menu.addAction(contextString)->setEnabled(false);

    menu.addAction(QIcon(":/img/context-pm.png"), "Private Message",
    this, &ZChannel::onContextPm);

    menu.addAction(QIcon(":/img/context-full-profile.png"), "View Profile",
    this, &ZChannel::onContextFullProfile);

    int flags = persondb->flagsFor(contextString);

    if (flags & PERSON_INTERESTED) {
        menu.addAction(QIcon(":/img/context-not-interested.png"), "Not Interested",
        this, &ZChannel::onContextNotInterested);
    }
    else {
        menu.addAction(QIcon(":/img/context-interested.png"), "Interested",
        this, &ZChannel::onContextInterested);
    }

    menu.exec(QCursor::pos());
}

void ZChannel::onBrowserContextMenu(const QPoint &point)
{
    contextString = chatBrowser->anchorAt(point);
    /* Only give a custom context menu for user profiles. */

    if (contextString.startsWith("https://f-list.net/c/")) {
        contextString = contextString.mid(QString("https://f-list.net/c/").length());
        emit runAction(ACTION_RECENT, contextString);
        doContextMenu(contextString);
        return;
    }

    chatBrowser->createStandardContextMenu(point)->exec(QCursor::pos());
}

void ZChannel::onSidebarContextMenu(const QPoint &point)
{
    QListWidgetItem *item = characterSidebar->itemAt(point);
    if (item == nullptr)
        return;

    ZChannelInfo *info = channelPeople[characterSidebar->row(item)];
    contextString = info->person->getName();
    emit runAction(ACTION_RECENT, contextString);
    doContextMenu(contextString);
}

void ZChannel::setMode(int m)
{
    flags = (flags & TYPE_ANY) | m;
    adButton->setEnabled(m & MODE_ADS);
}

/* ---Character handling--- */

QListWidgetItem *ZChannel::itemFor(ZChannelInfo *info)
{
    ZPerson *person = info->person;
    QString title;
    int flags = info->infoFlags;

    if (flags != 0) {
        title = "";
        if (flags & PERSON_ADMIN)
            title += "Ⓐ";
        if (flags & PERSON_FRIEND)
            title += "Ⓕ";
        if (flags & PERSON_OWNER)
            title += "Ⓞ";
        if (flags & PERSON_MOD)
            title += "Ⓜ";

        title += person->getName();
    }
    else
        title = person->getName();

    QListWidgetItem *item = new QListWidgetItem(title);

    QFont f = item->font();
    f.setBold(true);

    if ((flags & PERSON_NOT_IGNORED) == false ||
        (flags & PERSON_INTERESTED) == false) {
        f.setStrikeOut(true);
    }

    item->setForeground(ZCache::brushForGender[person->getGender()]);
    item->setFont(f);
    item->setIcon(ZCache::iconForId[person->getStatus()]);
    item->setFlags(Qt::ItemIsEnabled);

    info->channelItem = item;
    return item;
}

void ZChannel::initializeInfo(QList<ZChannelInfo *> info)
{
    channelPeople = info;
    foreach (ZChannelInfo *i, info) {
        characterSidebar->addItem(itemFor(i));
    }
}

void ZChannel::addInfo(ZChannelInfo *info)
{
    QListWidgetItem *item = itemFor(info);
    QList<ZChannelInfo *>::iterator it = qUpperBound(channelPeople.begin(), channelPeople.end(), info, ZChannelInfo::compareInfo);
    if (it != channelPeople.end()) {
        int row = characterSidebar->row((*it)->channelItem);
        characterSidebar->insertItem(row, item);
        channelPeople.insert(it, info);
    } else {
        characterSidebar->addItem(item);
        channelPeople.append(info);
    }
}

void ZChannel::deleteInfo(ZChannelInfo *info)
{
    removeInfo(info);
    delete info;
}

void ZChannel::removeInfo(ZChannelInfo *info)
{
    delete characterSidebar->takeItem(characterSidebar->row(info->channelItem));
    QList<ZChannelInfo *>::iterator it = qBinaryFind(channelPeople.begin(), channelPeople.end(), info, ZChannelInfo::compareInfo);

    if (it != channelPeople.end()) {
        channelPeople.erase(it);
    }
}

void ZChannel::demoteInfo(ZChannelInfo *info, QString name)
{
    opbase.insert(name, opbase.value(name, 0) & ~PERSON_MOD);
    if (info) {
        removeInfo(info);
        info->infoFlags &= ~PERSON_MOD;
        addInfo(info);
    }
}

void ZChannel::promoteInfo(ZChannelInfo *info, QString name)
{
    opbase.insert(name, opbase.value(name, 0) | PERSON_MOD);
    if (info) {
        removeInfo(info);
        info->infoFlags |= PERSON_MOD;
        addInfo(info);
    }
}

/* ---Input processing--- */

void ZChannel::parseCommand(QString text)
{
    int firstSpace = text.indexOf(" ");
    QString command = text.mid(0, firstSpace);
    bool success = true;

    if (command == "/clear") {
        chatBrowser->clear();
    }
    else if (command == "/test-input") {
        QString base = text.mid(firstSpace, -1);
        QStringList split = base.split("\n");
        foreach (QString fake, split) {
            fake = fake.simplified();

            if (fake.isEmpty() == false) {
                emit socketFakeInput(fake);
            }
        }
    }
    else if (command == "/test-send") {
        emit socketMessage(text.mid(firstSpace, -1).simplified());
    }
    else if (command == "/status") {
        QString data = text.mid(8).simplified();
        if (data.isEmpty() == false) {
            int divide = data.indexOf(" ");
            QString statusString;
            QString statusMessage;
            if (divide == -1) {
                statusString = data;
                statusMessage = "";
            }
            else {
                statusString = data.mid(0, divide);
                statusMessage = data.mid(divide + 1, -1);
            }

            statusString = statusString.toLower();
            if (statusString == "away" ||
                statusString == "busy" ||
                statusString == "dnd" ||
                statusString == "idle" ||
                statusString == "looking" ||
                statusString == "online") {
                emit socketMessage(MAKE_2("STA", "status", statusString, "statusmsg", statusMessage));
                statusString = statusString[0].toUpper() + statusString.mid(1);
                QString message = QString("Your status is now <b>%1</b> (%2).")
                    .arg(statusString).arg(statusMessage);
                pushMessage(message);
            }
            else
                pushMessage(QString("Error: /status value '%1' is invalid.").arg(statusString));
        }
        else
            pushMessage("Usage: /status (value) (optional message).");
    }
    else {
        success = false;
    }

    if (success == false) {
        pushMessage(QString("Error: Command '%1' is invalid.").arg(command));
    }
}

void ZChannel::onEnterPressed()
{
    QString text = inputEdit->toPlainText();
    int type = getType();

    if (text.isEmpty() ||
        /* They're offline and can't hear you. */
        (type == TYPE_PM && persondb->find(name) == nullptr) ||
        /* Only commands for console. */
        (type == TYPE_CONSOLE && text[0] != '/')) {
        return;
    }

    /* Input has been captured and will be processed, so clear the window.
       QTextEdit::{setPlainText,clear} erase ALL undo history. That's bad. */
    QTextCursor cursor(inputEdit->textCursor());
    cursor.clearSelection();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();

    if (text.startsWith("/")) {
        parseCommand(text);
        return;
    }

    text = text.toHtmlEscaped();

    if (text.contains("\\")) {
        text = text.replace("\\", "\\\\");
    }

    QString toSend;

    if (type == TYPE_PM)
        toSend = MAKE_2("PRI", "recipient", name, "message", text);
    else if (sendingAd || (flags & MODE_CHAT) == 0)
        toSend = MAKE_2("LRP", "channel", name, "message", text);
    else
        toSend = MAKE_2("MSG", "channel", name, "message", text);

    QSound::play(ZCache::soundNameForId[SOUND_CHAT]);
    emit socketMessage(toSend);
}
