#include "SqlDatabase.h"
#include "func_util.h"

#ifdef MYSQL_USE
#include "mysql_driver.h"
#endif

#ifdef PSQL_USE
#include "libpq-fe.h"
#include "pg_config.h"

#include <stdlib.h>
#include <math.h>

// below code taken from an example at http://www.gnu.org/software/hello/manual/autoconf/Function-Portability.html
#ifndef isnan
    # define isnan(x) \
        (sizeof (x) == sizeof (long double) ? isnan_ld (x) \
        : sizeof (x) == sizeof (double) ? isnan_d (x) \
        : isnan_f (x))
    static inline int isnan_f  (float       x) { return x != x; }
    static inline int isnan_d  (double      x) { return x != x; }
    static inline int isnan_ld (long double x) { return x != x; }
#endif

#ifndef isinf
    # define isinf(x) \
        (sizeof (x) == sizeof (long double) ? isinf_ld (x) \
        : sizeof (x) == sizeof (double) ? isinf_d (x) \
        : isinf_f (x))
    static inline int isinf_f  (float       x) { return isnan (x - x); }
    static inline int isinf_d  (double      x) { return isnan (x - x); }
    static inline int isinf_ld (long double x) { return isnan (x - x); }
#endif


// workaround for postgres defining their OIDs in a private header file
#define QBOOLOID 16
#define QINT8OID 20
#define QINT2OID 21
#define QINT4OID 23
#define QNUMERICOID 1700
#define QFLOAT4OID 700
#define QFLOAT8OID 701
#define QABSTIMEOID 702
#define QRELTIMEOID 703
#define QDATEOID 1082
#define QTIMEOID 1083
#define QTIMETZOID 1266
#define QTIMESTAMPOID 1114
#define QTIMESTAMPTZOID 1184
#define QOIDOID 2278
#define QBYTEAOID 17
#define QREGPROCOID 24
#define QXIDOID 28
#define QCIDOID 29

/* This is a compile time switch - if PQfreemem is declared, the compiler will use that one,
   otherwise it'll run in this template */
template <typename T>
inline void PQfreemem(T *t, int = 0) { free(t); }

Q_DECLARE_METATYPE(PGconn*)
Q_DECLARE_METATYPE(PGresult*)

QT_BEGIN_NAMESPACE

inline void qPQfreemem(void *buffer)
{
    PQfreemem(buffer);
}

class QPSQLDriverPrivate
{
public:
    QPSQLDriverPrivate() : connection(0), isUtf8(false), pro(QPSQLDriver::Version6), sn(0) {}
    PGconn *connection;
    bool isUtf8;
    QPSQLDriver::Protocol pro;
    QSocketNotifier *sn;
    QStringList seid;

    void appendTables(QStringList &tl, QSqlQuery &t, QChar type);
};

void QPSQLDriverPrivate::appendTables(QStringList &tl, QSqlQuery &t, QChar type)
{
    QString query;
    if (pro >= QPSQLDriver::Version73) {
        query = QString::fromLatin1("select pg_class.relname, pg_namespace.nspname from pg_class "
                  "left join pg_namespace on (pg_class.relnamespace = pg_namespace.oid) "
                  "where (pg_class.relkind = '%1') and (pg_class.relname !~ '^Inv') "
                  "and (pg_class.relname !~ '^pg_') "
                  "and (pg_namespace.nspname != 'information_schema') ").arg(type);
    } else {
        query = QString::fromLatin1("select relname, null from pg_class where (relkind = '%1') "
                  "and (relname !~ '^Inv') "
                  "and (relname !~ '^pg_') ").arg(type);
    }
    t.exec(query);
    while (t.next()) {
        QString schema = t.value(1).toString();
        if (schema.isEmpty() || schema == QLatin1String("public"))
            tl.append(t.value(0).toString());
        else
            tl.append(t.value(0).toString().prepend(QLatin1Char('.')).prepend(schema));
    }
}

class QPSQLResultPrivate
{
public:
    QPSQLResultPrivate(QPSQLResult *qq): q(qq), driver(0), result(0), currentSize(-1), preparedQueriesEnabled(false) {}

    QPSQLResult *q;
    const QPSQLDriverPrivate *driver;
    PGresult *result;
    int currentSize;
    bool preparedQueriesEnabled;
    QString preparedStmtId;

    bool processResults();
};

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type,
                            const QPSQLDriverPrivate *p)
{
    const char *s = PQerrorMessage(p->connection);
    QString msg = p->isUtf8 ? QString::fromUtf8(s) : QString::fromLocal8Bit(s);
    return QSqlError(QLatin1String("QPSQL: ") + err, msg, type);
}

bool QPSQLResultPrivate::processResults()
{
    if (!result)
        return false;

    int status = PQresultStatus(result);
    if (status == PGRES_TUPLES_OK) {
        q->setSelect(true);
        q->setActive(true);
        currentSize = PQntuples(result);
        return true;
    } else if (status == PGRES_COMMAND_OK) {
        q->setSelect(false);
        q->setActive(true);
        currentSize = -1;
        return true;
    }
    q->setLastError(qMakeError(QCoreApplication::translate("QPSQLResult",
                    "Unable to create query"), QSqlError::StatementError, driver));
    return false;
}

static QVariant::Type qDecodePSQLType(int t)
{
    QVariant::Type type = QVariant::Invalid;
    switch (t) {
    case QBOOLOID:
        type = QVariant::Bool;
        break;
    case QINT8OID:
        type = QVariant::LongLong;
        break;
    case QINT2OID:
    case QINT4OID:
    case QOIDOID:
    case QREGPROCOID:
    case QXIDOID:
    case QCIDOID:
        type = QVariant::Int;
        break;
    case QNUMERICOID:
    case QFLOAT4OID:
    case QFLOAT8OID:
        type = QVariant::Double;
        break;
    case QABSTIMEOID:
    case QRELTIMEOID:
    case QDATEOID:
        type = QVariant::Date;
        break;
    case QTIMEOID:
    case QTIMETZOID:
        type = QVariant::Time;
        break;
    case QTIMESTAMPOID:
    case QTIMESTAMPTZOID:
        type = QVariant::DateTime;
        break;
    case QBYTEAOID:
        type = QVariant::ByteArray;
        break;
    default:
        type = QVariant::String;
        break;
    }
    return type;
}

static void qDeallocatePreparedStmt(QPSQLResultPrivate *d)
{
    const QString stmt = QLatin1String("DEALLOCATE ") + d->preparedStmtId;
    PGresult *result = PQexec(d->driver->connection,
                              d->driver->isUtf8 ? stmt.toUtf8().constData()
                                                : stmt.toLocal8Bit().constData());

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
        qWarning("Unable to free statement: %s", PQerrorMessage(d->driver->connection));
    PQclear(result);
    d->preparedStmtId.clear();
}

QPSQLResult::QPSQLResult(const QPSQLDriver* db, const QPSQLDriverPrivate* p)
    : QSqlResult(db)
{
    d = new QPSQLResultPrivate(this);
    d->driver = p;
    d->preparedQueriesEnabled = db->hasFeature(QSqlDriver::PreparedQueries);
}

QPSQLResult::~QPSQLResult()
{
    cleanup();

    if (d->preparedQueriesEnabled && !d->preparedStmtId.isNull())
        qDeallocatePreparedStmt(d);

    delete d;
}

QVariant QPSQLResult::handle() const
{
    return QVariant::fromValue(d->result);
}

void QPSQLResult::cleanup()
{
    if (d->result)
        PQclear(d->result);
    d->result = 0;
    setAt(QSql::BeforeFirstRow);
    d->currentSize = -1;
    setActive(false);
}

bool QPSQLResult::fetch(int i)
{
    if (!isActive())
        return false;
    if (i < 0)
        return false;
    if (i >= d->currentSize)
        return false;
    if (at() == i)
        return true;
    setAt(i);
    return true;
}

bool QPSQLResult::fetchFirst()
{
    return fetch(0);
}

bool QPSQLResult::fetchLast()
{
    return fetch(PQntuples(d->result) - 1);
}

QVariant QPSQLResult::data(int i)
{
    if (i >= PQnfields(d->result)) {
        qWarning("QPSQLResult::data: column %d out of range", i);
        return QVariant();
    }
    int ptype = PQftype(d->result, i);
    QVariant::Type type = qDecodePSQLType(ptype);
    const char *val = PQgetvalue(d->result, at(), i);
    if (PQgetisnull(d->result, at(), i))
        return QVariant(type);
    switch (type) {
    case QVariant::Bool:
        return QVariant((bool)(val[0] == 't'));
    case QVariant::String:
        return d->driver->isUtf8 ? QString::fromUtf8(val) : QString::fromLocal8Bit(val);
    case QVariant::LongLong:
        if (val[0] == '-')
            return QString::fromLatin1(val).toLongLong();
        else
            return QString::fromLatin1(val).toULongLong();
    case QVariant::Int:
        return atoi(val);
    case QVariant::Double:
        if (ptype == QNUMERICOID) {
            if (numericalPrecisionPolicy() != QSql::HighPrecision) {
                QVariant retval;
                bool convert;
                double dbl=QString::fromLocal8Bit(val).toDouble(&convert);
                if (numericalPrecisionPolicy() == QSql::LowPrecisionInt64)
                    retval = (qlonglong)dbl;
                else if (numericalPrecisionPolicy() == QSql::LowPrecisionInt32)
                    retval = (int)dbl;
                else if (numericalPrecisionPolicy() == QSql::LowPrecisionDouble)
                    retval = dbl;
                if (!convert)
                    return QVariant();
                return retval;
            }
            return QString::fromLocal8Bit(val);
        }
        return QString::fromLocal8Bit(val).toDouble();
    case QVariant::Date:
        if (val[0] == '\0') {
            return QVariant(QDate());
        } else {
#ifndef QT_NO_DATESTRING
            return QVariant(QDate::fromString(QString::fromLatin1(val), Qt::ISODate));
#else
            return QVariant(QString::fromLatin1(val));
#endif
        }
    case QVariant::Time: {
        const QString str = QString::fromLatin1(val);
#ifndef QT_NO_DATESTRING
        if (str.isEmpty())
            return QVariant(QTime());
        if (str.at(str.length() - 3) == QLatin1Char('+') || str.at(str.length() - 3) == QLatin1Char('-'))
            // strip the timezone
            // TODO: fix this when timestamp support comes into QDateTime
            return QVariant(QTime::fromString(str.left(str.length() - 3), Qt::ISODate));
        return QVariant(QTime::fromString(str, Qt::ISODate));
#else
        return QVariant(str);
#endif
    }
    case QVariant::DateTime: {
        QString dtval = QString::fromLatin1(val);
#ifndef QT_NO_DATESTRING
        if (dtval.length() < 10)
            return QVariant(QDateTime());
        // remove the timezone
        // TODO: fix this when timestamp support comes into QDateTime
        if (dtval.at(dtval.length() - 3) == QLatin1Char('+') || dtval.at(dtval.length() - 3) == QLatin1Char('-'))
            dtval.chop(3);
        // milliseconds are sometimes returned with 2 digits only
        if (dtval.at(dtval.length() - 3).isPunct())
            dtval += QLatin1Char('0');
        if (dtval.isEmpty())
            return QVariant(QDateTime());
        else
            return QVariant(QDateTime::fromString(dtval, Qt::ISODate));
#else
        return QVariant(dtval);
#endif
    }
    case QVariant::ByteArray: {
        size_t len;
        unsigned char *data = PQunescapeBytea((unsigned char*)val, &len);
        QByteArray ba((const char*)data, len);
        qPQfreemem(data);
        return QVariant(ba);
    }
    default:
    case QVariant::Invalid:
        qWarning("QPSQLResult::data: unknown data type");
    }
    return QVariant();
}

bool QPSQLResult::isNull(int field)
{
    PQgetvalue(d->result, at(), field);
    return PQgetisnull(d->result, at(), field);
}

bool QPSQLResult::reset (const QString& query)
{
    cleanup();
    if (!driver())
        return false;
    if (!driver()->isOpen() || driver()->isOpenError())
        return false;
    d->result = PQexec(d->driver->connection,
                       d->driver->isUtf8 ? query.toUtf8().constData()
                                         : query.toLocal8Bit().constData());
    return d->processResults();
}

int QPSQLResult::size()
{
    return d->currentSize;
}

int QPSQLResult::numRowsAffected()
{
    return QString::fromLatin1(PQcmdTuples(d->result)).toInt();
}

QVariant QPSQLResult::lastInsertId() const
{
    if (isActive()) {
        Oid id = PQoidValue(d->result);
        if (id != InvalidOid)
            return QVariant(id);
    }
    return QVariant();
}

QSqlRecord QPSQLResult::record() const
{
    QSqlRecord info;
    if (!isActive() || !isSelect())
        return info;

    int count = PQnfields(d->result);
    for (int i = 0; i < count; ++i) {
        QSqlField f;
        if (d->driver->isUtf8)
            f.setName(QString::fromUtf8(PQfname(d->result, i)));
        else
            f.setName(QString::fromLocal8Bit(PQfname(d->result, i)));
        f.setType(qDecodePSQLType(PQftype(d->result, i)));
        int len = PQfsize(d->result, i);
        int precision = PQfmod(d->result, i);
        // swap length and precision if length == -1
        if (len == -1 && precision > -1) {
            len = precision - 4;
            precision = -1;
        }
        f.setLength(len);
        f.setPrecision(precision);
        f.setSqlType(PQftype(d->result, i));
        info.append(f);
    }
    return info;
}

void QPSQLResult::virtual_hook(int id, void *data)
{
    Q_ASSERT(data);

    switch (id) {
    default:
        QSqlResult::virtual_hook(id, data);
    }
}

static QString qReplacePlaceholderMarkers(const QString &query)
{
    const int originalLength = query.length();
    bool inQuote = false;
    int markerIdx = 0;
    QString result;
    result.reserve(originalLength + 23);
    for (int i = 0; i < originalLength; ++i) {
        const QChar ch = query.at(i);
        if (ch == QLatin1Char('?') && !inQuote) {
            result += QLatin1Char('$');
            result += QString::number(++markerIdx);
        } else {
            if (ch == QLatin1Char('\''))
                inQuote = !inQuote;
            result += ch;
        }
    }

    result.squeeze();
    return result;
}

static QString qCreateParamString(const QVector<QVariant> boundValues, const QSqlDriver *driver)
{
    if (boundValues.isEmpty())
        return QString();

    QString params;
    QSqlField f;
    for (int i = 0; i < boundValues.count(); ++i) {
        const QVariant &val = boundValues.at(i);

        f.setType(val.type());
        if (val.isNull())
            f.clear();
        else
            f.setValue(val);
        if(!params.isNull())
            params.append(QLatin1String(", "));
        params.append(driver->formatValue(f));
    }
    return params;
}

Q_GLOBAL_STATIC(QMutex, qMutex)
QString qMakePreparedStmtId()
{
    qMutex()->lock();
    static unsigned int qPreparedStmtCount = 0;
    QString id = QLatin1String("qpsqlpstmt_") + QString::number(++qPreparedStmtCount, 16);
    qMutex()->unlock();
    return id;
}

bool QPSQLResult::prepare(const QString &query)
{
    if (!d->preparedQueriesEnabled)
        return QSqlResult::prepare(query);

    cleanup();

    if (!d->preparedStmtId.isEmpty())
        qDeallocatePreparedStmt(d);

    const QString stmtId = qMakePreparedStmtId();
    const QString stmt = QString::fromLatin1("PREPARE %1 AS ").arg(stmtId).append(qReplacePlaceholderMarkers(query));

    PGresult *result = PQexec(d->driver->connection,
                              d->driver->isUtf8 ? stmt.toUtf8().constData()
                                                : stmt.toLocal8Bit().constData());

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        setLastError(qMakeError(QCoreApplication::translate("QPSQLResult",
                                "Unable to prepare statement"), QSqlError::StatementError, d->driver));
        PQclear(result);
        d->preparedStmtId.clear();
        return false;
    }

    PQclear(result);
    d->preparedStmtId = stmtId;
    return true;
}

bool QPSQLResult::exec()
{
    if (!d->preparedQueriesEnabled)
        return QSqlResult::exec();

    cleanup();

    QString stmt;
    const QString params = qCreateParamString(boundValues(), d->q->driver());
    if (params.isEmpty())
        stmt = QString::fromLatin1("EXECUTE %1").arg(d->preparedStmtId);
    else
        stmt = QString::fromLatin1("EXECUTE %1 (%2)").arg(d->preparedStmtId).arg(params);

    d->result = PQexec(d->driver->connection,
                       d->driver->isUtf8 ? stmt.toUtf8().constData()
                                         : stmt.toLocal8Bit().constData());

    return d->processResults();
}

///////////////////////////////////////////////////////////////////

