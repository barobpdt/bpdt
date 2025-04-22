#ifndef FUNC_UTIL_H
#define FUNC_UTIL_H
#include "XFunc.h"
#include "SqlDatabase.h"
#include "KorAutomata.h"

#include "../webserver/ConnectNode.h"
#ifdef WIDGET_USE
#include "widgets.h"
#include "widget_video.h"
#endif
#ifdef WEBVIEW_USE
#include "widget_webview.h"
#endif
#ifdef EXCEL_USE

#endif

#ifdef _WIN32
typedef CONST WCHAR *LPCWCHAR, *PCWCHAR;
typedef CONST WCHAR UNALIGNED *LPCUWCHAR, *PCUWCHAR;
#endif

using namespace RBAUTOMATA;

#ifdef WEBVIEW_USE
bool callWebViewFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, WebView* wv, XFunc* fc);
#endif

int execCallFunc(XFunc* func, XFuncNode* fn, StrVar* rst);
bool execObjectFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst, StrVar* origin=NULL, LPCC funcName=NULL );
bool callNodeFunc(TreeNode* tn, LPCC fnm, PARR arrs, XFuncNode* fn, StrVar* rst, StrVar* var, XFunc* fc=NULL);
XListArr* baroSourceApply(StrVar* var, int sp, int ep, StrVar* rst=NULL, XListArr* arr=NULL);
bool baroSourceUpdate(StrVar* var, int sp, int ep, StrVar* rst);
bool includeBaro(LPCC baseFileName);



#define INFO_BUFFER_SIZE	32767
#define SOCKETTYPE_ASYNC	0x800
#define FLAG_LOCALARRAY_SET 0x1000

#define DIRECT_LEFT		0x10
#define DIRECT_RIGHT	0x20
#define DIRECT_UP		0x40
#define DIRECT_DOWN		0x80

#define XML_ROOT	9
#define XML_VALUE	1
#define XML_LIST	2
#define XML_ROW		3
#define XML_DATA	4
#define XML_SUB		5
#define XML_TAGS	9
#define XML_ERROR	99

#define HTTP_INIT		0x10000
#define HTTP_ERROR		0x20000
#define HTTP_DOWNLOAD	0x40000
#define HTTP_UPLOAD		0x80000
#define HTTP_PROGRESS	0x100000
#define HTTP_FINISH		0x800000

#define EF_REGEXP		0x100
#define EF_CASE			0x200
#define EF_BACK			0x400
#define EF_WHOLEWORD	0x800
#define EF_MARK			0x1000
#define EF_SELECTTEXT	0x2000
#define EF_REPLACEALL	0x4000
#define EF_WRAPEND		0x8000

class MyTimeLine;
//////////////////////////////////////////////////////////////////////
//#> func exec common
void regArrayFuncs();
void regSystemFuncs();
void regConfigFuncs();
void regMathFuncs();
bool execArrayFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* sv, StrVar* rst);
bool execInternalFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);
bool execSystemFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);
bool execConfigFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);
bool execMathFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);
bool execMemberFunc(StrVar* sv, PARR arr, StrVar* rst, XFuncNode* fn, TreeNode* tn, LPCC funcName );
bool execCallSubfunc(LPCC funcNm, LPCC subNm, XFuncNode* fn, XListArr* arrs, StrVar* rst, bool bSubClass, bool bExtern=false, bool bSwitch=false );
TreeNode* baroWidgetApply(LPCC tag, LPCC id, StrVar* var, int sp, int ep, LPCC classId );
bool setBaroDefine(StrVar* var, int sp, int ep, TreeNode* node);

//#> func class
void regClassFuncs();
bool execClassFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);
bool setClassRect(PARR arrs, StrVar* rst, bool bDouble=false);
bool setClassPoint(PARR arrs, StrVar* rst, bool bDouble=false);
void setClassTag(LPCC tag, TreeNode* node, U32 flag=0 );
bool setClassTagObject(LPCC tag, LPCC id, StrVar* rst=NULL, StrVar* svObj=NULL);
int setClassFuncNode(TreeNode* h, XFuncNode* pfn, char ch, TreeNode* thisNode, U16 stat );
TreeNode* getBaroObject(LPCC tag, LPCC id);
bool setClassCanvas(PARR arrs, StrVar* rst);
bool setClassColor(PARR arrs, StrVar* rst);

