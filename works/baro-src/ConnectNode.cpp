#include "ConnectNode.h"
#include "WorkerThread.h"
#include "HttpNode.h"
#include "../util/user_event.h"
#include "../baroscript/func_util.h"
#include "../fobject/FMutex.h"
// #define SOCKET_ENDNODE_USE

#ifdef WINDOW_USE
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#ifndef QT5_USE
#define MAX_MESSAGE_BUFFER_SIZE 0x9FFF
#endif

FMutex gMutexClientNode;
FMutex gMutexClientClose;

void closeRunFuncNode(XFuncNode* fn ) {
    /*
    XFuncNode* pfn=fn;
    while( pfn ) {
        if( pfn->isNodeFlag(FLAG_WEB_CALL) ) {
            if( pfn->isNodeFlag(FLAG_RUN) )
                pfn->clearNodeFlag( FLAG_RUN );
            pfn->clearNodeFlag( FLAG_WEB_CALL  );
            int idx=pfn->getInt("@poolIndex");
            if( idx>0 ) {
                for( int n=0;n<idx;n++ ) {
                    StrVar* sv=pfn->gv(cget(16,"@pool%d",n));
                    if( SVCHK('n',0) ) {
                        TreeNode* m=(TreeNode*)SVO;
                        if( m->isNodeFlag(FLAG_RUN) ) {
                            m->clearNodeFlag(FLAG_RUN);
                        }
                    }
                }
                pfn->getInt("@poolIndex",0);
            }
            break;
        }
        pfn = pfn->parent();
    }
    */
}

int initConnectNode( ConnectNode* conn, bool keepCheck, bool closeCheck ) {
    if(conn==NULL) return 0;
    /*
    LPCC cache=conn->get("Cache-Control");
    if(slen(cache)) {
        qDebug("xxx initConnectNode cache control xxx idx:%d (dist:%d, flag:0x%X, url:%s)\n", conn->xclientIndex, conn->getTick() , conn->xflag, conn->get("url"));
        conn->closeClient();
        return 0;
    }
    */
    int ret=0;
    if( keepCheck ) {
        TreeNode* cf=conn->xcf;
        int useCount = conn->getInt("@useCount");
        StrVar* sv=conn->gv("@database");
        if( SVCHK('n',0) ) {
            TreeNode* model=(TreeNode*)SVO;
            // qDebug("#0#init client data base release db url:%s\n", conn->get("url") );
            model->clearNodeFlag(FLAG_USE);
        }
        qDebug("xxx useCount==%d xxx idx:%d (dist:%ld, flag:0x%X)\n", useCount, conn->xclientIndex, conn->getTick() , conn->xflag);
		conn->reuse();
        conn->xsend.reuse();
        conn->xcontent.reuse();
        conn->xtype = conn->xstat = 0;
        if( conn->xparent && cf ) {
            cf->reset(true);
            cf->reuse();
        }
        conn->xflag=FLAG_SET;
        conn->setTick();
        // conn->setNodeFlag(FLAG_END);
        conn->setInt("@useCount", useCount+1);
        XSocket* sock=conn->getSocket();
        if(sock ) {
#ifdef _WIN32
            DWORD dwRet=0;
            tcp_keepalive tcpkl;
            tcpkl.onoff = 1; // KEEPALIVE ON
            tcpkl.keepalivetime = 5000; // 2초 마다 KEEPALIVE 신호를 보내겠다. (Default 는 2시간이다)
            tcpkl.keepaliveinterval = 5000; // keepalive 신호를 보내고 응답이 없으면 1초마다 재 전송하겠다. (ms tcp 는 10회 재시도 한다)
            WSAIoctl(sock->GetSocketDescriptor(),
                         SIO_KEEPALIVE_VALS, &tcpkl, sizeof(tcp_keepalive),
                         0,0, &dwRet, NULL, NULL);
#else
            int val = 1;
            sock->Flush();
            setsockopt(sock->GetSocketDescriptor(), SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val) );
#endif
        }
        ret=2;
    } else if( conn->valid() ) {
        if(conn->xflag==FLAG_END ) {
            // qDebug("xxx initConnectNode reuse xxx idx:%d (dist:%d, flag:0x%X)\n", conn->xclientIndex, conn->getTick() , conn->xflag);
        } else {
            if(closeCheck) {
                conn->closeClient();
            } else {
                conn->setNodeFlag(FLAG_END);
                qDebug("xxx initConnectNode close check xxx idx:%d (dist:%d, flag:0x%X)\n", conn->xclientIndex, conn->getTick() , conn->xflag);
            }
        }
        ret=1;
    } else {
        qDebug("xxx initConnectNode check xxx idx:%d (dist:%d, flag:0x%X)\n", conn->xclientIndex, conn->getTick() , conn->xflag);
    }
    return ret;
}


/************************************************************************/
/*  xsocket                                                    */
/************************************************************************/

#define SCK_CALL_ACCEPT	0x10
XSocket::XSocket(U32 flag, ConnectNode* node) : FSocket(), m_maxSock(INVALID_SOCKET),
    xflag(flag), xnode(node), xendNode(NULL), xfunc(NULL)//, xworkers(NULL)
{
    Init(AF_INET, SOCK_STREAM, 0);
}

XSocket::XSocket(SOCKET sck) : FSocket(sck), m_maxSock(INVALID_SOCKET),
    xflag(0), xnode(NULL), xendNode(NULL), xfunc(NULL)//, xworkers(NULL)
{

}

XSocket::~XSocket() {

}

bool XSocket::dispatchWas( ConnectNode* root,ListObject<ConnectNode*>& list, struct timeval* timeout ) FTHROW( FSocketException ) {
    // long timechk = KEEPALIVE_SEC * 1000;
    SOCKET maxSck = GetSocketDescriptor();

    int fdNum=1, offset=0;
    FD_ZERO(&m_readFd);
    FD_SET(GetSocketDescriptor(),&m_readFd);
#ifdef SOCKET_ENDNODE_USE
    if( xendNode ) {
        offset = xendNode->row();
    }
#endif
    int n=offset, cnt=root->childCount();
    ConnectNode* cur = NULL;
    XSocket* sck = NULL;
    for( ; n<cnt ; n++ ) {
        cur = root->child(n);
        sck = cur->getSocket();
        if( sck && sck->IsValid() ) {
            if( maxSck < sck->GetSocketDescriptor() )
                maxSck = sck->GetSocketDescriptor();
            if( fdNum==FD_SETSIZE ) {
                qDebug("#0#socket fd set error fd num:%d, index:%d\n", fdNum, n);
#ifdef SOCKET_ENDNODE_USE
                xendNode = root->child(n);
#endif
                break;
            }
            FD_SET(sck->GetSocketDescriptor(),&m_readFd);
            fdNum++;
        }
    }

#ifdef SOCKET_ENDNODE_USE
    if( xendNode && n==cnt-1 ) {
        xendNode = NULL;
    }
#endif
    // qDebug("dispatch =========================== %d\n",maxSck );
    // m_maxSock = maxSck;
    DWORD tick=GetTickCount();
    int ret = select( maxSck+1, (fd_set*)&m_readFd, NULL, NULL, timeout );
    // qDebug("dispatch select ########################## %d, fdNum:%d (max:%d, cnt:%d, read:0x%x)\n", ret, fdNum, maxSck, cnt, m_readFd);

    switch ( ret )
    {
    case -1 :
        {
            qDebug("dispatch select error result:%d (fdNum:%d, max:%d, cnt:%d, read:0x%x)\n", ret, fdNum, maxSck, cnt, m_readFd);
            FTHROW_EXCEPTION( FSocketException, FSocketException::SELECT_FAILURE );
        }; break;
    case 0:
        {
            /*
            int dist=(int)(GetTickCount()-tick);
            if( dist>1000 ) {
                qDebug("#0#dispatch select timeout (dist:%d, maxSocket:%d)\n", dist, maxSck);
            }
            */
        }; break;
    default :
        {
            if( FD_ISSET( GetSocketDescriptor(), &m_readFd ) ) {
                // qDebug("Accept Client ************************************* \n");
                xfunc((LPVOID)root);    // socketHttpAcceptFunc
                return true;
            }
            int count=root->childCount();
#ifdef SOCKET_ENDNODE_USE
            if( offset>0 ) {
                qDebug("#0#dispatch count check (offset:%d) %d==%d\n", offset, cnt, count);
            }
#endif
            for( n=offset ; n<count; n++ ) {
                cur = root->child(n);
                sck = cur->getSocket();
                if( sck==NULL ) {
                    qDebug("dispatch socket is null index:%d, socket:%d\n", n, cur->xclientIndex );
                    continue;
                }
                if( sck->IsValid() && FD_ISSET(sck->GetSocketDescriptor(),&m_readFd) && sck->xfunc ) {
                    sck->xfunc((LPVOID)cur); // socketHttpReadFunc
                }
            }
            // qDebug(". ");
        };
    }
    // CLEARBIT(xflag,SCK_CALL_ACCEPT);
    return true;
}

