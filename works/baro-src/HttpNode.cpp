#include "HttpNode.h"
#include "../util/user_event.h"
#include <QCoreApplication>

#define KEEPALIVE_USE
#define WAS_THREAD_USE
#define FILE_SEND_THREAD_USE
#define QFILE_SEND

FMutex		gWasFuncNode;
XListArr	gWasStrVars;
int gWasStrVarsIdx=0;

inline TreeNode* getProxyMap() {
	TreeNode* base=uom.getInfo();
    StrVar* sv=base->gv("proxyMaps");
    TreeNode* map=NULL;
	if(SVCHK('n',0)) {
		map=(TreeNode*)SVO;
	} else {
		map=new TreeNode(8);
		map->setNodeFlag(FLAG_PERSIST);
		base->GetVar("proxyMaps")->setVar('n',0,(LPVOID)map);
	}
	return map;
}

int getWebContentLength(StrVar* var) {
    LPCC prop="Content-Length: ";
    int pos=var->find(prop);
    if(pos>0) {
        pos+=slen(prop);
        int end=var->find("\n",pos);
        if(pos<end) {
            LPCC val=var->trimv(pos,end);
            if(isnum(val)) {
                return atoi(val);
            }
        }
    }
    return 0;
}

inline StrVar* getWasStrVar() {
	if( gWasStrVarsIdx>=64 ) {
        gWasStrVarsIdx=0;
    }
    if( gWasStrVars.size()==0 ) {
		for(int n=0; n<64;n++ ) gWasStrVars.add();
    }
    return gWasStrVars.get(gWasStrVarsIdx++)->reuse();
}
inline LPCC wasRootPath(ConnectNode* cn) {
    ConnectNode* root = cn->parent();
    TreeNode* cf=root? root->config() : NULL;
    return cf? cf->get("rootPath"): NULL;
}

inline LPCC getTextVarCheck(TreeNode* param, StrVar* var, int sp, int ep) {
    if(var->find("$",sp,ep)!=-1 ) {
        XParseVar pv(var,sp,ep);
        StrVar* src=uom.getStrVar();
        makeTextData(pv, NULL, src, 0, param);
        return src->get();
    }
    return NULL;
}

inline TreeNode* getUrlMap(ConnectNode* root) {
    TreeNode* uriMap=NULL;
    StrVar* sv = root->config()->GetVar("@uriMap");
    if( SVCHK('n',0) ) {
        uriMap=(TreeNode*)SVO;
    } else {
        uriMap=new TreeNode(8);
        sv->setVar('n',0,(LPVOID)uriMap);
    }
    /*
    TreeNode* child=uriMap->child(0);
    if( child==NULL ) {
        child=uriMap->addNode();
    }
    */
    return uriMap;
}
inline bool callReqFunc(ConnectNode* cn, TreeNode* node, LPCC fnm, XFuncNode* fnInit) {
    StrVar* sv=node->gv(fnm);
    if( SVCHK('f',1) ) {
        XFuncSrc* fsrc = (XFuncSrc*)SVO;
        XFunc* func=fsrc->xfunc;
        XFuncNode* fnCur = gfns.getFuncNode(func, fnInit);
        if( func && fnCur ) {
            XListArr* arrs=getParamArray(func,fnCur);
            XParseVar pv(&(fsrc->xparam));
            if( pv.valid() ) {
                LPCC req=pv.findEnd(",").v();
                fnCur->GetVar(req)->setVar('v',1,(LPVOID)cn);
            } else {
                fnCur->GetVar("req")->setVar('v',1,(LPVOID)cn);
            }
            if( pv.valid() ) {
                LPCC param=pv.findEnd(",").v();
                fnCur->GetVar(param)->setVar('n',0,(LPVOID)cn->config());
            } else {
                fnCur->GetVar("param")->setVar('n',0,(LPVOID)cn->config());
            }
#ifdef INLINENODE_USE
            if( func->xflag & FUNC_INLINE ) {
                fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)node);
            }
#endif
            getCf()->set("@currentFuncName", fnm);
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node );
            fnCur->call(func);
            funcNodeDelete(fnCur);
            releaseLocalArray(arrs);
            return true;
        }
    }
    return false;
}
inline bool sendVar(ConnectNode* conn, StrVar* sv ) {
    if( sv==NULL) return false;
    if( !conn->valid() ) {
        qDebug("#9#send var error socket not valid idx:%d (dist:%d, flag:0x%X)\n", conn->xclientIndex, conn->getTick() , conn->xflag);
        return false;
    }
    int len=sv->length();
    if( len==0 ) {
        StrVar* var=confVar("web.pageEmpty");
        if( var->length()==0 ) {
            var->set("error: connectNode send size==0");
        }
        sv=var;
        len=sv->length();
    }
    qDebug("#0#send page start url:%s len:%d \n", conn->get("url"), len);
    conn->set("Cache-Control", "no-cache");
    conn->setInt("@state", 200);
    if(sendHttpHeader(conn,conn->getSendVar(),(qint64)len) ) {
        conn->send(sv);
    } else {
        qDebug("#0#send header error url:%s\n", conn->get("url"));
    }
    // initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
}

inline bool httpSendBuffer(ConnectNode* node, StrVar* data, LPCC typeNm ) {
    StrVar* varSend=node->GetVar("@httpSendData");
    ConnectNode* root=node->xparent;
    varSend->reuse();
    if(data) {
        varSend->add(data);
    } else {
        if(ccmp(typeNm,"notfound") ) {
            data=confVar("msg.notfound");
            if(data && data->length() ) {
                varSend->add(data);
            } else {
                varSend->add("404 not found");
            }
        } else if(slen(typeNm)>0 ) {
            data=confVar(gBuf.fmt("msg.%s",typeNm));
            if(data ) {
                varSend->add(data);
            } else {
                varSend->format(128,"http send buffer error message code:%s", typeNm);
            }
        } else {
            varSend->set("http send buffer error");
        }
    }
    StrVar* sv=root? root->gv("@c"): NULL;
    if( SVCHK('v',0) ) {
        HttpServerThread* ht=(HttpServerThread*)SVO;
        WorkerThread* w = ht->getWorkerFile();
        if( w ) {
            if( (w->xflag & FLAG_START)==0 ) {
                w->exec();
            }
            node->setNodeFlag(FLAG_PERSIST);
            w->pushObject('v',1,(LPVOID)node);
            return true;
        }  else {
            qDebug("#9#web send file worker not define (client:%d)\n", node->xclientIndex );
        }
    }
    return false;
}

inline bool httpSendFileName(ConnectNode* node, LPCC filenm, LPCC url ) {
    UL64 filelen = 0;
    U32 state=httpFileState(filenm,node->GetVar("@modify"),node->get("If-Modified-Since"),&filelen);
    if( state==1 && !node->isBool("@paramUse") ) {    //  && !StartWith(url,"/pages/")
        node->setInt("@state", 304);
        // qDebug("#0#web send file worker 304 (SIZE:%d, URL:%s)", node->getInt("Content-Length"), node->get("url"));
        httpSend(node);
        return true;
    }

    if( filelen>0 ) {
        node->setInt("@state", 200);
        node->setLong("fileLength", filelen);
    } else {
        node->setInt("@state", 404);
        return httpSendBuffer(node, NULL,"notfound");
    }
    ConnectNode* root=node->xparent;
    StrVar* sv=root? root->gv("@c"): NULL;
    if( SVCHK('v',0) ) {
        HttpServerThread* ht=(HttpServerThread*)SVO;
        WorkerThread* w = ht->getWorkerFile();
        if( w ) {
            if( (w->xflag & FLAG_START)==0 )
                w->exec();
            node->setNodeFlag(FLAG_PERSIST);
            w->pushObject('v',1,(LPVOID)node);
            return true;
        }  else {
            qDebug("#9#web send file worker not define (client:%d)\n", node->xclientIndex );
        }
    }
    qDebug("==== http send file error ==== uri : %s (size:%d)\n", filenm, (int)filelen);
    return true;
}