//#> func util
TreeNode* getTreeNodeTemp();
XListArr* getListArrTemp();
StrVar* getStrVarTemp();
XListArr* getLocalArray(bool bset=false);
StrVar* getStrVarPointer(StrVar* sv, int* sp, int* ep );
SqlDatabase* getModelDatabase(LPCC code, LPCC inode=NULL, TreeNode* node=NULL );
void deleteAllTreeNode(TreeNode* root, U32 flag=0);
void deleteAllArray(XListArr* a);
void deleteAllFunc(XFunc* fc);
void funcNodeDelete(XFuncNode* fnCur );
XFuncSrc* getFuncSrc();
XFuncNode* getInitFuncNode(TreeNode* node);
bool reloadFuncSrc(XFuncSrc* fsrc, StrVar* sv, int sp, int ep, LPCC fnm );
void setFuncSrcParam(XFuncNode* fnCur, PARR arrs, XFuncSrc* fsrc=NULL, int start=0, XFuncNode* fn=NULL);
// func call
bool callScriptFunc(StrVar* src, int sp, int ep, XFuncNode* fn=NULL);
bool callScript(StrVar* src, XFuncNode* fn=NULL);
bool callEventFunc(XFuncNode* fn, XFunc* fc, StrVar* rst);
#ifdef SETIF_USE
bool callIfCheck(XFunc* fc, XFuncNode* fn, StrVar* rst );
#endif
int callControl(XFunc* fc, XFuncNode* fn, StrVar* rst, int* offset);
int callSubFuncs(XFunc* fc, XFuncNode* fn, StrVar* rst);
bool callFuncSrc(XFuncSrc* fsrc, XFuncNode* fn, bool remove, TreeNode* thisNode, TreeNode* meNode, LPCC eventNodeName, PARR arrs=NULL, StrVar* rst=NULL );
bool callObjectFunc(LPCC funcName, StrVar* cur, XFuncNode* fn, StrVar* rst, XFunc* func=NULL);
bool callModuleFuncCheck(TreeNode* tn, LPCC fnm, XListArr* arrs, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool callAutomataFunc(XFunc* fc, XListArr* arrs, XFuncNode* fn, StrVar* rst, StrVar* var);
// func load
void setUseVars(XFunc* func, XFuncNode* fn, StrVar* rst=NULL);
bool setMemberSource(TreeNode* node, XFuncNode* fn, StrVar* src, int sp=0, int ep=0, LPCC funcLine=NULL);
int setFuncSource(StrVar* sv, int sp=0, int ep=0, LPCC funcLine=NULL, bool bset=false, bool bevent=false);
StrVar* getSubFuncSrcSqlite(LPCC funcCode);
StrVar* getSubFuncSrc(LPCC funcNm, LPCC val, bool sqliteUse=true );
StrVar* getCmsFuncSrc(LPCC funcNm );
unsigned long mixNum(unsigned long a, unsigned long b, unsigned long c);
void printCallLog(LPCC msg);
void printVar(StrVar* sv, LPCC var, XFuncNode* fn, StrVar* rst);
void printBox(WBox* box, StrVar* rst, int depth=0);
void getPrintInfo(LPCC type, XFuncNode* fn, StrVar* rst, PARR arrs);
TreeNode* mergeNode(TreeNode* info, TreeNode* node, U32 flag=0);
StrVar* loadWebPageFunc(LPCC pageCode, TreeNode* uriMap, StrVar* rst );

//#> check
bool isCheckFunc(XFunc* fc );
bool isTrueCheck(XFunc* fc);
bool isFuncType(XFunc* func);
bool isVarType(XFunc* func);
bool isStringVar( StrVar* sv );
bool funcTrueCheck(XFunc* fc, XFuncNode* fn, StrVar* rst );
bool isFuncTrue(XFunc* fcParam, XFuncNode* fn, StrVar* rst );
LPCC getBaseFuncName(XFuncNode* fn);
void releaseLocalArray(XListArr* arrs);
bool isUpperCase(char c);
char getFuncType(LPCC fnm);
int getControlFuncKind(XFunc* func);
bool funcArrayVarOperCheck(StrVar* kv, int sp, int ep);
bool funcArrayVarCheck(StrVar* kv, int sp, int ep);
StrVar* getArrayKeyVar(StrVar* var, LPCC key, XFuncNode* fn, StrVar* rst );
bool compareResult(StrVar* lr, StrVar* rr, LPCC op );

//#> func var
int toNum(StrVar* sv, int def=0);
int baroFuncEndPos( StrVar*sv, int sp=0, int ep=0 );
XListArr* getLocalParams(XFuncNode* fn);
int setFuncNodeParams(XFuncNode* fn, XListArr* arrs);
StrVar* getNodeVar(TreeNode* node, XFuncNode* fn, LPCC key, bool vchk=true, U32 flag=0, bool bset=false );
StrVar* getFuncVar(XFuncNode* fn, LPCC code, bool memberCheck=true);
StrVar* getVarValue( TreeNode* tn, LPCC cd, StrVar* rst=NULL);
StrVar* getMeVar(XFuncNode* fn );
StrVar* getSubVarValue( TreeNode* tn, LPCC fnm);

bool takeVarAdd(StrVar* rst, XFunc* func, XFuncNode* fn);
bool takeParseVar(StrVar* rst, XFunc* func, XFuncNode* fn);
StrVar* getFuncArrayVar(XFuncNode* fn, LPCC left, StrVar* kv, int sp, int ep, StrVar* rst, StrVar* sv=NULL, XFunc* param=NULL );
StrVar* parseArrayVar(XParseVar& sub, TreeNode* node, XFuncNode* fn, StrVar* rst, U32 flag=0, XFunc* fc=NULL );
bool makeTextData(XParseVar& pv, XFuncNode* fn, StrVar* rst, U32 flag=0, TreeNode* tn=NULL );
StrVar* getFuncStrVarValue(XFunc* func, XFuncNode* fn, StrVar* rst=NULL);
void getOperValue(XFuncNode* fn, XFunc* param, int size, StrVar* rst);
bool setResultVal(XParseVar* pv, XFuncNode* fn, StrVar* result, TreeNode* target=NULL);
bool setModifyClassFunc(TreeNode* node, StrVar* sv, int sp, int ep, XFuncNode* fn, StrVar* rst, bool append=false, LPCC funcLine=NULL);
XFuncNode* setFuncNodeParent(XFuncSrc* fsrc, XFuncNode* fn, TreeNode* thisNode=NULL, TreeNode* meNode=NULL);
XFuncNode* setCallbackFunction(XFuncSrc* fsrc, XFuncNode* fn, TreeNode* thisNode=NULL, XFuncNode* fnPrev=NULL, StrVar* sv=NULL);
XFuncNode* getEventFuncNode(TreeNode* tn, LPCC eventName );
XFuncNode* setFuncResult(XFuncNode* pfn, StrVar* var, int psp, int pep, int sp, int ep, StrVar* rst);
XListArr* setArrayVal(XParseVar* pv, XListArr* arr, XFuncNode* fn=NULL);
TreeNode* setJsonVal(XParseVar* pv, TreeNode* node, XFuncNode* fn=NULL);
StrVar* setStrVar(StrVar* rst, StrVar* sv);
void getObjectType(StrVar* var, StrVar* rst);
LPCC getObjecTypeName(StrVar* sv);
bool getTypeOfResult(LPCC ty, StrVar* sv, StrVar* rst, XFunc* fc=NULL);
void getTypeOf(XFunc* fc, StrVar* rst, XFuncNode* fn=NULL );
void nodeInjectVar(XParseVar* pv, TreeNode* node, XFuncNode* fn, XFuncNode* fnInit=NULL);

//#> func exec
U32 makeLocalFunction( XFunc* func, bool bset, XFuncNode* fn, StrVar* rst, LPCC name );
XListArr* getParamArray(XFunc* func, XFuncNode* fn, StrVar* rst=NULL);
void execParamFunc(XFunc* param, XFuncNode* fn, StrVar* var=NULL);
int execParams(XFunc* func, XFuncNode* fn, StrVar* rst );
int execFunc(XFunc* func, XFuncNode* fn, StrVar* rst, StrVar* cur=NULL );
int execFuncSet(XFunc* func, XFuncNode* fn, StrVar* origin=NULL );
void execSubObjectFunc(XFunc* func, XFuncNode* fn, StrVar* rst, int depth=0);
bool execUserFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst, LPCC funcName=NULL);
XFuncSrc* nodeInlineFunc( XFunc* fc, int findex, StrVar* var, int psp, int pep, int sp, int ep, StrVar* rst );
int nodeFuncCall(TreeNode* node, XFuncNode* fn, XFuncSrc* fsrc, StrVar* rst, XParseVar* param, bool bParam, bool bSubfunc, bool bMember, XListArr* arr=NULL );
bool funcCall(StrVar* sv, XFuncNode* fn, XListArr* arrs, StrVar* rst, bool bmap=false);
bool callUserFunc(XFuncSrc* fsrc, PARR arrs, XFuncNode* fn, StrVar* rst, int startParam=0, U32 flag=0);
bool callEvalFunc(StrVar* sv, int sp, int ep, XFuncNode* fn, StrVar* rst, TreeNode* thisNode, StrVar* appendVar, bool bset=false );