bool XSocket::dispatchProxy( ConnectNode* root,ListObject<ConnectNode*>& list, struct timeval* timeout ) FTHROW( FSocketException ) {
    // long timechk = KEEPALIVE_SEC * 1000;
    SOCKET maxSck = GetSocketDescriptor();
    ConnectNode* cur = NULL;
    XSocket* sck = NULL;
    int fdNum=1, n=0, cnt=root->childCount();
    FD_ZERO(&m_readFd);
    FD_SET(GetSocketDescriptor(),&m_readFd);
    for( ; n<cnt ; n++ ) {
        cur = root->child(n);
        sck = cur->getSocket();
        if( sck && sck->IsValid() ) {
            if( maxSck < sck->GetSocketDescriptor() )
                maxSck = sck->GetSocketDescriptor();
            FD_SET(sck->GetSocketDescriptor(),&m_readFd);
            fdNum++;
        }
    }
    bool chk=false;
    int ret = select( maxSck+1, (fd_set*)&m_readFd, NULL, NULL, timeout );
    switch ( ret )
    {
    case -1 :
        {
            qDebug("dispatch select error result:%d (fdNum:%d, max:%d, cnt:%d, read:0x%x)\n", ret, fdNum, maxSck, cnt, m_readFd);
            FTHROW_EXCEPTION( FSocketException, FSocketException::SELECT_FAILURE );
            chk=false;
        } break;
    case 0:
        {
            /*
            int dist=(int)(GetTickCount()-tick);
            if( dist>1000 ) {
                qDebug("#0#dispatch select timeout (dist:%d, maxSocket:%d)\n", dist, maxSck);
            }
            FTHROW_EXCEPTION( FSocketException, FSocketException::TIME_OUT_ATTEMPTING_CONNECTION );
            */

        } break;
    default :
        {
            if( FD_ISSET( GetSocketDescriptor(), &m_readFd ) ) {
                xfunc((LPVOID)root);    // socketHttpAcceptFunc
                chk=true;
            }
            int count=root->childCount();
            for( n=0 ; n<count; n++ ) {
                cur = root->child(n);
                if(cur->isNodeFlag(FLAG_SETEVENT)) continue;
                sck = cur->getSocket();
                if( sck==NULL ) {
                    qDebug("dispatch proxy socket is null index:%d, socket:%d\n", n, cur->xclientIndex );
                    continue;
                }
                if( sck->IsValid() && FD_ISSET(sck->GetSocketDescriptor(),&m_readFd) && sck->xfunc ) {
                    sck->xfunc((LPVOID)cur); // socketHttpReadFunc
                    if(!chk) chk=true;
                }
            }
        } break;
    }
    return chk;
}


bool XSocket::dispatchServer( ConnectNode* root,ListObject<ConnectNode*>& list, struct timeval* timeout ) FTHROW( FSocketException ) {
    // long timechk = KEEPALIVE_SEC * 1000;
    SOCKET maxSck = GetSocketDescriptor();
    ConnectNode* cur = NULL;
    XSocket* sck = NULL;
    int fdNum=1, n=0, cnt=root->childCount();
    FD_ZERO(&m_readFd);
    FD_SET(GetSocketDescriptor(),&m_readFd);
    for( ; n<cnt ; n++ ) {
        cur = root->child(n);
        if(cur->isNodeFlag(FLAG_CALL) ) {
            // qDebug("called %d fd:%d", n, fdNum);
            continue;
        }
        sck = cur->getSocket();
        if( sck && sck->IsValid() ) {
            if( maxSck < sck->GetSocketDescriptor() )
                maxSck = sck->GetSocketDescriptor();
            FD_SET(sck->GetSocketDescriptor(),&m_readFd);
            fdNum++;
        }
    }
    bool chk=false;
    int ret = select( maxSck+1, (fd_set*)&m_readFd, NULL, NULL, timeout );
    switch ( ret )
    {
    case -1 :
        {
            qDebug("dispatch select error result:%d (fdNum:%d, max:%d, cnt:%d, read:0x%x)\n", ret, fdNum, maxSck, cnt, m_readFd);
            FTHROW_EXCEPTION( FSocketException, FSocketException::SELECT_FAILURE );
        }; break;
    case 0:
        {
            /*
            int dist=(int)(GetTickCount()-tick);
            if( dist>1000 ) {
                qDebug("#0#dispatch select timeout (dist:%d, maxSocket:%d)\n", dist, maxSck);
            }
            FTHROW_EXCEPTION( FSocketException, FSocketException::TIME_OUT_ATTEMPTING_CONNECTION );
            */
        }; break;
    default :
        {
            if( FD_ISSET( GetSocketDescriptor(), &m_readFd ) ) {
                // qDebug("Accept Client ************************************* \n");
                xfunc((LPVOID)root);    // socketHttpAcceptFunc
                chk=true;
            }
            int count=root->childCount();
            for( n=0 ; n<count; n++ ) {
				cur = root->child(n);
                sck = cur->getSocket();
                if( sck==NULL ) {
                    qDebug("dispatch proxy socket is null index:%d, socket:%d\n", n, cur->xclientIndex );
                    continue;
				}
                if( sck->IsValid() && FD_ISSET(sck->GetSocketDescriptor(),&m_readFd) && sck->xfunc ) {
                    sck->xfunc((LPVOID)cur); // socketHttpReadFunc
                    if(!chk) chk=true;
                }
            }
        };
    }
    return chk;
}

XSocket*	XSocket::acceptClient( struct sockaddr* addr, int* addrlen, ConnectNode* node ) {
    if( node==NULL ) return NULL;
    FTRY {
#ifdef WINDOW_USE
        SOCKET sck = accept( GetSocketDescriptor(), addr, (int*)addrlen );
#else
        SOCKET sck = accept( GetSocketDescriptor(), addr, (socklen_t*)addrlen );
#endif
        if ( sck == INVALID_SOCKET ) {
            FTHROW_EXCEPTION( FSocketException, FSocketException::INVALID_SOCKET_HANDLE );
            node->config()->GetVar("@error")->set("accept socket invalid handle");
            return NULL;
        }
        FTRY {
            XSocket* socket=node->getSocket();
            if( socket==NULL ) {
                socket=FNEW XSocket(sck);
                node->setSocket( socket );
                // qDebug("xxx accept new socket fd:%d\n", sck);
            }
            if( m_maxSock<sck ) {
                m_maxSock = sck;
                // qDebug("xxx accept max socket fd:%d\n", sck);
            }
            if( socket ) {
                socket->SetSocketDescriptor(sck);
#ifdef _LINUX
                /*
                int enableKeepAlive = 1;
                int maxIdle = 5; // seconds
                int count = 2;  // send up to 3 keepalive packets out, then disconnect if no response
                int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
                setsockopt(sck, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
                setsockopt(sck, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
                setsockopt(sck, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));
                setsockopt(sck, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
                */
                /*
                struct timeval timeout;
                timeout.tv_sec=1;
                timeout.tv_usec=0;
                setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
                */
                int opt = 3;
                setsockopt(sck, SOL_SOCKET, SO_RCVLOWAT,&opt,sizeof(opt));
#else
                /*
                DWORD tm = 2500;
                setsockopt(sck, SOL_SOCKET, SO_RCVTIMEO,(const char*)&tm,sizeof(tm));
                */
#endif
            }
            return socket;
        } FCATCH( FMemoryException, ex ) {
            node->config()->GetVar("@error")->set("MemoryException");
            node->closeSocket();
        }
    } FCATCH( FSocketException,ex ) {
        // node->setNodeFlag(FLAG_ERROR);
        node->config()->GetVar("@error")->set("SELECT_FAILURE timeout");
        node->closeSocket();
    }
    return NULL;
}


ConnectNode::ConnectNode(TreeNode* cf) :
    XNode(4), xcf(cf) , xsock(0), xclientIndex(0), xtimeout(2000),
    xtype(0), xstat(0), xparent(NULL)
{
}

ConnectNode::~ConnectNode() {
    if( xcf==NULL ) {
        return;
    }
	if( !xcf->isNodeFlag(FLAG_PERSIST) && xcf->isNodeFlag(FLAG_NEW) ) {
        SAFE_DELETE(xcf);
    }
}

LPCC ConnectNode::setIp(const in_addr &addr) {
#ifdef _WIN32
    return GetVar("ip")->fmt("%u.%u.%u.%u",
        addr.s_net,
        addr.s_host,
        addr.s_lh,
        addr.s_impno);
#else
    char ip[32];
    inet_ntop(AF_INET, &addr.s_addr, ip, sizeof(ip));
    return GetVar("ip")->set((LPCC)ip).get();
#endif
}

void ConnectNode::setSocket(XSocket* s ) {
    if( s==NULL ) return;
    xsock = s;
    xsock->xnode = this;
}

void ConnectNode::setProcFunc(LPSOCKET_ROUTINE func, TreeNode* cf) {
    if( xsock ) {
        xsock->xfunc = func;
    }
    if( cf ) {
        xcf=cf;
    } else if( xcf==NULL ) {
        xcf=new TreeNode(2, true);
    }
}

