#ifndef ZPERSONDB_H
#define ZPERSONDB_H

#include <QHash>

#include "zperson.h"
#include "zsettings.h"

class ZPersonDb
{
public:
    ZPersonDb(ZSettings *settings) {
        people = QHash<QString, ZPerson *>();
        flagbase = QHash<QString, int>();
        self = nullptr;
        selfName = "";

        foreach (QString ni, settings->getNotInterested()) {
            flagbase[ni] = PERSON_DEFAULT & ~PERSON_INTERESTED;
        }
    }

    int flagsFor(QString name) { return flagbase.value(name, PERSON_DEFAULT); }
    QString getSelfName() { return selfName; }
    ZPerson *getSelf() { return self; }
    void setSelf(ZPerson *s) { self = s; }
    void setSelfName(QString n) { selfName = n; }

    void setFlag(QString name, int flag) {
        ZPerson *p = people.value(name, nullptr);
        if (p) p->setFlag(flag);
        flagbase.insert(name, flagbase.value(name, PERSON_DEFAULT) | flag);
    }

    void unsetFlag(QString name, int flag) {
        ZPerson *p = people.value(name, nullptr);
        if (p) p->unsetFlag(flag);
        flagbase.insert(name, flagbase.value(name, PERSON_DEFAULT) & ~flag);
    }

    void setMarker(QString name, int flag) {
        ZPerson *p = people.value(name, nullptr);
        if (p) p->setMarker(flag);
        flagbase.insert(name, flagbase.value(name, PERSON_DEFAULT) | flag);
    }

    void unsetMarker(QString name, int flag) {
        ZPerson *p = people.value(name, nullptr);
        if (p) p->unsetMarker(flag);
        flagbase.insert(name, flagbase.value(name, PERSON_DEFAULT) & ~flag);
    }

    void insert(QString s, ZPerson *p) { people.insert(s, p); }
    void remove(QString name) { people.remove(name); }

    ZPerson *find(QString name) { return people.value(name, nullptr); }

private:
    QHash<QString, ZPerson *> people;
    QHash<QString, int> flagbase;
    QString selfName;
    ZPerson *self;
};

#endif
