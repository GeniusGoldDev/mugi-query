#include "tests.h"

#include <QStringList>
#include "jointokenlist.h"

#include "zipunzip.h"
#include <QDebug>
#include "queryparser.h"
#include "lit.h"
#include "stringstringmap.h"

using namespace Lit;

void Tests::run()
{
    testSplit();
    testJoinSplit();
    testAliases();
    testFlatQueries();
    testClosingBracket();
    testParseCreateTable();
    textParseCreateTableCreateDefinition();
}

namespace {

QStringList mapTrimmed(const QStringList& vs) {
    QStringList res;
    foreach(const QString& v, vs) {
        res << v.trimmed();
    }
    return res;
}

QString mapAsString(const QMap<QString,QString>& m) {
    QStringList keys = m.keys();
    QStringList vs;
    foreach(const QString& key, keys) {
        vs << QString("%1:%2").arg(key).arg(m[key]);
    }
    return QString("{%1}").arg(vs.join("|"));
}

QString tokenListAsString(const QList<QPair<JoinToken::JoinToken,QString> >& vs) {
    QStringList res;
    for(int i=0;i<vs.size();i++) {
        const QPair<JoinToken::JoinToken,QString>& v = vs[i];
        res << QString("%1, %2").arg(v.first).arg(v.second);
    }
    return QString("{%1}").arg(res.join(", "));
}

void compare(int a, int b) {
    if (a == b) {
        return;
    }
    qDebug() << "not equal, expected: " << a << ", actual " << b;
}

void compare(const QMap<QString,QString>& e, const QMap<QString,QString>& a) {
    if (e == a) {
        return;
    }
    QString e_ = mapAsString(e);
    QString a_ = mapAsString(a);
    qDebug() << "not equal, expected: " << e_.toStdString().c_str() << ", actual " << a_.toStdString().c_str();
}

void compare(const QList<QPair<JoinToken::JoinToken,QString> >& e, const QList<QPair<JoinToken::JoinToken,QString> >& a) {

    QStringList es = mapTrimmed(unzipSeconds(e));
    QStringList as = mapTrimmed(unzipSeconds(a));
    QList<JoinToken::JoinToken> ef = unzipFirsts(e);
    QList<JoinToken::JoinToken> af = unzipFirsts(a);
    if (as == es && ef == af) {
        return ;
    }
    QString e_ = tokenListAsString(e);
    QString a_ = tokenListAsString(a);
    qDebug() << "not equal, expected: " << e_.toStdString().c_str() << ", actual " << a_.toStdString().c_str();
}

void compare(const QPair<QString,QStringList>& e, const QPair<QString,QStringList>& a) {
    if (e == a) {
        return;
    }
    QString e_ = e.first + "{" + e.second.join("|") + "}";
    QString a_ = a.first + "{" + a.second.join("|") + "}";
    qDebug() << "not equal, expected: " << e_.toStdString().c_str() << ", actual " << a_.toStdString().c_str();
}

void compare(const QStringList e, const QStringList a) {
    QString e_ = "{" + e.join("|") + "}";
    QString a_ = "{" + a.join("|") + "}";
    if (e_ == a_) {
        return;
    }
    qDebug() << "not equal, expected: " << e_.toStdString().c_str() << ", actual " << a_.toStdString().c_str();
}

} // namespace

void Tests::testAliases() {

    qDebug() << "testAliases() started";

    using namespace StringStringMap;

    QString q;
    QMap<QString,QString> e,a;

    q = "select * from foo, bar left join baz";
    e = ssm();
    a = QueryParser::aliases(q);
    compare(e,a);

    q = "select * from foo f1, bar b1 left join baz b2 on baz.id=bar.id join qix q1";
    e = ssm("f1","foo","b1","bar","b2","baz","q1","qix");
    a = QueryParser::aliases(q);
    compare(e,a);

    q = "select * from (select abc from zen z1 having 2) x, bar b1 left join baz b2 on baz.id=bar.id join qix q1 where a=b";
    e = ssm("b1","bar","b2","baz","q1","qix","z1","zen");
    a = QueryParser::aliases(q);
    compare(e,a);

    qDebug() << "testAliases() finished";
}

void Tests::testJoinSplit() {

    qDebug() << "testJoinSplit started";

    using namespace JoinTokenList;
    using namespace JoinToken;

    QString q;
    QList<QPair<JoinToken::JoinToken,QString> > e, a;

    q = "foo, bar left join baz on baz.id=bar.id join qix";
    e = jtl(JoinTokenTable,"foo",
            JoinTokenComma,",",
            JoinTokenTable,"bar",
            JoinTokenJoin,"left join",
            JoinTokenTable,"baz",
            JoinTokenOn,"on",
            JoinTokenCondition,"baz.id=bar.id",
            JoinTokenJoin,"join",
            JoinTokenTable,"qix");

    a = QueryParser::joinSplit(q);
    compare(e,a);

    q = "foo f1, bar b1 left join baz b2 on baz.id=bar.id join qix q1";
    e = jtl(JoinTokenTable,"foo f1",
            JoinTokenComma,",",
            JoinTokenTable,"bar b1",
            JoinTokenJoin,"left join",
            JoinTokenTable,"baz b2",
            JoinTokenOn,"on",
            JoinTokenCondition,"baz.id=bar.id",
            JoinTokenJoin,"join",
            JoinTokenTable,"qix q1");

    a = QueryParser::joinSplit(q);
    compare(e,a);

    qDebug() << "testJoinSplit() finished";
}


