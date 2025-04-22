#include "func_util.h"
#include "../util/canvas_item.h"

StrVar gRectVar;

LPCC getBoxInfo(StrVar* sv, int sp, int ep, PARR keys, StrVar* rst) {
    XParseVar pv(sv,sp,ep);
    char ch=0;
    rst->reuse();
    LPCC key=NULL;
    for(int n=0; n<2048 && pv.valid(); n++ ) {
        LPCC val=NULL;
        ch=pv.ch();
        if( ch==0 ) break;
        sp=pv.start;
        if( ch=='-' ) {
            ch=pv.incr().ch();
            if( ch==',' || ch=='(' || ch==0 ) {
                if( rst->length() ) rst->add(',');
                if( ch=='(' ) {
                    if( pv.match("(",")") ) {
                        val=pv.v();
                    } else break;
                    ch=pv.ch();
                } else {
                    val="*";
                }
                rst->add(val);
                if( keys ) keys->add()->set("space");
            }
            if( ch==0 || ch!=',') {
                break;
            }
            pv.incr();
            continue;
        }
        if( ch=='#' || ch=='&' ) {
            ch=pv.incr().ch();
        }
        ch=pv.moveNext().ch();
        while( ch=='-' ) {
            ch=pv.moveNext().ch();
        }
        if( ch=='%' || ch=='*' ) pv.incr().ch();
        if( n==0 && ch==0 ) {
            return pv.Trim(sp, pv.start);
        }
        if( ch==':' ) {
            key=pv.Trim(sp, pv.start);
        } else if( ch=='[' ) {
            if( pv.match("[","]") ) {
                val=FMT("%s|%s",key,pv.v());
            } else break;
        } else if( ch==0 || ch==',') {
            val=pv.Trim(sp, pv.start);
        } else {
            pv.setPos(sp);
            val=pv.findEnd(",", FIND_SKIP).v();
        }
        if( val ) {
            if( rst->length() ) {
                rst->add(",");
            }
            rst->add(val);
            if( key ) {
                keys->add()->add(key);

            } else {
                if( keys ) keys->add()->add(val);
            }
            rst->add(val);
            key=NULL;
        }
        ch=pv.ch();
        if( ch==',' ) {
            pv.incr();
        }
    }
    return NULL;
}

inline bool canvasWidgetMove(GCanvas* canvas, QWidget* widget, StrVar* sv, TreeNode* node, bool offset) {
    if( node==NULL ) return false;
    QRectF rc;
    if( sv==NULL ) {
        rc=getQRectF(node->gv("@geo"));
    } else {
        rc=getQRectF(sv);
    }
    if( rc.isValid() && widget ) {
        QPoint pt = canvas->mapToGlobal(QPoint(0,0));
        int cx=pt.x(), cy=pt.y();
        int rx=(int)rc.x(), ry=(int)rc.y();
        if( offset ) {
            cx=rx, cy=ry;
        } else {
            cx+=rx, cy+=ry;
        }
        widget->setGeometry(cx, cy, (int)rc.width(), (int)rc.height() );
        if( sv ) {
            setVarRectF(node->GetVar("@geo"), rc);
        }
        return true;
    }
    return false;
}

