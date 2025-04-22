#include "HttpConnection.h"
#include "../webserver/WorkerThread.h"
#include "../webserver/HttpNode.h"
#include <QHttpMultiPart>
#include <QHttpPart>

#define MAX_RECV_SIZE 4096

bool waitForSignal(QObject *sender, const char *signal, int timeout) {
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(timeout);
    timer.setSingleShot(true);

    loop.connect(sender, signal, SLOT(quit()));
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    timer.start();
    loop.exec();

    return timer.isActive();
}

HttpConnection::HttpConnection(TreeNode* cf) : 
	xrun(false), xrep(NULL), xflag(0), xfile(NULL),  xcf(cf)  {
	xfnCur = NULL;
} 

HttpConnection::~HttpConnection() {
    if(xcf) xcf->reset(true);
}

void HttpConnection::onSslError(QNetworkReply* pReply,QList<QSslError> ssl) {
	pReply->ignoreSslErrors(ssl);
    xcf->GetVar("@error")->set("https call error");
	// gum.onHttpError(config());
}

void HttpConnection::slotFinish() {
    TreeNode* node=xcf->getNode("@params");
    if( node ) {
        node->removeAll(true);
    }
    node=xcf->getNode("@header");
    if( node ) {
        node->removeAll(true);
    }
    close();
}

void HttpConnection::slotTimeoutCheck() {
	if( xrep ) {
		if( xrep->isRunning() ) {
			qDebug()<<"slotTimeoutCheck ############ isRunning \n";
            xcf->GetVar("@error")->set("http call timeout");
			xrep->abort();
		}
		qDebug()<<"slotTimeoutCheck ############ close \n";
        // close();
	}
	// gum.onHttpError(config());
}
 

 
void HttpConnection::setStop() {    
    if( xrep && xrep->isRunning() ) {
        xcf->GetVar("@error")->set("http stop client abort");
        xrep->abort();
    }
    if( xrun==false ) {
        if(xfile) {
            xfile->close();
            SAFE_DELETE(xfile);
        }
        return;
    }
    close();
}

