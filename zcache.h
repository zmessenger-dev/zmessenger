#ifndef ZCACHE_H
#define ZCACHE_H

#include <QBrush>
#include <QIcon>

#include "zperson.h"

enum ZSound {
    SOUND_CHAT,
    SOUND_PM,
};

class ZCache
{
public:
    static void init();

    static QBrush brushForGender[GENDER_OFFLINE + 1];
    static QString hexForGender[GENDER_OFFLINE + 1];
    static QIcon iconForId[TYPING_PAUSED + 1];
    static QString soundNameForId[SOUND_PM + 1];
};

#endif