static bool setEncodingUtf8(PGconn* connection)
{
    PGresult* result = PQexec(connection, "SET CLIENT_ENCODING TO 'UNICODE'");
    int status = PQresultStatus(result);
    PQclear(result);
    return status == PGRES_COMMAND_OK;
}

static void setDatestyle(PGconn* connection)
{
    PGresult* result = PQexec(connection, "SET DATESTYLE TO 'ISO'");
    int status =  PQresultStatus(result);
    if (status != PGRES_COMMAND_OK)
        qWarning("%s", PQerrorMessage(connection));
    PQclear(result);
}

static QPSQLDriver::Protocol qMakePSQLVersion(int vMaj, int vMin)
{
    switch (vMaj) {
    case 6:
        return QPSQLDriver::Version6;
    case 7:
    {
        switch (vMin) {
        case 1:
            return QPSQLDriver::Version71;
        case 3:
            return QPSQLDriver::Version73;
        case 4:
            return QPSQLDriver::Version74;
        default:
            return QPSQLDriver::Version7;
        }
        break;
    }
    case 8:
    {
        switch (vMin) {
        case 1:
            return QPSQLDriver::Version81;
        case 2:
            return QPSQLDriver::Version82;
        case 3:
            return QPSQLDriver::Version83;
        case 4:
            return QPSQLDriver::Version84;
        default:
            return QPSQLDriver::Version8;
        }
        break;
    }
    case 9:
        return QPSQLDriver::Version9;
        break;
    default:
        break;
    }
    return QPSQLDriver::VersionUnknown;
}

static QPSQLDriver::Protocol getPSQLVersion(PGconn* connection)
{
    QPSQLDriver::Protocol serverVersion = QPSQLDriver::Version6;
    PGresult* result = PQexec(connection, "select version()");
    int status = PQresultStatus(result);
    if (status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK) {
        QString val = QString::fromLocal8Bit(PQgetvalue(result, 0, 0));

        QRegExp rx(QLatin1String("(\\d+)\\.(\\d+)"));
        rx.setMinimal(true); // enforce non-greedy RegExp

        if (rx.indexIn(val) != -1) {
            int vMaj = rx.cap(1).toInt();
            int vMin = rx.cap(2).toInt();
            serverVersion = qMakePSQLVersion(vMaj, vMin);
#ifdef PG_MAJORVERSION
            if (rx.indexIn(QLatin1String(PG_MAJORVERSION)) != -1) {
                vMaj = rx.cap(1).toInt();
                vMin = rx.cap(2).toInt();
            }
            QPSQLDriver::Protocol clientVersion = qMakePSQLVersion(vMaj, vMin);

            if (serverVersion >= QPSQLDriver::Version9 && clientVersion < QPSQLDriver::Version9) {
                //Client version before QPSQLDriver::Version9 only supports escape mode for bytea type,
                //but bytea format is set to hex by default in PSQL 9 and above. So need to force the
                //server use the old escape mode when connects to the new server with old client library.
                result = PQexec(connection, "SET bytea_output=escape; ");
                status = PQresultStatus(result);
            } else if (serverVersion == QPSQLDriver::VersionUnknown) {
                serverVersion = clientVersion;
                if (serverVersion != QPSQLDriver::VersionUnknown)
                   qWarning("The server version of this PostgreSQL is unknown, falling back to the client version.");
            }
#endif
        }
    }
    PQclear(result);

    //keep the old behavior unchanged
    if (serverVersion == QPSQLDriver::VersionUnknown)
        serverVersion = QPSQLDriver::Version6;

    if (serverVersion < QPSQLDriver::Version71) {
        qWarning("This version of PostgreSQL is not supported and may not work.");
    }

    return serverVersion;
}

QPSQLDriver::QPSQLDriver(QObject *parent)
    : QSqlDriver(parent)
{
    init();
}

QPSQLDriver::QPSQLDriver(PGconn *conn, QObject *parent)
    : QSqlDriver(parent)
{
    init();
    d->connection = conn;
    if (conn) {
        d->pro = getPSQLVersion(d->connection);
        setOpen(true);
        setOpenError(false);
    }
}

void QPSQLDriver::init()
{
    d = new QPSQLDriverPrivate();
}

QPSQLDriver::~QPSQLDriver()
{
    if (d->connection)
        PQfinish(d->connection);
    delete d;
}

QVariant QPSQLDriver::handle() const
{
    return QVariant::fromValue(d->connection);
}

bool QPSQLDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions:
    case QuerySize:
    case LastInsertId:
    case LowPrecisionNumbers:
    case EventNotifications:
        return true;
    case PreparedQueries:
    case PositionalPlaceholders:
        return d->pro >= QPSQLDriver::Version82;
    case BatchOperations:
    case NamedPlaceholders:
    case SimpleLocking:
    case FinishQuery:
    case MultipleResultSets:
        return false;
    case BLOB:
        return d->pro >= QPSQLDriver::Version71;
    case Unicode:
        return d->isUtf8;
    }
    return false;
}

/*
   Quote a string for inclusion into the connection string
   \ -> \\
   ' -> \'
   surround string by single quotes
 */
static QString qQuote(QString s)
{
    s.replace(QLatin1Char('\\'), QLatin1String("\\\\"));
    s.replace(QLatin1Char('\''), QLatin1String("\\'"));
    s.append(QLatin1Char('\'')).prepend(QLatin1Char('\''));
    return s;
}

bool QPSQLDriver::open(const QString & db,
                        const QString & user,
                        const QString & password,
                        const QString & host,
                        int port,
                        const QString& connOpts)
{
    if (isOpen())
        close();
    QString connectString;
    if (!host.isEmpty())
        connectString.append(QLatin1String("host=")).append(qQuote(host));
    if (!db.isEmpty())
        connectString.append(QLatin1String(" dbname=")).append(qQuote(db));
    if (!user.isEmpty())
        connectString.append(QLatin1String(" user=")).append(qQuote(user));
    if (!password.isEmpty())
        connectString.append(QLatin1String(" password=")).append(qQuote(password));
    if (port != -1)
        connectString.append(QLatin1String(" port=")).append(qQuote(QString::number(port)));

    // add any connect options - the server will handle error detection
    if (!connOpts.isEmpty()) {
        QString opt = connOpts;
        opt.replace(QLatin1Char(';'), QLatin1Char(' '), Qt::CaseInsensitive);
        connectString.append(QLatin1Char(' ')).append(opt);
    }

    d->connection = PQconnectdb(connectString.toLocal8Bit().constData());
    if (PQstatus(d->connection) == CONNECTION_BAD) {
        setLastError(qMakeError(tr("Unable to connect"), QSqlError::ConnectionError, d));
        setOpenError(true);
        PQfinish(d->connection);
        d->connection = 0;
        return false;
    }

    d->pro = getPSQLVersion(d->connection);
    d->isUtf8 = setEncodingUtf8(d->connection);
    setDatestyle(d->connection);

    setOpen(true);
    setOpenError(false);
    return true;
}

void QPSQLDriver::close()
{
    if (isOpen()) {

        d->seid.clear();
        if (d->sn) {
            disconnect(d->sn, SIGNAL(activated(int)), this, SLOT(_q_handleNotification(int)));
            delete d->sn;
            d->sn = 0;
        }

        if (d->connection)
            PQfinish(d->connection);
        d->connection = 0;
        setOpen(false);
        setOpenError(false);
    }
}

QSqlResult *QPSQLDriver::createResult() const
{
    return new QPSQLResult(this, d);
}

bool QPSQLDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::beginTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "BEGIN");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        PQclear(res);
        setLastError(qMakeError(tr("Could not begin transaction"),
                                QSqlError::TransactionError, d));
        return false;
    }
    PQclear(res);
    return true;
}

bool QPSQLDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::commitTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "COMMIT");

    bool transaction_failed = false;

    // XXX
    // This hack is used to tell if the transaction has succeeded for the protocol versions of
    // PostgreSQL below. For 7.x and other protocol versions we are left in the dark.
    // This hack can dissapear once there is an API to query this sort of information.
    if (d->pro == QPSQLDriver::Version8 ||
        d->pro == QPSQLDriver::Version81 ||
        d->pro == QPSQLDriver::Version82 ||
        d->pro == QPSQLDriver::Version83 ||
        d->pro == QPSQLDriver::Version84 ||
        d->pro == QPSQLDriver::Version9) {
        transaction_failed = qstrcmp(PQcmdStatus(res), "ROLLBACK") == 0;
    }

    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK || transaction_failed) {
        PQclear(res);
        setLastError(qMakeError(tr("Could not commit transaction"),
                                QSqlError::TransactionError, d));
        return false;
    }
    PQclear(res);
    return true;
}

bool QPSQLDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::rollbackTransaction: Database not open");
        return false;
    }
    PGresult* res = PQexec(d->connection, "ROLLBACK");
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        setLastError(qMakeError(tr("Could not rollback transaction"),
                                QSqlError::TransactionError, d));
        PQclear(res);
        return false;
    }
    PQclear(res);
    return true;
}

QStringList QPSQLDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;
    QSqlQuery t(createResult());
    t.setForwardOnly(true);

    if (type & QSql::Tables)
        d->appendTables(tl, t, QLatin1Char('r'));
    if (type & QSql::Views)
        d->appendTables(tl, t, QLatin1Char('v'));
    if (type & QSql::SystemTables) {
        t.exec(QLatin1String("select relname from pg_class where (relkind = 'r') "
                "and (relname like 'pg_%') "));
        while (t.next())
            tl.append(t.value(0).toString());
    }

    return tl;
}

static void qSplitTableName(QString &tablename, QString &schema)
{
    int dot = tablename.indexOf(QLatin1Char('.'));
    if (dot == -1)
        return;
    schema = tablename.left(dot);
    tablename = tablename.mid(dot + 1);
}

QSqlIndex QPSQLDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex idx(tablename);
    if (!isOpen())
        return idx;
    QSqlQuery i(createResult());
    QString stmt;

    QString tbl = tablename;
    QString schema;
    qSplitTableName(tbl, schema);

    if (isIdentifierEscaped(tbl, QSqlDriver::TableName))
        tbl = stripDelimiters(tbl, QSqlDriver::TableName);
    else
        tbl = tbl.toLower();

    if (isIdentifierEscaped(schema, QSqlDriver::TableName))
        schema = stripDelimiters(schema, QSqlDriver::TableName);
    else
        schema = schema.toLower();

    switch(d->pro) {
    case QPSQLDriver::Version6:
        stmt = QLatin1String("select pg_att1.attname, int(pg_att1.atttypid), pg_cl.relname "
                "from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
                "where pg_cl.relname = '%1_pkey' "
                "and pg_cl.oid = pg_ind.indexrelid "
                "and pg_att2.attrelid = pg_ind.indexrelid "
                "and pg_att1.attrelid = pg_ind.indrelid "
                "and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
                "order by pg_att2.attnum");
        break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
        stmt = QLatin1String("select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
                "from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
                "where pg_cl.relname = '%1_pkey' "
                "and pg_cl.oid = pg_ind.indexrelid "
                "and pg_att2.attrelid = pg_ind.indexrelid "
                "and pg_att1.attrelid = pg_ind.indrelid "
                "and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
                "order by pg_att2.attnum");
        break;
    case QPSQLDriver::Version73:
    case QPSQLDriver::Version74:
    case QPSQLDriver::Version8:
    case QPSQLDriver::Version81:
    case QPSQLDriver::Version82:
    case QPSQLDriver::Version83:
    case QPSQLDriver::Version84:
    case QPSQLDriver::Version9:
        stmt = QLatin1String("SELECT pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_class.relname "
                "FROM pg_attribute, pg_class "
                "WHERE %1 pg_class.oid IN "
                "(SELECT indexrelid FROM pg_index WHERE indisprimary = true AND indrelid IN "
                " (SELECT oid FROM pg_class WHERE relname = '%2')) "
                "AND pg_attribute.attrelid = pg_class.oid "
                "AND pg_attribute.attisdropped = false "
                "ORDER BY pg_attribute.attnum");
        if (schema.isEmpty())
            stmt = stmt.arg(QLatin1String("pg_table_is_visible(pg_class.oid) AND"));
        else
            stmt = stmt.arg(QString::fromLatin1("pg_class.relnamespace = (select oid from "
                   "pg_namespace where pg_namespace.nspname = '%1') AND ").arg(schema));
        break;
    case QPSQLDriver::VersionUnknown:
        qFatal("PSQL version is unknown");
        break;
    }

    i.exec(stmt.arg(tbl));
    while (i.isActive() && i.next()) {
        QSqlField f(i.value(0).toString(), qDecodePSQLType(i.value(1).toInt()));
        idx.append(f);
        idx.setName(i.value(2).toString());
    }
    return idx;
}

QSqlRecord QPSQLDriver::record(const QString& tablename) const
{
    QSqlRecord info;
    if (!isOpen())
        return info;

    QString tbl = tablename;
    QString schema;
    qSplitTableName(tbl, schema);

    if (isIdentifierEscaped(tbl, QSqlDriver::TableName))
        tbl = stripDelimiters(tbl, QSqlDriver::TableName);
    else
        tbl = tbl.toLower();

    if (isIdentifierEscaped(schema, QSqlDriver::TableName))
        schema = stripDelimiters(schema, QSqlDriver::TableName);
    else
        schema = schema.toLower();

    QString stmt;
    switch(d->pro) {
    case QPSQLDriver::Version6:
        stmt = QLatin1String("select pg_attribute.attname, int(pg_attribute.atttypid), "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "int(pg_attribute.attrelid), pg_attribute.attnum "
                "from pg_class, pg_attribute "
                "where pg_class.relname = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid ");
        break;
    case QPSQLDriver::Version7:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attribute.attrelid::int, pg_attribute.attnum "
                "from pg_class, pg_attribute "
                "where pg_class.relname = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid ");
        break;
    case QPSQLDriver::Version71:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attrdef.adsrc "
                "from pg_class, pg_attribute "
                "left join pg_attrdef on (pg_attrdef.adrelid = "
                "pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
                "where pg_class.relname = '%1' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid "
                "order by pg_attribute.attnum ");
        break;
    case QPSQLDriver::Version73:
    case QPSQLDriver::Version74:
    case QPSQLDriver::Version8:
    case QPSQLDriver::Version81:
    case QPSQLDriver::Version82:
    case QPSQLDriver::Version83:
    case QPSQLDriver::Version84:
    case QPSQLDriver::Version9:
        stmt = QLatin1String("select pg_attribute.attname, pg_attribute.atttypid::int, "
                "pg_attribute.attnotnull, pg_attribute.attlen, pg_attribute.atttypmod, "
                "pg_attrdef.adsrc "
                "from pg_class, pg_attribute "
                "left join pg_attrdef on (pg_attrdef.adrelid = "
                "pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
                "where %1 "
                "and pg_class.relname = '%2' "
                "and pg_attribute.attnum > 0 "
                "and pg_attribute.attrelid = pg_class.oid "
                "and pg_attribute.attisdropped = false "
                "order by pg_attribute.attnum ");
        if (schema.isEmpty())
            stmt = stmt.arg(QLatin1String("pg_table_is_visible(pg_class.oid)"));
        else
            stmt = stmt.arg(QString::fromLatin1("pg_class.relnamespace = (select oid from "
                   "pg_namespace where pg_namespace.nspname = '%1')").arg(schema));
        break;
    case QPSQLDriver::VersionUnknown:
        qFatal("PSQL version is unknown");
        break;
    }

    QSqlQuery query(createResult());
    query.exec(stmt.arg(tbl));
    if (d->pro >= QPSQLDriver::Version71) {
        while (query.next()) {
            int len = query.value(3).toInt();
            int precision = query.value(4).toInt();
            // swap length and precision if length == -1
            if (len == -1 && precision > -1) {
                len = precision - 4;
                precision = -1;
            }
            QString defVal = query.value(5).toString();
            if (!defVal.isEmpty() && defVal.at(0) == QLatin1Char('\''))
                defVal = defVal.mid(1, defVal.length() - 2);
            QSqlField f(query.value(0).toString(), qDecodePSQLType(query.value(1).toInt()));
            f.setRequired(query.value(2).toBool());
            f.setLength(len);
            f.setPrecision(precision);
            f.setDefaultValue(defVal);
            f.setSqlType(query.value(1).toInt());
            info.append(f);
        }
    } else {
        // Postgres < 7.1 cannot handle outer joins
        while (query.next()) {
            QString defVal;
            QString stmt2 = QLatin1String("select pg_attrdef.adsrc from pg_attrdef where "
                            "pg_attrdef.adrelid = %1 and pg_attrdef.adnum = %2 ");
            QSqlQuery query2(createResult());
            query2.exec(stmt2.arg(query.value(5).toInt()).arg(query.value(6).toInt()));
            if (query2.isActive() && query2.next())
                defVal = query2.value(0).toString();
            if (!defVal.isEmpty() && defVal.at(0) == QLatin1Char('\''))
                defVal = defVal.mid(1, defVal.length() - 2);
            int len = query.value(3).toInt();
            int precision = query.value(4).toInt();
            // swap length and precision if length == -1
            if (len == -1 && precision > -1) {
                len = precision - 4;
                precision = -1;
            }
            QSqlField f(query.value(0).toString(), qDecodePSQLType(query.value(1).toInt()));
            f.setRequired(query.value(2).toBool());
            f.setLength(len);
            f.setPrecision(precision);
            f.setDefaultValue(defVal);
            f.setSqlType(query.value(1).toInt());
            info.append(f);
        }
    }

    return info;
}

