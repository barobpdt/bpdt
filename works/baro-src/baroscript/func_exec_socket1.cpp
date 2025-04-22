#include "func_util.h"
#include "../webserver/HttpNode.h"
#include "../webserver/ConnectNode.h"
#include "../fobject/FMutex.h"
// #include "../XCOM/XCOM.h"


FMutex	gMutexDatabase;


void regConnectNodeFuncs() {
    if( isHashKey("socket") )
        return;
    PHashKey hash = getHashKey("socket");
    U16 uid = 1;
    hash->add("param", uid);		uid++;	//
    hash->add("send", uid);			uid++;	//
    uid++;	//
    uid++;	//
    hash->add("socketType", uid);	uid++;	//
    hash->add("sendFile", uid);		uid++;	//
    hash->add("sendBuffer", uid);	uid++;	//
    hash->add("waitConnect", uid);  uid++;	//
    hash->add("readBuffer", uid);	uid++;	//
    uid = 12;
    hash->add("sendPage", uid);		uid++;	//
    hash->add("readAll", uid);      uid++;	//
    hash->add("read", uid);         uid++;	//
    hash->add("sendData", uid);		uid++;	//
    uid = 22;
    hash->add("timeout", uid);		uid++;
    hash->add("content", uid);		uid++;	//
    uid = 31;
    hash->add("connect", uid);		uid++;	//
    hash->add("isConnect", uid);		uid++;	//
    hash->add("isRead", uid);		uid++;	//
    hash->add("isWrite", uid);		uid++;	//
    hash->add("close", uid);		uid++;	//
    hash->add("delay", uid);		uid++;	//
    uid = 41;
    hash->add("postData", uid);		uid++;	//
    hash->add("clients", uid);	uid++;	//
    hash->add("recv", uid);		uid++;	//
    uid++;	// hash->add("conf", uid);
    hash->add("rootNode", uid);	uid++;	//
    hash->add("sendData", uid);	uid++;	//
    uid++;	// hash->add("was", uid);
    uid++;	// hash->add("reqInfo", uid);
    hash->add("sendStream", uid);	uid++;	//
    hash->add("reqHeaders", uid);	uid++;	//
    hash->add("getValue", uid);	uid++;	//
    hash->add("setValue", uid);	uid++;	//
    hash->add("database", uid);	uid++;	//
    hash->add("setCookie", uid);	uid++;	//
    uid=60;
    hash->add("flagState", uid);	uid++;	// 60
    hash->add("redirect", uid);	uid++;	//
    hash->add("webEval", uid);	uid++;	//
    hash->add("getVar", uid);	uid++;	//
    hash->add("recvWeb", uid);	uid++;	//
    hash->add("proxySend", uid);	uid++;	//
    hash->add("proxyRecv", uid);	uid++;	//
}


