#ifndef WIDGET_UTIL_H
#define WIDGET_UTIL_H

#include "mywidget.h"
#include "db_util.h"
#include <QNetworkReply>

#define PROP_GETID 0x10
#define PROP_SOURCE 0x20
#define PROP_SOURCE_ALL 0x80

/*========================================================================
*    widget node
========================================================================*/
typedef WHash<U16> HashKey, *PHashKey;
typedef WHash<U32> HashFlag, *PHashFlag;
typedef WHash<StrVar> HashStr, *PHashStr;
typedef WHash<TreeNode> HashTreeNode, *PHashTreeNode;

#define WCF2N(field)		toInteger(wcf->gv((field)))
#define WCF2S(field)		KR(toString(wcf->gv((field))))
#define WCFS(field)		toString(wcf->gv((field)))
#define WCFSS(field, val)		wcf->isset((field)) ? toString(wcf->gv((field))): (val)

#define WCFNUM(field, def) wcf->isset((field)) ? toInteger(wcf->gv(field)):(def)
#define WCFSTR(field, def) wcf->isset((field)) ? toString(wcf->gv(field)):(def)
#define WCFIS(field)		isVarTrue(wcf->gv((field)))
#define WCFEQ(field,...) cf_eq(wcf,(field),#__VA_ARGS__)
#define IS_NOT_DIGIT(ch) (ch < 48 || ch > 57)

#define ifid(...) if( cf_eq(wcf,"id",#__VA_ARGS__) )
#define iftype(...) if( cf_eq(wcf,"type",#__VA_ARGS__) )
#define ifsender(...) if( cf_eq(wcf,"@sender",#__VA_ARGS__) )

#ifndef _WIN32
QString KR( QString str );
#endif

class XFuncNode;

DbUtil* sqliteDb(LPCC code, LPCC fileName=NULL, TreeNode* node=NULL);
DbUtil* sqliteOpen(DbUtil* db, LPCC fileName);
DbUtil* getConfDb();
DbUtil* getIconDb();
DbUtil* getFuncsDb();

TreeNode* getCf();
StrVar* cfVar(LPCC code, bool reuse=false, LPCC str=NULL);
XListArr* cfArray(LPCC code, bool reuse=false );
TreeNode* cfNode(LPCC code, bool reuse=false );

StrVar* confVar(LPCC confCode, StrVar* value=NULL, bool overwrite=false);
LPCC conf(LPCC confCode, LPCC def=NULL, bool overwrite=false);

int confLoad(LPCC fileName, LPCC lang=NULL, bool overwite=false);

int getAlignFlag(LPCC align);
int alert(LPCC message, LPCC title=NULL);
int confirm(LPCC message, LPCC info, LPCC title=NULL);
WidgetConf* getWidgetConf(TreeNode* node);
WidgetConf* parentWidgetConf(WidgetConf* wcf);


bool isHashKey(LPCC type);
PHashKey getHashKey(LPCC type);
U16 getHashKeyVal(LPCC type, LPCC name, U16 key=0);

bool isHashFlag(LPCC type);
PHashFlag getHashFlag(LPCC type);
U32 getHashFlagVal(LPCC type, LPCC name, U32 key=0);

TreeNode* getTreeNode(LPCC code, LPCC name, bool newNode=true );
StrVar* getStrVar(LPCC code, LPCC name, char ch='\0', U16 state=0, LPVOID pobj=NULL );
StrVar* setStrVar(LPCC code, LPCC name, StrVar* sv, bool overwrite=false);

/*========================================================================
*    etc util
========================================================================*/
bool printError(LPCC errorCode, LPCC defaultFmt, ... );
StrVar* setVarRect(StrVar* sv, const QRect& r );
StrVar* setVarRectF(StrVar* sv, const QRectF& r );
QRect setQRect( StrVar* sv);
QRectF getQRectF( StrVar* sv);

TreeNode* nodeJsonData(XParseVar& pv, TreeNode* root, StrVar*sv);
int parseProp(TreeNode* node, StrVar* var, int sp, int ep, U32 flags=0, StrVar* rst=NULL, XFuncNode* fnParent=NULL);
bool parsePage(WidgetConf* root, StrVar* var, int sp, int ep, WidgetConf* rootNode, int* idx=NULL);
bool parseXml(TreeNode* root, StrVar* var, int sp, int ep, XFuncNode* fnParent=NULL);
bool parseXmlUpdate(TreeNode *root, StrVar *var, int sp, int ep, StrVar* rst, XFuncNode* fnParent);

/*========================================================================
*    common util
========================================================================*/
bool cf_eq(TreeNode* cf, LPCC field, LPCC args);
LPCC getFullFileName(LPCC fileName);
bool keyEventCheck(int key, U32 modifyer, LPCC keyMap);
XListArr* convertArray(TreeNode* cf, LPCC field, LPCC sep=NULL);

time_t getTm();
int firstNonSpace(const QString& text);

void hsv2rgb(QColor& clr, double hh, double hs, double hv, int alpha=-1);
StrVar* GetColorValue(LPCC s, StrVar* rst );
QColor getQColor(StrVar* sv );
QColor getGlobalColor(StrVar* sv, LPCC def=NULL);
QColor darkColor(const QColor& clr, int scale=100);
QColor lightColor(const QColor& clr, int scale=100);
QColor difColor(const QColor& clr, int scale=100);
QColor midColor(const QColor& clr1, const QColor& clr2);
QColor mixColor(const QColor& clr1, const QColor& clr2);
QColor invertColor(const QColor& clr);

