#ifndef XFUNC_H
#define XFUNC_H

#include "../util/widget_util.h"
#include "./LZ77LIB.H"


#define NODEBUFFER_BIT_USE
#define NODEBUFFER_REUSE		0x100
#define NODEBUFFER_PANDDING		0x200

#define STARTWITH(a,b,size) StartWith((a),(b),-1,(size))
#define AS(a) toString(arrs->get((a)),NULL,true)
#define PV(a,b) StrVar* sv=(a)->gv((b)); XParseVar pv(sv)

#define FTYPE_SET			1
#define FTYPE_VAR			2
#define FTYPE_TEXTVAR		5
#define FTYPE_STR			6
#define FTYPE_SETARRAY		7
#define FTYPE_ARRAY			8
#define FTYPE_CASE			9
#define FTYPE_CASEVAR		19

#define FTYPE_RESERVE		10

#define FTYPE_GROUP			11
#define FTYPE_USEGROUP		12
#define FTYPE_FUNCGROUP		13
#define FTYPE_OPER			15
#define FTYPE_NULL			16
#define FTYPE_BOOL			17

#define FTYPE_FUNCSTART		20
#define FTYPE_FUNC			21
#define FTYPE_FUNC_RESV		22
#define FTYPE_FUNC_THIS		23
#define FTYPE_FUNCLIB		24
#define FTYPE_FUNCCLASS		25
#define FTYPE_FUNCLOCAL		26
#define FTYPE_FUNCINNER		27
#define FTYPE_FUNCOPEN		28
#define FTYPE_FUNCPARSE		29
#define FTYPE_FUNCCHECK		30
#define FTYPE_FUNCNEW		31
#define FTYPE_FUNCA			32
#define FTYPE_FUNCB			33
#define FTYPE_FUNCEND		40

#define FTYPE_PLUS			101
#define FTYPE_MINUS			102
#define FTYPE_DIV			103
#define FTYPE_MULTI			104
#define FTYPE_MOD			105

#define FTYPE_SINGLEVAR		41
#define FTYPE_LIST			42
#define FTYPE_HASH			43
#define FTYPE_EXPR			44
#define FTYPE_CONST			45
#define FTYPE_CALLBACK		46
#define FTYPE_TEMPLATE		47

#define FTYPE_VARCHECK		56
#define FTYPE_TEXT			61
#define FTYPE_TEXTDATA		62

#define FTYPE_DEFINE		111
#define FTYPE_NUMBER		112
#define FTYPE_STATIC		113


#define FTYPE_CONTROL		1001
#define FTYPE_CATCH			1002

#define FTYPE_BREAK			2001
#define FTYPE_RETURN		2002
#define FTYPE_CONTINUE		2003
#define FTYPE_THIS			2004


#define FKIND_SYSTEM		1
#define FKIND_CLASS			2
#define FKIND_WAS			3
#define FKIND_APP			4
#define FKIND_MODEL			5
#define FKIND_PAGE			6
#define FKIND_CF			7
#define FKIND_MATH			8
#define FKIND_NET			9
#define FKIND_EVENT			10
#define FKIND_JSON			11
#define FKIND_GRAPHIC		12
#define FKIND_FUNC			13

#define FKIND_DECODE		51
#define FKIND_PRINT			52
#define FKIND_ARRAY			53
#define FKIND_STR			54
#define FKIND_ISNULL		55
#define FKIND_EXPR			56
#define FKIND_HASH			57
#define FKIND_RANGE			58
#define FKIND_REUSE			59
#define FKIND_REGEXPR		60
#define FKIND_TEXT			62
#define FKIND_PATH			63
#define FKIND_ACTION		64

#define WTYPE_USER(a)		0x10000|(a)
#define WTYPE_OBJECT(a)		0x20000|(a)

#define FFLAG_SUBS			0x1		// SUB function
#define FFLAG_PARAM			0x2		// 자신이 PARAM이다
#define FFLAG_LAST			0x4		// 마지막 function
#define FFLAG_DOTPOS		0x8		// 외부 변수 or 함수 사용

