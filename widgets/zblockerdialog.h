#ifndef ZBLOCKERDIALOG_H
#define ZBLOCKERDIALOG_H

#include <QCheckBox>
#include <QDialog>
#include <QLineEdit>

#include "zsettings.h"

class ZBlockerDialog: public QDialog
{
    Q_OBJECT

public:
    ZBlockerDialog(ZSettings *);

private slots:
    void onAccepted();

private:
    ZSettings *settings;

    QCheckBox *enabled;
    QCheckBox *markNotInterested;
    QLineEdit *characterMax;
    QLineEdit *lineMax;
    QLineEdit *tagMax;
};

#endif