QString QPSQLDriver::formatValue(const QSqlField &field, bool trimStrings) const
{
    QString r;
    if (field.isNull()) {
        r = QLatin1String("NULL");
    } else {
        switch (field.type()) {
        case QVariant::DateTime:
#ifndef QT_NO_DATESTRING
            if (field.value().toDateTime().isValid()) {
                QDate dt = field.value().toDateTime().date();
                QTime tm = field.value().toDateTime().time();
                // msecs need to be right aligned otherwise psql
                // interpretes them wrong
                r = QLatin1Char('\'') + QString::number(dt.year()) + QLatin1Char('-')
                          + QString::number(dt.month()) + QLatin1Char('-')
                          + QString::number(dt.day()) + QLatin1Char(' ')
                          + tm.toString() + QLatin1Char('.')
                          + QString::number(tm.msec()).rightJustified(3, QLatin1Char('0'))
                          + QLatin1Char('\'');
            } else {
                r = QLatin1String("NULL");
            }
#else
            r = QLatin1String("NULL");
#endif // QT_NO_DATESTRING
            break;
        case QVariant::Time:
#ifndef QT_NO_DATESTRING
            if (field.value().toTime().isValid()) {
                r = QLatin1Char('\'') + field.value().toTime().toString(QLatin1String("hh:mm:ss.zzz")) + QLatin1Char('\'');
            } else
#endif
            {
                r = QLatin1String("NULL");
            }
            break;
        case QVariant::String:
        {
            // Escape '\' characters
            r = QSqlDriver::formatValue(field, trimStrings);
            r.replace(QLatin1String("\\"), QLatin1String("\\\\"));
            break;
        }
        case QVariant::Bool:
            if (field.value().toBool())
                r = QLatin1String("TRUE");
            else
                r = QLatin1String("FALSE");
            break;
        case QVariant::ByteArray: {
            QByteArray ba(field.value().toByteArray());
            size_t len;
#if defined PG_VERSION_NUM && PG_VERSION_NUM-0 >= 80200
            unsigned char *data = PQescapeByteaConn(d->connection, (unsigned char*)ba.constData(), ba.size(), &len);
#else
            unsigned char *data = PQescapeBytea((unsigned char*)ba.constData(), ba.size(), &len);
#endif
            r += QLatin1Char('\'');
            r += QLatin1String((const char*)data);
            r += QLatin1Char('\'');
            qPQfreemem(data);
            break;
        }
        case QVariant::Double: {
            double val = field.value().toDouble();
            if (isnan(val))
                r = QLatin1String("'NaN'");
            else {
                int res = isinf(val);
                if (res == 1)
                    r = QLatin1String("'Infinity'");
                else if (res == -1)
                    r = QLatin1String("'-Infinity'");
                else
                    r = QSqlDriver::formatValue(field, trimStrings);
            }
            break;
        }
        default:
            r = QSqlDriver::formatValue(field, trimStrings);
            break;
        }
    }
    return r;
}

QString QPSQLDriver::escapeIdentifier(const QString &identifier, IdentifierType) const
{
    QString res = identifier;
    if(!identifier.isEmpty() && !identifier.startsWith(QLatin1Char('"')) && !identifier.endsWith(QLatin1Char('"')) ) {
        res.replace(QLatin1Char('"'), QLatin1String("\"\""));
        res.prepend(QLatin1Char('"')).append(QLatin1Char('"'));
        res.replace(QLatin1Char('.'), QLatin1String("\".\""));
    }
    return res;
}

bool QPSQLDriver::isOpen() const
{
    return PQstatus(d->connection) == CONNECTION_OK;
}

QPSQLDriver::Protocol QPSQLDriver::protocol() const
{
    return d->pro;
}

bool QPSQLDriver::subscribeToNotificationImplementation(const QString &name)
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::subscribeToNotificationImplementation: database not open.");
        return false;
    }

    if (d->seid.contains(name)) {
        qWarning("QPSQLDriver::subscribeToNotificationImplementation: already subscribing to '%s'.",
            qPrintable(name));
        return false;
    }

    int socket = PQsocket(d->connection);
    if (socket) {
        QString query = QLatin1String("LISTEN ") + escapeIdentifier(name, QSqlDriver::TableName);
        if (PQresultStatus(PQexec(d->connection,
                                  d->isUtf8 ? query.toUtf8().constData()
                                            : query.toLocal8Bit().constData())
                          ) != PGRES_COMMAND_OK) {
            setLastError(qMakeError(tr("Unable to subscribe"), QSqlError::StatementError, d));
            return false;
        }

        if (!d->sn) {
            d->sn = new QSocketNotifier(socket, QSocketNotifier::Read);
            connect(d->sn, SIGNAL(activated(int)), this, SLOT(_q_handleNotification(int)));
        }
    }

    d->seid << name;
    return true;
}

bool QPSQLDriver::unsubscribeFromNotificationImplementation(const QString &name)
{
    if (!isOpen()) {
        qWarning("QPSQLDriver::unsubscribeFromNotificationImplementation: database not open.");
        return false;
    }

    if (!d->seid.contains(name)) {
        qWarning("QPSQLDriver::unsubscribeFromNotificationImplementation: not subscribed to '%s'.",
            qPrintable(name));
        return false;
    }

    QString query = QLatin1String("UNLISTEN ") + escapeIdentifier(name, QSqlDriver::TableName);
    if (PQresultStatus(PQexec(d->connection,
                              d->isUtf8 ? query.toUtf8().constData()
                                        : query.toLocal8Bit().constData())
                      ) != PGRES_COMMAND_OK) {
        setLastError(qMakeError(tr("Unable to unsubscribe"), QSqlError::StatementError, d));
        return false;
    }

    d->seid.removeAll(name);

    if (d->seid.isEmpty()) {
        disconnect(d->sn, SIGNAL(activated(int)), this, SLOT(_q_handleNotification(int)));
        delete d->sn;
        d->sn = 0;
    }

    return true;
}

QStringList QPSQLDriver::subscribedToNotificationsImplementation() const
{
    return d->seid;
}


void QPSQLDriver::_q_handleNotification(int)
{
    PQconsumeInput(d->connection);

    PGnotify *notify = 0;
    while((notify = PQnotifies(d->connection)) != 0) {
        QString name(QLatin1String(notify->relname));
        if (d->seid.contains(name))
            emit notification(name);
        else
            qWarning("QPSQLDriver: received notification for '%s' which isn't subscribed to.",
                    qPrintable(name));

        qPQfreemem(notify);
    }
}



//
// ODBC driver
//


// undefine this to prevent initial check of the ODBC driver
#define ODBC_CHECK_DRIVER

static const int COLNAMESIZE = 256;
//Map Qt parameter types to ODBC types
static const SQLSMALLINT qParamType[4] = { SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, SQL_PARAM_INPUT_OUTPUT };

inline static QString fromSQLTCHAR(const QVarLengthArray<SQLTCHAR>& input, int size=-1)
{
    QString result;

    int realsize = qMin(size, input.size());
    if(realsize > 0 && input[realsize-1] == 0)
        realsize--;
    switch(sizeof(SQLTCHAR)) {
        case 1:
            result=QString::fromUtf8((const char *)input.constData(), realsize);
            break;
        case 2:
            result=QString::fromUtf16((const ushort *)input.constData(), realsize);
            break;
        case 4:
            result=QString::fromUcs4((const uint *)input.constData(), realsize);
            break;
        default:
            qCritical() << "sizeof(SQLTCHAR) is " << sizeof(SQLTCHAR) << "Don't know how to handle this";
    }
    return result;
}

inline static QVarLengthArray<SQLTCHAR> toSQLTCHAR(const QString &input)
{
    QVarLengthArray<SQLTCHAR> result;
    result.resize(input.size());
    switch(sizeof(SQLTCHAR)) {
        case 1:
            memcpy(result.data(), input.toUtf8().data(), input.size());
            break;
        case 2:
            memcpy(result.data(), input.unicode(), input.size() * 2);
            break;
        case 4:
            memcpy(result.data(), input.toUcs4().data(), input.size() * 4);
            break;
        default:
            qCritical() << "sizeof(SQLTCHAR) is " << sizeof(SQLTCHAR) << "Don't know how to handle this";
    }
    result.append(0); // make sure it's null terminated, doesn't matter if it already is, it does if it isn't.
    return result;
}

class QODBCDriverPrivate
{
public:
    enum DefaultCase{Lower, Mixed, Upper, Sensitive};
    QODBCDriverPrivate()
    : hEnv(0), hDbc(0), unicode(false), useSchema(false), disconnectCount(0), isMySqlServer(false),
           isMSSqlServer(false), isFreeTDSDriver(false), hasSQLFetchScroll(true),
           hasMultiResultSets(false), isQuoteInitialized(false), quote(QLatin1Char('"'))
    {
    }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;

    bool unicode;
    bool useSchema;
    int disconnectCount;
    bool isMySqlServer;
    bool isMSSqlServer;
    bool isFreeTDSDriver;
    bool hasSQLFetchScroll;
    bool hasMultiResultSets;

    bool checkDriver() const;
    void checkUnicode();
    void checkSqlServer();
    void checkHasSQLFetchScroll();
    void checkHasMultiResults();
    void checkSchemaUsage();
    bool setConnectionOptions(const QString& connOpts);
    void splitTableQualifier(const QString &qualifier, QString &catalog,
                             QString &schema, QString &table);
    DefaultCase defaultCase() const;
    QString adjustCase(const QString&) const;
    QChar quoteChar();
private:
    bool isQuoteInitialized;
    QChar quote;
};

class QODBCPrivate
{
public:
    QODBCPrivate(QODBCDriverPrivate *dpp)
    : hStmt(0), useSchema(false), hasSQLFetchScroll(true), driverPrivate(dpp), userForwardOnly(false)
    {
        unicode = dpp->unicode;
        useSchema = dpp->useSchema;
        disconnectCount = dpp->disconnectCount;
        hasSQLFetchScroll = dpp->hasSQLFetchScroll;
    }

    inline void clearValues()
    { fieldCache.fill(QVariant()); fieldCacheIdx = 0; }

    SQLHANDLE dpEnv() const { return driverPrivate ? driverPrivate->hEnv : 0;}
    SQLHANDLE dpDbc() const { return driverPrivate ? driverPrivate->hDbc : 0;}
    SQLHANDLE hStmt;

    bool unicode;
    bool useSchema;

    QSqlRecord rInf;
    QVector<QVariant> fieldCache;
    int fieldCacheIdx;
    int disconnectCount;
    bool hasSQLFetchScroll;
    QODBCDriverPrivate *driverPrivate;
    bool userForwardOnly;

    bool isStmtHandleValid(const QSqlDriver *driver);
    void updateStmtHandleState(const QSqlDriver *driver);
};

bool QODBCPrivate::isStmtHandleValid(const QSqlDriver *driver)
{
    const QODBCDriver *odbcdriver = static_cast<const QODBCDriver*> (driver);
    return disconnectCount == odbcdriver->d->disconnectCount;
}

void QODBCPrivate::updateStmtHandleState(const QSqlDriver *driver)
{
    const QODBCDriver *odbcdriver = static_cast<const QODBCDriver*> (driver);
    disconnectCount = odbcdriver->d->disconnectCount;
}

static QString qWarnODBCHandle(int handleType, SQLHANDLE handle, int *nativeCode = 0)
{
    SQLINTEGER nativeCode_ = 0;
    SQLSMALLINT msgLen = 0;
    SQLRETURN r = SQL_NO_DATA;
    SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
    QVarLengthArray<SQLTCHAR> description_(SQL_MAX_MESSAGE_LENGTH);
    QString result;
    int i = 1;

    description_[0] = 0;
    do {
        r = SQLGetDiagRec(handleType,
                          handle,
                          i,
                          state_,
                          &nativeCode_,
                          0,
                          0,
                          &msgLen);
        if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && msgLen > 0)
            description_.resize(msgLen+1);
        r = SQLGetDiagRec(handleType,
                            handle,
                            i,
                            state_,
                            &nativeCode_,
                            description_.data(),
                            description_.size(),
                            &msgLen);
        if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
            if (nativeCode)
                *nativeCode = nativeCode_;
            QString tmpstore;
#ifdef UNICODE
            tmpstore = fromSQLTCHAR(description_, msgLen);
#else
            tmpstore = QString::fromUtf8((const char*)description_.constData(), msgLen);
#endif
            if(result != tmpstore) {
                if(!result.isEmpty())
                    result += QLatin1Char(' ');
                result += tmpstore;
            }
        } else if (r == SQL_ERROR || r == SQL_INVALID_HANDLE) {
            return result;
        }
        ++i;
    } while (r != SQL_NO_DATA);
    return result;
}

static QString qODBCWarn(const QODBCPrivate* odbc, int *nativeCode = 0)
{
    return QString(qWarnODBCHandle(SQL_HANDLE_ENV, odbc->dpEnv()) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->dpDbc()) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_STMT, odbc->hStmt, nativeCode)).simplified();
}

static QString qODBCWarn(const QODBCDriverPrivate* odbc, int *nativeCode = 0)
{
    return QString(qWarnODBCHandle(SQL_HANDLE_ENV, odbc->hEnv) + QLatin1Char(' ')
             + qWarnODBCHandle(SQL_HANDLE_DBC, odbc->hDbc, nativeCode)).simplified();
}

static void qSqlWarning(const QString& message, const QODBCPrivate* odbc)
{
    qWarning() << message << "\tError:" << qODBCWarn(odbc);
}

static void qSqlWarning(const QString &message, const QODBCDriverPrivate *odbc)
{
    qWarning() << message << "\tError:" << qODBCWarn(odbc);
}

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type, const QODBCPrivate* p)
{
    int nativeCode = -1;
    QString message = qODBCWarn(p, &nativeCode);
    return QSqlError(QLatin1String("QODBC3: ") + err, message, type, nativeCode);
}

static QSqlError qMakeError(const QString& err, QSqlError::ErrorType type,
                            const QODBCDriverPrivate* p)
{
    int nativeCode = -1;
    QString message = qODBCWarn(p, &nativeCode);
    return QSqlError(QLatin1String("QODBC3: ") + err, qODBCWarn(p), type, nativeCode);
}

template<class T>
static QVariant::Type qDecodeODBCType(SQLSMALLINT sqltype, const T* p, bool isSigned = true)
{
    Q_UNUSED(p);
    QVariant::Type type = QVariant::Invalid;
    switch (sqltype) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
        type = QVariant::Double;
        break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
        type = isSigned ? QVariant::Int : QVariant::UInt;
        break;
    case SQL_TINYINT:
        type = QVariant::UInt;
        break;
    case SQL_BIGINT:
        type = isSigned ? QVariant::LongLong : QVariant::ULongLong;
        break;
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
        type = QVariant::ByteArray;
        break;
    case SQL_DATE:
    case SQL_TYPE_DATE:
        type = QVariant::Date;
        break;
    case SQL_TIME:
    case SQL_TYPE_TIME:
        type = QVariant::Time;
        break;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
        type = QVariant::DateTime;
        break;
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
        type = QVariant::String;
        break;
    case SQL_CHAR:
    case SQL_VARCHAR:
#if (ODBCVER >= 0x0350)
    case SQL_GUID:
#endif
    case SQL_LONGVARCHAR:
        type = QVariant::String;
        break;
    default:
        type = QVariant::ByteArray;
        break;
    }
    return type;
}