inline LPCC getImagesMapFullPath(ConnectNode* node, StrVar* sv, LPCC rootPath) {
    StrVar* pathVar=node->GetVar("@filenm")->reuse();
    if( sv ) {
        LPCC path=toString(sv);
        if( path ) {
            if( path[0]=='/' ) {
                pathVar->add(path);
            } else {
                pathVar->add(rootPath).add('/').add(path);
            }
            return pathVar->get();
        }
    }
    pathVar->add(rootPath).add("/images/baroScript.png");
    return pathVar->get();
}



HttpServerThread::HttpServerThread(TreeNode* cf) : xcf(cf), xroot(NULL), xmainWorker(NULL), xfnSize(0)
{
    StrVar* sv= getCf()->GetVar("@contentType");
    TreeNode* node=NULL;
    if( SVCHK('n',0) ) {
        node=(TreeNode*)SVO;
    } else {
        node = new TreeNode(2);
        setHttpContentType(node);
        sv->setVar('n',0,(LPVOID)node);
    }
    // getCf()->setNodeFlag(CNF_HEADER_THREAD);

    xwebDataWorkers.initWorker(8, FLAG_WEB_CALL);
    xwebFileWorkers.initWorker(16, FLAG_WEB_CALL);
    // xwebConnectionWorkers.initWorker(4, FLAG_WEB_CALL);
    xfnSize = 8;
    xfnList.Create(xfnSize*3);
    for( int n=0;n<xfnSize;n++ ) {
        XFuncNode* fn= new XFuncNode();
        // fn->setNodeFlag(FLAG_PERSIST);
        xfnList.add(fn);
    }
    if( xcf==NULL ) {
        xcf=new TreeNode(4);
    }
    if( xroot==NULL ) {
        xroot = new ConnectNode(xcf);   //gConns.getNode();
        if( xroot->xcf==NULL ) {
            xroot->xcf=xcf;
        }
    }
    if( xmainWorker==NULL ) {
        xmainWorker=new WorkerThread(cf);
        xmainWorker->httpServer=this;
        xmainWorker->exec();
    }
    xcloseCheck=false;
}

HttpServerThread::~HttpServerThread() {
    /*
    SAFE_DELETE(xroot);
    SAFE_DELETE(xmainWorker);
    */
}

WorkerThread* HttpServerThread::getWorker() {
    return  xwebDataWorkers.getWorker();
}

WorkerThread* HttpServerThread::getWorkerFile() {
    return  xwebFileWorkers.getWorker();
}
WorkerThread* HttpServerThread::getWorkerConnection() {
    if( xwebConnectionWorkers.xworkerSize==0 ) {
        xwebConnectionWorkers.initWorker(4, FLAG_WEB_CALL);
    }
    return  xwebConnectionWorkers.getWorker();
}


#ifdef FUNCNODE_USE
XFuncNode* HttpServerThread::getFuncNode(ConnectNode* cn, XFuncSrc* fsrc, TreeNode* page, U32 flag) {
    XFuncNode* fnCur = NULL;
    int idx=0;
    DWORD tick=GetTickCount();
    gWasFuncNode.EnterMutex();
    for( int n=0;n<xfnSize;n++ ) {
        XFuncNode* fn= xfnList.getvalue(n);
        if( fn->isNodeFlag(FLAG_RUN|FLAG_PERSIST) ) {
            DWORD dist=tick-fn->xtick;
            if( dist>5000 ) {
                qDebug("#0#HttpServerThread::getFuncNode called (idx:%d, tick:%u)\n\n", n, dist);
                fn->clearNodeFlag(FLAG_CALL | FLAG_RUN );
                if( fn->isNodeFlag( FLAG_PERSIST ) ) {
                    if( dist>600000 ) {
                        fn->clearNodeFlag( FLAG_PERSIST );
                    }
                }
            }
            continue;
        }
        fn->setNodeFlag(FLAG_RUN );
        idx=n;
        fnCur=fn;
        break;
    }
    if( fnCur ) {
        fnCur->initVar();
    } else  {
        fnCur= new XFuncNode();
        xfnList.add(fnCur);
        idx=xfnSize;
        xfnSize++;
        qDebug("#0# new funcNode ## getFuncNode size: %d\n",xfnSize);
    }
    fnCur->setTick();
    gWasFuncNode.LeaveMutex();
    if( cn==NULL ) {
        fnCur->setNodeFlag(FLAG_RUN);
        if( fsrc ) {
            fnCur->xfunc = fsrc->xfunc;
        }
        return fnCur;
    }
    if( cn->isNodeFlag(FLAG_SINGLE) ) {
        fnCur->setNodeFlag(FLAG_SINGLE);
    }
    if( page ) {
        StrVar* sv = page->gv("onInit");
        XFuncNode* fnInit=NULL;
        if( SVCHK('f',0) ) {
            fnInit=(XFuncNode*)SVO;
        }
        fnCur->xparent=fnInit;
    } else if(fnCur->xparent ) {
        fnCur->xparent=NULL;
    }
    /*
    fnCur->GetVar("@server")->setVar('v',0,(LPVOID)this);
    fnCur->GetVar("@fnIndex")->setVar('0',0).addInt(idx);
    */
#ifdef WAS_THREAD_USE
    WorkerThread* w=getWorker();
    if( w && fsrc ) {        
        if( flag )
            w->config()->setNodeFlag(flag);
        cn->setNodeFlag(FLAG_INIT|FLAG_PERSIST);
        fnCur->setNodeFlag(FLAG_RUN | FLAG_WEB_CALL);
        fnCur->xfunc = fsrc->xfunc;
        fnCur->GetVar("req")->setVar('v',1,(LPVOID)cn);
        fnCur->GetVar("param")->setVar('n',0,(LPVOID)cn->config());
        if( page ) {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)page);
        } else {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)cn->config());
        }
        if( (w->xflag & FLAG_START)==0 ) {
            w->exec();
        }
        w->pushObject('f',0,(LPVOID)fnCur);
    } else {
        qDebug("#0#web func node error (url:%s)\n", cn->get("url"));
        cn->closeSocket();
        fnCur->clearNodeFlag(FLAG_CALL|FLAG_RUN);
    }
#else
    cn->setNodeFlag(FLAG_WEB_CALL);
    fnCur->setNodeFlag(FLAG_RUN | FLAG_WEB_CALL);
    fnCur->xfunc = fsrc->xfunc;
    fnCur->GetVar("req")->setVar('v',1,(LPVOID)cn);
    if( page ) {
        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)page);
    }
    fnCur->xtick = tick;
    fnCur->call();
    fnCur->clearNodeFlag( FLAG_RUN );
    cn->closeSocket();
#endif
    return fnCur;
}

#endif

