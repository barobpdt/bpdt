#ifndef DEF_H
#define DEF_H
#define QT5_USE
#define WIDGET_USE
// #define MPLAYER_USE
// #define WEBVIEW_USE
#define WINDOW_USE
#define WEBTOOL_USE
#define FUNCNODE_USE
#define MYSQL_USE
// #define PSQL_USE
#define EXCEL_USE

#define FUNC_HEADER_POS		4
#define FUNC_SUB_POS		8
#ifdef QT5_USE
#define OBJ_PROP_START_POS  12
#else
#define OBJ_PROP_START_POS  8
#endif

#include <string>
#include <vector>
using namespace std;
using std::vector;

#define CNF_FUNC_LOG			0x2000
#define CNF_TOUCH_USE			0x4000
#define CNF_SQLITE      		0x8000
#define CNF_LOGFILE 			0x10000
#define CNF_OUTPUT_QUERY		0x20000
#define CNF_DEBUG   			0x40000
#define CNF_WORKERCALL			0x80000


#define FLAG_SETPROP		0x1
#define FLAG_REUSE			0x2
#define FLAG_TRIM			0x4
#define FLAG_SUB			0x8
#define FLAG_APPEND			0x10
#define FLAG_CONTINUE		0x100

#define FLAG_MODIFY			0x20
#define FLAG_FUNCCHK		0x100
#define FLAG_VARCHECK		0x40000

#define FLAG_ERROR			0x1
#define FLAG_SET			0x2
#define FLAG_PARAM			0x4
#define FLAG_END			0x8
#define FLAG_REF			0x8
#define FLAG_INIT			0x10
#define FLAG_START			0x20
#define FLAG_RESULT			0x40
#define FLAG_EXEC			0x40
#define FLAG_ROOT			0x80
#define FLAG_SKIP			0x80
#define FLAG_RUN			0x100
#define FLAG_CALL			0x200
#define FLAG_PERSIST		0x400
#define FLAG_DELETE			0x800
#define FLAG_NEW            0x1000
#define FLAG_USE			0x2000
#define FLAG_OK             0x4000
#define FLAG_SINGLE         0x4000
#define FLAG_DRAW			0x10000
#define FLAG_SETEVENT		0x20000

#define SVO				sv->getObject(FUNC_HEADER_POS)
#define SVCHK(a,b)		checkFuncObject(sv,(a),(b))
#define FMT             gBuf.fmt
#define SCPY            gBuf.add
#define uom             UserObjectManager::instance()

#ifdef WIDGET_USE
#include <QList>
#include <QVariant>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QEvent>
#include <QDebug>
#define _LOG        qDebug
#define SVQ			(QObject*)sv->getObject(FUNC_HEADER_POS)

#else
#define _LOG        printf
#endif

#ifdef Q_WS_WIN
#define KR(str)     QString::fromUtf8(str)
#define Q2A(str)	(str).toUtf8().constData()
#else
#define KR(str)		QString::fromUtf8(str)
#define Q2A(str)	(str).toUtf8().constData()
#endif

#define BTYPE_INT		1
#define BTYPE_INT64		2
#define BTYPE_INT32		3
#define BTYPE_IDX		4
#define BTYPE_OBJECT	5
#define BTYPE_VAR		6
#define BTYPE_DOUBLE	7
#define BTYPE_INT16		8
#define BTYPE_ARRAY		0x8

#ifdef WINDOW_USE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

typedef HANDLE HID;
#define HANDLE_INVALID			INVALID_HANDLE_VALUE
typedef unsigned long			PUID;

#else
typedef void					VOID, *LPVOID;
typedef int                     HID;
#define HANDLE_INVALID  		-1
typedef pthread_t				PUID;
#endif

typedef int				BOOL ;
typedef int				SPOS ;
typedef unsigned char			U8  , *PU8  ;
typedef unsigned short			U16 , *PU16 ;
typedef unsigned int			U32 , *PU32 ;
typedef long long				UL64, *PUL64;
typedef const char				CC, *LPCC;

#define FIND_FORWORD		0x1
#define FIND_BACK			0x2
#define FIND_TAG			0x8
#define FIND_LOWERCASE		0x10
#define FIND_UPPERCASE		0x20
#define FIND_STARTWITH		0x40
#define FIND_WORD			0x80
#define FIND_FUNCVAL		0x100
#define FIND_FUNCSTR		0x200
#define FIND_BLANK			0x400
#define FIND_OPER			0x800
#define FIND_NOCASE			0x1000
#define FIND_SKIP			0x2000
#define FIND_CHARS			0x4000
#define FIND_WCHAR			0x8000

