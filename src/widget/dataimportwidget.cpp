#include "dataimportwidget.h"
#include "ui_dataimportwidget.h"

#include <QStandardItemModel>

#include "richheaderview/richheaderview.h"
#include "model/dataimportmodel.h"
#include "model/checkablestringlistmodel.h"
#include "model/dataimportcolumnmodel.h"
#include "widget/selectcolumnslistwidget.h"
#include "copyeventfilter.h"
#include "clipboard.h"
#include <QTimer>
#include "splitterutil.h"
#include "datastreamer.h"
#include "sqldatatypes.h"
#include "error.h"
#include <QComboBox>
#include <QCheckBox>
#include <QtGlobal>
#include <QSqlRecord>
#include <QSqlField>
#include "callonce.h"
#include "dataformat.h"
#include "datetime.h"
#include "widget/fieldattributeswidget.h"
#include "widget/intlineedit.h"

namespace  {

QModelIndex currentOrFirstIndex(QTableView* view) {
    QModelIndex index = view->currentIndex();
    if (!index.isValid()) {
        index = view->model()->index(0,0);
    }
    return index;
}

void setSpacerEnabled(QSpacerItem* spacer, bool enabled, QLayout* layout) {
    if (enabled) {
        spacer->changeSize(1,1,QSizePolicy::Expanding,QSizePolicy::Expanding);
    } else {
        spacer->changeSize(0,0,QSizePolicy::Fixed,QSizePolicy::Fixed);
    }
    layout->invalidate();
}

void recordToNamesTypes(const QSqlRecord& record, QStringList& names, QList<QVariant::Type>& types) {
    for(int c=0;c<record.count();c++) {
        names << record.fieldName(c);
        types << record.field(c).type();
    }
    //qDebug() << "recordToNamesTypes";
}

} // namespace

DataImportWidget::DataImportWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataImportWidget),
    mUpdatePreview(new CallOnce("onUpdatePreview",0,this)),
    mSetModelTypes(new CallOnce("onSetTypes",0,this)),
    mSetColumnNamesAndTypes(new CallOnce("onSetColumnNamesAndTypes",0,this))
{
    ui->setupUi(this);
}

DataImportWidget::~DataImportWidget()
{
    delete ui;
}

void DataImportWidget::init(const QString &connectionName)
{
    QTimer::singleShot(0,[=](){
        SplitterUtil::setRatio(ui->horizontalSplitter,{1,2});
        SplitterUtil::setRatio(ui->verticalSplitter,{4,1});
    });

    ui->columns->setLabelsMode(SelectColumnsWidget::LabelsModeShort);

    connect(mUpdatePreview,SIGNAL(call()),this,SLOT(onUpdatePreview()));
    connect(mSetModelTypes,SIGNAL(call()),this,SLOT(onModelSetTypes()));

    mConnectionName = connectionName;

    //connect(ui->existingTable,SIGNAL(currentIndexChanged(int)),this,SLOT(onExistingTableCurrentIndexChanged(int)));

    RichHeaderView* view = new RichHeaderView(Qt::Horizontal,ui->data);

    int fontHeight = view->fontMetrics().height();
    int twoLines = fontHeight * 2;

    RichHeaderData* data = view->data();
    data->subsectionSizes({twoLines, twoLines, twoLines, twoLines * 3 / 2});

    DataImportModel* model = new DataImportModel(data->sizeHint(),ui->data);
    model->setLocale(locale());

    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            mUpdatePreview,SLOT(onPost()));

    ui->data->setModel(model);
    ui->data->setHorizontalHeader(view);

    CopyEventFilter* filter = new CopyEventFilter();
    filter->setView(ui->data);

    connect(filter,SIGNAL(paste()),this,SLOT(onDataPaste()));
    connect(filter,SIGNAL(delete_()),filter,SLOT(onDeleteSelected()));
    connect(filter,SIGNAL(copy()),this,SLOT(onDataCopy()));

    on_abc_clicked();

    DataFormat::initComboBox(ui->format, true);

    connect(mSetColumnNamesAndTypes,SIGNAL(call()),this,SLOT(onSetColumnNamesAndTypes()));

    connect(ui->columns,SIGNAL(dataChanged(int,QModelIndex,QModelIndex)),
            mSetColumnNamesAndTypes,SLOT(onPost()));

    onUpdateTitle();

    connect(ui->minYear,&IntLineEdit::valueChanged,[=](int value){
         ui->maxYear->setValue(value + 99);
         mUpdatePreview->onPost();
    });

    connect(ui->maxYear,&IntLineEdit::valueChanged,[=](int value){
        ui->minYear->setValue(value - 99);
        mUpdatePreview->onPost();
    });

    int currentYear = QDate::currentDate().year();
    // assume two digit year means years in range [currentYear - 50, currentYear + 50)
    // where currentYear is current year rounded to tens
    int minYear = ((currentYear + 5) / 10) * 10 - 50;
    int maxYear = minYear + 99;
    ui->minYear->setValue(minYear);

    qDebug() << (currentYear - minYear) << (maxYear - currentYear);

    connect(ui->inLocal,SIGNAL(clicked(bool)),mUpdatePreview,SLOT(onPost()));
    connect(ui->inUtc,SIGNAL(clicked(bool)),mUpdatePreview,SLOT(onPost()));
    connect(ui->outAsIs,SIGNAL(clicked(bool)),mUpdatePreview,SLOT(onPost()));
    connect(ui->outUtc,SIGNAL(clicked(bool)),mUpdatePreview,SLOT(onPost()));

}


