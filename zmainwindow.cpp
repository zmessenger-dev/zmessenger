#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QSplitter>
#include <QStackedWidget>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPalette>
#include <QShortcut>
#include <QSound>
#include <QStatusBar>
#include <QTime>

#include "widgets/zchanneldialog.h"
#include "widgets/zlogindialog.h"

#include "zcache.h"
#include "zdefines.h"
#include "zchannel.h"
#include "zmainwindow.h"
#include "zparser.h"
#include "zperson.h"
#include "zpersondb.h"
#include "zsettings.h"

#define ChannelNameRole (Qt::UserRole + 1)
#define ChannelKindRole (Qt::UserRole + 2)

enum CommandEnum
{
    COMMAND_ADL,
    COMMAND_AOP,
    COMMAND_BRO,
    COMMAND_CBU,
    COMMAND_CDS,
    COMMAND_CHA,
    COMMAND_CIU,
    COMMAND_CKU,
    COMMAND_COA,
    COMMAND_COL,
    COMMAND_CON,
    COMMAND_COR,
    COMMAND_CTU,
    COMMAND_DOP,
    COMMAND_ERR,
    COMMAND_FLN,
    COMMAND_FRL,
    COMMAND_HLO,
    COMMAND_ICH,
    COMMAND_IDN,
    COMMAND_IGN,
    COMMAND_JCH,
    COMMAND_LCH,
    COMMAND_LIS,
    COMMAND_LRP,
    COMMAND_MSG,
    COMMAND_NLN,
    COMMAND_ORS,
    COMMAND_PIN,
    COMMAND_PRI,
    COMMAND_RMO,
    COMMAND_RTB,
    COMMAND_STA,
    COMMAND_SYS,
    COMMAND_TPN,
    COMMAND_VAR,

    COMMAND_BAD
};

#define RECORD(fmt, args...) \
{ \
    if (record) { \
        fprintf(stderr, fmt, args); \
    } \
}

#define LOG(fmt, args...) \
fprintf(stderr, fmt, args)

#define LOG_1(msg) \
fputs(msg, stderr)

ZMainWindow::ZMainWindow()
{
    ZCache::init();
    ZParser::init();
    settings = new ZSettings(QString("%1/settings.json").arg(QApplication::applicationDirPath()));
    persondb = new ZPersonDb(settings);
    socket = new QWebSocket;
    publicChannels = QHash<QString, ZChannel *>();
    privateChannels = QHash<QString, ZChannel *>();
    commandMapping = QHash<QString, int>();
    pendingOpbase = QHash<QString, int>();
    testing = false;
    record = false;
    unreadTotal = 0;
    channelDialog = nullptr;

    commandMapping["ADL"] = COMMAND_ADL;
    commandMapping["AOP"] = COMMAND_AOP;
    commandMapping["BRO"] = COMMAND_BRO;
    commandMapping["CBU"] = COMMAND_CBU;
    commandMapping["CDS"] = COMMAND_CDS;
    commandMapping["CHA"] = COMMAND_CHA;
    commandMapping["CIU"] = COMMAND_CIU;
    commandMapping["CKU"] = COMMAND_CKU;
    commandMapping["COA"] = COMMAND_COA;
    commandMapping["COL"] = COMMAND_COL;
    commandMapping["CON"] = COMMAND_CON;
    commandMapping["COR"] = COMMAND_COR;
    commandMapping["CTU"] = COMMAND_CTU;
    commandMapping["DOP"] = COMMAND_DOP;
    commandMapping["ERR"] = COMMAND_ERR;
    commandMapping["FLN"] = COMMAND_FLN;
    commandMapping["FRL"] = COMMAND_FRL;
    commandMapping["HLO"] = COMMAND_HLO;
    commandMapping["ICH"] = COMMAND_ICH;
    commandMapping["IDN"] = COMMAND_IDN;
    commandMapping["IGN"] = COMMAND_IGN;
    commandMapping["JCH"] = COMMAND_JCH;
    commandMapping["LCH"] = COMMAND_LCH;
    commandMapping["LIS"] = COMMAND_LIS;
    commandMapping["LRP"] = COMMAND_LRP;
    commandMapping["MSG"] = COMMAND_MSG;
    commandMapping["NLN"] = COMMAND_NLN;
    commandMapping["ORS"] = COMMAND_ORS;
    commandMapping["PIN"] = COMMAND_PIN;
    commandMapping["PRI"] = COMMAND_PRI;
    commandMapping["RMO"] = COMMAND_RMO;
    commandMapping["RTB"] = COMMAND_RTB;
    commandMapping["STA"] = COMMAND_STA;
    commandMapping["SYS"] = COMMAND_SYS;
    commandMapping["TPN"] = COMMAND_TPN;
    commandMapping["VAR"] = COMMAND_VAR;

    genderMapping["Cunt-boy"] = GENDER_CUNT_BOY;
    genderMapping["Female"] = GENDER_FEMALE;
    genderMapping["Herm"] = GENDER_HERM;
    genderMapping["Male"] = GENDER_MALE;
    genderMapping["Male-Herm"] = GENDER_MALE_HERM;
    genderMapping["None"] = GENDER_NONE;
    genderMapping["Shemale"] = GENDER_SHEMALE;
    genderMapping["Transgender"] = GENDER_TRANSGENDER;

    statusMapping["away"] = STATUS_LOOKING;
    statusMapping["busy"] = STATUS_BUSY;
    statusMapping["dnd"] = STATUS_DND;
    statusMapping["looking"] = STATUS_LOOKING;
    statusMapping["online"] = STATUS_ONLINE;

    panelStackWidget = new QStackedWidget;

    panelPicker = new QListWidget;
    panelPicker->setSelectionMode(QAbstractItemView::SingleSelection);
    panelPicker->setDragEnabled(true);
    panelPicker->viewport()->setAcceptDrops(true);
    panelPicker->setDropIndicatorShown(true);
    panelPicker->setDragDropMode(QAbstractItemView::InternalMove);
    panelPicker->setMaximumWidth(175);
    panelPicker->setProperty("widgetname", "panel");

    connect(panelPicker, &QListWidget::currentItemChanged,
    this, &ZMainWindow::onPickerItemChanged);

    recentEdit = new QTextEdit("Recent: ");
    recentEdit->setProperty("widgetname", "recent");
    recentEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    recentEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recentEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    recentEdit->setLineWrapMode(QTextEdit::NoWrap);
    recentEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);

    QFontMetrics fm(recentEdit->font());
    recentEdit->setMaximumHeight(fm.lineSpacing() + fm.ascent());

    connect(recentEdit, &QWidget::customContextMenuRequested,
    this, &ZMainWindow::onRecentContextMenu);

    QWidget *w = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    QSplitter *s = new QSplitter;
    s->addWidget(panelPicker);
    s->addWidget(panelStackWidget);

    layout->addWidget(recentEdit);
    layout->addWidget(s, 1);
    layout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(layout);

    /* Allowing the use of common browser shortcuts feels right. */
    QShortcut *closeShortcut = new QShortcut(QKeySequence("Ctrl+W"), this);
    connect(closeShortcut, &QShortcut::activated, this, &ZMainWindow::onClose);

    QShortcut *pageDown = new QShortcut(QKeySequence("Ctrl+PgDown"), this);
    connect(pageDown, &QShortcut::activated, this, &ZMainWindow::onPageDown);

    QShortcut *pageUp = new QShortcut(QKeySequence("Ctrl+PgUp"), this);
    connect(pageUp, &QShortcut::activated, this, &ZMainWindow::onPageUp);

    setAttribute(Qt::WA_DeleteOnClose);
    setCentralWidget(w);
    setStyleSheet(
    "[widgetname=\"panel\"] { background: black; padding: 2px }"
    "[widgetname=\"panel\"]::item { border: 1px solid black; }"
    "[widgetname=\"panel\"]::item:selected {"
    "    border: 1px solid #55ddbb;"
    "}"
    "[widgetname=\"chat\"] { background-color: black; color: lightgray; }"
    "[widgetname=\"input\"] { background-color: black; color: lightgray; }"
    "[widgetname=\"sidebar\"] { background-color: black; }"
    "[widgetname=\"recent\"] { background-color: black; color: lightgray; }"
    "[widgetname=\"item-button\"] { background-color: black; text-align: left}");
}