void HttpServerThread::startHttpServer() {
    if( xcf==NULL ) return;
    xroot->GetVar("@c")->setVar('v',0,(LPVOID)this);
    int port = xcf->getInt("port", 80);
    int block = xcf->getInt("block", 64);

    if( xroot->bind(port,block,STYPE_HTTP_SERVER) ) {
        StrVar* sv=NULL;
        LPCC serverType=xroot->get("serverType");
        qDebug("#0#----- Server bind ----- (port:%d, block:%d %s)\n", port, block, serverType);
        if(ccmp(serverType,"was")) {
            getUrlMap(xroot);
            sv=getStrVar("fsrc","webFileFilter");
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFunc* func=fsrc->xfunc;
                if( func ) {
                    func->xflag|=FUNC_INLINE;
                }
            }
            qDebug("#0#----- WAS bind ----- (port:%d, block:%d)\n", port, block );
        } else {
            xroot->setNodeFlag(FLAG_SINGLE);
        }
        int timeout=xroot->getInt("@timeout");
        xcf->setNodeFlag(FLAG_RUN);
        xroot->getSocket()->setFunc(socketHttpAcceptFunc);
        if(timeout==0) timeout=2000;
        xtm.tv_sec = (long)(timeout/1000);
        xtm.tv_usec = (timeout%1000)*1000;

        Create();
        XSocket* sck = xroot ? xroot->getSocket(): NULL;
        if( sck ) {
#ifndef WINDOW_USE
            int opt = 3;
            setsockopt(sck->GetSocketDescriptor(), SOL_SOCKET, SO_RCVLOWAT,&opt,sizeof(opt));
#endif
        }

    } else {
        qDebug("#9#----- WAS bind error ----- (port:%d, block:%d)\n", port, block );
        xcf->set("@error",gBuf.add("bind port error : %d",port));
    }
}

void HttpServerThread::procHttpServer() {
    FTRY {
        XSocket* sck = xroot ? xroot->getSocket(): NULL;
        if( sck==NULL || xroot==NULL ) {
            HttpServerThread::Sleep(100);
            return;
        }
        /*
        int num=0;
        DWORD tick=GetTickCount();
        if( xcloseCheck ) {
            HttpServerThread::Sleep(20);
            num++;
        }
        */
        /*
        int dist=(int)(GetTickCount()-tick);
        if( num>0 || dist>50 ) {
            qDebug("#0#socket dispatch tick:%d %s (num:%d)\n", dist, xcloseCheck? "close checked": "ok", num );
        }
        if( xcloseCheck ) {
            HttpServerThread::Sleep(20);
            num++;
            qDebug("dispatch stay (num:%d)\n", num);
        }
        */

        if( (xcloseCheck==false) && (xnodeList.size()>0) ) {
            xcloseCheck=true;
            /*
            int offset = sck->xendNode ? sck->xendNode->row(): 0;
            if( offset>0 ) {
                qDebug("#0#Http Server dispatch list size:%d, offset:%d\n", size, offset );
            }
            */
            int size=xnodeList.size();
            for( int n=0; n<size; n++ ) {
                ConnectNode* cur = xnodeList.getvalue(n);
                if(cur==NULL ) {
                    qDebug("#9#current node not define !!!");
                    break;
                }
                if( cur->valid()) {
                    cur->closeSocket();
                } else {
                    StrVar* sv=cur->gv("@database");
                    if( SVCHK('n',0) ) {
                        TreeNode* model=(TreeNode*)SVO;
                        model->clearNodeFlag(FLAG_USE);
                        qDebug("#0# close check client data base release db url:%s\n", cur->get("url") );
                        sv->reuse();
                    }
                }
                // (JWas)
                xroot->removeNode(cur);
                for(WBoxNode* bn=cur->First();bn;bn=cur->Next()) {
                    bn->value.close();
                }
                gConns.deleteNode(cur);
            }
            xnodeList.reuse();
            xcloseCheck=false;
        }

        LPCC serverType=xroot->get("serverType");
        bool ret=false;
        if(ccmp(serverType,"was")) {
            ret=sck->dispatchServer(xroot,xnodeList,&xtm);
        } else if(ccmp(serverType,"proxy")) {
            ret=sck->dispatchProxy(xroot,xnodeList,&xtm);
        } else {
            ret=sck->dispatchWas(xroot,xnodeList,&xtm);
        }

        if(ret) {
            HttpServerThread::Sleep(25);
        } else {
            HttpServerThread::Sleep(60);
        }
    } FCATCH( FSocketException, ex ) {
        StrVar* msg = config()->GetVar("msg");
        xroot->eventMessage(msg->set("procHttpServer FSocketException").add(" ==== ").add(ex.Get()) );
        HttpServerThread::Sleep(50);
    }
}
int HttpServerThread::clientCloseCheck() {
    if( xroot==NULL || xcloseCheck ) {
        return 0;
    }
    if( xnodeList.size()>0 ) {
        return 0;
    }
    xcloseCheck=true;
    ConnectNode* cur=NULL;
    int cnt=xroot->childCount();
    int notValidCnt=0;
    xroot->setTick();
    for(int n=0 ; n<cnt ; n++ ) {
        cur = xroot->child(n);
        if( !cur->valid() ) {
            if(cur->xflag==0) {
                xnodeList.add(cur);
            }
            notValidCnt++;
            continue;
        }
        // if( cur->isNodeFlag(FLAG_SINGLE) ) { continue; }
        if( cur->isNodeFlag(FLAG_SKIP)) {
            qDebug("#0#connect node flag is skip maybe closed (flag:0x%X)\n", cur->xflag);
            if( cur->getTick() > 500 ) {
                xnodeList.add(cur);
            }
            continue;
        }
        if( cur->isNodeFlag(FLAG_SET)) {
            int dist = cur->getTick();
            qDebug("#0#connect node FLAG_SET timeout (%d flag:0x%X)\n", dist, cur->xflag);
            if( dist > 2500 ) {
                xnodeList.add(cur);
            }
            continue;
        }
    }
    if(cnt>0 && cnt==notValidCnt) {
        qDebug("@@ client all close %d == %d ", cnt, notValidCnt);
        for(int n=0;n<xnodeList.size();n++) {
            ConnectNode* cur=xnodeList.getvalue(n);
            if(cur) {
                for(WBoxNode* bn=cur->First();bn;bn=cur->Next()) {
                    bn->value.close();
                }
                cur->reuse();
                gConns.deleteNode(cur);
            }
        }
        xroot->xchilds.reuse();
        xnodeList.reuse();
    }
    xcloseCheck=false;
    return cnt;
}

void HttpServerThread::Run() {    
    while( true ) {
        procHttpServer();
    }
    FTRY {
        XSocket* sck = xroot ? xroot->getSocket(): NULL;
        if( sck ) {
            sck->Close();
        }
    } FCATCH( FSocketException, ex ) {
        StrVar* msg = config()->GetVar("msg");
        xroot->eventMessage(msg->set("procHttpServer FSocketException").add(" ==== ").add(ex.Get()) );
    }
}

void HttpServerThread::Final() {
    qDebug("Final =========================== %d\n", xroot->xclientIndex );
}


//
// http UTIL
//
char* httpDate(StrVar* var)
{
    time_t tt;
    time( &tt);
    return var->ftime("%a, %d %b %Y %H:%M:%S GMT", gmtime(&tt) );
}

