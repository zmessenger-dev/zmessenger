#include <QList>
#include <QUrl>

#include "zparser.h"
#include "zsettings.h"

enum ZTag {
    TAG_BOLD,
    TAG_CHANNEL,
    TAG_COLOR,
    TAG_ICON,
    TAG_INVALID,
    TAG_ITALIC,
    TAG_NOPARSE,
    TAG_SESSION,
    TAG_STRIKE,
    TAG_TEXT,
    TAG_UNDERLINE,
    TAG_USER,
    TAG_URL,
};

class ZTagData
{
public:
    ZTagData(ZTag tag_, QString attr_):
        tag(tag_),
        attr(attr_),
        body("")
    {}

    ZTag tag;
    QString attr;
    QString body;
};

QStringList ZParser::allowedColors;

void ZParser::init()
{
    allowedColors = QStringList()
        << "black"
        << "blue"
        << "cyan"
        << "gray"
        << "green"
        << "magenta"
        << "pink"
        << "red"
        << "white"
        << "yellow";
}

bool ZParser::checkAttrValidFor(int tag, QString attr)
{
    bool result = true;

    switch ((ZTag) tag) {
        case TAG_BOLD:
        case TAG_CHANNEL:
        case TAG_ICON:
        case TAG_ITALIC:
        case TAG_NOPARSE:
        case TAG_STRIKE:
        case TAG_UNDERLINE:
            result = attr.isEmpty();
            break;
        case TAG_COLOR:
            result = allowedColors.contains(attr);
            break;
        case TAG_SESSION:
            /* The ' check prevents injection. */
            result = attr.isEmpty() == false && attr.indexOf("'") == -1;
            break;
        case TAG_URL:
            /* Check for injection and spoofing channels. */
            result = attr.isEmpty() ||
                     (attr.indexOf("'") == -1 && attr.startsWith("session://") == false);
            break;
        case TAG_USER:
            result = attr.isEmpty();
            break;
        /* These two are impossible, but complete the switch. */
        case TAG_INVALID:
        case TAG_TEXT:
            break;
    }

    return result;
}

QString ZParser::finishTag(ZTagData *data, bool noparse)
{
    QString result;
    if (noparse == true) {
        QString attr = data->attr;
        if (attr.isEmpty()) {
            result = QString("[%1]%2[/%1]")
                .arg(textForTag(data->tag))
                .arg(data->body);
        }
        else {
            result = QString("[%1=%2]%3[/%1]")
                .arg(textForTag(data->tag))
                .arg(data->attr)
                .arg(data->body);
        }
    }

    switch (data->tag) {
        case TAG_BOLD:
            result = QString("<b>%1</b>").arg(data->body);
            break;
        case TAG_ITALIC:
            result = QString("<i>%1</i>").arg(data->body);
            break;
        case TAG_STRIKE:
            result = QString("<s>%1</s>").arg(data->body);
            break;
        case TAG_UNDERLINE:
            result = QString("<u>%1</u>").arg(data->body);
            break;
        case TAG_COLOR:
            result = QString("<span style='color: %1'>%2</span>")
                .arg(data->attr)
                .arg(data->body);
            break;
        case TAG_URL: {
            QString urlString = data->attr;
            if (urlString.isEmpty())
                urlString = data->body;

            QUrl url(urlString);

            if (url.isRelative() || url.isLocalFile() || url.isValid() == false)
                result = QString("(Bad URL: '%1')").arg(urlString);
            else
                result = QString("<a href='%1'><span style='color: #ADD8E6;'>↗%2 [%3]</span></a>")
                    .arg(data->attr)
                    .arg(data->body)
                    .arg(url.host());

            break;
        }
        case TAG_USER:
            result = QString("<a href='http://f-list.net/c/%1'>▽%2</a>")
                .arg(data->body)
                .arg(data->attr);
            break;
        case TAG_SESSION:
            result = QString("<a href='session://%1'>⚷%2</a>")
                .arg(data->body)
                .arg(data->attr);
            break;
        case TAG_CHANNEL:
            result = QString("<a href='session://%1'>▦%1</a>")
                .arg(data->body);
            break;
        case TAG_NOPARSE:
            result = QString("[noparse]%1[/noparse]")
                .arg(data->body);
            break;
        /* Intentionally NOT supporting icon because it's abused too much. */
        case TAG_ICON:
            result = QString("[icon]%1[/icon").arg(data->body);
            break;
        /* These two are impossible, but complete the switch. */
        case TAG_INVALID:
        case TAG_TEXT:
            result = "";
            break;
    }

    return result;
}