static QString qGetStringData(SQLHANDLE hStmt, int column, int colSize, bool unicode = false)
{
    QString fieldVal;
    SQLRETURN r = SQL_ERROR;
    SQLLEN lengthIndicator = 0;

    // NB! colSize must be a multiple of 2 for unicode enabled DBs
    if (colSize <= 0) {
        colSize = 256;
    } else if (colSize > 65536) { // limit buffer size to 64 KB
        colSize = 65536;
    } else {
        colSize++; // make sure there is room for more than the 0 termination
    }
    if(unicode) {
        r = SQLGetData(hStmt,
                        column+1,
                        SQL_C_TCHAR,
                        NULL,
                        0,
                        &lengthIndicator);
        if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && lengthIndicator > 0)
            colSize = lengthIndicator/sizeof(SQLTCHAR) + 1;
        QVarLengthArray<SQLTCHAR> buf(colSize);
        memset(buf.data(), 0, colSize*sizeof(SQLTCHAR));
        while (true) {
            r = SQLGetData(hStmt,
                            column+1,
                            SQL_C_TCHAR,
                            (SQLPOINTER)buf.data(),
                            colSize*sizeof(SQLTCHAR),
                            &lengthIndicator);
            if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
                if (lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL) {
                    fieldVal.clear();
                    break;
                }
                // if SQL_SUCCESS_WITH_INFO is returned, indicating that
                // more data can be fetched, the length indicator does NOT
                // contain the number of bytes returned - it contains the
                // total number of bytes that CAN be fetched
                // colSize-1: remove 0 termination when there is more data to fetch
                int rSize = (r == SQL_SUCCESS_WITH_INFO) ? colSize : lengthIndicator/sizeof(SQLTCHAR);
                    fieldVal += fromSQLTCHAR(buf, rSize);
                if (lengthIndicator < SQLLEN(colSize*sizeof(SQLTCHAR))) {
                    // workaround for Drivermanagers that don't return SQL_NO_DATA
                    break;
                }
            } else if (r == SQL_NO_DATA) {
                break;
            } else {
                qWarning() << "qGetStringData: Error while fetching data (" << qWarnODBCHandle(SQL_HANDLE_STMT, hStmt) << ')';
                fieldVal.clear();
                break;
            }
        }
    } else {
        r = SQLGetData(hStmt,
                        column+1,
                        SQL_C_CHAR,
                        NULL,
                        0,
                        &lengthIndicator);
        if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && lengthIndicator > 0)
            colSize = lengthIndicator + 1;
        QVarLengthArray<SQLCHAR> buf(colSize);
        while (true) {
            r = SQLGetData(hStmt,
                            column+1,
                            SQL_C_CHAR,
                            (SQLPOINTER)buf.data(),
                            colSize,
                            &lengthIndicator);
            if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
                if (lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL) {
                    fieldVal.clear();
                    break;
                }
                // if SQL_SUCCESS_WITH_INFO is returned, indicating that
                // more data can be fetched, the length indicator does NOT
                // contain the number of bytes returned - it contains the
                // total number of bytes that CAN be fetched
                // colSize-1: remove 0 termination when there is more data to fetch
                int rSize = (r == SQL_SUCCESS_WITH_INFO) ? colSize : lengthIndicator;
                    fieldVal += QString::fromUtf8((const char *)buf.constData(), rSize);
                if (lengthIndicator < SQLLEN(colSize)) {
                    // workaround for Drivermanagers that don't return SQL_NO_DATA
                    break;
                }
            } else if (r == SQL_NO_DATA) {
                break;
            } else {
                qWarning() << "qGetStringData: Error while fetching data (" << qWarnODBCHandle(SQL_HANDLE_STMT, hStmt) << ')';
                fieldVal.clear();
                break;
            }
        }
    }
    return fieldVal;
}

static QVariant qGetBinaryData(SQLHANDLE hStmt, int column)
{
    QByteArray fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLULEN colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLLEN lengthIndicator = 0;
    SQLRETURN r = SQL_ERROR;

    QVarLengthArray<SQLTCHAR> colName(COLNAMESIZE);

    r = SQLDescribeCol(hStmt,
                       column + 1,
                       colName.data(),
                       COLNAMESIZE,
                       &colNameLen,
                       &colType,
                       &colSize,
                       &colScale,
                       &nullable);
    if (r != SQL_SUCCESS)
        qWarning() << "qGetBinaryData: Unable to describe column" << column;
    // SQLDescribeCol may return 0 if size cannot be determined
    if (!colSize)
        colSize = 255;
    else if (colSize > 65536) // read the field in 64 KB chunks
        colSize = 65536;
    fieldVal.resize(colSize);
    ulong read = 0;
    while (true) {
        r = SQLGetData(hStmt,
                        column+1,
                        SQL_C_BINARY,
                        (SQLPOINTER)(fieldVal.constData() + read),
                        colSize,
                        &lengthIndicator);
        if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
            break;
        if (lengthIndicator == SQL_NULL_DATA)
            return QVariant(QVariant::ByteArray);
        if (lengthIndicator > SQLLEN(colSize) || lengthIndicator == SQL_NO_TOTAL) {
            read += colSize;
            colSize = 65536;
        } else {
            read += lengthIndicator;
        }
        if (r == SQL_SUCCESS) { // the whole field was read in one chunk
            fieldVal.resize(read);
            break;
        }
        fieldVal.resize(fieldVal.size() + colSize);
    }
    return fieldVal;
}

static QVariant qGetIntData(SQLHANDLE hStmt, int column, bool isSigned = true)
{
    SQLINTEGER intbuf = 0;
    SQLLEN lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              isSigned ? SQL_C_SLONG : SQL_C_ULONG,
                              (SQLPOINTER)&intbuf,
                              sizeof(intbuf),
                              &lengthIndicator);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
        return QVariant(QVariant::Invalid);
    if (lengthIndicator == SQL_NULL_DATA)
        return QVariant(QVariant::Int);
    if (isSigned)
        return int(intbuf);
    else
        return uint(intbuf);
}

static QVariant qGetDoubleData(SQLHANDLE hStmt, int column)
{
    SQLDOUBLE dblbuf;
    SQLLEN lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              SQL_C_DOUBLE,
                              (SQLPOINTER) &dblbuf,
                              0,
                              &lengthIndicator);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        return QVariant(QVariant::Invalid);
    }
    if(lengthIndicator == SQL_NULL_DATA)
        return QVariant(QVariant::Double);

    return (double) dblbuf;
}


static QVariant qGetBigIntData(SQLHANDLE hStmt, int column, bool isSigned = true)
{
    SQLBIGINT lngbuf = 0;
    SQLLEN lengthIndicator = 0;
    SQLRETURN r = SQLGetData(hStmt,
                              column+1,
                              isSigned ? SQL_C_SBIGINT : SQL_C_UBIGINT,
                              (SQLPOINTER) &lngbuf,
                              sizeof(lngbuf),
                              &lengthIndicator);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
        return QVariant(QVariant::Invalid);
    if (lengthIndicator == SQL_NULL_DATA)
        return QVariant(QVariant::LongLong);

    if (isSigned)
        return qint64(lngbuf);
    else
        return quint64(lngbuf);
}

// creates a QSqlField from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
static QSqlField qMakeFieldInfo(const SQLHANDLE hStmt, const QODBCDriverPrivate* p)
{
    QString fname = qGetStringData(hStmt, 3, -1, p->unicode);
    int type = qGetIntData(hStmt, 4).toInt(); // column type
    QSqlField f(fname, qDecodeODBCType(type, p));
    int required = qGetIntData(hStmt, 10).toInt(); // nullable-flag
    // required can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    if (required == SQL_NO_NULLS)
        f.setRequired(true);
    else if (required == SQL_NULLABLE)
        f.setRequired(false);
    // else we don't know
    QVariant var = qGetIntData(hStmt, 6);
    f.setLength(var.isNull() ? -1 : var.toInt()); // column size
    var = qGetIntData(hStmt, 8).toInt();
    f.setPrecision(var.isNull() ? -1 : var.toInt()); // precision
    f.setSqlType(type);
    return f;
}

static QSqlField qMakeFieldInfo(const QODBCPrivate* p, int i )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLULEN colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    QVarLengthArray<SQLTCHAR> colName(COLNAMESIZE);
    r = SQLDescribeCol(p->hStmt,
                        i+1,
                        colName.data(),
                        (SQLSMALLINT)COLNAMESIZE,
                        &colNameLen,
                        &colType,
                        &colSize,
                        &colScale,
                        &nullable);

    if (r != SQL_SUCCESS) {
        qSqlWarning(QString::fromLatin1("qMakeField: Unable to describe column %1").arg(i), p);
        return QSqlField();
    }

    SQLLEN unsignedFlag = SQL_FALSE;
    r = SQLColAttribute (p->hStmt,
                         i + 1,
                         SQL_DESC_UNSIGNED,
                         0,
                         0,
                         0,
                         &unsignedFlag);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QString::fromLatin1("qMakeField: Unable to get column attributes for column %1").arg(i), p);
    }

#ifdef UNICODE
    QString qColName(fromSQLTCHAR(colName, colNameLen));
#else
    QString qColName = QString::fromUtf8((const char *)colName.constData());
#endif
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if (nullable == SQL_NO_NULLS) {
        required = 1;
    } else if (nullable == SQL_NULLABLE) {
        required = 0;
    }
    QVariant::Type type = qDecodeODBCType(colType, p, unsignedFlag == SQL_FALSE);
    QSqlField f(qColName, type);
    f.setSqlType(colType);
    f.setLength(colSize == 0 ? -1 : int(colSize));
    f.setPrecision(colScale == 0 ? -1 : int(colScale));
    if (nullable == SQL_NO_NULLS)
        f.setRequired(true);
    else if (nullable == SQL_NULLABLE)
        f.setRequired(false);
    // else we don't know
    return f;
}

static int qGetODBCVersion(const QString &connOpts)
{
    if (connOpts.contains(QLatin1String("SQL_ATTR_ODBC_VERSION=SQL_OV_ODBC3"), Qt::CaseInsensitive))
        return SQL_OV_ODBC3;
    return SQL_OV_ODBC2;
}

QChar QODBCDriverPrivate::quoteChar()
{
    if (!isQuoteInitialized) {
        SQLTCHAR driverResponse[4];
        SQLSMALLINT length;
        int r = SQLGetInfo(hDbc,
                SQL_IDENTIFIER_QUOTE_CHAR,
                &driverResponse,
                sizeof(driverResponse),
                &length);
        if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
#ifdef UNICODE
            quote = QChar(driverResponse[0]);
#else
            quote = QLatin1Char(driverResponse[0]);
#endif
        else
            quote = QLatin1Char('"');
        isQuoteInitialized = true;
    }
    return quote;
}


bool QODBCDriverPrivate::setConnectionOptions(const QString& connOpts)
{
    // Set any connection attributes
    const QStringList opts(connOpts.split(QLatin1Char(';'), QString::SkipEmptyParts));
    SQLRETURN r = SQL_SUCCESS;
    for (int i = 0; i < opts.count(); ++i) {
        const QString tmp(opts.at(i));
        int idx;
        if ((idx = tmp.indexOf(QLatin1Char('='))) == -1) {
            qWarning() << "QODBCDriver::open: Illegal connect option value '" << tmp << '\'';
            continue;
        }
        const QString opt(tmp.left(idx));
        const QString val(tmp.mid(idx + 1).simplified());
        SQLUINTEGER v = 0;

        r = SQL_SUCCESS;
        if (opt.toUpper() == QLatin1String("SQL_ATTR_ACCESS_MODE")) {
            if (val.toUpper() == QLatin1String("SQL_MODE_READ_ONLY")) {
                v = SQL_MODE_READ_ONLY;
            } else if (val.toUpper() == QLatin1String("SQL_MODE_READ_WRITE")) {
                v = SQL_MODE_READ_WRITE;
            } else {
                qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_CONNECTION_TIMEOUT")) {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_LOGIN_TIMEOUT")) {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_CURRENT_CATALOG")) {
            val.utf16(); // 0 terminate
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CURRENT_CATALOG,
#ifdef UNICODE
                                    toSQLTCHAR(val).data(),
#else
                                    (SQLCHAR*) val.toUtf8().data(),
#endif
                                    val.length()*sizeof(SQLTCHAR));
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_METADATA_ID")) {
            if (val.toUpper() == QLatin1String("SQL_TRUE")) {
                v = SQL_TRUE;
            } else if (val.toUpper() == QLatin1String("SQL_FALSE")) {
                v = SQL_FALSE;
            } else {
                qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_PACKET_SIZE")) {
            v = val.toUInt();
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_PACKET_SIZE, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_TRACEFILE")) {
            val.utf16(); // 0 terminate
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACEFILE,
#ifdef UNICODE
                                    toSQLTCHAR(val).data(),
#else
                                    (SQLCHAR*) val.toUtf8().data(),
#endif
                                    val.length()*sizeof(SQLTCHAR));
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_TRACE")) {
            if (val.toUpper() == QLatin1String("SQL_OPT_TRACE_OFF")) {
                v = SQL_OPT_TRACE_OFF;
            } else if (val.toUpper() == QLatin1String("SQL_OPT_TRACE_ON")) {
                v = SQL_OPT_TRACE_ON;
            } else {
                qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_TRACE, (SQLPOINTER) v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_CONNECTION_POOLING")) {
            if (val == QLatin1String("SQL_CP_OFF"))
                v = SQL_CP_OFF;
            else if (val.toUpper() == QLatin1String("SQL_CP_ONE_PER_DRIVER"))
                v = SQL_CP_ONE_PER_DRIVER;
            else if (val.toUpper() == QLatin1String("SQL_CP_ONE_PER_HENV"))
                v = SQL_CP_ONE_PER_HENV;
            else if (val.toUpper() == QLatin1String("SQL_CP_DEFAULT"))
                v = SQL_CP_DEFAULT;
            else {
                qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_CP_MATCH")) {
            if (val.toUpper() == QLatin1String("SQL_CP_STRICT_MATCH"))
                v = SQL_CP_STRICT_MATCH;
            else if (val.toUpper() == QLatin1String("SQL_CP_RELAXED_MATCH"))
                v = SQL_CP_RELAXED_MATCH;
            else if (val.toUpper() == QLatin1String("SQL_CP_MATCH_DEFAULT"))
                v = SQL_CP_MATCH_DEFAULT;
            else {
                qWarning() << "QODBCDriver::open: Unknown option value '" << val << '\'';
                continue;
            }
            r = SQLSetConnectAttr(hDbc, SQL_ATTR_CP_MATCH, (SQLPOINTER)v, 0);
        } else if (opt.toUpper() == QLatin1String("SQL_ATTR_ODBC_VERSION")) {
            // Already handled in QODBCDriver::open()
            continue;
        } else {
                qWarning() << "QODBCDriver::open: Unknown connection attribute '" << opt << '\'';
        }
        if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO)
            qSqlWarning(QString::fromLatin1("QODBCDriver::open: Unable to set connection attribute'%1'").arg(
                        opt), this);
    }
    return true;
}

void QODBCDriverPrivate::splitTableQualifier(const QString & qualifier, QString &catalog,
                                       QString &schema, QString &table)
{
    if (!useSchema) {
        table = qualifier;
        return;
    }
    QStringList l = qualifier.split(QLatin1Char('.'));
    if (l.count() > 3)
        return; // can't possibly be a valid table qualifier
    int i = 0, n = l.count();
    if (n == 1) {
        table = qualifier;
    } else {
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
            if (n == 3) {
                if (i == 0) {
                    catalog = *it;
                } else if (i == 1) {
                    schema = *it;
                } else if (i == 2) {
                    table = *it;
                }
            } else if (n == 2) {
                if (i == 0) {
                    schema = *it;
                } else if (i == 1) {
                    table = *it;
                }
            }
            i++;
        }
    }
}

QODBCDriverPrivate::DefaultCase QODBCDriverPrivate::defaultCase() const
{
    DefaultCase ret;
    SQLUSMALLINT casing;
    int r = SQLGetInfo(hDbc,
            SQL_IDENTIFIER_CASE,
            &casing,
            sizeof(casing),
            NULL);
    if ( r != SQL_SUCCESS)
        ret = Mixed;//arbitrary case if driver cannot be queried
    else {
        switch (casing) {
            case (SQL_IC_UPPER):
                ret = Upper;
                break;
            case (SQL_IC_LOWER):
                ret = Lower;
                break;
            case (SQL_IC_SENSITIVE):
                ret = Sensitive;
                break;
            case (SQL_IC_MIXED):
            default:
                ret = Mixed;
                break;
        }
    }
    return ret;
}

/*
   Adjust the casing of an identifier to match what the
   database engine would have done to it.
*/
QString QODBCDriverPrivate::adjustCase(const QString &identifier) const
{
    QString ret = identifier;
    switch(defaultCase()) {
        case (Lower):
            ret = identifier.toLower();
            break;
        case (Upper):
            ret = identifier.toUpper();
            break;
        case(Mixed):
        case(Sensitive):
        default:
            ret = identifier;
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////

QODBCResult::QODBCResult(const QODBCDriver * db, QODBCDriverPrivate* p)
: QSqlResult(db)
{
    d = new QODBCPrivate(p);
}

QODBCResult::~QODBCResult()
{
    if (d->hStmt && d->isStmtHandleValid(driver()) && driver()->isOpen()) {
        SQLRETURN r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS)
            qSqlWarning(QLatin1String("QODBCDriver: Unable to free statement handle ")
                         + QString::number(r), d);
    }

    delete d;
}

bool QODBCResult::reset (const QString& query)
{
    setActive(false);
    setAt(QSql::BeforeFirstRow);
    d->rInf.clear();
    d->fieldCache.clear();
    d->fieldCacheIdx = 0;

    // Always reallocate the statement handle - the statement attributes
    // are not reset if SQLFreeStmt() is called which causes some problems.
    SQLRETURN r;
    if (d->hStmt && d->isStmtHandleValid(driver())) {
        r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS) {
            qSqlWarning(QLatin1String("QODBCResult::reset: Unable to free statement handle"), d);
            return false;
        }
    }
    r  = SQLAllocHandle(SQL_HANDLE_STMT,
                         d->dpDbc(),
                         &d->hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QLatin1String("QODBCResult::reset: Unable to allocate statement handle"), d);
        return false;
    }

    d->updateStmtHandleState(driver());

    if (d->userForwardOnly) {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                            SQL_IS_UINTEGER);
    } else {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_STATIC,
                            SQL_IS_UINTEGER);
    }
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
            "QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. "
            "Please check your ODBC driver configuration"), QSqlError::StatementError, d));
        return false;
    }

