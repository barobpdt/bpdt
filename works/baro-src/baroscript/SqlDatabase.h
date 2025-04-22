#ifndef SqlDatabase_H
#define SqlDatabase_H
#include <QtSql>
#include "../util/widget_util.h"

#ifdef PSQL_USE
#ifdef Q_OS_UNIX
#define HAVE_LONG_LONG 1 // force UnixODBC NOT to fall back to a struct for BIGINTs
#endif

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>

#include <sql.h>
#include <sqlext.h>

#if defined(Q_CC_BOR)
// workaround for Borland to make sure that SQLBIGINT is defined
#  define _MSC_VER 900
#endif

#if defined(Q_CC_BOR)
#  undef _MSC_VER
#endif

QT_BEGIN_HEADER

typedef struct pg_conn PGconn;
typedef struct pg_result PGresult;

QT_BEGIN_NAMESPACE

class QPSQLResultPrivate;
class QPSQLDriverPrivate;
class QPSQLDriver;
class QSqlRecordInfo;

class QPSQLResult : public QSqlResult
{
    friend class QPSQLResultPrivate;
public:
    QPSQLResult(const QPSQLDriver* db, const QPSQLDriverPrivate* p);
    ~QPSQLResult();

    QVariant handle() const;
    void virtual_hook(int id, void *data);

protected:
    void cleanup();
    bool fetch(int i);
    bool fetchFirst();
    bool fetchLast();
    QVariant data(int i);
    bool isNull(int field);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    QVariant lastInsertId() const;
    bool prepare(const QString& query);
    bool exec();

private:
    QPSQLResultPrivate *d;
};

class QPSQLDriver : public QSqlDriver
{
    Q_OBJECT
public:
    enum Protocol {
        VersionUnknown = -1,
        Version6 = 6,
        Version7 = 7,
        Version71 = 8,
        Version73 = 9,
        Version74 = 10,
        Version8 = 11,
        Version81 = 12,
        Version82 = 13,
        Version83 = 14,
        Version84 = 15,
        Version9 = 16,
    };

    explicit QPSQLDriver(QObject *parent=0);
    explicit QPSQLDriver(PGconn *conn, QObject *parent=0);
    ~QPSQLDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
              const QString & user,
              const QString & password,
              const QString & host,
              int port,
              const QString& connOpts);
    bool isOpen() const;
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QSqlRecord record(const QString& tablename) const;

    Protocol protocol() const;
    QVariant handle() const;

    QString escapeIdentifier(const QString &identifier, IdentifierType type) const;
    QString formatValue(const QSqlField &field, bool trimStrings) const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

protected Q_SLOTS:
    bool subscribeToNotificationImplementation(const QString &name);
    bool unsubscribeFromNotificationImplementation(const QString &name);
    QStringList subscribedToNotificationsImplementation() const;

private Q_SLOTS:
    void _q_handleNotification(int);

private:
    void init();
    QPSQLDriverPrivate *d;
};


class QODBCPrivate;
class QODBCDriverPrivate;
class QODBCDriver;
class QSqlRecordInfo;

class QODBCResult : public QSqlResult
{
public:
    QODBCResult(const QODBCDriver * db, QODBCDriverPrivate* p);
    virtual ~QODBCResult();

    bool prepare(const QString& query);
    bool exec();

    QVariant handle() const;
    virtual void setForwardOnly(bool forward);

protected:
    bool fetchNext();
    bool fetchFirst();
    bool fetchLast();
    bool fetchPrevious();
    bool fetch(int i);
    bool reset (const QString& query);
    QVariant data(int field);
    bool isNull(int field);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    void virtual_hook(int id, void *data);
    bool nextResult();

private:
    QODBCPrivate *d;
};

class QODBCDriver : public QSqlDriver
{
    Q_OBJECT
public:
    explicit QODBCDriver(QObject *parent=0);
    QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent=0);
    virtual ~QODBCDriver();
    bool hasFeature(DriverFeature f) const;
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QVariant handle() const;
    QString formatValue(const QSqlField &field,
                        bool trimStrings) const;
    bool open(const QString& db,
              const QString& user,
              const QString& password,
              const QString& host,
              int port,
              const QString& connOpts);

    QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

protected Q_SLOTS:
    bool isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

private:
    void init();
    bool endTrans();
    void cleanup();
    QODBCDriverPrivate* d;
    friend class QODBCPrivate;
};

QT_END_NAMESPACE
QT_END_HEADER
#endif

class XListArr;
class XFuncNode;
class SqlDatabase  : public WBox {
public:
    SqlDatabase(TreeNode* cf);
	~SqlDatabase();
public:
	QSqlDatabase*	xdb;
	QSqlQuery*		xquery;
    TreeNode*		xcf;
	U32				xstat;
    U32				xflag;
    StrVar			xvar;
	StrVar			xerror;
	XListArr		xarrs;
public:
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)	{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag) { return xflag&=~flag; }

    StrVar* lastError();
	StrVar*	lastInsertId();

	bool isOpen();
	bool isVaild();
	bool exec(LPCC sql);
	bool open(LPCC dsn);

	int fieldIndex(LPCC field);
	StrVar* fieldName(int idx);
	int affectRows();

	int fieldCount();
    SqlDatabase& bind(int idx, StrVar* sv);
    SqlDatabase& bindInt(int idx, int val);
    SqlDatabase& bindLong(int idx, UL64 val);
    SqlDatabase& bindBuffer(int idx, StrVar* val);
    SqlDatabase& bindStr(int idx, LPCC val);

    TreeNode*	fetchRoot(TreeNode* root);
    bool		fetchNode(WBox* node=NULL);
	TreeNode*	fetchTreeNode(TreeNode* root);
	int			fetchTreeResult(StrVar* rst, U16 flag=0);

	StrVar*		fetchValue(int idx=0);
	XListArr*	fields(U32 flag=0);
	bool prepare(LPCC sql);
	bool exec();
	void closeStatement();

	bool isDsn(LPCC dsn);
	bool addDsn(TreeNode* tn);
};

#endif