#define FFLAG_USE_SUB		0x10
#define FFLAG_USE_GROUP		0x20
#define FFLAG_CASE			0x40
#define FFLAG_CASETRUE		0x80
#define FFLAG_SET			0x100
#define FFLAG_EQ			0x200
#define FFLAG_PSET			0x400
#define FFLAG_PPARAM		0x800
#define FFLAG_PBOX			(FFLAG_PSET | FFLAG_PPARAM)
#define FFLAG_REF			0x1000
#define FFLAG_SETREF		(FFLAG_SET | FFLAG_REF)
#define FFLAG_NOT			0x2000
#define FFLAG_DEBUG			0x4000
#define FFLAG_SAVE			0x400
#define FFLAG_STRSET		FFLAG_DEBUG
#define FFLAG_OPEFCALL		FFLAG_LAST
#define FUNC_INLINE			0x100


#define FPARAM_FUNC			0x10
#define FPARAM_GROUP		0x20
#define FPARAM_PARAM		0x40
#define FPARAM_CASE			0x80
#define FPARAM_FUNCSUB		0x100

#define FPARAM_LIST			0x1000
#define FPARAM_PLIST		0x2000
#define FPARAM_HASH			0x4000

#define RST_ELSE			0x10

#define FRST_OUTCALL		0x100
#define FRST_STATIC			0x200

#define FRST_POINT_VALUE	0x100

#define RANGE_RECT			0x1000
#define RANGE_POS			0x2000
#define RANGE_SIZE			0x3000

#define FNSTATE_DRAW		4001
#define FNSTATE_MODEL		4002
#define FNSTATE_FILESYSTEM	4003
#define FNSTATE_FILEFIND	4004
#define FNSTATE_PROCESS		4005
#define FNSTATE_JOB			4006
#define FNSTATE_CRON		4007
#define FNSTATE_WORKER		4008
#define FNSTATE_WEBCLIENT	4011
#define FNSTATE_WEBSERVER	4012
#define FNSTATE_WEBPAGE		4013
#define FNSTATE_FTPSERVER	4021
#define FNSTATE_FTPCLIENT	4022
#define FNSTATE_EXCEL		4031
#define FNSTATE_FILE		4041
#define FNSTATE_SNMP		4042
#define FNSTATE_WATCHER		4043
#define FNSTATE_PDF			4044
#define FNSTATE_ZIP			4045
#define FNSTATE_WEBFUNC		4046
#define FNSTATE_SOCKET		4047
#define FNSTATE_PYTHON		4051
#define FNSTATE_TIMELINE	4052
#define FNSTATE_MPPLAYER	4061
#define FNSTATE_MODBUS		4062
#define FNSTATE_WEBNODE		4101
#define FNSTATE_CLASS		4102
#define FNSTATE_AUDIO		4103
#define FNSTATE_GOOGLE		4104
#define FNSTATE_CANVASITEM	4105
#define FNSTATE_TELNET_SERVER		4111
#define FNSTATE_TELNET_CLIENT		4112
#define FNSTATE_MESSAGE_SERVER		4121
#define FNSTATE_MESSAGE_CLIENT		4122
#define FNSTATE_SMTP		4131
#define FNSTATE_POP3		4132
#define FNSTATE_UDP         4133
#define FNSTATE_MQTT        4134

#define SOKETTYPE_MODULE	11
#define WORKER_MODULE		11
#define WORKER_REFRESH		12
#define WORKER_COMPRESS		13
#define WORKER_ASYNC		19

#ifdef WIDGET_USE
#define GFLAG_LAYOUT		0x1000
#define GFLAG_PAGE			0x2000

#define FNSTATE_PAGE		25
#define FNSTATE_CANVAS		27
#define FNSTATE_TREE		28
#define FNSTATE_GRID		29
#define FNSTATE_COMBO		30

#define FNSTATE_SQLITE		41
#define FNSTATE_DB			45
#define FNSTATE_UTIL		61

#define FNSTATE_ACTION		101
#define FNSTATE_MENU		102
#define FNSTATE_TRAY		103
#define FNSTATE_COMPLETER	104 // Completer ==> 12
#define FNSTATE_TOOLBOX		105
#define FNSTATE_TOOLBAR		106
#define FNSTATE_DOCK		107

// WIDGET
#define WTYPE_WIDGET	1
#define WTYPE_BUTTON	2
#define WTYPE_LINEINPUT	3
#define WTYPE_EDIT		4
#define WTYPE_TREE		5
#define WTYPE_TABLE		6
#define WTYPE_LIST		7
#define WTYPE_COMBO		8
#define WTYPE_PROGRESS	9
#define WTYPE_TAB 		10
#define WTYPE_CONTEXT	11