QStringList comboBoxItems(QComboBox* comboBox) {
    QStringList items;
    for(int i=0;i<comboBox->count();i++) {
        items << comboBox->itemText(i);
    }
    return items;
}

void DataImportWidget::update(const Tokens& tokens) {
    mTokens = tokens;

    QStringList currentTables = comboBoxItems(ui->existingTable);
    QStringList newTables = mTokens.tables();

    bool firstRun = ui->existingTable->count() == 0;

    if (newTables != currentTables) {
        QString selected = ui->existingTable->currentText();
        ui->existingTable->clear();
        ui->existingTable->addItems(newTables);
        int index = selected.isEmpty() ? 0 : qMax(0,newTables.indexOf(selected));
        if (newTables.size() > 0) {
            ui->existingTable->setCurrentIndex(index);
        }
    }

    if (firstRun) {
        if (ui->optionNewTable->isChecked()) {
            on_optionNewTable_clicked();
        } else {
            on_optionExistingTable_clicked();
        }
    }

    ui->preview->setTokens(mTokens);
    mSetModelTypes->onPost();
}

QString DataImportWidget::connectionName() const
{
    return mConnectionName;
}

void DataImportWidget::on_existingTable_currentIndexChanged(int index) {

    if (!ui->optionExistingTable->isChecked()) {
        return;
    }

    QString table = ui->existingTable->itemText(index);

    QSqlDatabase db = QSqlDatabase::database(mConnectionName);
    QSqlRecord record = db.record(table);

    QStringList names;
    QList<QVariant::Type> types;
    recordToNamesTypes(record,names,types);

    ui->columns->setTable(table);
    ui->columns->setFields(names, types);

    ui->columns->data()->model()->setAllChecked();

    //onColumnDataChanged(0,QModelIndex(),QModelIndex());
    mSetColumnNamesAndTypes->onPost();
    onUpdateTitle();
}

/*
void DataImportWidget::onDataChanged(QModelIndex,QModelIndex,QVector<int>) {
    // qDebug() << "onDataChanged updatePreview";
    mUpdatePreview->onPost();
}*/

RichHeaderView* DataImportWidget::headerView() const {
    return qobject_cast<RichHeaderView*>(ui->data->horizontalHeader());
}

DataImportModel* DataImportWidget::dataModel() const {
    return qobject_cast<DataImportModel*>(ui->data->model());
}

