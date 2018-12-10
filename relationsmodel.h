#ifndef RELATIONSMODEL_H
#define RELATIONSMODEL_H

#include <QStandardItemModel>

class RelationsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit RelationsModel(QObject *parent = nullptr);
    ~RelationsModel() override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void load(const QString& path);
    void save(const QString& path);
protected:
    bool mChanged;
};

#endif // RELATIONSMODEL_H