#define WTYPE_GROUP		12
#define WTYPE_CANVAS	20
#define WTYPE_RADIO		21
#define WTYPE_CHECK		22
#define WTYPE_SPIN		23
#define WTYPE_TEXTEDIT	24

// FRAME
#define WTYPE_STACKED	31
#define WTYPE_LABEL		32
#define WTYPE_SPLITTER	33
#define WTYPE_TOOLBOX	34

// BAR
#define WTYPE_TOOLBAR	51
#define WTYPE_MENUBAR	52
#define WTYPE_STATUSBAR	53

// ETC
#define WTYPE_FORM		61
#define WTYPE_DIV		62
#define WTYPE_GRAPHIC	63
#define WTYPE_DATEEDIT	64
#define WTYPE_CALENDAR	65
#define WTYPE_TIMEEDIT	66

#define WTYPE_DIALOG	101
#define WTYPE_MAIN		102
#define WTYPE_PAGE		103

#define WTYPE_DOCK		110
#define WTYPE_ACTION	111
#define WTYPE_MENU		112
#define WTYPE_TRAY		113
#define WTYPE_COMPLETER	114

#define WTYPE_IMGBTN	121
#define WTYPE_LINEEDIT	122
#define WTYPE_WEBVIEW	123
#define WTYPE_BROWSER	124			// TabWidget
#define WTYPE_TEXTBROWSER	125
#define WTYPE_TOOLBTN	126

#define WTYPE_CHART		131
#define WTYPE_POPUP		132
#define WTYPE_PLAYER	134
#define WTYPE_VIDEO     135

#define LTYPE_VBOX		(1<<8)
#define LTYPE_HBOX		(2<<8)
#define LTYPE_GRID		(3<<8)
#define LTYPE_ROW		(4<<8)

#define WKIND_WIDGET	0
#define WKIND_MODEL		1
#define WKIND_PAGE		2
#define WKIND_HTTP		4
#define WKIND_JOB		5
#define WKIND_CRON		6
#define WKIND_WORKER	7
#define WKIND_STRUCT	8
#endif

#define UID_NULL            0xFFFF
#define FUNCVAR_CHECK				( (fc->xfflag==0x200 && fc->parent()->xftype==8) || ( (fc->xfflag&0xF) && ((fc->xfflag&(FFLAG_PARAM|FFLAG_SUBS))==0)) )
#define FLAG_USERFUNC_CALL			0x2000

// #define IMAGESTONE_USE
#define FUNC_REF_USE

class XPos {
public:
    XPos() : xr(UID_NULL),xc(0) {}
	U16 xr,xc;
    void set(U16 r, U16 c) {
		xr=r;
		xc=c; 
	}
	bool isValid() { return xr==UID_NULL ? false: true; }
};

class XType {
public:
	XType();
public:
    XPos	xpos;
    U32     xflag;
public:
    void setNode(U16 r, U16 c=0);
	U16  getXRow();
    U16  getXCol();
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)	{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag) { return xflag&=~flag; }
};

class XStr : public StrVar {
public:
    XStr(LPCC value=NULL);
public:
    XPos	xpos;
public:
    void setNode(U16 r, U16 c=0);
    U16  getXRow();
    U16  getXCol();
};


class XNode : public WBox {
public:
	XNode(int size=2);
public:
	XPos	xpos;
    U32     xflag;
    DWORD	xtick;
    void setNode(U16 r, U16 c=0);
    U16  getXRow();
    U16  getXCol();
    bool isNodeFlag(U32 flag)	{ return (xflag & flag)>0 ? true: false; }
    U32 setNodeFlag(U32 flag)	{ return xflag|=flag; }
    U32 clearNodeFlag(U32 flag) { return xflag&=~flag; }
	void setTick();
	long getTick();
};

class TextPos
{
public:
    int	start;
    int	end;
    TextPos(int s=0, int e=0) { set(s,e); }
    ~TextPos() {}
public:
    const TextPos &	operator=( TextPos& tp ) {
        start = tp.start;
        end = tp.end;
        return *this;
    }
    BOOL IsIn(int pos) { return (pos>=start && pos<=end) ? TRUE : FALSE; }
    int length() { return end-start; }
    TextPos* set(int sp, int ep) {
        start = sp;
        end = ep;
        return this;
    }
};


