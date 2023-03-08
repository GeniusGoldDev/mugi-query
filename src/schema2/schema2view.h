#ifndef SCHEMA2VIEW_H
#define SCHEMA2VIEW_H

#include <QWidget>

class QGraphicsScene;
class QPushButton;
class Schema2Data;

namespace Ui {
class Schema2View;
}

class Schema2View : public QWidget
{
    Q_OBJECT

public:
    enum Mode {
        ModeNone,
        ModeMove,
        ModeRelate,
        ModeAlter,
        ModeInsert,
        ModeSelect
    };

    explicit Schema2View(QWidget *parent = nullptr);
    ~Schema2View();

    void setData(Schema2Data* data);

    static QList<double> mScales;
    static QStringList mScalesText;

public slots:
    void onTableClicked(QString tableName);

protected:
    void uncheckAllExcept(QPushButton *checked);

    Mode mMode;

    Schema2Data* mData;

    QStringList mRelationTables;

    int mScaleIndex;

private slots:

    void on_move_clicked(bool checked);

    void on_relate_clicked(bool checked);

    void on_alter_clicked(bool checked);

    void on_insert_clicked(bool checked);

    void on_create_clicked();

    void on_arrange_clicked();

    void on_pull_clicked();

    void on_push_clicked();

    void on_zoomOut_clicked();

    void on_zoomIn_clicked();

    void on_scale_currentIndexChanged(int index);

    void on_select_clicked(bool checked);

private:
    Ui::Schema2View *ui;
};

#endif // SCHEMA2VIEW_H