//#> func exec check
bool execCheckFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst);

//#> func exec str
void regStrFuncs();
bool execStrFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* sv, StrVar* rst, StrVar* orgin=NULL);
int execFuncStr(XFuncNode* fnParent, StrVar* sv, int sp, int ep, StrVar* rst, LPCC param);
void lpadString(LPCC a, int mx, StrVar* rst, char ch);
void lpad(int num, int mx, StrVar* rst, char ch);
void getStringTypeValue(StrVar* sv, int sp, int ep, StrVar* rst, bool bset=false);
void decodeUnicode(StrVar* sv, int sp, int ep, StrVar* out);
void encodeEntities(LPCC src, LPCC st, LPCC et, int mod, int shift, int base, StrVar* rst );
void decodeEntities( const QString& src, const QString& exp, int mod, int base, StrVar* rst );

//#> func exec node
void regNodeFuncs();
int parseVarFunc( XParseVar* pv, TreeNode* node, LPCC code, StrVar* rst, XFuncNode* fn, XFunc* fc);
bool callExecNodeFunc(TreeNode* node, XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* refVar=NULL, StrVar* rst=NULL);
bool objectFunction(XFunc* fc, LPCC fnm, StrVar* var, PARR arrs, XFuncNode* fn, StrVar* rst);
int nodeInlineSource(TreeNode* node, XFuncNode* fn, StrVar* var, int sp, int ep, StrVar* rst, LPCC curId, LPCC curMode, bool event=false, bool checkClass=false, bool checkExtend=false);
//#> func exec db
void regDbFuncs();
bool callDbFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool callModelFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst);
bool modelDbExecute(TreeNode* tn, PARR arrs, StrVar* rst, int mode=0, TreeNode* root=NULL, XFuncNode* fn=NULL );
StrVar* parseDbQuery(StrVar* sv, XListArr*a, TreeNode* param, StrVar* rst );
LPCC getQueryString(StrVar* sv, XListArr* a, TreeNode* b, StrVar* rst, int sp, int ep, XListArr* z );