void ConnectNode::initNode(int idx, LPSOCKET_ROUTINE func) {
    if( xsock && xsock->IsValid() ) {
        qDebug("initNode close socket (index:%d)\n", idx);
        closeSocket();
        xsock = NULL;
    }
    setProcFunc(func);
    reuse();
    // xsend.reuse();
    xcontent.reuse();
    xflag = 0;
    xtype = xstat = 0;
    xclientIndex = idx;
    xcf->reset(true);
}

ConnectNode* ConnectNode::addNode(LPSOCKET_ROUTINE func) {
    ConnectNode* conn = gConns.getNode();
    gMutexClientNode.EnterMutex();
    if( conn ) {
        conn->initNode(++xclientIndex, func);
        conn->xparent = this;
        xchilds.add(conn);
    }
    gMutexClientNode.LeaveMutex();
    return conn;
}

// (JWas)
void ConnectNode::removeNode(ConnectNode*cur) {
    gMutexClientNode.EnterMutex();
    for(int n=0, cnt=childCount();n<cnt;n++) {
        if( child(n)==cur ) {
            xchilds.remove(n);
            break;
        }
    }
    gMutexClientNode.LeaveMutex();
}



bool ConnectNode::closeClient() {
    gMutexClientClose.EnterMutex();
    LPCC uid=this->get("@uid");
    if(slen(uid)) {
        StrVar* sv=uom.getInfo()->gv("proxyMaps");
        if(SVCHK('n',0)) {
            TreeNode* proxyMaps=(TreeNode*)SVO;
            qDebug("proxy client close UID: %s",uid);
            sv=proxyMaps->gv(uid);
            if(sv) sv->reuse();
        }
    }

    StrVar* sv=this->gv("@database");
    if( SVCHK('n',0) ) {
        TreeNode* model=(TreeNode*)SVO;
        qDebug("#0#client close data base release db url:%s\n", this->get("url") );
        model->clearNodeFlag(FLAG_USE);
        sv->reuse();
    }
    // qDebug("#0#client close idx:%d, url:%s\n", xclientIndex, this->get("url") );
    ConnectNode* root=this->xparent;
    if( xcf && root ) {
        xcf->reset(true);
        xcf->reuse();
        // qDebug("client close flag:0x%X\n", xflag);
    }
    bool ok=false;
    setTick();
    this->reuse();
    this->xsend.reuse();
    this->xcontent.reuse();
    this->xtype = this->xstat = 0;
    this->xflag=0;
    FTRY {
        if( xsock && xsock->IsValid() ) {
            xsock->CloseSocket();
            ok=true;
        }
    } FCATCH( FSocketException, ex ) {
        qDebug("client close Exception = %s\n", ex.Get() );
    }
    gMutexClientClose.LeaveMutex();
    return ok;
}

bool ConnectNode::closeSocket() {
    // qDebug("close socket %d", xclientIndex);
    if( xparent ) {
        return closeClient();
    }
    xflag=0;
    // clearNodeFlag(FLAG_RUN | FLAG_CLIENT_CONNECT );
    if( xsock==NULL || !xsock->IsValid() )
        return false;
    FTRY {
        xsock->CloseSocket();
        return true;
    } FCATCH( FSocketException, ex ) {
        qDebug("#0#close socket Exception = %s\n", ex.Get() );
    }
    return false;
}

bool ConnectNode::connect(LPCC host, int port) {    
    FTRY {
        int timeout=0;
        if(isNumberVar(xcf->gv("@timeout")) ) {
            timeout=toInteger(xcf->gv("@timeout"));
        }
        closeSocket();
        if(xsock==NULL ) {
            xsock = new XSocket(FLAG_CLIENT_CONNECT,this);
        }
        xsock->Connect(host,(WORD)port,timeout);
        setNodeFlag(FLAG_CONNECTED);
        return true;
    } FCATCH( FSocketException, ex ) {
        qDebug("#0#connect socket Exception = %s (%s %d)\n", ex.Get(), host, port );
        closeSocket();
        // xsock->Close();
    }
    return false;
}

bool ConnectNode::bind(int port, int block, U8 type) {
    if( xsock==NULL )
        xsock = new XSocket(FLAG_SERVER_SOCKET,this);
    else
        xsock->Init(AF_INET, SOCK_STREAM, 0);
    FTRY {
        struct sockaddr_in local;
#ifdef WINDOW_USE
        ZeroMemory(&local, sizeof(local));
#endif
        local.sin_family		= AF_INET;
        local.sin_port			= htons( port );
        local.sin_addr.s_addr	= htonl( INADDR_ANY );
        if( type==STYPE_SERVER_SELECT || type==STYPE_HTTP_SERVER ) {
            getSocket()->SetReuseAddress(TRUE);
            // getSocket()->SetCompletion(TRUE);
        }
        getSocket()->Bind(local);
        getSocket()->Listen(block);
        return true;
    } FCATCH( FSocketException, ex ) {
         qDebug("#0#server bind exception = %s\n", ex.Get() );
         xsock->Close();
         return false;
    }
}

ConnectNode* ConnectNode::acceptClient(LPSOCKET_ROUTINE func, TreeNode* cf) {
    if( xsock==NULL )
        return NULL;
    struct sockaddr_in addr;
    int addrLen = sizeof (addr);
    // func = recv call back function,
    // fn = recv func node

    FTRY {
        ConnectNode* conn = addNode(func);
        if( conn ) {
            if(isNodeFlag(FLAG_SINGLE) ) {
                conn->setNodeFlag(FLAG_SINGLE);
                // qDebug("........ accept client sigle mode %X %X............\n", xflag, conn->xflag);
            }
            conn->setNodeFlag(FLAG_START);
            conn->setTick();
            xsock->acceptClient((sockaddr*)&addr, &addrLen, conn);
            conn->setProcFunc(func, cf);
            conn->setIp(addr.sin_addr);
            qDebug("@@ client connect: %s", conn->get("ip") );
        } else {
            qDebug("#9#client accept error\n" );
        }
        return conn;
    } FCATCH( FSocketException, ex ) {
        qDebug("#9#client accept exception= %s\n", ex.Get() );
    }
    return NULL;
}
bool ConnectNode::isRead(int tm) {
    // int tm = toInteger(config()->gv("@timeout"));
    FTRY {
        if( xsock==NULL || !valid() )
            return false;
        timeval timeout;
        timeout.tv_sec = (long)(tm/1000);
        timeout.tv_usec = (tm%1000)*1000;
        WORD val = xsock->Select(FSocket::READY_FOR_READ | FSocket::TIME_OUT_OCCUR | FSocket::EXCEPTION_OCCUR , &timeout);
        if( val&FSocket::READY_FOR_READ ) {
            return true;
        }
        if( val&FSocket::TIME_OUT_OCCUR ) {
            // qDebug(">> isRead select read timeout (%d)\n", tm );
        } else if( val&FSocket::EXCEPTION_OCCUR ) {
            qDebug(">> isRead EXCEPTION_OCCUR exception (%d)\n", tm );
        }
    } FCATCH( FSocketException, ex ) {
        qDebug("#9#isRead exception= %s\n", ex.Get() );
    }
    return false;
}
bool ConnectNode::isWrite(int tm) {
    // int tm = toInteger(config()->gv("@timeout"));
    FTRY {
        if( xsock==NULL || !valid() )
            return false;
        timeval timeout;
        timeout.tv_sec = (long)(tm/1000);
        timeout.tv_usec = (tm%1000)*1000;
        /*
        timeout.tv_sec = tm<10 ? tm: 0;
        timeout.tv_usec = tm<10? 0: tm*1000;
        */
        WORD val = xsock->Select(FSocket::READY_FOR_WRITE | FSocket::TIME_OUT_OCCUR | FSocket::EXCEPTION_OCCUR, &timeout);
        if( val&FSocket::READY_FOR_WRITE ) {
            return true;
        }
        if( val&FSocket::TIME_OUT_OCCUR ) {
            // qDebug(">> isWrite select write timeout (%d) \n", tm );
        } else if( val&FSocket::EXCEPTION_OCCUR ) {
            qDebug(">> isWrite EXCEPTION_OCCUR exception (%d)\n", tm );
        }
    } FCATCH( FSocketException, ex ) {
        qDebug("#9#isWrite exception= %s\n", ex.Get() );
    }
    return false;
}

int ConnectNode::dispatch() {
    if( isRead() ) {
        qDebug("#0### dispatch read");
    } else {
        // send();
        qDebug("#0### dispatch send = %s\n", xsend.get() );
    }
    return 0;
}

int ConnectNode::eventMessage(StrVar& var) {
    qDebug("#0### %s\n",var.get() );
    return 0;
}


