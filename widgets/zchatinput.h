#ifndef ZCHATINPUT_H
#define ZCHATINPUT_H

#include <QPlainTextEdit>
#include <QTime>
#include <QTimer>

class ZChatInput: public QPlainTextEdit
{
    Q_OBJECT

public:
    ZChatInput();
    void startTimer();

signals:
    void enterPressed();
    void typingStartStop(bool);
    void typingClear();

private slots:
    void onTimer();
    void onTextChanged();

private:
    void keyPressEvent(QKeyEvent *);

    bool isPaused;
    bool timerStarted;
    QTime tracker;
    QTimer *typingTimer;
};

#endif
