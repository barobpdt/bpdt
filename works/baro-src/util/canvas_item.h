#ifndef CANVAS_ITEM_H
#define CANVAS_ITEM_H
#include "widget_util.h"

#define ITEMFLAG_SAVE 0X1
#define ITEMFLAG_DISABLE 0X2
#define ITEMFLAG_CANVAS 0X4
#define ITEMFLAG_CHECK 0X8
#define ITEMFLAG_HIDE 0X10
#define ITEMFLAG_IMGSET 0X20
#define ITEMFLAG_TIMELINE 0X40
#define ITEMFLAG_BASE 0X80
#define ITEMFLAG_FIXED 0X100
#define ITEMFLAG_SKIP 0X200
#define ITEMFLAG_TEMP 0X400

class GCanvas;
class CanvasItem;

class MyTimeLine : public QTimeLine {
    Q_OBJECT
public:
    MyTimeLine(WidgetConf* cf, QWidget* p=NULL);

public:
    WidgetConf* config() { return xcf; }
    void setCode(LPCC code) { xinfoNode.set("code", code); }
    LPCC getCode() { return xinfoNode.get("code"); }
    TreeNode* info() { return &xinfoNode; }
public:
    WidgetConf* xcf;
    GCanvas* xcanvasWidget;
    TreeNode xinfoNode;
};

class CanvasItemDraw {
public:
    CanvasItemDraw(CanvasItem* item, LPCC id, QRectF rect=QRectF());
    ~CanvasItemDraw();
public:
    CanvasItem* xcanvasItem;
    QImage* ximage;
    QPainter* xpainter;
    TreeNode xdrawObject;
    QRectF xrect;
public:
    LPCC getId() { return xdrawObject.get("id"); }
    void setId(LPCC id) { xdrawObject.set("id",id); }
    int width() { return ximage? ximage->width(): 0; }
    int height() { return ximage? ximage->height(): 0; }
    QImage* getImage() { return ximage; }
    void setRect(const QRectF& rc);
    const QRectF& getRect() { return xrect; }
    QRect getImageRect();
    void clear();
    QImage* createImage(int w, int h);
    bool begin();
    bool end();
    bool drawItem(QPainter* p, const QRectF& rcDraw);
    TreeNode* getDrawObject();
};


class CanvasItem {
public:
    CanvasItem(CanvasItem* parent, WidgetConf* tagNode, TreeNode* targetNode=NULL);
    ~CanvasItem();

public:
    CanvasItem* parent()        { return xparent; }
    void setTargetNode(TreeNode* node) { xtargetNode=node; }
    TreeNode* targetNode()  { return xtargetNode ? xtargetNode : static_cast<TreeNode*>(xtagNode); }
    WidgetConf* tagNode()   { return xtagNode; }
    LPCC get(LPCC code) { return xtagNode ? xtagNode->get(code): NULL; }
    StrVar* gv(LPCC code) { return xtagNode ? xtagNode->gv(code): NULL; }
    void set(LPCC code, LPCC val) { if( xtagNode && slen(code) ) xtagNode->set(code, val); }
    bool cmp(LPCC field, LPCC val) { return xtagNode? xtagNode->cmp(field, val): false; }
    int index()                 { return (int)xidx; }
    int childCount()			{ return xchilds.size(); }
    CanvasItem* child(int n)	{ return xchilds.getvalue(n); }
    U16 state()                 { return xstate; }
    void setState(U16 state);
    void setRect(double cx, double cy, double w, double h);
    bool contains(double x, double y) { return xrc.contains(x,y); }
    const QRectF& getRect() { return xrc; }
    const QRectF& getDrawRect() { return xdrawItem.getRect(); }
    CanvasItem* addChild(WidgetConf* wcf);
    CanvasItem* addChild(LPCC tag, LPCC id);
    CanvasItemDraw* addDraw(LPCC drawId, const QRectF& rect);
    CanvasItem* findTag(LPCC tag);
    CanvasItem* findId(LPCC id);
    CanvasItemDraw* findDraw(LPCC drawId=NULL, bool bContinue=false);
    bool removeDraw(LPCC drawId);
    int removeAll();
    void removeRange();
    bool remove(CanvasItem* item);
    void draw(QPainter* painter, const QRectF& rcDraw);
    bool update(const QRectF& rect);
    bool drawTimeLine(TreeNode* d, MyTimeLine* timeline, LPCC state=NULL);
    void invalidate(bool update=false);
    bool addPairs(LPCC key, StrVar* sv, LPCC ty=NULL);
    XListArr* getRangeGroups(LPCC group, bool newCheck=false, StrVar* rst=NULL);
    StrVar* getRectVar(LPCC code, StrVar* rst=NULL);
    StrVar* getRectVar(LPCC group, LPCC code, StrVar* rst=NULL);
    StrVar* setRectVar(LPCC group, LPCC code, QRectF rect);
    GCanvas* canvas();
public:
    QRectF xrc;
    U32 xflag;
    U16 xidx, xtype, xstate, xdepth;
    CanvasItem* xparent;
    TreeNode* xtargetNode;
    WidgetConf* xtagNode;
    XListArr xranges;
    XListArr xeventPairs;
    XListArr xselectedPairs;
    StrVar xrst;
    CanvasItemDraw xdrawItem;
    ListObject<CanvasItemDraw*>	xdrawItems;
    ListObject<CanvasItem*>	xchilds;
};

#endif // CANVAS_ITEM_H