void DataImportWidget::setColumnNames(const QStringList& names) {

    RichHeaderView* view = headerView();
    if (!view) {
        return;
    }

    QAbstractItemModel* model = dataModel();
    if (model->columnCount() < names.size()) {
        model->insertColumns(model->columnCount(), names.size() - model->columnCount());
    }

    createHeaderViewWidgets();

    for(int c=0;c<model->columnCount();c++) {
        QLineEdit* name = widgetName(c);
        if (!name) {
            qDebug() << "!name" << __FILE__ << __LINE__;
            continue;
        }
        name->setText(names.value(c));
    }

    // qDebug() << "setColumnNames updatePreview";
}

void DataImportWidget::setColumnAttributes() {
    QAbstractItemModel* model = dataModel();
    for(int c=0;c<model->columnCount();c++) {
        FieldAttributesWidget* attributes = widgetFieldAttributes(c);
        attributes->setPrimaryKey(false);
        attributes->setAutoincrement(false);
    }
}

void DataImportWidget::setColumnSizes() {
    QAbstractItemModel* model = dataModel();
    for(int c=0;c<model->columnCount();c++) {
        IntLineEdit* size = widgetSize(c);
        size->setText(QString());
    }
}

void DataImportWidget::setColumnTypes(const QList<QVariant::Type>& types) {

    RichHeaderView* view = headerView();
    if (!view) {
        return;
    }

    QAbstractItemModel* model = dataModel();

    QStringList names = SqlDataTypes::names();
    QMap<QVariant::Type, QString> m = SqlDataTypes::mapFromVariant();

    for(int c=0;c<model->columnCount();c++) {
        QComboBox* type = widgetType(c);
        if (!type) {
            qDebug() << "!type" << __FILE__ << __LINE__;
            continue;
        }
        if (types.size() > c) {
            type->setCurrentIndex(names.indexOf(m[types[c]]));
        } else {
            type->setCurrentIndex(0);
        }
    }

}

QWidget* DataImportWidget::widget(int row, int column) const {
    RichHeaderView* view = headerView();
    if (!view) {
        return 0;
    }
    RichHeaderData* data = view->data();
    return data->cell(row,column).widget();
}

QLineEdit* DataImportWidget::widgetName(int column) const {
    return qobject_cast<QLineEdit*>(widget(WidgetName,column));
}

QComboBox* DataImportWidget::widgetType(int column) const {
    return qobject_cast<QComboBox*>(widget(WidgetType,column));
}

IntLineEdit* DataImportWidget::widgetSize(int column) const {
    return qobject_cast<IntLineEdit*>(widget(WidgetSize,column));
}

FieldAttributesWidget* DataImportWidget::widgetFieldAttributes(int column) const {
    return qobject_cast<FieldAttributesWidget*>(widget(WidgetFieldAttributes,column));
}

QList<Field> DataImportWidget::fields() const {
    QList<Field> result;
    RichHeaderView* view = headerView();
    QAbstractItemModel* model = dataModel();
    if (!view || !model) {
        qDebug() << __FILE__ << __LINE__ << view << model;
        return result;
    }
    for(int c=0;c<model->columnCount();c++) {
        QLineEdit* name = widgetName(c);
        QComboBox* type = widgetType(c);
        FieldAttributesWidget* attributes = widgetFieldAttributes(c);
        IntLineEdit* size = widgetSize(c);

        if (!name || !type || !attributes || !size) {
            qDebug() << __FILE__ << __LINE__ << name << type << attributes << size;
            continue;
        }

        Field field(name->text(), type->currentText(), size->value(-1),
                    attributes->primaryKey(), attributes->autoincrement(),
                    attributes->index(), attributes->unique());
        result << field;
    }
    return result;
}