StrVar* ConnectNode::read( StrVar* var, int len ) {
    FTRY {
        if(len==0) {
            if( xsock ) {
                len=xsock->Peek();
            }
        }
        if(var==NULL ) {
            var=getSendVar()->reuse();
        }
        int recv=0;
        while( valid() && len>0 ) {
            int remain=len<SOCK_BUF_SIZE ? len: SOCK_BUF_SIZE;
            BYTE* buf = (BYTE*)var->memalloc(remain);
            recv = xsock->Receive((LPVOID)buf,remain);
            if( recv>0 ) {
                var->movepos(recv);
                len-= recv;
            } else {
                qDebug("#0#read not vaild (size:%d, result:%d)\n", len, recv );
                if(recv==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                break;
            }
        }
        return var;
    } FCATCH( FSocketException, ex ) {
        if( ex.GetErrorCode()==EWOULDBLOCK ) {
            return var;
        }
        if( ex.GetErrorCode()==FECONNRESET ) {
            qDebug("#0#read(var,int) socket exception reset : %s\n", ex.Get() );
        } else {
            qDebug("#0#read(var,int) socket exception : %s\n", ex.Get() );
            setNodeFlag(FLAG_ERROR);
        }
        closeSocket();
    }
    return NULL;
}

StrVar* ConnectNode::read(StrVar* var) {
    bool one = false;
    if( var==NULL ) {
        var=getContentVar()->reuse();
        one = true;
    }

    FTRY {
        int recv=0;
        while( valid() ) {
            BYTE* buf = (BYTE*)var->memalloc(2048);
            recv = xsock->Receive((LPVOID)buf,2048);
            if(recv>0) {
                var->movepos(recv);
            } else {
                if(recv==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                if( var->length() ) {
                    qDebug("connect node recvSize==%d bufferSize==%d\n", recv, var->length() );
                }
                return NULL;
            }

            if( one || recv!=2048 ) {
                break;
            }
        }
        return var;
    } FCATCH( FSocketException, ex ) {
        if( ex.GetErrorCode()==EWOULDBLOCK ) {
            return var;
        }
        if( ex.GetErrorCode()==FECONNRESET ) {
            qDebug("#0#read(var) socket exception reset : %s\n", ex.Get() );
        } else {
            // qDebug("#0#read(var) socket exception : %s\n", ex.Get() );
            setNodeFlag(FLAG_ERROR);
        }
        closeSocket();
    }
    return NULL;
}


int ConnectNode::saveFile( LPCC filenm, qint64 len, BYTE* buf, int recv ) {
    if( xsock==NULL && !xsock->IsValid() ) {
        return 0;
    }

#ifdef WINDOW_USE
    FFile file;
    file.SetFileName(filenm);
    /**********************************************************************************
    if( isReadWrite ){
        dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    }else{
        dwDesiredAccess = GENERIC_READ;
    }
    if( isExclusive ){
        dwCreationDisposition = CREATE_NEW;
    } else if( isCreate ){
        dwCreationDisposition = OPEN_ALWAYS;
    }else{
        dwCreationDisposition = OPEN_EXISTING;
    }
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    if( isDelete ){
        dwFlagsAndAttributes = FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE;
    }else{
        dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    }*********************************************************************************/
    U32 dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    U32 dwCreationDisposition = OPEN_ALWAYS;
    U32 dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
    U32 dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    FTRY {
        UL64 pos=0;
        file.Open(dwDesiredAccess,dwShareMode,dwCreationDisposition,dwFlagsAndAttributes);
        if( buf && recv>0 )
            file.Write(buf,recv);
        StrVar* var = GetVar("tempBuf")->reuse();
        buf = (BYTE*)var->memalloc(SOCK_BUF_SIZE);
        while( len>0 ) {
            FTRY {
                recv = xsock->Receive((LPVOID)buf,SOCK_BUF_SIZE);
                if( recv ) {
                    if( recv!=SOCK_BUF_SIZE ) break;
                    file.Write(buf,recv);
                    len-= recv;
                    pos+= recv;
                } else {
                    // closeSocket();
                    break;
                }
            } FCATCH( FSocketException, ex ) {
                config()->GetVar("@error")->fmt("save file error : %s", ex.Get() );
                qDebug("save file error = %s\n", ex.Get() );
                // eventMessage(GetVar("msg")->set("saveFile error = ").add(ex.Get()) );
                // closeSocket();
            }

        }
        file.Close();
        return len;
    } FCATCH( FFileSystemException, ex ) {
        // config()->GetVar("@error")->fmt("save file system error : %s", ex.Get() );
        qDebug("save file system error = %s\n", ex.Get() );
        closeSocket();
    }
#else
    QFile file(KR(filenm));
     if( file.open(QFile::WriteOnly) ) {
         if( buf && recv>0 ) {
             file.write((LPCC)buf,(qint64)recv);
         }
         BYTE buffer[SOCK_BUF_SIZE];
         while( len>0 ) {
             FTRY {
                 recv = xsock->Receive((LPVOID)buffer,SOCK_BUF_SIZE);
                 if( recv>0 ) {
                     file.write((LPCC)buffer,(qint64)recv);
                     len-= recv;
                 } else {
                     closeSocket();
                     break;
                 }
             } FCATCH( FSocketException, ex ) {
                 eventMessage(GetVar("msg")->set("saveFile error = ").add(ex.Get()) );
                 closeSocket();
             }
         }
     }
     file.close();
#endif
    return len;
}

int ConnectNode::send(BYTE* buf, int size) {
    if( xsock==NULL ) return 0;
    FTRY {
        if( !isWrite(1000) ) {
            qDebug("#0#socket send buffer timeout (size:%d)\n", size);
            return 0;
        }
        if( size==0 ) {
            qDebug("#0#send size==0\n");
            return 0;
        }
        int len=size, offset=0;
        // int idx=0;
        while( len>0 ) {
            int remain = min(SOCK_BUF_SIZE,len);
#ifdef _WIN32
            int sz = xsock->Send((const void*)(buf+offset), remain );
#else
            int sz = xsock->Send((const void*)(buf+offset), remain, MSG_NOSIGNAL); // idx>0? MSG_NOSIGNAL: 0 );
#endif
            if( sz>0 ) {
                len-= sz;
                offset+= sz;
                // idx++;
            } else {
                qDebug("#0#send error (size:%d, result:%d)\n", len, sz );
                if(sz==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                break;
            }
        }

        if(len!=0 ) {
            qDebug("#0#socket send not match (total:%d, remain:%d)\n", size, len);
        }
        return offset;
    } FCATCH( FSocketException, ex ) {
        qDebug("#0#socket send exception (%s = %s)\n", this->get("url"), ex.Get() );
        setNodeFlag(FLAG_ERROR);
        closeSocket();
    }
    return 0;
}

StrVar* ConnectNode::send(StrVar* var) {
    if( xsock==NULL ) return NULL;
    FTRY {
        if( var==NULL ) {
            var=&xsend;
        }
        int len = var->length();
        if( !isWrite(1000)) {
            qDebug("#0#socket send var timeout (size:%d)", len);
            setNodeFlag(FLAG_SKIP);
            return NULL;
        }
        int sp = 0;
        // qDebug("send: %s (len=%d)", var->get(sp), len);
        while( sp<len ) {
            int remain = min(SOCK_BUF_SIZE,len);
            int sz = xsock->Send((const void*)var->get(sp), remain );
            if( sz>0 ) {
                sp+= sz;
            } else {
                qDebug("#0#send var error (size:%d, result:%d)\n", len, sz );
                if(sz==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                break;
            }
        }
        // qDebug("send end: %d %d", sp, len);
        return var;
    } FCATCH( FSocketException, ex ) {
        LPCC url=get("url");
        qDebug("#0#socket send var error (%s = %s)\n", url, ex.Get() );
        setNodeFlag(FLAG_ERROR);
        // closeSocket();
    }
    return NULL;
}
int ConnectNode::getTotalSize(StrVar* var, int sp, int ep) {
    XParseVar pv(var, sp,ep);
    int pos=pv.findPos("##>");
    LPCC type=NULL, sz=NULL;
    if( pos!=-1 && pos<32 ) {
        pv.start=pos+3;
        pv.findEnd("{");
        pv.start-=1;
        if( pv.match("{","}",FLAG_SKIP) ) {
            type=pv.findEnd(":").v();
            sz=pv.findEnd(":").v();
            pv.findEnd("\r\n");
        }
    } else {
        pv.findEnd("Content-Length:");
        if(pv.valid()) {
            sz=pv.findEnd("\n").v();
            pv.findEnd("\r\n\r\n");
        }
    }
    if( !isnum(sz)) return 0;
    int remain=ep-pv.start;
    int total=atoi(sz);
    return total>remain ? total-remain: 0;
}

StrVar* ConnectNode::recvData(StrVar* var, int total) {
    FTRY {
        if( xsock==NULL || !valid() || !isRead(1000) ) {
            qDebug("recv data socket error");
            return NULL;
        }
        char buf[SOCK_BUF_SIZE];
        if(total<=0) {
            int recv=xsock->Receive(buf,SOCK_BUF_SIZE);
            if( recv>0) {
                var->add(buf,recv);
                if( total==-1 ) {
                    total=getTotalSize(var, 0, recv);
                    if(total>0) qDebug("recv data start recv:%d total==%d", recv, total);
                }
            } else {
                qDebug("#0#recv data error:%d next close", recv);
                if(recv==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                return NULL;
            }
        }

        while(total>0 ) {
            int recv=xsock->Receive(buf,SOCK_BUF_SIZE);
            if(recv>0) {
                var->add(buf,recv);
                total-=recv;
            } else {
                qDebug("#0#recv data error:%d remain:%d", recv, total);
                /*
                if(recv==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                */
                break;
            }
        }
        xcf->GetVar("@remain")->setVar('0',0).addInt(total);
    } FCATCH(FSocketException, ex) {
        qDebug("recv data exception %s",ex.Get() );
    }
    return var;
}

int ConnectNode::recvProxy(ConnectNode* target, StrVar* var, int total) {
    int sendSize=0;
    FTRY {
        if(xsock==NULL || !valid() || !isRead(2000) ) {
            qDebug("recv proxy read timeout");
            return 0;
        }
        char buf[SOCK_BUF_SIZE];
        int sz=0;
        if(total==0) {
            int recv=xsock->Receive(buf,SOCK_BUF_SIZE);
            if(recv>0) {
                var->add(buf,recv);
                total=getTotalSize(var, 0, recv);
                sz=target->send((BYTE*)buf,recv);
                if(sz==0) {
                    qDebug("recv proxy error send 0 byte [recv:%d total:%d]", recv, total);
                    xsock->Flush();
                    return 0;
                }
                if(recv!=sz) {
                    qDebug("recv proxy not match recv:%d send:%d", recv, sz);
                }
                sendSize+=sz;
            } else {
                qDebug("#0#recv data error:%d", recv);
                if(recv==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                return 0;
            }
        }
        while(total>0 ) {
            if(valid() ) {
                int recv=xsock->Receive(buf,SOCK_BUF_SIZE);
                if( recv>0 && target->isWrite(1000) ) {
                    sz=target->send((BYTE*)buf,recv);
                    total-=recv;
                    if(sz==0) {
                        xsock->Flush();
                        qDebug("recv proxy error send 0 byte [send:%d remain:%d]", sendSize, total);
                        return 0;
                    }
                    if(recv!=sz) {
                        qDebug("recv proxy not match recv:%d send:%d", recv, sz);
                    }
                    sendSize+=sz;
                } else {
                    qDebug("#0#recv data error recv:%d remain:%d", recv, total);
                    if(recv==0) {
                        closeSocket();
                    } else {
                        if(target->valid() ) setNodeFlag(FLAG_SKIP);
                    }
                }
            } else {
                qDebug("recv proxy socket not valid send:%d, total:%d", sendSize, total);
                break;
            }
        }
    } FCATCH( FSocketException, ex ) {
        qDebug("recv proxy patch exception: %s", ex.Get());
    }
    return sendSize;
}

StrVar* ConnectNode::sendData(StrVar* var, int sp, int ep) {
    FTRY {
        if( xsock==NULL || sp>=ep || !isWrite(1000) ) {
            qDebug("send data error(%d, %d)", sp, ep);
            return NULL;
        }
        while( sp<ep ) {
            int remain = min(SOCK_BUF_SIZE,ep-sp);
#ifdef _WIN32
            int sz = xsock->Send((const void*)var->get(sp),remain );
#else
            int sz = xsock->Send((const void*)var->get(sp),remain, MSG_NOSIGNAL );
#endif
            if( sz>0 ) {
                sp+=sz;
            } else {
                qDebug("#0#send data error:%d remain:%d\n", ep-sp, sz );
                if(sz==0) {
                    closeSocket();
                } else {
                    setNodeFlag(FLAG_SKIP);
                }
                return NULL;
            }
        }
        // qDebug("socekt send data %d, %d", sp, ep);
        return var;
    } FCATCH( FSocketException, ex ) {
        LPCC url=get("url");
        qDebug("#0#socket send data exception (%s = %s)\n", url, ex.Get() );
        setNodeFlag(FLAG_ERROR);
        closeSocket();
    }
    return NULL;
}

StrVar* ConnectNode::getSendVar() {
    // setInt("sendOffset", 0);
    StrVar* var=&xsend;
    // var->reuse();
    return var;
}
StrVar* ConnectNode::getContentVar() {
    // setInt("sendOffset", 0);
    return &xcontent;
}

StrVar* ConnectNode::getRecvVar() {
    setInt("recvOffset", 0);
    StrVar* var = GetVar("recvBuf");
    var->reuse();
    return var;
}
StrVar* ConnectNode::recvHttpHeader( StrVar* recvBuf ) {
    if( !valid() ) {
        qDebug("#0#recvHttpHeader recv error (socket not valid)\n");
        return NULL;
    }
    // xtick = GetTickCount();
    qint64 total=0;
    if( recvBuf==NULL ) {
        recvBuf=&xsend;
    }
    char buf[SOCK_BUF_SIZE];
    int cnt=0xFFFF;
    xsock->SetReceiveBufferSize(SOCK_BUF_SIZE);
    for( int n=0; n<cnt && valid() ; n++ ) { // && (GetTickCount() - xtick) < xtimeout ) {
        FTRY {
            int recv = xsock->Receive(buf,SOCK_BUF_SIZE);
            if( recv>0 ) {
                recvBuf->add(buf, recv);
                if(n==0 ) {
                    XParseVar pv(recvBuf,0,recv);
                    pv.findEnd("Content-Length:");
                    if(pv.valid()) {
                        LPCC val=pv.findEnd("\n").v();
                        if(isnum(val) ) {
                            total=atoi(val);
                            qDebug("Content-Length read %s ", val);
                            pv.findEnd("\r\n\r\n");
                            if( pv.valid()) {
                                int remain=pv.wep-pv.start;
                                if( remain>0 ) {
                                    total-=remain;
                                }
                                qDebug("Content-Length total:%d remain:%d recv:%d", (int)total, remain, recv);
                            }
                            if(total<=0) break;
                        } else {
                            qDebug("Content-Length read %s is not number ", val);
                            if(recv!=SOCK_BUF_SIZE) break;
                        }
                    } else {
                        qDebug("Content-Length not found buffer ");
                    }
                } else {
                    if(total>0 ) {
                        total-=recv;
                        if(total<=0) break;
                    } else if(recv!=SOCK_BUF_SIZE) {
                        break;
                    }
                }
            } else {
                qDebug("#0#recvHttpHeader recv==0 (size:%d, result:%d)\n", recvBuf->length(), recv );
                // setNodeFlag(FLAG_SKIP);
                // closeSocket();
                break;
            }
        } FCATCH( FSocketException, ex ) {
            GetVar("@error")->fmt("%d: recv error %s", ex.GetErrorCode(), ex.Get());
            qDebug("#0#recv var error = %s\n", ex.Get() );
            break;
        }
    }
    qDebug("ConnectNode::recvHttpHeader (total=%d)\n",(int)total);
    return recvBuf;
}
StrVar* ConnectNode::recv( StrVar* var, qint64 size ) {
    if( !valid() ) {
        qDebug("#0#socket recv error (socket not valid)\n");
        return NULL;
    }
    // xtick = GetTickCount();
    StrVar* recvBuf = var;
    qint64 total=size;
    if( total==0 ) {
        total=(qint64)this->getInt("Content-Length");
    }
    if( recvBuf==NULL ) {
        recvBuf=&xsend;
		if(recvBuf->length()>0 ) {
            qDebug("recv check total:%d, first buffer size:%d \n", (int)total, recvBuf->length() );
			total-=recvBuf->length();
		}
        if( total<=0 ) {
            return recvBuf;
        }
	} else if( recvBuf==this->getContentVar() ) {
		int pos=var->findPos("\r\n\r\n");
        if( pos!=-1 ) {
            pos+=4;
            int remain=var->length()-pos;
            if( remain>0 ) {
                total-=remain;
            }
        }
    }
    bool sendAll=false;
    if( total<=0 ) {
        if( total<0 ) {
			qDebug("socket recv total check start (total:%d)\n", (int)total);
            return NULL;
        }
        sendAll=true;
    }

    char buf[SOCK_BUF_SIZE];
    // config()->GetVar("@error")->reuse();
    xsock->SetReceiveBufferSize(SOCK_BUF_SIZE);
    for( int n=0; n<4096 && valid() ; n++ ) { // && (GetTickCount() - xtick) < xtimeout ) {
        FTRY {
            int recv = xsock->Receive(buf,SOCK_BUF_SIZE);
            if( recv>0 ) {
                recvBuf->add(buf, recv);
				if(n==0 && sendAll ) {
					XParseVar pv(recvBuf,0,recv);
					pv.findEnd("Content-Length:");
					if(pv.valid()) {
						LPCC val=pv.findEnd("\n").v();
						if(isnum(val) ) {
							total=atoi(val);
							qDebug("Content-Length read %s ", val);
							pv.findEnd("\r\n\r\n");
							if( pv.valid()) {
								int remain=pv.wep-pv.start;
								if( remain>0 ) {
									total-=remain;
									sendAll=false;
								}
								qDebug("Content-Length total:%d remain:%d recv:%d", (int)total, remain, recv);
							}
							if(total<=0) break;
						} else {
							qDebug("Content-Length read %s is not number ", val);
						}
					} else {
						qDebug("Content-Length not found buffer ");
					}
				}
                if( sendAll==false ) {
                    total-=recv;
                    if( total<=0 ) {
                        if( total<0 ) {
                            qDebug("#0#ConnectNode::recv (total=%d recv=%d)\n",(int)total, recv);
                        }
                        break;
                    }
                }
                /*
                // recvBuf->movepos(recv);
                if( SOCK_BUF_SIZE!=recv  ) {
                    break;
                }
                */
            } else {
                qDebug("#0#recv var error (size:%d, result:%d)\n", recvBuf->length(), recv );
                // setNodeFlag(FLAG_SKIP);
                // closeSocket();
                break;
            }
        } FCATCH( FSocketException, ex ) {
            GetVar("@error")->fmt("%d: recv error %s", ex.GetErrorCode(), ex.Get());
            qDebug("#0#recv var error = %s\n", ex.Get() );
            break;
        }
    }
    if(total>0 ) {
        qDebug("ConnectNode::recv (total=%d)\n",(int)total);
    }
    return recvBuf;
}

//-------------------------------------------------------------------
//	Message Server (21)
//-------------------------------------------------------------------

MessageServer::MessageServer(TreeNode* cf, QObject *parent) : xcf(cf), QTcpServer(parent) {
#ifdef SERVER_THREAD_USE
    myThread = new QThread();
    myThread->start();
#else
    myThread=NULL;
#endif
    connect(this, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
}
void MessageServer::callback(StrVar* rst ) {
    StrVar* sv=xcf->gv("@callback");
    LPCC type=toString(xparam.get(0));
    if( SVCHK('f',0) ) {
        XFuncNode* fn=(XFuncNode*)SVO;
        qDebug("message server callback type==%s paramSize:%d\n", type, xparam.size() );
        if( rst==NULL ) rst=&xrst;
        setFuncNodeParams(fn,&xparam);
        fn->call(NULL, rst);
    }
}
XListArr* MessageServer::getParam(LPCC type) {
    xparam.reuse();
    xparam.add()->set(type);
    return &xparam;
}
void MessageServer::sendAll(LPCC msg) {

}

void MessageServer::acceptConnection() {
    qDebug("... server aceept started ...\n");
    MessageClient* messageClient=NULL;
    QTcpSocket* sock=nextPendingConnection();
    if( sock ) {
        TreeNode* client=xcf->addNode();
        client->set("tag","messageClient");
        client->xstat=FNSTATE_MESSAGE_CLIENT;
        messageClient = new MessageClient(client, this, sock, this);
#ifdef SERVER_THREAD_USE
        messageClient->moveToThread(myThread);
#endif
        connect(this,SIGNAL(OnStopped()),messageClient,SLOT(CloseSocket()));
    } else {
        qDebug("#9#server aceept error (pending connection fail)\n");
    }
}
void MessageServer::StartServer(qint16 port) {
    getParam("start");
    if(!this->listen(QHostAddress::Any,port)) {
        qDebug("Message Server bind error (listen port: %d)\n", port );
        xparam.add()->set("error");
    } else {
        qDebug("Message Server Listening... (listen port: %d)\n", port );
        xparam.add()->set("ok");
    }
    callback();
}

void MessageServer::StopServer() {
    getParam("stop");
    if( xcf ) {
        int sz=xcf->childCount();
        /*
        StrVar* sv=NULL;
        for( int n=0; n<sz; n++ ) {
            xcf->child(n)->reuse();
        }
        */
        xcf->reset(true);
        xparam.add()->setVar('0',0).addInt(sz);
    }
    callback();
    this->close();
    qDebug() << "Message Server stopped\n";
    emit OnStopped();
}
void MessageServer::CloseClient(MessageClient* client) {
    if( client==NULL ) {
        return;
    }
    //#baropage
    TreeNode* cur=client->xcf;
    StrVar* sv=NULL;
    if( xcf && cur ) {
        getParam("clientClose");
        xparam.add()->setVar('n',0,(LPVOID)cur);
        callback();
        sv=cur->gv("onInit");
        if(SVCHK('f',0)) {
            XFuncNode* fnInit=(XFuncNode*)SVO;
            fnInit->clearNodeFlag(FLAG_PERSIST);
            funcNodeDelete(fnInit);
            sv->reuse();
        }
        cur->reset(true);

        xcf->remove(cur);
    } else {
        qDebug("#9#server client close fail!!! client node not define\n");
    }
}
/*
void MessageServer::incomingConnection(int socketDescriptor) {
    qDebug() << socketDescriptor << " Connecting..." << socketDescriptor;
}
*/

//-------------------------------------------------------------------
//	Message Client
//-------------------------------------------------------------------

MessageClient::MessageClient(TreeNode* cf, MessageServer* server, QTcpSocket* sock, QObject *parent) :
    xcf(cf), xserver(server), Socket(sock), QObject(parent) {
    if( Socket==NULL) {
        Socket=new QTcpSocket(this);
        SessionID=0;
    } else {
        SessionID=Socket->socketDescriptor(); // SocketDescriptor;

    }
    if( SessionID>0 ) {
        /*
        if( !Socket->setSocketDescriptor(SessionID) ) {
            qDebug() << SessionID << " Error binding socket";
            return;
        }
        */
#ifndef WINDOW_USE
        int enableKeepAlive = 1;
        int fd = Socket->socketDescriptor();
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));

        int maxIdle = 10; /* seconds */
        setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));

        int count = 3;  // send up to 3 keepalive packets out, then disconnect if no response
        setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

        int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
        setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
#endif
        qDebug() << SessionID << " session Connected";
        xcf->setInt("@sessionId",SessionID);
    }
    //connect the signals
    connect(Socket,SIGNAL(readyRead()),this,SLOT(SocketReadyRead()),Qt::DirectConnection);
    connect(Socket,SIGNAL(disconnected()),this,SLOT(SocketDisconnected()),Qt::DirectConnection);
    connect(Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(Socket, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(Socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)) );
    //Create a new process for them
    if( server ) {
        TreeNode* parentNode= xcf->parent();
		StrVar* sv=NULL; // parentNode ? parentNode->gv("@callback"): NULL;
		if( server->xcf==parentNode ) {
			XFuncNode* fnParent=NULL;//(XFuncNode*)SVO;
			sv=parentNode->gv("onInit");
			if(SVCHK('f',0) ) {
				fnParent=(XFuncNode*)SVO;
			}
            /*
            TreeNode* thisNode=NULL;
            sv=fnParent->gv("@this");
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            }
            */
            sv=parentNode->gv("@callbackClient");
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnCur=setCallbackFunction(fsrc, fnParent, xcf, NULL, xcf->GetVar("@callback"));
                sv=parentNode->gv("@moduleClient");
                if( SVCHK('n',0)) {
                    TreeNode* thisNode=(TreeNode*)SVO;
                    fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)xcf);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            }
            sv=parentNode->gv("@type");
            if(sv) {
                xcf->GetVar("@type")->reuse()->add(sv);
            }
        }
        xcf->GetVar("@c")->setVar('v',22,(LPVOID)this);
        getParam("start");
        callback();
    }
}
void MessageClient::closeCheck(bool reset) {

}
void MessageClient::callback(StrVar* rst ) {
    if( xcf==NULL ) {
        LPCC type=toString(xparam.get(0));
        qDebug("#9#socket callback error config not defined (type:%s) !!!\n", type);
        return;
    }
    StrVar* sv=xcf->gv("@callback");
    // qDebug("message client callback type==%s paramSize:%d\n", type, xparam.size() );
    if( SVCHK('f',0) ) {
        XFuncNode* fn=(XFuncNode*)SVO;
        if( rst==NULL ) {
            rst=xrst.reuse();
        }
        setFuncNodeParams(fn,&xparam);
        fn->call(NULL, rst);
    }
}
XListArr* MessageClient::getParam(LPCC type) {
    xparam.reuse();
	xparam.add()->setVar('n',0,(LPVOID)xcf);
	xparam.add()->set(type);
	return &xparam;
}
void MessageClient::onError(QAbstractSocket::SocketError ) {
    QTcpSocket* sock = (QTcpSocket*)sender();
    QString msg=sock ? sock->errorString(): "";
    getParam("error")->add()->add(Q2A(msg));
    callback();
}

void MessageClient::onConnect() {
#ifdef _LINUX
    int enableKeepAlive = 1;
    int fd = Socket->socketDescriptor();
    if( fd>0 ) {
        setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));

        int maxIdle = 10; /* seconds */
        setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));

        int count = 3;  // send up to 3 keepalive packets out, then disconnect if no response
        setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

        int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
        setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    }