template <class Etype>
class XNodeBuffer {
private:
	U16 nUnitNum , nUnitCount, nGetPos, nFlag;		// nUnitSize, 
	U16 nCurrent, nCount;
	U32 nMaxNum;
	Etype **The_Lists ;
#ifdef NODEBUFFER_BIT_USE
	PU32 xdelete;
	ListObject<XPos> xpandding;
	ListObject<XPos> xlock;
#else
	ListObject<XPos> xdelete;
#endif
	void Allocate_Lists();

public:
	XNodeBuffer(unsigned int unitNum=10, unsigned int unitCount=10, bool loop=false );
	~XNodeBuffer() ;
	int total();
	Etype* getNode() ;
	void deleteNode(Etype* e) ;
	void reuse();
	void destroy();
	bool isLast();
	bool idle(int delay);
	bool setLock(U16 r, U16 c);
	PU32 getDeleteArray(U32& dsize) {
		dsize = (nUnitNum*nUnitCount)/8;
		return xdelete;
	}
	Etype* getNode(int r, int c) {
		if( r<nUnitNum && c<nUnitCount ) {
			return (Etype*)&The_Lists[r][c];
		} 
		return NULL;
	}
	void getInfo(StrVar* rst) {
		rst->fmt("%d,%d,%d | %d,%d,%d", nUnitNum , nUnitCount, nGetPos, nCurrent, nCount, nMaxNum);
	} 
};

//
//
//
template <class Etype>
XNodeBuffer<Etype>::XNodeBuffer(unsigned int unitNum, unsigned int unitCount, bool loop ) :
	nUnitNum(unitNum), nUnitCount(unitCount), nCount(0), nCurrent(0), nGetPos(0), nFlag(0), nMaxNum(0)
#ifdef NODEBUFFER_BIT_USE
	, xdelete(NULL) 
#endif
{
	// nUnitSize = sizeof(Etype) ;
	int mod=nUnitCount%32;
	if( mod!=0 ) 
		nUnitCount+=mod;
	Allocate_Lists();
}


template <class Etype>
XNodeBuffer<Etype>::~XNodeBuffer() {
	destroy();
}

template <class Etype>
void XNodeBuffer<Etype>::destroy() {
	if( The_Lists ) {
		for( int i=0 ;i<nUnitNum; i++ ) if( The_Lists[i] ) delete [] The_Lists[i] ;
		delete [] The_Lists ;
		The_Lists = NULL;
	}
#ifdef NODEBUFFER_BIT_USE
	SAFE_FREE(xdelete);
#else
	xdelete.Close();
#endif
}

template <class Etype>
void XNodeBuffer<Etype>::Allocate_Lists() {
	try {
		The_Lists = new Etype*[nUnitNum] ;
		for( int i=0 ;i<nUnitNum; i++ ) 
			The_Lists[i] = NULL ;
		The_Lists[0] = new Etype[nUnitCount] ; //E(Etype*)malloc( nUnitSize*nUnitCount );
		// xdelete.Create(MAX_DELETE*sizeof(XPos)+8);
		nMaxNum = 1;
#ifdef NODEBUFFER_BIT_USE 
		U32 dsize = (nUnitNum*nUnitCount)/8;
		xdelete = (PU32)malloc( dsize );
		memset(xdelete,0, dsize );
#else
		xdelete.Create(MAX_DELETE*sizeof(XPos)+8);
#endif
	} catch( ... ) {
	
	}
}

template <class Etype>
void XNodeBuffer<Etype>::reuse() {
	nCurrent = nCount = nGetPos = nFlag = 0;
	nMaxNum = 0;
#ifdef NODEBUFFER_BIT_USE
	SAFE_FREE(xdelete);
#else
	xdelete.reuse();
#endif
}

template <class Etype>
int XNodeBuffer<Etype>::total() {
	return nFlag&NODEBUFFER_REUSE ? nUnitNum*nUnitCount : (nCurrent*nUnitCount)+nCount;
}

