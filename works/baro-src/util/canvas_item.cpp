#include "canvas_item.h"
#ifdef FUNCNODE_USE
#include "../baroscript/func_util.h"
#endif

MyTimeLine::MyTimeLine(WidgetConf* cf, QWidget* parent) : QTimeLine(), xcf(cf) {
    xinfoNode.xstat=FNSTATE_TIMELINE;
    xinfoNode.GetVar("@c")->setVar('t',20,(LPVOID)this);
    xcanvasWidget=NULL; //qobject_cast<GCanvas*>(parent);
}

CanvasItemDraw::CanvasItemDraw(CanvasItem *item, LPCC id, QRectF rect) : xcanvasItem(item) {
    xpainter=NULL;
    ximage=NULL;
    setId(id);
    setRect(rect);
}

CanvasItemDraw::~CanvasItemDraw() {
    SAFE_DELETE(ximage);
    SAFE_DELETE(xpainter);
}
void CanvasItemDraw::clear() {
    if( begin() ) {
        xpainter->fillRect(xrect, Qt::NoBrush);
        end();
    }
}
void CanvasItemDraw::setRect(const QRectF& rc) {
    if( rc.isValid() ) {
        xrect=rc;
    }
}
QRect CanvasItemDraw::getImageRect() {
    if( ximage ) {
        return QRect(0,0,ximage->width(), ximage->height());
    }
    return QRect();
}
QImage* CanvasItemDraw::createImage(int w, int h) {
    if( ximage && w==width() && h==height() ) {
        // ximage->fill(Qt::transparent);
        if( xpainter==NULL ) {
            xpainter = new QPainter(ximage);
        }
        xdrawObject.GetVar("@noChange")->setVar('3',1);
    } else {
        end();
        SAFE_DELETE(xpainter);
        SAFE_DELETE(ximage);
        ximage=new QImage(w, h, QImage::Format_ARGB32);
        ximage->fill(Qt::transparent);
        xpainter = new QPainter(ximage);
        // qDebug("xcanvasItem invalidate =========== %d, %d, %d", w, h, xcanvasItem->state());
        if( xcanvasItem && xcanvasItem->state()==0 ) {
            xcanvasItem->invalidate();
        }
        xdrawObject.setLong("@updateTick", GetTickCount());
    }
    return ximage;
}

bool CanvasItemDraw::begin() {
    if( !xrect.isValid() ) {
        return false;
    }
    int w=(int)xrect.width(), h=(int)xrect.height();
    createImage(w,h);
    return ximage ? true: false;
}
bool CanvasItemDraw::end() {
    if( xpainter && xpainter->isActive() ) {
        return xpainter->end();
    }
    return false;
}

bool CanvasItemDraw::drawItem(QPainter* p, const QRectF& rcDraw) {
    if( !xrect.intersects(rcDraw) ) {
        return false;
    }
    if( ximage ) {
        // qDebug()<<"-- drawItem rect: "<< rcDraw;
        p->drawImage( xrect, *ximage);
        return true;
    } else {
        if( !xcanvasItem->tagNode()->isNodeFlag(FLAG_INIT) ) {
            qDebug("canvasItemDraw tag: %s", xcanvasItem->tagNode()->get("tag"));
        }
        p->fillRect( xrect, randomColor() );
    }
    return false;
}

TreeNode* CanvasItemDraw::getDrawObject() {
    if( begin() ) {
        StrVar* sv=NULL;
        TreeNode* node=&xdrawObject;
        node->xstat=FNSTATE_DRAW;        
        setVarRectF(node->GetVar("@rect"), xrect);
        sv=node->GetVar("@offsetUse");
        if( !SVCHK('9',0) ) {
            sv->setVar('3',1);
        }
        node->GetVar("@g")->setVar('g',0,(LPVOID)xpainter);
        node->GetVar("@img")->setVar('i',9,(LPVOID)ximage);
        return node;
    }
    return NULL;
}