LPCC httpStatusCode( int status ) {
    switch(status) {
    case 100 : return "Continue";
    case 101 : return "Switching Protocols";

    case 200 : return "OK";
    case 201 : return "Created";
    case 202 : return "Accepted";
    case 203 : return "Non-Authoritative Information";
    case 204 : return "No Content";
    case 205 : return "Reset Content";
    case 206 : return "Partial Content";

    case 300 : return "Multiple Choices";
    case 301 : return "Moved Permanently";
    case 302 : return "Found";
    case 303 : return "See Other";
    case 304 : return "Not Modified";
    case 305 : return "Use Proxy";
    case 307 : return "Temporary Redirect";

    case 400 : return "Bad Request";
    case 401 : return "Unauthorized";
    case 402 : return "Payment Required";
    case 403 : return "Forbidden";
    case 404 : return "Not Found";
    case 405 : return "Method Not Allowed";
    case 406 : return "Not Acceptable";
    case 407 : return "Proxy Authentication Required";
    case 408 : return "Request Timeout";
    case 409 : return "Conflict";
    case 410 : return "Gone";
    case 411 : return "Length Required";
    case 412 : return "Precondition Failed";
    case 413 : return "Request Entity Too Large";
    case 414 : return "Request-URI Too Long";
    case 415 : return "Unsupported Media Type";
    case 416 : return "Requested Range Not Satisfiable";
    case 417 : return "Expectation Failed";

    case 500 : return "Internal Server Error";
    case 501 : return "Not Implemented";
    case 502 : return "Bad Gateway";
    case 503 : return "Service Unavailable";
    case 504 : return "Gateway Timeout";
    case 505 : return "HTTP Version Not Supported";

    default:
        return "Undefined";
    }
    return NULL;
}

LPCC getHttpContentType( LPCC ext) {
    if(IndexOf(ext,'/')>0 ) return ext;
    LPCC ct = NULL;
    StrVar* sv=getCf()->gv("@contentType");
    if( ext && SVCHK('n',0) ) {
        TreeNode* node=(TreeNode*)SVO;
        char* val=gBuf.add(ext);
        for( int n=0,len=slen(val); n<len; n++ ) val[n]=tolower(val[n]);
        ct = node->get(val);
    }
    return ct? ct: "application/octet-stream";
}


U32 httpFileState( LPCC filename, StrVar* rst, LPCC modified, PUL64 filesize ) {
    if( slen(filename)==0 ) return 0;
    struct stat filestat;
    FILE* fp = fopen(filename, "r+");
    U32 rtn = 0;
    if( fp && rst ) {
        fstat(fileno(fp), &filestat);
        struct tm* time_s = gmtime(&filestat.st_mtime);
        rst->ftime("%a, %d %b %Y %H:%M:%S GMT", time_s );
        if( filesize ) {
            *filesize = filestat.st_size;
        }
        if( ccmp(rst->get(),modified) ) {
            // qDebug("httpFileState (%s==%s)--------------\n\n", rst->get(), modified);
            rtn = 1;
        }
        fclose(fp);
    }
    return rtn;
}


char* getUrlDecode(const char *source, StrVar* var ) {
    if( source==NULL )
        return NULL;
    if( var==NULL ) {
        var=getWasStrVar();
    } else {
        var->reuse();
    }
    while( *source ) {
        switch ( *source ) {
        case '+':
            var->add(' ');
            break;
        case '%':
            if (source[1] && source[2]) {
                int value = HexPairValue(source + 1);
                if (value >= 0) {
                    var->add((char)value);
                    source += 2;
                }
                else {
                    var->add('?');
                }
            }
            else {
                var->add('?');
            }
            break;
        default:
            var->add(*source);
        }
        source++;
    }
    return var->get();
}

char* getUrlEncode(const char *source, StrVar* var)  {
    static const char *digits = "0123456789ABCDEF";
    if( var==NULL ) {
        var=getStrVarTemp(); // gStrDecode.reuse();
    } else {
        var->reuse();
    }
    var->reuse();
    unsigned char ch;
    while ( *source) {
        ch = (unsigned char)*source;
        if (*source == ' ') {
            var->add('+');
        }
        else if (isalnum(ch) || strchr("-_.!~*'()", ch)) {
            var->add(*source);
        }
        else {
            var->add('%');
            var->add(digits[(ch >> 4) & 0x0F]);
            var->add(digits[       ch & 0x0F]);
        }
        source++;
    }
    return var->get();
}

U32 httpParseParamVar(ConnectNode* node, XParseVar& pv, StrVar* decode) {
    return 0;
}

bool httpSendFile(ConnectNode* node ) {
    LPCC fileName=node->get("@filenm");
#ifdef QFILE_SEND
    QFile file(KR(fileName));
    // qDebug("... http send file %s ...\n", fileName);
    if( file.open(QFile::ReadOnly)) {
        qint64 read=0;
        char buf[SOCK_BUF_SIZE];
        while( !file.atEnd() ) {
            read=file.read(buf, SOCK_BUF_SIZE);
            if( read<=0 ) {
                break;
            }
            if( node->send((BYTE*)buf,read)==0 ) {
                // node->closeClient();
                qDebug("#0#http file send error (send size==0, filename:%s)\n",node->get("@filenm") );
                break;
            }
        }
        file.close();
        return true;
    } else {
        qDebug("#0#http file send error (file.open fail, filename:%s)\n",node->get("@filenm") );
    }
#else
    FTRY {
        FFile file;
        file.SetFileName(fileName);
        file.OpenIfExist(GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,0,0 );
        // StrVar* var = node->getSendVar();
        char buf[SOCK_BUF_SIZE]; // = var->memalloc(SOCK_BUF_SIZE);
        DWORD read = 0;
        while( true ) {
            file.Read(buf,SOCK_BUF_SIZE,&read);
            if( read==0 ) break;
            // DWORD send=node->send(buf,read);
            if( node->send(buf,read)<=0 ) {
                node->closeClient();
                break;
            }
        }
        file.Close();
        return true;
    } FCATCH( FFileSystemException, ex ) {
        qDebug("#0#http file send error (filename:%s, error:%s)\n",fileName, ex.Get() );
    }
    return false;
#endif
}

bool httpSendContent(ConnectNode* node, StrVar* content ) {
    node->send(content);
    return false;
}