bool callCanvasFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GCanvas* canvas, XFunc* fc) {
    U32 fref=fc? fc->xfuncRef: 0;
    if( canvas==NULL ) {
        qDebug("#9#canvas func call error (name:%s)", fnm);
        return false;
    }
    if( fref==0 ) {
        fref=
            ccmp(fnm, "layout") ? 1 :
            ccmp(fnm, "rootItem") ? 2 :
            ccmp(fnm, "timeline") ? 3 :
            ccmp(fnm, "timelineStop") ? 4 :
            ccmp(fnm, "is") ? 173 :
            ccmp(fnm, "update") ? 6 :
            ccmp(fnm, "current") ? 7 :
            ccmp(fnm, "drawItem") ? 8 :
            ccmp(fnm, "draw") ? 9 :
            ccmp(fnm, "widgetAdd") ? 10 :
            ccmp(fnm, "widgetShow") ? 11 :
            ccmp(fnm, "widgetHide") ? 12 :
            ccmp(fnm, "config") ? 13 :
            ccmp(fnm, "addItem") ? 14 :
            0;
        if( fc ) fc->xfuncRef=fref;
    }
    switch(fref) {
    case 1: {   // layout(str)
        TreeNode* node=static_cast<TreeNode*>(canvas->widgetConf());
        if( node ) rst->setVar('n',0,(LPVOID)node );
        if( arrs ) {
            int sp=0,ep=0;
            StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            canvas->setLayout(sv, sp, ep);
        }
    } break;
    case 2: {   // rootItem
        CanvasItem* rootItem=canvas->canvasItems();
        rst->setVar('c',1,(LPVOID)rootItem);
    } break;
    case 3: {  // timeline
        MyTimeLine* timeline=NULL;
        if( arrs==NULL ) {
            rst->setVar('a',0,(LPVOID)&(canvas->xtimelines));
            return true;
        }
        LPCC code=AS(0);
        if( slen(code) ) {
            if(ccmp(code,"next") ) {
                timeline=canvas->nextTimeline(rst, isVarTrue(arrs->get(1)) );
            } else if( ccmp(code,"prev") ) {
                timeline=canvas->prevTimeline(rst, isVarTrue(arrs->get(1)) );
            } else if( ccmp(code,"current") ) {
                timeline=canvas->xcurrentTimeline;
            } else {
                timeline=canvas->setTimeline(code, rst);
                if( timeline && arrs->size()>1 ) {
                    int frameCount= isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): 20;
                    int duration= isNumberVar(arrs->get(2)) ? toInteger(arrs->get(2)): 1000;
                    LPCC mode=AS(3);
                    bool back=isVarTrue(arrs->get(4))? true: false;
                    canvas->startTimeLine(code, frameCount, duration, mode, back);
                }
            }
            if( timeline ) {
                rst->setVar('t',20,(LPVOID)timeline);
            }
        }
    } break;
    case 4: {   // timelineStop
        canvas->stopTimeLine();
    } break;
    case 173: {   // is
        if( arrs==NULL ) {
            return false;
        }
        LPCC ty=AS(0);
        StrVar* sv=arrs->get(1);
        if( ccmp(ty,"transBackground") ) {
            canvas->setAttribute(Qt::WA_TranslucentBackground, isVarTrue(sv));
            canvas->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        } else if( ccmp(ty,"touchMode") ) {
            if( sv ) {
                bool on=isVarTrue(sv) ? true: false;
                canvas->setTouchMode(on);
                tn->GetVar("@touchMode")->setVar('3', on? 1: 0);
            } else {
                sv=tn->gv("@touchMode");
                rst->setVar('3',isVarTrue(sv)?1:0 );
            }
        } else  {
            return false;
        }
    } break;
    case 6: {   // update
        if( arrs==NULL ) {
            // qDebug("xxx canvas update %s %s xxx", canvas->xpageItem? "page":"page not", canvas->xcf?"ok":"cf error");
            canvas->update();
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('3',1) ) {
            canvas->invalidate(true, true);
        } else if( SVCHK('i',5) ) {
            QRect rc=getQRectF(sv).toRect();
            /*
            if(tn->isBool("@updateCheck")) {
                qDebug("#0# canvas draw not finished...\n");
            }
            */
            if( rc.isValid() ) {
                tn->setBool("@updateCheck", true);
                canvas->update(rc );
            }
        }
    } break;
    case 7: {   // current
        LPCC ty=arrs? AS(0): "current";
        if( ccmp(ty,"current") ) {
            if( canvas->currentItem() ) {
                rst->setVar('c',1,(LPVOID)canvas->currentItem() );
            }
        } else if( ccmp(ty,"page") || ccmp(ty,"main")  ) {
            if( canvas->xpageItem ) rst->setVar('c',1,(LPVOID)canvas->xpageItem);
        } else if( ccmp(ty,"layer") ) {
            if( canvas->xdivItem ) rst->setVar('c',1,(LPVOID)canvas->xdivItem);
        } else if( ccmp(ty,"popup") ) {
            if( canvas->xpopupItem ) rst->setVar('c',1,(LPVOID)canvas->xpopupItem);
        } else if( ccmp(ty,"root") ) {
            if( canvas->xcanvasItems ) rst->setVar('c',1,(LPVOID)canvas->xcanvasItems);
        } else {
            if( canvas->xpageItem ) rst->setVar('c',1,(LPVOID)canvas->xpageItem);
        }
    } break;
    case 8: {   // drawItem
        if(arrs==NULL ) {
            if( canvas->xpageItem ) rst->setVar('c',1,(LPVOID)canvas->xpageItem);
            return true;
        }
        // findItem(id) or findItem(tag, id, type);
        CanvasItem* root=canvas->xcanvasItems;
        int cnt=arrs->size();
        LPCC tag=NULL, id=NULL, ty=NULL;
        if( cnt==1 ) {
            id=AS(0);
        } else {
            tag=AS(0), id=AS(1);
            if( cnt==3 ) {
                /* set find item (ty: page, popup, layer ) */
                ty=AS(2);
            }
        }
        if( root ) {
            bool find=false;
            for( int n=0,sz=root->childCount(); n<sz; n++ ) {
                CanvasItem* cur=root->child(n);
                if( slen(tag) ) {
                    if( cur->cmp("tag",tag) && cur->cmp("id",id) ) {
                        find=true;
                    }
                } else if( cur->cmp("id",id) ) {
                    find=true;
                }
                if( find ) {
                    if( slen(ty) ) {
                        if( ccmp(ty,"page") || ccmp(ty,"main") ) {
                            canvas->xpageItem=cur;
                        } else if( ccmp(ty,"popup") ) {
                            canvas->xpopupItem=cur;
                        } else if( ccmp(ty,"layer") ) {
                            canvas->xdivItem=cur;
                        }
                        canvas->invalidate();
                    }
                    rst->setVar('c',1,(LPVOID)cur );
                    break;
                }
            }
        }
    } break;
    case 9: {   // draw
        if( arrs==NULL ) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            sv=node->gv("@g");
            if( SVCHK('g',0) ) {
                QPainter* p=(QPainter*)SVO;
                QRectF rc=getQRectF(node->gv("@rect"));
                canvas->drawDefault(p,rc);
            }
        }

    } break;
    case 10: {   // widgetAdd
        TreeNode* cf=canvas->config();
        TreeNode* root=cf?cf->addNode("@widgets", NULL):NULL;
        TreeNode* target=NULL;
        int sp=0, ep=0;
        StrVar* sv=NULL;
        if( arrs==NULL ) {
            sv=tn->gv("@widgetAdd");
            if( sv ) {
                ep=sv->length();
            } else {
                return true;
            }
        } else {
            sv=arrs->get(1);
            if( SVCHK('n',0) ) {
                target=(TreeNode*)SVO;
                cf->GetVar("@targetControl")->setVar('n',0,(LPVOID)target);
            }
            sv=getStrVarPointer( arrs->get(0),&sp,&ep);
        }
        if( root && sp<ep ) {
            canvas->addWidgets(root, sv, sp, ep);
            sv=cf->gv("@targetControl");
            if(sv) sv->reuse();
            rst->setVar('n',0,(LPVOID)root);
        }
    } break;
    case 11: { // widgetShow
        TreeNode* cf=canvas->config();
        TreeNode* root=cf?cf->addNode("@widgets"):NULL;
        if( arrs==NULL ) {
            if( root ) {
                QWidget* widget=NULL;
                for(int n=0,sz=root->childCount(); n<sz; n++) {
                    TreeNode* cur=root->child(n);
                    widget=gwidget(cur);                    
                    if( widget ) {
                        LPCC mode=cur->get("mode");
                        if(ccmp(mode,"sub") || isPageNode(cur) ) continue;
                        canvasWidgetMove(canvas, widget, NULL, cur, false);
                    }
                }
            }
            return true;
        }
        LPCC wid=NULL;
        QWidget* widget=NULL;
        StrVar* sv=arrs->get(0);
        TreeNode* node=NULL;
        if( SVCHK('n',0) ) {
            node=(TreeNode*)SVO;
            widget=gwidget(node);
        } else if(root) {
            wid=toString(sv);
            for(int n=0,sz=root->childCount(); n<sz; n++) {
                TreeNode* cur=root->child(n);
                if( cur->cmp("id",wid)) {
                    node=cur;
                    widget=gwidget(cur);
                    break;
                }
            }
        }
        if( widget ) {
            bool offset=isVarTrue(arrs->get(2));
            canvasWidgetMove( canvas, widget, arrs->get(1), node, offset);
            widget->show();
        } else {
            qDebug("#9#canvas widget show error (id:%s)", wid);
        }
    } break;
    case 12: {  // widgetHide
        TreeNode* cf=canvas->config();
        TreeNode* root=cf?cf->addNode("@widgets"):NULL;
        if( arrs==NULL ) {
            if( root ) {
                QWidget* widget=NULL;
                for(int n=0,sz=root->childCount(); n<sz; n++) {
                    TreeNode* cur=root->child(n);
                    widget=gwidget(cur);
                    if( widget ) {
                        LPCC mode=cur->get("mode");
                        if(ccmp(mode,"sub") || isPageNode(cur) ) continue;
                        widget->hide();
                    }
                }
            }
            return true;
        }
        LPCC wid=NULL;
        QWidget* widget=NULL;
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            widget=gwidget((TreeNode*)SVO);
        } else {                        
            wid=toString(sv);
            if( root ) {
                for(int n=0,sz=root->childCount(); n<sz; n++) {
                    TreeNode* cur=root->child(n);
                    if( cur->cmp("id",wid)) {
                        widget=gwidget(cur);
                        break;
                    }
                }
            }
        }
        if( widget ) {
            widget->hide();
        } else {
            qDebug("#9#canvas widget hide error (id:%s)", wid);
        }
    } break;
    case 13: {  // config
        if( arrs==NULL ) {
            TreeNode* node=tn->addNode("#config");
            if(node) rst->setVar('n',0,(LPVOID)node);
            return true;
        }
        LPCC code=AS(0);
        if( slen(code) ) {
            TreeNode* map=tn->addNode(FMT("#%s",code));
            StrVar* sv=arrs->get(1);
            if( SVCHK('3',1) ) {
                map->removeAll(true, true);
            }
            rst->setVar('n',0,(LPVOID)map);
        }
    } break;
    case 14: {  // addItem
        if( arrs==NULL ) {
            return true;
        }
        CanvasItem* rootItem=canvas->canvasItems();
        LPCC tag=AS(0), id=AS(1);
        if( rootItem && slen(tag) && slen(id) ) {
            WidgetConf* wcf=canvas->widgetConf()->addWidgetConf();
            TreeNode* node=static_cast<TreeNode*>(wcf);
            wcf->set("tag",tag);
            wcf->set("id",id);
            wcf->xwidget=canvas;
            rootItem->addChild(wcf);
            if( node ) {
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    default: return false;
    }
    return true;
}

inline bool drawTargetItem(TreeNode* dsrc, TreeNode* ddest, StrVar* svRect ) {
    if( dsrc && ddest ) {
        // qDebug("drawTargetItem target=%s (src: %s %s,dest: %s %s)", toString(svRect), dsrc->get("id"), toString(dsrc->gv("@rect")), ddest->get("id"), toString(ddest->gv("@rect")) );
        QRectF rc=getQRectF(svRect);
        QRectF rcSrc=getQRectF(dsrc->gv("@rect")), rcDest=getQRectF(ddest->gv("@rect"));
        if( rcSrc.isValid() && rcDest.isValid() ) {
            double x=rcSrc.x(), y=rcSrc.y(), w=rcSrc.width(), h=rcSrc.height();
            double x0=0, y0=0;
            // qDebug("xxx %f, %f, %f, %f ", x,y,rcSrc.width(),h );
            if( rc.isValid() ) {
                x=rc.x(), y=rc.y(), w=rc.width(), h=rc.height(); ;
                x0=x-rcDest.x(), y0=y-rcDest.y();
            }
            if( ddest->isTrue("@offsetUse") ) {
                x-=rcDest.x(), y-=rcDest.y();
            }
            qDebug()<<"drawTargetItem src:"<<rcSrc<<" , dest:"<<rcDest<<" ,rc:"<<rc;
            StrVar* sv=ddest->gv("@g");
            if( SVCHK('g',0) ) {
                QPainter* p=(QPainter*)SVO;
                sv=dsrc->gv("@img");
                if( SVCHK('i',9) ) {
                    QImage* img=(QImage*)SVO;
                    p->drawImage(x,y,*img, x0,y0,w,h);
                }
            }
            return true;
        }
    }
    return false;
}
inline bool rectCodeState(LPCC code, StrVar* prev, StrVar* rst, bool disable ) {
    if( !checkFuncObject(prev,'x',21) ) return false;
    StrVar* sv=NULL;
    bool ok=false;
    if( disable ) {
        int pos=prev->find("\f",FUNC_HEADER_POS,64);
        if( pos>0 ) {
            sv=rst;
            sv->set(prev, pos+1, prev->length() );
        }
        if( SVCHK('i',5) ) {
            rst->setVar('x',21).add("disable\f").add(sv);
            prev->setVar('x',21).add(code).add('\f').add(rst);
            ok=true;
        }
    } else {
        int pos=prev->find("\f",FUNC_HEADER_POS,64);
        if( pos>0 ) {
            sv=rst;
            sv->set(prev, pos+1, prev->length() );
        }
        if( SVCHK('x',21) ) {
            pos=sv->find("\f",FUNC_HEADER_POS,64);
            if( pos>0 ) {
                StrVar* var=gRectVar.reuse();
                var->set(sv, pos+1, sv->length());
                if( checkFuncObject(var,'i',5) ) {
                    prev->setVar('x',21).add(code).add('\f').add(var);
                    ok=true;
                }
            }
        } else if( SVCHK('i',5) ) {
            ok=true;
        }

    }
    return ok;
}
bool callCanvasItemFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst) {
    if( !SVCHK('c',1) ) return true;
    CanvasItem* item=static_cast<CanvasItem*>(SVO);
    if( item==NULL ) return true;
    U32 fref=fc ? fc->xfuncRef: 0;
    if( fref==0 ) {
        if( slen(fnm)==0 ) fnm=fc->getFuncName();
        fref =
            ccmp(fnm,"current") ? 1 :
            ccmp(fnm,"canvas") ? 2 :
            ccmp(fnm,"tagNode") ? 3 :
            ccmp(fnm,"target") ? 4 :
            ccmp(fnm,"draw") ? 5 :
            ccmp(fnm,"rect") ? 6 :
            ccmp(fnm,"sub") ? 7 :
            ccmp(fnm,"vbox") ? 8 :
            ccmp(fnm,"hbox") ? 9 :
            ccmp(fnm,"grid") ? 10 :
            ccmp(fnm,"merge") ? 11 :
            ccmp(fnm,"prop") ? 12 :
            ccmp(fnm,"drawObject") ? 13 :
            ccmp(fnm,"img") ? 14 :
            ccmp(fnm,"is") ? 15 :
            ccmp(fnm,"update") ? 16 :
            ccmp(fnm,"paint") ? 17 :
            ccmp(fnm,"state") ? 18 :
            ccmp(fnm,"info") ? 19 :
            ccmp(fnm,"invalidate") ? 21 :
            ccmp(fnm,"sp") ? 22 :
            ccmp(fnm,"end") ? 23 :
            ccmp(fnm,"map") ? 24 :
            ccmp(fnm,"config") ? 25 :
            ccmp(fnm,"eventRect") ? 26 :
            0;
        if( fc ) fc->xfuncRef=fref;
    }
    switch( fref ) {
    case 1: { // current
        GCanvas* canvas=item->canvas();
        if( canvas ) {
            LPCC ty=arrs? AS(0): "page";
            if( ccmp(ty,"page") || ccmp(ty,"main")  ) {
                if( canvas->xpageItem ) rst->setVar('c',1,(LPVOID)canvas->xpageItem);
            } else if( ccmp(ty,"div") ) {
                if( canvas->xdivItem ) rst->setVar('c',1,(LPVOID)canvas->xdivItem);
            } else if( ccmp(ty,"popup") ) {
                if( canvas->xpopupItem ) rst->setVar('c',1,(LPVOID)canvas->xpopupItem);
            } else if( ccmp(ty,"root") ) {
                if( canvas->xcanvasItems ) rst->setVar('c',1,(LPVOID)canvas->xcanvasItems);
            }
        }
    } break;
    case 2: { // canvas
        GCanvas* canvas=item->canvas();
        if( canvas ) {
            rst->setVar('n',0,(LPVOID)canvas->config());
        }
    } break;
    case 3: { // tagNode
        TreeNode* tagNode=static_cast<TreeNode*>(item->tagNode());
        if( tagNode ) {
            if( arrs==NULL ) {
                rst->setVar('n',0,(LPVOID)tagNode);
            }
        }
    } break;
    case 4: { // target
        TreeNode* targetNode=static_cast<TreeNode*>(item->targetNode());
        if( targetNode ) {
            if( arrs==NULL ) {
                rst->setVar('n',0,(LPVOID)targetNode);
            }
        }
    } break;
    case 5: { // draw
        if( arrs==NULL ) {
            return false;
        }
        StrVar* sv=arrs->get(0);
        StrVar* svRect=NULL;
        TreeNode* dsrc=NULL;
        TreeNode* ddest=NULL;
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            if( node->xstat==FNSTATE_DRAW ) {
                sv=arrs->get(1);
                if( SVCHK('n',0) ) {
                    dsrc=(TreeNode*)SVO, ddest=node;
                    svRect=arrs->get(2);
                } else if(sv) {
                    LPCC code=toString(sv);
                    CanvasItemDraw * src= item->findDraw(code);
                    if( src ) {
                        dsrc=src->getDrawObject(), ddest=node;
                    }
                } else {
                    dsrc=node, ddest=item->xdrawItem.getDrawObject();
                }
                drawTargetItem(dsrc, ddest, svRect);
            }
        } else {
            int cnt=arrs->size();
            if( cnt==1 ) {
                int sp=0, ep=0;
                StrVar* var=getStrVarPointer(sv, &sp, &ep);
                XParseVar pv(var, sp, ep);
                ddest=item->xdrawItem.getDrawObject();
                while( pv.valid() ) {
                    LPCC code=pv.findEnd(",").v();
                    CanvasItemDraw * src=item->findDraw(code);
                    qDebug("item draw (code:%s %s)", code, src?"ok":"error");
                    if( src ) {
                        dsrc=src->getDrawObject();
                        drawTargetItem(dsrc, ddest, NULL);
                    }
                }
            } else {
                LPCC code=toString(sv);
                CanvasItemDraw * src= item->findDraw(code);
                if( src ) {
                    ddest=src->getDrawObject();
                    sv=arrs->get(1);
                    svRect=arrs->get(2);
                    if( SVCHK('n',0) ) {
                        dsrc=(TreeNode*)SVO;
                    } else if(sv) {
                        int sp=0, ep=0;
                        StrVar* var=getStrVarPointer(sv, &sp, &ep);
                        XParseVar pv(var, sp, ep);
                        while( pv.valid() ) {
                            code=pv.findEnd(",").v();
                            src=ccmp(code,"#")? &(item->xdrawItem):item->findDraw(code);
                            if( src ) {
                                dsrc=src->getDrawObject();
                                drawTargetItem(dsrc, ddest, svRect);
                            }
                        }
                    }
                }
            } // cnt>1
        }
        rst->reuse();
        if( dsrc) {
            rst->setVar('n',0,(LPVOID)dsrc);
        }
    } break;
    case 6: {   // rect
        if( arrs==NULL ) {
            setVarRectF(rst, item->getRect());
            return true;
        }
        int cnt=arrs->size();
        sv=arrs->get(0);
        if( cnt==1 && (SVCHK('i',5) || SVCHK('i',4)) ) {
            QRectF rc=getQRectF(sv);
            if( rc.isValid() ) {
                item->setRect(rc.x(), rc.y(), rc.width(), rc.height() );
            }
            return true;
        }
        LPCC code=AS(0);
        if( cnt==1 ) {
            item->getRectVar( code, rst);
        } else {
            StrVar* sv=arrs->get(1);
            if( SVCHK('i',5) ) {
                item->setRectVar(NULL, code, getQRectF(sv));
            }
        }
    } break;
    case 7: {   // sub
        if( arrs==NULL ) {
            item->getRangeGroups(NULL, false, rst);
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('i',2) || SVCHK('i',20) ) {
            int sz=sizeof(double);
            double x=SVCHK('i',2)?sv->getInt(4): sv->getDouble(4), y=SVCHK('i',2)?sv->getInt(8): sv->getDouble(4+sz);
            PARR a=&(item->xeventPairs);
            sz=a->size();
            sv=rst->reuse();
            for(int n=0; n<sz; n++ ) {
                if( getPairValue(a,n,sv) ) {
                    if( SVCHK('i',5)) {
                        if( getQRectF(sv).contains(x,y) ) {
                            rst->reuse()->add(a->get(n));
                            return true;
                        }
                    }
                }
            }
        } else {
            LPCC group=AS(0);
            PARR a=NULL;
            if( group[0]=='@' ) {
                group+=1;
                if( ccmp(group,"event") ) {
                    a=&(item->xeventPairs);
                } else if( ccmp(group,"selected") ) {
                    a=&(item->xselectedPairs);
                } else if( ccmp(group,"range") ) {
                    a=&(item->xranges);
                } else {
                    a=&(item->xeventPairs);
                }
            } else {
                a=item->getRangeGroups(group, false);
            }
            if( a ) rst->setVar('a',0,(LPVOID)a);
        }
    } break;
    case 8: // vbox
    case 9: {   // hbox
    } break;
    case 10: {  // grid
    } break;
    case 11: {  // merge
        if( arrs==NULL ) return true;
        QRectF rc;
        int idx=0;
        double sx=0, sy=0, ex=0, ey=0;
        for( int n=0,sz=arrs->size(); n<sz; n++ ) {
            sv=arrs->get(n);
            if( !SVCHK('i',5) ) {
                LPCC nm=toString(sv);
                sv=item->getRectVar(nm, rst);
            }
            if( SVCHK('i',5) ) {
                rc=getQRectF(sv);
                if( idx==0 ) {
                    sx=rc.x(), sy=rc.y(), ex=rc.right(), ey=rc.bottom();
                } else {
                    if( rc.x()<sx ) sx=rc.x();
                    if( rc.y()<sy ) sx=rc.y();
                    if( ex<rc.right() ) ex=rc.right();
                    if( ey<rc.bottom() ) ey=rc.bottom();
                }
                idx++;
            }
        }
        rc=QRectF(sx, sy, ex-sx, ey-sy);
        if( rc.isValid() ) {
            setVarRectF(rst, rc );
        } else {
            rst->reuse();
        }
    } break;
    case 12: {  // prop
        if( arrs==NULL ) return false;
        LPCC code=AS(0);
        int cnt=arrs->size();
        qDebug("@@ prop code:%s %s", code, item->tagNode()->log() );
        if( cnt==1 ) {
            StrVar* sv=item->tagNode()->gv(code);
            if( sv ) rst->add(sv);
        } else {
            int sp=0,ep=0;
            sv=getStrVarPointer(arrs->get(1),&sp,&ep);
            if( sp<ep) item->tagNode()->GetVar(code)->set(sv, sp,ep);
        }
    } break;
    case 13: {  // drawObject
        if( arrs==NULL ) {
            TreeNode* d=item->xdrawItem.getDrawObject();
            if( d ) rst->setVar('n',0,(LPVOID)d);
            return true;
        }
        LPCC code=AS(0);
        CanvasItemDraw* cd=item->findDraw(code);
        if( cd ) {
            QRectF rc;
            sv=arrs->get(1);
            if( SVCHK('i',5) ) {
                rc=getQRectF(sv);
            } else if( sv ) {
                LPCC name=toString(sv);
                sv=item->getRectVar(name, rst);
                if( SVCHK('i',5) ) {
                    rc=getQRectF(sv);
                }
            }
            if( rc.isValid() ) {
                cd->setRect(rc);
            }
        } else {
            sv=arrs->get(1);
            if( SVCHK('i',5) ) {
                qDebug("xxx drawObject rect ok (code:%s) xxx", code );
            } else if(sv) {
                LPCC name=toString(sv);
                sv=item->getRectVar(name, rst);
            } else {
                sv=item->getRectVar(code, rst);
            }
            if( SVCHK('i',5) ) {
                QRectF rc=getQRectF(sv);
                cd=item->addDraw(code, rc);
            }
        }
        rst->reuse();
        if( cd ) {
            TreeNode* node=cd->getDrawObject();
            rst->setVar('n',0,(LPVOID)node);
            sv=arrs->get(2);
            if( SVCHK('3',1) ) {
                node->GetVar("@offsetUse")->setVar('9',0);
            }
        } else {
            qDebug("#9#drawObject rect not define (%s)",code);
        }
    } break;
    case 14: {  // img
        if( arrs==NULL ) {
            QImage* img=item->xdrawItem.getImage();
            if( img ) {
                rst->setVar('i',9,(LPVOID)img);
            }
            return true;
        }
        LPCC code=AS(0);
        CanvasItemDraw* cd=item->findDraw(code);
        if( cd ) {
            QImage* img=cd->getImage();
            if( img ) {
                rst->setVar('i',9,(LPVOID)img);
            }
        }
    } break;
    case 15: {  // is
        if( arrs==NULL ) {
            rst->setVar('3', item->state()==0? 0: 1);
            return true;
        }
        LPCC code=AS(0);
        if( ccmp(code,"first") ) {
            rst->setVar('3', item->state()==0? 1: 0);
        } else if( ccmp(code,"update") ) {
            rst->setVar('3', item->state()==1? 1: 0);
        }
    } break;
    case 16: {  // update
        if( arrs==NULL ) {
            TreeNode* d=item->xdrawItem.getDrawObject();
            QRectF rc=item->getRect();
            item->update(rc);
            rst->setVar('n',0,(LPVOID)d);
            return true;
        }
    } break;
    case 17: {  // paint
        if( arrs==NULL ) {
            return false;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* d=(TreeNode*)SVO;
            sv=d->gv("@g");
            if( SVCHK('g',0) ) {
                QPainter* p=(QPainter*)SVO;
                QImage* img=item->xdrawItem.getImage();
                if( p && img ) {
                    QRectF rc;
                    int idx=1;
                    sv=arrs->get(1);
                    if( SVCHK('i',5) ) {
                        rc=getQRectF(sv);
                        idx=2;
                    }
                    if( !rc.isValid() ) {
                        rc=getQRectF(d->gv("@rect"));
                    }
                    double x=rc.x(), y=rc.y(), w=rc.width(), h=rc.height();
                    double x0=x, y0=y;
                    sv=arrs->get(idx++);
                    if( isNumberVar(sv) ) {
                        x=toDouble(sv), y=toDouble(arrs->get(idx));
                    }
                    p->drawImage(x,y, *img, x0,y0,w,h );
                }
            }
        }
    } break;
    case 18: {  // state
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt((int)item->state());
        } else {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                int state=toInteger(sv);
                item->setState((U16)state);
            }
        }
    } break;
    case 19: {  // info        
        if( arrs==NULL ) {            
            rst->set("ranges: [");
            for( int n=0,sz=item->xranges.size(); n<sz; n++ ) {
                LPCC group=getPairKey(&(item->xranges),n );
                if( slen(group) ) {
                    if( n>0 ) rst->add(",");
                    rst->add(group);
                }
            }
            rst->add("]\n");
            rst->add("event: [");
            for( int n=0,sz=item->xeventPairs.size(); n<sz; n++ ) {
                LPCC code=getPairKey(&(item->xeventPairs),n );
                if( slen(code) ) {
                    if( n>0 ) rst->add(",");
                    rst->add(code);
                }
            }
            rst->add("]\n");
            rst->add("selected: [");
            for( int n=0,sz=item->xselectedPairs.size(); n<sz; n++ ) {
                LPCC code=getPairKey(&(item->xselectedPairs),n );
                if( slen(code) ) {
                    if( n>0 ) rst->add(",");
                    rst->add(code);
                }
            }
            rst->add("]\n");
            rst->add("drawObject: [");
            for( int n=0,sz=item->xdrawItems.size(); n<sz; n++ ) {
                CanvasItemDraw* cd=item->xdrawItems.getvalue(n);
                LPCC code=cd->getId();
                if( slen(code) ) {
                    if( n>0 ) rst->add(",");
                    rst->add(code);
                }
            }
            rst->add("]\n");
            rst->add(FMT("state: %d\n", item->state()) );
            LPCC group=item->tagNode()->get("@saveGroup");
            if( slen(group) ) {
                rst->add("SavePoint: ").add(group);
            }
            return true;
        }
        WidgetConf* wcf=item->tagNode();
        LPCC code=AS(0);
        if( wcf && slen(code) ) {
            if( code[0]=='@' ) {
                code+=1;
                if( ccmp(code,"event") ) {
                    rst->setVar('a',0,(LPVOID)&(item->xeventPairs));
                } else if( ccmp(code,"selected") ) {
                    rst->setVar('a',0,(LPVOID)&(item->xselectedPairs));
                } else if( ccmp(code,"range") ) {
                    rst->setVar('a',0,(LPVOID)&(item->xranges));
                }
            } else {
                TreeNode* node=wcf->addNode(FMT("#%s",code), isVarTrue(arrs->get(1)) );
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 20: {  // over
        if( arrs==NULL ) {
            return true;
        }
    } break;
    case 21: {  // invalidate
        item->invalidate(true);
    } break;
    case 22: {  // sp
        if( arrs==NULL ) {
            item->tagNode()->set("@saveGroup", "@root");
            setVarRectF(rst, item->getRect() );
        } else {
            LPCC code=AS(0);
            item->tagNode()->set("@saveGroup", code);
            item->getRectVar(code, rst);
        }
    } break;
    case 23: {  // end
        if( arrs==NULL ) {
            rst->setVar('3', item->xdrawItem.end()?1:0);
            return true;
        }
        LPCC code=AS(0);
        CanvasItemDraw* cd=item->findDraw(code);
        if( cd ) {
            rst->setVar('3', cd->end()?1: 0);
        }
    } break;
    case 24: {  // map
        TreeNode* map=item->tagNode()->addNode("map");
        if( arrs==NULL ) {
            rst->setVar('n',0,(LPVOID)map);
            return true;
        }
        LPCC code=AS(0);
        sv=arrs->get(1);
        if( sv ) {
            int sp=0, ep=0;
            StrVar* var=getStrVarPointer(sv, &sp, &ep);
            map->GetVar(code)->set(var, sp, ep);
            rst->setVar('n',0,(LPVOID)map);
        } else {
            sv=map->GetVar(code);
            rst->reuse()->add(sv);
        }
    } break;
    case 25: {  // config
        GCanvas* canvas=item->canvas();
        TreeNode* cf=canvas?canvas->config(): NULL;
        if( cf==NULL ) {
            qDebug("#9#drawItem canvas config not define");
            return true;
        }
        if( arrs==NULL ) {
            TreeNode* node=cf->addNode("#config");
            if(node) rst->setVar('n',0,(LPVOID)node);
            return true;
        }
        LPCC code=AS(0);
        if( slen(code) ) {
            TreeNode* map=cf->addNode(FMT("#%s",code));
            StrVar* sv=arrs->get(1);
            if( SVCHK('3',1) ) {
                map->removeAll(true, true);
            }
            rst->setVar('n',0,(LPVOID)map);
        }

    } break;
    case 26: {   // eventRect
        XListArr* eventArr=&(item->xeventPairs);
        if( arrs==NULL ) {
            rst->setVar('a',0,(LPVOID)eventArr);
            return true;
        }
        int cnt=arrs->size();
        LPCC code=AS(0);
        if( slen(code) ) {
            if( code[0]=='#' ) code+=1;
            if( cnt==1 ) {
                findPairKey(eventArr, code, rst);
            } else {
                bool ok=false;
                int idx=findPairIndex(eventArr, code);
                sv=arrs->get(1);
                if( SVCHK('3',1) ) {
                    if( idx!=-1 ) {
                        ok=rectCodeState(code, eventArr->get(idx), rst, false);
                    }
                    rst->setVar('3', ok?1:0);
                } else if( SVCHK('3',0) ) {
                    if( idx!=-1 ) {
                        ok=rectCodeState(code, eventArr->get(idx), rst, true);
                    }
                    rst->setVar('3', ok?1:0);
                } else if( SVCHK('9',0) ) {
                    if( idx!=1 ) {
                        eventArr->remove(idx);
                        ok=true;
                    }
                    rst->setVar('3', ok?1:0);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    if( SVCHK('i',4) ) {
                        QRectF rc=getQRectF(sv);
                        if( rc.isValid() ) setVarRectF(sv, rc);
                    }
                    if( SVCHK('i',5) ) {
                        StrVar* var= idx!=-1 ? eventArr->get(idx): eventArr->add();
                        var->setVar('x',21).add(code).add('\f').add(sv);
                        ok=true;
                    }
                    rst->setVar('3', ok?1:0);
                } else {
                    LPCC ty=toString(sv);
                    int applyCnt=0;
                    if( ccmp(ty,"delete") ) {
                        if( idx!=1 ) {
                            eventArr->remove(idx);
                            applyCnt=1;
                        } else {
                            XListArr* arr=NULL;
                            for(int n=0,sz=eventArr->size(); n<sz; n++) {
                                sv=eventArr->get(n);
                                if( SVCHK('x',21) && StartWith(sv->get(FUNC_HEADER_POS),code) ) {
                                    if( arr==NULL) arr=getListArrTemp();
                                    arr->add()->setVar('0',0).addInt(n);
                                }
                            }
                            if( arr && arr->size()) {
                                for( int n=0,sz=arr->size();n<sz;n++) {
                                    eventArr->remove(toInteger(arr->get(n)) );
                                    applyCnt++;
                                }
                            }
                        }
                    } else if( ccmp(ty,"disable") ) {
                        if( idx!=1 ) {
                            ok=rectCodeState(code, eventArr->get(idx), rst, true);
                            if( ok ) applyCnt++;
                        } else {
                            for(int n=0,sz=eventArr->size(); n<sz; n++) {
                                sv=eventArr->get(n);
                                if( SVCHK('x',21) && StartWith(sv->get(FUNC_HEADER_POS),code) ) {
                                    int pos=sv->find("\f",FUNC_HEADER_POS,64);
                                    // qDebug("getPairKey (%d, pos:%d)", n, pos);
                                    if( pos>FUNC_HEADER_POS ) {
                                        pos-=FUNC_HEADER_POS;
                                        code=rst->set(sv->get(FUNC_HEADER_POS),pos).get();
                                        ok=rectCodeState(code, sv, rst, true);
                                        if( ok ) applyCnt++;
                                    }
                                }
                            }
                        }
                    } else if( ccmp(ty,"enable") ) {
                        if( idx!=1 ) {
                            ok=rectCodeState(code, eventArr->get(idx), rst, false);
                            if( ok ) applyCnt++;
                        } else {
                            for(int n=0,sz=eventArr->size(); n<sz; n++) {
                                sv=eventArr->get(n);
                                if( SVCHK('x',21) && StartWith(sv->get(FUNC_HEADER_POS),code) ) {
                                    int pos=sv->find("\f",FUNC_HEADER_POS,64);
                                    // qDebug("getPairKey (%d, pos:%d)", n, pos);
                                    if( pos>FUNC_HEADER_POS ) {
                                        pos-=FUNC_HEADER_POS;
                                        code=rst->set(sv->get(FUNC_HEADER_POS),pos).get();
                                        ok=rectCodeState(code, sv, rst, false);
                                        if( ok ) applyCnt++;
                                    }
                                }
                            }
                        }
                    }
                    rst->setVar('0',0).addInt(applyCnt);
                }
            }
        }
    } break;
    default: return false;
    }
    return true;
}

bool callTimelineFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc, MyTimeLine* timeline) {
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref =
          ccmp(fnm,"start") ? 1:
          ccmp(fnm,"stop") ? 2:
          ccmp(fnm,"frame") ? 3:
          ccmp(fnm,"canvas") ? 4:
          ccmp(fnm,"canvasItem") ? 5:
          ccmp(fnm,"config") ? 6:
          ccmp(fnm,"loopCount") ? 7:
          ccmp(fnm,"direction") ? 8:
          ccmp(fnm,"duration") ? 9:
          ccmp(fnm,"updateInterval") ? 10:
          ccmp(fnm,"currentTime") ? 11:
          ccmp(fnm,"currentValue") ? 12:
          ccmp(fnm,"frameForTime") ? 13:
          ccmp(fnm,"code") ? 14:
          ccmp(fnm,"state") ? 15:
          ccmp(fnm,"mode") ? 16:
          ccmp(fnm,"running") ? 17:
          0;
        if( fc ) fc->xfuncRef = ref;
    }

    if( ref==0 )
        return false;
    StrVar* sv=NULL;
    if( timeline==NULL ) {
        sv=tn?tn->gv("@c"):NULL;
        if( SVCHK('t',20) ) {
            timeline=(MyTimeLine*)SVO;
        } else {
            return false;
        }
    }
    switch( ref ) {
    case 1: { // start
        if( arrs ) {
            int idx=0;
            if( timeline->state()==QTimeLine::Running ) {
                timeline->stop();
            }
            sv=arrs->get(0);
            if(SVCHK('i',5) || SVCHK('i',4)) {
               tn->GetVar("@updateRect")->reuse()->add(sv);
               idx=1;
            }
            if( isNumberVar(arrs->get(idx)) ) {
                int frameCount= toInteger(arrs->get(idx++));
                timeline->setFrameRange(0,frameCount);
            }
            if( isNumberVar(arrs->get(idx)) ) {
                int duration= toInteger(arrs->get(idx++));
                timeline->setDuration(duration );
            }
            LPCC mode=AS(idx++);
            if(slen(mode)) {
                if( ccmp(mode,"in") ) {
                    timeline->setCurveShape(QTimeLine::EaseInCurve);
                } else if( ccmp(mode,"out") ) {
                    timeline->setCurveShape(QTimeLine::EaseOutCurve);
                } else if( ccmp(mode,"inout") ) {
                    timeline->setCurveShape(QTimeLine::EaseInOutCurve);
                } else if( ccmp(mode,"linear") ) {
                    timeline->setCurveShape(QTimeLine::LinearCurve);
                } else if( ccmp(mode,"sine") ) {
                    timeline->setCurveShape(QTimeLine::SineCurve);
                } else if( ccmp(mode,"cosine") ) {
                    timeline->setCurveShape(QTimeLine::CosineCurve);
                } else {
                    timeline->setCurveShape(QTimeLine::LinearCurve);
                }
            }
            sv=arrs->get(idx++);
            if( SVCHK('3',0) ) {
                timeline->setDirection(QTimeLine::Backward);
            } else if(SVCHK('3',1)) {
                timeline->setDirection(QTimeLine::Forward);
            }
        }
        if( timeline->state()!=QTimeLine::Running ) {
            timeline->start();
        }
    } break;
    case 2: { // stop
        if( timeline->state()==QTimeLine::Running ) {
            timeline->stop();
        }
    } break;
    case 3: { // frame
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->currentFrame() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int sp=toInteger(sv);
            sv-arrs->get(1);
            if( isNumberVar(sv) ) {
                timeline->setFrameRange(sp, toInteger(sv));
            } else {
                timeline->setStartFrame(sp);
            }
            return true;
        }
        LPCC code=AS(0);
        if( ccmp(code,"start") ) {
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                timeline->setStartFrame(toInteger(sv));
            } else {
                rst->setVar('0',0).addInt(timeline->startFrame());
            }
        } else if( ccmp(code,"end") ) {
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                timeline->setEndFrame(toInteger(sv));
            } else {
                rst->setVar('0',0).addInt(timeline->endFrame());
            }
        } else {
            rst->setVar('0',0).addInt(timeline->currentFrame() );
        }
    } break;
    case 4: { // canvas
        GCanvas* canvas=timeline->xcanvasWidget;
        if( canvas ) {
            rst->setVar('n',0,(LPVOID)canvas->config() );
        }
    } break;
    case 5: { // canvasItem
        GCanvas* canvas=timeline->xcanvasWidget;
        if( canvas ) {
            rst->setVar('n',0,(LPVOID)canvas->config() );
        }
    } break;
    case 6: { // config
        if( arrs==NULL ) {
            if( timeline->info() ) {
                rst->setVar('n',0,(LPVOID)timeline->info() );
            }
            return true;
        }
        TreeNode* info=timeline->info();
        if( info ) {
            LPCC code=AS(0);
            sv=arrs->get(1);
            if( sv ) {
                int sp=0, ep=0;
                StrVar* var=getStrVarPointer(sv, &sp, &ep);
                info->GetVar(code)->set(var, sp, ep);
            } else {
                sv=info->gv(code);
                rst->add(sv);
            }
        }
    } break;
    case 7: { // loopCount
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->loopCount() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            timeline->setLoopCount(toInteger(sv));
        }
    } break;
    case 8: { // direction
        if( arrs==NULL ) {
            rst->set(timeline->direction()==QTimeLine::Forward?"forward":"backward" );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            timeline->setLoopCount(toInteger(sv));
        }
    } break;
    case 9: { // duration
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->duration() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            timeline->setDuration(toInteger(sv));
        }
    } break;
    case 10: { // updateInterval
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->updateInterval() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            timeline->setUpdateInterval(toInteger(sv));
        }
    } break;
    case 11: { // currentTime
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->currentTime() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            timeline->setCurrentTime(toInteger(sv));
        }
    } break;
    case 12: { // currentValue
        if( arrs==NULL ) {
            rst->setVar('4',0).addDouble(timeline->currentValue() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            rst->setVar('4',0).addDouble( timeline->valueForTime(toDouble(sv)) );
        }
    } break;
    case 13: { // frameForTime
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(timeline->currentTime() );
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            rst->setVar('0',0).addInt(timeline->frameForTime(toInteger(sv)) );
        }
    } break;
    case 14: { // code
        if( arrs==NULL ) {
            rst->set(timeline->getCode() );
            return true;
        }
        LPCC code=AS(0);
        if( slen(code) ) timeline->setCode(code);
    } break;
    case 15: { // state
        if( timeline->state()==QTimeLine::Running ) {
            rst->set("running");
        } else if( timeline->state()==QTimeLine::Paused ) {
            rst->set("paused");
        }
    } break;
    case 17: { // running
        rst->setVar('3', timeline->state()==QTimeLine::Running? 1:0);
    } break;
    case 16: { // mode
        if( arrs==NULL ) {
            rst->set(
                timeline->curveShape()==QTimeLine::EaseInCurve ? "in":
                timeline->curveShape()==QTimeLine::EaseOutCurve ? "out":
                timeline->curveShape()==QTimeLine::EaseInOutCurve ? "inout":
                timeline->curveShape()==QTimeLine::LinearCurve ? "linear":
                timeline->curveShape()==QTimeLine::SineCurve ? "sine":
                timeline->curveShape()==QTimeLine::CosineCurve ? "cosine": "");
            return true;
        }
        LPCC mode=AS(0);
        if( ccmp(mode,"in") ) {
            timeline->setCurveShape(QTimeLine::EaseInCurve);
        } else if( ccmp(mode,"out") ) {
            timeline->setCurveShape(QTimeLine::EaseOutCurve);
        } else if( ccmp(mode,"inout") ) {
            timeline->setCurveShape(QTimeLine::EaseInOutCurve);
        } else if( ccmp(mode,"linear") ) {
            timeline->setCurveShape(QTimeLine::LinearCurve);
        } else if( ccmp(mode,"sine") ) {
            timeline->setCurveShape(QTimeLine::SineCurve);
        } else if( ccmp(mode,"cosine") ) {
            timeline->setCurveShape(QTimeLine::CosineCurve);
        } else {
            timeline->setCurveShape(QTimeLine::LinearCurve);
        }
    } break;
    default: return false;
    }
    return true;
}