void DataImportWidget::createHeaderViewWidgets() {

    // qDebug() << "createHeaderViewWidgets" << mUpdatePreview;

    RichHeaderView* view = headerView();
    QAbstractItemModel* model = dataModel();
    if (!view || !model) {
        return;
    }

    bool newTable = ui->optionNewTable->isChecked();

    RichHeaderData* data = view->data();
    QWidget* viewport = view->viewport();
    for(int c=0;c<model->columnCount();c++) {

        QLineEdit* name = widgetName(c);
        if (!name) {
            name = new QLineEdit(viewport);
            name->setPlaceholderText("name");

            data->cell(WidgetName,c).widget(name);

            connect(name, &QLineEdit::textChanged,[=](){
                onColumnNameChanged(c);
            });

        }

        QComboBox* types = widgetType(c);
        if (!types) {
            types = new QComboBox(viewport);

            types->addItems(SqlDataTypes::names());
            data->cell(WidgetType,c).widget(types);

            connect(types,qOverload<int>(&QComboBox::currentIndexChanged),[=](){
                onColumnTypeChanged(c);
            });
        }

        IntLineEdit* size = widgetSize(c);
        if (!size) {
            size = new IntLineEdit(viewport);
            size->setPlaceholderText("size");
            connect(size, &IntLineEdit::textChanged,[=](){
               onColumnSizeChanged(c);
            });
            data->cell(WidgetSize,c).widget(size);
        }

        FieldAttributesWidget* attributes = widgetFieldAttributes(c);
        if (!attributes) {
            attributes = new FieldAttributesWidget(viewport);
            data->cell(WidgetFieldAttributes,c).widget(attributes).padding(0,5);
            connect(attributes,&FieldAttributesWidget::attributeClicked,[=](){
                onFieldAttributeClicked(c);
            });
        }

        widgetName(c)->setReadOnly(!newTable);
        widgetType(c)->setEnabled(newTable);
        widgetFieldAttributes(c)->setEnabled(newTable);
    }
    view->update();
}



/*
void DataImportWidget::onColumnDataChanged(int,QModelIndex,QModelIndex) {
    qDebug() << "onColumnDataChanged";
}*/

void DataImportWidget::onSetColumnNamesAndTypes() {
    qDebug() << "onSetColumnNamesAndTypes";
    QStringList names;
    QList<QVariant::Type> types;
    ui->columns->checked(names,types);
    setColumnNames(names);
    setColumnTypes(types);
    setColumnSizes();
    setColumnAttributes();
}

void DataImportWidget::onUpdateTokens(QString connectionName, Tokens tokens)
{
    if (connectionName != mConnectionName) {
        return;
    }
    update(tokens);
}

void DataImportWidget::onModelSetTypes() {

    DataImportModel* model = dataModel();
    if (!model) {
        return;
    }

    QMap<int,QVariant::Type> columnType;
    QMap<int,int> columnSize;

    /*QStringList names;
    QStringList types;
    QList<bool> primaryKeys;
    QList<bool> autoincrements;
    namesTypesAndAttributes(names,types,primaryKeys,autoincrements);*/

    QList<Field> fields = this->fields();

    QMap<QString,QVariant::Type> m = SqlDataTypes::mapToVariant();

    for(int c=0;c<fields.size();c++) {
        const Field& field = fields[c];
        if (field.name().isEmpty()) {
            continue;
        }
        columnType[c] = m[field.type()];
        columnSize[c] = field.size();
    }

    model->setTypes(columnType, columnSize);
    mUpdatePreview->onPost();
}

void DataImportWidget::onColumnTypeChanged(int) {
    mSetModelTypes->onPost();
}

void DataImportWidget::onColumnSizeChanged(int) {
    mSetModelTypes->onPost();
}

void DataImportWidget::onColumnNameChanged(int) {
    mSetModelTypes->onPost();
}

void DataImportWidget::onFieldAttributeClicked(int)
{
    mUpdatePreview->onPost();
}