ZMainWindow::~ZMainWindow()
{
    if (channelDialog)
        delete channelDialog;

    if (settings->getShouldSave()) {
        settings->save();
    }
}

bool ZMainWindow::start()
{
    QDir baseDir = QDir(QApplication::applicationDirPath());
    if (baseDir.exists("logs") == false) {
        if (baseDir.mkdir("logs") == false) {
            /* This shouldn't happen. If it does, make SURE the user knows. */
            QMessageBox::critical(this, "ZMessenger",
                "Could not create log directory (./logs). Stopping.");
            return false;
        }
    }

    QString ticket = "";
    QString account = "";

    if (testing == false) {
        ZLoginDialog d;
        if (d.exec() != QDialog::Accepted) {
            socket->close();
            return false;
        }

        ticket = d.getTicket();
        account = d.getAccount();
        persondb->setSelfName(d.getCharacter());
    }
    else {
        persondb->setSelfName("TestCharacter");
    }

    console = new ZChannel(persondb, TYPE_CONSOLE, persondb->getSelfName(),
        persondb->getSelfName(), settings);
    currentPanel = console;
    setupChannel(console, true);

    connect(panelPicker, &QListWidget::currentItemChanged,
    this, &ZMainWindow::onPickerItemChanged);

    /* Sets the starting window title. */
    onUpdateItemText(console->getPanelItem(), persondb->getSelfName(), 0);
    adjustSize();
    show();

    if (testing == false) {
        startConnect(ticket, account);
    }

    return true;
}

/* ---Messaging--- */

enum ZMessageType
{
    TYPE_INFO,
    TYPE_INVITE,
    TYPE_MESSAGE,
    TYPE_OFFLINE,
    TYPE_ONLINE,
    TYPE_RP_AD,
};

QString ZMainWindow::buildStampedMessage(QString message)
{
    return QString("[%1] %2")
        .arg(QTime::currentTime().toString("HH:mm"))
        .arg(message);
}

QString ZMainWindow::buildMessageFrom(ZPerson *source, QString message, int i)
{
    QString result = QString("[%1] ").arg(QTime::currentTime().toString("HH:mm"));

    switch ((ZMessageType)i) {
        case TYPE_INFO: break;
        case TYPE_INVITE: break;
        case TYPE_MESSAGE: break;
        case TYPE_OFFLINE:
            result += QString("<span style='color: #ff0000'>(Offline)</span> ");
            break;
        case TYPE_ONLINE:
            result += QString("<span style='color: #0000ff'>(Online)</span> ");
            break;
        case TYPE_RP_AD:
            result += QString("<span style='color: #7fff00'>(RP AD)</span> ");
            break;
    }

    result += linkFor(source);
    result += message;

    return result;
}

QString ZMainWindow::buildPoseFrom(ZPerson *person, QString message)
{
    bool saypose = false;
    /* This is to allow /me as well as variants like /me's and /me 's.
     * Not going to bother with validation. */
    if (message.startsWith("/me")) {
        saypose = true;
        message = message.mid(3);
    }

    message = ZParser::parse(message);

    QString fullMessage = buildMessageFrom(person, message, TYPE_MESSAGE);
    if (saypose)
        fullMessage = QString("<i>%1</i>").arg(fullMessage);

    return fullMessage;
}

QString ZMainWindow::linkFor(ZPerson *person)
{
    return QString("<b><a style='color: %1' href='https://f-list.net/c/%2'>%2</a></b> ")
        .arg(ZCache::hexForGender[person->getGender()])
        .arg(person->getName());
}

void ZMainWindow::sendOnOffMessage(ZPerson *person, bool on)
{
    QString message;
    if (on) {
        message = "is now ";
        message += statusString(person->getStatus());
        QString statusMessage = person->getMessage();
        if (statusMessage.isEmpty() == false)
            message += QString(" (%1)").arg(statusMessage);

        message += ".";
        message = buildMessageFrom(person, message, TYPE_ONLINE);
    }
    else
        message = buildMessageFrom(person, "is now offline.", TYPE_OFFLINE);

    currentPanel->pushMessage(message);
    if (currentPanel != console)
        console->pushMessage(message);
}

QString ZMainWindow::statusString(int s)
{
    switch ((ZStatus)s) {
        case STATUS_AWAY:    return "Away";    break;
        case STATUS_BUSY:    return "Busy";    break;
        case STATUS_DND:     return "DND";     break;
        case STATUS_LOOKING: return "Looking"; break;
        case STATUS_ONLINE:  return "Online";  break;
    }
    return "";
}

/* ---GUI--- */