bool httpSend(ConnectNode* node ) {
    // ConnectNode* root = node->parent();
    StrVar* var=node->getSendVar();
    StrVar* varSend=node->gv("@httpSendData");
    int state = node->getInt("@state");
    if( !node->isNodeFlag(FLAG_RUN) ) {
        qDebug("xxxxx httpSend header not set run flag (0x%x) xxxx\n", node->xflag );
		node->setNodeFlag(FLAG_RUN);
	}
    var->reuse();
    var->format(1024,"HTTP/1.1 %d %s\r\nDate: %s\r\nServer: baro\r\n",
        state,
        httpStatusCode(state),
        httpDate(node->GetVar("@date")) );

    bool bSendBuffer=varSend && varSend->length()>0 ? true: false;

    if( state==304 ) {
        var->add( "Content-Length: 0\r\n");
    } else if( bSendBuffer ) {
        var->format(1024, "Content-Length: %d\r\n", varSend->length());
    } else  {
        UL64 filelen = node->getLong("fileLength");
        if( filelen>0 )
            var->format(1024, "Content-Length: %d\r\n", (int)filelen );
    }
    StrVar* modify = node->gv("@modify");
    if( modify && modify->length()>0 ) {
        var->format(1024, "Last-Modified: %s\r\n", modify->get() );
    } else {
        var->format(1024, "Last-Modified: %s\r\n", httpDate(node->GetVar("@date")) );
    }

#ifdef KEEPALIVE_USE

    LPCC connection=node->get("Connection");
    bool kalive = ccmp(connection,"keep-alive") || node->cmp("Keep-Alive","115") ? true: false;
    if( kalive ) { // && state==200 && filelen>0
        int useCount = node->getInt("@useCount"), maxNum=KEEPALIVE_MAX; // (state==304) ? 5 : 10; //KEEPALIVE_MAX
        if( useCount < maxNum ) {
            var->format(1024, "Keep-Alive: timeout=%d, max=%d\r\nConnection: keep-alive\r\n", KEEPALIVE_SEC, (maxNum-useCount) );
        } else {
            qDebug(">> HTTP NODE == Keep-Alive url: %s (useCount:%d)==\n", node->get("url"), useCount );
            var->add("Connection: close\r\n");
        }
    } else {
        var->add("Connection: close\r\n");
    }
#else
    var->add("Connection: close\r\n");
#endif

    LPCC charset = node->get("@charset");
    if( slen(charset) ) {
        var->format(1024, "Content-Type: %s; charset=%s\r\n", getHttpContentType(node->get("@ext")), charset );
    } else {
        var->format(1024, "Content-Type: %s\r\n", getHttpContentType(node->get("@ext")) );
    }

    LPCC cache=node->get("Cache-Control");
    if(  slen(cache) ) {
        var->format(1024, "Cache-Control: %s\r\n", cache );
    }
    StrVar* sv = node->gv("@attachHeader");
    if( sv ) {
        XParseVar pv(sv);
        while( pv.valid() ) {
            LPCC value=pv.findEnd("\n").v();
            if( slen(value) ) {
                var->format(1024, "%s\r\n", value);
            }
        }
    }
    sv=node->gv("@setCookies");
    if( sv ) {
        XParseVar pv(sv);
        while( pv.valid() ) {
            LPCC value=pv.findEnd("\n").v();
            if( slen(value) ) {
                var->format(1024, "Set-Cookie: %s\r\n", value);
            }
        }
    }

    // qDebug("#0#was send file: %s (url:%s, ext:%s, state:%d)\n", var->get(), node->get("url"), node->get("@ext"), state );
    var->add("\r\n");
    if( node->send(var) ) {
        // qDebug("httpSend %s-------------------\n%s---------------------------------\n",node->get("@filenm"), var->get());
        // if( state==200 ) {}
        if( state==304 ) return true;
        if( bSendBuffer ) {
            node->send((BYTE*)varSend->get(), varSend->length() );
            varSend->reuse();
        } else if( !httpSendFile(node) ) {
            return false;
        }
    } else {
        qDebug("#0#http send fail (URL:%s)\n", node->get("url") );
        return false;
    }
    qint64 dist = GetTickCount() - node->getLong("@pageStartTick");
    if(dist>1500 ) {
        qDebug("#0#http send duration time %lld(ms)\n", dist );
    }
    return true;
}


void httpParseParam(ConnectNode* node, LPCC param ) {
    LPCC key=NULL, val=NULL;
    TreeNode* cf=node->config();
    char* params=(char*)(param);
    int pos=IndexOf(params,'=');
    node->setBool("@paramUse", true);
    while(pos>0) {
        params[pos]=0;
        key=getUrlDecode(params,NULL);
        params+=pos+1;
        pos=IndexOf(params,'&');
        if(pos>0) {
            params[pos]=0;
        }
        val=getUrlDecode(params,NULL);
        // qDebug("xxxx %s=%s (%s, %d)\n", key,val, params, pos);
        cf->set(key, val);
        if(pos>0 ) {
            params+=pos+1;
            pos=IndexOf(params,'=');
        } else {
            break;
        }
    }
}

//
//
//
inline int readNodeVar(ConnectNode* node, StrVar* var, UL64 size) {
    FTRY {
        XSocket* sck = node->getSocket();
        if( sck && sck->IsValid() ) {
            int recv = 0;
            int remain = min((UL64)SOCK_BUF_SIZE,size-var->length() );
            if( remain<=0 )
                return 2;
            BYTE* buf = (BYTE*)var->memalloc(remain);
            recv = sck->Receive((LPVOID)buf,remain);
            var->movepos(recv);
            return 1;
        }
    } FCATCH( FSocketException, ex ) {
        node->GetVar("@error")->set("readNodeVar error = ").add(" ==== ").add(ex.Get());
    }
    return 0;
}


U32 parsePostData(ConnectNode* node, XParseVar& pv, StrVar* decode, U32 flag) {
    return 0;
}

U32 parseMultiPartData(ConnectNode* node, XParseVar& pv, UL64 size) {
    return 0;
}


void setHttpContentType( TreeNode* node) {
    if( node==NULL ) return;
    struct tagMimeType
    {
        char id[8] ;
        char data[32] ;
    } mt[] = {
        { "htm", 	    "text/html"},
        { "html", 	    "text/html"},
        { "xss", 	    "text/html"},
        { "doc", 	    "application/msword"},
        { "xls", 	    "application/msword"},
        { "xlsx", 	    "application/msword"},
        { "bin", 	    "application/octet-stream"},
        { "dll", 	    "application/octet-stream"},
        { "exe", 	    "application/octet-stream"},
        { "pdf", 	    "application/pdf"},
        { "p7c", 	    "application/pkcs7-mime"},
        { "ai", 		"application/postscript"},
        { "eps", 	    "application/postscript"},
        { "ps", 		"application/postscript"},
        { "rtf", 	    "application/rtf"},
        { "fdf", 	    "application/vnd.fdf"},
        { "arj", 	    "application/x-arj"},
        { "gz", 		"application/x-gzip"},
        { "class", 	    "application/x-java-class"},
        { "js", 		"application/x-javascript"},
        { "json", 		"application/x-javascript"},
        { "lzh", 	    "application/x-lzh"},
        { "lnk", 	    "application/x-ms-shortcut"},
        { "tar", 	    "application/x-tar"},
        { "hlp", 	    "application/x-winhelp"},
        { "cert", 	    "application/x-x509-ca-cert"},
        { "zip", 	    "application/zip"},
        { "cab", 	    "application/x-compressed"},
        { "arj", 	    "application/x-compressed"},
        { "aif", 	    "audio/aiff"},
        { "aifc", 	    "audio/aiff"},
        { "aiff", 	    "audio/aiff"},
        { "au", 		"audio/basic"},
        { "snd", 	    "audio/basic"},
        { "mid", 	    "audio/midi"},
        { "rmi", 	    "audio/midi"},
        { "mp3", 	    "audio/mpeg"},
        { "vox", 	    "audio/voxware"},
        { "wav", 	    "audio/wav"},
        { "ra", 		"audio/x-pn-realaudio"},
        { "ram", 	    "audio/x-pn-realaudio"},
        { "svg", 	    "image/svg+xml"},
        { "bmp", 	    "image/bmp"},
        { "png", 	    "image/png"},
        { "gif", 	    "image/gif"},
        { "jpeg", 	    "image/jpeg"},
        { "jpg", 	    "image/jpeg"},
        { "tif", 	    "image/tiff"},
        { "tiff", 	    "image/tiff"},
        { "xbm", 	    "image/xbm"},
        { "wrl", 	    "model/vrml"},
        { "c", 		    "text/plain"},
        { "cpp", 	    "text/plain"},
        { "def", 	    "text/plain"},
        { "h", 		    "text/plain"},
        { "txt", 	    "text/plain"},
        { "rtx", 	    "text/richtext"},
        { "rtf", 	    "text/richtext"},
        { "java", 	    "text/x-java-source"},
        { "css", 	    "text/css"},
        { "mp4", 	    "video/mp4"},
        { "mpeg", 	    "video/mpeg"},
        { "mpg", 	    "video/mpeg"},
        { "mpe", 	    "video/mpeg"},
        { "avi", 	    "video/msvideo"},
        { "asf", 	    "video/msvideo"},
        { "asx", 	    "video/msvideo"},
        { "wma", 	    "video/msvideo"},
        { "wmv", 	    "video/msvideo"},
        { "mov", 	    "video/quicktime"},
        { "qt", 		"video/quicktime"},
        { "shtml", 	    "wwwserver/html-ssi"},
        { "asa", 	    "wwwserver/isapi"},
        { "asp", 	    "wwwserver/isapi"},
        { "cfm", 	    "wwwserver/isapi"},
        { "dbm", 	    "wwwserver/isapi"},
        { "isa", 	    "wwwserver/isapi"},
        { "plx", 	    "wwwserver/isapi"},
        { "url", 	    "wwwserver/isapi"},
        { "cgi", 	    "wwwserver/isapi"},
        { "php", 	    "wwwserver/isapi"},
        { "wcgi", 	    "wwwserver/isapi"}
    } ;

    for( int n=0, size=asizeof(mt);n<size;n++)
        node->set(mt[n].id, mt[n].data);
}

