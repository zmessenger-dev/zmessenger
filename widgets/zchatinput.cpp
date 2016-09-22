#include "zchatinput.h"
#include "zperson.h"

ZChatInput::ZChatInput()
{
    timerStarted = false;
}

void ZChatInput::startTimer()
{
    tracker.start();
    timerStarted = true;
    isPaused = true;

    typingTimer = new QTimer;

    connect(typingTimer, &QTimer::timeout,
    this, &ZChatInput::onTimer);

    connect(this, &QPlainTextEdit::textChanged,
    this, &ZChatInput::onTextChanged);

    typingTimer->start(1000);
}

void ZChatInput::onTimer()
{
    if (isPaused == false && tracker.elapsed() > 3000) {
        emit typingStartStop(false);
        isPaused = true;
    }
}

void ZChatInput::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();

    if (key == Qt::Key_Return || key == Qt::Key_Return) {
        if (e->modifiers() == Qt::NoModifier) {
            e->ignore();
            emit enterPressed();
            if (timerStarted) {
                tracker.start();
                isPaused = true; /* Prevent clear becoming pause after idle. */
            }

            return;
        }
        else if (e->modifiers() == Qt::ShiftModifier) {
            insertPlainText("\n");
            e->ignore();
            if (timerStarted) {
                tracker.start();
                if (isPaused) {
                    emit typingStartStop(true);
                    isPaused = false;
                }
            }

            return;
        }
    }

    QPlainTextEdit::keyPressEvent(e);
}

void ZChatInput::onTextChanged()
{
    tracker.start();
    if (toPlainText().size() == 0) {
        emit typingClear();
        isPaused = true; /* Prevent clear becoming pause after idle. */
    }
    else if (isPaused) {
        emit typingStartStop(true);
        isPaused = false;
    }
}