#endif
    getParam("connect");
    callback();
}

int MessageClient::readAll(StrVar* rst) {
    // QDataStream in(Socket);
    if( Socket ){
        if( Socket->state()!=QAbstractSocket::ConnectedState ) {
            qDebug("#9#socket recv var error socket closed\n config: %s\n", xcf->log() );
            return false;
        }
        int byteCheck=(int)Socket->bytesAvailable();
        if( byteCheck > 0) {
            QByteArray ba=Socket->readAll();
            rst->add(ba.constData(), ba.size() );
            qDebug("... client recv var (%d==%d)\n", byteCheck, ba.size() );
            return ba.size();
        }
    }
    return 0;
}

/*
void MessageClient::recvVar(StrVar* rst) {
    if( Socket->state()!=QAbstractSocket::ConnectedState ) {
        qDebug("#9#socket recv var error socket closed\n config: %s\n", xcf->log() );
        break;
    }
    QByteArray ba=Socket->readAll();
    rst->add(ba.constData(), ba.size() );
}
*/

void MessageClient::sendVar(StrVar* data) {
    int sp=0, ep=0;
    StrVar* var=getStrVarPointer(data,&sp,&ep);
    if( Socket && Socket->isValid() && sp<ep ) {
        /*
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out<<quint16(0);
        out.writeRawData(var->get(sp), ep-sp);
        out.device()->seek(0);
        out<<quint16(block.size() - sizeof(quint16));
        Socket->write(block);
        */
        Socket->write(var->get(sp), ep-sp );
    }
}


