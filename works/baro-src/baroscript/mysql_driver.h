#ifndef _MYSQL_DRIVER_H
#define _MYSQL_DRIVER_H
#include "func_util.h"

#ifdef MYSQL_USE
#include <QtSql/qsqldriver.h>
#include <QtSql/qsqlresult.h>

#if defined (Q_OS_WIN32)
#include <QtCore/qt_windows.h>
#endif
#include <mysql.h>

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
    friend class QMYSQLDriver;
    friend class QMYSQLResultPrivate;
public:
    explicit QMYSQLResult(const QMYSQLDriver* db);
    ~QMYSQLResult();

    QVariant handle() const;
protected:
    void cleanup();
    bool fetch(int i);
    bool fetchNext();
    bool fetchLast();
    bool fetchFirst();
    QVariant data(int field);
    bool isNull(int field);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QVariant lastInsertId() const;
    QSqlRecord record() const;
    void virtual_hook(int id, void *data);
    bool nextResult();

#if MYSQL_VERSION_ID >= 40108
    bool prepare(const QString& stmt);
    bool exec();
#endif
private:
    QMYSQLResultPrivate* d;
};

class QMYSQLDriver : public QSqlDriver
{
    Q_OBJECT
    friend class QMYSQLResult;
public:
    explicit QMYSQLDriver(QObject *parent=0);
    explicit QMYSQLDriver(MYSQL *con, QObject * parent=0);
    ~QMYSQLDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
               const QString & user,
               const QString & password,
               const QString & host,
               int port,
               const QString& connOpts);
    void close();
    QSqlResult *createResult() const;
    QStringList tables(QSql::TableType) const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    QSqlRecord record(const QString& tablename) const;
    QString formatValue(const QSqlField &field,
                                     bool trimStrings) const;
    QVariant handle() const;
    QString escapeIdentifier(const QString &identifier, IdentifierType type) const;

protected Q_SLOTS:
    bool isIdentifierEscapedImplementation(const QString &identifier, IdentifierType type) const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    void init();
    QMYSQLDriverPrivate* d;
};
#endif // MYSQL_USE

#endif // _MYSQL_DRIVER_H