void DataImportWidget::newOrExistingTable() {
    bool newTable = ui->optionNewTable->isChecked();
    ui->existingTable->setEnabled(!newTable);
    ui->newTable->setEnabled(newTable);
    ui->columns->setVisible(!newTable);
    ui->format->setEnabled(!newTable);
    setSpacerEnabled(ui->verticalSpacer,newTable,ui->groupBox->layout());
    createHeaderViewWidgets();
    onUpdateTitle();

    mUpdatePreview->onPost();
}

void DataImportWidget::on_optionNewTable_clicked()
{
    newOrExistingTable();
}

void DataImportWidget::on_optionExistingTable_clicked()
{
    newOrExistingTable();
    //onColumnDataChanged(0,QModelIndex(),QModelIndex());
    mSetColumnNamesAndTypes->onPost();
    if (!ui->columns->data()->model()) {
        on_existingTable_currentIndexChanged(ui->existingTable->currentIndex());
    }
}

QVariant::Type DataImportWidget::guessType(QAbstractItemModel* model,
                                    const QModelIndex& topLeft,
                                    const QModelIndex& bottomRight) {

    int ints = 0;
    int dates = 0;
    int dateTimes = 0;
    int doubles = 0;
    int strings = 0;
    int times = 0;
    int total = 0;

    QRegularExpression rxInt("^[+-]?[0-9]+$");
    QRegularExpression rxDouble("^([+-]?(?:[0-9]+[.,]?|[0-9]*[.,][0-9]+))(?:[Ee][+-]?[0-9]+)?$");

    QItemSelection selection(topLeft, bottomRight);
    QModelIndexList indexes = selection.indexes();
    foreach(const QModelIndex& index, indexes) {
        QString text = model->data(index).toString();

        if (text.isEmpty()) {
            continue;
        }
        if (rxInt.match(text).hasMatch()) {
            ints++;
        } else if (rxDouble.match(text).hasMatch()) {
            doubles++;
        } else {

            QDate date;
            QTime time;
            QDateTime dateTime;

            if (DateTime::parse(DateTime::TypeUnknown,text,date,time,dateTime,1950,true,true)) {
                if (dateTime.isValid()) {
                    dateTimes++;
                } else if (date.isValid()) {
                    dates++;
                } else if (time.isValid()) {
                    times++;
                } else {
                    qDebug() << __FILE__ << __LINE__ << text;
                }
            } else {
                strings++;
            }
        }

        total++;
    }

    if (total < 1) {
        return QVariant::Invalid;
    }

    if (dates * 100 / total > 90) {
        return QVariant::Date;
    } else if (ints * 100 / total > 90) {
        return QVariant::Int;
    } else if ((doubles + ints) * 100 / total > 90) {
        return QVariant::Double;
    } else if (times * 100 / total > 90) {
        return QVariant::Time;
    } else if (strings * 100 / total > 90) {
        return QVariant::String;
    } else if (dateTimes * 100 / total > 90) {
        return QVariant::DateTime;
    }
    return QVariant::Invalid;
}

void DataImportWidget::guessColumnType(int column) {
    QAbstractItemModel* model = dataModel();
    QMap<QVariant::Type, QString> m = SqlDataTypes::mapFromVariant();
    QStringList ts = SqlDataTypes::names();
    QVariant::Type type = guessType(model,model->index(0,column),model->index(model->rowCount()-1,column));
    if (type == QVariant::Invalid) {
        return;
    }
    QComboBox* types = widgetType(column);
    types->setCurrentIndex(ts.indexOf(m[type]));
}

void DataImportWidget::onDataPaste() {

    QAbstractItemModel* model = dataModel();
    QModelIndex topLeft = currentOrFirstIndex(ui->data);
    QModelIndex bottomRight = Clipboard::pasteTsv(model,topLeft,true,true);

    if (!ui->optionNewTable->isChecked()) {
        return;
    }

    for(int column = topLeft.column(); column <= bottomRight.column(); column++) {
        guessColumnType(column);
    }

    //onColumnTypeChanged(0);
    //mUpdatePreview->onPost();
}