inline bool inlineProxyCheck(ConnectNode* conn, StrVar* var, int sp, int ep) {
    XParseVar pv(var, sp, ep);
    LPCC ext=NULL;
    LPCC method=pv.findEnd(" ").v();
    char* url=(char*)pv.findEnd(" ").v();
    int pos=IndexOf(url,'?');
    if(pos>0) {
        httpParseParam(conn, url+pos+1);
        url[pos]=0;
        // qDebug("#inlineProxyCheck %s, %d, %s\n", url, pos, url+pos+1);
    }
    pos=LastIndexOf(url,'.');
    if(pos>0 ) {
        ext=url+pos+1;
    }
    conn->set("method", method);
    conn->set("url", url);
    conn->set("@ext", ext);
    return false;
}

bool httpParseHeader(ConnectNode* node ) {
    StrVar* var=node->read();
    if( var==NULL ) {
        // qDebug("#0#httpParseHeader recv error\n" );
        return false;
    }
    int rsize = var->length();
    if( rsize==0 ) {
        // qDebug("#0#httpParseHeader recv size==0\n" );
        return false;
    }
    int pos = var->findPos("\r\n\r\n");
    if( pos==-1 ) { //  || var->charAt(0)=='-'
        qDebug("#0#httpParseHeader error (URL:%s)\n", node->get("url"));
        return false;
    }	
    char* buf = var->get();
    XParseVar pv(var,0,pos);
	pos+= 4;
    node->setNodeFlag(FLAG_USE);
    if( node->isNodeFlag(FLAG_SKIP)) {
        node->clearNodeFlag(FLAG_SKIP);
    }
	for(int n=0; n<64 && pv.valid(); n++ ) {
        pv.findEnd("\n");
        if( n==0 ) {
            if(inlineProxyCheck(node, var, pv.prev, pv.cur) ) {
                StrVar* sendVar=node->getSendVar()->reuse();
                var->setpos(pos);
                if(rsize>pos) {
					LPCC ctt=buf+pos;
					sendVar->set(ctt,rsize-pos);
                }
                return false;
            }
        } else {
            char* line=(char*)pv.v();
            int p = IndexOf(line,':');
            if( p>0 ) {
                line[p] = 0;
                LPCC code = Trim(line);
                node->set(code,Trim(line+p+1));
            }
        }
    }
	LPCC ctt=buf+pos;
    StrVar* sendVar=node->getSendVar();
    sendVar->reuse();
    if( node->gv("@httpSendData") ) {
        node->gv("@httpSendData")->reuse();
    }
	// var->setpos(pos);
    node->setLong("@pageStartTick", GetTickCount());
    /*
	TreeNode* proxys=getTreeNode("config","proxys");
	if( proxys && proxys->childCount() ) {
		LPCC url=node->get("url");
		for(int n=0; n<proxys->childCount(); n++) {
			TreeNode* cur=proxys->child(n);
			LPCC proxyUri=cur->get("uri");
			if(slen(proxyUri) && StartWith(url, proxyUri)>0) {
				node->GetVar("@proxyNode")->setVar('n',0,(LPVOID)cur);
				StrVar* header=node->GetVar("@proxyHeader");
				header->set(var,0,pos);
				if( rsize>pos ) {
					header->add(ctt,rsize-pos);
				}
				qDebug("xxxxxxxx url:%s, proxyUri:%s header:%s xxxxxxxxx", url, proxyUri, header->get() );
				return 0;
			}
		}
	}
    */
	LPCC method=node->get("method");
    if( ccmp(method,"GET") || ccmp(method,"OPTIONS")  || ccmp(method,"PUT") || ccmp(method,"PATCH") ) {


    } else if( ccmp(method,"POST") ) {        
		XParseVar pv(node->GetVar("Content-Type"));
		if( rsize>pos ) {
			sendVar->add(ctt,rsize-pos);
		}
		if( pv.StartWith( "multipart/form-data",-1,1) ) {
			if( pv.NextChar()==';')
				pv.start++;
			pv.NextChar();
			if( pv.StartWith("boundary",-1,1) && pv.NextChar()=='=' ) {
				pv.start++;
				node->set("boundary",pv.get());
			}
			node->setNodeFlag(FLAG_RUN|WOKRER_UPLOAD);
			qDebug("###multipart read check### readLength: %d, (%d)\n", sendVar->length(), node->getInt("Content-Length") );
		} else {
			// httpParseParam(node, sendVar->get());
		}
    } else {
        qDebug("\n######### HTTP header method not define : %s ############## (sp:%d, ep:%d)\n", node->get("method"), pos, rsize);
        if( rsize>pos ) {
            sendVar->add(ctt,rsize-pos);
        } else {
            return false;
        }
    }
    return true;
}
bool wasCallApiFun(ConnectNode* node, LPCC url) {
    LPCC param=NULL, serviceCode=NULL;
    if( ccmp(url,"/api") ) {
         serviceCode="all";
    } else {
        param=url+5;
        int pos=IndexOf(param,'/');
        if(pos>0 ) {
            serviceCode=gBuf.add(param,pos);
            param+=pos+1;
        } else {
            serviceCode=param;
        }
    }
    StrVar* sv=getStrVar("fsrc","apiController");
    ConnectNode* root = node->parent();
    if( root && SVCHK('f',1) ) {
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        sv=root->gv("@c");
        if( SVCHK('v',0) ) {
            HttpServerThread* ht=(HttpServerThread*)SVO;
            XFuncNode* fnCur = ht->getFuncNode(node, fsrc, NULL);
            fnCur->set("service", serviceCode);
            fnCur->set("uri", param);
            fnCur->set("url", url);
            fnCur->set("_funcName_", "apiController");
            if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                node->setNodeFlag(FLAG_INIT);
                return true;
            } else {
                qDebug("#9#jwas apiController funcNode thread error url:%s --------\n", url);
            }
        } else {
            qDebug("#0#jwas apiController HttpServerThread error url:%s --------\n", url);
        }
    } else {
        qDebug("#9#api apiController function not found url:%s --------\n", url);
    }
    return false;
}
bool wasCallProxyFun(ConnectNode* node, LPCC url) {
    StrVar* sv=getStrVar("fsrc","proxyController");
    ConnectNode* root = node->parent();
    if( root && SVCHK('f',1) ) {
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        node->set("@url", url);
        node->setNodeFlag(FLAG_SETEVENT);
        sv=root->gv("@c");
        if( SVCHK('v',0) ) {
            TreeNode* map=getProxyMap();
            HttpServerThread* ht=(HttpServerThread*)SVO;
            XFuncNode* fnCur = ht->getFuncNode(node, fsrc, NULL);
            StrVar* var=node->GetVar("@url");
            fnCur->GetVar("url")->setVar('s',0,(LPVOID)var).addInt(0).addInt(var->length()).addInt(0).addInt(var->length());
            fnCur->GetVar("proxy")->setVar('n',0,(LPVOID)map);
            fnCur->set("_funcName_", "proxyController");
            if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                node->setNodeFlag(FLAG_INIT);
                return true;
            } else {
                qDebug("#9#jwas proxyController funcNode thread error url:%s --------\n", url);
            }
        } else {
            qDebug("#0#jwas proxyController HttpServerThread error url:%s --------\n", url);
        }
    } else {
        qDebug("#9#proxyController function not found url:%s --------\n", url);
    }
    return false;
}

