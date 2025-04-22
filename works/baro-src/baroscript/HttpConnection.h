#ifndef HttpConnection_H
#define HttpConnection_H


#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>

#include "func_util.h"

bool waitForSignal(QObject *sender, const char *signal, int timeout);

class HttpConnection : public QObject {
	Q_OBJECT
public:
	HttpConnection(TreeNode* cf);
	~HttpConnection();

public:
	bool				recv(); 
	QNetworkReply*		call();
	void				setHeader(LPCC key, LPCC value);

public:	
	void close();
	TreeNode* config()	{ return xcf; }
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)	{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag) { return xflag&=~flag; }

	//////////////////////////////////////////////////////////////////////////
 
public slots:
	void onSslError(QNetworkReply*,QList<QSslError>);
	void slotDownloadProgress(qint64, qint64);
	void slotDownloadReadyRead();
	void slotUploadProgress(qint64, qint64);
	void slotError(QNetworkReply::NetworkError code );
	void slotTimeoutCheck();
	void slotFinish();
    void slotReadData();

public: 
	TreeNode*				xcf; 
	XFuncNode*				xfnCur;
	XListArr				xarrs;
    bool					xrun;
    U32						xflag;


protected:
	QNetworkAccessManager	xaccess;
	QNetworkReply*			xrep;

	QFile*					xfile;
	QTimer					xtimer;

public:	
	void setStop();
};


#endif