void ZMainWindow::setupChannel(ZChannel *channel, bool select)
{
    connect(channel, &ZChannel::socketMessage,
    this, &ZMainWindow::onSocketMessage);

    connect(channel, &ZChannel::socketFakeInput,
    this, &ZMainWindow::onTextMessageReceived);

    connect(channel, &ZChannel::closeChannel,
    this, &ZMainWindow::onCloseChannel);

    connect(channel, &ZChannel::runAction,
    this, &ZMainWindow::onAction);

    connect(channel, &ZChannel::updateItemText,
    this, &ZMainWindow::onUpdateItemText);

    QString title = channel->getTitle();

    QListWidgetItem *item = new QListWidgetItem;
    item->setForeground(Qt::white);
    item->setData(ChannelNameRole, title);
    item->setData(ChannelKindRole, channel->getType());

    int type = channel->getType();

    if (type == TYPE_ADHOC)
        item->setIcon(QIcon(":/img/channel-adhoc.png"));
    else if (type == TYPE_PM)
        item->setIcon(ZCache::iconForId[TYPING_CLEAR]);
    else if (type == TYPE_PUBLIC)
        item->setIcon(QIcon(":/img/channel-public.png"));
    else if (type == TYPE_CONSOLE)
        item->setIcon(QIcon(":/img/channel-console.png"));

    channel->setPanelItem(item);
    panelPicker->addItem(item);
    panelStackWidget->addWidget(channel->getDisplayWidget());

    QPushButton *button = new QPushButton(title);
    QPalette p = button->palette();
    QBrush brush = Qt::white;

    /* The button is so the item always shows the gender color.
     * Without it, the color is wiped by the item's highlight. */
    if (type == TYPE_PM) {
        ZPerson *person = persondb->find(channel->getName());
        if (person)
            brush = ZCache::brushForGender[person->getGender()];
    }

    p.setColor(QPalette::ButtonText, brush.color());

    button->setFlat(true);
    button->setAutoFillBackground(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setProperty("widgetname", "item-button");
    button->setPalette(p);

    connect(button, &QAbstractButton::clicked,
    this, &ZMainWindow::onPickerWidgetClicked);

    panelPicker->setItemWidget(item, button);

    if (select)
        panelPicker->setCurrentItem(item);
}

void ZMainWindow::openPm(QString personName)
{
    ZPerson *person = persondb->find(personName);

    if (person == nullptr) {
        QString message = QString("%1 is not online.").arg(personName);
        currentPanel->pushMessage(buildStampedMessage(message));
        return;
    }

    ZChannel *channel = privateChannels.value(personName, nullptr);

    if (channel == nullptr) {
        channel = new ZChannel(persondb, TYPE_PM | MODE_CHAT, personName);
        privateChannels[personName] = channel;
        setupChannel(channel, true);
    }
    else {
        QListWidgetItem *item = channel->getPanelItem();
        item->setHidden(false);
    }

    persondb->setMarker(personName, PERSON_HAS_PM);
}

void ZMainWindow::onAction(int a, QString actionString)
{
    ZAction action = (ZAction) a;

    switch (action) {
        case ACTION_CHANNEL_DIALOG:
            if (channelDialog == nullptr) {
                channelDialog = new ZChannelDialog;
                /* It only sends the join action. */
                connect(channelDialog, &ZChannelDialog::join,
                this, &ZMainWindow::onAction);

                /* There are a lot of channels, and they don't change much.
                   Query once and cache the results. */
                socket->sendTextMessage("CHA");
                socket->sendTextMessage("ORS");
            }

            channelDialog->show();
            channelDialog->raise();
            break;
        case ACTION_FULL_PROFILE:
            QDesktopServices::openUrl(QUrl(QString("https://www.f-list.net/c/%1/")
                .arg(actionString)));
            break;
        case ACTION_INTERESTED: {
            settings->changed();
            persondb->setFlag(actionString, PERSON_INTERESTED);
            QString message = QString("You are now interested in '%1'.").arg(actionString);
            message = buildStampedMessage(message);
            currentPanel->pushMessage(message);
            if (currentPanel != console)
                console->pushMessage(message);
            break;
        }
        case ACTION_JOIN:
            if (publicChannels.value(actionString, nullptr) == nullptr) {
                QString message = QString("JCH {\"channel\":\"%1\"}")
                    .arg(actionString);
                socket->sendTextMessage(message);
            }
            break;
        case ACTION_NOT_INTERESTED: {
            settings->changed();
            persondb->unsetFlag(actionString, PERSON_INTERESTED);
            QString message = QString("You are no longer interested in '%1'.").arg(actionString);
            message = buildStampedMessage(message);
            currentPanel->pushMessage(message);
            if (currentPanel != console)
                console->pushMessage(message);
            break;
        }
        case ACTION_PM:
            openPm(actionString);
            break;
        case ACTION_RECENT:
            recentName = actionString;
            ZPerson *p = persondb->find(actionString);
            if (p != nullptr) {
                QString message = QString("Recent: %2 is %3")
                    .arg(linkFor(p))
                    .arg(statusString(p->getStatus()));

                QString status = p->getMessage();
                if (status.isEmpty() == false)
                    message += QString(" (%1)").arg(status);

                recentEdit->setText(message);
            }

            break;
    }
}

void ZMainWindow::onPickerItemChanged(QListWidgetItem *item, QListWidgetItem *)
{
    int kind = item->data(ChannelKindRole).toInt();
    QString name = item->data(ChannelNameRole).toString();

    currentPanel->setIsCurrent(false);

    if (kind & (TYPE_PUBLIC | TYPE_ADHOC)) {
        currentPanel = findPublicChannel(name);
    }
    else if (kind == TYPE_PM) {
        currentPanel = findPrivateChannel(name);
        if (recentName != name)
            onAction(ACTION_RECENT, name);

        int unread = currentPanel->getNumUnread();
        if (unread)
            onUpdateItemText(item, name, -unread);
    }
    else if (kind == TYPE_CONSOLE) {
        currentPanel = console;
        onAction(ACTION_RECENT, persondb->getSelfName());
    }
    else {
        currentPanel = nullptr;
        return;
    }

    currentPanel->setIsCurrent(true);
    panelStackWidget->setCurrentWidget(currentPanel->getDisplayWidget());
}

void ZMainWindow::onPickerWidgetClicked()
{
    QWidget *button = (QWidget *)sender();

    /* Transfer the button press through to the appropriate item. */
    for (int i = 0;i < panelPicker->count();i++) {
        QListWidgetItem *item = panelPicker->item(i);
        if (panelPicker->itemWidget(item) == button) {
            panelPicker->setCurrentItem(item);
            break;
        }
    }
}

void ZMainWindow::onCloseChannel(ZChannel *channel)
{
    panelPicker->setCurrentItem(console->getPanelItem());
    QListWidgetItem *item = channel->getPanelItem();

    if (channel->getType() & (TYPE_PUBLIC | TYPE_ADHOC)) {
        publicChannels[channel->getName()] = nullptr;
        QString message = QString("LCH {\"channel\":\"%1\",\"character\":\"%2\"}")
            .arg(channel->getName())
            .arg(persondb->getSelfName());
        socket->sendTextMessage(message);
    }
    else {
        QString name = channel->getName();
        int flags = persondb->flagsFor(name);
        if ((flags & PERSON_INTERESTED) && (flags & PERSON_NOT_IGNORED)) {
            /* Hide for now. Might be interested later? Accidental close? */
            item->setHidden(true);
            return;
        }
        else
            persondb->unsetMarker(name, PERSON_HAS_PM);

        privateChannels[name] = nullptr;
    }

    delete panelPicker->takeItem(panelPicker->row(item));
    panelStackWidget->removeWidget(channel->getDisplayWidget());

    delete channel;
}

void ZMainWindow::onUpdateItemText(QListWidgetItem *item, QString text, int adjust)
{
    QPushButton *button = (QPushButton *)panelPicker->itemWidget(item);
    button->setText(text);

    unreadTotal += adjust;
    QString title = "";

    if (unreadTotal)
        title = QString("(%1) ").arg(unreadTotal);

    title += CLIENT_NAME " " CLIENT_VERSION;
    setWindowTitle(title);
}

void ZMainWindow::onRecentContextMenu()
{
    console->doContextMenu(recentName);
}

void ZMainWindow::onClose()
{
    if (currentPanel != console)
        onCloseChannel(currentPanel);
}

void ZMainWindow::onPageDown()
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, 0);
    QCoreApplication::postEvent(panelPicker, event);
}