bool httpParseUrl(ConnectNode* node ) {
	char* url = getUrlDecode(node->get("url"),NULL); // &temp);
	if(slen(url)==0 ) {
		qDebug("#0#httpParseUrl root node not found (url:NULL)\n");
		return false;
	}

	ConnectNode* root = node->parent();
    if( root==NULL ) {
        qDebug("#0#httpParseUrl root node not found (url:%s)\n", url);
        return false;
    }
    TreeNode* cf=root->config();
    if( cf==NULL ) {
        qDebug("#0#httpParseUrl error root cofing not define (url:%s)", url);
        return false;
    }

    StrVar* sv=NULL;
    LPCC method=node->get("method");
    if(ccmp(method,"OPTIONS") || ccmp(method,"PUT") || ccmp(method,"PATCH") ) {
        sv=getStrVar("fsrc","methodController");
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            sv=root->gv("@c");
            if( SVCHK('v',0) ) {
                HttpServerThread* ht=(HttpServerThread*)SVO;
                XFuncNode* fnCur = ht->getFuncNode(node, fsrc, NULL);
                fnCur->set("url", url);
                fnCur->set("_funcName_", "methodController");
                if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                    node->setNodeFlag(FLAG_INIT);
                    return true;
                } else {
                    qDebug("#9#jwas apiController funcNode thread error url:%s --------\n", url);
                }
            } else {
                qDebug("#0#jwas apiController HttpServerThread error url:%s --------\n", url);
            }
        } else {
            qDebug("#9#api apiController function not found url:%s --------\n", url);
        }
        return false;
    }
    // StrVar temp;

    LPCC subhost=NULL;
    LPCC hostUri=NULL;
    subhost=url+1;
    int pos=IndexOf(subhost,'/');
    if(pos>0 ) {
        hostUri=subhost+pos;
        subhost=gBuf.add(subhost, pos);
    }
    if( ccmp(subhost,"api") ) {
        return wasCallApiFun(node, url );
    }

    if(ccmp(subhost,"proxy")) {
        return wasCallProxyFun(node, url);
    }
    char* uri=url;
    LPCC ext=NULL;
    LPCC rootPath=NULL;
    sv=cf->gv("pathInfo");
    if( SVCHK('n',0)) {
        TreeNode* pathInfo=(TreeNode*)SVO;
        sv=pathInfo->gv(subhost);
        if( sv && sv->length() ) {
            uri=(char*)hostUri;
            rootPath=sv->get();
            qDebug("sub host %s %s (var:%s, path:%s)", sv->get(), uri, subhost, rootPath);
        }
    }
    if( slen(rootPath)==0 ) {
        rootPath=cf->get("rootPath");
    }
    StrVar* path=node->GetVar("@filenm")->reuse();
    path->add(rootPath).add(uri);

    LPCC filenm=path->get();
    QFileInfo fiDir(KR(filenm));
    if(fiDir.isDir()) {
        StrVar* urlVal=node->GetVar("url");
        LPCC defName=cf->get("defaultFile");
        if(slen(defName)==0) {
            defName="index.html";
        }
        if(ccmp(uri,"/")) {
            path->add(defName);
            urlVal->add(defName);
        } else {
            path->add("/").add(defName);
            urlVal->add("/").add(defName);
        }
        filenm=path->get();
        ext="html";
        node->set("@ext", ext);
        qDebug("html default file name: %s\n", node->get("@filenm"));
    } else {
        ext=node->get("@ext");
    }
    QFileInfo fi(KR(filenm));
    if( fi.isFile() ) {
        return httpSendFileName(node, filenm, url );
    }
    TreeNode* uriMap=getUrlMap(root);
    sv= uriMap ? uriMap->gv(uri):NULL;
    if(SVCHK('f',1)) {
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        sv=root->gv("@c");
        // qDebug("#0#>> load web page func URI:%s (ext:%s) \n", uri, ext);
        if( SVCHK('v',0) ) {
            HttpServerThread* ht=(HttpServerThread*)SVO;
            XFuncNode* fnCur = ht->getFuncNode(node, fsrc );
            if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                node->setNodeFlag(FLAG_INIT);
                return true;
            } else {
                qDebug("#9#web url map function error URI:%s --------\n", uri);
            }
        } else {
            qDebug("#9#web url map function thread error URI:%s --------\n", uri);
        }
    }
    qDebug("web resource not found file: %s\n", node->get("@filenm") );
    node->setInt("@state", 404);
    return httpSendBuffer(node, NULL,"notfound");
}

//
//
//
DWORD IGIMSAPI socketHttpReadFunc( LPVOID lpConnectNode ) {
    ConnectNode* node = (ConnectNode*)lpConnectNode;
    if( node->isNodeFlag(FLAG_SINGLE)) {
        if(node->isNodeFlag(FLAG_CALL)) {
            qint64 tick = node->xcf->getDouble("@lastSendTick");
            if( tick==0 ) return 0;
            if( tick>0 ) {
                qint64 dist = GetTickCount()- tick;
                if( dist<1500 ) return 0;
            }
        }
        ConnectNode* root = node->parent();
        TreeNode* param=node->xcf;
        if( root && param ) {
            XFuncSrc* fsrc=NULL;
            StrVar* sv=root->gv("@clientCallback");
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
            node->setNodeFlag(FLAG_RUN);
            sv=root->gv("@c");
            if( SVCHK('v',0) && fsrc ) {
                HttpServerThread* ht=(HttpServerThread*)SVO;
                XFuncNode* fnCur = ht->getFuncNode(node, fsrc);
                fnCur->GetVar("proxy")->setVar('n',0,(LPVOID)getProxyMap());
                fnCur->GetVar("client")->setVar('v',1,(LPVOID)node);
                // qDebug("client read proc tick:%ld idx:%d FLAG:0x%X", node->getTick(), node->xclientIndex, node->xflag );
                if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                    sv=root->gv("@classModule");
                    if(SVCHK('n',0)) {
                        TreeNode* thisNode=(TreeNode*)SVO;
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                    }
                    node->setNodeFlag(FLAG_SETEVENT);
                    node->clearNodeFlag(FLAG_CALL);
                    node->setTick();
                } else {
                    qDebug("#9#serverRecvData call error params:%s --------\n", param->log() );
                }
            } else {                
                qDebug("#0#serverRecvData Thread error params:%s --------\n", param->log() );
            }
        } else {            
            qDebug("#9#serverRecvData function not define params:%s --------\n", param->log() );
        }
        return 1;
    }
    node->setNodeFlag(FLAG_RUN);
    return (DWORD)httpRequest(node);
}