void MessageClient::sendBuffer(LPCC buf, int size) {
    if( Socket && Socket->isValid() && size>0 ) {
       Socket->write(buf, size);
    }
}

void MessageClient::SocketReadyRead() {
    //Keep adding to the command buffer
    if( xcf==NULL ) {
        qDebug("#9#socket read error config not defined !!!\n");
        return;
    }
    StrVar* sv=xcf->gv("@targetObject");
    if( sv ) {
        QByteArray data = Socket->readAll();
        int remain=xcf->incrNum("@remainSize");
        remain-=data.size();
        if(remain<=0 ) {
            remain=0;
        }
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            if(node->cmp("tag","file")) {
                sv=node->GetVar("@c");
                if( SVCHK('z',1) ) {
                    QFile* file=(QFile*)SVO;
                    file->write(data.constData(), data.size() );
                    if(remain==0) {
                        file->close();
                    }
                }
            } else {
                sv=node->gv("@webClient");
                if(SVCHK('v',1) ) {
                    ConnectNode* web=(ConnectNode*)SVO;
                    web->send((BYTE*)data.constData(), data.size() );
                    if( remain==0 ) {
                        web->setBool("@finishCheck", true);
                    }
                } else {
                    XFuncNode* fn = getEventFuncNode(node, "onRead");
                    if(fn ) {
                        PARR arrs=getLocalParams(fn);
                        arrs->add()->set( data.constData(), data.size() );
                        arrs->add()->setVar('0',0).addInt(remain);
                        arrs->add()->setVar('0',0).addInt(data.size());
                        setFuncNodeParams(fn, arrs);
                        fn->call();
                    } else {
                        qDebug("#0#client onRead event function not define (data size: %d)\n", data.size() );
                        xcf->GetVar("@targetObject")->reuse();
                    }
                }
            }
        } else if(SVCHK('v',1) ) {
            ConnectNode* web=(ConnectNode*)SVO;
            web->send((BYTE*)data.constData(), data.size() );
            if( remain==0 ) {
                web->setBool("@finishCheck", true);
            }
        } else if(SVCHK('v',22)) {
            MessageClient* client=(MessageClient*)SVO;
            client->sendBuffer(data.constData(), data.size() );
        }
        if(remain==0) {
            sv=xcf->gv("@targetObject");
            if(sv) sv->reuse();
            xcf->GetVar("@finishCheck")->setVar('3',1);
        }
        xcf->setInt("@remainSize", remain );
        return;
    }

    LPCC type=xcf->get("@type");
    if(slen(type)==0) type="client";
    // #baropage
    if( ccmp(type, "websocket") ) {
        getParam("recv");
        callback();
    } else {
        QByteArray data = Socket->readAll();
        getParam("recv");
        xparam.add()->set( data.constData(), data.size() );
        callback();
    }
}