void ZMainWindow::onPageUp()
{
    QKeyEvent *event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, 0);
    QCoreApplication::postEvent(panelPicker, event);
}

/* ---Connection--- */

#define ADD_FIELD(what, pre, fieldname, fieldvalue) \
what += pre; \
what += "\""; \
what += fieldname; \
what += "\":\""; \
what += fieldvalue; \
what += "\"";

#define ADD_LAST(what, pre, fieldname, fieldvalue) \
ADD_FIELD(what, pre, fieldname, fieldvalue) \
what += "}";

void ZMainWindow::startConnect(QString ticket, QString account)
{
    /* Assume socket connect is faster than user login picking. */
    connectString = QString("IDN ");
    ADD_FIELD(connectString, "{", "method", "ticket")
    ADD_FIELD(connectString, ",", "ticket", ticket)
    ADD_FIELD(connectString, ",", "account", account)
    ADD_FIELD(connectString, ",", "character", persondb->getSelfName())
    ADD_FIELD(connectString, ",", "cname", CLIENT_NAME)
    ADD_LAST(connectString, ",", "cversion", CLIENT_VERSION)

    connect(socket, &QWebSocket::connected,
    this, &ZMainWindow::onSocketConnected);
    connect(socket, &QWebSocket::disconnected,
    this, &ZMainWindow::onSocketDisconnected);
    connect(socket, &QWebSocket::textMessageReceived,
    this, &ZMainWindow::onTextMessageReceived);

    socket->open(QUrl(CHAT_FULL));
}

void ZMainWindow::onTextMessageReceived(const QString &message)
{
    /* Safely returns "" on error. */
    QString toParse = message.mid(4, -1);

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(toParse.toUtf8(), &err);
    if (err.error != QJsonParseError::NoError) {
        if (message.mid(0, 3) == "PIN") {
            processPIN();
            return;
        }
        LOG("(ERR [bad json]): '%s'.\n", message.toStdString().c_str());
        return;
    }

    QJsonObject o = doc.object();
    CommandEnum e = (CommandEnum)commandMapping.value(message.mid(0, 3), COMMAND_BAD);
    if (e == COMMAND_BAD) {
        LOG("(ERR [unknown command]): '%s'.\n", message.toStdString().c_str());
        return;
    }

    /* Assume a complete, valid message and log it. */
    RECORD("(IN): %s\n", message.toStdString().c_str());

    switch (e) {
        case COMMAND_ADL: processADL(o); break;
        case COMMAND_AOP: processAOP(o); break;
        case COMMAND_BRO: processBRO(o); break;
        case COMMAND_CBU: processCBU(o); break;
        case COMMAND_CDS: processCDS(o); break;
        case COMMAND_CHA: processCHA(o); break;
        case COMMAND_CIU: processCIU(o); break;
        case COMMAND_CKU: processCKU(o); break;
        case COMMAND_COA: processCOA(o); break;
        case COMMAND_COL: processCOL(o); break;
        case COMMAND_CON: processCON(o); break;
        case COMMAND_COR: processCOR(o); break;
        case COMMAND_CTU: processCTU(o); break;
        case COMMAND_DOP: processDOP(o); break;
        case COMMAND_ERR: processERR(o); break;
        case COMMAND_FLN: processFLN(o); break;
        case COMMAND_FRL: processFRL(o); break;
        case COMMAND_HLO: processHLO(o); break;
        case COMMAND_ICH: processICH(o); break;
        case COMMAND_IDN: processIDN(o); break;
        case COMMAND_IGN: processIGN(o); break;
        case COMMAND_JCH: processJCH(o); break;
        case COMMAND_LCH: processLCH(o); break;
        case COMMAND_LIS: processLIS(o); break;
        case COMMAND_LRP: processLRP(o); break;
        case COMMAND_MSG: processMSG(o); break;
        case COMMAND_NLN: processNLN(o); break;
        case COMMAND_ORS: processORS(o); break;
        case COMMAND_PRI: processPRI(o); break;
        case COMMAND_RMO: processRMO(o); break;
        case COMMAND_RTB: processRTB(o); break;
        case COMMAND_STA: processSTA(o); break;
        case COMMAND_SYS: processSYS(o); break;
        case COMMAND_TPN: processTPN(o); break;
        case COMMAND_VAR: processVAR(o); break;

        /* These two are impossible, but complete the switch. */
        case COMMAND_PIN:
        case COMMAND_BAD:
        return;
    }
}