bool callConnectNodeFunc(XFunc* fc, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    if( tn==NULL )
        return false;
    ConnectNode* conn=NULL;
    StrVar* sv=tn->GetVar("@c");
    if( SVCHK('v',1) ) {
        conn=(ConnectNode*)SVO;
    } else {
        conn = new ConnectNode(tn);
        sv->setVar('v',1,(LPVOID)conn);
    }
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        regConnectNodeFuncs();
        if( fc ) {
            LPCC fnm=fc->getFuncName();
            ref = getHashKeyVal("socket",fnm);
            fc->xfuncRef = ref;
        }
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {		// param
        TreeNode* param=NULL;
        StrVar* sv = tn->gv("@host");
        if( sv ) {
            sv=tn->gv("@param");
            if( SVCHK('n',0) ) {
                param=(TreeNode*)SVO;
            } else {
                param=new TreeNode(2, true);
                tn->GetVar("@param")->setVar('n',0,(LPVOID)param);
            }
        } else {
            param=conn->config();
        }

        if( arrs==NULL ) {
            rst->setVar('n',0,(LPVOID)param);
        } else {
            LPCC code= AS(0);
            if( slen(code) ) rst->reuse()->add(param->gv(code));
        }
    } break;
    case 2: {		// send
        if( conn->xtype==SOKETTYPE_MODULE ) {
            int sp=0,ep=0;
            StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            if( sp<ep ) conn->send(sv->get(sp), ep-sp);
            return true;
        }
        StrVar* sv=NULL;
        if( arrs==NULL ) {
            rst->set("layout data not found");
            sendHttpHeader(conn,conn->getSendVar(),(qint64)rst->length());
            conn->send(rst);
        } else {
            int sp=0,ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            int len = ep-sp;
            // tn->GetVar("@sendCheck")->setVar('3',1);
            // qDebug(">> connect node send size = %d\n", len);
            if( len>0 ) {
                sendHttpHeader(conn,conn->getSendVar(),(qint64)len);
                conn->send(sv->get(sp), len);
            } else {
                StrVar* var=cfVar("httpEmpty");
                if( var->length()==0 ) {
                    var->set("error: connectNode send size==0");
                }
                sendHttpHeader(conn,conn->getSendVar(),(qint64)var->length() );
                conn->send(var->get(), var->length() );
            }
        }
        TreeNode* db=NULL;
        sv=conn->gv("@database");
        if( SVCHK('n',0) ) {
            db=(TreeNode*)SVO;
        }
        /*
        sv = conn->gv("resHeader");
        if( sv && sv->length() ) {
            qDebug("== proxy req socket close ==\n");
            conn->closeClient();
        } else {
            //(na) 20191011
        }
        */
        initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
        if(db) {
            db->clearNodeFlag(FLAG_USE);
            qDebug("..... page data base release .........\n");
        }
    } break;
    case 5: {		// socketType
        if( arrs==NULL ) {
            rst->set( tn->isNodeFlag(SOCKETTYPE_ASYNC) ? "async" : "sync");
        } else {
            LPCC type=AS(0);
            if( ccmp(type,"async") ) {
                tn->setNodeFlag(SOCKETTYPE_ASYNC);
            } else {
                tn->clearNodeFlag(SOCKETTYPE_ASYNC);
            }
        }
    } break;
    case 6: {		// sendFile
        LPCC path=AS(0);
        QFile file(KR(path));
        XSocket* sock=conn->xsock;
        conn->setNodeFlag( FLAG_RUN | WOKRER_DOWNLOAD );
        if( file.open(QIODevice::ReadOnly) && sock ) {
            int pos= LastIndexOf(path,'.');
            LPCC ext = pos>0 ? path+pos+1: NULL;
            pos= LastIndexOf(path,'/');
            if( pos==-1 ) {
                pos=LastIndexOf(path,'/');
            }
            LPCC filenm = pos!=-1 ? path+pos+1: path;
            StrVar* sv=conn->GetVar("@attachHeader");
            sv->add("Content-Disposition: attachment; filename=\"").add(filenm).add("\"\r\n");
            qint64 fsize=file.size(), r=0, w=0;
            StrVar* sendCheck=conn->getSendVar();
            sendCheck->add(filenm);
            sendHttpHeader(conn, rst, fsize, ext);
            char buf[4096];
            while( fsize>0 ) {
                r=file.read(buf,4096);
                if( r<=0 ) break;
                if( !conn->valid() ) break;
                conn->setTick();
                w=sock->Send(buf,r);
                if( w<=0 ) break;
                if( r!=w ) {
                    qDebug(">> sendFile read send check (read:%d, send:%d)\n", r, w);
                    while( w<r ) {
                        int s=sock->Send(buf+w,r-w);
                        if( s<=0 ) break;
                        w+=s;
                    }
                    if( r!=w ) {
                        qDebug(">> sendFile error (read:%d, send:%d)\n", r, w);
                        break;
                    }
                }
                fsize-=r;
            }
            file.close();
        }
        initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
    } break;
    case 7: {		// sendBuffer
        if( conn==NULL || conn->xsock==NULL ) return false;

        if( !conn->valid() ) {
            return true;
        }
        if( arrs==NULL ) {
            if( tn->isNodeFlag(SOCKETTYPE_ASYNC) ) {
                StrVar* sv=tn->gv("@sendData");
                int len=sv->length();
                if( len>0 ) {
                    if( conn->send(sv->get(), len) > 0 ) {
                        tn->GetVar("@sendData")->reuse();
                    }
                    rst->setVar('3',1);
                }

            }
        } else {
            StrVar* sv=arrs->get(1);
            if( sv ) {
                LPCC ext=toString(sv);
                if( ccmp(ext,"png") ) {
                    QPixmap* pm=getStrVarPixmap(arrs->get(0) );
                    if( pm ) {
                        QByteArray ba;
                        QBuffer buf(&ba);
                        pm->save(&buf,"PNG");
                        sendHttpHeader(conn,rst,ba.size(), ext);
                        conn->send(ba.constData(),ba.size());
                    }
                    initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
                } else {
                    qDebug("#0#sendBuffer error (type: %s)\n\n", ext);
                }
            } else {	// if( conn->xtype==SOKETTYPE_MODULE )
                rst->setVar('3',0);
                int sp=0,ep=0;
                sv = getStrVarPointer(arrs->get(0), &sp, &ep);
                if( sp<ep ) {
                    if( tn->isNodeFlag(SOCKETTYPE_ASYNC) ) {
                        tn->GetVar("@sendData")->set(sv->get(sp), ep-sp);
                        rst->setVar('3',1);
                    } else {
                        int send=conn->send(sv->get(sp), ep-sp);
                        rst->setVar('0',0).addInt(send);
                    }
                }
            }
        }
    } break;
    case 8: {		// waitConnect
        int cnt=16;
        if(arrs) {
            sv=arrs->get(0);
            if(isNumberVar(sv)) {
                cnt=toInteger(sv);
            }
        }
        if(conn->valid() ) {
            rst->setVar('3',1);
        } else {
            for(int n=0; n<cnt; n++ ) {
                FThread::Sleep(50);
                if(conn->valid() ) {
                    rst->setVar('3',1);
                    break;
                }
            }
        }
    } break;
    case 9: {		// readBuffer
        if( conn==NULL || conn->xsock==NULL ) {
            return true;
        }
        if( !conn->valid() ) {
            qDebug("socket readBuffer not vaild");
            return true;
        }
        rst->reuse();
        // qDebug("readBuffer start===================\n");
        if( arrs==NULL ) {
            StrVar* result=conn->recv(NULL);
            if( result ) {
                int sp=0, ep=result->length();
                // qDebug("readBuffer ===================(%d,%d)\n", sp, ep);
                if( sp<=ep ) {
                    rst->setVar('s',0,(LPVOID)result).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                }
            }
        } else {
            StrVar* sv=arrs->get(0);
            if( SVCHK('3',1) ) {
                StrVar* result=conn->recv(conn->getContentVar() );
                if( result ) {
                    int sp=0, ep=result->length();
                    if( sp<ep ) {
                        rst->setVar('s',0,(LPVOID)result).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                    }
                }
            } else if( isNumberVar(sv) ) {
                int size = toInteger(arrs->get(0));
                if( size>0 ) {
                    conn->read(rst, size);
                }
            } else {
                LPCC type=toString(sv);
                if(ccmpl(type,slen(type),"post",4)) {
                    StrVar* result=conn->recv(NULL);
                    if(result) {
                        httpParseParam(conn,result->get());
                    }
                } else {
                    conn->recv(rst);
                }
            }
        }
    } break;

    case 12: {		// sendPage
        if( arrs==NULL ) {
            return false;
        }
        sv=arrs->get(0);
        if(SVCHK('n',0)) {
            TreeNode* page=(TreeNode*)SVO;
            if( sendWebPageSource(conn,page) ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        }
        initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
    } break;
    case 13: {		// readAll
        if(arrs==NULL) {
            if(conn->valid()) {
                XSocket* sock=conn->xsock;
                for(int n=0; n<4096; n++ ) {
                    if( conn->isRead(50) ) {
                        BYTE* buf = (BYTE*)rst->memalloc(SOCK_BUF_SIZE);
                        int recv = sock->Receive((LPVOID)buf,SOCK_BUF_SIZE);
                        if(recv>0 ) {
                            rst->movepos(recv);
                        } else {
                            break;
                        }
                    } else {
                        qDebug("#0#readAll read timeout (idx:%d, size:%d)\n", n, rst->length());
                        break;
                    }
                }
            } else {
                qDebug("#0#readAll socket not vaild !!!\n");
            }
        } else {
            LPCC ty=AS(0);
            if(ccmp(ty,"http") ) {
                int contentLen = readHttpHeader(conn, rst->reuse(), NULL);
                if( contentLen>0 ) {
                    int sz = rst->length();
                    if( sz<contentLen ) {
                        conn->read(rst,contentLen-sz);
                    }
                }
            }
        }

    } break;
    case 14: {		// read
        if(arrs==NULL) {
            StrVar* var=conn->read();
            int len=var? var->length(): 0;
            if(len > 0 ) {
                rst->setVar('s',0,(LPVOID)var).addInt(0).addInt(len).addInt(0).addInt(len);
            }
        } else {
            conn->read(rst);
        }
    } break;
    case 15: {		// sendData
        StrVar* var=conn->getSendVar();
        if(arrs==NULL ) {
            rst->set(toString(var));
            return true;
        }
        for(int n=0,sz=arrs->size();n<sz;n++ ) {
            int sp=0,ep=0;
            StrVar* sv=getStrVarPointer(arrs->get(n),&sp,&ep);
            if(sp<ep) var->add(sv,sp,ep );
        }
        qDebug("...send data var size:%d\n", var->length() );
        conn->send(var);
    } break;

    case 22: {		// timeout
        if( arrs && isNumberVar(arrs->get(0)) ) {
            int timeout=toInteger(arrs->get(0));
            conn->xtick+=timeout;
            tn->GetVar("@timeout")->setVar('0',0).addInt(timeout);
        }
    } break;
    case 23: {		// content
        if( arrs==NULL ) {
            StrVar* var=conn->getContentVar();
            int sp=0, ep=var->length();
            rst->setVar('s',0,(LPVOID)var ).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
        } else {
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            conn->getContentVar()->set(sv, sp, ep);
        }
    } break;
    case 31: {		// connect
        if( conn==NULL ) return false;
        if( arrs==NULL ) {
            if( tn->isNodeFlag(SOCKETTYPE_ASYNC) ) {
                LPCC host=tn->get("@host");
                int port=0;
                if( slen(host) ) {
                    StrVar* sv=tn->gv("@port");
                    if( isNumberVar(sv) ) port=toInteger(sv);
                    if( port==0 ) port=80;
                    if( conn->connect(host, port) ) {
                        tn->GetVar("@host")->set(host);
                        tn->GetVar("@port")->setVar('0',0).addInt(port);
                        conn->isNodeFlag(FLAG_RUN);
                        conn->xtype=SOKETTYPE_MODULE;
                        rst->setVar('3',1);
                    } else {
                        rst->setVar('3',0);
                    }
                }
            } else {
                rst->setVar('3',conn->valid()?1:0);
            }
            return true;
        }
        if( conn->valid() ) {
            conn->closeClient();
        }
        LPCC host = AS(0);
        int port = isNumberVar(arrs->get(1))? toInteger(arrs->get(1)): 80;
        if( tn->isNodeFlag(SOCKETTYPE_ASYNC) ) {
            tn->GetVar("@host")->set(host);
            tn->GetVar("@port")->setVar('0',0).addInt(port);
            rst->setVar('3',1);
        } else {
            if( isNumberVar(arrs->get(2)) ) {
                int timeout=toInteger(arrs->get(2));
                conn->xtick+=timeout;
                tn->GetVar("@timeout")->setVar('0',0).addInt(timeout);
            }
            if( conn->connect(host, port) ) {
                conn->isNodeFlag(FLAG_RUN);
                if( !isVarTrue(arrs->get(2)) ) {
                    conn->xtype=SOKETTYPE_MODULE;
                }
                tn->GetVar("@host")->set(host);
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        }
    } break;
    case 32: {		// isConnect
        if( conn->xsock ) {
            rst->setVar('3',conn->valid()? 1:0 );
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 33: {		// isRead
        int delay =100;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            delay=toInteger(arrs->get(0));
        }
        rst->setVar('3', conn->valid() && conn->isRead(delay)? 1:0);
    } break;
    case 34: {		// isWrite
        int delay =100;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            delay=toInteger(arrs->get(0));
        }
        rst->setVar('3', conn->valid() && conn->isWrite(delay)? 1:0);
    } break;
    case 35: {		// close
        if( conn->valid() ) {
            conn->closeClient();
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 36: {		// delay
        int delay = 0;
        if( arrs==NULL ) {
            delay = toInteger(tn->gv("@delay"));
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                delay = toInteger(arrs->get(0));
                tn->GetVar("@delay")->setVar('0',0).addInt(delay);
            }
        }
        rst->setVar('0',0).addInt(delay);
    } break;
    case 41: {		// postData
        StrVar* sv = &(conn->xsend);
        int sp=0, ep=sv->length();
        rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
    } break;
    case 42: {		// clients
        XListArr* a=tn->addArray("@childs",true);
        for( int n=0, sz=conn->xchilds.size(); n<sz; n++ ) {
            ConnectNode* c=conn->xchilds.getvalue(n);
            a->add()->setVar('v',1,(LPVOID)c);
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 43: {		// recv
        conn->recv( rst->reuse() );
    } break;
    case 44: {		// conf

    } break;
    case 45: {		// rootNode
        ConnectNode* parent=conn->parent();
        if( parent ) {
            TreeNode* cf=parent->config();
            StrVar* sv=cf? cf->gv("@c"): NULL;
            if( SVCHK('v',0) ) {
                rst->setVar('n',0,(LPVOID)cf);
            }
        }
    } break;
    case 46: {		// sendData
        if( arrs==NULL ) {
            return false;
        }
        int sp=0, ep=0;
        StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        int size= sp<ep ? ep-sp: 0;
        StrVar* send=sendHttpHeader(conn,conn->xsend.reuse(),(qint64)size,"local");
        if( size > 0 ) {
            send->add(sv->get(sp), size);
        }
        conn->send(send);
        initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
    } break;

    case 49: {		// sendStream
        LPCC path=AS(0);
        StrVar* sv=arrs->get(1);
        qint64 start=0, end=0;
        MessageClient* client=NULL;
        StrVar* proto=NULL;
        bool bClose=false;
        if( isNumberVar(sv) ) {
            // ex) sendStream(path, start, end, client, protocol, bclose );
            // conn->GetVar("bufferLength")->reuse()->add(sv);
            start=toUL64(sv);
            sv=arrs->get(2);
            if( isNumberVar(sv) ) {
                end=toUL64(sv);
            }            
            sv=arrs->get(3);
            if(SVCHK('v',1)) {
                client=(MessageClient*)SVO;
                proto=arrs->get(4);
            }
            sv=arrs->get(5);
            if( SVCHK('3',1)) {
                bClose=true;
            }
        } else {
            // ex) sendStream(path, header, client, protocol, bclose );
            sv=arrs->get(1);
            readHttpHeader(conn, rst, sv);
            sv=arrs->get(2);
            if(SVCHK('v',22)) {
                client=(MessageClient*)SVO;
                proto=arrs->get(3);
            }
            sv=arrs->get(4);
            if( SVCHK('3',1)) {
                bClose=true;
            }
        }
        if(slen(path) ) {
            sendWebStream(conn, path, start, end, rst, fn, client, proto, bClose);
        }
    }
    case 50: {		// reqHeaders
        XListArr* arr=getListArrTemp();
        for( WBoxNode* bn=conn->First(); bn; bn=conn->Next() ) {
            LPCC code=conn->getCurCode();
            if( code[0]=='@' ) continue;
            arr->add()->add(code);
        }
        rst->setVar('a',0,(LPVOID)arr);
    } break;
    case 51: {		// getValue
        // ex) req.getValue() or req.getValue("Cookie");
        if(arrs==NULL ) {
            rst->reuse();
            for( WBoxNode* bn=conn->First(); bn; bn=conn->Next() ) {
                LPCC code=conn->getCurCode();
                if( code[0]=='@' ) continue;
                if( rst->length() ) rst->add(",\n");
                rst->add(code).add("=").add(toString(conn->gv(code)) );
            }
            return true;
        }
        LPCC code=AS(0);
        rst->add(toString(conn->gv(code)) );
    } break;
    case 52: {		// setValue
        if(arrs==NULL ) {
            return true;
        }
        LPCC code=AS(0);
        StrVar* sv = arrs->get(1);
        bool append=false;
        if( isVarTrue(arrs->get(2)) ) {
            rst=conn->GetVar(code);
            append=true;
            if( rst->length() ) rst->add('\f');
        } else {
            rst->reuse();
        }
        if( append ) {
            rst->add(toString(sv));
        } else {
            rst->add(toString(sv));
            conn->GetVar(code)->reuse()->add(rst);
        }
        qDebug("...req setValue %s\n", rst->get());
    } break;
    case 53: {		// database
        TreeNode* cf=getTreeNode("was","main");
        if( cf==NULL ) {
            ConnectNode* parent=conn->parent();
            cf=parent? parent->config(): NULL;
            if( cf==NULL ) {
                qDebug("#9# database not define (was is null)\n");
                return true;
            }
        }
        LPCC dsn=NULL, driver=NULL;
        if( arrs ) {
            dsn=AS(0);
        }
        gMutexDatabase.EnterMutex();
        XListArr* a=cf->getArray("@databasePool");
        if( a==NULL ) {
            qDebug("#9# database pools not define (data pool size==0)\n");
            return true;
        }
        TreeNode* model=NULL;
        for( int n=0; n<a->size(); n++ ) {
            sv=a->get(n);
            if( SVCHK('n',0) ) {
                TreeNode* cur=(TreeNode*)SVO;
                if( cur->isNodeFlag(FLAG_USE) ) {
                    if(slen(dsn)==0) dsn=cur->get("dsn");
                    qDebug("#0#database used idx:%d dns:%s\n", n, cur->get("dsn") );
                    continue;
                }
                if( dsn ) {
                    if( !ccmp(dsn,cur->get("dsn")) ) {
                        continue;
                    }
                }
                model=cur;
                break;
            }
        }
        qDebug("database dsn:%s", dsn);
        if( model==NULL && slen(dsn)  ) {
            int idx=a->size()+1;
            LPCC inode=gBuf.fmt("%s_%d", dsn, idx );
            model=getTreeNode("dbms", inode);
            model->setNodeFlag(FLAG_USE);
            getModelDatabase(dsn, inode, model);
            if( model==NULL) {
            }
        }
        if( model ) {
            qDebug("web database set dsn:%s %s", dsn, model->get("inode") );
            model->setLong("@startTick", GetTickCount() );
            model->setNodeFlag(FLAG_USE);
            conn->GetVar("@database")->setVar('n',0,(LPVOID)model);
            rst->setVar('n',0,(LPVOID)model );
        } else {
            qDebug("web database not set driver:%s", driver );
        }
        gMutexDatabase.LeaveMutex();
    } break;
    case 54: {		// setCookie
        if( arrs==NULL ) {
            return true;
        }
        LPCC val=AS(0);
        if( slen(val) ) {
            StrVar* var=conn->GetVar("@setCookies");
            if( var->length() ) {
                var->add("\n");
            }
            var->add(val);
        }
    } break;
    case 60: {		// flagState
        int cnt =arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(conn->xflag);
            return true;
        }
        U32 flag = (U32)toInteger(arrs->get(0));
        if( cnt==1 ) {
            if( flag ) {
                rst->setVar('3',conn->isNodeFlag((U32)flag) ? 1: 0);
            } else {
                // ### version 1.0.4 node->xflag=0;
                rst->setVar('3',0);
            }
        } else if( cnt==2 ) {
            bool chk=isVarTrue(arrs->get(1));
            if( chk )
                conn->setNodeFlag(flag);
            else
                conn->clearNodeFlag(flag);
        } else if( cnt==3 ) {
            conn->xflag=flag;
        }
    } break;
    case 61: {		// redirect
        int cnt =arrs ? arrs->size(): 0;
        LPCC url=NULL;
        if( cnt==0 ) {
            url="/";
        } else {
            url=AS(0);
        }
        if( slen(url) ) {
            rst->reuse();
            rst->format(1024,"HTTP/1.1 302\r\nDate: %s\r\nServer: BaroWas\r\nLocation: %s\r\n",
                 httpDate(conn->GetVar("@date")), url
            );
            rst->add("Content-Type: text/html;charset=UTF-8\r\nContent-Length: 0\r\n\r\n");
            conn->send(rst->get(), rst->length() );
            conn->closeClient();
        }
    } break;

    case 62: {		// webEval
        if( arrs==NULL) return true;
        TreeNode* pageNode=NULL;
        XFuncNode* fnInit=fn;
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            pageNode=(TreeNode*)SVO;
        } else {
            sv=fn->gv("@this");
            if( SVCHK('n',0) ) {
                pageNode=(TreeNode*)SVO;
            }
        }
        if(pageNode==NULL) pageNode=tn;
        if(fnInit==NULL) {
            sv=pageNode->gv("onInit");
            if( SVCHK('f',0) ) {
                fnInit=(XFuncNode*)SVO;
            }
        }
        int sp=0, ep=0;
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        if( sp<ep) {
            LPCC incName=FMT("webEval%d",ep);
            qDebug("#0# webEval call start ========== %s (%d,%d)\n", incName, sp, ep);
            makeWebEvalText(sv, sp, ep, conn, pageNode, fnInit, rst, incName );
        }
    } break;
    case 63: {		// getVar
        StrVar* var=NULL;
        if(arrs==NULL) {
            var=&(conn->xcontent);
        } else {
            LPCC code=AS(0);
            if(ccmp(code,"content")) var=&(conn->xcontent);
            else if(ccmp(code,"content")) var=&(conn->xsend);
            else var=&(conn->xcontent);
        }
        if(var) {
            rst->add(var);
        }
    } break;
    case 64: {		// recvWeb        
        if(arrs==NULL ) {
            return true;
        }
        sv=arrs->get(0);
        if(SVCHK('v',1)) {
            ConnectNode* client=(ConnectNode*)SVO;
            recvWebProxy(conn, rst, fn, client, arrs->get(1), isVarTrue(arrs->get(2)));
        }
    } break;
    case 65: { // proxySend
        if(arrs==NULL) {
            qDebug("#0#proxy send mapping error \n");
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            if(node->cmp("tag","file")) {
                sv=node->GetVar("@c");
                if( SVCHK('z',1) ) {
                    QFile* file=(QFile*)SVO;
                    int fsize=(int)file->size(), read=0;
                    StrVar* varTotal=getStrVarTemp();
                    varTotal->setVar('0',0).addInt(fsize);
                    if( parseTextFormat(arrs->get(1),0,0,rst,varTotal) ) {
                        rst->add("\n");
                        conn->send(rst);
                    }
                    char buf[4096];
                    while( fsize>0 && conn->valid() ) {
                        read=file->read(buf, 4096);
                        if(read<=0) break;
                        conn->send(buf,read);
                        fsize-=read;
                    }
                    file->close();
                }
            }
            return true;
        }
        sv=arrs->get(0);
        if(SVCHK('v',1)) {
            // req.proxySend(web, [true|false], "proxyRecv:$mapId,{0}");
            ConnectNode* web=(ConnectNode*)SVO;
            StrVar* var=NULL;
            sv=arrs->get(1);
            if(SVCHK('3',1)) {
                var=web->read();
            } else if(SVCHK('3',0)) {
                var=web->getContentVar();
            }
            int contentLen=0;
            int pos=var->findPos("\r\n\r\n");
            if(pos>0 ) {
                int sp=var->findPos("Content-Length: ",0,pos);
                if(sp>0 ) {
                    int ep=var->findPos("\n",sp, pos);
                    if(sp<ep) {
                        LPCC val=var->trimv(sp,ep);
                        if(isnum(val)) {
                            contentLen=atoi(val);
                        }
                    }
                }
                pos+=4;
            }
            if(contentLen>0 ) {
                int remain=0, varLen=var->length(), len=varLen-pos;
                int total=pos+contentLen, sum=varLen;
                if( contentLen>len) {
                    remain=contentLen-len;
                }
                StrVar* varTotal=getStrVarTemp();
                varTotal->setVar('0',0).addInt(total);
                if( parseTextFormat(arrs->get(2),0,0,rst,varTotal) ) {
                    rst->add("\n");
                    conn->send(rst);
                }
                XSocket* sock=web->xsock;
                char buf[SOCK_BUF_SIZE];
                conn->send(var->get(), varLen);
                qDebug(".... proxySend (start)==> contentLen:%d varLen:%d, total:%d header:%s ...\n",contentLen, varLen, total, rst->get() );
                while(remain>0 && web->valid() ) {
                    int recv=sock->Receive(buf, SOCK_BUF_SIZE);
                    if(recv>0 ) {
                        int send=conn->send(buf, recv);
                        remain-=send;
                        sum+=send;
                    } else {
                        break;
                    }
                }
                qDebug(".... proxySend (end) ==> total:%d sum:%d, remain:%d ...\n", total, sum, remain);
            } else {
                qDebug("#0#proxySend content length not valid (header:%s)\n", var->get() );
            }
        } else {
            qDebug("#0#proxySend object not valid (object:%s)\n", toString(sv) );
        }
    } break;
    case 66: { // proxyRecv
        if(arrs==NULL ) {
            return true;
        }
        // ex) proxyRecv(object, data, total, header);
        int sp=0,ep=0;
        StrVar* sv=arrs->get(0);
        StrVar* var=getStrVarPointer(arrs->get(1), &sp, &ep);
        int dataSize=ep-sp;
        int totalSize=toInteger(arrs->get(2));
        rst->reuse();
        qDebug(".... proxyRecv (0) ==> totalSize:%d, dataSize:%d ...\n", totalSize, dataSize);
        if( totalSize==0 || dataSize==0 ) {
            return true;
        }
        int remain=totalSize>dataSize ? totalSize-dataSize: 0;
        int sum=dataSize;
        StrVar* varTotal=getStrVarTemp();
        varTotal->setVar('0',0).addInt(totalSize);
        if( parseTextFormat(arrs->get(3),0,0,rst,varTotal) ) {
            rst->add("\n");
        }
        qDebug(".... proxyRecv (1) ==> totalSize:%d, reamin:%d, header:%s ...\n", totalSize, remain, rst->get() );
        if(SVCHK('v',1) ) {
            ConnectNode* web=(ConnectNode*)SVO;
            if(rst->length()) web->send(rst);
            web->send( var->get(sp), dataSize );
            if( remain>0 ) {
                XSocket* sock=conn->xsock;
                char buf[SOCK_BUF_SIZE];
                qDebug(".... proxyRecv (2)==> totalSize:%d, sum:%d, reamin:%d  ...\n", totalSize, sum, remain );
                while(remain>0 && web->valid() && conn->valid() ) {
                    int recv=sock->Receive(buf, SOCK_BUF_SIZE);
                    if(recv>0 ) {
                        int send=web->send(buf, recv);
                        remain-=send;
                        sum+=send;
                    } else {
                        break;
                    }
                }
            }
            qDebug(".... proxyRecv (3) ==> totalSize:%d, sum:%d, reamin:%d  ...\n", totalSize, sum, remain  );
        } else if(SVCHK('v',22)) {
            MessageClient* client=(MessageClient*)SVO;
            if(rst->length()) client->sendVar(rst);
            client->sendBuffer(var->get(sp), dataSize );
            while(remain>0) {
                conn->setNodeFlag(FLAG_USE);
                if( conn->read(rst->reuse()) ) {
                    client->sendVar(rst);
                    remain-=rst->length();
                } else {
                    break;
                }
            }
        } else if(SVCHK('n',0)) {
            TreeNode* node=(TreeNode*)SVO;
            if(node->cmp("tag","file")) {
                sv=node->GetVar("@c");
                if( SVCHK('z',1) ) {
                    QFile* file=(QFile*)SVO;
                    file->write(var->get(sp), dataSize );
                    while(remain>0) {
                        conn->setNodeFlag(FLAG_USE);
                        if( conn->read(rst->reuse()) ) {
                            file->write(rst->get(), rst->length());
                            remain-=rst->length();
                        } else {
                            break;
                        }
                    }
                }
            } else {
                XFuncNode* fnInit=NULL;
                ConnectNode* web=NULL;
                sv=node->gv("onInit");
                if(SVCHK('f',0)) {
                    fnInit=(XFuncNode*)SVO;
                    sv=fnInit->gv("webClient");
                    if(SVCHK('n',0)) {
                        TreeNode* webClient=(TreeNode*)SVO;
                        sv=webClient->gv("@c");
                        if(SVCHK('v',1)) {
                            web=(ConnectNode*)SVO;
                        }
                    } else if(SVCHK('v',1)) {
                        web=(ConnectNode*)SVO;
                    }
                }
                if(web ) {
                    if(rst->length()) web->send(rst);
                    web->send( var->get(sp), dataSize );
                    while(remain>0) {
                        conn->setNodeFlag(FLAG_USE);
                        if( conn->read(rst->reuse()) ) {
                            web->send(rst);
                            remain-=rst->length();
                        } else {
                            break;
                        }
                    }
                }
            }
        }
        if(remain>0 ) {
            rst->setVar('3',0);
            qDebug("#0#socket proxyRecv error (remain:%s)\n", remain);
        } else {
            rst->setVar('3',1);
        }

    } break;
    default: return false;
    }
    return true;
}

bool callMessageServerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    StrVar* sv=tn->GetVar("@c");
    MessageServer* server= NULL;
    if( SVCHK('v',21) ) {
        server=(MessageServer*)SVO;
    } else {
        server=new MessageServer(tn);
        sv->setVar('v',21,(LPVOID)server);
    }
    int cnt=arrs ? arrs->size():0;
    if( ccmp(fnm,"start") ) {
        if( arrs==NULL ) {
            rst->setVar( '3', server->isListening()? 1: 0 );
            return true;
        }
        XFuncNode* fnCur=NULL;
        StrVar* sv=tn->gv("@error");
        if( sv ) {
            sv->reuse();
        }
        int port=isNumberVar(arrs->get(0)) ? toInteger(arrs->get(0)): 8090;
        server->StartServer(port);
        sv=tn->gv("@error");
        if( sv && sv->length() ) {
            rst->setVar('3',0 );
            return true;
        }
        sv=arrs->get(1);
        if( SVCHK('f',1) ) {            
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            fnCur=setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
            fnCur->GetVar("@params")->setVar('a',0,(LPVOID)&(server->xparam) );
            sv= arrs->get(2);
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
                tn->GetVar("@callbackClient")->setVar('f',1,(LPVOID)fsrc);
                rst->setVar('f',1,(LPVOID)fsrc);
            }
        }
        rst->setVar('3',1);
    } else if( ccmp(fnm,"mapId") ) {
        if(arrs==NULL ) {
            return true;
        }
        StrVar* sv= arrs->get(0);
        MessageClient* client=NULL;
        if( SVCHK('v',22) ) {
            client=(MessageClient*)SVO;
        } else if(SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            sv=node->gv("@c");
            if( SVCHK('v',22) ) {
                client=(MessageClient*)SVO;
            }
        } else {
            LPCC mapId=toString(sv);
            sv=tn->gv(mapId);
            if(SVCHK('n',0)) {
                TreeNode* node=(TreeNode*)SVO;
                rst->setVar('n',0,(LPVOID)node);
            } else {
                qDebug("#0#server mapId find error (mapId:%s)\n", mapId );
            }
            return true;
        }
        if(client ) {
            TreeNode* clientConfig=client->xcf;
            int idx=tn->incrNum("clientMapIndex") % 256;
            LPCC mapId=FMT("clientMap_%d",idx);
            tn->GetVar(mapId)->setVar('n',0,(LPVOID)clientConfig);
            rst->set(mapId);
        } else {
            qDebug("#0#server mapId error (map client not define node:%s)\n", toString(sv) );
        }
    } else if( ccmp(fnm,"callbackClient") ) {
        if( arrs==NULL ) return true;
        StrVar* sv= arrs->get(0);
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            tn->GetVar("@callbackClient")->setVar('f',1,(LPVOID)fsrc);
            rst->setVar('f',1,(LPVOID)fsrc);
        }
    } else if( ccmp(fnm,"stop") ) {
        server->StopServer();
    } else if( ccmp(fnm,"callback") ) {
        StrVar* sv=NULL;
        TreeNode* tn=server->xcf;
        if( arrs==NULL ) {
            sv=tn->gv("@callback");
            rst->reuse();
            if( SVCHK('f',0) ) {
                rst->add(sv);
            }
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
            fnCur->GetVar("@params")->setVar('a',0,(LPVOID)&server->xparam );
        }
    } else if( ccmp(fnm,"is") ) {
        LPCC ty=arrs ? AS(0): "listen";
        rst->reuse();
        if( ccmp(ty,"listen") ) {
            rst->setVar('3', server->isListening()? 1: 0 );
        } else if( ccmp(ty,"error") ) {
            QString err=server->errorString();
            if( !err.isNull() ) {
                rst->set(Q2A(err));
            }
        }
    } else if( ccmp(fnm,"maxPedding") ) {
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(server->maxPendingConnections() );
        } else {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                server->setMaxPendingConnections(toInteger(sv) );
            }
        }
    } else {
        return false;
    }
    return true;
}