int ZParser::tagForText(QString input)
{
    int result = TAG_INVALID;

    if (input == "b")
        result = TAG_BOLD;
    else if (input == "i")
        result = TAG_ITALIC;
    else if (input == "s")
        result = TAG_STRIKE;
    else if (input == "u")
        result = TAG_UNDERLINE;
    else if (input == "channel")
        result = TAG_CHANNEL;
    else if (input == "session")
        result = TAG_SESSION;
    else if (input == "url")
        result = TAG_URL;
    else if (input == "color")
        result = TAG_COLOR;
    else if (input == "user")
        result = TAG_USER;
    else if (input == "noparse")
        result = TAG_NOPARSE;
    else if (input == "icon")
        result = TAG_ICON;

    return result;
}

QString ZParser::textForTag(int tag)
{
    QString result = "";

    switch ((ZTag) tag) {
        case TAG_BOLD:      result = "b";       break;
        case TAG_CHANNEL:   result = "channel"; break;
        case TAG_COLOR:     result = "color";   break;
        case TAG_ICON:      result = "icon";    break;
        case TAG_ITALIC:    result = "i";       break;
        case TAG_NOPARSE:   result = "noparse"; break;
        case TAG_SESSION:   result = "session"; break;
        case TAG_STRIKE:    result = "s";       break;
        case TAG_UNDERLINE: result = "u";       break;
        case TAG_URL:       result = "url";     break;
        case TAG_USER:      result = "user";    break;
        case TAG_INVALID:                       break;
        case TAG_TEXT:                          break;
    }

    return result;
}

QString ZParser::parse(QString input, ZSettings *settings, bool *filter)
{
    /* Create an empty stack with a fake 'text' tag to capture toplevel text. */
    QList<ZTagData *> tagStack = QList<ZTagData *>();
    tagStack.append(new ZTagData(TAG_TEXT, ""));

    bool noparse = false;
    ZTagData *current = tagStack.last();
    QChar slash = QChar('/');
    int lastIndex = 0;
    int lineCount = 1, tagCount = 0;
    int charCount = 0;

    if (settings)
        charCount = input.size();

    if (input.indexOf("\n") != -1) {
        /* It's easier to count a single character versus a sequence. */
        if (settings != nullptr)
            lineCount = input.count("\n");

        input = input.replace("\n", "<br>");
    }

    while (1) {
        int nextOpen = input.indexOf("[", lastIndex);
        if (nextOpen == -1) {
            if (tagStack.size() == 1) {
                QString slice = input.mid(lastIndex, nextOpen - lastIndex);
                current->body += slice;
            }
            /* else it's broken, don't bother. */

            break;
        }

        QString slice = input.mid(lastIndex, nextOpen - lastIndex);
        current->body += slice;

        int nextClose = input.indexOf("]", nextOpen);
        if (nextClose == -1)
            break; /* [ with no ] is wrong. */

        QString tagString;
        ZTag tag = TAG_TEXT;

        if (input[nextOpen + 1] == slash) {
            tagString = input.mid(nextOpen + 2, nextClose - nextOpen - 2);
            tag = (ZTag) tagForText(tagString);
            if (tag != current->tag)
                break; /* Not closing the current tag. */
            else if (tag == TAG_NOPARSE)
                noparse = false;

            /* Out of bounds not possible: There's no close for TAG_TEXT. */
            ZTagData *recv = tagStack[tagStack.size() - 2];

            recv->body += finishTag(current, noparse);
            tagStack.removeLast();
            delete current;
            current = recv;
        }
        else {
            tagCount++;
            /* Tags are either [xyz] or [xyz=someattr]. */
            int eqIndex = input.indexOf("=", nextOpen);
            QString attr;

            if (eqIndex != -1 && eqIndex < nextClose) {
                attr = input.mid(eqIndex + 1, nextClose - eqIndex - 1);
                tagString = input.mid(nextOpen + 1, eqIndex - nextOpen - 1);
            }
            else {
                tagString = input.mid(nextOpen + 1, nextClose - nextOpen - 1);
                attr = "";
            }

            tag = (ZTag) tagForText(tagString);
            if (tag == TAG_INVALID)
                break; /* Bad tag; give up. */
            else if (tag == TAG_NOPARSE) {
                if (noparse == true)
                    break;

                noparse = true;
            }

            if (checkAttrValidFor(tag, attr) == false)
                break; /* This is missing an attr or shouldn't have one. */

            ZTagData *newData = new ZTagData(tag, attr);
            tagStack.append(newData);
            current = newData;
        }

        lastIndex = nextClose + 1;
    }

    if (tagStack.size() == 1) {
        input = current->body;
        if (settings != nullptr &&
            (charCount > settings->getBlockerCharacterMax() ||
             lineCount > settings->getBlockerLineMax() ||
             tagCount > settings->getBlockerTagMax()))
            *filter = true;
    }
    /* else render the original (post \n replacement, that is). */

    while (tagStack.size())
        delete tagStack.takeLast();

    return input;
}