template <class Etype>
Etype*  XNodeBuffer<Etype>::getNode() {
	if( (nFlag&3)==0 ) {
#ifdef NODEBUFFER_BIT_USE 
		int sz=(total()/32)+1;
		nFlag|=1;
		for( int n=0;n<sz;n++) {
			if( xdelete[n] ) { 
				int sp = xdelete[n]&0xFF ? 0 : xdelete[n]&0xFF00 ? 8 : xdelete[n]&0xFF0000 ? 16 : 24;
				int ep = sp+8;
				for( ; sp<ep; sp++ ) {
					if( xdelete[n] & (1<<sp) ) {
						int x=(n*32)+sp;
						int r=x/nUnitCount, c=x%nUnitCount;
						Etype* e = (Etype*)&The_Lists[r][c];
						xdelete[n] &= ~(1<<sp);
						e->setNode(r,c);
						nFlag&=~1;
						return e;
					}
				}
			}
		} 
		nFlag&=~1;

#else
 		int size = xdelete.size();
		if( size>0 ) { 
			nFlag|=1;
			if( nGetPos==size ) 
				nGetPos = 0;
			for( int n=nGetPos;nGetPos<size;nGetPos++ ) {
				XPos* pos = xdelete[n];
				if( pos && pos->xr<nMaxNum ) {
					U16 r= pos->xr,c=pos->xc;
					Etype* e = r<nMaxNum ? (Etype*)&The_Lists[r][c]: NULL;
					if( e==NULL ) {
                        _LOG("@@ XNodeBuffer<Etype>::getNode() (delete list maybe set null)");
						break;
					}
					if( pos!=xdelete[n] ) {
                        _LOG("@@ XNodeBuffer<Etype>::getNode() (maybe delete add error)");
						if( r!=xdelete[n]->xr )
                            _LOG("@@ XNodeBuffer<Etype>::getNode() (row error)");
						break;
					}
					e->setNode(r,c);
					pos->set(UID_NULL);
					nFlag&=~1;
					if( nGetPos>0 ) 
                        _LOG("@@ XNodeBuffer<Etype>::getNode() (%d,%d)\n",size, nGetPos );
					return e;
				} 
				/*
				else {
                    if( nGetPos==0 ) _LOG("@@ XNodeBuffer<Etype>::getNode() (%d,%d)\n", size, nGetPos );
                    break;
				}
				*/
			}
			if( nGetPos==size )
				xdelete.reuse();
			nGetPos = 0;
			nFlag&=~1;
		} 

#endif
	} else {
        _LOG("@@ XNodeBuffer<Etype>::getNode() padding (%d,%d)\n", nCurrent, nCount );
	}	
	if( nCount>=nUnitCount ) {
		nCurrent++ ;
		if( nCurrent>=nUnitNum ) {
			nCurrent = 1;
			nFlag|= NODEBUFFER_REUSE;
		}
		nCount = 0 ;
		if( The_Lists[nCurrent]==NULL ) {
			The_Lists[nCurrent] = new Etype[nUnitCount];
		}
		if( nCurrent>nMaxNum ) {
			nMaxNum = nCurrent;
		}
	} 
	Etype* e = (Etype*)&The_Lists[nCurrent][nCount];
	if( e ) 
		e->setNode(nCurrent,nCount++);	 
	return e;

}

template <class Etype>
void XNodeBuffer<Etype>::deleteNode(Etype* e) {
	if( e==NULL )
		return;
	if( nFlag&NODEBUFFER_PANDDING ) {
		xpandding.add(XPos())->set(e->getXRow(),e->getXCol());
		return;
	}
#ifdef NODEBUFFER_BIT_USE 
	U16 r=e->getXRow(), c=e->getXCol();
    if( r==UID_NULL ) return;
	if( r<=nMaxNum ) {
		if( (nFlag&3)==0 ) {
			nFlag|=2;
			int tot = ((r*nUnitCount)+c);
			int idx = tot/32;
			xdelete[idx] |= (1<<(tot%32));
			nFlag&=~2;
		} else {
			xpandding.add(XPos())->set(e->getXRow(),e->getXCol());
            _LOG("@@ delete buffer realloc");
		}
		e->setNode(UID_NULL);
	} else {
        _LOG("@@ void XNodeBuffer<Etype>::deleteNode (delete object not valid)\n");
	}
#else
	if( (nFlag&1)==0 && xdelete.size() ) {
		for( int n=0,size=xdelete.size();n<size;n++ ) {
			XPos* pos = xdelete[n];
			if( pos && pos->xr==UID_NULL ) {
				pos->set(e->getXRow(),e->getXCol());
				nFlag&=~2;
				return;
			}
		}
	}
	if( e->getXRow()<nMaxNum ) {
		xdelete.add(XPos())->set(e->getXRow(),e->getXCol());
	} else {
        _LOG("@@ void XNodeBuffer<Etype>::deleteNode (delete object not valid)");
	}
	e->setNode(UID_NULL);
#endif

}