#ifdef UNICODE
    r = SQLExecDirect(d->hStmt,
                       toSQLTCHAR(query).data(),
                       (SQLINTEGER) query.length());
#else
    QByteArray query8 = query.toUtf8();
    r = SQLExecDirect(d->hStmt,
                       (SQLCHAR*) query8.data(),
                       (SQLINTEGER) query8.length());
#endif
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r!= SQL_NO_DATA) {
        setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                     "Unable to execute statement"), QSqlError::StatementError, d));
        return false;
    }

    if(r == SQL_NO_DATA) {
        setSelect(false);
        return true;
    }

    SQLINTEGER isScrollable, bufferLength;
    r = SQLGetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, SQL_IS_INTEGER, &bufferLength);
    if(r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
        QSqlResult::setForwardOnly(isScrollable==SQL_NONSCROLLABLE);

    SQLSMALLINT count;
    SQLNumResultCols(d->hStmt, &count);
    if (count) {
        setSelect(true);
        for (int i = 0; i < count; ++i) {
            d->rInf.append(qMakeFieldInfo(d, i));
        }
        d->fieldCache.resize(count);
    } else {
        setSelect(false);
    }
    setActive(true);

    return true;
}

bool QODBCResult::fetch(int i)
{
    if (!driver()->isOpen())
        return false;

    if (isForwardOnly() && i < at())
        return false;
    if (i == at())
        return true;
    d->clearValues();
    int actualIdx = i + 1;
    if (actualIdx <= 0) {
        setAt(QSql::BeforeFirstRow);
        return false;
    }
    SQLRETURN r;
    if (isForwardOnly()) {
        bool ok = true;
        while (ok && i > at())
            ok = fetchNext();
        return ok;
    } else {
        r = SQLFetchScroll(d->hStmt,
                            SQL_FETCH_ABSOLUTE,
                            actualIdx);
    }
    if (r != SQL_SUCCESS) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                "Unable to fetch"), QSqlError::ConnectionError, d));
        return false;
    }
    setAt(i);
    return true;
}

bool QODBCResult::fetchNext()
{
    SQLRETURN r;
    d->clearValues();

    if (d->hasSQLFetchScroll)
        r = SQLFetchScroll(d->hStmt,
                           SQL_FETCH_NEXT,
                           0);
    else
        r = SQLFetch(d->hStmt);

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                "Unable to fetch next"), QSqlError::ConnectionError, d));
        return false;
    }
    setAt(at() + 1);
    return true;
}

bool QODBCResult::fetchFirst()
{
    if (isForwardOnly() && at() != QSql::BeforeFirstRow)
        return false;
    SQLRETURN r;
    d->clearValues();
    if (isForwardOnly()) {
        return fetchNext();
    }
    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_FIRST,
                       0);
    if (r != SQL_SUCCESS) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                "Unable to fetch first"), QSqlError::ConnectionError, d));
        return false;
    }
    setAt(0);
    return true;
}

bool QODBCResult::fetchPrevious()
{
    if (isForwardOnly())
        return false;
    SQLRETURN r;
    d->clearValues();
    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_PRIOR,
                       0);
    if (r != SQL_SUCCESS) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                "Unable to fetch previous"), QSqlError::ConnectionError, d));
        return false;
    }
    setAt(at() - 1);
    return true;
}

bool QODBCResult::fetchLast()
{
    SQLRETURN r;
    d->clearValues();

    if (isForwardOnly()) {
        // cannot seek to last row in forwardOnly mode, so we have to use brute force
        int i = at();
        if (i == QSql::AfterLastRow)
            return false;
        if (i == QSql::BeforeFirstRow)
            i = 0;
        while (fetchNext())
            ++i;
        setAt(i);
        return true;
    }

    r = SQLFetchScroll(d->hStmt,
                       SQL_FETCH_LAST,
                       0);
    if (r != SQL_SUCCESS) {
        if (r != SQL_NO_DATA)
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                "Unable to fetch last"), QSqlError::ConnectionError, d));
        return false;
    }
    SQLINTEGER currRow;
    r = SQLGetStmtAttr(d->hStmt,
                        SQL_ROW_NUMBER,
                        &currRow,
                        SQL_IS_INTEGER,
                        0);
    if (r != SQL_SUCCESS)
        return false;
    setAt(currRow-1);
    return true;
}

QVariant QODBCResult::data(int field)
{
    if (field >= d->rInf.count() || field < 0) {
        qWarning() << "QODBCResult::data: column" << field << "out of range";
        return QVariant();
    }
    if (field < d->fieldCacheIdx)
        return d->fieldCache.at(field);

    SQLRETURN r(0);
    SQLLEN lengthIndicator = 0;

    for (int i = d->fieldCacheIdx; i <= field; ++i) {
        // some servers do not support fetching column n after we already
        // fetched column n+1, so cache all previous columns here
        const QSqlField info = d->rInf.field(i);
        switch (info.type()) {
        case QVariant::LongLong:
            d->fieldCache[i] = qGetBigIntData(d->hStmt, i);
        break;
        case QVariant::ULongLong:
            d->fieldCache[i] = qGetBigIntData(d->hStmt, i, false);
            break;
        case QVariant::Int:
            d->fieldCache[i] = qGetIntData(d->hStmt, i);
        break;
        case QVariant::UInt:
            d->fieldCache[i] = qGetIntData(d->hStmt, i, false);
            break;
        case QVariant::Date:
            DATE_STRUCT dbuf;
            r = SQLGetData(d->hStmt,
                            i + 1,
                            SQL_C_DATE,
                            (SQLPOINTER)&dbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[i] = QVariant(QDate(dbuf.year, dbuf.month, dbuf.day));
            else
                d->fieldCache[i] = QVariant(QVariant::Date);
        break;
        case QVariant::Time:
            TIME_STRUCT tbuf;
            r = SQLGetData(d->hStmt,
                            i + 1,
                            SQL_C_TIME,
                            (SQLPOINTER)&tbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[i] = QVariant(QTime(tbuf.hour, tbuf.minute, tbuf.second));
            else
                d->fieldCache[i] = QVariant(QVariant::Time);
        break;
        case QVariant::DateTime:
            TIMESTAMP_STRUCT dtbuf;
            r = SQLGetData(d->hStmt,
                            i + 1,
                            SQL_C_TIMESTAMP,
                            (SQLPOINTER)&dtbuf,
                            0,
                            &lengthIndicator);
            if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (lengthIndicator != SQL_NULL_DATA))
                d->fieldCache[i] = QVariant(QDateTime(QDate(dtbuf.year, dtbuf.month, dtbuf.day),
                       QTime(dtbuf.hour, dtbuf.minute, dtbuf.second, dtbuf.fraction / 1000000)));
            else
                d->fieldCache[i] = QVariant(QVariant::DateTime);
            break;
        case QVariant::ByteArray:
            d->fieldCache[i] = qGetBinaryData(d->hStmt, i);
            break;
        case QVariant::String:
            d->fieldCache[i] = qGetStringData(d->hStmt, i, info.length(), d->unicode);
            break;
        case QVariant::Double:
            switch(numericalPrecisionPolicy()) {
                case QSql::LowPrecisionInt32:
                    d->fieldCache[i] = qGetIntData(d->hStmt, i);
                    break;
                case QSql::LowPrecisionInt64:
                    d->fieldCache[i] = qGetBigIntData(d->hStmt, i);
                    break;
                case QSql::LowPrecisionDouble:
                    d->fieldCache[i] = qGetDoubleData(d->hStmt, i);
                    break;
                case QSql::HighPrecision:
                    d->fieldCache[i] = qGetStringData(d->hStmt, i, info.length(), false);
                    break;
            }
            break;
        default:
            d->fieldCache[i] = QVariant(qGetStringData(d->hStmt, i, info.length(), false));
            break;
        }
        d->fieldCacheIdx = field + 1;
    }
    return d->fieldCache[field];
}

bool QODBCResult::isNull(int field)
{
    if (field < 0 || field > d->fieldCache.size())
        return true;
    if (field <= d->fieldCacheIdx) {
        // since there is no good way to find out whether the value is NULL
        // without fetching the field we'll fetch it here.
        // (data() also sets the NULL flag)
        data(field);
    }
    return d->fieldCache.at(field).isNull();
}

int QODBCResult::size()
{
    return -1;
}

int QODBCResult::numRowsAffected()
{
    SQLLEN affectedRowCount = 0;
    SQLRETURN r = SQLRowCount(d->hStmt, &affectedRowCount);
    if (r == SQL_SUCCESS)
        return affectedRowCount;
    else
        qSqlWarning(QLatin1String("QODBCResult::numRowsAffected: Unable to count affected rows"), d);
    return -1;
}

bool QODBCResult::prepare(const QString& query)
{
    setActive(false);
    setAt(QSql::BeforeFirstRow);
    SQLRETURN r;

    d->rInf.clear();
    if (d->hStmt && d->isStmtHandleValid(driver())) {
        r = SQLFreeHandle(SQL_HANDLE_STMT, d->hStmt);
        if (r != SQL_SUCCESS) {
            qSqlWarning(QLatin1String("QODBCResult::prepare: Unable to close statement"), d);
            return false;
        }
    }
    r  = SQLAllocHandle(SQL_HANDLE_STMT,
                         d->dpDbc(),
                         &d->hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QLatin1String("QODBCResult::prepare: Unable to allocate statement handle"), d);
        return false;
    }

    d->updateStmtHandleState(driver());

    if (d->userForwardOnly) {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                            SQL_IS_UINTEGER);
    } else {
        r = SQLSetStmtAttr(d->hStmt,
                            SQL_ATTR_CURSOR_TYPE,
                            (SQLPOINTER)SQL_CURSOR_STATIC,
                            SQL_IS_UINTEGER);
    }
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
            "QODBCResult::reset: Unable to set 'SQL_CURSOR_STATIC' as statement attribute. "
            "Please check your ODBC driver configuration"), QSqlError::StatementError, d));
        return false;
    }

#ifdef UNICODE
    r = SQLPrepare(d->hStmt,
                    toSQLTCHAR(query).data(),
                    (SQLINTEGER) query.length());
#else
    QByteArray query8 = query.toUtf8();
    r = SQLPrepare(d->hStmt,
                    (SQLCHAR*) query8.data(),
                    (SQLINTEGER) query8.length());
#endif

    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                     "Unable to prepare statement"), QSqlError::StatementError, d));
        return false;
    }
    return true;
}

bool QODBCResult::exec()
{
    setActive(false);
    setAt(QSql::BeforeFirstRow);
    d->rInf.clear();
    d->fieldCache.clear();
    d->fieldCacheIdx = 0;

    if (!d->hStmt) {
        qSqlWarning(QLatin1String("QODBCResult::exec: No statement handle available"), d);
        return false;
    }

    if (isSelect())
        SQLCloseCursor(d->hStmt);

    QList<QByteArray> tmpStorage; // holds temporary buffers
    QVarLengthArray<SQLLEN, 32> indicators(boundValues().count());
    memset(indicators.data(), 0, indicators.size() * sizeof(SQLLEN));

    // bind parameters - only positional binding allowed
    QVector<QVariant>& values = boundValues();
    int i;
    SQLRETURN r;
    for (i = 0; i < values.count(); ++i) {
        if (bindValueType(i) & QSql::Out)
            values[i].detach();
        const QVariant &val = values.at(i);
        SQLLEN *ind = &indicators[i];
        if (val.isNull())
            *ind = SQL_NULL_DATA;
        switch (val.type()) {
            case QVariant::Date: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                QByteArray ba;
                ba.resize(sizeof(DATE_STRUCT));
                DATE_STRUCT *dt = (DATE_STRUCT *)ba.constData();
                QDate qdt = val.toDate();
                dt->year = qdt.year();
                dt->month = qdt.month();
                dt->day = qdt.day();
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_DATE,
                                      SQL_DATE,
                                      0,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QVariant::Time: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                QByteArray ba;
                ba.resize(sizeof(TIME_STRUCT));
                TIME_STRUCT *dt = (TIME_STRUCT *)ba.constData();
                QTime qdt = val.toTime();
                dt->hour = qdt.hour();
                dt->minute = qdt.minute();
                dt->second = qdt.second();
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_TIME,
                                      SQL_TIME,
                                      0,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QVariant::DateTime: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                QByteArray ba;
                ba.resize(sizeof(TIMESTAMP_STRUCT));
                TIMESTAMP_STRUCT * dt = (TIMESTAMP_STRUCT *)ba.constData();
                QDateTime qdt = val.toDateTime();
                dt->year = qdt.date().year();
                dt->month = qdt.date().month();
                dt->day = qdt.date().day();
                dt->hour = qdt.time().hour();
                dt->minute = qdt.time().minute();
                dt->second = qdt.time().second();
                dt->fraction = qdt.time().msec() * 1000000;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_TIMESTAMP,
                                      SQL_TIMESTAMP,
                                      19,
                                      0,
                                      (void *) dt,
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
                tmpStorage.append(ba);
                break; }
            case QVariant::Int: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_SLONG,
                                      SQL_INTEGER,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
            } break;
            case QVariant::UInt: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_ULONG,
                                      SQL_NUMERIC,
                                      15,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
            } break;
            case QVariant::Double: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_DOUBLE,
                                      SQL_DOUBLE,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
          } break;
          case QVariant::LongLong: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_SBIGINT,
                                      SQL_BIGINT,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
            } break;
            case QVariant::ULongLong: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_UBIGINT,
                                      SQL_BIGINT,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
            } break;
            case QVariant::ByteArray: {
                if (*ind != SQL_NULL_DATA) {
                    *ind = val.toByteArray().size();
                }
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_BINARY,
                                      SQL_LONGVARBINARY,
                                      val.toByteArray().size(),
                                      0,
                                      (void *) val.toByteArray().constData(),
                                      val.toByteArray().size(),
                                      ind);
            } break;
            case QVariant::Bool: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_BIT,
                                      SQL_BIT,
                                      0,
                                      0,
                                      (void *) val.constData(),
                                      0,
                                      *ind == SQL_NULL_DATA ? ind : NULL);
            } break;
            case QVariant::String: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                if (d->unicode) {
                    QString str = val.toString();
                    if (*ind != SQL_NULL_DATA)
                        *ind = str.length() * sizeof(SQLTCHAR);
                    int strSize = str.length() * sizeof(SQLTCHAR);

                    if (bindValueType(i) & QSql::Out) {
                        QVarLengthArray<SQLTCHAR> ba(toSQLTCHAR(str));
                        ba.reserve(str.capacity());
                        r = SQLBindParameter(d->hStmt,
                                            i + 1,
                                            qParamType[idx],
                                            SQL_C_TCHAR,
                                            strSize > 254 ? SQL_WLONGVARCHAR : SQL_WVARCHAR,
                                            0, // god knows... don't change this!
                                            0,
                                            (void *)ba.constData(),
                                            ba.size(),
                                            ind);
                        tmpStorage.append(QByteArray((const char *)ba.constData(), ba.size()*sizeof(SQLTCHAR)));
                        break;
                    }
                    QByteArray strba((const char *)toSQLTCHAR(str).constData(), str.size()*sizeof(SQLTCHAR));
                    r = SQLBindParameter(d->hStmt,
                                          i + 1,
                                          qParamType[idx],
                                          SQL_C_TCHAR,
                                          strSize > 254 ? SQL_WLONGVARCHAR : SQL_WVARCHAR,
                                          strSize,
                                          0,
                                          (SQLPOINTER)strba.constData(),
                                          strba.size(),
                                          ind);
                    tmpStorage.append(strba);
                    break;
                }
                else
                {
                    QByteArray str = val.toString().toUtf8();
                    if (*ind != SQL_NULL_DATA)
                        *ind = str.length();
                    int strSize = str.length();

                    r = SQLBindParameter(d->hStmt,
                                          i + 1,
                                          qParamType[idx],
                                          SQL_C_CHAR,
                                          strSize > 254 ? SQL_LONGVARCHAR : SQL_VARCHAR,
                                          strSize,
                                          0,
                                          (void *)str.constData(),
                                          strSize,
                                          ind);
                    tmpStorage.append(str);
                    break;
                }
            } break;
            // fall through
            default: {
                int idx=(int)(bindValueType(i)) & QSql::InOut;
                QByteArray ba = val.toByteArray();
                if (*ind != SQL_NULL_DATA)
                    *ind = ba.size();
                r = SQLBindParameter(d->hStmt,
                                      i + 1,
                                      qParamType[idx],
                                      SQL_C_BINARY,
                                      SQL_VARBINARY,
                                      ba.length() + 1,
                                      0,
                                      (void *) ba.constData(),
                                      ba.length() + 1,
                                      ind);
                tmpStorage.append(ba);

            } break;
        }
        if (r != SQL_SUCCESS) {
            qWarning() << "QODBCResult::exec: unable to bind variable:" << qODBCWarn(d);
            setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                         "Unable to bind variable"), QSqlError::StatementError, d));
            return false;
        }
    }
    r = SQLExecute(d->hStmt);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qWarning() << "QODBCResult::exec: Unable to execute statement:" << qODBCWarn(d);
        setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                     "Unable to execute statement"), QSqlError::StatementError, d));
        return false;
    }

    SQLULEN isScrollable = 0;
    SQLINTEGER bufferLength;
    r = SQLGetStmtAttr(d->hStmt, SQL_ATTR_CURSOR_SCROLLABLE, &isScrollable, SQL_IS_INTEGER, &bufferLength);
    if(r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
        QSqlResult::setForwardOnly(isScrollable==SQL_NONSCROLLABLE);

    SQLSMALLINT count;
    SQLNumResultCols(d->hStmt, &count);
    if (count) {
        setSelect(true);
        for (int i = 0; i < count; ++i) {
            d->rInf.append(qMakeFieldInfo(d, i));
        }
        d->fieldCache.resize(count);
    } else {
        setSelect(false);
    }
    setActive(true);


    //get out parameters
    if (!hasOutValues())
        return true;

    for (i = 0; i < values.count(); ++i) {
        switch (values.at(i).type()) {
            case QVariant::Date: {
                DATE_STRUCT ds = *((DATE_STRUCT *)tmpStorage.takeFirst().constData());
                values[i] = QVariant(QDate(ds.year, ds.month, ds.day));
                break; }
            case QVariant::Time: {
                TIME_STRUCT dt = *((TIME_STRUCT *)tmpStorage.takeFirst().constData());
                values[i] = QVariant(QTime(dt.hour, dt.minute, dt.second));
                break; }
            case QVariant::DateTime: {
                TIMESTAMP_STRUCT dt = *((TIMESTAMP_STRUCT*)
                                        tmpStorage.takeFirst().constData());
                values[i] = QVariant(QDateTime(QDate(dt.year, dt.month, dt.day),
                               QTime(dt.hour, dt.minute, dt.second, dt.fraction / 1000000)));
                break; }
            case QVariant::Bool:
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Double:
            case QVariant::ByteArray:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                //nothing to do
                break;
            case QVariant::String:
                if (d->unicode) {
                    if (bindValueType(i) & QSql::Out) {
                        QByteArray first = tmpStorage.takeFirst();
                        QVarLengthArray<SQLTCHAR> array;
                        array.append((SQLTCHAR *)first.constData(), first.size());
                        values[i] = fromSQLTCHAR(array, first.size()/sizeof(SQLTCHAR*));
                    }
                    break;
                }
                // fall through
            default: {
                if (bindValueType(i) & QSql::Out)
                    values[i] = tmpStorage.takeFirst();
                break; }
        }
        if (indicators[i] == SQL_NULL_DATA)
            values[i] = QVariant(values[i].type());
    }
    return true;
}