CanvasItem::CanvasItem(CanvasItem *parent, WidgetConf *tagNode, TreeNode *targetNode) :
    xparent(parent), xtagNode(tagNode), xtargetNode(targetNode), xdrawItem(this,"root")
{
    xflag=0;
    xidx=xtype=0;
    xstate=xdepth=0;
    if( xtagNode ) {
        xtagNode->xstat=FNSTATE_CANVASITEM;
        xtagNode->GetVar("@c")->setVar('c',1,(LPVOID)this );
    }
}

CanvasItem::~CanvasItem() {
    removeAll();
    for(int n=0,sz=xdrawItems.size();n<sz; n++ ) {
        CanvasItemDraw* cid=xdrawItems.getvalue(n);
        SAFE_DELETE(cid);
    }
    removeRange();
    xranges.Close();
    xeventPairs.Close();
    xselectedPairs.Close();
    xchilds.Close();
    xdrawItems.Close();
}
void CanvasItem::removeRange() {
    XListArr* arr=NULL;
    StrVar* sv=xrst.reuse();
    for( int n=0,sz=xranges.size(); n<sz; n++ ) {
        if( getPairValue(&xranges,n,sv) ) {
            if( SVCHK('a',0) ) {
                arr=(XListArr*)SVO;
                if( arr->isNodeFlag(FLAG_NEW) ) {
                    SAFE_DELETE(arr);
                }
            }
        }
    }
}
int CanvasItem::removeAll() {
    int n=0;
    for( ; n<childCount(); n++) {
        CanvasItem* ci=child(n);
        ci->removeAll();
        SAFE_DELETE(ci);
    }
    xchilds.reuse();
    return n;
}
bool CanvasItem::remove(CanvasItem* item) {
    for( int n=0; n<childCount(); n++) {
        CanvasItem* ci=child(n);
        if( ci==item ) {
            ci->removeAll();
            xchilds.remove(n);
            SAFE_DELETE(ci);
            return true;
        }
        if( ci->remove(item) ) return true;
    }
    return false;
}
void CanvasItem::setRect(double cx, double cy, double w, double h) {
    if( cx==xrc.x() && cy==xrc.y() && w==xrc.width() && h==xrc.height() ) {
        return;
    }
    setState(0);
    xrc.setRect(cx, cy, w, h);
    if( xrc.isValid() ) {
        xdrawItem.setRect(xrc);        
    }
}

CanvasItem* CanvasItem::addChild(WidgetConf* wcf) {
    CanvasItem* cur=new CanvasItem(this, wcf);
    cur->xidx=xchilds.size();
    // _LOG("addChild : %s", wcf->log() );
    xchilds.add(cur);
    return cur;
}

CanvasItem* CanvasItem::addChild(LPCC tag, LPCC id) {
    WidgetConf* wcf=tagNode()->addWidgetConf();
    CanvasItem* cur=NULL;
    if( wcf ) {
        if( wcf->xwidget==NULL ) {
            wcf->xwidget=tagNode()->xwidget;
        }
        cur=new CanvasItem(this, wcf);
        cur->xidx=xchilds.size();
        cur->set("tag", tag);
        cur->set("id", id);
        _LOG("CanvasItem addChild : %s", wcf->get("tag") );
        xchilds.add(cur);
    }
    return cur;
}
CanvasItemDraw* CanvasItem::addDraw(LPCC drawId, const QRectF& rect) {
    if( slen(drawId)==0 || !rect.isValid() ) return NULL;
    CanvasItemDraw* itemDraw=findDraw(drawId);
    if( itemDraw==NULL ) {
        itemDraw=new CanvasItemDraw(this, drawId, rect);
        xdrawItems.add(itemDraw);
    } else {
        itemDraw->setRect(rect);
    }
    return itemDraw;
}

