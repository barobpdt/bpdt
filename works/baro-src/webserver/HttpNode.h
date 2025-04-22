#ifndef HTTP_NODE_H
#define HTTP_NODE_H

#include "ConnectNode.h"
#include "WorkerThread.h"

class WorkerThread;
class HttpServerThread : public FThread {
public:
	HttpServerThread(TreeNode* cf);
	~HttpServerThread();
public:
	void Run();
	void Final();
public:
	void startHttpServer();
	void procHttpServer();
    ConnectNode* getRootNode() {
        return xroot;
    }
    int clientCloseCheck();
	TreeNode* config() { return xcf; }
	WorkerThread* getWorker();
	WorkerThread* getWorkerFile();
	WorkerThread* getWorkerConnection();
#ifdef FUNCNODE_USE
    XFuncNode* getFuncNode(ConnectNode* cn, XFuncSrc* fsrc, TreeNode* page=NULL, U32 flag=0);
#endif
public:
	ConnectNode*	xroot;
    WorkerThread*   xmainWorker;
	timeval			xtm;
	TreeNode*		xcf;
	StrVar			xvar;
	XListArr		xarrs;
	WorkerGroup		xwebDataWorkers;
	WorkerGroup		xwebFileWorkers;
	WorkerGroup		xwebConnectionWorkers;
    bool            xcloseCheck;
    int				xfnSize;

	ListObject<ConnectNode*>	xnodeList;
    ListObject<XFuncNode*>		xfnList;
	ListObject<WorkerThread*>	xworkerList;
	ListObject<WorkerThread*>	xworkerFile;
};

int getWebContentLength(StrVar* var);
DWORD IGIMSAPI socketHttpAcceptFunc( LPVOID lpConnectNode ); 
DWORD IGIMSAPI socketHttpReadFunc( LPVOID lpConnectNode );  
StrVar* sendHttpHeader(ConnectNode* conn, StrVar* rst, qint64 length, LPCC ctype=NULL, bool local=false );
void setHttpContentType( TreeNode* node);

bool httpParseHeader(ConnectNode* node ); 
void httpParseParam(ConnectNode* node, LPCC param );
bool httpParseUrl(ConnectNode* node );
int httpRequest(ConnectNode* node );
bool callHttpFunc(ConnectNode* node, LPCC url, int check);

char* getUrlDecode(const char *source, StrVar* var );
char* getUrlEncode(const char *source, StrVar* var);
LPCC getHttpContentType( LPCC ext);
bool httpSend(ConnectNode* node );
bool httpSendFile(ConnectNode* node );
bool httpSendContent(ConnectNode* node, StrVar* content );
char* httpDate(StrVar* var);
LPCC httpStatusCode( int status );
U32 httpFileState( LPCC filename, StrVar* rst, LPCC modified, PUL64 filesize );

U32 parseMultiPartData(ConnectNode* node, XParseVar& pv, UL64 size);
U32 parsePostData(ConnectNode* node, XParseVar& pv, StrVar* decode, U32 flag=0);
U32 httpParseParamVar(ConnectNode* node, XParseVar& pv, StrVar* decode);

bool wasCallApiFun(ConnectNode* node, LPCC url);
bool wasCallProxyFun(ConnectNode* node, LPCC url);

typedef ListObject<TreeNode*> TreeNodeList, *PTreeNodeList;

#endif