inline bool clientLockCheck(TreeNode* tn, StrVar* rst, LPCC type) {
    /*
    StrVar* sv=tn->GetVar("@lockFlag");
    if( SVCHK('3',1) ) {
        bool block=true;
        qDebug("#0#clientLockCheck start type:%s----------------------\n", type );
        for( int n=0; n<4; n++ ) {
            FThread::Sleep(200);
            sv=tn->GetVar("@lockFlag");
            if( SVCHK('3',1) ) {
                continue;
            } else {
                block=false;
                break;
            }
        }
        if( block ) {
            printBox(tn, rst);
            qDebug("#0#client %s lock : %s\n", type, rst->get() );
            return true;
        }
    }
    */
    return false;
}

inline void swapByteOrder(unsigned short& us) {
    /*
    short big = 0xdead;
    short little = (((big & 0xff)<<8) | ((big & 0xff00)>>8));
    */
    us = (us >> 8) |
         (us << 8);
}

inline void swapByteOrder(unsigned int& ui) {
    /*
        unsigned char *ptr = (unsigned char *)&x;
        return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    */
    ui = (ui >> 24) |
         ((ui<<8) & 0x00FF0000) |
         ((ui>>8) & 0x0000FF00) |
         (ui << 24);
}

inline void swapByteOrder(unsigned long long& ull) {
    ull = (ull >> 56) |
          ((ull<<40) & 0x00FF000000000000) |
          ((ull<<24) & 0x0000FF0000000000) |
          ((ull<<8) & 0x000000FF00000000) |
          ((ull>>8) & 0x00000000FF000000) |
          ((ull>>24) & 0x0000000000FF0000) |
          ((ull>>40) & 0x000000000000FF00) |
          (ull << 56);
}