//#> func exec draw
bool callDraw(QPainter* p, StrVar* var, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, LPCC fnm, XFunc* fc);
bool callDrawFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool drawApplyEffect(QPainter* painter, QRect& rc, QPixmap* src, QGraphicsEffect *effect, int extent, int sx, int sy, int sw, int sh, LPCC imageMode=NULL, StrVar* rcTarget=NULL );
bool fillGradient(QPainter* painter, QRect& rc, double x, double y, double dx, double dy, XListArr* arr, int idx, QPainterPath* path=NULL );
bool fillRadialGradient(QPainter* painter, QRect& rc, double x, double y, double dx, double dy, double radius, XListArr* arr, int idx, QPainterPath* path=NULL );
bool fillConicalGradient(QPainter* painter, QRect& rc, double x, double y, XListArr* arr, int idx, QPainterPath* path=NULL );

//#> func exec cron
bool callCronFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool jobScriptApply(TreeNode* node, TreeNode* tn, XFuncNode* fn, TreeNode* thisNode, StrVar* var, StrVar* rst );

//#> func exec worker
bool callWorkerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);

//#> func exec process
bool callProcessFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);

//#> func exec was
bool callWasFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
void pageMapUri(LPCC pageGroup, LPCC pageCode, bool map, TreeNode* root, TreeNode* sub=NULL );
void loadWebPage(PARR arrs, XFuncNode* fn, StrVar* rst, TreeNode* parentNode );

