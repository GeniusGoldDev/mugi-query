// DO NOT EDIT ** This file was generated by mugicpp ** DO NOT EDIT
#include <QDebug>
#include <QList>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QVariant>
#include <cstdint>
namespace mugisql {

    QString inParenthesis(const QString& expr, bool yes = true);
    QString spaced(const QString& expr, bool yes = true);

    class table_t {

    public:
        virtual QString aliasName() const = 0;
        virtual QString tableName() const = 0;
    };

    class alias_t : public table_t {};

    class field_t {

    public:
        virtual QString name() const = 0;
        virtual QString fullName() const = 0;
        virtual QString aliasName() const = 0;
    };

    class fieldlist_t : public QList<const field_t*> {

    public:
        fieldlist_t();
        fieldlist_t(const field_t& field);
        fieldlist_t(const std::initializer_list<const field_t*>& values);
        QString toString(bool full = true) const;
    };

    class aliasedtable_t {

    public:
        aliasedtable_t();
        aliasedtable_t(const table_t& table, const alias_t& alias);
        aliasedtable_t(const table_t& table);
        QString toString() const;
        const table_t* table() const;
        void setTable(const table_t* value);
        const alias_t* alias() const;
        void setAlias(const alias_t* value);

    protected:
        const table_t* mTable;
        const alias_t* mAlias;
    };

    class aliasedtablelist_t : public QList<aliasedtable_t> {};

    class arg_t {

    public:
        arg_t();
        arg_t(const field_t& field);
        arg_t(const QVariant& value);
        virtual ~arg_t();
        virtual QString toString(QSqlDriver* driver) const;
        const field_t* field() const;
        QVariant value() const;

    protected:
        const field_t* mField;
        QVariant mValue;
    };

    class expr_t : public arg_t {

    public:
        enum expr_type {
            expr_null,
            expr_leaf,
            expr_asc,
            expr_desc,
            expr_equal,
            expr_not_equal,
            expr_less,
            expr_more,
            expr_less_or_equal,
            expr_more_or_equal,
            expr_or_,
            expr_and_,
            expr_add,
            expr_sub,
            expr_mul,
            expr_div,
            expr_sqlite_mod,
            expr_not_,
            expr_abs,
            expr_sum,
            expr_avg,
            expr_min,
            expr_max,
            expr_round,
            expr_sqlite_datetime,
            expr_mysql_sign,
            expr_mysql_floor,
            expr_mysql_ceil,
            expr_mysql_sqrt,
            expr_mysql_asin,
            expr_mysql_acos,
            expr_mysql_atan,
            expr_mysql_sin,
            expr_mysql_cos,
            expr_mysql_tan,
            expr_mysql_mod,
            expr_mysql_power
        };
        expr_t();
        expr_t(const field_t& field);
        expr_t(int value);
        expr_t(const QString& value);
        expr_t(const char* value);
        expr_t(const QVariant& value);
        expr_t(expr_type type, const QList<expr_t>& args);
        bool isNull() const;
        expr_type type() const;
        QList<expr_t> args() const;
        QString infix(QSqlDriver* driver, const QString& name) const;
        QString func(QSqlDriver* driver, const QString& name) const;
        QString prefix(QSqlDriver* driver, const QString& name) const;
        QString suffix(QSqlDriver* driver, const QString& name) const;
        QString toString(QSqlDriver* driver) const;

    protected:
        QString joinArgs(QSqlDriver* driver, bool parenthesis, const QString& glue) const;
        expr_type mType;
        QList<expr_t> mArgs;
    };
    bool operator==(const expr_t& expr1, const expr_t& expr2);

    class exprlist_t : public QList<expr_t> {

    public:
        exprlist_t();
        exprlist_t(const fieldlist_t& fields);
        exprlist_t(const QList<const field_t*>& fields);
        exprlist_t(const field_t& field);
        exprlist_t(const expr_t& expr);
        exprlist_t(const std::initializer_list<expr_t>& values);
        QString toString(QSqlDriver* driver) const;
    };
    exprlist_t operator+(const exprlist_t& arg1, const exprlist_t& arg2);
    exprlist_t operator+(const exprlist_t& arg1, const QVariant& arg2);
    exprlist_t operator+(const QVariant& arg1, const exprlist_t& arg2);

    class joinexpr_t {

    public:
        enum join_type { join_inner, join_left, join_right };
        joinexpr_t();
        joinexpr_t(join_type type, const expr_t& expr);
        QString toString(const aliasedtable_t& table, QSqlDriver* driver) const;
        expr_t expr() const;
        join_type type() const;

