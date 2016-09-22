#include <QFile>
#include <QJsonDocument>

#include "zsettings.h"

ZSettings::ZSettings(QString name):
    shouldSave(false),
    blockerEnabled(false),
    blockerMarkNotInterested(false),
    blockerCharacterMax(1024),
    blockerLineMax(8),
    blockerTagMax(10),
    filename(name),
    notInterested()
{
    QFile f(name);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text) == false) {
        save();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (doc.isEmpty()) {
        return;
    }

    QJsonObject o = doc.object();
    readBlockerPrefs(o);
    notInterested = QSet<QString>::fromList(readJsonToStringList(o, "not-interested"));
}

void ZSettings::save()
{
    QFile f(filename);
    if (f.open(QIODevice::WriteOnly) == false) {
        return;
    }

    QJsonObject writeObj;
    QStringList sl = QList<QString>::fromSet(notInterested);
    /* A sorted list looks nicer. */
    qSort(sl);

    writeBlockerPrefs(writeObj);
    writeObj["not-interested"] = writeStringListToJson(sl);

    QJsonDocument saveDoc(writeObj);
    f.write(saveDoc.toJson());
    f.close();
}

void ZSettings::writeBlockerPrefs(QJsonObject &o)
{
    QJsonObject blocker;

    blocker["enabled"] = blockerEnabled;
    blocker["mark-not-interested"] = blockerMarkNotInterested;
    blocker["character-max"] = blockerCharacterMax;
    blocker["line-max"] = blockerLineMax;
    blocker["tag-max"] = blockerTagMax;

    o["adblocker"] = blocker;
}

QJsonArray ZSettings::writeStringListToJson(QStringList sl)
{
    QJsonArray ja;

    for (int i = 0;i < sl.size();i++) {
        ja.append(QJsonValue(sl.at(i)));
    }

    return ja;
}

void ZSettings::readBlockerPrefs(QJsonObject o)
{
    QJsonObject blocker = o["adblocker"].toObject();

    blockerEnabled = blocker["enabled"].toBool(false);
    blockerMarkNotInterested = blocker["mark-not-interested"].toBool(false);
    blockerCharacterMax = blocker["character-max"].toInt(1024);
    blockerLineMax = blocker["line-max"].toInt(8);
    blockerTagMax = blocker["tag-max"].toInt(10);
}

QStringList ZSettings::readJsonToStringList(QJsonObject o, QString s)
{
    QStringList sl;
    QJsonArray a = o[s].toArray();

    for (int i = 0;i < a.size();i++) {
        sl.append(a.at(i).toString());
    }

    return sl;
}