CanvasItem* CanvasItem::findId(LPCC id) {
    for( int n=0; n<childCount(); n++) {
        CanvasItem* cur=child(n);
        if( cur->cmp("id", id) ) {
            return cur;
        }
    }
    return NULL;
}
CanvasItem* CanvasItem::findTag(LPCC tag) {
    for( int n=0; n<childCount(); n++ ) {
        CanvasItem* cur=child(n);
        if( cur->cmp("tag", tag) ) {
            return cur;
        }
    }
    return NULL;
}
CanvasItemDraw* CanvasItem::findDraw(LPCC drawId, bool bContinue) {
    if( slen(drawId)==0 ) return &xdrawItem;
    for( int n=0,sz=xdrawItems.size(); n<sz; n++ ) {
        if( ccmp(drawId,xdrawItems.getvalue(n)->getId()) ) {
            return xdrawItems.getvalue(n);
        }
    }
    if( bContinue ) {
        for( int n=0; n<childCount(); n++ ) {
            CanvasItemDraw* itemDraw=child(n)->findDraw(drawId);
            if( itemDraw ) return itemDraw;
        }
    }
    return NULL;
}
bool CanvasItem::removeDraw(LPCC drawId) {
    if( slen(drawId) ) {
        for( int n=0,sz=xdrawItems.size(); n<sz; n++ ) {
            if( ccmp(drawId,xdrawItems.getvalue(n)->getId()) ) {
                CanvasItemDraw* cd=xdrawItems.getvalue(n);
                if( cd) {
                    xdrawItems.remove(n);
                    SAFE_DELETE(cd);
                    return true;
                }
            }
        }
    }
    return false;
}

void CanvasItem::setState(U16 state) {
    xstate=state;
    for( int n=0; n<childCount(); n++ ) {
        child(n)->setState(state);
    }
}
void CanvasItem::draw(QPainter* painter, const QRectF& rcDraw) {
    qDebug("CanvasItem::draw state:%d", xstate);
    if( xstate==0 ) {
        XFuncNode* fn=getEventFuncNode((TreeNode*)tagNode(), "onRange");
        if( fn ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('c',1,(LPVOID)this);
            setFuncNodeParams(fn,arrs);
            fn->call();
            setState(1);
        }
    }
    if( xstate==1 ) {
        update(rcDraw);
    }
    xdrawItem.drawItem(painter, rcDraw);
}
bool CanvasItem::drawTimeLine(TreeNode* d, MyTimeLine* timeline, LPCC state) {
    XFuncNode* fn=getEventFuncNode(tagNode(),"onDraw");
    if( xstate==1 ) {
        update(getRect());
    }
    if( fn && timeline ) {
        // ex) onDraw(d, stat, timeline)
        if( slen(state)==0 ) {
            state=timeline->getCode();
        }
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('n',0,(LPVOID)d);
        arrs->add()->set(state);
        arrs->add()->setVar('t',20,(LPVOID)timeline);
        fn->call();
        return true;
    }
    return false;
}

bool CanvasItem::update(const QRectF& rect) {
    XFuncNode* fn=getEventFuncNode(tagNode(),"onUpdate");
    if( fn ) {
        // ex) onUpdate(item )
        TreeNode* node=xdrawItem.getDrawObject();
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('c',1,(LPVOID)this);
        setFuncNodeParams(fn, arrs);
        fn->call();
        if( xstate==1 ) xstate=2;
        node->setLong("@updateTick", 0);
        return true;
    }
    return false;
}
void CanvasItem::invalidate(bool update) {
    XFuncNode* fn=getEventFuncNode((TreeNode*)tagNode(), "onRange");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        removeRange();
        arrs->add()->setVar('c',1,(LPVOID)this);
        setFuncNodeParams(fn,arrs);
        fn->call();
        setState(1);
    }
    if( update && canvas() ) {
        canvas()->update();
    }
}

