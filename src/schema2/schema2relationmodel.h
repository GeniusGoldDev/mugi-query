#ifndef SCHEMA2RELATIONMODEL_H
#define SCHEMA2RELATIONMODEL_H

#include <QAbstractTableModel>

class Schema2RelationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum rows {
        row_child_table,
        row_child_column,
        row_parent_table,
        row_parent_column,
        rows_count
    };

    Schema2RelationModel(const QString& childTable, const QString& childColumn,
                         const QString& parentTable, const QString& parentColumn,
                         bool constrained, bool existing, QObject *parent = nullptr);

    QString childTable() const {
        return mChildTable;
    }

    QString childColumn() const {
        return mChildColumn;
    }

    QString parentTable() const {
        return mParentTable;
    }

    QString parentColumn() const {
        return mParentColumn;
    }

    void setChildColumn(const QString& name) {
        if (mChildColumn == name) {
            return;
        }
        mChildColumn = name;
        mExisting = false;
    }

    void setParentColumn(const QString& name) {
        if (mParentColumn == name) {
            return;
        }
        mParentColumn = name;
        mExisting = false;
    }

    bool hasPendingChanges() const {
        return mConstrained && !mExisting;
    }

protected:
    QString mChildTable;
    QString mChildColumn;
    QString mParentTable;
    QString mParentColumn;
    bool mExisting;
    bool mConstrained;

signals:


    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // SCHEMA2RELATIONMODEL_H