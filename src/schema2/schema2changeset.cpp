#include "schema2changeset.h"

Schema2ChangeSet::Schema2ChangeSet(QObject *parent)
    : QAbstractTableModel{parent}
{

}

#include "history.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

bool Schema2ChangeSet::execute(const QString& connectionName)
{
    History* history = History::instance();
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    bool ok = true;
    for(int row=0;row<mItems.size();row++) {
        QSqlQuery q(db);
        QString query = mItems[row].query;
        QString error;
        if (!q.exec(query)) {
            error = q.lastError().text();
        } else {
            history->addQuery(connectionName, query);
            mItems[row].handler();
        }

        mItems[row].error = error;
    }
    return ok;
}

bool Schema2ChangeSet::isEmpty() const
{
    return mItems.isEmpty();
}

int Schema2ChangeSet::rowCount(const QModelIndex &parent) const
{
    return mItems.size();
}

int Schema2ChangeSet::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant Schema2ChangeSet::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch(index.column()) {
        case 0: return mItems[index.row()].query;
        case 1: return mItems[index.row()].error;
        }
    }
    return QVariant();
}

QVariant Schema2ChangeSet::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QStringList header = {"Query", "Error"};
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return header.value(section);
    }
    return QVariant();
}