void MessageClient::SocketDisconnected() {
    qDebug() << SessionID << " session disconnected";
    if( xserver ) {
        getParam("disconnect");
        callback();
        if( xcf ) {
            StrVar* sv=xcf->gv("@c");
            if( sv ) {
                sv->reuse();
            }
            sv=xcf->GetVar("@callback");
            if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                fnCur->clearNodeFlag(FLAG_PERSIST);
                gfns.deleteFuncNode(fnCur);
                sv->reuse();
            }
        }
        xserver->CloseClient(this);
        qDebug("#0#disconnect client \n");
        //Cleanup
    } else {
        qDebug("#9#>> disconnect server object is null\n");
    }
}

void MessageClient::SendResponse(LPCC data)
{
    SendResponse(QString::fromUtf8(data));
}

void MessageClient::SendResponse(QString data)
{
    SendResponse(data.toUtf8() );
}

void MessageClient::SendResponse(QByteArray data)
{
    //Send the data to the client
    if( Socket && Socket->isValid() ) {
        Socket->write(data);
    }
}


void MessageClient::CloseSocket() {
    if( Socket && Socket->isValid() ) {
        qDebug("========== close client==========");
        // !=QAbstractSocket::UnconnectedState )
        Socket->close(); // disconnectFromHost();
    } else {
        LPCC log=xcf? xcf->log():"null";
        qDebug("#0#close socket error socket is not valid node:%s\n", log);
    }
}


void MessageClient::onStateChanged(QAbstractSocket::SocketState state) {
    LPCC type="";
    switch( state ) {
    case QAbstractSocket::UnconnectedState:	type="UnConnected"; break;
    case QAbstractSocket::HostLookupState:	type="HostLookup"; break;
    case QAbstractSocket::ConnectingState:	type="Connecting"; break;
    case QAbstractSocket::ConnectedState:	type="Connected"; break;
    case QAbstractSocket::BoundState:		type="Bound"; break;
    case QAbstractSocket::ListeningState:	type="Listening"; break;
    case QAbstractSocket::ClosingState:		type="Closing"; break;
    default: type="UnDefine";
    }
    getParam("state")->add()->set(type);
    callback();
}

bool parseTextFormat(StrVar* var, int sp, int ep, StrVar* rst, StrVar* sv) {
    if(var==NULL ) return false;
    XListArr* arr=NULL;
    if( SVCHK('a',0) ) {
        arr=(XListArr*)SVO;
    }
    int idx=0;
    if( sp==0 && ep==0 ) {
        var=getStrVarPointer(var, &sp, &ep);
    }
    if( sp<ep ) {
        LPCC code=NULL;
        XParseVar pv(var, sp, ep);
        if(sv && pv.find("{")!=-1 ) {
            while( pv.valid() ) {
                pv.findEnd("{");
                rst->add(var, pv.prev, pv.cur);
                // (JWas)
                if(!pv.valid()) break;
                code=pv.findEnd("}").v();
                if(arr) {
                    if(isnum(code) ) {
                        idx=atoi(code);
                    }
                    sv=arr->get(idx);
                }
                toString(sv, rst);
            }
            qDebug("#0# parseTextFormat %s param: %s\n", code, rst->get());
        } else {
            rst->add(var,sp,ep);
        }
        return true;
    }
    return false;
}

bool recvWebProxy(ConnectNode* conn, StrVar* rst, XFuncNode* fn, ConnectNode* client, StrVar* header, bool bClose) {
    XSocket* sock=conn->xsock;
    if(sock==NULL) {
        qDebug("#9#recv web data error (socket is null)\n");
        return false;
    }
    bool ret=true;
    int contentLen=0, remain=0;
    char buf[SOCK_BUF_SIZE];
    rst->reuse();
    sock->SetReceiveBufferSize(SOCK_BUF_SIZE);
    for( int n=0; n<4096 && sock->IsValid() ; n++ ) {
        FTRY {
            int recv = sock->Receive(buf,SOCK_BUF_SIZE);
            if( recv>0 ) {
                if(n==0) {
                    rst->add(buf, recv);
                    LPCC prop="Content-Length: ";
                    int pos=rst->find(prop);
                    if(pos>0) {
                        pos+=slen(prop);
                        int end=rst->find("\n",pos);
                        if(pos<end) {
                            LPCC val=rst->trimv(pos,end);
                            if(isnum(val)) {
                                contentLen=atoi(val);
                                pos=rst->find("\r\n\r\n");
                                if(pos>0) {
                                    pos+=4;
                                    remain=contentLen-(rst->length()-pos);
                                    if( header ) {
                                        int sp=0, ep=0;
                                        StrVar* var=getStrVarPointer(header, &sp, &ep);
                                        if(sp<ep ) {
                                            StrVar* sendVar=getStrVarTemp();
                                            StrVar* sv=fn->GetVar("@contentLength");
                                            sv->setVar('0',0).addInt(contentLen);
                                            parseTextFormat(var, sp, ep, sendVar, sv);
                                            sendVar->add("\n").add(rst);
                                            if(client) {
                                                client->send(sendVar);
                                            }
                                        }
                                    }
                                    qDebug("#0#recv web proxy (content length:%d, remain:%d)\n", contentLen, remain);
                                } else {
                                    ret=false;
                                    qDebug("#0#recv web proxy http header bound error (content length:%d)\n", contentLen);
                                }
                            } else {
                                ret=false;
                                qDebug("#0#recv web proxy header Content-Length value is not number (value:%s)\n", val);
                            }
                        } else {
                            ret=false;
                            qDebug("#0#recv web proxy header Content-Length error (%d, %d)\n", pos, end);
                        }
                    } else {
                        ret=false;
                        qDebug("#0#recv web proxy header Content-Length not found ...\n");
                    }
                } else if(remain>0) {
                    remain-=recv;
                    if(client) {
                        client->send((BYTE*)buf, recv);
                    } else {
                        rst->add(buf, recv);
                    }
                    if( remain<=0 ) {
                        if( remain<0 ) {
                            ret=false;
                            qDebug("#0#recv web length check error (contentLen=%d, recv=%d, remain=%d)\n",(int)contentLen, recv, remain);
                        }
                        break;
                    }
                }
            } else {
                ret=false;
                qDebug("#0#recv web error (contentLen:%d, recvSize:%d, result:%d)\n", contentLen, rst->length(), recv );
                break;
            }
        } FCATCH( FSocketException, ex ) {
            ret=false;
            qDebug("#0#recv var error = %s\n", ex.Get() );
        }
        if(ret==false) break;
    }

    if( bClose ) {
        conn->closeSocket();
    }
    return ret;
}

