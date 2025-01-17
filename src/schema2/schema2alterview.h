#ifndef SCHEMA2ALTERVIEW_H
#define SCHEMA2ALTERVIEW_H

#include <QWidget>
#include "hash.h"

namespace Ui {
class Schema2AlterView;
}

class Schema2TableModel;

class Schema2Data;
class Schema2TablesModel;

class Schema2AlterView : public QWidget
{
    Q_OBJECT

public:
    explicit Schema2AlterView(QWidget *parent = nullptr);
    ~Schema2AlterView();

    //void setModel(Schema2TableModel* model);

    void init(Schema2Data* data,
              Schema2TablesModel *tableModels,
              Schema2TableModel *model,
              const QStringList &types);



protected:
    Schema2Data* mData;

    Schema2TableModel* mModel;
    Schema2TablesModel* mTables;
    QStringList mTypes;

    void initColumns();
    void initRelations();
    void initParentRelations();
    void initIndexes();
    QStringList selectedFields() const;
    void createIndex(bool primary, bool unique);
private slots:
    //void on_createRelation_clicked();

    void on_createIndex_clicked();
    void on_createPrimaryKey_clicked();

    void on_createUniqueIndex_clicked();

    void on_listRelatedTables_clicked();

    void on_tableName_textChanged(const QString &arg1);

    void on_script_clicked();

signals:
    void createRelation(QString childTable, QStringList childColumns, QString parentTable);
    void editRelation();

private:
    Ui::Schema2AlterView *ui;
};

#endif // SCHEMA2ALTERVIEW_H