QSqlRecord QODBCResult::record() const
{
    if (!isActive() || !isSelect())
        return QSqlRecord();
    return d->rInf;
}

QVariant QODBCResult::handle() const
{
    return QVariant(qRegisterMetaType<SQLHANDLE>("SQLHANDLE"), &d->hStmt);
}

bool QODBCResult::nextResult()
{
    setActive(false);
    setAt(QSql::BeforeFirstRow);
    d->rInf.clear();
    d->fieldCache.clear();
    d->fieldCacheIdx = 0;
    setSelect(false);

    SQLRETURN r = SQLMoreResults(d->hStmt);
    if (r != SQL_SUCCESS) {
        if (r == SQL_SUCCESS_WITH_INFO) {
            int nativeCode = -1;
            QString message = qODBCWarn(d, &nativeCode);
            qWarning() << "QODBCResult::nextResult():" << message;
        } else {
            if (r != SQL_NO_DATA)
                setLastError(qMakeError(QCoreApplication::translate("QODBCResult",
                    "Unable to fetch last"), QSqlError::ConnectionError, d));
            return false;
        }
    }

    SQLSMALLINT count;
    SQLNumResultCols(d->hStmt, &count);
    if (count) {
        setSelect(true);
        for (int i = 0; i < count; ++i) {
            d->rInf.append(qMakeFieldInfo(d, i));
        }
        d->fieldCache.resize(count);
    } else {
        setSelect(false);
    }
    setActive(true);

    return true;
}

void QODBCResult::virtual_hook(int id, void *data)
{
    switch (id) {
/* (na)
    case QSqlResult::DetachFromResultSet:
        if (d->hStmt)
            SQLCloseCursor(d->hStmt);
        break;
    case QSqlResult::NextResult:
        Q_ASSERT(data);
        *static_cast<bool*>(data) = nextResult();
        break;
*/
    default:
        QSqlResult::virtual_hook(id, data);
    }
}

void QODBCResult::setForwardOnly(bool forward)
{
    d->userForwardOnly = forward;
    QSqlResult::setForwardOnly(forward);
}

////////////////////////////////////////


QODBCDriver::QODBCDriver(QObject *parent)
    : QSqlDriver(parent)
{
    init();
}

QODBCDriver::QODBCDriver(SQLHANDLE env, SQLHANDLE con, QObject * parent)
    : QSqlDriver(parent)
{
    init();
    d->hEnv = env;
    d->hDbc = con;
    if (env && con) {
        setOpen(true);
        setOpenError(false);
    }
}

void QODBCDriver::init()
{
    d = new QODBCDriverPrivate();
}

QODBCDriver::~QODBCDriver()
{
    cleanup();
    delete d;
}

bool QODBCDriver::hasFeature(DriverFeature f) const
{
    switch (f) {
    case Transactions: {
        if (!d->hDbc)
            return false;
        SQLUSMALLINT txn;
        SQLSMALLINT t;
        int r = SQLGetInfo(d->hDbc,
                        (SQLUSMALLINT)SQL_TXN_CAPABLE,
                        &txn,
                        sizeof(txn),
                        &t);
        if (r != SQL_SUCCESS || txn == SQL_TC_NONE)
            return false;
        else
            return true;
    }
    case Unicode:
        return d->unicode;
    case PreparedQueries:
    case PositionalPlaceholders:
    case FinishQuery:
    case LowPrecisionNumbers:
        return true;
    case QuerySize:
    case NamedPlaceholders:
    case LastInsertId:
    case BatchOperations:
    case SimpleLocking:
    case EventNotifications:
        return false;
    case MultipleResultSets:
        return d->hasMultiResultSets;
    case BLOB: {
        if(d->isMySqlServer)
            return true;
        else
            return false;
    }
    }
    return false;
}

bool QODBCDriver::open(const QString & db,
                        const QString & user,
                        const QString & password,
                        const QString &,
                        int,
                        const QString& connOpts)
{
    if (isOpen())
      close();
    SQLRETURN r;
    r = SQLAllocHandle(SQL_HANDLE_ENV,
                        SQL_NULL_HANDLE,
                        &d->hEnv);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSqlWarning(QLatin1String("QODBCDriver::open: Unable to allocate environment"), d);
        setOpenError(true);
        return false;
    }
    r = SQLSetEnvAttr(d->hEnv,
                       SQL_ATTR_ODBC_VERSION,
                       (SQLPOINTER)qGetODBCVersion(connOpts),
                       SQL_IS_UINTEGER);
    r = SQLAllocHandle(SQL_HANDLE_DBC,
                        d->hEnv,
                        &d->hDbc);
    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        qSqlWarning(QLatin1String("QODBCDriver::open: Unable to allocate connection"), d);
        setOpenError(true);
        return false;
    }

    if (!d->setConnectionOptions(connOpts))
        return false;

    // Create the connection string
    QString connQStr;
    // support the "DRIVER={SQL SERVER};SERVER=blah" syntax
    if (db.contains(QLatin1String(".dsn"), Qt::CaseInsensitive))
        connQStr = QLatin1String("FILEDSN=") + db;
    else if (db.contains(QLatin1String("DRIVER="), Qt::CaseInsensitive)
            || db.contains(QLatin1String("SERVER="), Qt::CaseInsensitive))
        connQStr = db;
    else
        connQStr = QLatin1String("DSN=") + db;

    if (!user.isEmpty())
        connQStr += QLatin1String(";UID=") + user;
    if (!password.isEmpty())
        connQStr += QLatin1String(";PWD=") + password;

    SQLSMALLINT cb;
    QVarLengthArray<SQLTCHAR> connOut(1024);
    memset(connOut.data(), 0, connOut.size() * sizeof(SQLTCHAR));
    r = SQLDriverConnect(d->hDbc,
                          NULL,
#ifdef UNICODE
                          toSQLTCHAR(connQStr).data(),
#else
                          (SQLCHAR*)connQStr.toUtf8().data(),
#endif
                          (SQLSMALLINT)connQStr.length(),
                          connOut.data(),
                          1024,
                          &cb,
                          /*SQL_DRIVER_NOPROMPT*/0);

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) {
        setLastError(qMakeError(tr("Unable to connect"), QSqlError::ConnectionError, d));
        setOpenError(true);
        return false;
    }

    if (!d->checkDriver()) {
        setLastError(qMakeError(tr("Unable to connect - Driver doesn't support all "
                     "functionality required"), QSqlError::ConnectionError, d));
        setOpenError(true);
        return false;
    }

    d->checkUnicode();
    d->checkSchemaUsage();
    d->checkSqlServer();
    d->checkHasSQLFetchScroll();
    d->checkHasMultiResults();
    setOpen(true);
    setOpenError(false);
    if(d->isMSSqlServer) {
        QSqlQuery i(createResult());
        i.exec(QLatin1String("SET QUOTED_IDENTIFIER ON"));
    }
    return true;
}

void QODBCDriver::close()
{
    cleanup();
    setOpen(false);
    setOpenError(false);
}

void QODBCDriver::cleanup()
{
    SQLRETURN r;
    if (!d)
        return;

    if(d->hDbc) {
        // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
        if (isOpen()) {
            r = SQLDisconnect(d->hDbc);
            if (r != SQL_SUCCESS)
                qSqlWarning(QLatin1String("QODBCDriver::disconnect: Unable to disconnect datasource"), d);
            else
                d->disconnectCount++;
        }

        r = SQLFreeHandle(SQL_HANDLE_DBC, d->hDbc);
        if (r != SQL_SUCCESS)
            qSqlWarning(QLatin1String("QODBCDriver::cleanup: Unable to free connection handle"), d);
        d->hDbc = 0;
    }

    if (d->hEnv) {
        r = SQLFreeHandle(SQL_HANDLE_ENV, d->hEnv);
        if (r != SQL_SUCCESS)
            qSqlWarning(QLatin1String("QODBCDriver::cleanup: Unable to free environment handle"), d);
        d->hEnv = 0;
    }
}

// checks whether the server can return char, varchar and longvarchar
// as two byte unicode characters
void QODBCDriverPrivate::checkUnicode()
{
    SQLRETURN   r;
    SQLUINTEGER fFunc;

    unicode = false;
    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_CHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WCHAR)) {
        unicode = true;
        return;
    }

    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_VARCHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WVARCHAR)) {
        unicode = true;
        return;
    }

    r = SQLGetInfo(hDbc,
                    SQL_CONVERT_LONGVARCHAR,
                    (SQLPOINTER)&fFunc,
                    sizeof(fFunc),
                    NULL);
    if ((r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) && (fFunc & SQL_CVT_WLONGVARCHAR)) {
        unicode = true;
        return;
    }
    SQLHANDLE hStmt;
    r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  hDbc,
                                  &hStmt);

    r = SQLExecDirect(hStmt, toSQLTCHAR(QLatin1String("select 'test'")).data(), SQL_NTS);
    if(r == SQL_SUCCESS) {
        r = SQLFetch(hStmt);
        if(r == SQL_SUCCESS) {
            QVarLengthArray<SQLWCHAR> buffer(10);
            r = SQLGetData(hStmt, 1, SQL_C_WCHAR, buffer.data(), buffer.size() * sizeof(SQLWCHAR), NULL);
            if(r == SQL_SUCCESS ) { // && fromSQLTCHAR(buffer) == QLatin1String("test")
                unicode = true;
            }
        }
    }
    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
}

bool QODBCDriverPrivate::checkDriver() const
{
#ifdef ODBC_CHECK_DRIVER
    static const SQLUSMALLINT reqFunc[] = {
                SQL_API_SQLDESCRIBECOL, SQL_API_SQLGETDATA, SQL_API_SQLCOLUMNS,
                SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETDIAGREC, SQL_API_SQLEXECDIRECT,
                SQL_API_SQLGETINFO, SQL_API_SQLTABLES, 0
    };

    // these functions are optional
    static const SQLUSMALLINT optFunc[] = {
        SQL_API_SQLNUMRESULTCOLS, SQL_API_SQLROWCOUNT, 0
    };

    SQLRETURN r;
    SQLUSMALLINT sup;

    int i;
    // check the required functions
    for (i = 0; reqFunc[i] != 0; ++i) {

        r = SQLGetFunctions(hDbc, reqFunc[i], &sup);

        if (r != SQL_SUCCESS) {
            qSqlWarning(QLatin1String("QODBCDriver::checkDriver: Cannot get list of supported functions"), this);
            return false;
        }
        if (sup == SQL_FALSE) {
            qWarning () << "QODBCDriver::open: Warning - Driver doesn't support all needed functionality (" << reqFunc[i] <<
                    ").\nPlease look at the Qt SQL Module Driver documentation for more information.";
            return false;
        }
    }

    // these functions are optional and just generate a warning
    for (i = 0; optFunc[i] != 0; ++i) {

        r = SQLGetFunctions(hDbc, optFunc[i], &sup);

        if (r != SQL_SUCCESS) {
            qSqlWarning(QLatin1String("QODBCDriver::checkDriver: Cannot get list of supported functions"), this);
            return false;
        }
        if (sup == SQL_FALSE) {
            qWarning() << "QODBCDriver::checkDriver: Warning - Driver doesn't support some non-critical functions (" << optFunc[i] << ')';
            return true;
        }
    }
#endif //ODBC_CHECK_DRIVER

    return true;
}

void QODBCDriverPrivate::checkSchemaUsage()
{
    SQLRETURN   r;
    SQLUINTEGER val;

    r = SQLGetInfo(hDbc,
                   SQL_SCHEMA_USAGE,
                   (SQLPOINTER) &val,
                   sizeof(val),
                   NULL);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
        useSchema = (val != 0);
}

void QODBCDriverPrivate::checkSqlServer()
{
    SQLRETURN   r;
    QVarLengthArray<SQLTCHAR> serverString(200);
    SQLSMALLINT t;
    memset(serverString.data(), 0, serverString.size() * sizeof(SQLTCHAR));

    r = SQLGetInfo(hDbc,
                   SQL_DBMS_NAME,
                   serverString.data(),
                   serverString.size() * sizeof(SQLTCHAR),
                   &t);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
        QString serverType;
#ifdef UNICODE
        serverType = fromSQLTCHAR(serverString, t/sizeof(SQLTCHAR));
#else
        serverType = QString::fromUtf8((const char *)serverString.constData(), t);
#endif
        isMySqlServer = serverType.contains(QLatin1String("mysql"), Qt::CaseInsensitive);
        isMSSqlServer = serverType.contains(QLatin1String("Microsoft SQL Server"), Qt::CaseInsensitive);
    }
    r = SQLGetInfo(hDbc,
                   SQL_DRIVER_NAME,
                   serverString.data(),
                   serverString.size() * sizeof(SQLTCHAR),
                   &t);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO) {
        QString serverType;
#ifdef UNICODE
        serverType = fromSQLTCHAR(serverString, t/sizeof(SQLTCHAR));
#else
        serverType = QString::fromUtf8((const char *)serverString.constData(), t);
#endif
        isFreeTDSDriver = serverType.contains(QLatin1String("tdsodbc"), Qt::CaseInsensitive);
        unicode = unicode && !isFreeTDSDriver;
    }
}

