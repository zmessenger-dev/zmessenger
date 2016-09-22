#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>

#include "zblockerdialog.h"

ZBlockerDialog::ZBlockerDialog(ZSettings *settings_):
    settings(settings_)
{
    QGridLayout *layout = new QGridLayout;

    enabled = new QCheckBox;
    markNotInterested = new QCheckBox;

    enabled->setChecked(settings->getBlockerEnabled());
    markNotInterested->setChecked(settings->getBlockerMarkNotInterested());

    characterMax = new QLineEdit;
    characterMax->setText(QString::number(settings->getBlockerCharacterMax()));
    characterMax->setValidator(new QIntValidator());

    lineMax = new QLineEdit;
    lineMax->setText(QString::number(settings->getBlockerLineMax()));
    lineMax->setValidator(new QIntValidator());

    tagMax = new QLineEdit;
    tagMax->setText(QString::number(settings->getBlockerTagMax()));
    tagMax->setValidator(new QIntValidator());

    QDialogButtonBox *box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(box, &QDialogButtonBox::accepted,
    this, &ZBlockerDialog::onAccepted);

    connect(box, &QDialogButtonBox::rejected,
    this, &QDialog::reject);

    layout->addWidget(new QLabel("Enabled:"), 0, 0);
    layout->addWidget(enabled, 0, 1);
    layout->addWidget(new QLabel("Mark not interested:"), 1, 0);
    layout->addWidget(markNotInterested, 1, 1);
    layout->addWidget(new QLabel("Max # of characters:"), 2, 0);
    layout->addWidget(characterMax, 2, 1);
    layout->addWidget(new QLabel("Max # of lines:"), 3, 0);
    layout->addWidget(lineMax, 3, 1);
    layout->addWidget(new QLabel("Max # of tags:"), 4, 0);
    layout->addWidget(tagMax, 4, 1);
    layout->addWidget(box, 5, 1);

    setLayout(layout);
    setWindowTitle("ZMessenger Blocker Preferences");
}

void ZBlockerDialog::onAccepted()
{
    settings->setBlockerEnabled(enabled->isChecked());
    settings->setBlockerMarkNotInterested(markNotInterested->isChecked());
    settings->setBlockerCharacterMax(characterMax->text().toInt());
    settings->setBlockerLineMax(lineMax->text().toInt());
    settings->setBlockerTagMax(tagMax->text().toInt());
    accept();
}
