#ifndef ZSETTINGS_H
#define ZSETTINGS_H

#include <QSet>
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonObject>

class ZSettings
{
public:
    ZSettings(QString);
    void save();
    void changed() { shouldSave = true; }
    bool getShouldSave() { return shouldSave; }
    QSet<QString> getNotInterested() { return notInterested; }

    void addNotInterested(QString s) {
        shouldSave = true;
        notInterested.insert(s);
    }
    void removeNotInterested(QString s) {
        shouldSave = true;
        notInterested.remove(s);
    }

    bool getBlockerEnabled() { return blockerEnabled; }
    bool getBlockerMarkNotInterested() { return blockerMarkNotInterested; }
    int getBlockerCharacterMax() { return blockerCharacterMax; }
    int getBlockerLineMax() { return blockerLineMax; }
    int getBlockerTagMax() { return blockerTagMax; }

#define PROP_SETTER(type, name) \
    void setBlocker##name(type a) { \
        if (blocker##name != a) { \
            blocker##name = a; \
            shouldSave = true; \
        } \
    } \

    PROP_SETTER(bool, Enabled)
    PROP_SETTER(bool, MarkNotInterested)
    PROP_SETTER(int, CharacterMax)
    PROP_SETTER(int, LineMax)
    PROP_SETTER(int, TagMax)
#undef PROP_SETTER

private:
    void readBlockerPrefs(QJsonObject);
    QStringList readJsonToStringList(QJsonObject, QString);
    void writeBlockerPrefs(QJsonObject &);
    QJsonArray writeStringListToJson(QStringList);

    bool shouldSave;
    bool blockerEnabled;
    bool blockerMarkNotInterested;
    int blockerCharacterMax;
    int blockerLineMax;
    int blockerTagMax;
    QString filename;
    QSet<QString> notInterested;
};

#endif
