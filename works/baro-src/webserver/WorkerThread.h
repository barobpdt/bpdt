#ifndef WorkerThread_H
#define WorkerThread_H

#include "ConnectNode.h"

#ifdef WINDOW_USE
#ifndef _NO_FED_NAMESPACE
	using namespace fed;
#endif
#else
USING_NAMESPACE_FED
#endif

#define WORKER_SIGNAL	0x10000
#define WORKER_EVENT	0x20000
#define WOKRER_DOWNLOAD		0x200
#define WOKRER_UPLOAD		0x400
#define WOKRER_POST			0x800
#define WOKRER_UPDOWNLOAD	( WOKRER_UPLOAD | WOKRER_DOWNLOAD | WOKRER_POST )

#define WOKRER_PARSE_ERROR	0x10000
#define WOKRER_IO_ERROR		0x20000
#define WOKRER_EXCEPTION	0x40000
#define FLAG_WEB_CALL		0x1000

class HttpServerThread;
class WorkerThread : public FThread {
public:
	WorkerThread( TreeNode* cf ); 
	~WorkerThread();

public:
	void Run();
	void Final();
    HttpServerThread* httpServer;
	TreeNode*		xconfig;
	XListArr		xarr;
	XListArr		xparams;
	XListArr		xwait;
	U32				xflag;
	DWORD			xpushTick;
	U16				xtype, xstate;
	QIODevice*		xio;
	XFuncNode*		xfnWorker;
	StrVar			xrst;
	FMutex			xlock;

	bool exec(); 
    XListArr* getWorker() { return &xarr; }
	U32 flag() { return xflag; }
	U32 type() { return xtype; }
	TreeNode* config() { return xconfig; }
		
	void	push(StrVar* sv);
	void	pushNode(TreeNode* node);
	void	pushObject(char ty, U16 stat, LPVOID obj);

#ifdef ZIP_USE
	LPCC makeUnzipEntry(TreeNode* root );
	LPCC parseCentralDirectoryRecord(char* buf, TreeNode* root);
	LPCC unzipFile(LPCC path, TreeNode* node );
	LPCC parseLocalHeaderRecord(LPCC path, TreeNode* node);
	LPCC extractStoredFile( TreeNode* node, quint32& crc, QIODevice* outDev);
	LPCC inflateFile( TreeNode* node, quint32& crc, QIODevice* outDev );

	bool addZip(QIODevice& dev, LPCC path, TreeNode* root=NULL );
#endif
};

class WorkerGroup {
public:
	WorkerGroup( int size=4, U32 flag=0);
	~WorkerGroup();
public:
	U32							xflag;
	int							xworkerIdx;
	int							xworkerSize;
	ListObject<WorkerThread*>	xworkerList;
public:
	void initWorker(int size, U32 flag=0);
	WorkerThread* getWorker();
	WorkerThread* getWorker(int n);
	void reset();
	void idle();
	void stop();
	int getWorkerSize() { return xworkerSize; }
	int getWorkerIndex() { return xworkerIdx; }
};

/************************************************************************/
/*  ũ��ó���� ���� ���μ���                                            */
/************************************************************************/ 

class CronThread : public FThread {
public:
	CronThread( TreeNode* cf ); 
	~CronThread();

public:
	void Run();
	void Final();
	bool exec(); 
	
	TreeNode*		xconfig;
	TreeNode*		xroot;
	XListArr		xarr;
	U32				xflag;
	U16				xtype, xstate;
	
	XListArr* getWorker() { return &xarr; } 
	void	push(StrVar* sv);

	U32 flag() { return xflag; }
	U32 type() { return xtype; }
	TreeNode* config() { return xconfig; }
	TreeNode* root();
};


#endif

