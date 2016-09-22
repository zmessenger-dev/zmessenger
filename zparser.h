#ifndef ZPARSER_H
#define ZPARSER_H

#include <QString>
#include <QStringList>

class ZSettings;
class ZTagData;

class ZParser
{
public:
    static QString parse(QString, ZSettings * = nullptr, bool * = nullptr);
    static void init();

private:
    static QStringList allowedColors;

    static bool checkAttrValidFor(int, QString);
    static QString finishTag(ZTagData *, bool);
    static int tagForText(QString);
    static QString textForTag(int);
};

#endif
