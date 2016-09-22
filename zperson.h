#ifndef ZPERSON_H
#define ZPERSON_H

#include <QList>
#include <QString>

class QListWidgetItem;
class ZChannel;
class ZChannelInfo;

enum PersonFlag
{
    /* Flags up here are markers not used in sorting/broadcasts. */
    PERSON_HAS_PM      = 0x100,

    /* Admin = global chanop, mod = local chanop. */
    PERSON_ADMIN       = 0x20,
    PERSON_OWNER       = 0x10,
    PERSON_MOD         = 0x08,
    PERSON_FRIEND      = 0x04,
    PERSON_INTERESTED  = 0x02,
    PERSON_NOT_IGNORED = 0x01,

    PERSON_MARKERS     = (PERSON_HAS_PM),
    PERSON_POWER       = (PERSON_ADMIN | PERSON_OWNER | PERSON_MOD),
    PERSON_DEFAULT     = (PERSON_NOT_IGNORED | PERSON_INTERESTED),
};

enum ZGender
{
    GENDER_CUNT_BOY,
    GENDER_FEMALE,
    GENDER_HERM,
    GENDER_MALE,
    GENDER_MALE_HERM,
    GENDER_NONE,
    GENDER_SHEMALE,
    GENDER_TRANSGENDER,
    GENDER_OFFLINE,
};

enum ZStatus
{
    STATUS_AWAY = 0,
    STATUS_BUSY = 1,
    STATUS_DND = 2,
    STATUS_LOOKING = 3,
    STATUS_ONLINE = 4,
};

enum ZTyping
{
    /* Conflict intentional; They both use an 'empty' icon. */
    TYPING_CLEAR = 4,
    TYPING_IN_PROGRESS = 5,
    TYPING_OFFLINE = 6,
    TYPING_PAUSED = 7,
};

/* ZPerson represents the mainwindow's view of a person. */
class ZPerson
{
    friend class ZChannelInfo;

public:
    ZPerson(int, ZGender, ZStatus, QString, QString);

    int allFlags() { return flags; }
    bool getFlag(int f) { return flags & f; }
    void setFlag(int);
    void unsetFlag(int);
    void setMarker(int m) { flags |= m; }
    void unsetMarker(int m) { flags &= ~m; }
    ZChannelInfo *listenerFor(ZChannel *);
    void statusUpdate(ZStatus, QString);

    QString getName() { return name; }
    ZGender getGender() { return gender; }
    ZStatus getStatus() { return status; }
    QString getMessage() { return messageString; }
    QList<ZChannelInfo *> getListeners() { return listeners; }

private:
    int compareFlags;
    int flags;
    ZGender gender;
    ZStatus status;
    QString name;
    QString messageString;
    QList<ZChannelInfo *> listeners;
};

/* ZChannelInfo associates a ZPerson to a ZChannel. */
class ZChannelInfo
{
public:
    ZChannelInfo(ZPerson *, ZChannel *);
    ~ZChannelInfo();

    static bool compareInfo(ZChannelInfo *, ZChannelInfo *);

    int infoFlags;
    ZPerson *person;
    ZChannel *channel;
    QListWidgetItem *channelItem;
};

#endif