void HttpConnection::slotError(QNetworkReply::NetworkError code ) {
    XFuncNode* fn = xfnCur;
    qDebug("http call error node:%s", xcf->log() );
    if( fn && xrep ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set("error");
        StrVar* err=arrs->add();
        QVariant status = xrep->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        switch(code ) {
        case QNetworkReply::ConnectionRefusedError :			err->add("ConnectionRefusedError"); break;
        case QNetworkReply::RemoteHostClosedError :				err->add("RemoteHostClosedError"); break;
        case QNetworkReply::HostNotFoundError :					err->add("HostNotFoundError"); break;
        case QNetworkReply::TimeoutError :						err->add("TimeoutError"); break;
        case QNetworkReply::OperationCanceledError :			err->add("OperationCanceledError"); break;
        case QNetworkReply::SslHandshakeFailedError :			err->add("SslHandshakeFailedError"); break;
        case QNetworkReply::TemporaryNetworkFailureError :		err->add("TemporaryNetworkFailureError"); break;
        case QNetworkReply::UnknownNetworkError :				err->add("UnknownNetworkError"); break;
        case QNetworkReply::ProxyConnectionRefusedError :		err->add("ProxyConnectionRefusedError"); break;
        case QNetworkReply::ProxyConnectionClosedError :		err->add("ProxyConnectionClosedError"); break;
        case QNetworkReply::ProxyNotFoundError :				err->add("ProxyNotFoundError"); break;
        case QNetworkReply::ProxyTimeoutError :					err->add("ProxyTimeoutError"); break;
        case QNetworkReply::ProxyAuthenticationRequiredError :	err->add("ProxyAuthenticationRequiredError"); break;
        case QNetworkReply::UnknownProxyError :					err->add("UnknownProxyError"); break;
            // content errors (201-299):
        case QNetworkReply::ContentAccessDenied :				err->add("ContentAccessDenied"); break;
        case QNetworkReply::ContentOperationNotPermittedError : err->add("ContentOperationNotPermittedError"); break;
        case QNetworkReply::ContentNotFoundError :				err->add(""); break;
        case QNetworkReply::AuthenticationRequiredError :		err->add("AuthenticationRequiredError"); break;
        case QNetworkReply::ContentReSendError :				err->add("ContentReSendError"); break;
        case QNetworkReply::UnknownContentError :				err->add("UnknownContentError"); break;
            // protocol errors
        case QNetworkReply::ProtocolUnknownError :				err->add("ProtocolUnknownError"); break;
        case QNetworkReply::ProtocolInvalidOperationError :		err->add("ProtocolInvalidOperationError"); break;
        case QNetworkReply::ProtocolFailure :					err->add("ProtocolFailure"); break;
        default: err->add("unknown error"); break;
        }
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
    if( xcf->isNodeFlag(HTTP_DOWNLOAD ) ) {
        if( xfile && xfile->isOpen() ) {
            xfile->close();
            LPCC fnm=xcf->get("fileName");
            if( slen(fnm) ) {
                QFileInfo fi(KR(fnm));
                if( fi.isFile() ) {
                    if( QFile::remove(KR(fnm)) ) {
                        qDebug("#0#download file delete (file: %s)\n", fnm);
                    } else {
                        qDebug("#0#download file delete fail (file: %s)\n", fnm);
                    }
                }
            }
        }
    }
}

QNetworkReply* HttpConnection::call() {
	if( xcf==NULL )
		return NULL;
    if( xrun || xfile ) {
        qDebug("web call already running URL:%s", xcf->get("url"));
        // return NULL;
        setStop();
    }
	QNetworkRequest req;
	LPCC type = xcf->get("type");
	LPCC method = xcf->get("method");
    if(xcf->gv("@result") ) {
        xcf->GetVar("@result")->reuse();
    }
    xcf->xflag=0;
    xcf->setNodeFlag(FLAG_START );
	if( slen(method)==0 )
		method="GET";
    StrVar* sv=xcf->gv("@callback");
    if(SVCHK('f',0) ) {
        xfnCur=(XFuncNode*)SVO;
    }
    TreeNode* params= xcf->getNode("@params");
    if( params ) {
        int len=slen(method);
        if( ccmpl(method,len,"get") || ccmpl(method,len,"delete") ) {
			int n=0;
			StrVar* rst=xcf->GetVar("@data");
            StrVar* var=xcf->GetVar("url");
            for( WBoxNode* bn=params->First(); n<1024 && bn; bn = params->Next() ) {
                LPCC code=params->getCurCode();
				if( code[0]=='@' ) continue;
                if( n==0 )
                    var->add('?');
                else
                    var->add('&');
                var->add(code).add('=').add( getUrlEncode(bn->get(), rst->reuse()) );
				n++;
			}
            qDebug("#0# http call url=%s, (%d)\n", var->get(), n);
		}
	}
	LPCC url=xcf->get("url");
    req.setUrl( QUrl(KR(url)) );
    xcf->GetVar("@error")->reuse();

    if( xcf->isNode("@header") ) {
        TreeNode* header = xcf->getNode("@header");
		int n=0;
		for( WBoxNode* bn = header->First(); bn; bn = header->Next() ) {
            LPCC code = header->getCurCode();
			req.setRawHeader(QByteArray(code,slen(code)), QByteArray(bn->get(), bn->length()) );
			n++;
		}
		qDebug("## header count (%d)\n", n);
	}

	if( ccmp(type,"upload") ) {
		xcf->setNodeFlag(HTTP_UPLOAD);
	} else if( ccmp(type,"download") ) {
		xcf->setNodeFlag(HTTP_DOWNLOAD);
    }
    LPCC fnm=getFullFileName(xcf->get("fileName"));
	if( slen(fnm) ) {
		QString fileName=KR(fnm);
		if( xcf->isNodeFlag(HTTP_UPLOAD) ) {
			QFileInfo fi(fileName);
			if( fi.isFile() ) {
				xfile=new QFile(fileName);
				xfile->open(QIODevice::ReadOnly);
			} else {
                xcf->GetVar("@error")->fmt("upload file not found:%s ",fnm);
			}
        } else {
			xfile=new QFile(fileName);
            if( xfile->open(QIODevice::WriteOnly) ) {
                qDebug("download filename : %s\n", fnm);
            } else {
                xcf->GetVar("@error")->fmt("download file error:%s ",fnm);
            }
		}
	}
	
    int len=slen(method);    
    qDebug("#0#web call start method:%s URL:%s\n", method, xcf->get("url") );
    if( ccmpl(method,len,"get",3) ) {
		xrep = xaccess.get(req);
    } else  if( ccmpl(method,len,"post",4) || ccmpl(method,len,"upload",6) ) {
		QByteArray send;
		if( xcf->isNodeFlag(HTTP_UPLOAD) ) {
            if( xfile==NULL ) {
                qDebug("#0#upload file error file name: %s\n",xcf->get("fileName"));
                return NULL;
            }
            if(xcf->getBool("multiPart")) {
                QHttpMultiPart *httpMultPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
                if( params ) {
                    StrVar* rst=xcf->GetVar("@data");
                    for( WBoxNode* bn=params->First(); bn; bn = params->Next() ) {
                        LPCC code=params->getCurCode();
                        if( code[0]=='@' ) continue;
                        QHttpPart part;
                        LPCC val=getUrlEncode(bn->get(),rst->reuse() );
                        LPCC field=gBuf.fmt("form-data; name=\"%s\"",code);
                        QByteArray ba(val);
                        part.setHeader(QNetworkRequest::ContentDispositionHeader, KR(field));
                        part.setBody(ba);
                        httpMultPart->append(part);
                    }
                }
                QHttpPart filePart;
                LPCC mtype=xcf->get("multiPartType");
                LPCC mparam=xcf->get("multiPartParam");
                if(slen(mtype)==0) mtype="image/jpeg";
                filePart.setHeader(QNetworkRequest::ContentTypeHeader, KR(mtype));
                filePart.setHeader(QNetworkRequest::ContentDispositionHeader, KR(mparam));
                filePart.setBodyDevice(xfile);
                req.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=\""+httpMultPart->boundary()+"\"");
                httpMultPart->append(filePart);
                xrep = xaccess.post(req,httpMultPart);
                httpMultPart->setParent(xrep);
            } else {
                xrep = xaccess.post(req,xfile);
            }
		} else {
			StrVar* data = xcf->GetVar("data"); //->reuse(); 
            // StrVar* rst=xcf->GetVar("@data");
            if( data && data->length()>0 ) {
                send.append(data->get(),data->length());
			}
            if( send.length()==0 ) {
                qDebug()<<"HTTP POST length==0 " << KR(xcf->get("url")) << " method:" << method;
            } else {
                qDebug()<<"HTTP POST length==" << send.length();
            }
			req.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(send.size()));
			xrep = xaccess.post(req,send);
		}
    } else if( ccmpl(method,len,"put",3) || ccmpl(method,len,"patch",5) ) {
        QByteArray send;
        StrVar* data = xcf->GetVar("data");
        if( data && data->length()>0 ) {
            send.append(data->get(),data->length());
        }
        req.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(send.size()));
        if(ccmpl(method,len,"patch",5) ) {
            req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            xrep=xaccess.sendCustomRequest(req,"PATCH",send);
        } else {
            xrep=xaccess.put(req,send);
        }
    } else if( ccmpl(method,len,"delete",6) ) {
		xrep = xaccess.deleteResource(req);
    } else {
        qDebug("web call method not defined METHOD:%s", method);
        return NULL;
        /*
        if( xfile && ccmpl(method,len,"speech",6) ) {
            xrep = xaccess.post(req, xfile);
        }
        */
    }
    xrun=true;
	connect(&xaccess,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),this,SLOT(onSslError(QNetworkReply*,QList<QSslError>)));
	connect(xrep, SIGNAL(error(QNetworkReply::NetworkError)),	this, SLOT(slotError(QNetworkReply::NetworkError)));
    if( xcf->isNodeFlag(HTTP_DOWNLOAD ) ) {
        connect(xrep, SIGNAL(readyRead()), this, SLOT(slotDownloadReadyRead()));
    } else {
        connect(xrep, SIGNAL(readyRead()), this, SLOT(slotReadData()));
    }
    connect(xrep, SIGNAL(finished()), this, SLOT(slotFinish()));
    if( xcf->isNodeFlag(HTTP_DOWNLOAD|HTTP_UPLOAD ) ) {
        if( xcf->isNodeFlag(HTTP_DOWNLOAD ) ) {
            connect(xrep, SIGNAL(downloadProgress(qint64, qint64)),	this, SLOT(slotDownloadProgress(qint64, qint64)));
        } else {
            connect(xrep, SIGNAL(uploadProgress(qint64, qint64)), this,	SLOT(slotUploadProgress(qint64, qint64)));
        }
    } else {
        QEventLoop loop;
        int timeout=toInteger(xcf->gv("@timeout") );
        QObject::connect(xrep, SIGNAL(finished()), &loop, SLOT(quit()));
        if( timeout==0 ) timeout=15000;
        xtimer.setInterval(timeout);
        xtimer.setSingleShot(true);
        xtimer.start();
        QObject::connect(&xtimer, SIGNAL(timeout()), this, SLOT(slotTimeoutCheck()));
        qDebug("#0#web exec URL:%s, tmeout:%d\n", xcf->get("url"), timeout);
        loop.exec();
    }
	return xrep;
}
void HttpConnection::slotDownloadReadyRead() { 
    if( xrep==NULL ) return;
    if( xfile && xfile->isOpen() ) {
        xfile->write( xrep->readAll() );
        // qDebug("download size=%d\n", ba.length() );
    }
}