bool CanvasItem::addPairs(LPCC key, StrVar* sv, LPCC ty) {
    if( slen(ty)==0 ) ty="event";
    PARR arr=ccmp(ty,"event")? &xeventPairs: &xselectedPairs;
    int idx=findPairIndex(arr,key);
    StrVar* var=NULL;
    if( idx!=-1 ) {
        var=arr->get(idx);
    } else {
        var=arr->add();
    }
    if( var ) {
        var->setVar('x',21).add(key).add('\f').add(sv);
        return true;
    }
    return false;
}
XListArr* CanvasItem::getRangeGroups(LPCC group, bool newCheck, StrVar* rst) {
    if( slen(group)==0 ) {
        group=tagNode()->get("@saveGroup");
        if( slen(group)==0 ) group="@root";
    }
    StrVar* sv=xrst.reuse();
    XListArr* a=NULL;
    findPairKey(&xranges,group,sv);
    if( SVCHK('a',0) ) {
        a=(XListArr*)SVO;
    } else if(newCheck) {
        a=new XListArr(true);
        sv=xranges.add();
        sv->setVar('x',21).add(group).add("\f");
        sv->add('\b').add('a').addU16(0).addObject(a);
    }
    if( rst && a ) rst->setVar('a',0,(LPVOID)a);
    return a;
}

StrVar* CanvasItem::getRectVar(LPCC code, StrVar* rst) {
    if( slen(code)==0 ) return NULL;
    if( rst==NULL ) rst=xrst.reuse();
    if( rst ) rst->reuse();
    if( ccmp(code,"@root") ) {
        setVarRectF(rst, getRect() );
        return rst;
    }
    if( IndexOf(code,',')>0 ) {
        return NULL;
    }
    int pos=LastIndexOf(code,'.');
    if( pos>0 ) {
        LPCC group=gBuf.add(code,pos);
        code+=pos+1;
        if( slen(group)==0 ) {
            tagNode()->set("@saveGroup", group);
        }
        return getRectVar(group, code, rst);
    }
    LPCC groupCur=tagNode()->get("@saveGroup");
    if( slen(groupCur)==0 ) {
        groupCur="@root";
    }
    XListArr* arrs=getRangeGroups(groupCur);
    if( !ccmp(code, groupCur) && findPairKey(arrs,code,rst) ) {
        return rst;
    }
    for( int n=0,sz=xranges.size(); n<sz; n++ ) {
        LPCC group=getPairKey(&xranges,n,rst);
        if( ccmp(group, groupCur) ) continue;
        if( findPairKey(getRangeGroups(group),code,rst) ) {
            return rst;
        }
    }
    return NULL;
}
StrVar* CanvasItem::getRectVar(LPCC group, LPCC code, StrVar* rst) {
    if( rst==NULL ) rst=xrst.reuse();
    XListArr* arrs=getRangeGroups(group);
    if( findPairKey(arrs,code,rst) ) {
        return rst;
    }
    return NULL;
}
StrVar* CanvasItem::setRectVar(LPCC group, LPCC code, QRectF rect) {
    StrVar* rst=NULL;
    char ch=0;
    if( slen(group)==0 ) {
        if( code[0]=='#' || code[0]=='&' ) {
            ch=code[0];
            code+=1;
        }
        int pos=IndexOf(code,'.');
        if( pos>0 ) {
            group=gBuf.add(code,pos);
            code+=pos+1;
        } else {
            group=tagNode()->get("@saveGroup");
            if( slen(group)==0 ) {
                group="@root";
            }
        }
    }
    XListArr* arrs=getRangeGroups(group, true);
    int idx=findPairIndex(arrs,code);
    if( idx!=-1 ) {
        rst=arrs->get(idx);
    } else {
        rst=arrs->add();
    }
    StrVar* sv=xrst.reuse();
    if( rect.isValid() ) {
        setVarRectF(sv, rect);
    } else {
        sv->setVar('9',0);
    }
    if( ch=='#' ) {
        addPairs(code, sv, "event");
    } else if( ch=='&' ) {
        addPairs(code, sv, "selected");
    }
    // qDebug("### setRectVar code:%s rect:%s (ch:%c, idx:%d)", code, toString(sv), ch, idx );
    rst->setVar('x',21).add(code).add('\f').add(sv);
    return rst;
}
GCanvas* CanvasItem::canvas() {
    WidgetConf* wcf=tagNode();
    if( wcf && wcf->xwidget ) {
        return qobject_cast<GCanvas*>(wcf->xwidget);
    }
    return NULL;
}
