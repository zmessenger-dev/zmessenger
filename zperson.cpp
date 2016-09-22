#include "zcache.h"
#include "zchannel.h"
#include "zperson.h"

ZPerson::ZPerson(int flags_, ZGender gender_, ZStatus status_, QString name_,
QString messageString_):
    compareFlags(flags_ & ~PERSON_MARKERS),
    flags(flags_),
    gender(gender_),
    status(status_),
    name(name_),
    messageString(messageString_),
    listeners()
{

}

ZChannelInfo *ZPerson::listenerFor(ZChannel *channel)
{
    ZChannelInfo *result = nullptr;

    foreach (ZChannelInfo *i, listeners) {
        if (i->channel == channel) {
            result = i;
            break;
        }
    }

    return result;
}

void ZPerson::setFlag(int f)
{
    foreach (ZChannelInfo *info, listeners) {
        info->channel->removeInfo(info);
        info->infoFlags |= f;
    }

    flags |= f;

    foreach (ZChannelInfo *info, listeners) {
        info->channel->addInfo(info);
    }
}

void ZPerson::unsetFlag(int f)
{
    foreach (ZChannelInfo *info, listeners) {
        info->channel->removeInfo(info);
        info->infoFlags &= ~f;
    }

    flags &= ~f;

    foreach (ZChannelInfo *info, listeners) {
        info->channel->addInfo(info);
    }
}

void ZPerson::statusUpdate(ZStatus status, QString message)
{
    foreach (ZChannelInfo *info, listeners) {
        QListWidgetItem *item = info->channelItem;
        item->setIcon(ZCache::iconForId[status]);
    }

    status = status;
    messageString = message;
}

ZChannelInfo::ZChannelInfo(ZPerson *person_, ZChannel *channel_):
    infoFlags(person_->compareFlags),
    person(person_),
    channel(channel_)
{
    person->listeners.append(this);
}

ZChannelInfo::~ZChannelInfo()
{
    person->listeners.removeOne(this);
}

bool ZChannelInfo::compareInfo(ZChannelInfo *left, ZChannelInfo *right)
{
    if (right->infoFlags != left->infoFlags) {
        return right->infoFlags < left->infoFlags;
    }

    return left->person->name < right->person->name;
}
