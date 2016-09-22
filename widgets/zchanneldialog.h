#ifndef ZCHANNELDIALOG_H
#define ZCHANNELDIALOG_H

#include <QDialog>
#include <QList>
#include <QTreeWidget>
#include <QStringList>

class ZChannelDialog: public QDialog
{
    Q_OBJECT

public:
    ZChannelDialog();

    void updateAdhocRooms(QList<QStringList>);
    void updatePublicRooms(QList<QStringList>);

signals:
    void join(int, QString);

private slots:
    void onAdhocRoomSelected(QTreeWidgetItem *, int);
    void onPublicRoomSelected(QTreeWidgetItem *, int);

private:
    QTreeWidget *createTree();

    QTreeWidget *adhocRoomTree;
    QTreeWidget *publicRoomTree;
};

#endif
