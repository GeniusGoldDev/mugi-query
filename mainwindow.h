#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>

class SessionModel;
class SessionTab;
class History;
class QueryHistoryWidget;
class QCompleter;
class Highlighter;
class JoinHelperWidgets;

#include "tokens.h"

#include <QSqlDatabase>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    SessionModel* model() const;
    int tabIndex(QTabWidget *widget, const QString &name);
    void selectTab(QTabWidget *widget, const QString &name);
    SessionTab* tab(int index);
    SessionTab* currentTab();

    void updateTokens(const QString &connectionName);
    void pushTokens(const QString &connectionName);

    void addDatabase(bool showHistory = false);

    QString connectionName() const;


    void selectTab(const QString &name);
    QMenu* selectionMenu() const;

protected:
    void closeEvent(QCloseEvent *event);
    History* mHistory;
    QueryHistoryWidget* mQueryHistory;
    QMap<QString,Tokens> mTokens;
    JoinHelperWidgets* mJoinHelpers;

    QString mQuery;

    void copySelected(bool asList);

    int lastTabIndex(const QString &connectionName);
    void selectDatabase(const QString &connectionName);
public slots:
    void on_addDatabase_triggered();
    void on_sessionTree_customContextMenuRequested(QPoint pos);
    void onTreeCurrentChanged(QModelIndex, QModelIndex);
    void onSessionAdded(QString connectionName, QString name, QString namePrev);
    void onSessionRemoved(QString connectionName, QString name);
    void onAdjustSplitter();
    void onTabsCurrentChanged(int);
    void onQuery(QString query);
    void onShowQueryHistory();
    //void onAddSessionWithQuery(QString);
    void onAppendQuery(const QString &connectionName, QString);

    void on_databaseHistory_triggered();
    void on_queryHistory_triggered();
    void on_addSession_triggered();
    void on_removeSession_triggered();
    void on_removeDatabase_triggered();
    void on_reconnect_triggered();
    void on_queryHelp_triggered();
    void on_dataSave_triggered();
    void on_queryJoin_triggered();
    void on_selectionCopy_triggered();
    void on_selectionCopyAsList_triggered();
    void on_queryQuote_triggered();
    void on_queryUnquote_triggered();
    void on_settingsFormat_triggered();
    void on_dataPlot_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