//#> func exec web request
class HttpConnection;
bool callWebFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
XFuncNode* setWebCallback(HttpConnection* w, LPCC url, TreeNode* tn, XFuncNode* fn, StrVar* callback, StrVar* rst );

//#> func exec socket
void regConnectNodeFuncs();

bool callConnectNodeFunc(XFunc* fc, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst );
bool callMessageServerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst);
bool callMessageClientFunc(StrVar* me, XFunc* fc, LPCC funcName, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst);
bool callUdpFunc(StrVar* me, XFunc* fc, LPCC funcName, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst);

//#> func exec file
void regFileFuncs();
bool callFileFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool callFileInfoFunc(XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst);
void getFileInfoListFunc(XFuncNode* fn, XFuncSrc* fsrc, LPCC path, TreeNode* root, TreeNode* tn, StrVar* rst);
int fileRename( LPCC src, LPCC newName);
bool getDateTimeVar(QDateTime& dt, StrVar* sv, StrVar* rst);
bool setFileTimeStamp(const QString &fname, const QDateTime &cdt, const QDateTime &ldt);
QByteArray GetHash(const QByteArray& bBlockData);
LPCC GetFileHash(QString szSrc,  TreeNode* node );

//#> func exec range
void regRangeFuncs();
bool execRangeFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst);

bool callInitFunc(TreeNode* node);
QIcon& iconPixmap(QIcon& icon, QPixmap* img, LPCC mode, LPCC stat);
#ifdef WIDGET_USE
//#> func tray
void makeChildMenus(XListArr* a, QMenu* menu, TreeNode* pageNode);
void makeNodeChildMenus(TreeNode* a, QMenu* menu, TreeNode* pageNode);
bool callTrayFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);

//#> func exec widget
void regWidgetFuncs();
void setActionValue(GAction* act, TreeNode* cur);
GAction* getAction(LPCC id, TreeNode* cf, bool overwrite=false);
bool callWidgetFunc(TreeNode* cf, XFunc* fc, QWidget* w, PARR arrs, XFuncNode* fn, StrVar* rst);
XFuncNode* setEventMapUserFunction(TreeNode* cf, XParseVar& pv, XFunc* fc, LPCC eventName, XFuncNode* fn, StrVar* rst, XFunc* fcParent=NULL);
void readSetting(LPCC type, LPCC code, LPCC key, LPCC def, StrVar* rst, QWidget* w=NULL );
void writeSetting(LPCC type, LPCC code, LPCC key, LPCC def, StrVar* rst, QWidget* w=NULL );
bool isWidgetCheck(LPCC ty, QWidget* w, StrVar* rst);
TreeNode* findPageNode(TreeNode* node);
bool parsePageVar(TreeNode* root, StrVar* var, int sp=0, int ep=0, TreeNode* base=NULL );
bool callPageFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GPage* page=NULL, XFunc* fc=NULL);
bool qtSetWindowFlags(QWidget* w, StrVar* sv, bool badd=false);
void qtSetWindowState(QWidget* w, LPCC state);

//#> func exec widget all
TreeNode* getWidgetConfig(QWidget* w, U16 stat=0);
void showHideWidgets(TreeNode* tn, bool show, XListArr* ids);
bool callLayoutFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst);
bool callWidgetAll(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callSplitterFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callMainFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callDivFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callTabFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc=NULL);

bool callInputFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GInput* widget, XFunc* fc);
bool callRadioFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callCheckFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callMPlayerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GMpWidget* w=NULL, XFunc* fc=NULL);
bool callVideoFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GVideo* w, XFunc* fc);

//#> func exec canvas
bool callCanvasFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GCanvas* canvas=NULL, XFunc* fc=NULL);
bool callCanvasItemFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst);
bool callTimelineFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc, MyTimeLine* timeline=NULL);

//#> func exec pdf
bool callPdfFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
TreeNode* printerSetup(QPrinter* printer, StrVar* sv);

