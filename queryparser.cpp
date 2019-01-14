#include "queryparser.h"

#include "sl.h"
#include <QDebug>

QueryParser::QueryParser()
{

}


bool QueryParser::isAlterSchemaQuery(const QString& query) {
    return query.indexOf(QRegExp("(create|drop|alter)\\s+table",Qt::CaseInsensitive)) > -1;
}


void QueryParser::testSplitCompare(const QStringList e, const QStringList a) {

    QString e_ = "{" + e.join("|") + "}";
    QString a_ = "{" + a.join("|") + "}";

    if (e_ != a_) {
        qDebug() << "not equal" << e_.toStdString().c_str() << a_.toStdString().c_str();
    }
}

void QueryParser::testSplit() {

    qDebug() << "QueryParser::testSplit() started";

    QString q;
    QStringList e;
    QStringList a;

    q = "foo;bar;baz";
    e = sl("foo","bar","baz");
    a = split(q);
    testSplitCompare(e,a);

    q = ";;";
    e = sl("","","");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo;bar--;baz";
    e = sl("foo","bar--;baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo;bar;--baz";
    e = sl("foo","bar","--baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo--\n;bar";
    e = sl("foo--\n","bar");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo--;\nbar";
    e = sl("foo--;\nbar");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo'bar';baz";
    e = sl("foo'bar'","baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo'bar;baz";
    e = QStringList();
    a = split(q);
    testSplitCompare(e,a);

    q = "foo'bar--';baz";
    e = sl("foo'bar--'","baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo'bar'--;baz";
    e = sl("foo'bar'--;baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo/*bar;*/baz";
    e = sl("foo/*bar;*/baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo/*bar*/;baz";
    e = sl("foo/*bar*/","baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo--/*bar\n;*/baz";
    e = sl("foo--/*bar\n","*/baz");
    a = split(q);
    testSplitCompare(e,a);

    q = "foo'/*bar';*/baz";
    e = sl("foo'/*bar'","*/baz");
    a = split(q);
    testSplitCompare(e,a);

    qDebug() << "QueryParser::testSplit() complete";
}

QStringList QueryParser::split(const QString &queries)
{
    QList<int> bounds;
    bounds << -1;

    QChar quote = '\'';
    QChar semicolon = ';';
    QChar minus = '-';
    QChar newline = '\n';
    QChar slash = '/';
    QChar star = '*';
    bool comment = false;
    bool multilineComment = false;
    bool literal = false;
    for(int i=0;i<queries.size();i++) {
        QChar c = queries[i];
        bool hasNext = i + 1 < queries.size();
        bool hasPrev = i > 0;
        if (c == quote) {
            if (!comment && !multilineComment) {
                literal = !literal;
            }
        } else if (c == semicolon) {
            if (!literal && !comment && !multilineComment) {
                bounds << i;
            }
        } else if (c == minus) {
            if (hasNext && queries[i+1] == minus && !literal && !multilineComment) {
                comment = true;
            }
        } else if (c == newline) {
            comment = false;
        } else if (c == slash) {
            if (!comment && !literal) {
                if (!multilineComment) {
                    if (hasNext && queries[i+1] == star) {
                        multilineComment = true;
                    }
                } else {
                    if (hasPrev && queries[i-1] == star) {
                        multilineComment = false;
                    }
                }
            }
        }
    }
    bounds << queries.size();

    if (literal) {
        return QStringList();
    }

    QStringList res;
    for(int i=0;i<bounds.size()-1;i++) {
        int pos = bounds[i] + 1;
        int size = bounds[i+1] - pos;
        QString query = size > 0 ? queries.mid(pos,size) : QString();
        res << query;
    }
    return res;
}