void QODBCDriverPrivate::checkHasSQLFetchScroll()
{
    SQLUSMALLINT sup;
    SQLRETURN r = SQLGetFunctions(hDbc, SQL_API_SQLFETCHSCROLL, &sup);
    if ((r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO) || sup != SQL_TRUE) {
        hasSQLFetchScroll = false;
        qWarning() << "QODBCDriver::checkHasSQLFetchScroll: Warning - Driver doesn't support scrollable result sets, use forward only mode for queries";
    }
}

void QODBCDriverPrivate::checkHasMultiResults()
{
    QVarLengthArray<SQLTCHAR> driverResponse(2);
    SQLSMALLINT length;
    SQLRETURN r = SQLGetInfo(hDbc,
                             SQL_MULT_RESULT_SETS,
                             driverResponse.data(),
                             driverResponse.size() * sizeof(SQLTCHAR),
                             &length);
    if (r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO)
#ifdef UNICODE
        hasMultiResultSets = fromSQLTCHAR(driverResponse, length/sizeof(SQLTCHAR)).startsWith(QLatin1Char('Y'));
#else
        hasMultiResultSets = QString::fromUtf8((const char *)driverResponse.constData(), length).startsWith(QLatin1Char('Y'));
#endif
}

QSqlResult *QODBCDriver::createResult() const
{
    return new QODBCResult(this, d);
}

bool QODBCDriver::beginTransaction()
{
    if (!isOpen()) {
        qWarning() << "QODBCDriver::beginTransaction: Database not open";
        return false;
    }
    SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to disable autocommit"),
                     QSqlError::TransactionError, d));
        return false;
    }
    return true;
}

bool QODBCDriver::commitTransaction()
{
    if (!isOpen()) {
        qWarning() << "QODBCDriver::commitTransaction: Database not open";
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_COMMIT);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to commit transaction"),
                     QSqlError::TransactionError, d));
        return false;
    }
    return endTrans();
}

bool QODBCDriver::rollbackTransaction()
{
    if (!isOpen()) {
        qWarning() << "QODBCDriver::rollbackTransaction: Database not open";
        return false;
    }
    SQLRETURN r = SQLEndTran(SQL_HANDLE_DBC,
                              d->hDbc,
                              SQL_ROLLBACK);
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to rollback transaction"),
                     QSqlError::TransactionError, d));
        return false;
    }
    return endTrans();
}

bool QODBCDriver::endTrans()
{
    SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
    SQLRETURN r  = SQLSetConnectAttr(d->hDbc,
                                      SQL_ATTR_AUTOCOMMIT,
                                      (SQLPOINTER)ac,
                                      sizeof(ac));
    if (r != SQL_SUCCESS) {
        setLastError(qMakeError(tr("Unable to enable autocommit"), QSqlError::TransactionError, d));
        return false;
    }
    return true;
}

QStringList QODBCDriver::tables(QSql::TableType type) const
{
    QStringList tl;
    if (!isOpen())
        return tl;
    SQLHANDLE hStmt;

    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QLatin1String("QODBCDriver::tables: Unable to allocate handle"), d);
        return tl;
    }
    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    QStringList tableType;
    if (type & QSql::Tables)
        tableType += QLatin1String("TABLE");
    if (type & QSql::Views)
        tableType += QLatin1String("VIEW");
    if (type & QSql::SystemTables)
        tableType += QLatin1String("SYSTEM TABLE");
    if (tableType.isEmpty())
        return tl;

    QString joinedTableTypeString = tableType.join(QLatin1String(","));

    r = SQLTables(hStmt,
                   NULL,
                   0,
                   NULL,
                   0,
                   NULL,
                   0,
#ifdef UNICODE
                   toSQLTCHAR(joinedTableTypeString).data(),
#else
                   (SQLCHAR*)joinedTableTypeString.toUtf8().data(),
#endif
                   joinedTableTypeString.length() /* characters, not bytes */);

    if (r != SQL_SUCCESS)
        qSqlWarning(QLatin1String("QODBCDriver::tables Unable to execute table list"), d);

    if (d->hasSQLFetchScroll)
        r = SQLFetchScroll(hStmt,
                           SQL_FETCH_NEXT,
                           0);
    else
        r = SQLFetch(hStmt);

    if (r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO && r != SQL_NO_DATA) {
        qWarning() << "QODBCDriver::tables failed to retrieve table/view list: (" << r << "," << qWarnODBCHandle(SQL_HANDLE_STMT, hStmt) << ")";
        return QStringList();
    }

    while (r == SQL_SUCCESS) {
        QString fieldVal = qGetStringData(hStmt, 2, -1, false);
        tl.append(fieldVal);

        if (d->hasSQLFetchScroll)
            r = SQLFetchScroll(hStmt,
                               SQL_FETCH_NEXT,
                               0);
        else
            r = SQLFetch(hStmt);
    }

    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning(QLatin1String("QODBCDriver: Unable to free statement handle") + QString::number(r), d);
    return tl;
}

QSqlIndex QODBCDriver::primaryIndex(const QString& tablename) const
{
    QSqlIndex index(tablename);
    if (!isOpen())
        return index;
    bool usingSpecialColumns = false;
    QSqlRecord rec = record(tablename);

    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QLatin1String("QODBCDriver::primaryIndex: Unable to list primary key"), d);
        return index;
    }
    QString catalog, schema, table;
    d->splitTableQualifier(tablename, catalog, schema, table);

    if (isIdentifierEscaped(catalog, QSqlDriver::TableName))
        catalog = stripDelimiters(catalog, QSqlDriver::TableName);
    else
        catalog = d->adjustCase(catalog);

    if (isIdentifierEscaped(schema, QSqlDriver::TableName))
        schema = stripDelimiters(schema, QSqlDriver::TableName);
    else
        schema = d->adjustCase(schema);

    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);
    else
        table = d->adjustCase(table);

    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    r = SQLPrimaryKeys(hStmt,
#ifdef UNICODE
                        catalog.length() == 0 ? NULL : toSQLTCHAR(catalog).data(),
#else
                        catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.toUtf8().data(),
#endif
                        catalog.length(),
#ifdef UNICODE
                        schema.length() == 0 ? NULL : toSQLTCHAR(schema).data(),
#else
                        schema.length() == 0 ? NULL : (SQLCHAR*)schema.toUtf8().data(),
#endif
                        schema.length(),
#ifdef UNICODE
                        toSQLTCHAR(table).data(),
#else
                        (SQLCHAR*)table.toUtf8().data(),
#endif
                        table.length() /* in characters, not in bytes */);

    // if the SQLPrimaryKeys() call does not succeed (e.g the driver
    // does not support it) - try an alternative method to get hold of
    // the primary index (e.g MS Access and FoxPro)
    if (r != SQL_SUCCESS) {
            r = SQLSpecialColumns(hStmt,
                        SQL_BEST_ROWID,
#ifdef UNICODE
                        catalog.length() == 0 ? NULL : toSQLTCHAR(catalog).data(),
#else
                        catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.toUtf8().data(),
#endif
                        catalog.length(),
#ifdef UNICODE
                        schema.length() == 0 ? NULL : toSQLTCHAR(schema).data(),
#else
                        schema.length() == 0 ? NULL : (SQLCHAR*)schema.toUtf8().data(),
#endif
                        schema.length(),
#ifdef UNICODE
                        toSQLTCHAR(table).data(),
#else
                        (SQLCHAR*)table.toUtf8().data(),
#endif
                        table.length(),
                        SQL_SCOPE_CURROW,
                        SQL_NULLABLE);

            if (r != SQL_SUCCESS) {
                qSqlWarning(QLatin1String("QODBCDriver::primaryIndex: Unable to execute primary key list"), d);
            } else {
                usingSpecialColumns = true;
            }
    }

    if (d->hasSQLFetchScroll)
        r = SQLFetchScroll(hStmt,
                           SQL_FETCH_NEXT,
                           0);
    else
        r = SQLFetch(hStmt);

    int fakeId = 0;
    QString cName, idxName;
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while (r == SQL_SUCCESS) {
        if (usingSpecialColumns) {
            cName = qGetStringData(hStmt, 1, -1, d->unicode); // column name
            idxName = QString::number(fakeId++); // invent a fake index name
        } else {
            cName = qGetStringData(hStmt, 3, -1, d->unicode); // column name
            idxName = qGetStringData(hStmt, 5, -1, d->unicode); // pk index name
        }
        index.append(rec.field(cName));
        index.setName(idxName);

        if (d->hasSQLFetchScroll)
            r = SQLFetchScroll(hStmt,
                               SQL_FETCH_NEXT,
                               0);
        else
            r = SQLFetch(hStmt);

    }
    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning(QLatin1String("QODBCDriver: Unable to free statement handle") + QString::number(r), d);
    return index;
}

QSqlRecord QODBCDriver::record(const QString& tablename) const
{
    QSqlRecord fil;
    if (!isOpen())
        return fil;

    SQLHANDLE hStmt;
    QString catalog, schema, table;
    d->splitTableQualifier(tablename, catalog, schema, table);

    if (isIdentifierEscaped(catalog, QSqlDriver::TableName))
        catalog = stripDelimiters(catalog, QSqlDriver::TableName);
    else
        catalog = d->adjustCase(catalog);

    if (isIdentifierEscaped(schema, QSqlDriver::TableName))
        schema = stripDelimiters(schema, QSqlDriver::TableName);
    else
        schema = d->adjustCase(schema);

    if (isIdentifierEscaped(table, QSqlDriver::TableName))
        table = stripDelimiters(table, QSqlDriver::TableName);
    else
        table = d->adjustCase(table);

    SQLRETURN r = SQLAllocHandle(SQL_HANDLE_STMT,
                                  d->hDbc,
                                  &hStmt);
    if (r != SQL_SUCCESS) {
        qSqlWarning(QLatin1String("QODBCDriver::record: Unable to allocate handle"), d);
        return fil;
    }
    r = SQLSetStmtAttr(hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER);
    r =  SQLColumns(hStmt,
#ifdef UNICODE
                     catalog.length() == 0 ? NULL : toSQLTCHAR(catalog).data(),
#else
                     catalog.length() == 0 ? NULL : (SQLCHAR*)catalog.toUtf8().data(),
#endif
                     catalog.length(),
#ifdef UNICODE
                     schema.length() == 0 ? NULL : toSQLTCHAR(schema).data(),
#else
                     schema.length() == 0 ? NULL : (SQLCHAR*)schema.toUtf8().data(),
#endif
                     schema.length(),
#ifdef UNICODE
                     toSQLTCHAR(table).data(),
#else
                     (SQLCHAR*)table.toUtf8().data(),
#endif
                     table.length(),
                     NULL,
                     0);
    if (r != SQL_SUCCESS)
        qSqlWarning(QLatin1String("QODBCDriver::record: Unable to execute column list"), d);

    if (d->hasSQLFetchScroll)
        r = SQLFetchScroll(hStmt,
                           SQL_FETCH_NEXT,
                           0);
    else
        r = SQLFetch(hStmt);

    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while (r == SQL_SUCCESS) {

        fil.append(qMakeFieldInfo(hStmt, d));

        if (d->hasSQLFetchScroll)
            r = SQLFetchScroll(hStmt,
                               SQL_FETCH_NEXT,
                               0);
        else
            r = SQLFetch(hStmt);
    }

    r = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
    if (r!= SQL_SUCCESS)
        qSqlWarning(QLatin1String("QODBCDriver: Unable to free statement handle ") + QString::number(r), d);

    return fil;
}