StrVar* sendHttpHeader(ConnectNode* conn, StrVar* rst, qint64 length, LPCC ctype, bool local) {
    int state = conn->getInt("@state");
    if( state==0 ) state=200;
    rst->reuse();

    LPCC contentType = conn->get("@contentType");
    if(slen(contentType)==0 ) {
        if( slen(ctype)==0 ) {
            ctype="html";
        }
        contentType = getHttpContentType(ctype);
    }
    LPCC cache=conn->get("Cache-Control");
    if(slen(cache) && state!=200 ) {
        state=200;
    }
    StrVar* sv=conn->gv("Range");
    if( sv && sv->length()>0 ) {
        XParseVar pv(sv);
        LPCC type=pv.NextWord(), start=NULL, end=NULL;
        if( ccmp(type,"bytes") && pv.ch()=='=' ) {
            pv.incr();
            start=pv.findEnd("-").v();
            end=pv.findEnd("\n").v();
            conn->set("rangeStart", start);
            conn->set("rangeEnd", end);
        }
        qint64 startPos=(qint64)toUL64(conn->gv("rangeStart"));
        qint64 endPos=(qint64)toUL64(conn->gv("rangeEnd"));
        if( endPos==0 ) {
            int mega=1024*1024;
            int divid=(int)( length / (100*mega) );
            if( divid==0 ) {
                divid=2;
            } else if( divid<2 ) {
                divid=3;
            } else if( divid<3 ) {
                divid=4;
            } else if( divid<4 ) {
                divid=5;
            } else {
                divid=6;
            }
            qint64 chunkSize= divid*mega;
            endPos=startPos + chunkSize;
            // conn->set("Connection", "close");
        }
        if( endPos==startPos || endPos>=length ) {
            endPos=length-1;
        }
        if( startPos>=endPos ) {
            if(endPos==11) {
                // qDebug("start pos error %lld %lld (%s %s)", startPos, endPos, start, end);
                return NULL;
            }
            startPos= endPos- (1024*1024);
        }
        conn->GetVar("rangeEnd")->setVar('1',0).addUL64((UL64)endPos );

        if( startPos>length ) {
            // qDebug("start position error %lld %lld", startPos, length);
            rst->format(1024,"HTTP/1.1 %d Requested Range Not Satisfiable\r\nDate: %s\r\nServer: baro\r\n",
                 416,
                 httpDate(conn->GetVar("@date"))
            );
            rst->format(1024, "Content-Range: bytes %lld-%lld/%lld\r\n", startPos, endPos, length );
        }
        rst->format(1024,"HTTP/1.1 %d Partial Content\r\nDate: %s\r\nServer: baro\r\n",
            206,
            httpDate(conn->GetVar("@date")) );
        rst->add("Accept-Ranges: bytes\r\n" );
        rst->format(1024, "Content-Range: bytes %lld-%lld/%lld\r\n", startPos, endPos, length );
        length= endPos - startPos +1;
        rst->format(1024, "Content-Length: %lld\r\n", length );
        rst->format(1024, "Content-Type: %s\r\n", contentType );
        rst->format(1024, "Total-Size: %lld\r\n", (endPos-startPos) );
    } else {
        rst->format(1024,"HTTP/1.1 %d %s\r\nDate: %s\r\nServer: baro\r\n",
            state,
            httpStatusCode(state),
            httpDate(conn->GetVar("@date")) );

        rst->format(1024, "Content-Length: %lld\r\n", length );
        if(conn->isBool("@checkBlob")) {
            rst->format(1024, "Content-Type: %s\r\n", contentType);
        } else {
            LPCC charset="utf-8";
            if(conn->gv("@charset")) charset=conn->get("@charset");
            rst->format(1024, "Content-Type: %s; charset=%s\r\n", contentType, charset );
        }
        if(slen(cache)) {
            rst->format(1024, "Last-Modified: %s\r\n", conn->get("@date") );
        } else {
            StrVar* modify = conn->gv("@modify");
            if( modify && modify->length()>0 ) rst->format(1024, "Last-Modified: %s\r\n", modify->get() );
            else rst->format(1024, "Last-Modified: %s\r\n", httpDate(conn->GetVar("@date")) );
        }
    }
    LPCC connection=conn->get("Connection");
    bool kalive = ccmp(connection,"keep-alive") || conn->cmp("Keep-Alive","115") ? true: false;
    if( kalive && state==200 ) {
        int useCount = conn->getInt("@useCount"), maxNum=KEEPALIVE_MAX; // (state==304) ? 5 : 10; //KEEPALIVE_MAX
        if( useCount < maxNum ) {
            rst->format(1024, "Keep-Alive: timeout=%d, max=%d\r\nConnection: keep-alive\r\n", KEEPALIVE_SEC, (maxNum-useCount) );
        } else {
            qDebug(">> HTTP NODE == Keep-Alive END url: %s (useCount:%d)==\n", conn->get("url"), useCount );
            rst->add("Connection: close\r\n");
        }
    } else {
        rst->add("Connection: close\r\n");
    }

    sv = conn->gv("uriOrigin");
    if( sv ) {
        LPCC uri=toString(sv);
        rst->format(1024, "Uri-Origin: %s\r\n", uri );
    }

    if(  slen(cache) ) {
        rst->format(1024, "Cache-Control: %s\r\n", cache );
    }

    sv = conn->gv("@attachHeader");
    if( sv ) {
        XParseVar pv(sv);
        // qDebug("http header set %s\n", sv->get() );
        while( pv.valid() ) {
            LPCC value=pv.findEnd("\n").v();
            if( slen(value) ) {
                rst->format(1024, "%s\r\n", value);
            }
        }
    }

    sv=conn->gv("@setCookies");
    if( sv ) {
        XParseVar pv(sv);
        qDebug("http header set cookies %s\n", sv->get() );
        while( pv.valid() ) {
            LPCC value=pv.findEnd("\n").v();
            if( slen(value) ) {
                rst->format(1024, "Set-Cookie: %s\r\n", value);
            }
        }
    }
    rst->add("\r\n");
    if( local==false ) {
        // qDebug("http send LENGTH:%d", rst->length());
        conn->send(rst);
    }
    return rst;
}

DWORD IGIMSAPI socketHttpAcceptFunc( LPVOID lpConnectNode ) {
    ConnectNode* root = (ConnectNode*)lpConnectNode;
    ConnectNode* cur =  root->acceptClient(socketHttpReadFunc);
    if(cur ) cur->setInt("@useCount",0);
    return 0;
}


bool callHttpFunc(ConnectNode* node, LPCC url, int check) {
    // qDebug("----- call http func url:%s fileName:%s -----------\n", url, node->get("@filenm") );
    if( node==NULL || slen(url)==0 ) return false;
    ConnectNode* root = node->parent();
    StrVar* sv=getStrVar("fsrc","webFileFilter");
    if( root && SVCHK('f',1) ) {
        XFuncSrc* fsrc=fsrc=(XFuncSrc*)SVO;
        sv=root->gv("@c");
        if( SVCHK('v',0) ) {
            HttpServerThread* ht=(HttpServerThread*)SVO;
            XFuncNode* fnCur = ht->getFuncNode(node, fsrc);
            node->config()->set("url", url);
            if( fnCur && fnCur->isNodeFlag(FLAG_WEB_CALL) ) {
                return true;
            } else {
                qDebug("#9#jwas http filter thread error url:%s --------\n", url);
            }
        } else {
            qDebug("#0#jwas http filter Thread error url:%s --------\n", url);
        }
    } else {
        qDebug("#9#jwas http filter error url:%s --------\n", url);
    }
    return false;
}

int httpRequest(ConnectNode* node ) {
    if( node->isNodeFlag(FLAG_END) ) {
        node->clearNodeFlag(FLAG_END);
        // qDebug("#0#request socket reuse idx:%d tick:%d flags:0x%x\n", node->xclientIndex, node->getTick(), node->xflag );
    }
    if( httpParseHeader( node ) ) {
		// LPCC uid=node->get("UploadUid");
        node->setTick();
        node->clearNodeFlag(FLAG_END);
        // node->setTick();
        if( !httpParseUrl(node) ) {
            qDebug("#0#parse url error node=%s %s\n", node->get("url"), node->get("ip") );
            node->closeSocket();
        }
    } else if( node->valid() ) {
        node->closeSocket();
	}
    return 0;
}