void ZMainWindow::onSocketMessage(QString message)
{
    RECORD("(OUT): %s\n", message.toStdString().c_str());
    socket->sendTextMessage(message);
}

void ZMainWindow::onSocketConnected()
{
    /* For privacy, don't log this. */
    socket->sendTextMessage(connectString);
    connectString = "";
}

void ZMainWindow::onSocketDisconnected()
{
    QString message = buildStampedMessage("Disconnected from F-chat.");

    console->pushMessage(message);

    foreach (ZChannel *channel, publicChannels) {
        channel->pushMessage(message);
    }
}

/* ---Command processing--- */

#define ASSIGN_AND_CHECK(str, target, action) \
target = action; \
if (target == nullptr) { \
    LOG("[Error] %s: %s is null.\n", str, #target); \
    return; \
}

void ZMainWindow::processADL(QJsonObject o)
{
    /* Initial admin list. */
    /* ADL {"ops": ["Silver", "Hiro", "Jamii", "Oskenso", "Aniko", ...]} */
    QJsonArray a = o["ops"].toArray();
    int size = a.size();

    for (int i = 0;i < size;i++) {
        QString op = a.at(i).toString();
        persondb->setFlag(op, PERSON_ADMIN);
    }
}

void ZMainWindow::processAOP(QJsonObject o)
{
    /* Promote character to chatop */
    /* AOP {"character": string} */
    QString personName = o["character"].toString();
    persondb->setFlag(personName, PERSON_ADMIN);
}

void ZMainWindow::processBRO(QJsonObject o)
{
    /* Admin broadcast. */
    /* BRO { "message": string } */
    QString message = buildStampedMessage(ZParser::parse(o["message"].toString()));

    foreach (ZChannel *ch, publicChannels) {
        ch->pushMessage(message);
    }

    console->pushMessage(message);
}

void ZMainWindow::processBanOrKick(QJsonObject o, QString action, QString extra)
{
    /* Removes user from channel. */
    /* CBU/CKU {"operator":string,"channel":string,"character":string} */
    /* Since this isn't user-provided, assume the server has done power checks. */
    QString operatorName = o["operator"].toString();
    QString personName = o["character"].toString();
    QString channelName = o["channel"].toString();
    ZPerson *person;
    ZPerson *op;
    ZChannel *channel;
    ASSIGN_AND_CHECK("CBU", person, persondb->find(personName))
    ASSIGN_AND_CHECK("CBU", op, persondb->find(personName))
    ASSIGN_AND_CHECK("CBU", channel, publicChannels.value(channelName, nullptr))
    QString m = QString("has %1 '%2' from '%3'%4.")
        .arg(action)
        .arg(person->getName())
        .arg(channel->getTitle())
        .arg(extra);
    QString fullMessage = buildMessageFrom(op, m, TYPE_INFO);

    channel->pushMessage(fullMessage);

    if (person == persondb->getSelf() && currentPanel != console)
        console->pushMessage(fullMessage);

    ZChannelInfo *info = person->listenerFor(channel);
    if (info) {
        channel->deleteInfo(info);
    }
}

void ZMainWindow::processCBU(QJsonObject o)
{
    /* Removes user from channel, and prevents re-entry. */
    /* CBU {"operator":string,"channel":string,"character":string} */
    /* Since this isn't user-provided, assume the server has done power checks. */
    processBanOrKick(o, "banned", "");
}

void ZMainWindow::processCDS(QJsonObject o)
{
    /* The channel's description has changed, */
    /* Usually sent as part of a reply to JCH (join channel) */
    /* CDS { "channel": string, "description": string } */
    QString channelName = o["channel"].toString();
    QString description = o["description"].toString();
    ZChannel *channel;
    ASSIGN_AND_CHECK("CDS", channel, publicChannels.value(channelName, nullptr))

    QString rawMessage = QString("Welcome to <u>%1</u>: %2")
        .arg(channel->getTitle())
        .arg(ZParser::parse(description));

    channel->pushMessage(buildStampedMessage(rawMessage));
}

void ZMainWindow::processCHA(QJsonObject o)
{
    /* Sends the client a list of all public channels. */
    /* CHA {["name":"channel name (and title)", "characters": value]...} */
    QList<QStringList> channelList = QList<QStringList>();
    QJsonArray ja = o["channels"].toArray();
    QChar fillChar('0');

    foreach (QJsonValue v, ja) {
        QJsonObject channel = v.toObject();
        QString name = channel["name"].toString();
        QString count = QString("%1").arg(channel["characters"].toInt(), 4, 10, fillChar);

        channelList.append(QStringList() << count << name);
    }

    if (channelDialog)
        channelDialog->updatePublicRooms(channelList);
}

void ZMainWindow::processCKU(QJsonObject o)
{
    /* Kicks user from channel. */
    /* CKU {"operator":string,"channel":string,"character":string} */
    /* Since this isn't user-provided, assume the server has done power checks. */
    processBanOrKick(o, "kicked", "");
}

void ZMainWindow::processCOA(QJsonObject o)
{
    /* Promote user to channel operator */
    /* COA {"character":"character_name", "channel":"channel_name"} */
    /* Not spoofable, so assume sanity in server. */
    QString personName = o["character"].toString();
    QString channelName = o["channel"].toString();
    ZChannel *channel;
    ASSIGN_AND_CHECK("COA", channel, publicChannels.value(channelName, nullptr))

    ZPerson *person = persondb->find(personName);
    ZChannelInfo *info = nullptr;
    if (person) {
        info = person->listenerFor(channel);
    }

    channel->promoteInfo(info, personName);
}

void ZMainWindow::processCOL(QJsonObject o)
{
    /* Initial list of channel ops. */
    /* COL { "channel": string, "oplist": [string] } */
    /* The first entry is the owner, and may be "". */
    /* Sent in reply to JCH, but comes before ICH. */
    QString channelName = o["channel"].toString();
    QJsonArray ja = o["oplist"].toArray();
    pendingOpbase.clear();
    ZChannel *channel;
    ASSIGN_AND_CHECK("COL", channel, publicChannels.value(channelName, nullptr))

    if (ja.size() == 0)
        return;

    QString owner = ja.at(0).toString();

    if (owner.isEmpty() == false)
        pendingOpbase[owner] = PERSON_OWNER;

    for (int i = 1;i < ja.size();i++) {
        QString s = ja.at(i).toString();
        pendingOpbase[s] = PERSON_MOD;
    }

    channel->initializeOpbase(pendingOpbase);
}

void ZMainWindow::processCON(QJsonObject o)
{
    /* # of people connected. */
    /* CON { "count": int } */
    int count = o["count"].toInt(); /* Safely defaults to 0 on error. */
    QString m = QString("There are %1 users online.").arg(count);
    console->pushMessage(buildStampedMessage(m));
}

void ZMainWindow::processCOR(QJsonObject o)
{
    /* Demote user from channel operator */
    /* COR {"character":"character_name", "channel":"channel_name"} */
    /* Not spoofable, so assume sanity in server. */
    QString personName = o["character"].toString();
    QString channelName = o["channel"].toString();
    ZChannel *channel;
    ASSIGN_AND_CHECK("COR", channel, publicChannels.value(channelName, nullptr))
    ZPerson *person = persondb->find(personName);

    ZChannelInfo *info = nullptr;
    if (person) {
        info = person->listenerFor(channel);
    }

    channel->demoteInfo(info, personName);
}

void ZMainWindow::processCIU(QJsonObject o)
{
    /* Invite user to channel. */
    /* CIU { "sender":string,"title":string,"name":string } */
    QString senderName = o["sender"].toString();
    QString title = o["title"].toString();
    QString channelName = o["name"].toString();
    ZPerson *sender;
    ASSIGN_AND_CHECK("CIU", sender, persondb->find(senderName))

    QString message = QString(" has invited you to '%1' (%2)").arg(title).arg(channelName);
    QString fullMessage = buildMessageFrom(sender, message, TYPE_INVITE);

    currentPanel->pushMessage(fullMessage);
    if (currentPanel != console) {
        console->pushMessage(fullMessage);
    }
}

void ZMainWindow::processCTU(QJsonObject o)
{
    /* Temporary ban. */
    /* CTU {"operator":"string","channel":"string","length":int,"character":"string"} */
    QString timeout = QString(" for %1 minutes").arg(o["length"].toString());
    processBanOrKick(o, "banned", timeout);
}

void ZMainWindow::processDOP(QJsonObject o)
{
    /* Remove character from chatop. */
    /* DOP {"character": string} */
    QString personName = o["character"].toString();
    persondb->unsetFlag(personName, PERSON_ADMIN);
}

void ZMainWindow::processERR(QJsonObject o)
{
    /* There's an error. */
    /* ERR { "number": int, "message": string } */
    int error = o["number"].toInt(0);
    QString message = o["message"].toString(); /* todo: ZParse? */

    QString partial = QString("Error #%1: %2").arg(error).arg(message);
    QString fullMessage = buildStampedMessage(partial);
    currentPanel->pushMessage(fullMessage);
    if (currentPanel != console) {
        console->pushMessage(fullMessage);
    }
}

void ZMainWindow::processFLN(QJsonObject o)
{
    /* Character offline. */
    QString personName = o["character"].toString();
    ZPerson *person;
    ASSIGN_AND_CHECK("FLN", person, persondb->find(personName))

    int flags = person->allFlags();
    if (flags & PERSON_FRIEND)
        sendOnOffMessage(person, false);
    if (flags & PERSON_HAS_PM) {
        ZChannel *channel = privateChannels.value(personName, nullptr);
        if (channel)
            channel->getPanelItem()->setIcon(ZCache::iconForId[TYPING_OFFLINE]);
    }

    foreach (ZChannelInfo *info, person->getListeners()) {
        info->channel->deleteInfo(info);
    }

    persondb->remove(personName);
    delete person;
}

void ZMainWindow::processFRL(QJsonObject o)
{
    /* Friend/bookmark list. */
    /* FRL { "characters": [string] } */
    QJsonArray ja = o["characters"].toArray();

    foreach (QJsonValue v, ja) {
        QString name = v.toString();
        persondb->setFlag(name, PERSON_FRIEND);
    }
}

void ZMainWindow::processHLO(QJsonObject o)
{
    /* Server welcome message */
    /* HLO { "message": string } */
    /* HLO {"message":"Welcome. Running F-Chat 0.8.6-Lua by Kira. Enjoy your stay."} */
    QString message = o["message"].toString(); /* todo: ZParse? */

    console->pushMessage(buildStampedMessage(message));
}

void ZMainWindow::processICH(QJsonObject o)
{
    /* Initial channel data. Received in response to JCH, along with CDS. */
    /* ICH { "users": [object], "channel": string, "mode": enum } */
    /* ICH {"users": [{"identity": "Shadlor"}, ...], "channel": "Frontpage", mode: "chat"} */

    QString channelName = o["channel"].toString();
    QString channelMode = o["mode"].toString();
    QJsonArray ja = o["users"].toArray();
    QString channelTitle = "";
    int flags = TYPE_PUBLIC;
    ZChannel *channel;

    /* Should be a channel: JCH with self identity comes first. */
    ASSIGN_AND_CHECK("ICH", channel, publicChannels.value(channelName, nullptr))
    int mode;

    if (channelMode == "ads")
        mode = MODE_ADS;
    else if (channelMode == "chat")
        mode = MODE_CHAT;
    else if (channelMode == "both")
        mode = MODE_ADS | MODE_CHAT;
    else
        return;

    channel->setMode(mode);

    QList<ZChannelInfo *> infoList;

    foreach (QJsonValue v, ja) {
        QString personName = v.toObject()["identity"].toString();
        ZPerson *p = persondb->find(personName);

        if (p) {
            ZChannelInfo *info = new ZChannelInfo(p, channel);
            infoList.append(info);

            /* Default = 0 because NO flags should be assumed from this. */
            info->infoFlags |= pendingOpbase.value(p->getName(), 0);
        }
        else {
            LOG("[ICH]: Unable to find '%s'.\n", personName.toStdString().c_str());
        }
    }

    qSort(infoList.begin(), infoList.end(), ZChannelInfo::compareInfo);
    channel->initializeInfo(infoList);
}

void ZMainWindow::processIDN(QJsonObject o)
{
    /* Identity verified. */
    /* IDN { "character": string } */
    QString name = o["character"].toString();

    if (name != persondb->getSelfName()) {
        LOG("[IDN]: Wrong account. Expected '%s', but got '%s' instead.\n",
            persondb->getSelfName().toStdString().c_str(), name.toStdString().c_str());
        return;
    }

    QString message = QString("Connected to F-chat as '%1'.").arg(persondb->getSelfName());
    console->pushMessage(buildStampedMessage(message));
}

void ZMainWindow::processIGN(QJsonObject o)
{
    /* Ignore list action. */
    /* Samples: */
    /* IGN {"characters":["Teal Deer", "Pas un Caractere", "Testytest"],"action":"init"} */
    /* IGN {"character":"Teal Deer","action":"add"} */
    /* IGN {"character":"Teal Deer","action":"delete"} */
    QString action = o["action"].toString();
    QString message = "";

    if (action == "init") {
        QJsonArray ja = o["characters"].toArray();

        foreach (QJsonValue v, ja) {
            QString name = v.toString();
            persondb->unsetFlag(name, PERSON_NOT_IGNORED);
        }
        return;
    }
    else if (action == "add") {
        QString name = o["character"].toString();
        ZPerson *person = persondb->find(name);

        persondb->unsetFlag(name, PERSON_NOT_IGNORED);
        message = buildMessageFrom(person, "added to your ignore list.", TYPE_INFO);
    }
    else if (action == "delete") {
        QString name = o["character"].toString();
        ZPerson *person = persondb->find(name);

        persondb->setFlag(name, PERSON_NOT_IGNORED);
        message = buildMessageFrom(person, "removed from your ignore list.", TYPE_INFO);
    }
    else {
        LOG("Unknown IGN action '%s'.\n", action.toStdString().c_str());
        return;
    }

    currentPanel->pushMessage(message);
    if (currentPanel != console)
        console->pushMessage(message);
}

void ZMainWindow::processJCH(QJsonObject o)
{
    /* Join a channel. */
    /* JCH {"channel": string, "character": object, "title": string } */
    QString channelName = o["channel"].toString();
    QString personName = o["character"].toObject()["identity"].toString();
    ZChannel *channel = publicChannels.value(channelName, nullptr);

    if (channel == nullptr) {
        if (personName != persondb->getSelfName())
            return;

        /* Server's reply to connection request.
         * This contains the title for adhoc channels. */
        int type = TYPE_PUBLIC;
        QString title = "";
        if (channelName.startsWith("ADH-")) {
            type = TYPE_ADHOC;
            title = o["title"].toString();
        }

        /* The mode will be set when ICH is given. */
        ZChannel *channel = new ZChannel(persondb, type, channelName, title);
        publicChannels[channelName] = channel;
        setupChannel(channel, false);

        /* Let self be added when the ICH comes up. */
        return;
    }

    ZPerson *person;
    ASSIGN_AND_CHECK("JCH", person, persondb->find(personName));

    ZChannelInfo *info = new ZChannelInfo(person, channel);
    channel->addInfo(info);
}

void ZMainWindow::processLCH(QJsonObject o)
{
    /* Leave channel. */
    /* LCH { "channel": string, "character": character } */
    QString channelName = o["channel"].toString();
    QString personName = o["character"].toString();
    ZChannel *channel;
    ZPerson *person;
    ASSIGN_AND_CHECK("LCH", channel, publicChannels.value(channelName, nullptr))
    ASSIGN_AND_CHECK("LCH", person, persondb->find(channelName))
    ZChannelInfo *info = person->listenerFor(channel);

    if (info) {
        channel->deleteInfo(info);
    }
}

void ZMainWindow::processLIS(QJsonObject o)
{
    /* List of online characters. Might be part of several blocks. */
    /* LIS {"characters": [["Alexandrea", "Female", "online", ""], ...]]} */
    QJsonArray all = o["characters"].toArray();

    foreach (QJsonValue v, all) {
        QJsonArray ja = v.toArray();

        QString personName = ja.at(0).toString();
        ZGender gender = (ZGender) genderMapping.value(ja.at(1).toString(), GENDER_OFFLINE);
        ZStatus status = (ZStatus) statusMapping.value(ja.at(2).toString(), STATUS_ONLINE);
        QString messageString = ja.at(3).toString();

        ZPerson *p = persondb->find(personName);

        if (p == nullptr) {
            int flags = persondb->flagsFor(personName);
            p = new ZPerson(flags, gender, status, personName, messageString);
            persondb->insert(personName, p);
            if (flags & PERSON_FRIEND)
                sendOnOffMessage(p, true);
        }
        else {
            LOG("[LIS]: Unable to find '%s'.\n", personName.toStdString().c_str());
        }
    }

    if (persondb->getSelf() == nullptr)
        persondb->setSelf(persondb->find(persondb->getSelfName()));
}

void ZMainWindow::processLRP(QJsonObject o)
{
    /* Roleplay ad from user in channel. */
    /* LRP { "character": string, "message": string, "channel": string } */
    QString personName = o["character"].toString();
    QString channelName = o["channel"].toString();
    ZChannel *channel;
    ZPerson *person;
    ASSIGN_AND_CHECK("MSG", channel, publicChannels.value(channelName, nullptr))
    ASSIGN_AND_CHECK("MSG", person, persondb->find(personName))

    int flags = person->allFlags();
    if ((flags & PERSON_NOT_IGNORED) == false ||
        (flags & PERSON_INTERESTED) == false)
        return;

    bool filter = false;
    QString message = ZParser::parse(o["message"].toString(), settings, &filter);
    QString fullMessage;

    if (filter == true)
        message = "(message filtered by blocker.)";

    fullMessage = buildMessageFrom(person, message, TYPE_RP_AD);
    channel->pushMessage(fullMessage);

    if (filter == true && settings->getBlockerMarkNotInterested() == true)
        onAction(ACTION_NOT_INTERESTED, personName);
}

void ZMainWindow::processMSG(QJsonObject o)
{
    /* Message from user in channel */
    /* MSG { "character": string, "message": string, "channel": string } */
    QString personName = o["character"].toString();
    QString channelName = o["channel"].toString();
    ZChannel *channel;
    ZPerson *person;
    ASSIGN_AND_CHECK("MSG", channel, publicChannels.value(channelName, nullptr))
    ASSIGN_AND_CHECK("MSG", person, persondb->find(personName))

    if (person->getFlag(PERSON_NOT_IGNORED) == false)
        return;

    QString fullMessage = buildPoseFrom(person, o["message"].toString());
    channel->pushMessage(fullMessage);
}

void ZMainWindow::processRMO(QJsonObject o)
{
    /* Change room mode. */
    /* RMO {"mode": enum, "channel": string} */
    QString channelName = o["channel"].toString();
    QString modeString = o["mode"].toString();
    ZChannel *channel;
    ASSIGN_AND_CHECK("RMO", channel, publicChannels.value(channelName, nullptr))
    int mode;

    if (modeString == "both") {
        mode = MODE_ADS | MODE_CHAT;
    }
    else if (modeString == "chat") {
        mode = MODE_CHAT;
    }
    else if (modeString == "ads") {
        mode = MODE_ADS;
    }
    else {
        return;
    }

    channel->setMode(mode);
}

void ZMainWindow::processRTB(QJsonObject o)
{
    /* Real-time bridge. */
    /* RTB { "type": string, "character": string } */
    QString type = o["type"].toString();
    QString personName = o["character"].toString();
    QString message = "";

    if (type == "friendadd") {
        message = QString("%1 is now a friend of one of your characters.")
            .arg(personName);
    }
    else if (type == "friendremove") {
        message = QString("%1 removed as a friend from one of your characters.")
            .arg(personName);
    }
    else {
        LOG("Unknown real-time message '%s'.\n", type.toStdString().c_str());
        return;
    }

    message = buildStampedMessage(message);
    currentPanel->pushMessage(message);
    if (currentPanel != console)
        console->pushMessage(message);
}

void ZMainWindow::processNLN(QJsonObject o)
{
    /* A user connected. */
    /* NLN {"identity": "Character Name", "gender": genderenum, "status": statusenum} */
    QString personName = o["identity"].toString();
    ZGender gender = (ZGender) genderMapping.value(o["gender"].toString(), GENDER_NONE);
    ZStatus status = STATUS_ONLINE; /* What else would it be? */

    ZPerson *person = persondb->find(personName);
    if (person == nullptr) {
        int flags = persondb->flagsFor(personName);
        person = new ZPerson(flags, gender, status, personName, "");
        persondb->insert(personName, person);
        if (flags & PERSON_FRIEND)
            sendOnOffMessage(person, true);

        if (flags & PERSON_HAS_PM) {
            ZChannel *channel = privateChannels.value(personName, nullptr);
            if (channel) {
                channel->getPanelItem()->setIcon(ZCache::iconForId[TYPING_CLEAR]);
            }
        }
    }
}

void ZMainWindow::processORS(QJsonObject o)
{
    /* Gives a list of open private rooms. */
    /* ORS {["name": "channelname", "title":"User title", "count":value]...} */
    QList<QStringList> channelList = QList<QStringList>();
    QJsonArray ja = o["channels"].toArray();
    QChar fillChar('0');

    foreach (QJsonValue v, ja) {
        QJsonObject channel = v.toObject();
        int rawCount = channel["characters"].toInt();

        if (rawCount == 0)
            continue; /* It's dead. */

        QString count = QString("%1").arg(rawCount, 3, 10, fillChar);
        QString name = channel["name"].toString();
        QString title = channel["title"].toString();

        channelList.append(QStringList() << count << name << title);
    }

    if (channelDialog)
        channelDialog->updateAdhocRooms(channelList);
}

void ZMainWindow::processPIN()
{
    /* Server ping. Respond in kind. */
    /* Send it like this so it's logged. */
    onSocketMessage("PIN");
}

void ZMainWindow::processPRI(QJsonObject o)
{
    /* Private message */
    /* PRI {"character": string, "message": string} */
    QString personName = o["character"].toString();
    ZPerson *person;
    ASSIGN_AND_CHECK("PRI", person, persondb->find(personName))

    if (person->getFlag(PERSON_NOT_IGNORED) == false)
        return;

    persondb->setMarker(personName, PERSON_HAS_PM);

    ZChannel *channel = privateChannels.value(personName, nullptr);
    if (channel == nullptr) {
        channel = new ZChannel(persondb, TYPE_PM | MODE_CHAT, personName);
        privateChannels[personName] = channel;
        setupChannel(channel, false);
    }
    else {
        QListWidgetItem *item = channel->getPanelItem();
        item->setHidden(false);
        item->setIcon(ZCache::iconForId[TYPING_CLEAR]);
    }

    QSound::play(ZCache::soundNameForId[SOUND_PM]);

    channel->pushAndLogMessage(buildPoseFrom(person, o["message"].toString()));
}

void ZMainWindow::processSTA(QJsonObject o)
{
    /* A user changed their status. */
    /* STA { status: "status", character: "channel", statusmsg:"statusmsg" } */
    QString personName = o["character"].toString();
    ZPerson *person;
    ASSIGN_AND_CHECK("STA", person, persondb->find(personName))

    ZStatus status = (ZStatus) statusMapping.value(o["status"].toString(), STATUS_ONLINE);
    QString statusMessage = o["statusmsg"].toString();

    person->statusUpdate(status, statusMessage);

    if (person->getFlag(PERSON_FRIEND)) {
        QString m = QString("is now %1").arg(statusString(status));
        if (statusMessage.isEmpty() == false)
            m += QString(" (%1)").arg(statusMessage);

        m += ".";
        console->pushMessage(buildMessageFrom(person, m, TYPE_INFO));
    }
}

void ZMainWindow::processSYS(QJsonObject o)
{
    /* Message from server. May have a channel. */
    /* SYS { "message": string, "channel": string } */
    QString channelName = o["channel"].toString();
    QString message = buildStampedMessage(o["message"].toString()); /* todo: ZParse? */
    ZChannel *channel = publicChannels.value(channelName, nullptr);
    if (channel) {
        channel->pushMessage(message);
    }
    console->pushMessage(message);
}

void ZMainWindow::processTPN(QJsonObject o)
{
    /* Typing status notification. */
    /* TPN { "character": string, "status": enum } */
    QString personName = o["character"].toString();
    QString status = o["status"].toString();
    ZPerson *person;
    ZChannel *channel;

    ASSIGN_AND_CHECK("TPN", person, persondb->find(personName))
    ASSIGN_AND_CHECK("TPN", channel, privateChannels.value(personName, nullptr))

    int id = 0;

    if (status == "typing") {
        id = TYPING_IN_PROGRESS;
    }
    else if (status == "paused") {
        id = TYPING_PAUSED;
    }
    else if (status == "clear") {
        id = TYPING_CLEAR;
    }
    else {
        LOG("[TPN]: Invalid status '%s'.\n", status.toStdString().c_str());
        return;
    }

    channel->getPanelItem()->setIcon(ZCache::iconForId[id]);
}

void ZMainWindow::processVAR(QJsonObject o)
{
    /* Server variable */
    /* VAR { "variable": string, "value": int/float } */
    (void) o;
}
