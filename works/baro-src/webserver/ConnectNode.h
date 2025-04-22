#ifndef ConnectNode_H
#define ConnectNode_H

#include "../fobject/FSocket.h"
#include "../fobject/FThread.h"
#include "../fobject/FFile.h"
#include "../baroscript/func_util.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QUdpSocket>
#include <vector>

#ifdef WINDOW_USE
#ifndef _NO_FED_NAMESPACE
    using namespace fed;
#endif
#else
USING_NAMESPACE_FED
#endif

#define FLAG_SEND			0x400		// NF_NOTUSE
#define FLAG_RECV			0x800		// NF_MODIFY
#define FLAG_CONNECTED		0x1000		// NF_OK

#define HTTP_FLAG_KEEPALIVE	0x10000


#define SOCK_BUF_SIZE			4096 * 65
#define MAX_RECV_SIZE			4096

#define STYPE_SERVER_SELECT		1
#define STYPE_SERVER_THREAD		2
#define STYPE_CLIENT_THREAD		3
#define STYPE_CLIENT_CONNECT	4
#define STYPE_HTTP_SERVER		5
#define STYPE_SERVER_PROXY		6

#define STYPE_PIPE_PROC			11

#define FLAG_SERVER_SOCKET		0x10
#define FLAG_CLIENT_CONNECT		0x20
#define FLAG_ACCEPT_THREAD		0x40 // ACCEPT THREAD
#define FLAG_SERVER_HTTP		0x80
#define FLAG_SERVER_STOP		0x100

#define FLAGTYPE(a)				((a)&0xFF)
#define IS_FLAGTYPE(a,b)		((a)&0xFF)==(b)
#define ISNOT_FLAGTYPE(a,b)		((a)&0xFF)!=(b)

#define FLAG_CONNECT_NODE		0X8000

#ifdef _LINUX
#define IGIMSAPI
#else
#define IGIMSAPI WINAPI
#endif

void closeRunFuncNode(XFuncNode* fn );

typedef DWORD (IGIMSAPI *PSOCKET_ROUTINE)( LPVOID lpConnectNode );
typedef PSOCKET_ROUTINE LPSOCKET_ROUTINE;

//
//
//

#define KEEPALIVE_SEC		5
#define KEEPALIVE_MAX		100

#define NET_RECV		2
#define NET_SEND		3
#define NET_FINISH		4
#define NET_START		5
#define NET_RECVEND		6
#define NET_ERROR		9

#define NETFLAG_HTTP		0x100
#define NETFLAG_KEEP		0x200
#define NETFLAG_CONNECT		0x400
#define NETFLAG_DOWNLOAD	0x1000
#define NETFLAG_UPLOAD		0x2000
#define NETFLAG_SYNC		0x4000
#define NETFLAG_DISPATCH	0x4000


class ConnectNode;
class FEDLIBRARY_API XSocket : public FSocket {
public:
    XSocket(SOCKET sck);
    XSocket(U32 flag, ConnectNode* node);
    ~XSocket();
public:
    bool dispatchWas( ConnectNode* root, ListObject<ConnectNode*>& list, struct timeval* timeout=NULL) FTHROW( FSocketException );
    bool dispatchProxy( ConnectNode* root, ListObject<ConnectNode*>& list, struct timeval* timeout=NULL) FTHROW( FSocketException );
    bool dispatchServer( ConnectNode* root, ListObject<ConnectNode*>& list, struct timeval* timeout=NULL) FTHROW( FSocketException );

    XSocket* acceptClient( struct sockaddr* addr, int * addrlen, ConnectNode* node=NULL );
    void setFunc(LPSOCKET_ROUTINE fc ) { xfunc=fc; }
public:
    // __fd_set		m_readFd;
    fd_set				m_readFd;
    SOCKET				m_maxSock;
    ConnectNode*		xnode;
    ConnectNode*		xendNode;
    U32					xflag;
    LPSOCKET_ROUTINE	xfunc;
};


class ConnectNode : public XNode {
public:
    ConnectNode(TreeNode* cf=NULL);
    ~ConnectNode();
public:
    ConnectNode* child(int n)	{ return n<childCount() ? xchilds.getvalue(n): NULL; }
    ConnectNode* parent()		{ return xparent? xparent:this; }
    int childCount()			{ return xchilds.size(); }
    bool remove(ConnectNode* node, int offset=0) {
        for(int n=offset,cnt=childCount();n<cnt;n++) {
            if( child(n)==node ) {
                xchilds.remove(n);
                return true;
            }
        }
        return false;
    }
    int row() {
        if( parent()==this )
            return 0;
        for(int n=0,cnt=parent()->childCount();n<cnt;n++) {
            if( parent()->child(n)==this ) return n;
        }
        return 0;
    }