bool sendWebStream(ConnectNode* conn, LPCC path, qint64 start, qint64 end, StrVar* rst, XFuncNode* fn, MessageClient* client, StrVar* proto, bool bClose) {
    bool ret=true;
    QFile file(KR(path));
    if( file.open(QIODevice::ReadOnly) ) {
        int pos= LastIndexOf(path,'.');
        LPCC ext = pos>0 ? path+pos+1: NULL;
        DWORD tick=GetTickCount();
        qint64 fsize=file.size();
        conn->setNodeFlag(HTTP_DOWNLOAD);
        /*
        if( ccmp(code,"file") || ccmp(code,"filename") ) {
            sv->add("Content-Disposition: attachment; filename=\"").add(AS(1)).add("\"\r\n");
        }
        pos= LastIndexOf(path,'/');
        if( pos==-1 ) {
            pos=LastIndexOf(path,'/');
        }
        LPCC filenm = pos!=-1 ? path+pos+1: path;
        conn->GetVar("@attachHeader")->add("Content-Disposition: attachment; filename=\"").add(filenm).add("\"\r\n");

       //전체 요청일 경우 200, 부분 요청일 경우 206을 반환상태 코드로 지정
       response.setStatus(isPart ? 206 : 200);

       //mime type 지정
       response.setContentType("video/mp4");

       //전송 내용을 헤드에 넣어준다. 마지막에 파일 전체 크기를 넣는다.
       response.setHeader("Content-Range", "bytes "+rangeStart+"-"+rangeEnd+"/"+movieSize);
       response.setHeader("Accept-Ranges", "bytes");
       response.setHeader("Content-Length", ""+partSize);
        */
        rst->reuse();
        if( sendHttpHeader(conn, rst, fsize, ext, client?true:false) ) {
            FTRY {
                qDebug("#0#sendStream Header %s", rst->get() );
                // printBox(conn, rst->reuse() );
                // qDebug("#0#sendStream conn Header %s", rst->get() );chunkOffset
                char buf[4096];
                qint64 total=fsize, remain=0;
                if( conn->gv("rangeStart") ) {
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
                        ret=false;
                        qDebug("#0#send streaming file seek error (start:%lld)\n", start);
                    }
                }
                if( client && proto ) {
                    int sp=0, ep=0;
                    TreeNode* param=conn->config();
                    StrVar* var=getStrVarPointer(proto, &sp, &ep);
                    if( param && sp<ep ) {
                        StrVar* sendVar=getStrVarTemp();
                        XParseVar pv(var, sp, ep);
                        fn->setInt("Content-Length", remain);
                        makeTextData(pv, fn, sendVar, 0x20000, param );
                        sendVar->add("\n").add(rst);
                        if(client) {
                            client->sendVar(sendVar);
                        }
                    }
                }
                qDebug("#0#sendStream start:%lld, end:%lld, remain:%lld\n", start, end, remain );
                XSocket* sock=conn->xsock;
                int send=0;
                total=0;
                while( remain>0 && conn->valid() ) {
                    int read=(int)file.read(buf, remain > 4096?4096: remain );
                    if( read<=0 ) {
                        qDebug("#9#sendStream file read fail (remain:%lld, total:%lld, read:%d)\n", remain, total, read);
                        ret=false;
                        break;
                    }
                    if( client) {
                        client->sendBuffer(buf,read);
                    } else {
                        send=sock->Send(buf,read);
                    }
                    if( read==send ) {
                        total+=read;
                        remain-=read;
                        if( read<4096 ) {
                            qDebug("#0#sendStream is remain buffer %s (remain:%lld, total:%lld, read:%d)\n", path, remain, total, read);
                        }
                    } else {
                        ret=false;
                        qDebug("#0#sendStream file send not match %s (remain:%lld, total:%lld, read:%d)\n", path, remain, total, read);
                        break;
                    }
                }
                qDebug("#0#sendStream file file check total=%lld (remain:%lld ) xxxxxxxxxxxxx\n", total, remain );
                file.close();
            } FCATCH( FSocketException, ex ) {
                qDebug("#0#sendStream exception (message:%s)\n", ex.Get() );
                ret=false;
            }
        } else {
            qDebug("#0#sendStream http header send error %s\n", path);
        }
        tick=GetTickCount()-tick;
        qDebug("---------------------- sendStream %s end ( %d ms) ---------------------\n", path, (int)tick);
    } else {
        qDebug("#9#sendStream file not found error (path:%s)\n", path);
    }
    // initConnectNode(conn, conn->cmp("Connection","keep-alive"), true );
    return ret;
}

int readHttpHeader(ConnectNode* conn, StrVar* rst, StrVar* var ) {
    int sp=0, ep=0;
    if(var ) {
        var=getStrVarPointer(var,&sp,&ep);
    } else {
        var = conn->read();
        if(var==NULL) {
            qDebug("#0#http header parse error (read buffer not valid)\n");
            return 0;
        }
        ep=var->length();
    }
    if(sp>=ep ) {
        qDebug("#0#http header parse not valid (%d, %d)\n", sp, ep);
        return 0;
    }

    LPCC ctt=NULL;
    int pos = var->findPos("\r\n\r\n", sp, ep);
    if( pos==-1 ) {
        pos=ep;
    } else {
        ctt=var->get(pos+4);
    }
    if( rst && ctt ) {
        rst->set(ctt);
    }
    XParseVar pv(var,sp,ep);
    for(int n=0; n<64 && pv.valid(); n++ ) {
        pv.findEnd("\n");
        if( n==0 ) {
            XParseVar sub(var, pv.prev, pv.cur);
            for(int m=0; m<4 && sub.valid(); m++) {
                sub.findEnd(" ");
                switch( m ) {
                case 0:
                    conn->set("method",		sub.v());	break;
                case 1:
                    conn->set("url",		sub.v());	break;
                case 2:
                    conn->set("version",	sub.v());	break;
                }
            }

        } else {
            char* line=(char*)pv.v();
            int p = IndexOf(line,':');
            if( p>0 ) {
                line[p] = 0;
                LPCC code = Trim(line);
                conn->set(code,Trim(line+p+1));
            }
        }
    }
    return conn->getInt("Content-Length");
}



UdpServerSocket::UdpServerSocket(TreeNode* cf, QObject *parent) : xcf(cf), QObject(parent)
{
    m_nPort = toInteger(cf->gv("port"));
    m_hUdpSocket = NULL;
}

UdpServerSocket::~UdpServerSocket()
{
    SAFE_DELETE(m_hUdpSocket);
}

void UdpServerSocket::startUdpServer(int port)
{
    m_nPort=port;
    m_hUdpSocket = new QUdpSocket(this);
    m_hUdpSocket->bind(QHostAddress::Any, m_nPort);
    connect(m_hUdpSocket, SIGNAL(readyRead()), this, SLOT(slotsUdpReadDatagram()));
    qDebug("start udp server port: %d", m_nPort);
}

////////////// slots //////////////


void UdpServerSocket::slotsUdpReadDatagram() {
    XFuncNode* fnRecv=NULL;
    StrVar* sv=xcf->gv("onRecv");
    if( SVCHK('f',0) ) {
        fnRecv=(XFuncNode*)SVO;
    }
    qDebug("start udp slotsUdpReadDatagram: %s", fnRecv?"onRecv":"error");
    while ( m_hUdpSocket->hasPendingDatagrams() ) {
        QByteArray byteMsg;
        byteMsg.resize(m_hUdpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 nSenderPort;
        m_hUdpSocket->readDatagram(byteMsg.data(), byteMsg.size(), &sender, &nSenderPort);

        QString strMsg = KR(byteMsg);
        // qDebug("upd read datagram data:%s port:%d address:%s", Q2A(strMsg), nSenderPort, sender.toString());
        if(fnRecv) {
            XListArr* param=xparam.reuse();
            StrVar* rst=xrst.reuse();
            param->add()->add(Q2A(strMsg));
            setFuncNodeParams(fnRecv,param);
            fnRecv->call(NULL, rst);
        }
    }
}


