#include <QDialogButtonBox>
#include <QHeaderView>
#include <QTabWidget>
#include <QVBoxLayout>

#include "zchannel.h"
#include "zchanneldialog.h"

#define CustomSortRole (Qt::UserRole + 1)

ZChannelDialog::ZChannelDialog()
{
    QVBoxLayout *layout = new QVBoxLayout;
    QTabWidget *tabWidget = new QTabWidget;

    publicRoomTree = createTree();
    publicRoomTree->setColumnCount(2);
    publicRoomTree->setHeaderLabels(QStringList() << "Count" << "Title");
    tabWidget->addTab(publicRoomTree, QIcon(":/img/channel-public.png"), "Public");

    connect(publicRoomTree, &QTreeWidget::itemDoubleClicked,
    this, &ZChannelDialog::onPublicRoomSelected);

    adhocRoomTree = createTree();
    adhocRoomTree->setColumnCount(3);
    adhocRoomTree->setHeaderLabels(QStringList() << "Count" << "" << "Title");
    adhocRoomTree->hideColumn(1);
    tabWidget->addTab(adhocRoomTree, QIcon(":/img/channel-adhoc.png"), "Adhoc");

    connect(adhocRoomTree, &QTreeWidget::itemDoubleClicked,
    this, &ZChannelDialog::onAdhocRoomSelected);

    layout->addWidget(tabWidget);
    setMinimumWidth(350);
    setLayout(layout);
    setWindowTitle("Channel Selector");
}

QTreeWidget *ZChannelDialog::createTree()
{
    QTreeWidget *result = new QTreeWidget;

    result->setAllColumnsShowFocus(true);
    result->setIndentation(0);
    result->setSelectionMode(QAbstractItemView::SingleSelection);
    result->setDragEnabled(false);
    result->setUniformRowHeights(true);

    QHeaderView *header = result->header();
    header->setSortIndicatorShown(true);
    header->setStretchLastSection(true);

    return result;
}

void ZChannelDialog::onAdhocRoomSelected(QTreeWidgetItem *item, int)
{
    emit join(ACTION_JOIN, item->text(1));
}

void ZChannelDialog::onPublicRoomSelected(QTreeWidgetItem *item, int)
{
    emit join(ACTION_JOIN, item->text(1));
}

void ZChannelDialog::updateAdhocRooms(QList<QStringList> rooms)
{
    foreach (QStringList sl, rooms) {
        adhocRoomTree->addTopLevelItem(new QTreeWidgetItem(sl));
    }

    adhocRoomTree->setSortingEnabled(true);
}

void ZChannelDialog::updatePublicRooms(QList<QStringList> rooms)
{
    foreach (QStringList sl, rooms) {
        publicRoomTree->addTopLevelItem(new QTreeWidgetItem(sl));
    }

    publicRoomTree->setSortingEnabled(true);
}