//#> func exec excel
#ifdef EXCEL_USE
void excelCellReference(LPCC str, int& r, int& c);
LPCC excelRange(StrVar* sv, StrVar* rst=NULL);
bool excelCellRange(XFuncNode* fn, LPCC c, StrVar* rst, int incrc=-1, int incrr=-1, bool move=false, LPCC refr=NULL, LPCC refc=NULL );
bool callExcelFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
#endif

//#> func exec mq
bool callMqFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst,  XFunc* fc=NULL);

//#> func exec editor
void mergeFormatOnWordOrSelection(GTextEdit* widget, const QTextCharFormat &format);
QTextCursor::MoveOperation getTypeMoveOperation(LPCC ty);
int editorMovePos( QTextCursor* c, LPCC ty, TreeNode* tn);
bool callTextEditFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GTextEdit* edit=NULL, XFunc* fc=NULL);
bool callTextCursorFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst);
void textCharFormatInline(XParseVar& pv, QTextCharFormat& fmt);
void textCharFormatText( QTextCharFormat& fmt, QString& text, StrVar* rst);
void textCharFormatApply( QTextCharFormat& fmt, StrVar* sv);
void textCursorInsert(QTextDocument* doc, QTextCursor* cursor, LPCC text, StrVar* prop, StrVar* rst);
int findEditorCursor(QTextDocument* doc, QTextCursor& newCursor, LPCC val, U32 flag, bool bback, LPCC rep, StrVar* findsVar);
void editorInsertImage(QTextDocument* doc, QTextCursor& cursor, PARR arrs, StrVar* rst, bool skip=false);

//#> func exec widget tree
bool callTreeFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
bool callComboFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget=NULL, XFunc* fc=NULL);
void setModel(QAbstractItemView* view, TreeNode* cf, bool reset=false );
#endif

bool callModbusFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);
bool callFtpFunc(XFunc* fc, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst);
bool callZipFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc);

//#> func etc
#ifdef MAC_USE
LPCC machineHash(StrVar* rst);
#endif
LPCC getColorText(QColor& clr, StrVar* rst);
U32 getDirection(StrVar* p1, StrVar* p2, float base);
LPCC getDirectionValue(U32 flag, StrVar* rst);
void getBezier2D(XListArr* b, XListArr* p, int cpts);
bool recalcArrayPixcel(XListArr* a, XParseVar& pv, int wid, StrVar* rst, double rate=0);
bool getSystemEnv(StrVar* rst, LPCC type, LPCC  val=NULL, bool overwrite=true );
QPixmap* getStrVarPixmap(StrVar* sv, bool alpha=true, PU32 ref=0 );
bool loadPixmapImage(LPCC filenm, LPCC ext, bool bfile=true, StrVar* rst=NULL, StrVar* imgBuffer=NULL, bool thumb=false);
QDateTime getDateTime(LPCC date);
UL64 getTimet(LPCC ty, int n, QDateTime& dt);
QString dateTimeString(LPCC date, LPCC fmt);
LPCC getFileInode(LPCC filePath, StrVar* rst);
int parseXmlSub(TreeNode* root, StrVar* d, LPCC tag, int sp=0, int ep=0);
int parseXmlNode(TreeNode* root, StrVar* d, LPCC tag, int ntype, TreeNode* chkNode, int sp=0, int ep=0);
int parseXmlList(XParseVar& pv, TreeNode* root, TreeNode* chkNode, LPCC tag, int ntype=0, U32 flag=0);
double rounding( double value, int pos );
int HexPairValue(const char * code);
LPCC UTF2C( LPCC str, int len, StrVar* rst, int chk=0 );
char* C2UTF(char* cur, int size, StrVar* rst, int chk=0 );

void jsValueString(StrVar* sv, int sp, int ep, StrVar* rst);
LPCC getPairKey(PARR arrs, int n, StrVar* rst=NULL);
bool getPairValue(PARR arrs, int n, StrVar* rst);
bool findPairKey(PARR arrs, LPCC key, StrVar* rst);
int findPairIndex(PARR arrs, LPCC key=NULL );
XListArr* findPairKeyStart(PARR arrs, LPCC key=NULL);
bool getFullRectPos(double cw, double ch, double pw, double ph, StrVar* rst);
void getDivArr(PARR a, PARR arrs, StrVar* rst=NULL);
double getTimeSec();
void setQuitHandler();
#ifdef _WIN32
LPCC W2C(PCWCHAR str );
#endif
#endif