    void initNode(int idx,LPSOCKET_ROUTINE func=NULL );
    void setProcFunc(LPSOCKET_ROUTINE func, TreeNode* cf=NULL );
    void setSocket(XSocket* s);
    XSocket* getSocket() { return xsock; }
    ConnectNode* acceptClient(LPSOCKET_ROUTINE func=NULL,TreeNode* cf=NULL);
    ConnectNode* addNode(LPSOCKET_ROUTINE func);
    void        removeNode(ConnectNode*cur);
    bool		isRead(int tm=100);
    bool		isWrite(int tm=100);
    bool		valid()		{ return xsock && xsock->IsValid(); }
    int			dispatch();
    int			send(BYTE* msg, int size=0);
    StrVar*		read(StrVar* var, int len);
    StrVar*		recv(StrVar* var=NULL, qint64 size=0);
    int			saveFile(LPCC filenm, qint64 len, BYTE* buf=NULL, int recv=0);
    StrVar*		read(StrVar* var=NULL);
    StrVar*		send(StrVar* var=NULL);
    int         getTotalSize(StrVar* var, int sp, int ep);
    StrVar*		sendData(StrVar* var, int sp, int ep);
    StrVar*		recvData(StrVar* var, int total=0);
    int     	recvProxy(ConnectNode* target, StrVar* var, int total=0);

    StrVar*		getSendVar();
    StrVar*		getContentVar();
    StrVar*		getRecvVar();
    StrVar*     recvHttpHeader(StrVar* recvBuf);

    bool bind(int port, int block, U8 type=0);
    bool connect(LPCC host, int port);
    bool closeClient();
    bool closeSocket();
    int eventMessage(StrVar& var);
    LPCC setIp(const in_addr &addr);
    TreeNode* config() {
        if( xcf==NULL ) {
            xcf=new TreeNode(2, true);
        }
        return xcf;
    }
public:
    XSocket*					xsock;
    TreeNode*					xcf;
    U32							xclientIndex;
    U16							xtype, xstat;
    StrVar						xsend, xcontent;
    long						xtimeout;
    ConnectNode*				xparent;
    ListObject<ConnectNode*>	xchilds;
};


class ConnectNodeBuffer {
public:
    ConnectNodeBuffer() : xnodes(64,64) {}
    ~ConnectNodeBuffer() {
        release();
    }
    static ConnectNodeBuffer& instance() {
        static ConnectNodeBuffer connbuf;
        return connbuf;
    }
    XNodeBuffer<ConnectNode>	xnodes;

public:
    ConnectNode* getNode() {
        return xnodes.getNode();
    }
    void deleteNode(ConnectNode* node) {
        if( node ) {
            xnodes.deleteNode(node);
        } else {
            _LOG("delete ConnectNode error");
        }
    }
    void release() {
        xnodes.destroy();
    }
};


#define gConns	ConnectNodeBuffer::instance()
typedef ListObject<ConnectNode*>	ListConnectNode, *PListConnectNode;


void setSocketReadTimeout (const SOCKET sock, const int timeoutIn );
void setSocketWriteTimeout (const SOCKET sock, const int timeoutIn );
int initConnectNode( ConnectNode* conn, bool keepCheck, bool closeCheck=false );

class MessageServer;
class MessageClient : public QObject
{
    Q_OBJECT
public:
    explicit MessageClient(TreeNode* cf, MessageServer* server, QTcpSocket* sock=NULL, QObject *parent = 0);
    void callback(StrVar* rst=NULL );
    XListArr* getParam(LPCC type);
    void sendVar(StrVar* data);
    void readVar(StrVar* rst);
    void sendBuffer(LPCC buf, int size);
    int readAll(StrVar* rst);
    void closeCheck(bool reset);

public:
    QTcpSocket*		Socket;
    TreeNode*		xcf;
    XListArr		xparam;
    StrVar			xrst;
    MessageServer*	xserver;

public slots:
    void SocketReadyRead();
    void SocketDisconnected();
    void SendResponse(QByteArray data);
    void SendResponse(QString data);
    void CloseSocket();
    void onError(QAbstractSocket::SocketError);
    void onStateChanged(QAbstractSocket::SocketState);
    void onConnect();

private:
    int SessionID;          //! The Socket ID
    QString CommandBuffer;  //! The buffer holding the command

public:
    void SendResponse(LPCC data);
};

class MessageServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MessageServer(TreeNode* cf, QObject *parent = 0);
    void StartServer(qint16 port);
    void StopServer();
    void CloseClient(MessageClient* client);
    void callback(StrVar* rst=NULL );
    XListArr* getParam(LPCC type);
    void sendAll(LPCC msg);
    // void incomingConnection(int socketDescriptor) Q_DECL_OVERRIDE;

public:
    TreeNode*		xcf;
    XListArr		xparam;
    StrVar			xrst;

public slots:
    void acceptConnection();

signals:
    void OnStarted();
    void OnStopped();
private:
    QThread *myThread;
};

bool parseTextFormat(StrVar* var, int sp, int ep, StrVar* rst, StrVar* sv);
bool sendWebStream(ConnectNode* conn, LPCC path, qint64 start, qint64 end, StrVar* rst, XFuncNode* fn, MessageClient* client, StrVar* proto, bool bClose=false);
bool recvWebProxy(ConnectNode* conn, StrVar* rst, XFuncNode* fn, ConnectNode* client, StrVar* header, bool bClose=false);
int readHttpHeader(ConnectNode* conn, StrVar* rst, StrVar* var=NULL );

class UdpServerSocket : public QObject
{
    Q_OBJECT
public:
    explicit UdpServerSocket(TreeNode* cf, QObject *parent = 0);
    ~UdpServerSocket();

public:
    TreeNode*		xcf;
    XListArr		xparam;
    StrVar			xrst;

public:
    void startUdpServer(int port);

private:
    QUdpSocket      *m_hUdpSocket;
    QHostAddress    m_hHostAddress;
    int m_nPort;

public slots:
    void slotsUdpReadDatagram();
};


// A packet makes up the information that will be sent and received
class CPacket
{
public:
    CPacket();
    CPacket(const CPacket & rhs);
    CPacket(StrVar* csPassword, UINT nSessionId);
    CPacket(int nCompThreads,UINT nSessionId);
    CPacket(int cxScreen,int cyScreen,int nBitCount,int nGridThreads);
    CPacket(QRect Rect,char * pBuffer,DWORD dwBytes,DWORD dwSrcBytes,int nCompThreads,BOOL bImgDIB,BOOL bUseCompression,BOOL bAC,BOOL bXOR,BOOL bDIB);
    CPacket(DWORD dwFlags,DWORD dwXHotSpot,DWORD dwYHotSpot,int bmWidth,int bmHeight,WORD bmMaskPlanes,WORD bmMaskBitsPixel,WORD bmColorPlanes,WORD bmColorBitsPixel,std::vector<BYTE> MaskBits,std::vector<BYTE> ColorBits);
    CPacket & operator = (const CPacket & rhs);
    virtual ~CPacket();
    bool operator == (const CPacket & rhs) const;

    // Send the packet
    void SerializePacket(ConnectNode * pSocket,bool bSend);

    // Type of packet
    unsigned char m_ucPacketType;

    // m_ucPacketType = 1
    StrVar m_csPasswordHash;

    // m_ucPacketType = 10
    int m_nCompThreads;
    UINT m_nSessionId;

    // m_ucPacketType = 2
    int m_cxScreen;
    int m_cyScreen;
    int m_nBitCount;
    int m_nGridThreads;

    // m_ucPacketType = 7
    QRect m_Rect;
    DWORD m_dwSrcBytes,m_dwBytes;
    BOOL m_bImgDIB;
    BOOL m_bDIB;
    BOOL m_bUseCompression;
    BOOL m_bAC;
    BOOL m_bXOR;
    DWORD m_dwBufferSize;
    char * m_pBuffer;
    StrVar m_Buffer;
    bool m_bDeleteBuffer;

    // Mouse and Keyboard windows message
    WORD m_wWM;
    UINT m_nFlags;


    // m_ucPacketType = 3
    DWORD m_dwFlags;
    DWORD m_dwXHotSpot;
    DWORD m_dwYHotSpot;
    int m_bmWidth;
    int m_bmHeight;
    WORD m_bmMaskPlanes;
    WORD m_bmMaskBitsPixel;
    WORD m_bmColorPlanes;
    WORD m_bmColorBitsPixel;
    std::vector<BYTE> m_MaskBits;
    std::vector<BYTE> m_ColorBits;
};

#endif