template <class Etype>
bool XNodeBuffer<Etype>::isLast() {
	return (nCurrent==nUnitNum && nCount==nUnitCount)? true: false;
}

template <class Etype>
bool XNodeBuffer<Etype>::idle(int delay) {
#ifdef IDLE_USE
    for( int r=0;r<nUnitNum;r++) {
        for( int c=0;c<nUnitCount;c++) {
            if( (GetTickCount()-The_Lists[r][c].xtick)>delay ) {
                The_Lists[r][c].unuse();
                return true;
            }
        }
    }
#endif
    return false;
}

template <class Etype>
bool XNodeBuffer<Etype>::setLock(U16 r, U16 c) {
	return false;
}



class XListArr;
class XFunc;
class XFuncSrc : public XStr {
public:
	XFuncSrc();
public:
	XParseVar			xpv;
	StrVar				xparam;
    XFunc*				xfunc;
    U32                 xflag;
public:
    bool err(int sp, LPCC msg);
	bool readBuffer(StrVar* var, int sp=0, int ep=0);
	bool readFile(LPCC filenm);
	int subOper(StrVar* data, char c, int sp, int ep, char ft, U32* endchk=NULL);
	bool makeFunc(StrVar* data, int sp, int ep, int psp, int pep, char& ft );
	bool makeSub(XParseVar& pv, char ft);
	bool parseData(StrVar* data, int sp, int ep, bool sub=false);
	bool makeFunc();
	bool makeFunc(XParseVar& pv, XFunc* pfunc, U32 flag=0);
	int makeFuncCase(XParseVar& pv, U32 flag, XFunc** ppfunc, XFunc* pfunc, XFunc* pval=NULL );
	bool setLine(XParseVar& pv, XFunc** ppfunc, XFunc* pfunc, U32 flag );
 	int isFunc(StrVar* data, int sp, char& ft );
	int endVarPos(StrVar* data, int sp );
	void setParam(StrVar* var, int sp, int ep ) {
		xparam.reuse();
		xparam.add(var,sp,ep);
	}
	StrVar*  getParam() { return & xparam; }
};

//
//
//