bool callMessageClientFunc(StrVar* me, XFunc* fc, LPCC funcName, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    MessageClient* conn=NULL;
    if( checkFuncObject(me,'v',22) ) {
        conn=(MessageClient*)me->getObject(FUNC_HEADER_POS);
    } else {        
        if( tn==NULL ) {
            return false;
        }
        conn=new MessageClient(tn, NULL, 0);
        tn->GetVar("@c")->setVar('v',22,(LPVOID)conn );
    }
    // qDebug("message client func name==%s ref:%d\n", funcName, fc->xfuncRef );
    if( conn==NULL ) {
        return tn==NULL? true: false;
    }
    bool bsub=false;
    if( tn==NULL ) {
        tn=conn->xcf;
        bsub=true;
    }
    if( fc->xfuncRef==0 ) {
        LPCC fnm=funcName? funcName : fc->getFuncName();
        fc->xfuncRef= ccmp(fnm, "send")? 201:  ccmp(fnm, "is")? 203:
            ccmp(fnm, "close")? 204:
            ccmp(fnm, "abort")? 205:
            ccmp(fnm, "peerAddress")? 206:
            ccmp(fnm, "peerName")? 207:
            ccmp(fnm, "option")? 208:
            ccmp(fnm, "callback")? 209:
            ccmp(fnm, "sendWs") ? 210:
            ccmp(fnm, "readWs")? 211:
            ccmp(fnm, "readAll")? 212:
            ccmp(fnm, "sendData")? 213:
            ccmp(fnm, "config")? 214:
            ccmp(fnm, "connect")? 215:
            ccmp(fnm, "sendVar")? 216:
            ccmp(fnm, "proxySend")? 217:
            ccmp(fnm, "proxyRecv")? 218:
            ccmp(fnm, "flush")? 219:
            0;
    }
    switch( fc->xfuncRef ) {
    case 201:	{
        if( arrs==NULL ) return true;
        QTcpSocket* sock=conn->Socket;
        StrVar* sv=NULL;
        bool ok=false;
        if( sock && sock->state()==QAbstractSocket::ConnectedState ) {
            QByteArray ba;
            int sp=0,ep=0;
            for(int n=0; n<arrs->size(); n++ ) {
                sv=getStrVarPointer(arrs->get(n), &sp, &ep );
                if( sp<ep ) {
                    ba.append(sv->get(sp), ep-sp );
                }
            }
            int size=ba.size();
            if( size>0 ) {
                int writeSize=(int)sock->write(ba);
                if( size==writeSize ) {
                    ok=true;
                } else {
                    qDebug("#0#message socket write size error (buffer size=%d, write size=%d)\n", size, writeSize );
                }
                if( writeSize==0 && conn->Socket ) {
                    conn->Socket->abort();
                }
                tn->GetVar("@prevSendTick")->setVar('1',0).addUL64(GetTickCount());
            }
        }
        if( ok ) {
            // sock->flush();
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
        // tn->GetVar("@lockFlag")->setVar('3',0);
    } break;

    case 203: { // is
        if( arrs==NULL ) {
            rst->set("vaild,connecting, connect, bound, closing, state, error, lock");
            return true;
        }
        LPCC ty=AS(0);
        QTcpSocket* sock=conn->Socket;
        if( sock ) {
            if( ccmp(ty,"valid") ) {
                rst->setVar('3', sock->isValid() ? 1: 0 );
            } else if( ccmp(ty,"lock") ) {
                bool lock=false;
                StrVar* sv=tn->gv("@lockFlag");
                if( SVCHK('3',1) ) {
                    lock=true;
                } else {
                    sv=tn->gv("@lock");
                    if( SVCHK('3',1) ) {
                        lock=true;
                    }
                }
                if( lock==false ) {
                    sv=tn->gv("@prevRecvTick");
                    if( SVCHK('1',0) ) {
                        DWORD prev=(DWORD)sv->getUL64(FUNC_HEADER_POS), dist=GetTickCount()-prev;
                        if( dist<100 ) {
                            qDebug("#0#messageClient recv lock check (dist:%d)\n", dist );
                            lock=true;
                        }
                    }
                }
                if( lock==false ) {
                    sv=tn->gv("@prevSendTick");
                    if( SVCHK('1',0) ) {
                        DWORD prev=(DWORD)sv->getUL64(FUNC_HEADER_POS), dist=GetTickCount()-prev;
                        if( dist<100 ) {
                            qDebug("#0#messageClient send lock check (dist:%d)\n", dist );
                            lock=true;
                        }
                    }
                }
                rst->setVar('3', lock ? 1: 0 );
            } else if( ccmp(ty,"connecting") ) {
                rst->setVar('3', sock->state()==QAbstractSocket::ConnectingState ? 1: 0 );
            } else if( ccmp(ty,"connect") ) {
                rst->setVar('3', sock->state()==QAbstractSocket::ConnectedState ? 1: 0 );
            } else if( ccmp(ty,"bound") ) {
                rst->setVar('3', sock->state()==QAbstractSocket::BoundState ? 1: 0 );
            } else if( ccmp(ty,"closing") ) {
                rst->setVar('3', sock->state()==QAbstractSocket::ClosingState ? 1: 0 );
            } else if( ccmp(ty,"state") ) {
                rst->reuse();
                if( sock->isOpen() && sock->isValid() ) {
                    switch( sock->state() ) {
                    case QAbstractSocket::UnconnectedState:	break;
                    case QAbstractSocket::ConnectingState:	rst->add("connecting"); break;
                    case QAbstractSocket::ConnectedState:	rst->add("connect"); break;
                    case QAbstractSocket::BoundState:		rst->add("bound"); break;
                    case QAbstractSocket::ClosingState:		rst->add("closing"); break;
                    }
                }
            } else if( ccmp(ty,"error") ) {
                QString err=sock->errorString();
                if( err.isNull() || err.isEmpty() ) {
                    qDebug("message socket check (state:%d)\n", sock->state() );
                } else {
                    rst->set(Q2A(err));
                }
            }
        }
    } break;
    case 204: {
        StrVar* sv=tn->gv("@lockFlag");
        if( sv ) {
            sv->setVar('3',0);
        }
        conn->CloseSocket();
    } break;
    case 205: {
        StrVar* sv=tn->gv("@lockFlag");
        if( sv ) {
            sv->setVar('3',0);
        }
        if( conn->Socket ) {
            conn->Socket->abort();
        }
    } break;
    case 206: {
        if( conn->Socket ) {
            QString peer=conn->Socket->peerAddress().toString();
            rst->set(Q2A(peer));
        }
    } break;
    case 207: {
        if( conn->Socket ) {
            QString peer=conn->Socket->peerName();
            rst->set(Q2A(peer));
        }
    } break;
    case 209: {		// callback
        StrVar* sv=NULL;
        if( arrs==NULL ) {
            sv=tn->gv("@callback");
            rst->reuse();
            if( SVCHK('f',0) ) {
                rst->add(sv);
            }
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn,tn,NULL,tn->GetVar("@callback"));
            fnCur->GetVar("@params")->setVar('a',0,(LPVOID)&conn->xparam );
        } else {
            sv=tn->gv("@callback");
            if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                XListArr* param=&(conn->xparam);
                param->reuse();
                for( int n=1,sz=arrs->size(); n<sz; n++ ) {
                    param->add()->add(arrs->get(n));
                }
                fnCur->call(NULL, rst->reuse() );
                param->reuse();
            }
        }
    } break;
    case 210: { // sendWs
        if( arrs==NULL ) {
            return true;
        }
        if( clientLockCheck(tn, rst, "send web socket") ) {
            return true;
        }
        StrVar* sv=NULL;
        QTcpSocket* sock=conn->Socket;
        bool ok=false;
        if( sock && sock->state()==QAbstractSocket::ConnectedState ) {
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(0), &sp, & ep);
            if( sp<ep ) {
                char buf[4];
                int len=ep-sp;
                buf[0]=(char)0x81;
                if( len>126 ) {
                    buf[1]=(char)126;
                } else {
                    buf[1]=(char)len;
                }
                buf[2]=0;
                rst->add(buf,2);
                if( len>126 ) {
                    unsigned short sz=(unsigned short&)len;
                    swapByteOrder(sz);
                    rst->add((char*)&sz, sizeof(sz) );
                }
                rst->add(sv->get(sp), len);
                int writeSize=sock->write(rst->get(), rst->length() );
                if( writeSize>0 ) {
                    ok=true;
                } else  {
                    sock->abort();
                }
            }
        } else {
            qDebug("#0#socket sendWs error (not conntect)");
        }
        if( ok ) {
            rst->setVar('3',1);
            sock->flush();
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 211: { // readWs
         QTcpSocket* sock=conn->Socket;
        if( sock && sock->state()==QAbstractSocket::ConnectedState ) {
            char buf[1024];
           sock->read(buf,1);
            int num=(int)((U8)buf[0]);
            int fin=(0xFF&num)>>7, mask=0, opcode=0x0F & num;
            sock->read(buf,1);
            num=(int)((U8)buf[0]);
            mask=(0xFF&num)>>7;
            qint64 len=0x7F & num;
            qDebug("-- readWs -- fin=%d, mask=%d, opcode=%d, len=%d\n", fin, mask, opcode, (int)len );
            if( opcode< 3 ) {
                LPCC maskKey=NULL;
                if( len==126 ) {
                    U16 sz=0;
                    sock->read((char*)&sz,2);
                    swapByteOrder((unsigned short&)sz);
                    len=(qint64)sz;
                    qDebug("-- len=%d -- 126\n",  (int)len );
                } else if( len==127 ) {
                    sock->read((char*)&len,8);
                    swapByteOrder((unsigned long long&)len);
                    qDebug("-- len=%d -- 127\n",  (int)len );
                }
                if( mask==1 ) {
                    sock->read(buf,4);
                    maskKey=gBuf.add(buf,4);
                }
                qint64 total=len;
                rst->reuse();
                while( total>0 ) {
                    int remain=total>1024 ? 1024: total;
                    int read=sock->read(buf, remain);
                    if( read<=0 ) break;
                    rst->add(buf, read);
                    total-=read;
                }
                if( maskKey ) {
                    char* tmp=rst->get();
                    for( int n=0, sz=rst->length(); n<sz; n++ ) {
                        tmp[n]^=maskKey[n%4];
                    }
                }
                if( opcode!=0x1 ) {
                    qDebug("#0#web socket recv error code=%d, len=%d\n", opcode, (int)len );
                }
            } else if( opcode==0x8 ) {
                qDebug("#0#web socket close code=%d\n", opcode);
            } else {
                qDebug("#0#web socket recv error code=%d\n", opcode);
            }
        } else {
            qDebug("#0#socket recvWs error (not conntect)");
        }
    } break;
    case 212: { // readAll
        QTcpSocket* sock=conn->Socket;
        qDebug("#0#read all client 0\n");
        if( sock && sock->state()==QAbstractSocket::ConnectedState ) {
            QByteArray data = sock->readAll();
            rst->add(data.constData(), data.length() );
            qDebug("#0#read all client (size:%d)\n", data.length() );
        } else {
            qDebug("#0#socket readAll error (not conntect)");
        }
    } break;
    case 213: { // sendData
        QTcpSocket* sock=conn->Socket;
        if( clientLockCheck(tn, rst, "send data") ) {
            return true;
        }
        tn->GetVar("@lockFlag")->setVar('3',1);
        qDebug("#0#send data client 0\n");
        if( arrs && sock && sock->state()==QAbstractSocket::ConnectedState ) {
            int sp=0,ep=0;
            StrVar* sv=getStrVarPointer(arrs->get(0), &sp, &ep);
            int size=ep-sp;
            while( size>0 ) {
                int remain=size>2048 ? 2048: size;
                int write=(int)sock->write(sv->get(sp), remain );
                if( write<=0 ) break;
                sp+=write;
                size-=write;
            }
            sock->flush();
            qDebug("#0#send data client (sp:%d)\n", sp);
        } else {
            qDebug("#0#socket sendData error (not conntect)");
        }
        tn->GetVar("@lockFlag")->setVar('3',0);
    } break;
    case 214: { // config
        if( conn->xcf ) {
            rst->setVar('n',0,(LPVOID)conn->xcf );
        }
    } break;
    case 215: { // connect
        QTcpSocket* sock=conn->Socket;
        if( arrs==NULL ) {
            rst->setVar('3', sock && sock->isOpen()? 1: 0 );
            return true;
        }

        if( sock && sock->state()==QAbstractSocket::UnconnectedState ) {
            LPCC host=AS(0);
            int port=8090;
            /*
            if( isVarTrue(arrs->get(2)) ) {
                sock->setProxy(QNetworkProxy::NoProxy);
            }
            */
            if( isNumberVar(arrs->get(1)) ) {
                port=toInteger(arrs->get(1));
            }
            if( arrs->get(2) ) {
                LPCC type=AS(2);
                if(slen(type) ) tn->set("@type", type);
            }
            if( ccmp(host,"localhost")) {
                sock->connectToHost(QHostAddress::LocalHost, port);
            } else {
                sock->connectToHost(KR(host),port);
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 216: { // sendVar
        if(arrs==NULL ) {
            return true;
        }
        if( conn ) {
            conn->sendVar(arrs->get(0));
        }
    } break;
    case 217: { // proxySend
        if(arrs==NULL) {
            qDebug("#0#proxy send mapping error \n");
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            if(node->cmp("tag","file")) {
                sv=node->GetVar("@c");
                if( SVCHK('z',1) ) {
                    QFile* file=(QFile*)SVO;
                    qint64 fsize=file->size(), read=0;
                    char buf[4096];
                    int sp=0, ep=0, num=0;
                    StrVar* var=getStrVarPointer(arrs->get(1), &sp, &ep);
                    StrVar* varTotal=getStrVarTemp();
                    varTotal->setVar('0',0).addInt((int)fsize);
                    if( parseTextFormat(arrs->get(1),0,0,rst,varTotal) ) {
                        rst->add("\n");
                        conn->sendVar(rst);
                    }
                    while( fsize>0 ) {
                        read=file->read(buf,4096);
                        if( read<=0 ) break;
                        if(num==0 && sp<ep ) {
                            XParseVar pv(var, sp, ep);
                            fn->setInt("contentLength", (int)fsize);
                            makeTextData(pv, fn, rst);
                            rst->add("\n");
                            rst->add(buf, read);
                            conn->sendBuffer(rst->get(), rst->length());
                        } else {
                            conn->sendBuffer(buf,read);
                        }
                        fsize-=read;
                    }
                    file->close();
                }
            }
            return true;
        }
        TreeNode* server=getTreeNode("server", "UserConnectServer", false);
        MessageClient* client=NULL;
        LPCC mapId=AS(0), proxyId=AS(1);
        // proxySend( mapId, proxyId);
        if(server && slen(mapId) && slen(proxyId) ) {
            sv=server->gv(mapId);
            if(SVCHK('n',0)) {
                TreeNode* clientConf=(TreeNode*)SVO;
                sv=clientConf->gv("@c");
                if(SVCHK('v',22)) {
                    client=(MessageClient*)SVO;
                }
            }
        } else {
            qDebug("#0#proxy send server find error (mapId:%s, proxyId:%s) \n", mapId, proxyId);
            return true;
        }
        if( client==NULL ) {
            qDebug("#0#proxy send client error (mapId:%s, proxyId:%s) \n", mapId, proxyId);
            return true;
        }
        sv=client->xcf->gv(proxyId);
        if(SVCHK('v',1)) {
            ConnectNode* web=(ConnectNode*)SVO;
            StrVar* header=web->getContentVar();
            StrVar* content=web->getSendVar();
            int headerLen=header->length(), contentLen=web->getInt("@contentLength");
            int size=headerLen+contentLen;
            int remain=contentLen-content->length();
            rst->fmt("webProxy:%s,%s,%d\n", mapId, proxyId, size);
            rst->add(header);
            if(content->length() ) rst->add(content);
            conn->sendBuffer(rst->get(), rst->length() );
            if(remain>0 ) {
                char buf[SOCK_BUF_SIZE];
                while( web->xsock && remain>0 ) {
                    FTRY {
                        int recv = web->xsock->Receive(buf,SOCK_BUF_SIZE);
                        if( recv>0 ) {
                            conn->sendBuffer(buf, recv);
                            remain-=recv;
                        } else {
                            qDebug("#0#recv var error (size:%d, result:%d)\n", size, recv );
                            break;
                        }
                    } FCATCH( FSocketException, ex ) {
                        qDebug("#0#recv var error = %s\n", ex.Get() );
                        break;
                    }
                }
            }
            tn->GetVar("@webReq")->setVar('v',1,(LPVOID)web);
            qDebug("..... remain == %d ..... (%d, %d, size:%d) \n", remain, headerLen, contentLen, size);
        } else {
            qDebug("#0#proxy send web client error (mapId:%s, proxyId:%s) \n", mapId, proxyId);
        }
    } break;
    case 218: { // proxyRecv
        if(arrs==NULL ) {
            return true;
        }
        // ex) proxyRecv(object, data, size, header);
        int sp=0,ep=0;
        StrVar* sv=arrs->get(0);
        StrVar* var=getStrVarPointer(arrs->get(1), &sp, &ep);
        int dataSize=ep-sp;
        rst->reuse();
        if( dataSize>0  ) {
            int totalSize=toInteger(arrs->get(2));
            int remain=totalSize>dataSize ? totalSize-dataSize: 0;            
            StrVar* varTotal=getStrVarTemp();
            varTotal->setVar('0',0).addInt(totalSize);
            if( parseTextFormat(arrs->get(3),0,0,rst,varTotal) ) {
                rst->add("\n");
            }
            if(SVCHK('v',1) ) {
                ConnectNode* web=(ConnectNode*)SVO;
                if(rst->length()) web->send(rst);
                web->send( var->get(sp), dataSize );
            } else if(SVCHK('v',22)) {
                MessageClient* client=(MessageClient*)SVO;
                client->sendBuffer(var->get(sp), dataSize );
                if(rst->length()) client->sendVar(rst);
            } else if(SVCHK('n',0)) {
                TreeNode* node=(TreeNode*)SVO;
                if(node->cmp("tag","file")) {
                    sv=node->GetVar("@c");
                    if( SVCHK('z',1) ) {
                        QFile* file=(QFile*)SVO;
                        file->write(var->get(sp), dataSize );
                    }
                } else {
                    XFuncNode* fnInit=NULL;
                    ConnectNode* web=NULL;                    
                    sv=node->gv("onInit");
                    if(SVCHK('f',0)) {
                        fnInit=(XFuncNode*)SVO;
                        sv=fnInit->gv("webClient");
                        if(SVCHK('n',0)) {
                            TreeNode* webClient=(TreeNode*)SVO;
                            sv=webClient->gv("@c");
                            if(SVCHK('v',1)) {
                                web=(ConnectNode*)SVO;
                            }
                        } else if(SVCHK('v',1)) {
                            web=(ConnectNode*)SVO;
                        }
                    }
                    if(web ) {
                        node->GetVar("@webClient")->setVar('v',1,(LPVOID)web);
                        node->GetVar("@dataClient")->setVar('v',22,(LPVOID)conn);
                        if(rst->length()) web->send(rst);
                        web->send( var->get(sp), dataSize );
                    }
                    if(remain==0 ) {
                        XFuncNode* fn = getEventFuncNode(node, "onRead");
                        if(fn ) {
                            setFuncNodeParams(fn, NULL);
                            fn->call();
                        }
                    }
                }
            }
            StrVar* target=tn->GetVar("@targetObject")->reuse();
            if(remain>0 ) {
                tn->GetVar("@finishCheck")->setVar('3',0);
                target->add(sv);
            } else {
                tn->GetVar("@finishCheck")->setVar('3',1);
            }
            tn->setInt("@remainSize", remain);
        }
    } break;
    case 219: { // flush
        rst->setVar('3',0);
        QTcpSocket* sock=conn->Socket;
        if( sock ) {
            sock->flush();
        }
    } break;
    default: {
        if( bsub ) {
            if( fc->xfuncRef==0 ) {
                fc->xfuncRef = getHashKeyVal("node", fc->getFuncName() );
            }
            callExecNodeFunc(tn,fc,arrs,fn,me,rst);
            return true;
        }
        return false;
    }break;
    }
    return true;
}
