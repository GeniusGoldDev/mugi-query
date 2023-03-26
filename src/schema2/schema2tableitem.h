#ifndef SCHEMA2TABLEITEM_H
#define SCHEMA2TABLEITEM_H

#include <QGraphicsItem>

class Schema2TableModel;
class Schema2RelationItem2;

class Schema2TableItem : public QGraphicsItem
{
public:
    Schema2TableItem(Schema2TableModel* model, QGraphicsItem *parent = nullptr);

    void addRelation(Schema2RelationItem2 *relation);

    QString tableName() const;

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setCenterPos(const QPointF& point);

    void setGrayed(bool value);

    bool grayed() const {
        return mGrayed;
    }

    Schema2TableModel* model() const {
        return mModel;
    }

#if 0
    bool isIndexColumn(const QString& column) const;
#endif

    void removeRelation(Schema2RelationItem2 *relation);
protected:
    Schema2TableModel* mModel;
    QList<Schema2RelationItem2*> mRelations;
    bool mGrayed;

    // QGraphicsItem interface
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // QGraphicsItem interface
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // SCHEMA2TABLEITEM_H