class XFunc : public XType {
public:
	XFunc();
public:
 	void setSrc(XFuncSrc* src);
	XFuncSrc* getFuncSrc();
	void deleteFuncSrc();
	void setValuePos( int sp, int ep);
	XFunc* addParam();
	XFunc* addFunc();
	XFunc* getFunc(int n);
	XFunc* getParam(int n);
	XFunc* prevFunc();
	XFunc* prevParam();
	XFunc* nextFunc();
	XFunc* nextParam();
	int getFuncPos();
	int getParamPos();
	int getParamSize()	{ return xparams.size(); }
	int getFuncSize()	{ return xsubs.size(); }
	XFunc* parent()		{ return xparent; }
	bool printFunc(StrVar& var, int depth);
	bool printParam(StrVar& var, int depth, U32 flag);
	LPCC getValue();
	LPCC getVarName();
	LPCC getFuncName();
    LPCC getValueBuf(char* buf, int buflen);
    LPCC getVarBuf(char* buf, int buflen);
    LPCC getFuncBuf(char* buf, int buflen);
	U16 getType() { return xftype; }
	U16 getParentType() { return parent() ? parent()->xftype: 0; }
public:
	ListObject<XFunc*>	xparams;
	ListObject<XFunc*>	xsubs;
	XFunc*				xparent;
	U16					xftype, xfflag;
	U16					xdotPos, xfkind;
	U32					xfuncRef;
	XParseVar			xpv;	// pvar == xfuncsrc
};
//
//
//
class XFuncNode : public XNode { 
public:
    XFuncNode() : XNode(), xtype(0), xstat(0), xparent(NULL) {
		initVar();
	} 
public:
	XFuncNode* child(int n)	{ return n<childCount() ? xchilds.getvalue(n): NULL; }
	XFuncNode* parent()		{ return xparent; }
	int childCount()		{ return xchilds.size(); }
    XFuncSrc* getFuncSrc()  { return xfunc? xfunc->getFuncSrc(): NULL; }
    StrVar* getParams() {
        XFuncSrc* fsrc=getFuncSrc();
        return fsrc ? &(fsrc->xparam): NULL;
    }
	bool remove(XFuncNode* node, int offset=0) {
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
	XFuncNode* addfuncNode(XFunc* fc);
	int call(XFunc* fc=NULL, StrVar* rst=NULL);
	int callIf(XFunc* fc, StrVar* rst, bool& bcalled);
	int callSwitch(XFunc* fc, StrVar* rst);
	int callFor(XFunc* fc, StrVar* rst);
	U16 getNodeState()								{ return xstat&0xFF; }
	void setNodeState(U16 stat)						{ xstat|=stat&0xFF; }
	void reuseFuncNode();
	void initVar();
	StrVar*	getResult()		{ return &xrst; }
	XFuncSrc* getCommSrc()	{ return &xsrc; }
	bool checkTick(long msec ) { return GetTickCount()-xtick < msec; }
	int setJson(XParseVar& pv, XFuncNode* fn=NULL);
public:
    U16							xtype, xstat;
	XFuncNode*					xparent;
	XFunc*						xfunc;
	ListObject<XFuncNode*>		xchilds;
	StrVar						xrst;
	XFuncSrc					xsrc;
};
//
//
//
class XFuncBuffer {
public:
	XFuncBuffer() : xnodes(512,512) {
	}
	static XFuncBuffer& instance() {
		static XFuncBuffer funcbuf;
		return funcbuf;
	}
	XNodeBuffer<XFunc>			xnodes;

public:
	XFunc* getFunc(XFuncSrc* src);
	void deleteFunc(XFunc* func);
	void release() { xnodes.destroy(); }
};

//
//
//
class XFuncNodeBuffer {
public:
	XFuncNodeBuffer() : xnodes(256,256) {
	}
	static XFuncNodeBuffer& instance() {
		static XFuncNodeBuffer xfnbuf;
		return xfnbuf;
	}
	XNodeBuffer<XFuncNode>			xnodes;

public:
	XFuncNode* getFuncNode(XFunc* fc=NULL, XFuncNode* pfn=NULL);
	void deleteFuncNode(XFuncNode* fn);
	void release() { xnodes.destroy(); }
};

//
//
//
class XFuncSrcBuffer {
public:
	XFuncSrcBuffer() : xnodes(256,512) {
	}
	static XFuncSrcBuffer& instance() {
		static XFuncSrcBuffer xfsbuf;
		return xfsbuf;
	}
	XNodeBuffer<XFuncSrc>			xnodes;

public:
	XFuncSrc* getFuncSrc();
	void deleteFuncSrc(XFuncSrc* fs);
	void release() { xnodes.destroy(); }
};


typedef XStr**		PPStr;
typedef StrVar**	PPVar;

#define gfsrcs	XFuncSrcBuffer::instance() 
#define gfuncs	XFuncBuffer::instance() 
#define gfns	XFuncNodeBuffer::instance() 

//#> string parse object
class ParseText : public XParseVar {
public:
    ListObject<TextPos> poss;

public:
    ParseText(XParseVar& pv): XParseVar(pv) {}
public:
    int Find( LPCC find, int ep=-1, U32 flag=FIND_SKIP);
    BOOL FindCheck(int pos=-1, int* endpos=NULL);
    int AddComment( LPCC st, LPCC et, int s=0, int e=-1 );
    int AddLineComment( LPCC st, int s=0, int e=-1);
    StrVar* MakeText(StrVar* rtn, int first=0);
};

class ZipVar : public XParseVar {
public:
    CLZ77Lib	xzip;
    StrVar      xrst;
public:
    ZipVar(StrVar* var, int sp=0, int ep=0);
    BOOL Encode();
    BOOL Decode();
    BOOL baseEncode(StrVar* var);
    BOOL baseDecode(StrVar* var);
    StrVar* JEncode(StrVar* var, int key=12345 );
    StrVar* JDecode(StrVar* var, int key=12345 );
};


#endif

