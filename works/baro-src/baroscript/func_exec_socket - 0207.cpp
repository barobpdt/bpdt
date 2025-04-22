#include "func_util.h"
#include "../util/user_event.h"
#include "../webserver/HttpNode.h"
#include "../webserver/ConnectNode.h"
#include "../fobject/FMutex.h"
// #include "../XCOM/XCOM.h"

FMutex	gMutexDatabase;
FMutex	gMutexSetEvent;

void regConnectNodeFuncs() {
    if( isHashKey("socket") )
        return;
    PHashKey hash = getHashKey("socket");
    U16 uid = 1;
    hash->add("param", uid);		uid++;	//
    hash->add("send", uid);			uid++;	//
    hash->add("isError", uid);       uid++;	//
    hash->add("isEnd", uid);        uid++;	//
    hash->add("socketClear", uid);	uid++;	//
    hash->add("sendFile", uid);		uid++;	//
    hash->add("sendBuffer", uid);	uid++;	//
    hash->add("waitConnect", uid);  uid++;	//
    hash->add("readBuffer", uid);	uid++;	//
    uid = 12;
    hash->add("sendPage", uid);		uid++;	//
    hash->add("readAll", uid);      uid++;	//
    hash->add("read", uid);         uid++;	//
    hash->add("sendAll", uid);		uid++;	//
    hash->add("flush", uid);		uid++;
    hash->add("peek", uid);         uid++;
	hash->add("skip", uid);         
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
    uid++;	// hash->add("children", uid);
    hash->add("recv", uid);		uid++;	//
    hash->add("client", uid);     uid++;	//
    hash->add("server", uid);	uid++;	//
    hash->add("sendHttp", uid);	uid++;	//
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
    hash->add("redirect", uid);     uid++;	//
    hash->add("webEval", uid);  	uid++;	//
    hash->add("getVar", uid);       uid++;	//
    hash->add("recvWeb", uid);      uid++;	//
    hash->add("proxySend", uid);	uid++;	//
    hash->add("proxyRecv", uid);	uid++;	//
    hash->add("template", uid);     uid++;	//
    hash->add("pageSource", uid);   uid++;	//
    hash->add("tickCount", uid);   uid++;	//
    hash->add("asyncMode", uid);   uid++;   // 비동기모드
    hash->add("sendProxy", uid);   uid++;   //
    hash->add("sendData", uid);   uid++;   //
    hash->add("recvData", uid);   uid++;   //
    hash->add("resetFlag", uid);   uid++;   //
    hash->add("setEvent", uid);   uid++;   //
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
        StrVar* sv=NULL;
        if( arrs==NULL ) {
            rst->set("layout data not found");
            if( sendHttpHeader(conn,conn->getSendVar(),(qint64)rst->length()) ) {
				conn->send(rst);
			}
        } else {
            int sp=0,ep=0;
			StrVar* var=conn->getSendVar();
			var->reuse();
            sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            int len = ep-sp;
			conn->setTick();
			conn->setInt("@state", 200);
            conn->set("Cache-Control", "no-cache");
            // tn->GetVar("@sendCheck")->setVar('3',1);
            if( len>0 ) {
                if( sendHttpHeader(conn,var,(qint64)len) ) {
                    int sz=conn->send((BYTE*)sv->get(sp), len);
                    rst->setVar('0',0).addInt(sz);
                    // qDebug("send http size LENGTH:%d SEND:%d ", len, sz);
                } else {
                    qDebug("send http header error (LENGTH:%d)",len);
                }
            } else {
                StrVar* var=cfVar("httpEmpty");
                if( var->length()==0 ) {
                    var->set("error: connectNode send size==0");
                }
                if( sendHttpHeader(conn,conn->getSendVar(),(qint64)var->length() ) ) {					
                    int sz=conn->send((BYTE*)var->get(), var->length() );
                    rst->setVar('0',0).addInt(sz);
				}
            }
        }
    } break;
    case 3: {		// isError
        rst->setVar('3', conn->isNodeFlag(FLAG_ERROR)?1:0);
        if(arrs ) {
            sv=arrs->get(0);
            if(SVCHK('3',1)) {
                conn->clearNodeFlag(FLAG_ERROR);
            }
        }
    } break;
    case 4: {		// isEnd
        rst->setVar('3', conn->isNodeFlag(FLAG_END)?1:0);
    } break;
    case 5: {		// socketClear
        if(arrs==NULL) {

        } else {
            for(int n=0;n<arrs->size();n++) {
                LPCC name=AS(n);
                if(slen(name)) {
                    sv=conn->gv(name);
                    if(sv) {
                        qDebug("clear name==%s", name);
                        sv->reuse();
                    }
                }
            }
        }
    } break;
    case 6: {		// sendFile
		if(arrs==NULL) return true;
        LPCC path=AS(0);
        QFile file(KR(path));
        XSocket* sock=conn->xsock;
        conn->setNodeFlag( FLAG_RUN | WOKRER_DOWNLOAD );
        if( file.open(QIODevice::ReadOnly) && sock ) {
            int pos= LastIndexOf(path,'.');
            LPCC ext = pos>0 ? path+pos+1: NULL;
            LPCC filenm = NULL;
            sv=arrs->get(1);
            if(SVCHK('3',1)) {                
                pos= LastIndexOf(path,'/');            
                if(pos!=-1 ) filenm=path+pos+1;
            } else if(sv) {
                filenm=toString(sv);
            }
            if(slen(filenm)) {
                sv=conn->GetVar("@attachHeader");
                sv->add("Content-Disposition: attachment; filename=\"").add(filenm).add("\"\r\n");
            }
            qint64 fsize=file.size(), r=0, w=0;
            StrVar* sendCheck=conn->getSendVar();
            sendCheck->add(filenm);
            if( sendHttpHeader(conn, rst, fsize, ext)==NULL) {
                return true;
            }
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
            // initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
        } else {
            conn->GetVar("@error")->fmt("#0#Send File %s open error\n", path);
			conn->closeClient();
		}   
    } break;
    case 7: {		// sendBuffer
        if( conn==NULL || conn->xsock==NULL ) {
            qDebug("#0#sendBuffer socket error\n");
            return true;
        }
        if( !conn->valid() ) {
            qDebug("#0#sendBuffer socket not valid\n");
            return true;
        }
        rst->reuse();
        if( arrs ) {
            int sz=arrs->size();
            for(int n=0; n<sz; n++ ) {
                int sp=0,ep=0;
                sv = getStrVarPointer(arrs->get(n), &sp, &ep);
                if( sp<ep ) {
                    rst->add(sv,sp,ep);
                }
            }
            conn->setLong("@sendTick", GetTickCount());
            if(rst->length() ) {
                if(conn->isWrite(20)) {
                    conn->send(rst);
                }
            }
        }
    } break;
    case 8: {		// waitConnect

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
                            if(recv==0) {
                                conn->closeSocket();
                            } else {
                                conn->setNodeFlag(FLAG_SKIP);
                            }
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
            int contentLen = readHttpHeader(conn, rst->reuse(), NULL);
            qDebug("read all type:%s (size:%d)\n", ty, contentLen);
            if( contentLen>0 ) {
                int sz = rst->length();
                if( sz<contentLen ) {
                    conn->read(rst,contentLen-sz);
                }
            }
        }

    } break;
    case 14: {		// read
        conn->setNodeFlag(FLAG_USE);
        if(arrs==NULL) {
            StrVar* var=conn->read();
            if(var==NULL) {
                rst->setVar('3',0);
            } else {
                int len=var->length();
                rst->setVar('s',0,(LPVOID)var).addInt(0).addInt(len).addInt(0).addInt(len);
            }
        } else {
            conn->read(rst);
        }
    } break;
    case 15: {		// sendAll
        StrVar* var=rst;
        if(arrs==NULL ) {
            var=conn->getSendVar();
        } else {
            for(int n=0,sz=arrs->size();n<sz;n++ ) {
                int sp=0,ep=0;
                StrVar* sv=getStrVarPointer(arrs->get(n),&sp,&ep);
                if(sp<ep) var->add(sv,sp,ep );
            }
        }
        qDebug("...send all var size:%d\n", var->length() );
        conn->send(var);
    } break;
    case 16: {		// flush
        XSocket* sock=conn->xsock;
        if(sock) {
            FTRY {
                sock->Flush();
            } FCATCH(FSocketException, ex ) {
                conn->closeSocket();
            }
        }
    } break;
    case 17: {		// peek
        XSocket* sock=conn->xsock;
        if(sock) {
            FTRY {
                int peek=sock->Peek();
                rst->setVar('0',0).addInt(peek);
            } FCATCH(FSocketException, ex ) {
                conn->closeSocket();
            }
        }
    } break;
    case 18: {		// skip
        XSocket* sock=conn->xsock;
        if( sock ) {
			if( arrs && isNumberVar(arrs->get(0)) ) {
                char buf[2048];
                int delay =100;
                conn->setNodeFlag(FLAG_SKIP);
				delay=toInteger(arrs->get(0));
                conn->setTick();
                if(conn->isRead(delay) ) {
                    while(sock && sock->IsValid() ) {
                        int ret=sock->Receive((LPVOID)buf,2048);
                        if(ret<=0 ) {
                            conn->clearNodeFlag(FLAG_SKIP);
                            break;
                        }
                    }
                }
            } else {
                sock->Flush();
            }
		}
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
        if( conn->valid() ) {
            conn->closeSocket();
        }
        LPCC host = AS(0);
        int port = isNumberVar(arrs->get(1))? toInteger(arrs->get(1)): 80;        
        conn->clearNodeFlag(FLAG_ERROR);
        if( isNumberVar(arrs->get(2)) ) {
            int timeout=toInteger(arrs->get(2));
            conn->xtick+=timeout;
            tn->GetVar("@timeout")->setVar('0',0).addInt(timeout);
        }
        rst->reuse();
        if( conn->connect(host, port) ) {
            conn->isNodeFlag(FLAG_RUN);
            conn->setTick();
            if( !isVarTrue(arrs->get(2)) ) {
                conn->xtype=SOKETTYPE_MODULE;
            }
            tn->GetVar("@host")->set(host);
            if(!conn->isNodeFlag(FLAG_ERROR)) {
                rst->setVar('3',1);
            }
        } else {
            qDebug("#0#connect error %s %d",host, port);
            rst->setVar('3',0);
        }
    } break;
    case 32: {		// isConnect
        bool chk=false;
        if(conn->valid() && !conn->isNodeFlag(FLAG_ERROR)) {
            chk=true;
        }
        rst->setVar('3',chk? 1:0 );
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
            conn->closeSocket();
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
    case 42: {		//
    } break;
    case 43: {		// recv
        conn->recv( rst->reuse() );
    } break;
    case 44: {		// client
        rst->setVar('v',1,(LPVOID)conn);
    } break;
    case 45: {		// server
        ConnectNode* parent=conn->parent();
        if( parent ) {
            TreeNode* server=parent->config();
            if(server) rst->setVar('n',0,(LPVOID)server);
        }
    } break;
    case 46: {		// sendHttp
        if( arrs==NULL ) {
            return false;
        }
        int sp=0, ep=0;
        StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        int size= sp<ep ? ep-sp: 0;
        StrVar* send=sendHttpHeader(conn,conn->xsend.reuse(),(qint64)size,"local");
		if(send ) {
			if( size > 0 ) {
				send->add(sv->get(sp), size);
			}
			conn->send(send);			
            // initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
		}
    } break;

    case 49: {		// sendStream
        LPCC path=AS(0);
        QFile file(KR(path));
        if( file.open(QIODevice::ReadOnly) ) {
            qint64 fsize=file.size();
            int pos= LastIndexOf(path,'.');
            LPCC ext = pos>0 ? path+pos+1: NULL;
            if( sendHttpHeader(conn, rst, fsize, ext, false) ) {
                bool ok=true;
                qint64 total=fsize, remain=0, start=0, end=0;
                sv=conn->gv("rangeStart");
                if( sv && sv->length() ) {
                    start=(qint64)toUL64(conn->gv("rangeStart")), end=(qint64)toUL64(conn->gv("rangeEnd"));
                    if( start<end ) {
                        remain=end-start+1;
                    } else {
                        remain=total-start;
                        remain+=1;
                    }
                } else {
                    if(end==0 ) end=total;
                    if( start<end ) {
                        if( end>total ) end=total;
                    } else {
                        end=total;
                    }
                    remain=end-start;
                }
                if( start>0 ) {
                    if( !file.seek(start) ) {
                        remain=0;
                        ok=false;
                        qDebug("#0#send streaming file seek error (start:%lld)\n", start);
                    }
                }
                if(ok ) {
                    char buf[SOCK_BUF_SIZE];
                    while( remain>0 && conn->valid() ) {
                        int read=(int)file.read(buf, remain > SOCK_BUF_SIZE?SOCK_BUF_SIZE: remain );
                        if(read<=0) {
                            break;
                        }
                        int send=conn->send((BYTE*)buf,read);
                        if( read==send ) {
                            total+=read;
                            remain-=read;
                            if( read<4096 ) {
                                qDebug("#0#sendStream is remain buffer %s (remain:%lld, total:%lld, read:%d)\n", path, remain, total, read);
                            }
                        } else {
                            if(send>0) {
                                continue;
                            }
                            conn->closeSocket();
                            break;
                        }
                    }
                }
            }
            file.close();
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
        rst->reuse();
        if(arrs==NULL ) {
            for( WBoxNode* bn=conn->First(); bn; bn=conn->Next() ) {
                LPCC code=conn->getCurCode();
                if( code[0]=='@' ) continue;
                if( rst->length() ) rst->add("\n");
                rst->add(code).add("=").add(toString(conn->gv(code)) );
            }
            return true;
        } 
		if(checkFuncObject(arrs->get(0),'3',1) ) {
            for( WBoxNode* bn=conn->First(); bn; bn=conn->Next() ) {
                LPCC code=conn->getCurCode();
                if( rst->length() ) rst->add("\n");
                rst->add(code).add("=").add(toString(conn->gv(code)) );
            }
            return true;
        }
        LPCC code=AS(0);
		sv=conn->gv(code);
		if(sv) {
			rst->add(sv);
			if(isVarTrue(arrs->get(1)) ) {
				sv->reuse();
			}
		}
    } break;
    case 52: {		// setValue
        if(arrs==NULL ) {
            return true;
        }
        // (JWas)
        int sp=0,ep=0;
        StrVar* sv=getStrVarPointer(arrs->get(1),&sp,&ep);
        LPCC code=AS(0);
        if(slen(code)) {
            StrVar* var=conn->GetVar(code)->reuse();
            if( sp<ep ) var->add(sv, sp, ep);
            rst->setVar('3',1);
        }
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
                    qDebug("#0#database used idx:%d dns:%s\n", n, dsn );
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
        //
        if( model==NULL && slen(dsn)  ) {
            int idx=a->size()+1;
            LPCC inode=gBuf.fmt("%s_%d", dsn, idx );
            model=getTreeNode("dbms", inode);
            if(model ) {
                qDebug("database dsn:%s inode:%s\n", dsn, inode);
                model->setNodeFlag(FLAG_USE);
                getModelDatabase(dsn, inode, model);
                a->add()->setVar('n',0,(LPVOID)model);
            }
        }
        if( model ) {
            // qDebug("web database set dsn:%s %s", dsn, model->get("inode") );
            model->setLong("@startTick", GetTickCount() );
            model->setNodeFlag(FLAG_USE);
            conn->GetVar("@database")->setVar('n',0,(LPVOID)model);
            rst->setVar('n',0,(LPVOID)model );
        } else {
            qDebug("#9#web database not set driver:%s", driver );
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
            int sz=conn->send((BYTE*)rst->get(), rst->length() );
            rst->setVar('0',0).addInt(sz);
            conn->closeSocket();
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

    } break;
    case 66: { // proxyRecv
        if(arrs==NULL ) {
            return true;
        }
		// ex) proxyRecv(object, data, total, header);

    } break;
    case 67: { // template    
        if(arrs==NULL) {
            return true;
        }
        int sp=0, ep=0;
        StrVar* data=getStrVarPointer(arrs->get(0),&sp,&ep);
        if(sp<ep) {
            TreeNode* thisNode=NULL;
            sv=fn->gv("@this");
            if(SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
            }
            if( thisNode==NULL ) {
                thisNode=tn;
            }
            XFuncNode* fnCur=fn;
            sv=arrs->get(1);
            if(SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
            }
            StrVar* var=arrs->add();
            var->set("#");
            var->add(data,sp,ep);
            makeEvalSub(var, 0, var->length(), conn, thisNode, fnCur, rst, "template", false);
        }
    } break;
    case 68: { // pageSource
		if(arrs==NULL) return true;

    } break;
    case 69: { // tickCount
        // (JWas)
        if(arrs==NULL ) {
            rst->setVar('1',0).addUL64(conn->getTick());
            return true;
        }
        sv=arrs->get(0);
        if(SVCHK('3',1)) {
            conn->setTick();
            rst->setVar('1',0).addUL64(0);
        } else {
            LPCC name=AS(0);
            int sz=arrs->size();
            if(sz==1 ) {
                UL64 prev=conn->getLong(name), tick=GetTickCount();
                if( prev>0 && prev<tick) {
                    rst->setVar('1',0).addUL64(tick-prev);
                } else {
                    rst->setVar('1',0).addUL64(0);
                }
            } else {
                sv=arrs->get(1);
                if(SVCHK('3',0)) {
                    conn->setLong(name, 0);
                } else if(SVCHK('3',1)) {
                    conn->setLong(name, GetTickCount());
                }
            }
        }
    } break;
    case 70: { // asyncMode
        BOOL async=arrs==NULL || isVarTrue(arrs->get(0)) ? TRUE: FALSE;
        FTRY {
            conn->xsock->SetCompletion(async);
            rst->setVar('3',1);
        } FCATCH( FSocketException, ex ) {
            rst->setVar('3',0);
        }
    } break;
    case 71: { // sendProxy
        if(arrs==NULL ) return true;
        rst->reuse();
        sv=arrs->get(0);
        if(SVCHK('v',1)) {
            ConnectNode* target=(ConnectNode*)SVO;
            int sp=0,ep=0;
            StrVar* packet=getStrVarPointer(arrs->get(1),&sp,&ep);
            if(conn->isNodeFlag(FLAG_USE) ) {
                for(int n=0;n<64;n++) {
                    FThread::Sleep(50);
                    if(!conn->isNodeFlag(FLAG_USE) ) break;
                }
            }
            if(sp<ep ) {
                conn->setNodeFlag(FLAG_USE);
                if(conn->sendData(packet,sp,ep)) {
                    sv=arrs->get(2);
                    if(SVCHK('3',1)) {
                       int ret=conn->recvProxy(target, rst);
                       qDebug("send proxy recv result size:%d send:%d", rst->length(), ret);
                    } else {
                        if(conn->recvData(rst)) {
                            if(target && target->sendData(rst,0,rst->length()) ) {
                                rst->setVar('3',1);
                            } else {
                                rst->set("send proxy target send error");
                            }
                        } else {
                            rst->set("send proxy device recv error");
                        }
                    }
                } else {
                    rst->set("send proxy device recv error");
                }
                conn->clearNodeFlag(FLAG_USE);
            } else {
                rst->set("send proxy packet error");
            }
            target->clearNodeFlag(FLAG_SETEVENT);
        } else {
            rst->set("send proxy target error");
        }
    } break;
    case 72: { // sendData
        if(arrs==NULL ) return true;
        int sp=0,ep=0;
        StrVar* packet=getStrVarPointer(arrs->get(0),&sp,&ep);
        if(conn->sendData(packet,sp,ep)) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 73: { // recvData
        int total=0;
        if(arrs ) {
            if(isNumberVar(arrs->get(0))) {
                total=toInteger(arrs->get(0));
            }
        }
        rst->reuse();
        if(conn->recvData(rst,total)==NULL) {
            rst->setVar('3',0);
        }
    } break;
    case 74: { // resetFlag
        if(conn->valid()) {
            FTRY {
                conn->xsock->Flush();
                conn->clearNodeFlag(FLAG_SETEVENT | FLAG_RUN);
                conn->setTick();
            } FCATCH(FSocketException, ex ) {
                conn->closeSocket();
            }
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 75: { // setEvent
        if(arrs==NULL ) {
            conn->setNodeFlag(FLAG_SETEVENT);
            return true;
        }
        gMutexSetEvent.EnterMutex();
        XListArr* penddingClientList=NULL;
        StrVar* sv=conn->gv("@penddingClientList");
        if(SVCHK('a',0)) {
            penddingClientList=(XListArr*)SVO;
        } else {
            penddingClientList=new XListArr(true);
            conn->GetVar("@penddingClientList")->setVar('a',0,(LPVOID)penddingClientList);
        }
        sv=arrs->get(0);
        if(SVCHK('v',1) ) {
            ConnectNode* target=(ConnectNode*)SVO;
            if(conn->isNodeFlag(FLAG_SETEVENT)) {
                penddingClientList->add()->setVar('v',1,(LPVOID)target);
                rst->setVar('3', 0);
            } else {
                conn->setNodeFlag(FLAG_SETEVENT);
                rst->setVar('3', 1);
            }
        } else {
            LPCC ty=AS(0);
            if(ccmp(ty,"clear") ) {
                for(int n=0; n<penddingClientList->size(); n++) {
                    sv=penddingClientList->get(0);
                    if(SVCHK('v',1) ) {
                        ConnectNode* target=(ConnectNode*)SVO;
                        target->closeSocket();
                    }
                }
                penddingClientList->reuse();
            } else if(ccmp(ty,"next") ) {
                bool ok=false;
                conn->clearNodeFlag(FLAG_SETEVENT);
                while(penddingClientList->size()>0) {
                    sv=penddingClientList->get(0);
                    if(SVCHK('v',1) ) {
                        ConnectNode* target=(ConnectNode*)SVO;
                        if(target->valid() ) {
                            LPCC url=target->get("@url");
                            ok=wasCallProxyFun(target, url);
                        } else {
                            qDebug("proxy target socket not validate");
                            target->closeSocket();
                        }
                    } else {
                        qDebug("proxy target validate error");
                    }
                    penddingClientList->remove(0);
                    if(ok) break;
                }
                rst->setVar('3', ok?1:0);
            } else if(ccmp(ty,"size") ) {
                rst->setVar('0',0).addInt(penddingClientList->size());
            }
        }
        gMutexSetEvent.LeaveMutex();

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
            TreeNode* cf=conn->xcf;
            // StrVar* sv=NULL;
            int bufferSize=4096*10;
            /*
            int remainSize=toInteger(cf->gv("recvRemainSize"));
            if( remainSize > 0 ) {
                int total = remainSize;
                int mask = toInteger(cf->gv("readMask"));
                LPCC maskKey=cf->GetVar("maskKey")->get();
                char buffer[bufferSize];
                rst->reuse();
                while( total>0 ) {
                    int remain=total>bufferSize ? bufferSize: total;
                    int read=sock->read(buffer, remain);
                    if( read<=0 ) break;
                    rst->add(buffer, read);
                    qDebug("2 ===> %d %d", total, read);
                    total-=read;
                }
                qDebug("3==> %d (mask:%d)", total, mask);
                if( maskKey ) {
                    char* tmp=rst->get();
                    for( int n=0, sz=rst->length(); n<sz; n++ ) {
                        tmp[n]^=maskKey[n%4];
                    }
                }
            } else {
            }
            */
            char buf[16];
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
                cf->GetVar("readMask")->setVar('0',0).addInt(mask);

                qint64 total=len;
                char buffer[bufferSize];
                rst->reuse();
                while( total>0 ) {
                    int remain=total>bufferSize ? bufferSize: total;
                    int read=sock->read(buffer, remain);
                    if( read<=0 ) break;
                    rst->add(buffer , read);
                    total-=read;
                    qDebug("1 ===> %d %d %d", (int)total, read, (int)len);
                }
                if( maskKey ) {
                    char* tmp=rst->get();
                    cf->GetVar("maskKey")->set(maskKey,4);
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
        // qDebug("#0#send data client 0\n");
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
            qDebug("#0#send data client (sp:%d, size:%d)\n", sp, size);
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

    } break;
    case 218: { // proxyRecv

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

bool callUdpFunc(StrVar* me, XFunc* fc, LPCC funcName, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    UdpServerSocket* conn=NULL;
    if( checkFuncObject(me,'v',33) ) {
        conn=(UdpServerSocket*)me->getObject(FUNC_HEADER_POS);
    } else {
        if( tn==NULL ) {
            return false;
        }
        conn=new UdpServerSocket(tn, NULL);
        tn->GetVar("@c")->setVar('v',33,(LPVOID)conn );
    }
    // qDebug("message client func name==%s ref:%d\n", funcName, fc->xfuncRef );
    if( conn==NULL ) {
        return tn==NULL? true: false;
    }
    if( fc->xfuncRef==0 ) {
        LPCC fnm=funcName? funcName : fc->getFuncName();
        fc->xfuncRef=ccmp(fnm, "start")? 10 : fc->xfuncRef=ccmp(fnm, "send")? 11 : 0;
    }
    switch( fc->xfuncRef ) {
    case 10: { // start
        XFuncSrc* fsrc=NULL;
        int port=0;
        StrVar* sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
            sv=arrs->get(1);
            if(isNumberVar(sv)) port=toInteger(sv);
        } else if(isNumberVar(sv)) {
            port=toInteger(sv);
            sv=arrs->get(1);
            if( SVCHK('f',1) ) fsrc=(XFuncSrc*)SVO;
        }
        if(port>0 && fsrc ) {
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn,tn,NULL,tn->GetVar("onRecv"));
            fnCur->GetVar("@params")->setVar('a',0,(LPVOID)&conn->xparam );
            conn->startUdpServer(port);
        }
    }break;
    case 11: { // send
        if(arrs==NULL) return true;
        int sp=0, ep=0;
        StrVar* sv=arrs->get(0);
        int port=toInteger(sv);
        sv=getStrVarPointer(arrs->get(1),&sp, &ep);
        qDebug("udp send %d %d",sp, ep);
        if(sp<ep) {
            QUdpSocket socket;
            QByteArray ba;
            ba.append(sv->get(sp),ep-sp);
            if(socket.writeDatagram(ba.data(), ba.size(), QHostAddress::Broadcast, port)) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        }
    }break;
    default: return false;
    }
    return true;
}