LPCC getImageFileName(LPCC fileName);
QPixmap* getIconPixmapType(LPCC type, LPCC id, bool alpha=true);
QPixmap* getPixmap(LPCC c, bool alpha=true);
QPixmap* getPixmapFile(LPCC fileName, LPCC ext=NULL, bool thumb=false);

const QIcon& getQIcon(LPCC code, LPCC mode=NULL, LPCC stat=NULL);
void setFontInfo(QFont* font);
TreeNode* fontInfoNode(QFont* font, TreeNode* node, LPCC fontInfo=NULL);
TreeNode* getFontInfo(QFont* font, LPCC fontInfo, QPainter* painter=NULL);

LPCC getUri(LPCC uri, StrVar* rst, int interval=500, StrVar* err=NULL);
void getUriError(QNetworkReply::NetworkError code, int status, StrVar* err);
LPCC getSaveFileName(LPCC fileName);
bool isFile(LPCC fileName, bool makeFolder=false);
bool isFolder(LPCC path);
bool makeFileFolder(LPCC fileName, bool all=false, StrVar* rst=NULL);
TreeNode* getFileInfoList(LPCC path, TreeNode* root, LPCC fields=NULL, LPCC sort=NULL, LPCC kind=NULL, LPCC filter=NULL);

int fileReadAll(LPCC path, StrVar* rst);
int fileSaveAll(LPCC path, StrVar* rst);

// dir 0:(ascending), 1:(descending)
void bubbleSort(XListArr* list, int dir=0, LPCC field=NULL, LPCC subField=NULL);
int stringNameSort(LPCC left, LPCC right);
int stringNameCmp(LPCC left, LPCC right);

// random
double randomNum(double fMin, double fMax);
QColor randomColor();
QPixmap* randomPixmap();

#ifdef WIDGETMAP_USE
bool initPage(WidgetMap* ui);

bool setEventFunc_initPage(WidgetMap* ui );
bool setEventFunc_chage(WidgetConf* wcf, PWIDGET_CHANGE fpChange );
bool setEventFunc_click(WidgetConf* wcf, PWIDGET_CLICK fpClick );

MyCanvas* myCanvas(WidgetConf* cf);
MyCanvas* initCanvas(WidgetConf* cf);

MyPushButton* myButton(WidgetConf* cf);
MyLabel* myLabel(WidgetConf* cf);
MyTabWidget* myTab(WidgetConf* cf);
MySplitter* mySplitter(WidgetConf* cf);
MyGroupBox* myGroup(WidgetConf* cf);
MyStackedWidget* myDiv(WidgetConf* cf);
MyToolButton* myToolButton(WidgetConf* cf);
MyRadioButton* myRadio(WidgetConf* cf);
MyCheckBox* myCheck(WidgetConf* cf);
MySpinBox* mySpin(WidgetConf* cf);
MyLineEdit* myInput(WidgetConf* cf);
MyPlainTextEdit* myTextArea(WidgetConf* cf);
MyDateEdit* myDate(WidgetConf* cf);
MyTimeEdit* myTime(WidgetConf* cf);
MyCalendarWidget* myCalendar(WidgetConf* cf);
MyToolBar* myToolbar(WidgetConf* cf);
MyToolBox* myToolbox(WidgetConf* cf);
MyProgressBar* myProgress(WidgetConf* cf);
MyListView* myList(WidgetConf* cf);
MyComboBox* myCombo(WidgetConf* cf);
MyTree* myTree(WidgetConf* cf);
MyTextEdit* myEditor(WidgetConf* cf);
MyAction* myAction(WidgetConf* cf);
MyMenu* myMenu(WidgetConf* cf);
MySystemTrayIcon* myTray(WidgetConf* cf);

MyPushButton* initButton(WidgetConf* cf);
MyLabel* initLabel(WidgetConf* cf);
MyTabWidget* initTab(WidgetConf* cf);
MySplitter* initSplitter(WidgetConf* cf);
MyGroupBox* initGroup(WidgetConf* cf);
MyStackedWidget* initDiv(WidgetConf* cf);
MyToolButton* initToolButton(WidgetConf* cf);
MyRadioButton* initRadio(WidgetConf* cf);
MyCheckBox* initCheck(WidgetConf* cf);
MySpinBox* initSpin(WidgetConf* cf);
MyLineEdit* initInput(WidgetConf* cf);
MyPlainTextEdit* initTextArea(WidgetConf* cf);
MyDateEdit* initDate(WidgetConf* cf);
MyTimeEdit* initTime(WidgetConf* cf);
MyCalendarWidget* initCalendar(WidgetConf* cf);
MyToolBar* initToolbar(WidgetConf* cf);
MyToolBox* initToolbox(WidgetConf* cf);
MyProgressBar* initProgress(WidgetConf* cf);
MyListView* initList(WidgetConf* cf);
MyComboBox* initCombo(WidgetConf* cf);
MyTree* initTree(WidgetConf* cf);
MyTextEdit* initEditor(WidgetConf* cf);
MyAction* initAction(WidgetConf* cf);
MyMenu* initMenu(WidgetConf* cf);
MySystemTrayIcon* initTray(WidgetConf* cf);

MyMPlayer* initPlayer(WidgetConf* wcf);
MyMPlayer* myPlayer(WidgetConf* wcf);

WebView* initWebView(WidgetConf* wcf);
WebView* myWebView(WidgetConf* wcf);
#endif

/*========================================================================
*    widget init
========================================================================*/
void initWidget(TreeNode* cf, QWidget* widget);


#endif // WIDGET_UTIL_H