    protected:
        expr_t mExpr;
        join_type mType;
    };

    class joinexprlist_t : public QList<joinexpr_t> {};

    class limit_t {

    public:
        limit_t();
        limit_t(int offset, int count);
        limit_t(int count);
        QString toString() const;
        int offset() const;
        void setOffset(int value);
        int count() const;
        void setCount(int value);

    protected:
        int mOffset;
        int mCount;
    };

    expr_t desc(const expr_t& arg);
    expr_t asc(const expr_t& arg);
    expr_t equal(const exprlist_t& args);
    expr_t equal(const expr_t& arg0, const expr_t& arg1);
    expr_t notEqual(const exprlist_t& args);
    expr_t notEqual(const expr_t& arg0, const expr_t& arg1);
    expr_t less(const exprlist_t& args);
    expr_t less(const expr_t& arg0, const expr_t& arg1);
    expr_t more(const exprlist_t& args);
    expr_t more(const expr_t& arg0, const expr_t& arg1);
    expr_t lessOrEqual(const exprlist_t& args);
    expr_t lessOrEqual(const expr_t& arg0, const expr_t& arg1);
    expr_t moreOrEqual(const exprlist_t& args);
    expr_t moreOrEqual(const expr_t& arg0, const expr_t& arg1);
    expr_t or_(const exprlist_t& args);
    expr_t or_(const expr_t& arg0, const expr_t& arg1);
    expr_t or_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t or_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t or_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
               const expr_t& arg4);
    expr_t and_(const exprlist_t& args);
    expr_t and_(const expr_t& arg0, const expr_t& arg1);
    expr_t and_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t and_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t and_(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
                const expr_t& arg4);
    expr_t add(const exprlist_t& args);
    expr_t add(const expr_t& arg0, const expr_t& arg1);
    expr_t add(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t add(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t add(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
               const expr_t& arg4);
    expr_t sub(const exprlist_t& args);
    expr_t sub(const expr_t& arg0, const expr_t& arg1);
    expr_t sub(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t sub(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t sub(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
               const expr_t& arg4);
    expr_t mul(const exprlist_t& args);
    expr_t mul(const expr_t& arg0, const expr_t& arg1);
    expr_t mul(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t mul(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t mul(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
               const expr_t& arg4);
    expr_t div(const exprlist_t& args);
    expr_t div(const expr_t& arg0, const expr_t& arg1);
    expr_t div(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2);
    expr_t div(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3);
    expr_t div(const expr_t& arg0, const expr_t& arg1, const expr_t& arg2, const expr_t& arg3,
               const expr_t& arg4);
    expr_t not_(const expr_t& arg);
    expr_t abs(const expr_t& arg);
    expr_t sum(const expr_t& arg);
    expr_t avg(const expr_t& arg);
    expr_t min(const expr_t& arg);
    expr_t max(const expr_t& arg);
    expr_t round(const expr_t& arg);

    namespace sqlite {

        expr_t mod(const exprlist_t& args);
        expr_t mod(const expr_t& arg0, const expr_t& arg1);
        expr_t datetime(const exprlist_t& args);
        expr_t datetime(const expr_t& arg0, const expr_t& arg1);

    } // namespace sqlite

    namespace mysql {

        expr_t sign(const expr_t& arg);
        expr_t floor(const expr_t& arg);
        expr_t ceil(const expr_t& arg);
        expr_t sqrt(const expr_t& arg);
        expr_t asin(const expr_t& arg);
        expr_t acos(const expr_t& arg);
        expr_t atan(const expr_t& arg);
        expr_t sin(const expr_t& arg);
        expr_t cos(const expr_t& arg);
        expr_t tan(const expr_t& arg);
        expr_t mod(const exprlist_t& args);
        expr_t mod(const expr_t& arg0, const expr_t& arg1);
        expr_t power(const exprlist_t& args);
        expr_t power(const expr_t& arg0, const expr_t& arg1);

    } // namespace mysql

    class query_t {

    public:
        query_t();
        query_t(const QSqlDatabase& database);
        virtual QString toString() const = 0;
        bool exec();
        static QString joinNotEmpty(const QStringList& values, const QString& glue = " ");
        QSqlError lastError() const;

    protected:
        QSqlQuery mQuery;
        QSqlDatabase mDatabase;
    };

    class select_t : public query_t {

    public:
        select_t(const exprlist_t& fields);
        select_t(const QSqlDatabase& database, const exprlist_t& fields);
        QVariant value(int index) const;
        QVariant value(const expr_t& expr) const;
        bool next();
        QString joinExpr() const;
        QString fromExpr() const;
        QString toString(const QString& prefix, const exprlist_t& exprs) const;
        QString toString(const QString& prefix, const expr_t& expr) const;
        QString toString() const;
        select_t& from(const table_t& table);
        select_t& from(const table_t& table, const alias_t& alias);
        select_t& leftJoin(const table_t& table);
        select_t& leftJoin(const table_t& table, const alias_t& alias);
        select_t& rightJoin(const table_t& table);
        select_t& rightJoin(const table_t& table, const alias_t& alias);
        select_t& innerJoin(const table_t& table);
        select_t& innerJoin(const table_t& table, const alias_t& alias);
        select_t& on(const expr_t& expr);
        select_t& on(const field_t& f1, const field_t& f2);
        select_t& where(const expr_t& cond);
        select_t& having(const expr_t& cond);
        select_t& orderBy(const exprlist_t& order);
        select_t& groupBy(const exprlist_t& fields);
        select_t& limit(int count);
        select_t& limit(int offset, int count);

    protected:
        select_t& join(joinexpr_t::join_type type, const aliasedtable_t& table);
        exprlist_t mFields;
        aliasedtablelist_t mTables;
        joinexprlist_t mJoinExprs;
        expr_t mWhere;
        expr_t mHaving;
        exprlist_t mOrderBy;
        limit_t mLimit;
        exprlist_t mGroupBy;
        joinexpr_t::join_type mJoinType;
    };
    select_t select(const exprlist_t& fields);
    select_t select(const QSqlDatabase& database, const exprlist_t& fields);

    class insert_t : public query_t {

    public:
        insert_t();
        insert_t(const QSqlDatabase& database);
        insert_t& into(const table_t& table, const exprlist_t& fields);
        insert_t& values(const exprlist_t& values);
        insert_t& values(const QVariantList& values);
        QVariant lastInsertId() const;
        QString fieldsToString() const;
        QString toString() const;

    protected:
        const table_t* mTable;
        exprlist_t mFields;
        exprlist_t mValues;
    };
    insert_t insert();
    insert_t insert(const QSqlDatabase& database);

    class update_t : public query_t {

    public:
        update_t();
        update_t(const table_t& table);
        update_t(const QSqlDatabase& database, const table_t& table);
        update_t& set(const exprlist_t& nameValues);
        update_t& where(const expr_t& expr);
        QString toString() const;

    protected:
        const table_t* mTable;
        expr_t mWhere;
        exprlist_t mNameValues;
    };
    update_t update(const table_t& table);
    update_t update(const QSqlDatabase& database, const table_t& table);

    class delete_t : public query_t {

    public:
        delete_t();
        delete_t(const QSqlDatabase& database);
        delete_t& from(const table_t& table);
        delete_t& where(const expr_t& expr);
        QString toString() const;

    protected:
        expr_t mWhere;
        const table_t* mTable;
    };
    delete_t delete_();
    delete_t delete_(const QSqlDatabase& database);

    class field_database_date_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_connectionName_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_driver_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_host_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_user_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_password_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_database_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_database_port_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class table_database_t : public table_t {

    public:
        table_database_t();
        QString tableName() const;
        QString aliasName() const;
        field_database_date_t date;
        field_database_connectionName_t connectionName;
        field_database_driver_t driver;
        field_database_host_t host;
        field_database_user_t user;
        field_database_password_t password;
        field_database_database_t database;
        field_database_port_t port;
        fieldlist_t _all;
    };

    extern table_database_t database;

    class field_query_date_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_query_connectionName_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_query_query_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class table_query_t : public table_t {

    public:
        table_query_t();
        QString tableName() const;
        QString aliasName() const;
        field_query_date_t date;
        field_query_connectionName_t connectionName;
        field_query_query_t query;
        fieldlist_t _all;
    };

    extern table_query_t query;

    class field_relations_name_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class field_relations_value_t : public field_t {

    public:
        QString name() const;
        QString fullName() const;
        QString aliasName() const;
    };

    class table_relations_t : public table_t {

    public:
        table_relations_t();
        QString tableName() const;
        QString aliasName() const;
        field_relations_name_t name;
        field_relations_value_t value;
        fieldlist_t _all;
    };

    extern table_relations_t relations;

} // namespace mugisql
