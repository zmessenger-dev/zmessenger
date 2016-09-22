#include "zcache.h"

QBrush ZCache::brushForGender[GENDER_OFFLINE + 1];
QString ZCache::hexForGender[GENDER_OFFLINE + 1];
QIcon ZCache::iconForId[TYPING_PAUSED + 1];
QString ZCache::soundNameForId[SOUND_PM + 1];

void ZCache::init()
{
    brushForGender[GENDER_CUNT_BOY] = QColor(0x00, 0xdd, 0xdd);
    brushForGender[GENDER_FEMALE] = QColor(0xff, 0xbb, 0xdd);
    brushForGender[GENDER_HERM] = Qt::red;
    brushForGender[GENDER_MALE] = QColor(0x88, 0x99, 0xff);
    brushForGender[GENDER_MALE_HERM] = Qt::blue;
    brushForGender[GENDER_NONE] = Qt::darkGray;
    brushForGender[GENDER_SHEMALE] = QColor(0xff, 0x00, 0x88);
    brushForGender[GENDER_TRANSGENDER] = Qt::yellow;
    brushForGender[GENDER_OFFLINE] = Qt::white;

    hexForGender[GENDER_CUNT_BOY] = "#00dddd";
    hexForGender[GENDER_FEMALE] = "#ffbbdd";
    hexForGender[GENDER_HERM] = "#ff0000";
    hexForGender[GENDER_MALE] = "#8899ff";
    hexForGender[GENDER_MALE_HERM] = "#0000ff";
    hexForGender[GENDER_NONE] = "#a9a9a9";
    hexForGender[GENDER_SHEMALE] = "#ff0088";
    hexForGender[GENDER_TRANSGENDER] = "#ffff00";
    hexForGender[GENDER_OFFLINE] = "#ffffff";

    iconForId[STATUS_AWAY] = QIcon(":/img/away.png");
    iconForId[STATUS_BUSY] = QIcon(":/img/busy.png");
    iconForId[STATUS_DND] = QIcon(":/img/dnd.png");
    iconForId[STATUS_LOOKING] = QIcon(":/img/looking.png");
    iconForId[STATUS_ONLINE] = QIcon(":/img/online.png");
    /* TYPING_CLEAR == STATUS_ONLINE; do nothing for it. */
    iconForId[TYPING_IN_PROGRESS] = QIcon(":/img/typing-in-progress.png");
    iconForId[TYPING_PAUSED] = QIcon(":/img/typing-paused.png");
    iconForId[TYPING_OFFLINE] = QIcon(":/img/offline.png");

    soundNameForId[SOUND_CHAT] = ":/snd/chat.wav";
    soundNameForId[SOUND_PM] = ":/snd/pm.wav";
}