void DataImportWidget::onDataCopy() {
    QString error;
    Clipboard::copySelected(dataModel(),ui->data->selectionModel()->selection(), DataFormat::Csv, "\t", locale(), error);
    Error::show(this,error);
}

QString DataImportWidget::queries(bool preview) {
    DataImportModel* model = dataModel();
    if (!model) {
        return QString();
    }

    int minYear = ui->minYear->value(-1);
    if (minYear == -1) {
        return QString();
    }

    /*QStringList names;
    QStringList types;
    QList<bool> primaryKeys;
    QList<bool> autoincrements;
    namesTypesAndAttributes(names,types,primaryKeys,autoincrements);*/

    bool newTable = ui->optionNewTable->isChecked();

    QString table = tableName();

    QSqlDatabase db = QSqlDatabase::database(mConnectionName);

    QStringList schemaQueries;

    QList<Field> fields = this->fields();

    if (newTable) {
        schemaQueries.append(DataStreamer::createTableStatement(db,table,fields,false));
        schemaQueries.append(DataStreamer::createIndexStatements(db,table,fields));
    }

    QString error;
    bool hasMore;

    int rowCount = preview ? 5 : model->rowCount();

    DataFormat::Format format =
            newTable ?
                DataFormat::SqlInsert :
                DataFormat::value(ui->format);

    int dataColumns =
            format == DataFormat::SqlInsert ?
                fields.size() :
                ui->columns->data()->model()->countChecked();

    bool inLocal = ui->inLocal->isChecked();
    bool outUtc = ui->outUtc->isChecked();

    QString insertOrUpdateQuery = DataStreamer::stream(format, db, model, rowCount, table, fields, dataColumns, minYear, inLocal, outUtc, locale(), &hasMore, error);
    QString elipsis = (hasMore && preview) ? "..." : QString();

    return schemaQueries.join(";\n") + (schemaQueries.isEmpty() ? "" : ";\n") + insertOrUpdateQuery + elipsis;
}

void DataImportWidget::onUpdatePreview() {
    qDebug() << "onUpdatePreview";
    ui->preview->setText(queries(true));
}

QString DataImportWidget::tableName() {
    bool newTable = ui->optionNewTable->isChecked();
    QString table = newTable ? ui->newTable->text() : ui->existingTable->currentText();
    return table;
}

void DataImportWidget::onUpdateTitle()
{
    setWindowTitle(QString("%1 %2").arg(mConnectionName).arg(tableName()));
}

void DataImportWidget::on_newTable_textChanged(QString)
{
    if (!ui->optionNewTable->isChecked()) {
        return;
    }
    mUpdatePreview->onPost();
    onUpdateTitle();
}

void DataImportWidget::on_clearData_clicked()
{
    DataImportModel* model = dataModel();
    model->clear();
    model->setRowCount(5);
    if (ui->optionExistingTable->isChecked()) {
        //onColumnDataChanged(0,QModelIndex(),QModelIndex());
        mSetColumnNamesAndTypes->onPost();
    } else {
        model->setColumnCount(5);
        createHeaderViewWidgets();
    }
    mUpdatePreview->onPost();
}

void DataImportWidget::on_copyQuery_clicked()
{
    emit appendQuery(queries(false));
}

void DataImportWidget::on_abc_clicked()
{
    if (ui->optionExistingTable->isChecked()) {
        return;
    }
    QStringList alp = QString("abcdefghijklmnopqrstuvwxyz").split("", QString::SkipEmptyParts);
    setColumnNames(alp.mid(0,dataModel()->columnCount()));
}

void DataImportWidget::on_format_currentIndexChanged(int)
{
    ui->columns->setFormat(DataFormat::value(ui->format));
    mSetColumnNamesAndTypes->onPost();
}

void DataImportWidget::on_guessTypes_clicked()
{
    int columnCount = dataModel()->columnCount();
    for(int column=0;column < columnCount; column++) {
        guessColumnType(column);
    }
}