void HttpConnection::slotDownloadProgress(qint64 recv, qint64 total) {
    /*
	if( !config()->isNodeFlag(FLAG_START) ) {
		setStop();
		return;
	}
    */
    if( xfnCur ) {
        PARR arrs=getLocalParams(xfnCur);
        arrs->add()->set("progress");
        arrs->add()->setVar('1',0).addUL64(recv);
        arrs->add()->setVar('1',0).addUL64(total);
        setFuncNodeParams(xfnCur, arrs);
        xfnCur->call();
    }
}
void HttpConnection::slotUploadProgress(qint64 send, qint64 total) {
    /*
    if( !config()->isNodeFlag(FLAG_START) ) {
        setStop();
        return;
    }
    */
    if( xfnCur ) {
        PARR arrs=getLocalParams(xfnCur);
        arrs->add()->set("progress");
        arrs->add()->setVar('1',0).addUL64(send);
        arrs->add()->setVar('1',0).addUL64(total);
        setFuncNodeParams(xfnCur, arrs);
        xfnCur->call();
    }
}



void HttpConnection::close() {
    xrun=false;
	if( xrep ) {
        StrVar* sv=xcf->gv("@c");
        if( sv ) {
            sv->reuse();
        }
        if( xfile ) {
            if( xfile->isOpen() ) {
                xfile->close();
            }
			SAFE_DELETE(xfile);
        }
        if( xfnCur ) {
            PARR arrs=getLocalParams(xfnCur);
            arrs->add()->set("finish");
            arrs->add()->setVar('n',0,(LPVOID)xcf);
            setFuncNodeParams(xfnCur, arrs);
            xfnCur->call();
        }
        // deleteLater();
        xrep->deleteLater();
        xrep=NULL;
    }
    if( xcf->isNodeFlag(HTTP_UPLOAD) ) {
        if(xcf->getBool("multiPart")) {
            xcf->setBool("multiPart",false);
            xcf->set("multiPartType", "");
            xcf->set("multiPartParam", "");
        }
    }
    /*
    xcf->clearNodeFlag(FLAG_START );
    xcf->clearNodeFlag(HTTP_UPLOAD | HTTP_DOWNLOAD );
   */
    xcf->xflag=0;
    xcf->set("type","");
}


void HttpConnection::slotReadData() {
    if( xrep && xfnCur && xrep->error()==QNetworkReply::NoError ) {
        // callback(type, data, length, content-type);
        char buf[MAX_RECV_SIZE];
        qint64 length = xrep->rawHeader("Content-Length").toLongLong();
        PARR arrs=getLocalParams(xfnCur);
        arrs->add()->set("read");
        arrs->add();
        arrs->add()->setVar('1',0).addUL64(length);
        arrs->add()->set(xrep->rawHeader("Content-Type").constData());
        StrVar* var=arrs->get(1);
        for( int n=0; xrun; n++ ) {
            qint64 r = xrep->read(buf,MAX_RECV_SIZE);
            if( r<=0 )
                break;
            var->add(buf,r);
        }
        setFuncNodeParams(xfnCur, arrs);
        xfnCur->call();
	}
}
