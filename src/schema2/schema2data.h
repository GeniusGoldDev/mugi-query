#ifndef SCHEMA2DATA_H
#define SCHEMA2DATA_H

#include <QObject>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QModelIndex>

class Schema2TableModel;
class Schema2TableView;
class Schema2Data;
class Schema2View;
class QGraphicsItem;
class Schema2RelationModel;
class Schema2TableItem;
class ClickListener;
class Schema2AlterView;
class CheckableStringListModel;
class QSortFilterProxyModel;
class Schema2RelationItem2;
class Schema2Relation;
class Schema2TablesModel;
class QSqlDriver;

#include "schema2join.h"
#include "hash.h"
#include "schema2status.h"

class Schema2Data : public QObject
{
    Q_OBJECT
public:
    static QHash<int, QString> mOdbcTypes;

    static bool mDontAskOnDrop;

    static Schema2Data* instance(const QString& connectionName, QObject *parent = nullptr);

    static QHash<QString, Schema2Data*> mData;

    void pull();

    void push(QWidget* widget);

    void save();

    void load();

    bool hasPendingChanges() const;

    Schema2View* view();

    QGraphicsScene* scene();

    void showAlterView(const QString& tableName);

    void showInsertView(const QString &tableName);

    void loadPos();

    void savePos();

    void arrange();

    QList<Schema2Join> findJoin(const QStringList& join);

    QSortFilterProxyModel* selectProxyModel();

    void selectOrDeselect(const QString& table);

    void showRelationsListDialog(QWidget *widget);

    Schema2TableModel* createTable(const QString& name);

    Schema2TableItem* tableItem(const QString& name) const;

    QStringList dataTypes() const;

    //void indexPulled(const QString indexName, const QString &tableName, const QStringList &columns, bool primary, bool unique, Status status);

    void createRelationDialog(Schema2TableModel* childTable, QStringList childColumns, QString parentTable);
    void createRelationDialog(const QString &childTable, const QString &parentTable, QWidget* widget);
    void editRelationDialog(Schema2TableModel *childTable, Schema2Relation *relation, QWidget *widget);
    void dropRelationDialog(const QString &childTable, const QString &parentTable, QWidget *widget);
    void dropTableDialog(const QString &table, QWidget *widget);

    QString connectionName() const {
        return mConnectionName;
    }

    QString driverName() const;

    Schema2AlterView* alterView(const QString& table);

    Schema2TablesModel* tables() const;

    void scriptDialog(QWidget* widget);

    QSqlDriver *driver() const;
protected:
    Schema2Data(const QString& connectionName, QObject *parent = nullptr);

    QString mConnectionName;

    QGraphicsScene* mScene;

    ClickListener* mClickLister;

    Schema2View* mView;

    Schema2TablesModel* mTables;

    //QHash<QString, Schema2TableView*> mViews;

    StringHash<Schema2AlterView*> mAlterViews;

    //StringListHash<Schema2RelationModel*> mRelationModels;

    //QList<QPair<QString, Schema2Relation*>> mDropRelationsQueue;

    QList<Schema2TableModel*> mDropTableQueue;

    //CheckableStringListModel* mSelectModel;

    QSortFilterProxyModel* mSelectProxyModel;


    void pullTables();
    void pullIndexes();
    void pullRelations();

    void pullTablesOdbc();
    void pullTablesMysql();
    void pullTablesOther();

    void pullIndexesOdbc();
    void pullIndexesMysql();

    void pullRelationsMysql();
    void pullRelationsOdbc();

    //void unoverlapTables();
    void setTableItemsPos();


    //void relationPulled(const QString &constraintName, const QString &childTable, const QStringList &childColumns, const QString &parentTable, const QStringList &parentColumns, bool constrained, Status status);

    void tablePulled(const QString &tableName, Status status);
    //QStringList guessParentColumns(QString childTable, QStringList childColumns, QString parentTable);
signals:
    void tableClicked(QString);
protected slots:
    void onSelectModelChanged(QModelIndex, QModelIndex);

};

#endif // SCHEMA2DATA_H