QString QODBCDriver::formatValue(const QSqlField &field,
                                 bool trimStrings) const
{
    QString r;
    if (field.isNull()) {
        r = QLatin1String("NULL");
    } else if (field.type() == QVariant::DateTime) {
        // Use an escape sequence for the datetime fields
        if (field.value().toDateTime().isValid()){
            QDate dt = field.value().toDateTime().date();
            QTime tm = field.value().toDateTime().time();
            // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
            r = QLatin1String("{ ts '") +
                QString::number(dt.year()) + QLatin1Char('-') +
                QString::number(dt.month()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char('-') +
                QString::number(dt.day()).rightJustified(2, QLatin1Char('0'), true) +
                QLatin1Char(' ') +
                tm.toString() +
                QLatin1String("' }");
        } else
            r = QLatin1String("NULL");
    } else if (field.type() == QVariant::ByteArray) {
        QByteArray ba = field.value().toByteArray();
        QString res;
        static const char hexchars[] = "0123456789abcdef";
        for (int i = 0; i < ba.size(); ++i) {
            uchar s = (uchar) ba[i];
            res += QLatin1Char(hexchars[s >> 4]);
            res += QLatin1Char(hexchars[s & 0x0f]);
        }
        r = QLatin1String("0x") + res;
    } else {
        r = QSqlDriver::formatValue(field, trimStrings);
    }
    return r;
}

QVariant QODBCDriver::handle() const
{
    return QVariant(qRegisterMetaType<SQLHANDLE>("SQLHANDLE"), &d->hDbc);
}

QString QODBCDriver::escapeIdentifier(const QString &identifier, IdentifierType) const
{
    QChar quote = d->quoteChar();
    QString res = identifier;
    if(!identifier.isEmpty() && !identifier.startsWith(quote) && !identifier.endsWith(quote) ) {
        res.replace(quote, QString(quote)+QString(quote));
        res.prepend(quote).append(quote);
        res.replace(QLatin1Char('.'), QString(quote)+QLatin1Char('.')+QString(quote));
    }
    return res;
}

bool QODBCDriver::isIdentifierEscapedImplementation(const QString &identifier, IdentifierType) const
{
    QChar quote = d->quoteChar();
    return identifier.size() > 2
        && identifier.startsWith(quote) //left delimited
        && identifier.endsWith(quote); //right delimited
}


#endif

SqlDatabase::SqlDatabase(TreeNode* cf) : xdb(NULL), xquery(NULL), xcf(cf), xstat(0), xflag(0) {
    /*
    QLibrary odbclib("odbc32.dll");
    if( !odbclib.isLoaded() ) {
        odbclib.load();
        qDebug()<<"odbc lib loaded"<<odbclib.isLoaded();
        GetVar("@e")->add("\nodbc library loaded ").add(odbclib.isLoaded()?"TRUE":"FALSE");
    }
    QLibrary sqlib("qsqlodbc4.dll");
    if( !sqlib.isLoaded() ) {
        sqlib.load();
        qDebug()<<"sql library loaded"<<sqlib.isLoaded();
        GetVar("@e")->add("\nsql library loaded ").add(sqlib.isLoaded()?"TRUE":"FALSE");
    }
    QLibrary pqlib("qsqlpsql4.dll");
    if( !pqlib.isLoaded() ) {
        pqlib.load();
        qDebug()<<"postgres library loaded"<<pqlib.isLoaded();
        GetVar("@e")->add("\npq library loaded ").add(pqlib.isLoaded()?"TRUE":"FALSE");
    }

    GetVar("@e")->add("\n").add(Q2A(QSqlDatabase::drivers().join(",")));
    GetVar("@e")->add("\n").add(getCf()->get("@libpath"));
    QLibrary mysqlLib("qsqlmysql4.dll");
    if( !mysqlLib.isLoaded() ) {
        mysqlLib.load();
        qDebug()<<"mysql library loaded"<<mysqlLib.isLoaded();
        GetVar("@error")->add("\nmysql library loaded ").add(mysqlLib.isLoaded()?"TRUE":"FALSE");
    }
    */
    LPCC psql=getCf()->get("@sqlDrivers");
    if( slen(psql)==0 ) {
#ifdef PSQL_USE
        QSqlDatabase::registerSqlDriver("QPSQL9", new QSqlDriverCreator<QPSQLDriver>);
        QSqlDatabase::registerSqlDriver("QODBC2", new QSqlDriverCreator<QODBCDriver>);
#endif
#ifdef MYSQL_USE
        QSqlDatabase::registerSqlDriver("QMYSQL", new QSqlDriverCreator<QMYSQLDriver>);
#endif
        QString drivers=QSqlDatabase::drivers().join(", ");
        getCf()->set("@sqlDrivers", Q2A(drivers) );
         // QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL9");
        qDebug()<<"QSqlDatabase::drivers() ===> "<<drivers;
    }    
}

SqlDatabase::~SqlDatabase() {
     closeStatement();
     SAFE_DELETE(xquery);
}


bool SqlDatabase::addDsn(TreeNode* tn) {
    DbUtil* db=getConfDb();
    if( isDsn(tn->get("dsn")) ) {
        db->query("update db_info set  driver=?, server=?, dbnm=?, uid=?, pwd=?, port=? where dsn=?").
            bindStr(tn->get("driver")).bindStr(tn->get("server")).bindStr(tn->get("dbnm")).bindStr(tn->get("uid")).bindStr(tn->get("pwd")).bindInt(tn->getInt("port",1433)).bindStr(tn->get("dsn")).bind();
    } else {
        db->query("insert into db_info(dsn, driver, server, dbnm, uid, pwd, port ) values (?,?,?,?,?,?,?)").
            bindStr(tn->get("dsn")).bindStr(tn->get("driver")).bindStr(tn->get("server")).bindStr(tn->get("dbnm")).bindStr(tn->get("uid")).bindStr(tn->get("pwd")).bindInt(tn->getInt("port",1433)).bind();
    }
    return true;
}

void SqlDatabase::closeStatement() {
    xstat=0;
    GetVar("@error")->reuse();
    if( xquery ) {
        xquery->clear();
    }
}

bool SqlDatabase::isOpen() {
    if( xdb==NULL ) {
        return false;
    }
    if( xdb->isOpen() ) {
        if( xquery==NULL ) {
            xquery=new QSqlQuery(*xdb);
        }
        if( xstat==0 ) xstat=2;
        return true;
    }
    return false;
}

bool SqlDatabase::isVaild() {
    return xdb? xdb->isValid():false;
}

bool SqlDatabase::exec(LPCC sql) {
    xstat=4;
    if( !isOpen() ) {
        return false;
    }
    if( xquery->exec(KR(sql)) ) {
        return true;
    } else {
        qDebug("#0#[SqlDatabase] exec(query) error");
    }
    lastError();
    return false;
}

bool SqlDatabase::isDsn(LPCC dsn) {
    SqlDatabase* db=getModelDatabase("config");
    /*
    if( db->tableCount("db_info")==0 ) {
        db->exec("create table db_info( dsn text, driver text, server text, dbnm text, uid text, pwd text, port text)");
        return false;
    }
    */
    if( db && db->prepare("select count(1) from db_info where dsn=?") &&
        db->bindStr(1,dsn).exec() ) {
        StrVar* sv=db->fetchValue();
        int num=toInteger(sv);
        if( num==0 ) {
            return false;
        }
    }
    return true;
}

StrVar* SqlDatabase::lastError() {
    //#baropage
    if( xquery ) {
        QString err=xquery->lastError().text();
        if( err.isNull() || err.isEmpty() ) return NULL;
        StrVar* sv=GetVar("@error");
        sv->set( Q2A(err) );
        qDebug("#0#database error: %s\n", sv->get());
        if( sv->find("server has gone away")!=-1 ) {
            if( xdb ) {
                this->closeStatement();
                xdb->close();
            }
        }
        return sv;
    }
    return NULL;
}

bool SqlDatabase::open(LPCC inode) {
    qDebug("SqlDataBase open start INODE=%s %d %s", inode, xstat, xcf?"ok":"error" );
    if( isOpen() ) {
        xstat=2;
        return true;
    }
    if( slen(inode) && xcf ) {
        if( isNodeFlag(FLAG_CALL) ) {
            qDebug("#0#db status connecting ... error INODE=%s\n", inode );
            return false;
        }
        LPCC dbtype="QPSQL"; // get("dbtype");
        LPCC dbms = xcf->get("dbms");
        LPCC dsn= xcf->get("dsn");
        if( slen(dbms)==0 ) {
            dbms=xcf->get("driver");
        }
        if( ccmp(dbms,"postgres") ) {
            dbtype="QPSQL";
        } else if( ccmp(dbms,"mysql") ) {
            dbtype="QMYSQL";
        } else if( ccmp(dbms,"mssql") ) {
            dbtype="QODBC3";
        } else {
            qDebug("#9#dbms(%s) not define (config:%s)", dbms, xcf->log() );
            return false;
        }

        QString timeout="";
        if( ccmp(dbtype,"QPSQL") ) {
            timeout="connect_timeout";
        } else if( ccmp(dbtype,"QOCI") ) {
            timeout="Connection Timeout";
        } else if( ccmp(dbtype,"QTDS") ) {
            timeout="TimeOut";
        } else {  //Q ODBC (open database connector (vendria a ser el general).
            timeout="SQL_ATTR_CONNECTION_TIMEOUT";
        }
        /*
        } else if( ccmp(dbtype,"QMYSQL") ) {
            timeout = "MYSQL_OPT_CONNECT_TIMEOUT";
        */

        set("dbtype",dbtype);
        if( xdb==NULL && slen(dbtype) ) {
            if( QSqlDatabase::contains(KR(inode)) ) {
                xdb = new QSqlDatabase(QSqlDatabase::database(KR(inode)));
            } else {
                qDebug("... database %s:%s ....\n", dbtype, inode);
                if( ccmp(dbtype,"QPSQL") ) {
                    xdb = new QSqlDatabase(QSqlDatabase::addDatabase("QPSQL9",KR(inode)));
                } else {
                    xdb = new QSqlDatabase(QSqlDatabase::addDatabase(KR(dbtype),KR(inode)));
                }
            }
        }
        if( xdb==NULL ) {
            qDebug("#0#db create error INODE=%s\n", inode );
            return false;
        }
        if(timeout!="") {
            xdb->setConnectOptions(timeout+"="+QString::number(3));
        }
        setNodeFlag(FLAG_CALL);
        //SQL Server Native Client 10.0 , 202.68.237.137, universiade

        LPCC driver=NULL, dbnm=NULL, server=NULL, uid=NULL, pw=NULL, port=NULL;
        if( xcf ) {
            uid=xcf->get("uid"), pw=xcf->get("pwd"), port=xcf->get("port");
            qDebug("#0# db info: DB_TYPE=%s, DSN=%s\n", dbtype, dsn);
        }
        bool ok=false;
        if( slen(uid) && slen(pw) ) {
            ok=true;
        } else if( slen(dsn) ) {
            DbUtil* dbCf = getConfDb();
            if( dbCf && dbCf->prepare("select server, driver, dbnm, uid, pwd, port from db_info where dsn=?") &&
                dbCf->bindStr(dsn).bind() && dbCf->fetch() ) {
                StrVar* pwd=dbCf->GetVar("pwd");
                ZipVar z(pwd);
                z.Decode();
                server=dbCf->get("server"), dbnm=dbCf->get("dbnm"), uid=dbCf->get("uid");
                driver=dbCf->get("driver");
                pw=pwd->get();
                port=dbCf->get("port");
                ok=true;
            }
        }

        if( ok ) {
            int portNum=0;
            if( slen(server)==0 ) {
                server="127.0.0.1";
            }
            if( isnum(port) ) {
                portNum=atoi(port);
            }
            if( ccmp(dbtype, "QPSQL9") || ccmp(dbtype, "QPSQL") || ccmp(dbtype, "QMYSQL") ) {
                if( portNum==0 ) {
                    portNum=5432;
                }                
                qDebug("#0#DB_TYPE=%s, DRIVER=%s\nSERVER=%s, NAME=%s\nuid:%s, pwd:%s, port:%d (ok==%d)\n",
                       dbtype, driver, server, dbnm, uid, pw, portNum, ok);
                xdb->setHostName(server); // db->get("server")
                xdb->setPort(portNum);
                xdb->setDatabaseName(KR(dbnm));
                xdb->setUserName(KR(uid));
                xdb->setPassword(KR(pw));
            } else {
                xdb->setConnectOptions(); // "SQL_ATTR_ACCESS_MODE=SQL_MODE_READ_ONLY;SQL_ATTR_TRACE=SQL_OPT_TRACE_ON");
                if( slen(driver) ) {
                    if( portNum>0 ) {
                        QString info=KR("DRIVER={%1};SERVER=%2,%3;DATABASE=%4;Uid=%5;Pwd=%6;").
                            arg(driver).arg(server).arg(port).arg(dbnm).arg(uid).arg(pw);
                        // qDebug()<<"DB Connection String: "<<info<<"\n\n";
                        xdb->setDatabaseName(info);

                    } else {
                        QString info=KR("DRIVER={%1};SERVER=%2;DATABASE=%3;Uid=%4;Pwd=%5;Trusted_Connection=Yes;").
                            arg(driver).arg(server).arg(dbnm).arg(uid).arg(pw);
                        xdb->setDatabaseName(info);
                        // qDebug()<<"DB Connection Server: "<<server<<"\n\n";
                        // qDebug()<<"DB Connection String: "<<info<<"\n\n";
                    }
                } else {
                    QString info=KR("DRIVER={SQL Server};SERVER=%1;DATABASE=%2;").arg(server).arg(dbnm);
                    xdb->setDatabaseName(info);
                    xdb->setPort(portNum);
                    xdb->setUserName(KR(uid));
                    xdb->setPassword(KR(pw));
                }
            }
        } else {
            qDebug("#0# data conect info not define (DB_TYPE=%s, DSN=%s)\n", dbtype, dsn);
            clearNodeFlag(FLAG_CALL);
            return false;
        }

        if( xdb->open() ) {
            xstat=2;
            qDebug("#0#database open ok db type:%s\n", dbtype);
            clearNodeFlag(FLAG_CALL);
            return true;
        } else {
            StrVar* sv=GetVar("@error");
            sv->set( Q2A(xdb->lastError().text()) );
            qDebug("#9#database open fail dbtype:%s(%s), error:%s", inode, dbtype, sv->get());
        }
    } else {
        qDebug("#9#database open not valid inode:%s", inode);
    }
    clearNodeFlag(FLAG_CALL);
    return false;
}


bool SqlDatabase::prepare(LPCC sql) {
    if( !isOpen() )
        return false;
    GetVar("@error")->reuse();
    xstat=3;
    // set("@query",sql);
    // closeStatement();
    xquery->setForwardOnly(true);
    if( xquery->prepare(KR(sql)) ) {
        return true;
    } else {
        qDebug("#0#[SqlDatabase] prepare error");
        GetVar("@error")->set("db prepare error");
    }
    lastError();
    return false;
}

bool SqlDatabase::exec() {
    if( !isOpen() ) return false;
    if( xquery->exec() ) {
        return true;
    } else {
        qDebug("#0#[SqlDatabase] exec error");
    }
    lastError();
    return false;
}


int SqlDatabase::fieldIndex(LPCC field) {
    if( !isOpen() ) return false;
    return xquery->record().indexOf(KR(field));
}

StrVar* SqlDatabase::fieldName(int idx) {
    xvar.reuse();
    if( isOpen() && idx>=0 && idx<xquery->record().count() ) {
        QString field=xquery->record().fieldName(idx);
        xvar.add(Q2A(field));
    }
    return &xvar;
}
int SqlDatabase::affectRows() {
    // xstat maybe 3
    if( !isOpen() ) return 0;
    return xquery->numRowsAffected();
}
StrVar* SqlDatabase::lastInsertId() {
    xvar.reuse();
    if( !isOpen() ) return NULL;
    QVariant var=xquery->lastInsertId();
    xvar.add(Q2A(var.toString()));
    return &xvar;
}

int SqlDatabase::fieldCount() {
    if( !isOpen() ) return 0;
    return xquery->record().count();
}

SqlDatabase& SqlDatabase::bind(int idx, StrVar* sv) {
    if( !isOpen() || xstat!=3 ) return *this;
    if( sv==NULL )
        xquery->bindValue(idx, QVariant());
    else if( sv->charAt(0)=='\b' ) {
        switch(sv->charAt(1)) {
            case '0':
            case '2':
            case '3': {
                xquery->bindValue(idx, QVariant(toInteger(sv)));
            } break;
            case '1': {
                xquery->bindValue(idx, QVariant(toUL64(sv)));
            } break;
            case '4': {
                xquery->bindValue(idx, QVariant(toDouble(sv)));
            } break;
            default: {
                if( SVCHK('i',7) ) {
                    xquery->bindValue(idx, QVariant(QByteArray(sv->get(FUNC_HEADER_POS+4),sv->getInt(FUNC_HEADER_POS))) );
                } else if( SVCHK('9',0)) {
                    xquery->bindValue(idx, QVariant());
                } else {
                    if( SVCHK('s',0)) {
                        int sp=0,ep=0;
                        StrVar* val=getStrVarPointer(sv,&sp,&ep);
                        xquery->bindValue(idx, QVariant(QByteArray(val->get(sp),ep-sp)));
                    } else {
                        LPCC val=toString(sv);
                        xquery->bindValue(idx, QVariant(KR(val)));
                    }
                }
            } break;
        }
    } else {
        LPCC val=toString(sv);
        xquery->bindValue(idx, QVariant(KR(val)));
        // xquery->bindValue( idx, QVariant(QByteArray(sv->get(),sv->length())) );
    }
    return *this;
}

SqlDatabase& SqlDatabase::bindStr(int idx, LPCC val) {
    if( !isOpen() || xstat!=3 ) return *this;
    xquery->bindValue(idx,QVariant(KR(val)));
    return *this;
}

SqlDatabase& SqlDatabase::bindInt(int idx, int val) {
    if( !isOpen() || xstat!=3 ) return *this;
    xquery->bindValue(idx,QVariant(val));
    return *this;
}
SqlDatabase& SqlDatabase::bindLong(int idx, UL64 val) {
    if( !isOpen() || xstat!=3 ) return *this;
    xquery->bindValue(idx,QVariant(val));
    return *this;
}
SqlDatabase& SqlDatabase::bindBuffer(int idx, StrVar* val) {
    if( !isOpen() || val==NULL || xstat!=3 ) return *this;

    char ch=val->charAt(0);
    if( ch=='\b' ) {
        StrVar* sv=val;
        if( SVCHK('s',0) ) {
            int sp=0,ep=0;
            val=getStrVarPointer(sv,&sp,&ep);
            xquery->bindValue(idx,QVariant(QByteArray(val->get(sp),ep-sp)));
        } else {
            LPCC buf=toString(val);
            xquery->bindValue(idx,QVariant(KR(buf)) );
        }
    } else {
        xquery->bindValue(idx,QVariant(QByteArray(val->get(),val->length())));
    }
    return *this;
}

bool SqlDatabase::fetchNode(WBox* node) {
    if( !isOpen() || !xquery->next() ) return false;
    if( node==NULL ) {
        node=(WBox*)this;
    }
    for( int n=0,sz=xquery->record().count();n<sz;n++ ) {
        node->GetVar(Q2A(xquery->record().fieldName(n)))->set(Q2A(xquery->value(n).toString()));
    }
    return true;
}

TreeNode* SqlDatabase::fetchRoot(TreeNode* root) {
    if( !isOpen() || !xquery->next() )
        return NULL;
    TreeNode* node=root->addNode();
    for( int n=0,sz=xquery->record().count();n<sz;n++ ) {
        node->GetVar(Q2A(xquery->record().fieldName(n)))->set(Q2A(xquery->value(n).toString()));
    }
    return node;
}

StrVar* SqlDatabase::fetchValue(int idx) {
    xvar.reuse();
    if( isOpen() && xquery->next() ) {
        QByteArray ba = xquery->value(idx).toByteArray();
        xvar.set(ba.constData(),ba.size());
        // qDebug("fetch value size==%d", ba.size() );
    } else {
        qDebug("#0#[SqlDatabase] fetchValue error(%idx)", idx);
    }
    return &xvar;
}

TreeNode*	SqlDatabase::fetchTreeNode(TreeNode* root) {
    if( !isOpen() || root==NULL ) return root;
    xarrs.reuse();
    for( int n=0,sz=xquery->record().count();n<sz;n++ ) {
        QString field = xquery->record().fieldName(n);
        xarrs.add()->add(Q2A(field));
    }
    if( root->gv("@fields")==NULL ) {
        root->GetVar("@fields")->setVar('a',0,(LPVOID)&xarrs );
    }
    LPCC code = NULL;
    while( xquery->next() ) {
        TreeNode* cur=root->addNode();
        for( int n=0,sz=xarrs.size();n<sz;n++ ) {
            code = xarrs.get(n)->get();            
            cur->GetVar(code)->set(Q2A(xquery->value(n).toString()));
        }
    }
    return root;
}

int	SqlDatabase::fetchTreeResult(StrVar* rst, U16 flag) {
    if( !isOpen() || rst==NULL ) return 0;
    xarrs.reuse();
    for( int n=0,sz=xquery->record().count();n<sz;n++ ) {
        QString field = xquery->record().fieldName(n);
        xarrs.add()->add(Q2A(field));
    }
    int sz = xarrs.size();
    int n=0;
    for(; xquery->next(); n++ ) {
        if( n>0 ) rst->add("\r\n");
        for( int x=0;x<sz;x++ ) {
            if( x>0 ) rst->add('\f');
            rst->add(Q2A(xquery->value(x).toString()) );
        }
    }
    return n;
}

XListArr*	SqlDatabase::fields(U32 flag) {
    XListArr* a=xarrs.reuse(); // xcf->addArray("@fields");
    if( !isOpen() ) {
        return a;
    }
    for( int n=0,sz=xquery->record().count();n<sz;n++ ) {
        QString field = xquery->record().fieldName(n);
        a->add()->set(Q2A(field));
    }
    return a;
}