#ifdef _WIN32
#include <time.h>
#include <shlobj.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#else

#include <time.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#if defined( __linux__ )
# include <sys/statfs.h>
#elif defined (__FreeBSD__)
# include <sys/param.h>
# include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>

#include <time.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#if defined( __linux__ )
# include <sys/statfs.h>
#elif defined (__FreeBSD__)
# include <sys/param.h>
# include <sys/mount.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <utime.h>
#include <dirent.h>

#ifndef BASE_TYPE_USE
#define BASE_TYPE_USE
#define FALSE  						0
#define TRUE   						1

typedef int						BOOL;
typedef char						CHAR;
typedef wchar_t		 				WCHAR;
typedef wchar_t *					LPWSTR;
typedef const wchar_t *					LPCWSTR;
typedef char *    					LPSTR;
typedef const char *    				LPCSTR;
typedef int						INT;
typedef unsigned int					UINT;
typedef long						LONG;
typedef float						FLOAT;
typedef double						DOUBLE;
typedef unsigned char					BYTE;
typedef unsigned short int				WORD;
typedef short int 					SHORT;
typedef unsigned long 					ULONG;
typedef unsigned long 					DWORD;
typedef long long int					LONGLONG;

typedef char *         	 				LPSTR;
typedef int						HANDLE;
typedef long long int					ULARGE_INTEGER;
typedef unsigned long long int				ULONGLONG;
typedef unsigned int                                    COLORREF;
#endif
#endif

#define SP_END			-1
#define WUID_NULL		0xFFFF
#define BLANK_CHECK			1
#define MOVE_STARTPOS		2

#define TIME_DIFF( et, st ) 	((unsigned)( (et.tv_sec - st.tv_sec)*1000000 + ( et.tv_usec - st.tv_usec  ) ))
#define IS_2BYTEWORD( _a_ ) 	((char)(0x80) <= (_a_) && (_a_) <= (char)(0xFF) )
#define IS_DIGIT(ch)			(ch >= 48 && ch <= 57)


#define MIN(a,b) 				(((a)<(b)) ? (a) : (b))
#define MAX(a,b) 				(((a)>(b)) ? (a) : (b))
#define SWAP(a,b) 				{(a)^=(b)^=(a)^=(b);}
#define ISBLANK( c )			( (c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n' || (c) == 0 )
#define asizeof( x )			(sizeof(x)/sizeof(x[0]))

#define SETBIT(variable,bit)	variable|=bit
#define CLEARBIT(variable,bit)	variable&=~bit
#define SAFE_FREE(p)			{ if(p) { free(p);     (p)=NULL; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_CLOSESOCKET(p)		{ if(p != SOCKET_ERROR) { closesocket(p); (p) = -1; } }
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_COLOR(a,c)	ARGB( ((a&0x000000ff) << 24) | ((c&0x000000ff) << 16) |((c&0x0000ff00) <<  0) | ((c&0x00ff0000) >> 16) )


#define IS_BLANKW( c )	( (c) == L' ' || (c) == L'\t' || (c) == L'\r' || (c) == L'\n' || (c) == 0 )
#define IS_OPERW(ch)  (c == L'%' || c == L'^' || c == L'&' || c == L'*' ||\
            c == L'(' || c == L')' || c == L'-' || c == L'+' ||\
            c == L'=' || c == L'|' || c == L'{' || c == L'}' ||\
            c == L'[' || c == L']' || c == L':' || c == L';' ||\
            c == L'<' || c == L'>' || c == L',' || c == L'/' ||\
            c == L'?' || c == L'!' || c == L'~' )

#define IS_OPERATION(ch)  (ch == '%' || ch == '^' || ch == '&' || ch == '*' ||\
            ch == '(' || ch == ')' || ch == '-' || ch == '+' ||\
            ch == '=' || ch == '|' || ch == '{' || ch == '}' ||\
            ch == '[' || ch == ']' || ch == ':' || ch == ';' ||\
            ch == '<' || ch == '>' || ch == ',' || ch == '.' ||\
            ch == '~' || ch == '!' || ch == '@' || ch == '#' || ch == '$' || ch == '?' || ch == '/' || ch == '\'' || ch == '\"' )


#endif // DEF_H