void Tests::testSplit() {

    qDebug() << "testSplit() started";

    QString q;
    QStringList e;
    QStringList a;

    q = "foo;bar;baz";
    e = sl("foo","bar","baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = ";;";
    e = sl("","","");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo;bar--;baz";
    e = sl("foo","bar--;baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo;bar;--baz";
    e = sl("foo","bar","--baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo--\n;bar";
    e = sl("foo--\n","bar");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo--;\nbar";
    e = sl("foo--;\nbar");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo'bar';baz";
    e = sl("foo'bar'","baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo'bar;baz";
    e = QStringList();
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo'bar--';baz";
    e = sl("foo'bar--'","baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo'bar'--;baz";
    e = sl("foo'bar'--;baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo/*bar;*/baz";
    e = sl("foo/*bar;*/baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo/*bar*/;baz";
    e = sl("foo/*bar*/","baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo--/*bar\n;*/baz";
    e = sl("foo--/*bar\n","*/baz");
    a = QueryParser::split(q);
    compare(e,a);

    q = "foo'/*bar';*/baz";
    e = sl("foo'/*bar'","*/baz");
    a = QueryParser::split(q);
    compare(e,a);

    qDebug() << "testSplit() finished";
}

void Tests::testFlatQueries() {

    qDebug() << "testFlatQueries() started";

    QString q;
    QStringList e,a;

    q = "select foo from (select * from bar) union (select baz from qix)";
    e = sl("select foo from () union ()","select * from bar","select baz from qix");
    a = QueryParser::flatQueries(q);
    compare(e,a);

    q = "select a,b,c from (select foo from (select * from bar) union (select baz from qix)) where 1";
    e = sl("select a,b,c from () where 1","select foo from () union ()","select * from bar","select baz from qix");
    a = QueryParser::flatQueries(q);
    compare(e,a);

    q = "select * from foo";
    e = sl("select * from foo");
    a = QueryParser::flatQueries(q);
    compare(e,a);

    qDebug() << "testFlatQueries() finished";
}

void Tests::testClosingBracket() {

    qDebug() << "testClosingBracket() started";

    QString t;
    int a,e;

    t = "()";
    a = QueryParser::closingBracket(t,0);
    e = 1;
    compare(e,a);

    t = "(())";
    a = QueryParser::closingBracket(t,0);
    e = 3;
    compare(e,a);

    t = "(())";
    a = QueryParser::closingBracket(t,1);
    e = 2;
    compare(e,a);

    t = "create table foo(bar int(11),baz text)";
    a = QueryParser::closingBracket(t,t.indexOf("("));
    e = t.lastIndexOf(")");
    compare(e,a);
    qDebug() << "testClosingBracket() finished";
}

void Tests::testParseCreateTable()
{
    qDebug() << "testParseCreateTable() started";

    typedef QPair<QString,QStringList> X;

    QString t;
    X a,e;

    t = "create table foo(a,b)";
    a = QueryParser::parseCreateTable(t);
    e = X("foo",sl("a","b"));
    compare(e,a);

    t = "create temporary table if not exists foo(a,b)";
    a = QueryParser::parseCreateTable(t);
    e = X("foo",sl("a","b"));
    compare(e,a);

    t = "create table foo like bar(a,b)";
    a = QueryParser::parseCreateTable(t);
    e = X("foo",sl("a","b"));
    compare(e,a);

    qDebug() << "testParseCreateTable() finished";
}

void Tests::textParseCreateTableCreateDefinition() {

    qDebug() << "textParseCreateTableCreateDefinition() started";

    QString t,err;
    QStringList e,a;

    t = "`foo` int(11) NOT NULL";
    e = sl("foo","int(11)","NOT NULL","","");
    a = QueryParser::parseCreateTableCreateDefinition(t,err);
    compare(e,a);

    t = "`bar` text";
    e = sl("bar","text","","","");
    a = QueryParser::parseCreateTableCreateDefinition(t,err);
    compare(e,a);

    t = "foo int unsigned zerofill null";
    e = sl("foo","int unsigned zerofill","null","","");
    a = QueryParser::parseCreateTableCreateDefinition(t,err);
    compare(e,a);

    t = "`foo` varchar(100) DEFAULT NULL";
    e = sl("foo","varchar(100)","","DEFAULT NULL","");
    a = QueryParser::parseCreateTableCreateDefinition(t,err);
    compare(e,a);

    t = "`bar` int(11) NOT NULL DEFAULT '0'";
    e = sl("bar","int(11)","NOT NULL","DEFAULT '0'","");
    a = QueryParser::parseCreateTableCreateDefinition(t,err);
    compare(e,a);

    qDebug() << "textParseCreateTableCreateDefinition() finished";
}
