#include "widgets.h"
#include "widget_player.h"
#include "func_util.h"
#include "tree_model.h"
#include "../util/widget_util.h"
#include "../util/user_event.h"
#include <QApplication>
#include <QTreeWidget>
#include <QScrollBar>
#include <QScrollArea>
#include <QLineEdit>

inline bool isLayoutTag(LPCC tag ) { return ccmp(tag,"layout") || ccmp(tag,"hbox") || ccmp(tag,"vbox") ? true: false; }
inline TreeNode* findLayoutNode(TreeNode* node ) {
    if( isLayoutNode(node) ) return node;
    while( node ) {
        node=node->parent();
        if( isLayoutNode(node) ) return node;
    }
    return NULL;
}
inline TreeNode* findParentNode(TreeNode* node) {
    TreeNode* p=node->parent();
    if( p && p->xstat==LTYPE_ROW ) p = p->parent();
    return p;
}
inline QWidget* getNodeWidget(TreeNode* cf) {
    StrVar* sv=cf->gv("@scroll");
    return SVCHK('w',20) ? (QWidget*)SVO : gwidget(cf);
}


bool isLayoutNode(TreeNode* node) {
    U16 stat=node? node->xstat:0;
    return (stat==LTYPE_GRID || stat==LTYPE_VBOX || stat==LTYPE_HBOX);
}

bool isPageNode(TreeNode* node) {
    U16 stat=node? node->xstat:0;
    return (stat==WTYPE_PAGE || stat==WTYPE_MAIN || stat==WTYPE_DIALOG);
}
QWidget* gwidget( TreeNode* cf ) {
    StrVar* sv=cf?cf->gv("@this"): NULL;
    return SVCHK('w',0) ? (QWidget*)SVO : NULL;
}
QLayout* getLayout(TreeNode* cf, U16* stat) {
    StrVar* sv=cf ? cf->gv("@this"): NULL;
    if( SVCHK('l',0) ) {
        QLayout* layout=(QLayout*)SVO;
        if( stat ) *stat=cf->xstat;
        return layout;
    }
    return NULL;
}

void setLayout(TreeNode* cf, QLayout* layout, U16 stat) {
    StrVar* sv=cf->gv("margin");
    if( sv ) {
        if( isNumberVar(sv) ) {
            int num=toInteger(sv);
            layout->setMargin(num);
        } else {
            XParseVar pv(sv);
            int l=0,t=0,r=0,b=0;
            for(int n=0; n<4 && pv.valid(); n++ ) {
                LPCC next=pv.NextWord();
                if( isnum(next) ) {
                    if( n==0 ) l=atoi(next);
                    else if( n==1 ) t=atoi(next);
                    else if( n==2 ) r=atoi(next);
                    else if( n==3 ) b=atoi(next);
                }
                char ch=pv.ch();
                if( isoper(ch) || ch==',' ) pv.incr();
            }
            layout->setContentsMargins(l,t,r,b);
        }

    }
    sv=cf->gv("spacing");
    if( isNumberVar(sv) ) {
        layout->setSpacing(toInteger(sv));
    }
    cf->xstat=stat;
    cf->GetVar("@this")->setVar('l',0,(LPVOID)layout);
}
void setWidget(TreeNode* cf, QWidget* widget, U16 stat) {
    cf->xstat=stat;
    cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
}


bool setScroll(TreeNode* cf) {
    QWidget* w=gwidget(cf);
    StrVar* sv=cf->GetVar("@scroll");
    if( w && !SVCHK('w',20) ) {
        QScrollArea* scroll=new QScrollArea();
        scroll->setWidget(w);
        scroll->setWidgetResizable(true);
        sv->setVar('w',20,(LPVOID)scroll);
        return true;
    }
    return false;
}

TreeNode* findChildId( TreeNode* root, LPCC id ) {
    if( root==NULL || slen(id)==0 ) return NULL;
    // if( root->cmp("id",id) ) return root;
    int cnt=root->childCount();
    for( int n=0; n<cnt ; n++ ) {
        TreeNode* cf=root->child(n);
        if( cf->cmp("id",id) ) return cf;
        cf = cf->childCount()>0 ? findChildId(cf,id): NULL;
        if( cf ) return cf;
    }
    return NULL;
}

TreeNode* findWidgetId( TreeNode* root, LPCC id, int mode ) {
    if( root==NULL || slen(id)==0 ) return NULL;
    int cnt=root->childCount();
    for( int n=0; n<cnt ; n++ ) {
        TreeNode* cf=root->child(n);
        if( !isLayoutNode(cf) ) { // cf->isNodeFlag(GFLAG_LAYOUT)
            if( cf->cmp("id",id) ) {
                if( mode>0 ) {
                    n+=mode;
                    if( n>=cnt ) n=cnt-1;
                    cf=root->child(n);
                } else if( mode<0 ) {
                    n-=mode;
                    if( n<0 ) n=0;
                    cf=root->child(n);
                }
                return cf;
            }
        }
        cf = findWidgetId(cf,id,mode);
        if( cf ) return cf;
    }
    return NULL;

}

TreeNode* findWidgetTag( TreeNode* root, LPCC tag, XListArr* arr  ) {
    for( int n=0; n<root->childCount() ; n++ ) {
        TreeNode* cf=root->child(n);        
        if( cf->cmp("tag",tag) ) {
            if( arr ) {
                arr->add()->setVar('n',0,(LPVOID)cf);
            } else {
                return cf;
            }
        }
        cf = findWidgetTag(cf,tag,arr);
        if( cf && arr==NULL ) return cf;
    }
    return NULL;

}

TreeNode* findWidgetNode(TreeNode*cf, U16 stat) {
    for(int n=0; cf && n<16; n++ ) {
        if( stat==0 ) {
            if( cf->isNodeFlag(GFLAG_PAGE) )
                return cf;
        } else if( cf->xstat==stat)
            return cf;
        cf=cf->parent();
    }
    return getCf();
}
XListArr* getWidgetArr(TreeNode* root, XListArr* a ) {
    if( root==NULL ) return a;
    for( int n=0; n<root->childCount() ; n++ ) {
        TreeNode* cf=root->child(n);
        if( cf->isNodeFlag(GFLAG_LAYOUT) || ccmp(cf->get("tag"),"row") ) {
            getWidgetArr(cf, a );
            continue;
        }
        if( gwidget(cf)==NULL ) continue;
        a->add()->setVar('n',0,(LPVOID)cf);
        LPCC tag=cf->get("tag");
        if( slen(tag) ) {
            getWidgetArr(cf, a );
        }
    }
    return a;
}
bool addMenuActions(GMenu* menu, XListArr* a, LPCC code, TreeNode* thisNode, XListArr* menuActions) {
    if( menu==NULL || a==NULL ) return false;
    for(int n=0,sz=a->size();n<sz;n++) {
        StrVar* sv=a->get(n);
        if( SVCHK('a',0) ) {
            GMenu* sub=new GMenu(menu->config(), menu);
            addMenuActions(sub, (XListArr*)SVO, code, thisNode, menuActions );
        } else {
            LPCC actionId=toString(sv );
            if( ccmp(actionId,"-") ) {
                menu->addSeparator();
            } else {
                sv=getStrVar("action", actionId);
                if( SVCHK('w',11) ) {
                    GAction* act=(GAction*)SVO;
                    if( thisNode ) act->config()->GetVar("@page")->setVar('n',0,(LPVOID)thisNode );
                    if( menuActions ) menuActions->add()->setVar('n',0,(LPVOID)act->config());
                    menu->addAction(act);
                }
            }
        }
    }
    return true;
}

bool procWidgetEvent(QEvent* evt, TreeNode* cf, QWidget* curWidget, U32 flag ) {

    if( cf==NULL || !cf->isNodeFlag(FLAG_SETEVENT) ) {
        return false;
    }
    /* GViewEventFilter
    if( flag==1 ) {
        return false;
    }
    */

    U16 stat=cf->xstat;
    switch(evt->type()) {
    case QEvent::KeyPress: {
        XFuncNode* fn = getEventFuncNode(cf, "onKeyDown");        
        QKeyEvent* e = static_cast<QKeyEvent*>(evt);
        cf->GetVar("@key")->setVar('0',0).addInt(e->key());
        if( fn ) {
            // key, mode
            PARR arrs=getLocalParams(fn);
            QKeyEvent* e = static_cast<QKeyEvent*>(evt);
            arrs->add()->setVar('0',0).addInt(e->key());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            StrVar* sv = &(fn->xrst);
            if( SVCHK('3',1) ) {
                return true;
            }
        }
    } break;
    case QEvent::ActivationChange: {
        XFuncNode* fn = getEventFuncNode(cf, "onActivationChange");
        if( fn ) {
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
        }
    } break;
    case QEvent::FocusIn: {
        XFuncNode* fn = getEventFuncNode(cf, "onFocusIn");
        if( fn ) {
            QFocusEvent *e = static_cast<QFocusEvent*>(evt);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            StrVar* sv = &(fn->xrst);
            if( SVCHK('3',1) ) {
                return true;
            }
        }
    } break;
    case QEvent::FocusOut: {
        XFuncNode* fn = getEventFuncNode(cf, "onFocusOut");
        if( fn ) {
            QFocusEvent *e = static_cast<QFocusEvent*>(evt);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            StrVar* sv = &(fn->xrst);
            if( SVCHK('3',1) ) {
                return true;
            }
        }
    } break;
    case QEvent::Move: {
        XFuncNode* fn = getEventFuncNode(cf, "onMove");
        if( fn ) {
            QMoveEvent *e = static_cast<QMoveEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            StrVar* sv = &(fn->xrst);
            if( SVCHK('3',1) ) {
                return true;
            }
        }
    } break;
    case QEvent::Wheel: {
        XFuncNode* fn = getEventFuncNode(cf, "onMouseWheel");
        if( fn ) {
            // delta, pos, mode
            QWheelEvent *e = static_cast<QWheelEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            arrs->add()->setVar('0',0).addInt(e->delta());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            StrVar* sv = &(fn->xrst);
            if( SVCHK('3',1) ) {
                return true;
            }
        }
    } break;
    case QEvent::DragEnter: {
        if( stat==WTYPE_TREE  ) return false;
        XFuncNode* fn = getEventFuncNode(cf, "onDrag");
        if( fn ) {
            // type, data
            QDragEnterEvent *e = static_cast<QDragEnterEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->set("enter");
            arrs->add()->setVar('m',2,(LPVOID)e->mimeData());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            LPCC ty=toString(fn->getResult());
            if( ccmp(ty,"accept") ) {
                e->accept();
            } else if( ccmp(ty,"move") ) {
                e->setDropAction(Qt::MoveAction); e->accept();
            } else if( ccmp(ty,"copy") ) {
                e->setDropAction(Qt::CopyAction); e->accept();
            } else if( ccmp(ty,"proposed") ) {
                e->acceptProposedAction();
            } else
                e->ignore();
        }
    } break;
    case QEvent::DragLeave: {
        if( stat==WTYPE_TREE  ) return false;
        XFuncNode* fn = getEventFuncNode(cf, "onDrag");
        if( fn ) {
            // type, data, pos
            QDragLeaveEvent *e = static_cast<QDragLeaveEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->set("leave");
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            LPCC ty=toString(fn->getResult());
            if( ccmp(ty,"accept") || ccmp(ty,"copy") || ccmp(ty,"move") )
                e->accept();
            else
                e->ignore();
        }
    } break;
    case QEvent::DragMove: {
        if( stat==WTYPE_TREE  ) return false;
        XFuncNode* fn = getEventFuncNode(cf, "onDragMove");
        if( fn ) {
            // data, pos
            QDragMoveEvent *e = static_cast<QDragMoveEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('m',2,(LPVOID)e->mimeData());
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            LPCC ty=toString(fn->getResult());
            if( ccmp(ty,"accept") ) {
                e->accept();
            } else if( ccmp(ty,"move") ) {
                e->setDropAction(Qt::MoveAction); e->accept();
            } else if( ccmp(ty,"copy") ) {
                e->setDropAction(Qt::CopyAction); e->accept();
            } else if( ccmp(ty,"proposed") ) {
                e->acceptProposedAction();
            } else
                e->ignore();
        }
    } break;
    case QEvent::Drop: {
        if( stat==WTYPE_TREE  ) return false;
        XFuncNode* fn = getEventFuncNode(cf, "onDrop");
        if( fn ) {
            // data, pos
            QDropEvent *e = static_cast<QDropEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('m',2,(LPVOID)e->mimeData());
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            LPCC result = toString(&fn->xrst);
            if( ccmp(result,"ignore") ) {
                e->ignore();
                return true;
            } else if( ccmp(result,"accept") ) {
                e->accept();
            } else if( ccmp(result,"move") ) {
                e->setDropAction(Qt::MoveAction); e->accept();
            } else if( ccmp(result,"copy") ) {
                e->setDropAction(Qt::CopyAction); e->accept();
            } else if( ccmp(result,"proposed") ) {
                e->acceptProposedAction();
            }
        }
    } break;
    case QEvent::ContextMenu: {
        static long prevContextTick=0;
        XFuncNode* fn = getEventFuncNode(cf, "onContextMenu");
        if( fn ) {
            if( GetTickCount()-prevContextTick > 500 ) {
                // pos
                QContextMenuEvent* e = static_cast<QContextMenuEvent*>(evt);
                PARR arrs=getLocalParams(fn);
                prevContextTick=GetTickCount();
                arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
                setFuncNodeParams(fn, arrs);
                if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
                fn->call();
                LPCC result = toString(&fn->xrst);
                if( ccmp(result,"ignore") ) {
                    e->ignore();
                    return true;
                }
            }
            return true;
        }
    } break;
    case QEvent::MouseButtonDblClick: {
        // if( stat==WTYPE_TREE  ) return false;
        XFuncNode* fn = getEventFuncNode(cf, "onDoubleClick");
        if( fn ) {
            QMouseEvent* e = static_cast<QMouseEvent*>(evt);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            LPCC rtn = toString(&(fn->xrst));
            if( ccmp(rtn,"ignore") ) return true;
        }
        // cf->GetVar("@mouseDbClick")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
    } break;
    case QEvent::MouseMove:{
        if( stat==WTYPE_TREE  ) return false;
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if( (e->buttons() == Qt::LeftButton) && (cf->xstat!=WTYPE_TREE) ) {
            StrVar* sv=cf->GetVar("@mousePos");
            if( SVCHK('i',2) ) {
                QPoint pt = e->pos()-QPoint(sv->getInt(4),sv->getInt(8));
                if( pt.manhattanLength()>QApplication::startDragDistance() ) {
                    XFuncNode* fn = getEventFuncNode(cf, "onDragStart");
                    if( fn ) {
                        PARR arrs=getLocalParams(fn);
                        arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
                        setFuncNodeParams(fn, arrs);
                        if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
                        fn->call();
                        sv=&(fn->xrst);
                        if( SVCHK('3',1) ) {
                            return true;
                        }
                    }
                }
            }
        }
        XFuncNode* fn = getEventFuncNode(cf, "onMouseMove");
        if( fn ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            if( ccmp(toString(&(fn->xrst)),"ignore") ) {
                return true;
            }
        }
    } break;
    case QEvent::MouseButtonPress: {
        if( stat==WTYPE_TREE  ) return false;
        // if( getCf()->isNodeFlag(0x4000) ) break;
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        cf->GetVar("@mousePos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
        XFuncNode* fn = getEventFuncNode(cf, "onMouseDown");
        if( fn ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            arrs->add()->set(e->button()==Qt::LeftButton?"left":"right");
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            if( ccmp(toString(&(fn->xrst)),"ignore") ) {
                return true;
            }
        }
    } break;
    case QEvent::MouseButtonRelease: {
        if( stat==WTYPE_TREE  ) return false;
        // if( getCf()->isNodeFlag(0x4000) ) break;
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        StrVar* sv=cf->gv("@mousePos");
        if( SVCHK('i',2) ) {
            int x=sv->getInt(4)-8,y=sv->getInt(8)-8;
            QRect rc(x,y,16,16);
            sv->reuse();
            if( rc.contains(e->pos()) ) {
                sv=cf->gv("onMouseClick");
            }
            if( !SVCHK('f',0) ) {
                sv=cf->gv("onMouseUp");
            }
        } else {
            qDebug("QEvent::MouseButtonRelease=>onMouseClick %s %d",cf->get("tag"),flag);
            sv=cf->gv("onMouseUp");
        }

        if( SVCHK('f',0) ) {
            XFuncNode* fn=(XFuncNode*)SVO;
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
            arrs->add()->set(e->button()==Qt::LeftButton?"left":"right");
            setFuncNodeParams(fn, arrs);
            if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
            fn->call();
            cf->GetVar("@mousePos")->reuse();
            if( ccmp(toString(&(fn->xrst)),"ignore") ) {
                return true;
            }
        }
    } break;
    case QEvent::Timer:{
        if( !cf->isNodeFlag(FLAG_RUN) ) {
            // qDebug("timer skip %s", cf->get("id"));
            return false;
        }
        StrVar* sv=NULL;
        QTimerEvent* e = static_cast<QTimerEvent*>(evt);
        int tid=e->timerId();
        if( cf->isNodeFlag(FLAG_SINGLE)) {
            char tcode[32];
            sprintf(tcode,"@timer_%d",tid);
            sv=cf->gv(tcode);
            if( SVCHK('f',1) ) {
                XFuncNode* fnInit=NULL;
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                // qDebug("#0#timer once %s\n", tcode);
                sv=cf->gv("onInit");
                if( SVCHK('f',0) ) {
                    fnInit=(XFuncNode*)SVO;
                }
                XFunc* pfc=fsrc->xfunc;
                XFuncNode* fn=gfns.getFuncNode(pfc, fnInit);
                if( fn ) {
                    fn->GetVar("@this")->setVar('n',0,(LPVOID)cf);
                    fn->call();
                    funcNodeDelete(fn);
                    if(curWidget) {
                        curWidget->killTimer(tid);
                    } else {
                        qDebug("#0#timer sigle shot killed widget error (%s:%d)", cf->get("id"), tid);
                    }
                }
            } else {
                qDebug("#0#timer single function error %s\n", tcode);
            }
            cf->clearNodeFlag(FLAG_SINGLE);
            return true;
        }
        sv=cf->gv("@timers");
        if(!SVCHK('a',0)) {
            return false;
        }
        XListArr* arr=(XListArr*)SVO;
        int idx=-1;
        for(int n=0, sz=arr->size(); n<sz; n++ ) {
            if( tid==toInteger(arr->get(n)) ) {
                idx=n;
                break;
            }
        }
        if( idx==-1 ) {
            return false;
        }


        XFuncNode* fn=getEventFuncNode(cf, "onTimer");
        if( fn ) {
            if( fn->isNodeFlag(FLAG_CALL) ) {
                qDebug("#0#timer timeOut (tid:%d)\n",tid);
            } else {
                PARR arrs=getLocalParams(fn);
                StrVar* param=arrs->add();
                param->setVar('0',0).addInt(tid);
                /*
                XListArr* arr=cf->addArray("@timers");
                bool find=false;
                for(int n=0, sz=arr->size(); n<sz; n++ ) {
                    if( getPairValue(arr,n,param->reuse()) ) {
                        if( checkFuncObject(param,'0',0) ) {
                            int id=param->getInt(4);
                            if( tid==id ) {
                                find=true;
                                getPairKey(arr,n,param->reuse());
                                break;
                            }
                        }
                    }
                }
                if( find ) {
                    setFuncNodeParams(fn, arrs);
                    fn->call();
                } else {
                    if(curWidget) {
                        curWidget->killTimer(tid);
                        qDebug("#0#timer code not find (tid:%d)", tid);
                    }
                }
                */
                setFuncNodeParams(fn, arrs);
                fn->call();
            }
            return true;
        }
    } break;
    case QEvent::Resize:{
        XFuncNode* fn = getEventFuncNode(cf, "onResize");
         if( fn ) {
             if(flag==1) fn->GetVar("@view")->setVar('w',1,(LPVOID)curWidget);
             fn->call();
        }
    } break;
    case QEvent::Close:{
        XFuncNode* fn = getEventFuncNode(cf, "onClose");
        if( fn ) {
            QCloseEvent* e = static_cast<QCloseEvent*>(evt);
            fn->call();
            LPCC ty = toString(fn->getResult());
            if( ccmp(ty,"hide") ) {
                e->ignore();
                curWidget->hide();
                return true;
            } else if( ccmp(ty,"ignore") ) {
                e->ignore();
                return true;
            }
        }
    } break;
    case QEvent::Paint: {
        /*
        if( !cf->isNodeFlag(FLAG_DRAW) ) return false;
        if( stat==WTYPE_CANVAS ) {
            GCanvas* canvas=qobject_cast<GCanvas*>(curWidget);
            // qDebug("canvas draw %s %s", canvas?"canvas":"notset", cf->get("id"));
            if( canvas ) {
                QPaintEvent* e = static_cast<QPaintEvent*>(evt);
                QPainter painter(canvas);
                canvas->draw(&painter, QRectF(e->rect()) );
                painter.end();
            }
        } else
        */


        if( stat==WTYPE_CONTEXT || stat==WTYPE_VIDEO || stat==WTYPE_TREE || stat==WTYPE_COMBO || stat==WTYPE_SPLITTER ) return false;
        {
            XFuncNode* fn = getEventFuncNode(cf, "onDraw");
            if( fn ) {
                QPaintEvent* e = static_cast<QPaintEvent*>(evt);                
                TreeNode* d=cf->addNode("@drawObject");
                d->xstat=FNSTATE_DRAW;
                d->xtype=0;
                /*
                QPainter* painter=NULL;
                StrVar* sv=d->GetVar("@g");
                if( SVCHK('g',0) ) {
                    painter=(QPainter*)SVO;
                } else {
                    painter=new QPainter(curWidget);
                    sv->setVar('g',0,(LPVOID)painter);
                }
                */
                QPainter painter;
                if(painter.begin(curWidget) && painter.isActive() ) {
                    // param: dc, rc
                    PARR arrs=getLocalParams(fn);
                    StrVar* sv=d->GetVar("@rect");
                    setVarRectF(sv, e->rect());
                    d->GetVar("@g")->setVar('g',0,(LPVOID)&painter);
                    arrs->add()->setVar('n',0,(LPVOID)d);
                    arrs->add()->add(sv);
                    setFuncNodeParams(fn, arrs);
                    fn->call();
                    painter.end();
                }
                if(cf->isBool("@updateCheck")) {
                    cf->setBool("@updateCheck", false);
                }
                /*
                if( painter.begin(curWidget) ) {
                }
                */
                return false;
            }
        }
    } break;
    case QEvent::TouchBegin:{
        QTouchEvent* e = static_cast<QTouchEvent*>(evt);
        if (e->touchPoints().count()==1 && getCf()->isNodeFlag(0x4000) ) {
            const QTouchEvent::TouchPoint &tp = e->touchPoints().first();
            QPointF pf=tp.scenePos();
            QPoint pt = curWidget->mapFromGlobal(QPoint((int)pf.x(), (int)pf.y()));
            qDebug("QEvent::TouchBegin (%d,%d)\n", pt.x(), pt.y() );
            XFuncNode* fn = getEventFuncNode(cf, "onMouseDown");
            if( fn ) {
                PARR arrs=getLocalParams(fn);
                arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
                setFuncNodeParams(fn, arrs);
                fn->call();
            }
        }
    } break;
    case QEvent::TouchUpdate:{
        QTouchEvent* e = static_cast<QTouchEvent*>(evt);
        if (e->touchPoints().count()==1 && getCf()->isNodeFlag(0x4000) ) {
            const QTouchEvent::TouchPoint &tp = e->touchPoints().first();
            QPointF pf=tp.scenePos();
            QPoint pt = curWidget->mapFromGlobal(QPoint((int)pf.x(), (int)pf.y()));
            XFuncNode* fn = getEventFuncNode(cf, "onMouseMove");
            if( fn ) {
                PARR arrs=getLocalParams(fn);
                arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
                setFuncNodeParams(fn, arrs);
                fn->call();
            }
        }
    } break;
    case QEvent::TouchEnd:{
        QTouchEvent* e = static_cast<QTouchEvent*>(evt);
        if (e->touchPoints().count()==1 && getCf()->isNodeFlag(0x4000) ) {
            const QTouchEvent::TouchPoint &tp = e->touchPoints().first();
            QPointF pf=tp.scenePos();
            QPoint pt = curWidget->mapFromGlobal(QPoint((int)pf.x(), (int)pf.y()));
            qDebug("QEvent::TouchEnd (%d,%d)\n", pt.x(), pt.y() );
            XFuncNode* fn = getEventFuncNode(cf, "onMouseUp");
            if( fn ) {
                PARR arrs=getLocalParams(fn);
                arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
                setFuncNodeParams(fn, arrs);
                fn->call();
            }
        }
    } break;

    case USER_EVENT: {
        // ### version 1.0.4
        UserEvent* e = static_cast<UserEvent*>(evt);
        if( e==NULL )
            return false;
        LPCC type=toString( &(e->xdata));
        TreeNode* eventNode=e->xcf;
        XFuncNode* fn = getEventFuncNode(cf, "onEvent");
        if( fn && e->xcf ) {
            PARR arrs=getLocalParams(fn);
            qDebug("#0#onEvent type:%s\n", type );
            arrs->add()->set(type);
            arrs->add()->setVar('n',0,(LPVOID)eventNode);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    } break;
    default: {
		GWidget* gw=static_cast<GWidget*>(curWidget);
		if(gw  && cf->cmp("type","overlay") ) {
			if (evt->type() == QEvent::ParentAboutToChange) {
				if (gw->parent()) gw->parent()->removeEventFilter(gw);
			} else if (evt->type() == QEvent::ParentChange) {
				qDebug("#0#widget parent change!!! ");
				gw->newParent();
			}
		}
    }break;
    }
    return false;
}


QLayout* createLayout(TreeNode* cf, QWidget* parent ) {
    if( cf==NULL ) return NULL;
    LPCC tag=cf->get("tag");
    cf->setNodeFlag(GFLAG_LAYOUT);
    if( ccmp(tag,"form") ) {
        GridLayout* grid = new GridLayout(cf, parent);
        setLayout(cf, grid, LTYPE_GRID);
        cf->setInt("@r",0);
        cf->setInt("@c",0);
        return grid;
    } else if( ccmp(tag,"hbox") ) {
        HBox* box = new HBox(cf,parent);
        setLayout(cf, box, LTYPE_HBOX);
        return box;
    } else if( ccmp(tag,"vbox") || ccmp(tag,"page") ) {
        VBox* box = new VBox(cf,parent);
        setLayout(cf, box, LTYPE_VBOX);
        return box;
    }
    return NULL;
}
QWidget* createPageVar(TreeNode* tn, StrVar* var, int sp, int ep) {
    if( parsePageVar(tn, var, sp, ep, tn) ) {
        LPCC tag=tn->get("tag");
        if( slen(tag)==0 ) {
            tag="page";
            tn->set("tag", tag);
        }
        return createWidget(tag, tn, NULL );
    }
    return NULL;
}
GPage* createPage(LPCC pageId, LPCC fileName, StrVar* rst) {
    if( rst==NULL ) rst=getStrVarTemp();
    TreeNode* tn=getTreeNode("page", pageId);
    tn->set("id", pageId);
    tn->setNodeFlag(FLAG_PERSIST);
    if( tn && fileReadAll(fileName, rst) ) {
        QWidget* widget=createPageVar(tn, rst, 0, rst->length());
        if( widget ) return qobject_cast<GPage*>(widget);
    }
    return NULL;
}

QWidget* createWidget(LPCC tag, TreeNode* cf, QWidget* parent ) {
    if( cf==NULL || slen(tag)==0 )
        return NULL;
    QWidget* widgetPrev=gwidget(cf);
    if( widgetPrev ) {
        return widgetPrev;
    }
    if( slen(cf->get("tag"))==0 ) {
        cf->set("tag", tag);
    }
    qDebug("xxxxxx createWidget %s xxxxxxx", tag);
    if( ccmp(tag,"dialog") ) {
        // if( cf->childCount()==0 ) return NULL;
        QLayout* layout=NULL;
        GDialog* widget = new GDialog(cf,parent);
        widget->hide();
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        cf->xstat=WTYPE_DIALOG;
        U32 flag = widget->windowFlags();
        flag |= Qt::WindowMaximizeButtonHint;
        flag|=Qt::WindowMinimizeButtonHint;
        widget->setWindowFlags((Qt::WindowFlags)flag);
        createWidgetChild(cf,widget);
        layout=getLayout(cf->child(0));
        if( layout ) {
            widget->setLayout(layout);
            qDebug("xxxxxx createWidget set dialog layout ok (id:%s) xxxxxx", cf->get("id") );
        } else {
            qDebug("#9# createWidget dialog layout error node:%s", cf->log() );
        }
    } else if( ccmp(tag,"page") ) {
        QLayout* layout=NULL;
        GPage* widget = new GPage(cf,parent);
        widget->hide();
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        cf->xstat=WTYPE_PAGE;
        createWidgetChild(cf,widget);
        layout=getLayout(cf->child(0));
        if( layout ) {
            widget->setLayout(layout);
            qDebug("xxxxxx createWidget set page layout ok (id:%s) xxxxxx", cf->get("id") );
        } else {
            qDebug("#9# createWidget page layout error node:%s", cf->log() );
        }
    } else if( ccmp(tag,"main") ) {
        // if( cf->childCount()==0 ) return NULL;
        QLayout* layout=NULL;
        GMainPage* widget = new GMainPage(cf,parent);
        GPage* page = new GPage(cf,widget);
        widget->hide();
        cf->GetVar("@this")->setVar('w',0,(LPVOID)page);
        cf->xstat=WTYPE_PAGE;
        createWidgetChild(cf,page);
        layout=getLayout(cf->child(0));
        if( layout ) {
            qDebug("xxxxxx createWidget main ok xxxxxxx" );
            page->setLayout(layout);
            cf->xstat=WTYPE_MAIN;
            cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
            cf->GetVar("@page")->setVar('w',0,(LPVOID)page);
            widget->setCentralWidget(page);
        } else {
            qDebug("#9# createWidget main layout error node:%s", cf->log() );
        }
	} else if( ccmp(tag,"widget") ) {
		GWidget* widget = new GWidget(cf,parent);
		cf->xstat=WTYPE_WIDGET;
		cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
	} else if( ccmp(tag,"context") ) {
		GContext* widget = new GContext(cf,parent);
		cf->xstat=WTYPE_CONTEXT;
		cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
	} else if( ccmp(tag,"button") ) {
        GButton* widget = new GButton(cf,parent);
        cf->xstat=WTYPE_BUTTON;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"tree") ) {
        GTree* widget = new GTree(cf,parent);
        cf->xstat=WTYPE_TREE;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        widget->setAttribute(Qt::WA_StyledBackground);
        widget->setIndentation(20);
        widget->setAnimated(false);
        widget->setFocusPolicy(Qt::StrongFocus);
        widget->setHeaderHidden(true);
        widget->setSelectionMode(QAbstractItemView::SingleSelection);
        widget->setSelectionBehavior(QAbstractItemView::SelectRows);
        widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        // cf->GetVar("@treeMode")->setVar('3',1);
    } else if( ccmp(tag,"grid") ) {
        GTree* widget = new GTree(cf,parent);
        cf->xstat=WTYPE_TREE;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        widget->setIndentation(0);
        widget->setExpandsOnDoubleClick(false);
        widget->header()->setStretchLastSection(false);
        widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    } else if( ccmp(tag,"label") || ccmp(tag,"space") ) {
        GLabel* widget = new GLabel(cf);
        cf->xstat=WTYPE_LABEL;
        if( ccmp(tag,"space") ) {
            widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        }
        LPCC text=cf->get("text");
        if(slen(text)) {
            widget->setText(KR(text));
        }
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"editor") ) {
        GTextEdit* widget = new GTextEdit(cf,parent);
        cf->xstat=WTYPE_TEXTEDIT;
        // widget->setAcceptRichText( false );
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"video") ) {
        GVideo* widget = new GVideo(cf,parent);
        cf->xstat=WTYPE_VIDEO;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        // cf->setNodeFlag(FLAG_DRAW);
    } else if( ccmp(tag,"player") ) {
        GMpWidget* widget = new GMpWidget(cf,parent);
        cf->xstat=FNSTATE_MPPLAYER;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"toolbutton") ) {
        GToolButton* widget = new GToolButton(cf,parent);
        cf->xstat=WTYPE_TOOLBTN;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        LPCC icon = cf->get("icon");
        if( slen(icon) ) {
            QPixmap* pixmap = getPixmap(icon);
            if( pixmap ) {
                widget->setIcon(QIcon(*pixmap));
            }
        }
#ifdef WEBVIEW_USE
    } else if( ccmp(tag,"webview") ) {
        WebView* widget = new WebView(cf,parent);
        cf->xstat=WTYPE_WEBVIEW;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
#endif
    } else if( ccmp(tag,"combo") ) {
        GCombo* widget = new GCombo(cf,parent);
        StrVar* sv=confVar("setup.comboLineHeight");
        if( isNumberVar(sv) ) {
            ComboDelegate* d = new ComboDelegate(cf);
            int lineHeight=toInteger(sv);
            cf->GetVar("@d")->setVar('w',9,(LPVOID)d);
            cf->GetVar("@lineHeight")->setVar('0',0).addInt(lineHeight);
            widget->setItemDelegate(d);
        }
        // widget->setStyleSheet("QComboBox QAbstractItemView::item {  min-height : 32px;}");
        cf->xstat=WTYPE_COMBO;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"calendar") ) {
        GCalendar* widget = new GCalendar(cf,parent);
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        cf->xstat=WTYPE_CALENDAR;

    } else if( ccmp(tag,"date") ) {
        GDateEdit* widget = new GDateEdit(cf,parent);
        widget->setCalendarPopup(true);
        widget->setDate( QDate::currentDate());
        cf->xstat=WTYPE_DATEEDIT;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"time") ) {
        GTimeEdit* widget = new GTimeEdit(cf,parent);
        LPCC fmt = cf->get("format");
        if( slen(fmt) )
            widget->setDisplayFormat(KR(fmt)); //"HH:mm:ss.zzz");
        else
            widget->setDisplayFormat("HH:mm");
        cf->xstat=WTYPE_TIMEEDIT;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"list") ) {
        GListVIew* widget = new GListVIew(cf,parent);
        cf->xstat=WTYPE_LIST;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"table") ) {
        GTable* widget = new GTable(cf,parent);
        cf->xstat=WTYPE_TABLE;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"group") ) {
        GGroup* widget = new GGroup(cf,parent);
        LPCC title=cf->get("title");
        cf->xstat=WTYPE_GROUP;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        if( cf->gv("form") ) {
            if( slen(title)==0 ) {
                widget->setContentsMargins(0,0,0,0);
            }
        }
        if(slen(title)) {
            widget->setTitle(KR(title));
        }
    } else if( ccmp(tag,"radio") ) {
        cf->xstat=WTYPE_RADIO;
        StrVar* sv = cf->gv("buttons");
        if( sv && sv->length() ) {
            GGroup* grp = new GGroup(cf,parent);
            QVBoxLayout* vbox = new QVBoxLayout(grp);
            XListArr* a = new XListArr(true);
            XParseVar pv(sv);
            while( pv.valid() ) {
                pv.findEnd(",");
                GRadio* rd = new GRadio(cf,pv.v());
                a->add()->setVar('w',41,(LPVOID)rd);
                vbox->addWidget(rd);
            }
            sv->setVar('a',0,(LPVOID)a);
            vbox->addStretch(1);
            grp->setLayout(vbox);
            sv = cf->gv("title");
            if( sv ) {
                LPCC title = toString(sv);
                grp->setTitle(KR(title));
            }
            cf->GetVar("@this")->setVar('w',0,(LPVOID)grp);
            return grp;
        }
        sv = cf->gv("text");
        GRadio* rd = new GRadio(cf, sv?toString(sv): NULL, parent);
        cf->GetVar("@this")->setVar('w',0,(LPVOID)rd);
    } else if( ccmp(tag,"check") ) {
        GCheck* widget = new GCheck(cf,parent);
        cf->xstat=WTYPE_CHECK;
        StrVar* sv = cf->gv("text");
        if( sv && sv->length() ) {
            LPCC txt = toString(sv);
            widget->setText(KR(txt));
        }
        if( isVarTrue(cf->gv("checked")) ) widget->setChecked(true);
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"spin") ) {
        GSpin* widget = new GSpin(cf,parent);
        cf->xstat=WTYPE_SPIN;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        int sp=0, ep=100;
        StrVar* sv = cf->gv("range");
        if( sv ) {
            XParseVar pv(sv);
            LPCC val=pv.findEnd(",").v();
            if( isnum(val) ) {
                if( pv.valid() ) {
                    sp=atoi(val);
                    val=pv.findEnd(",").v();
                    if( isnum(val) ) ep=atoi(val);
                } else {
                    ep=atoi(val);
                }
            }
        }
        if( sp<ep ) {
            widget->setRange(sp,ep);
        }
    } else if( ccmp(tag,"input") ) {
        GInput* widget = new GInput(cf,parent);
        cf->xstat=WTYPE_LINEINPUT;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
        LPCC fmt = cf->get("format");
        if( slen(fmt) ) {
            if( ccmp(fmt,"number") ) widget->setValidator(new QIntValidator(widget));
            else if( ccmp(fmt,"float") ) widget->setValidator(new QDoubleValidator(widget));
        }
        StrVar* sv=cf->gv("maxLength");
        if( isNumberVar(sv) ) {
            widget->setMaxLength(toInteger(sv));
        }
    } else if( ccmp(tag,"tab") ) {
        GTab* widget = new GTab(cf, parent);
        StrVar* sv=cf->gv("addPages");
        if( sv && sv->length() ) {
            TreeNode* funcInfo=cfNode("@funcInfo");
            XListArr* arr=funcInfo->addArray("@addPages");
            arr->add()->setVar('n',0,(LPVOID)cf);
        }
        cf->xstat=WTYPE_TAB;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"div") ) {
        GStacked* widget = new GStacked(cf, parent);
        StrVar* sv=cf->gv("addPages");
        if( sv && sv->length() ) {
            TreeNode* funcInfo=cfNode("@funcInfo");
            XListArr* arr=funcInfo->addArray("@addPages");
            arr->add()->setVar('n',0,(LPVOID)cf);
        }
        cf->xstat=WTYPE_DIV;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"toolbox") ) {
        GToolBox* widget = new GToolBox(cf, parent);
        cf->xstat=WTYPE_TOOLBOX;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"canvas") ) {
        GCanvas* widget = new GCanvas(cf,parent);
        cf->xstat=WTYPE_CANVAS;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);        
        widget->setMouseTracking(true);
        // widget->setAcceptDrops(true);
        // widget->setAutoFillBackground(false);
        widget->setContentsMargins(0, 0, 0, 0);
        widget->setAttribute(Qt::WA_AcceptTouchEvents, true);

        // widget->setAttribute(Qt::WA_PaintOnScreen,true);
        // cf->GetVar("@dtid")->setVar('0',0).addInt(widget->startTimer(300));
        if( isVarTrue(cf->gv("scroll")) ) {
            /*
            widget->setAutoFillBackground(true);
            widget->setBackgroundRole(QPalette::Base);
            widget->setAttribute(Qt::WA_TranslucentBackground,true);
            */
            widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            // widget->setScaledContents(true);
            StrVar* sv = cf->GetVar("@scroll");
            if( widget && !SVCHK('w',20) ) {
                QScrollArea* scroll=new QScrollArea(parent);
                scroll->setWidget(widget);
                scroll->setWidgetResizable(false);
                scroll->setStyleSheet("QScrollArea {border: none;}");
                LPCC bg=cf->get("background");
                if( slen(bg) ) {
                    scroll->setBackgroundRole(QPalette::Dark);
                }
                sv->setVar('w',20,(LPVOID)scroll);
                scroll->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
                return scroll;
            }
        } else {
            // widget->setScaledContents(false);
            // ### version 1.0.4
            if( cf->gv("width")==NULL ) {
                widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
                if( cf->gv("height")==NULL )
                    widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
            } else if( cf->gv("height")==NULL ) {
                widget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
            }
        }
    } else if( ccmp(tag,"splitter") ) {
        GSplitter* widget = new GSplitter( cf, parent);
        StrVar* sv=cf->gv("addPages");
        if( sv && sv->length() ) {
            TreeNode* funcInfo=cfNode("@funcInfo");
            XListArr* arr=funcInfo->addArray("@addPages");
            arr->add()->setVar('n',0,(LPVOID)cf);
        }
        sv = cf->gv("handleWidth");
        if( sv ) {
            int w = isNumberVar(sv) ? toInteger(sv): 4;
            widget->setHandleWidth(w);
        }
        sv=cf->gv("type");
        if( sv ) {
            LPCC ty=toString(sv);
            if( ccmp(ty,"vbox") ) {
                widget->setOrientation(Qt::Vertical);
            } else if(ccmp(ty,"hbox")) {
                widget->setOrientation(Qt::Horizontal);
            }
        }
        /*
        qApp->setStyleSheet("QSplitter::handle { image: url(images/splitter.png); }\
QSplitter::handle:horizontal { width: 2px; }\
QSplitter::handle:vertical { height: 2px;  }\
QSplitter::handle:pressed { url(images/splitter_pressed.png); }");
        */

        // qApp->setStyleSheet("QSplitter::handle{background-color: black;}");
        cf->xstat=WTYPE_SPLITTER;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    } else if( ccmp(tag,"progress") ) {
        GProgress* widget = new GProgress(cf, parent);
        cf->xstat=WTYPE_PROGRESS;
        cf->GetVar("@this")->setVar('w',0,(LPVOID)widget);
    }
    QWidget* widget=gwidget(cf);
    if( widget ) {
        StrVar* sv=cf->gv("@source");
        initWidget(cf, widget);
        if( sv ) {
            StrVar* rst=getStrVarTemp();
            XParseVar pv(sv);
            LPCC varCode=NULL;
            for(int n=0;n<256&&pv.valid(); n++ ) {
                pv.findEnd("##>",FLAG_SKIP);
                if( n==0 ) {
                    setModifyClassFunc(cf, sv, pv.prev, pv.cur, NULL, rst, false, "func");
                } else if(slen(varCode)) {
                    cf->GetVar(FMT("@%s",varCode))->set(sv, pv.prev, pv.cur);
                }
                if(pv.ch()==0) break;
                pv.findEnd("\n");
                varCode=pv.Trim(pv.prev, pv.cur);
            }
        }
        if( !cf->isNodeFlag(FLAG_INIT) ) {
            callInitFunc(cf);
            // qDebug("...createWidget call init func...");
        }
    }
    return widget;
}

bool createWidgetChild(TreeNode* root, QWidget* parent, int depth) {
    if( root==NULL ) return false;
    bool rtn = true;
#ifdef PAGE_NODE_USE
    if( isPageNode(root) ) {
        bool childCheck=true;
        if( root->childCount()==1 ) {
            TreeNode* cur=root->child(0);
            LPCC tag=cur->get("tag");
            if( ccmp(tag,"hbox")|| ccmp(tag,"vbox") || ccmp(tag,"form") ) {
                StrVar* var=root->gv("margin");
                if( var ) {
                    cur->GetVar("margin")->reuse()->add(var);
                }
                var=root->gv("spacing");
                if( var ) {
                    cur->GetVar("spacing")->reuse()->add(var);
                }
                childCheck=false;
            }
        }
        if( childCheck ) {
            TreeNode* cf = new TreeNode(2, true);
            LPCC tag=root->get("type");
            if( slen(tag)==0 ) tag="vbox";
            if( ccmp(tag,"vbox") ) {
                cf->set("tag","vbox");
                cf->xstat=LTYPE_VBOX;
            } else {
                cf->set("tag","hbox");
                cf->xstat=LTYPE_HBOX;
            }
            StrVar* var=root->gv("margin");
            if( var ) {
                cf->GetVar("margin")->reuse()->add(var);
            }
            var=root->gv("spacing");
            if( var ) {
                cf->GetVar("spacing")->reuse()->add(var);
            }
            for( int n=0,cnt=root->childCount();n<cnt;n++ ) {
                cf->addChild(root->child(n));
            }
            root->xchilds.reuse();
            root->addChild(cf);
        }
    }
#endif
    for( int n=0,cnt=root->childCount();n<cnt;n++ ) {
        TreeNode* cf=root->child(n);
        QLayout* layout=NULL;
        QWidget* widget=NULL;
        LPCC tag=cf->get("tag");
        if( ccmp(tag,"hbox")|| ccmp(tag,"vbox") || ccmp(tag,"form") ) {
            layout=createLayout(cf, parent);
        } else if(ccmp(tag,"row") ) {
            // cf->setNodeFlag(GFLAG_LAYOUT);
            cf->xstat=LTYPE_ROW;
            TreeNode* form=findLayoutNode(cf);
            // qDebug("xxxxx form tag:%s xxxxxxx", form->get("tag"));
            if( form ) {
                form->incrNum("@r");
                form->setInt("@c",0);
            }
            createWidgetChild(cf, parent, depth+1);
            continue;
        } else {
            widget=createWidget(tag, cf, parent);
        }
        if( layout || widget ) {
            TreeNode* pnd = findParentNode(cf);
            if( pnd==NULL ) {
                qDebug("#9#widget create child error %s", cf->log() );
                break;
            }
            // qDebug("... pnd: %s ...", pnd->log() );
            if( isLayoutNode(pnd) ) {
                switch(pnd->xstat) {
                case LTYPE_GRID: {
                    QGridLayout* grid = qobject_cast<QGridLayout*>(getLayout(pnd));
                    if( grid ) {
                        if( cf->xstat==WTYPE_DIV ) {
                            grid->setContentsMargins(0,0,0,0);
                            if( widget ) {
                                widget->setContentsMargins(0,0,0,0);
                            }
                        }
                        int r=pnd->getInt("@r");
                        int c=pnd->getInt("@c");
                        int rs=cf->getInt("rowspan",1);
                        int cs=cf->getInt("colspan",1);
                        if( layout ) {
                            grid->addLayout( layout, r, c, rs, cs  );
                        } else {
                            grid->addWidget( widget, r, c, rs, cs );
                        }
                        pnd->incrNum("@c", cs);
                        if( c==0 ) {
                            TreeNode* row=cf->parent();
                            int stretch=row->getInt("stretch");
                            if( stretch>0 ) grid->setRowStretch(r,stretch);
                        }
                    }
                } break;
                case LTYPE_HBOX:
                case LTYPE_VBOX: {
                    QBoxLayout* box = qobject_cast<QBoxLayout*>(getLayout(pnd));
                    if( box ) {
                        if( cf->xstat==WTYPE_DIV ) {
                            box->setContentsMargins(0,0,0,0);
                            box->setSpacing(0);
                            if( widget ) {
                                widget->setContentsMargins(0,0,0,0);
                            }
                        }
                        if( layout ) {
                            box->addLayout(layout, cf->getInt("stretch"));
                        } else {
                            box->addWidget(widget, cf->getInt("stretch"));
                        }
                    }
                } break;

                }
            } else {
                QWidget* pwd=getNodeWidget(pnd);
                switch(pnd->xstat) {
                case WTYPE_DIALOG:
                case WTYPE_PAGE:
                case WTYPE_MAIN: {
                    // qDebug("xxxxx create later tag: %s xxxxx", pnd->get("tag"));
                } break;
                case WTYPE_TAB: {
                    GTab* gwd = qobject_cast<GTab*>(pwd);
                    if( gwd ) {
                        LPCC title=cf->get("title");
                        gwd->addTab(widget,KR(title));
                    }
                } break;
                case WTYPE_STACKED: {
                    GStacked* gwd = qobject_cast<GStacked*>(pwd);
                    if( gwd ) {
                        gwd->addWidget(widget);
                    }
                } break;
                case WTYPE_SPLITTER: {
                    GSplitter* gwd = qobject_cast<GSplitter*>(pwd);
                    if( gwd ) {
                        gwd->addWidget(widget);
                    }
                } break;
                case WTYPE_GROUP: {
                    GGroup* gwd = qobject_cast<GGroup*>(pwd);
                    if( layout && gwd ) {
                        gwd->setLayout(layout);
                    }
                } break;
                default: {
                    rtn = false;
                }break;
                }
            }
        } else {
            qDebug("#9#widget parent error %s", cf->log() );
            break;
        }
        createWidgetChild(cf, widget, depth+1);
    }
    return rtn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  widget class
//
//////////////////////////////////////////////////////////////////////////////////////////////////
GWidget::GWidget(TreeNode* cf, QWidget* p) : QWidget(p), xcf(cf),
	fade_effect(this),
	animation(&fade_effect, "opacity")
{
	if(cf->cmp("type","overlay") ) {
		// setPalette(Qt::transparent);
		// setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground, true);
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_NoBackground);
		setStyleSheet("background:transparent;");
		if (parent()) {
			parent()->installEventFilter(this);
			raise();
		}
		setVisible(false);
		setEnabled(false);
	}
}
bool GWidget::openOverlay(int duration) {
	if(!parent()) return false;
	setGraphicsEffect(&fade_effect);
	animation.setStartValue(0.0);
	animation.setEndValue(1.0);
	animation.setEasingCurve(QEasingCurve::InOutQuint);
	animation.setDuration(duration);
	setVisible(false);
	setEnabled(false);
	return true;
}
void GWidget::showOverlay(const bool& show)
{
	animation.stop();
	animation.setStartValue(animation.currentValue());
	animation.setEndValue(show ? 1.0 : 0.0); // show or hide
	animation.start();
	if (show) {
		setVisible(true);
		setEnabled(true);
		setAttribute(Qt::WA_TransparentForMouseEvents, false);
	}
	else {
		setVisible(false);
		setEnabled(false);
		setAttribute(Qt::WA_TransparentForMouseEvents);
	}
}
void GWidget::newParent()
{
	if (!parent()) return;
	parent()->installEventFilter(this);
	raise();
}

bool GWidget::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == parent()) {
		if (ev->type() == QEvent::Resize) {
			// resize(static_cast<QResizeEvent*>(ev)->size());
			raise();
		} else if (ev->type() == QEvent::ChildAdded) {
			raise();
		}
	}
	return QWidget::eventFilter(obj, ev);
}

GContext::GContext(TreeNode* cf, QWidget* p) : QOpenGLWidget(p), xcf(cf),
	fade_effect(this),
	animation(&fade_effect, "opacity")
{
	if(cf->cmp("type","overlay") ) {
		// setPalette(Qt::transparent);
		// setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_TranslucentBackground, true);
		setAttribute(Qt::WA_NoSystemBackground);
		setAttribute(Qt::WA_OpaquePaintEvent);
		setAttribute(Qt::WA_NoBackground);
		setStyleSheet("background:transparent;");
		if (parent()) {
			parent()->installEventFilter(this);
			raise();
		}
		setVisible(false);
		setEnabled(false);
    } else {
        setAutoFillBackground(cf->cmp("mode","autofill") );
	}
    _contextWidth=0;
    _contextHeight=0;
}
void GContext::startTimer(int delay) {
    _interval=delay;
    if(_qTimer.isActive() ) {
        _qTimer.stop();
    }
    _qTimer.setInterval(delay);
    _qTimer.start();
    QObject::connect(&_qTimer, &QTimer::timeout, this, &GContext::timeout);
}
void GContext::timeout() {
    XFuncNode* fn = getEventFuncNode(xcf, "onTimeout");
    if(fn) {
        fn->call();
    } else {
        update();
    }
}
void GContext::initializeGL() {
    startTimer(500);
}
void GContext::resizeGL(int w, int h) {
    _contextWidth=w;
    _contextHeight=h;
    qDebug("resizeGL %d %d", w, h);
}

void GContext::paintGL() {
    QPainter painter;
    if( painter.begin(this) && painter.isActive() ) {
        // param: dc, rc
        XFuncNode* fn = getEventFuncNode(xcf, "onDraw");
        if(fn) {
            PARR arrs=getLocalParams(fn);
            TreeNode* d=xcf->addNode("@drawObject");
            StrVar* sv=d->GetVar("@rect");
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            d->GetVar("@g")->setVar('g',0,(LPVOID)&painter);
            setVarRectF(sv, QRect(0,0,_contextWidth,_contextHeight) );
            arrs->add()->setVar('n',0,(LPVOID)d);
            arrs->add()->add(sv);
            setFuncNodeParams(fn, arrs);
            fn->call();
            painter.end();
        }
    }
}

bool GContext::openOverlay(int duration) {
	if(!parent()) return false;
	setGraphicsEffect(&fade_effect);
	animation.setStartValue(0.0);
	animation.setEndValue(1.0);
	animation.setEasingCurve(QEasingCurve::InOutQuint);
	animation.setDuration(duration);
	setVisible(false);
	setEnabled(false);
	return true;
}
void GContext::showOverlay(const bool& show)
{
	animation.stop();
	animation.setStartValue(animation.currentValue());
	animation.setEndValue(show ? 1.0 : 0.0); // show or hide
	animation.start();
	if (show) {
		setVisible(true);
		setEnabled(true);
		setAttribute(Qt::WA_TransparentForMouseEvents, false);
	}
	else {
		setVisible(false);
		setEnabled(false);
		setAttribute(Qt::WA_TransparentForMouseEvents);
	}
}
void GContext::newParent()
{
	if (!parent()) return;
	parent()->installEventFilter(this);
	raise();
}

bool GContext::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            // resize(static_cast<QResizeEvent*>(ev)->size());
            raise();
        } else if (ev->type() == QEvent::ChildAdded) {
            raise();
        }
    }
    return QOpenGLWidget::eventFilter(obj, ev);
}

class GDateEdit::Private {
public:
    Private( GDateEdit* qq ) : q(qq),  clearButton(0), null(false), nullable(false) {}

    GDateEdit* const q;
    QPushButton* clearButton;

    bool null;
    bool nullable;

    void setNull(bool n) {
        null = n;
        /*
        if (null) {
            QLineEdit *edit = qFindChild<QLineEdit *>(q, "qt_spinbox_lineedit");
            if (!edit->text().isEmpty()) {
                edit->clear();
            }
        }
        */
        if (nullable) {
            clearButton->setVisible(!null);
        }

    }
};
GDateEdit::GDateEdit(TreeNode* cf, QWidget* p) : QDateEdit(p), xcf(cf), d(new Private(this)) {
}


QDateTime GDateEdit::dateTime() const {
    if (d->nullable && d->null) {
        return QDateTime();
    } else {
        return QDateEdit::dateTime();
    }
}

QDate GDateEdit::date() const {
    if (d->nullable && d->null) {
        return QDate();
    } else {
        return QDateEdit::date();
    }
}

QTime GDateEdit::time() const {
    if (d->nullable && d->null) {
        return QTime();
    } else {
        return QDateEdit::time();
    }
}

void GDateEdit::setDateTime(const QDateTime &dateTime) {
    if (d->nullable && !dateTime.isValid()) {
        d->setNull(true);
    } else {
        d->setNull(false);
        QDateEdit::setDateTime(dateTime);
    }
}

void GDateEdit::setDate(const QDate &date) {
    if (d->nullable && !date.isValid()) {
        d->setNull(true);
    } else {
        d->setNull(false);
        QDateEdit::setDate(date);
    }
}

void GDateEdit::setTime(const QTime &time) {
    if (d->nullable && !time.isValid()) {
        d->setNull(true);
    } else {
        d->setNull(false);
        QDateEdit::setTime(time);
    }
}

bool GDateEdit::isNullable() const {
    return d->nullable;
}

void GDateEdit::setNullable(bool enable) {
    d->nullable = enable;

    if (enable && !d->clearButton) {
        d->clearButton = new QPushButton(this);
        d->clearButton->setFlat(true);
        d->clearButton->setIcon(QIcon(":/images/edit-clear-locationbar-rtl.png"));
        d->clearButton->setFocusPolicy(Qt::NoFocus);
        d->clearButton->setFixedSize(17, d->clearButton->sizeHint().height()-6);
        connect(d->clearButton,SIGNAL(clicked()),this,SLOT(clearButtonClicked()));
        d->clearButton->setVisible(!d->null);
    } else if (d->clearButton) {
        disconnect(d->clearButton,SIGNAL(clicked()),this,SLOT(clearButtonClicked()));
        delete d->clearButton;
        d->clearButton = 0;
    }
    update();
}

QSize GDateEdit::sizeHint() const {
    const QSize sz = QDateEdit::sizeHint();
    if (!d->clearButton)
        return sz;
    return QSize(sz.width() + d->clearButton->width() + 3, sz.height());
}

QSize GDateEdit::minimumSizeHint() const {
    const QSize sz = QDateEdit::minimumSizeHint();
    if (!d->clearButton)
        return sz;
    return QSize(sz.width() + d->clearButton->width() + 3, sz.height());
}

void GDateEdit::showEvent(QShowEvent *event) {
    QDateEdit::showEvent(event);
    d->setNull(d->null); // force empty string back in
}

void GDateEdit::resizeEvent(QResizeEvent *event) {
    if (d->clearButton) {
        QStyleOptionSpinBox opt;
        initStyleOption(&opt);
        opt.subControls = QStyle::SC_SpinBoxUp;

        int left = style()->subControlRect(QStyle::CC_SpinBox, &opt, QStyle::SC_SpinBoxUp, this).left() - d->clearButton->width() - 3;
        d->clearButton->move(left, (height() - d->clearButton->height()) / 2);
    }

    QDateEdit::resizeEvent(event);
}

void GDateEdit::paintEvent(QPaintEvent *event) {
    QDateEdit::paintEvent(event);
}
void GDateEdit::keyPressEvent(QKeyEvent *event) {
    if (d->nullable &&
        (event->key() >= Qt::Key_0) &&
        (event->key() <= Qt::Key_9) &&
        d->null) {
            setDateTime(QDateTime::currentDateTime());
        }
    if (event->key() == Qt::Key_Tab && d->nullable && d->null) {
        QAbstractSpinBox::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Backspace && d->nullable){
        /*
        QLineEdit *edit = qFindChild<QLineEdit *>(this, "qt_spinbox_lineedit");
        if (edit->selectedText() == edit->text()) {
            setDateTime(QDateTime());
            event->accept();
            return;
        }
        */
    }

    QDateEdit::keyPressEvent(event);
}

void GDateEdit::mousePressEvent(QMouseEvent *event) {
    bool saveNull = d->null;
    QDateEdit::mousePressEvent(event);
    if (d->nullable && saveNull && calendarWidget()->isVisible()) {
        setDateTime(QDateTime::currentDateTime());
    }
}

bool GDateEdit::focusNextPrevChild(bool next) {
    if (d->nullable && d->null){
        return QAbstractSpinBox::focusNextPrevChild(next);
    } else {
        return QDateEdit::focusNextPrevChild(next);
    }
}

QValidator::State GDateEdit::validate(QString &input, int &pos) const {
    if (d->nullable && d->null){
        return QValidator::Acceptable;
    }
    return QDateEdit::validate(input, pos);
}

void GDateEdit::clearButtonClicked() {
    d->setNull(true);
}



GInput::GInput(TreeNode* cf, QWidget* p) : QLineEdit(p), xcf(cf) {
    connect(this, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(slotCursorPositionChanged(int,int)) );
    connect(this, SIGNAL(editingFinished()), this, SLOT(slotEditingFinished()) );
    connect(this, SIGNAL(returnPressed()), this, SLOT(slotReturnPressed()) );
    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)) );
    connect(this, SIGNAL(textEdited(QString)), this, SLOT(slotTextEdited(QString)) );
}
void	GInput::slotCursorPositionChanged ( int prev, int cur )							{ uom.onInputPositionChanged(config(),prev,cur); }
void	GInput::slotEditingFinished ()													{ uom.onEditingFinished(config()); }
void	GInput::slotReturnPressed ()													{ uom.onReturnPressed(config()); }
void	GInput::slotSelectionChanged ()													{ uom.onSelectionChanged(config()); }
void	GInput::slotTextChanged ( const QString & text )								{ uom.onTextChanged(config(),text); }
void	GInput::slotTextEdited ( const QString & text )									{ }

/************************************************************************/
/*   TextEdit                                                     */
/************************************************************************/

inline int getFirstVisibleBlockId(QTextEdit* editor) {
    QWidget* viewPort=editor->viewport();
    QTextDocument* doc=editor->document();
    QTextCursor curs = QTextCursor(doc);
    QRect r1 = viewPort->geometry();
    int sp=editor->verticalScrollBar()->sliderPosition();
    curs.movePosition(QTextCursor::Start);
    for(int i=0; i < doc->blockCount(); ++i) {
        QTextBlock block = curs.block();
        QRect r2 = doc->documentLayout()->blockBoundingRect(block).translated( r1.x(), r1.y() - sp ).toRect();
        if (r1.contains(r2, true)) { return i; }
        curs.movePosition(QTextCursor::NextBlock);
    }
    return 0;
}
inline QTextBlock editorOffsetBlock(QTextEdit* editor, QPoint& offset) {
    // last visible block offset QPoint(0, this->viewport().height() - 1)
    return editor->cursorForPosition(offset).block();
}


inline void editorTabIndent(QTextDocument *doc, QTextCursor& cursor, bool doIndent) {
    cursor.beginEditBlock();
    // Indent or unindent the selected lines
    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos), end = qMax(anchor, pos);

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end-1).next();

    for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
        QString text = block.text();
        if( doIndent) {
            int indentPosition = firstNonSpace(text);
            cursor.setPosition(block.position() + indentPosition);
            cursor.insertText(QString(1, QLatin1Char('\t')));
        } else {
            if( text.length()==0 )
                continue;
            int from = 0;
            if( text.at(0)=='\t') {
                from = 1;
            } else if( text.length()>3 ) {
                for( ; from<4; from++) {
                    if( text.at(from)=='\t' || text.at(from)!=' ' ) break;
                }
            }
            if( from>0 ) {
                cursor.setPosition(block.position() + from);
                cursor.setPosition(block.position(), QTextCursor::KeepAnchor);
                cursor.removeSelectedText();
            }
        }
    }

    // Reselect the selected lines
    cursor.setPosition(startBlock.position());
    cursor.setPosition(endBlock.previous().position(), QTextCursor::KeepAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    cursor.endEditBlock();
}

GTextEdit::GTextEdit(TreeNode* cf, QWidget* p) : QTextEdit(p), xcf(cf), xscrollLock(false) {
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(textCurrentCharFormatChanged(QTextCharFormat)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(textCursorPositionChanged()));
    connect(this->document(), SIGNAL(modificationChanged(bool)), this, SLOT(textModificationChanged(bool)));

    xcf->setNodeFlag(NF_TABUSE);			// enterKeyUse
    xcf->setNodeFlag(NF_LINENUM);

    setCursorWidth(2);
    setLineWrapMode(QTextEdit::NoWrap);
    // setMouseTracking(true);
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setLayoutDirection(Qt::LeftToRight);
    setTabStopWidth(16);
    setTabChangesFocus(true);

    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(new GViewEventFilter(viewport(),xcf));
    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollValueChange(int)));
    connect(this, SIGNAL(undoAvailable( bool)), this, SLOT(slotUndoAvailable( bool)) );
    connect(this, SIGNAL(copyAvailable( bool)), this, SLOT(slotCopyAvailable( bool)) );
    connect(this, SIGNAL(redoAvailable( bool)), this, SLOT(slotRedoAvailable( bool)) );
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(textCurrentCharFormatChanged(QTextCharFormat)) );

    connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect(this, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    // connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(slotModificationChanged(bool)));
    // QObject::connect(document(), SIGNAL(contentsChanged()), this, SLOT(editorContentsChanged()), Qt::DirectConnection);
    QObject::connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(textContentsChange(int,int,int)), Qt::DirectConnection);
    QObject::connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    QObject::connect(this, SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));

    xlinenum = new TextLineNumberArea(this);
    xsyntax = new XHighlighter(document(),xcf);
    updateLineNumberAreaWidth(0);

}

void GTextEdit::textCurrentCharFormatChanged(const QTextCharFormat &format) {
    XFuncNode* fn = getEventFuncNode(config(), "onFormatChange");
    if( fn ) {
        fn->call();
    }
}
void GTextEdit::textCursorPositionChanged () {
    StrVar* sv = config()->gv("@mark");
    if( SVCHK('a',0) ) {
        XListArr* a=(XListArr*)SVO;
        if( a->size() ) {
            UL64 tick=toUL64( config()->gv("@markTick") );
            if( (GetTickCount()-tick) > 500 ) {
                a->reuse();
            }
        }
    }
    sv = config()->gv("@tapPositions");
    if( SVCHK('a',0) ) {
        XListArr* arr = (XListArr*)SVO;
        int size = arr->size();
        if( size>0 ) {
            bool chk=false;
            QTextCursor c = textCursor();
            int pos = c.position();
            sv=arr->get(0);
            if( SVCHK('i',2) ) {
                int sp=sv->getInt(4), ep=sv->getInt(8)+1;
                if( pos<sp ) {
                    chk=true;
                } else if( size==1 && ep<pos ) {
                    chk=true;
                }
                if( size>1 && chk==false ) {
                    sv=arr->get(size-1);
                    if( SVCHK('i',2) ) {
                        int ep=sv->getInt(8);
                        if( ep<=pos ) {
                            chk=true;
                        }
                    }
                }
            }
            if( chk ) {
                // qDebug("#0#### tapPositions reuse ####\n");
                arr->reuse();
            }
        }
    }

    XFuncNode* fn = getEventFuncNode(config(), "onCursorChange");
    if( fn ) {
        fn->call();
        // xfnCursorChange->call();
    }

    viewport()->update();
    updateLineNumberArea(contentsRect(), 0);
}


void GTextEdit::textClipboardDataChanged() {
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if( md->hasText() ) {
        qDebug("clipboardDataChanged => text");
    }
}

void GTextEdit::textModificationChanged(bool modify) {
    XFuncNode* fn = getEventFuncNode(config(), "onModificationChanged");
    if( fn ) {
        fn->call();
    }
}
void GTextEdit::textContentsChange(int pos,int removePos,int addPos) {
    StrVar* sv = config()->gv("@finds");
    if( SVCHK('a',0) ) {
        XListArr* a=(XListArr*)SVO;
        if( a->size() ) a->reuse();
    }
    XFuncNode* fnCur = getEventFuncNode(config(), "onContentChange");
    if( fnCur ) {
        PARR arrs=getLocalParams(fnCur);
        arrs->add()->setVar('0',0).addInt(pos);
        arrs->add()->setVar('0',0).addInt(addPos);
        arrs->add()->setVar('0',0).addInt(removePos);
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
    }
}

inline void drawEditorMark(QPainter& painter, XListArr* a, QTextBlock& block, QTextCursor& tc, int bottom, int top, QColor& color) {
    if( a==NULL ) return;
    QTextLayout *layout = block.layout();
    StrVar* sv=NULL;

    for( int n=0,sz=a->size(); n<sz; n++ ) {
        sv = a->get(n);
        if( SVCHK('i',2) ) {
            int sp=sv->getInt(4), ep=sv->getInt(8);
            int bsp=block.position();
            bool beq=false;
            if( bsp<=ep ) {
                if( sp==ep ) {
                    beq=true;
                    ep+=1;
                }
                tc.setPosition(bsp);
                tc.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
                int bep=tc.position(), pos1=0, pos2=0;
                bool ok = true;
                if( bsp<=sp && sp<bep) {
                    pos1=sp-bsp, pos2=ep< bep ? ep-bsp : bep-bsp;
                } else if( bsp>=sp && bsp<ep ) {
                    pos1=0, pos2=ep< bep ? ep-bsp : bep-bsp;
                } else {
                    ok = false;
                }
                if( ok && (pos1< pos2) ) {
                    int hei=bottom-top-1;
                    qreal x1 = layout->lineForTextPosition(pos1).cursorToX(pos1);
                    qreal y = top + 2;
                    qreal x2 = layout->lineForTextPosition(pos2).cursorToX(pos2);
                    x1+=5, x2+=5;
                    if( beq ) {
                        painter.fillRect(QRectF(x1-3,y,4,hei), color);
                    } else {
                        painter.fillRect(QRectF(x1,y,x2-x1,hei), color);
                    }
                }
            }
        }
    }
}

void GTextEdit::paintEvent(QPaintEvent *event) {
    // begin QPlainTextEdit::paintEvent() copy
    QPainter painter(viewport());
    XListArr* a = NULL;
    XListArr* ta = NULL;
    StrVar* sv = config()->gv("@mark");
    if( SVCHK('a',0) ) {
        a = (XListArr*)SVO;
        if( a->size()==0 ) {
            a=NULL;
        }
    }
    if( a==NULL ) {
        sv = config()->gv("@finds");
        if( SVCHK('a',0) ) {
            a = (XListArr*)SVO;
            if( a->size()==0 ) {
                a=NULL;
            }
        }
    }
    sv=config()->gv("@tapPositions");
    if( SVCHK('a',0) ) {
        ta=(XListArr*)SVO;
        if( ta->size()<3 ) {
            ta=NULL;
        }
    }
    XFuncNode* fnCur = getEventFuncNode(config(), "onDraw");
    if( fnCur ) {
        TreeNode* d=NULL;
        sv=fnCur->GetVar("@draw");
        if( SVCHK('n',0) ) {
            d=(TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            sv->setVar('n',0,(LPVOID)d);
        }
        PARR arrs=getLocalParams(fnCur);
        arrs->add()->setVar('n',0,(LPVOID)d);
        setVarRectF(d->GetVar("@rect"), event->rect() );
        d->GetVar("@g")->setVar('g',0,(LPVOID)&painter);
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
    }
    if( a==NULL && ta==NULL ) {
        QTextEdit::paintEvent(event);
        return;
    }
    QColor color(200,200,80,150);
    QColor colorTab(220,220,220,150);
    QTextBlock block= cursorForPosition(QPoint(0, 1)).block();
    if( !block.isValid() ) {
        QTextEdit::paintEvent(event);
        return;
    }
    QRect rect = event->rect();
    QRect rc=getBlockRect(block).toRect();
    if( rc.y()<0 ) {
        block=block.next();
    }
    rc=getBlockRect(block).toRect();
    int top=rc.y(), bottom=top+rc.height();
    int prevY=-1, checkNum=0;
    QTextCursor tc = textCursor();
    while( block.isValid() && top <= rect.bottom()) {
        int blockNumber=block.blockNumber();
        if( prevY==rc.y() ) {
            block = block.next();
            rc=getBlockRect(block).toRect();
            if( prevY!=rc.y() ) {
                top=rc.y();
                bottom=top+rc.height();
            }
            continue;
        }
        if( block.isVisible() && bottom >= rect.top()) {
            if( a ) drawEditorMark(painter, a, block, tc, bottom, top, color);
            if( ta ) drawEditorMark(painter, ta, block, tc, bottom, top, colorTab );
        }
        prevY=rc.y();
        block = block.next();
        rc=getBlockRect(block).toRect();
        top = bottom;
        bottom = top + rc.height();
    }
    QTextEdit::paintEvent(event);
}


void GTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) {
    if( !xcf->isNodeFlag(NF_LINENUM) ) return;
    QTextDocument* doc=this->document();
    QPainter painter(xlinenum);
    XFuncNode* fnCur = getEventFuncNode(config(), "onLineNumberDraw");
    TreeNode* d=NULL;
    StrVar* sv=NULL;
    if( fnCur ) {
        sv=fnCur->GetVar("@draw");
        if( SVCHK('n',0) ) {
            d=(TreeNode*)SVO;
            d->GetVar("@c")->setVar('w',0,(LPVOID)this);
        } else {
            d=new TreeNode(2, true);
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            sv->setVar('n',0,(LPVOID)d);
        }        
        setVarRectF(d->GetVar("@rect"), event->rect() );
        d->GetVar("@g")->setVar('g',0,(LPVOID)&painter);
        config()->GetVar("@prevLineNum")->reuse();        
    }
    QRect rect = event->rect();
    painter.fillRect(rect, QColor(220,220,220,220) ); // Qt::lightGray);
    painter.setPen(Qt::darkGray);
    painter.setFont(font());

    QTextBlock block= cursorForPosition(QPoint(0, 1)).block();
    if( block.isValid() ) {
        QRect rc=getBlockRect(block).toRect();
        if( rc.y()<0 ) {
            block=block.next();
        }
        rc=getBlockRect(block).toRect();
        int top=rc.y(), bottom=top+rc.height();
        int prevY=-1, checkNum=0;
        while( block.isValid() && top <= rect.bottom()) {
            int blockNumber=block.blockNumber();
            if( prevY==rc.y() ) {
                // if( bottom < (top + rc.height()) ) bottom =top + rc.height();
                // qDebug("lineNumDraw top=%d, bottom=%d, (block num:%d)\n", top, bottom, blockNumber);
                block = block.next();
                rc=getBlockRect(block).toRect();
                if( prevY!=rc.y() ) {
                    top=rc.y();
                    bottom=top+rc.height();
                }
                continue;
            }
            if( block.isVisible() && bottom >= rect.top()) {
                if( fnCur && d ) {
                    // ex) d, rcNum, startPos, endPos, lineNumber
                    int hei=bottom-top;
                    QRect rcNumber(0, top, xlinenum->width(), hei);
                    int bsp=block.position(), bep=bsp+block.length();
                    PARR arrs=getLocalParams(fnCur);
                    arrs->add()->setVar('n',0,(LPVOID)d);
                    setVarRectF(arrs->add(), rcNumber);
                    arrs->add()->setVar('0',0).addInt(bsp);
                    arrs->add()->setVar('0',0).addInt(bep);
                    arrs->add()->setVar('0',0).addInt(blockNumber + 1);
                    setFuncNodeParams(fnCur, arrs);
                    fnCur->call();
                } else {
                    QString number = QString::number(blockNumber + 1);
                    painter.drawText(0, top+2, xlinenum->width()-2, bottom-top, Qt::AlignRight, number);
                }
            }
            prevY=rc.y();
            block = block.next();
            rc=getBlockRect(block).toRect();
            top = bottom;
            bottom = top + rc.height();
        }
    }
}

void GTextEdit::zoomIn(int range) {
    QFont f = font();
    const int newSize = f.pointSize() + range;
    if (newSize <= 0)
        return;
    f.setPointSize(newSize);
    setFont(f);

}
void GTextEdit::zoomOut(int range) {
   zoomIn(-range);
}

QRectF& GTextEdit::getBlockRect(QTextBlock& block) {
    QTextDocument* doc=this->document();
    QWidget* viewPort=this->viewport();
    mBlockRect = doc->documentLayout()->blockBoundingRect(block).translated(
            viewPort->geometry().x(), viewPort->geometry().y() - (this->verticalScrollBar()->sliderPosition()) );
    return mBlockRect;
}
QPointF& GTextEdit::getContentOffset() {
    return mOffsetPt;
}
QTextBlock* GTextEdit::getLineBlock(int sp, int ep) {
    mStartBlock=document()->findBlockByNumber(sp);
    if( ep<sp ) {
        mEndBlock=document()->lastBlock();
    } else {
        mEndBlock=document()->findBlockByNumber(ep);
    }
    return &mStartBlock;
}
QTextBlock* GTextEdit::getFirstBlock(int ep) {
    QTextDocument* doc=this->document();
    int blockNumber = getFirstVisibleBlockId(this);
    mStartBlock = doc->findBlockByNumber(blockNumber);
    if( ep==-1 ) {
        QRect geo=this->viewport()->geometry();
        QTextBlock block=mStartBlock;
        QTextBlock prev_block = (blockNumber > 0) ? doc->findBlockByNumber(blockNumber-1) : block;
        int translate_y = (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;
        int top = geo.top();
        int additional_margin=0;
        if (blockNumber == 0)
            additional_margin = (int)doc->documentMargin() -1 - this->verticalScrollBar()->sliderPosition();
        /*
        else
            additional_margin = (int)doc->documentLayout()->blockBoundingRect(prev_block).translated(0, translate_y).intersect(geo).height();
        */
        top += additional_margin;
        int bottom = top + (int) doc->documentLayout()->blockBoundingRect(block).height();
        // Draw the numbers (displaying the current line number in green)
        while( block.isValid() && top <=this->rect().bottom()) {
            block = block.next();
            top = bottom;
            bottom = top + (int) doc->documentLayout()->blockBoundingRect(block).height();
        }
        mEndBlock=block;
    } else {
        mEndBlock=document()->findBlock(ep);
    }
    return &mStartBlock;
}
QTextBlock* GTextEdit::getStartBlock(int sp, int ep) {
    mStartBlock=document()->findBlock(sp);
    if( ep==sp ) {
        mEndBlock=mStartBlock;
    } else if( ep<sp ) {
        mEndBlock=document()->lastBlock();
    } else {
        mEndBlock=document()->findBlock(ep);
    }
    return &mStartBlock;
}
QTextBlock* GTextEdit::nextBlock(QTextBlock* block) {
    if( block==NULL ) return NULL;
    if( mStartBlock==mEndBlock ) return NULL;
    mStartBlock=block->next();
    return mStartBlock.isValid() ? &mStartBlock: NULL;
}

QTextFragment* GTextEdit::getFragment(int pos) {
    if( pos==-1 ) {
        if( !mStartBlock.isValid() ) {
            return NULL;
        }
        mBlockIterator=mStartBlock.begin();
        if( mBlockIterator.atEnd() )
            return NULL;
        mNextFragment=mBlockIterator.fragment();
    } else {
        mStartBlock=document()->findBlock(pos);
        bool find=false;
        if( mStartBlock.isValid() ) {
            mBlockIterator=mStartBlock.begin();
            for(; !mBlockIterator.atEnd(); mBlockIterator++ ) {
                QTextFragment fragment=mBlockIterator.fragment();
                if( fragment.contains(pos) ) {
                    find=true;
                    mNextFragment=fragment;
                    break;
                }
            }
        }
        if( find==false ) return NULL;
    }
    return &mNextFragment;
}

QTextFragment* GTextEdit::nextFragment() {
    if( mBlockIterator.atEnd() ) return NULL;
    mBlockIterator++;
    if( mBlockIterator.atEnd() ) {
        return NULL;
    }
    mNextFragment=mBlockIterator.fragment();
    return &mNextFragment;
}

void GTextEdit::setScrollPosition(int ypos, int xpos) {
    this->verticalScrollBar()->setSliderPosition(ypos);
    if( xpos!=-1 ) {
        this->horizontalScrollBar()->setSliderPosition(xpos);
    }

}
void GTextEdit::setFrameMargin(int left, int top, int right, int bottom) {
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QTextFrame* root = this->textCursor().currentFrame();
    if( root ) {
        QTextFrameFormat format =  root->frameFormat();
        if( left!=-1 ) format.setLeftMargin(left);
        if( top!=-1 ) format.setTopMargin(top);
        if( right!=-1 ) format.setRightMargin(right);
        if( bottom!=-1 ) format.setBottomMargin(bottom);
        root->setFrameFormat(format);
    }
}

void GTextEdit::blockScrollTop(QTextBlock block) {
    QTextDocument* doc=this->document();
    QRectF rc = doc->documentLayout()->blockBoundingRect(block);
    this->setScrollPosition((int)rc.y(), (int)rc.x());
}

void GTextEdit::scrollContentsBy(int dx, int dy) {
    if( xscrollLock  ) {
        return;
    }
    QTextEdit::scrollContentsBy(dx, dy);
}


void	GTextEdit::slotTextChanged () {
    XFuncNode* fnCur = getEventFuncNode(config(), "onChange");
    if( fnCur ) {
        fnCur->call();
        // version 1.0.2
        StrVar* sv=config()->gv("@mp"); if( sv ) sv->reuse();
        sv=config()->gv("@fp"); if( sv ) sv->reuse();
    }
    updateLineNumberArea(contentsRect(), 0);
    // xfnTextChange->call();
}

void	GTextEdit::slotUndoAvailable ( bool available ) { }
void	GTextEdit::slotCopyAvailable ( bool yes ) { }
void	GTextEdit::slotRedoAvailable ( bool available ) { }


void GTextEdit::scrollValueChange(int dy) {
    updateLineNumberArea(contentsRect(), dy);
}

void GTextEdit::updateLineNumberAreaWidth( int newBlockCount ) {
    if( xcf->isNodeFlag(NF_LINENUM) ) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

void GTextEdit::updateLineNumberArea( const QRect &rect, int dy ) {
    /*
    if( dy )
        xlinenum->scroll(0, dy);
    else
        xlinenum->update(0, rect.y(), xlinenum->width(), rect.height());

    if( rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
    */
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());
    // QRect rect =  this->contentsRect();
    xlinenum->update(0, rect.y(), xlinenum->width(), rect.height());
    updateLineNumberAreaWidth(0);
    /*
    dy = this->verticalScrollBar()->sliderPosition();
    if (dy > -1) {
        xlinenum->scroll(0, dy);
    }

    // Addjust slider to alway see the number of the currently being edited line...
    int firstId = getFirstVisibleBlockId(this);
    if (firstId == 0 || this->textCursor().block().blockNumber() == firstId-1)
        this->verticalScrollBar()->setSliderPosition(dy-this->document()->documentMargin());
    */
}

int GTextEdit::lineNumberAreaWidth() {

    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
    while( max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void GTextEdit::resizeEvent( QResizeEvent* evt ) {
    QTextEdit::resizeEvent(evt);
    if( xcf->isNodeFlag(NF_LINENUM) ) {
        QRect cr = contentsRect();
        xlinenum->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void GTextEdit::extraAreaMouseEvent(QMouseEvent *evt) {

    // Set whether the mouse cursor is a hand or a normal arrow
    if( evt->type() == QEvent::MouseMove) {
        int markWidth = xlinenum->width();
        bool hand = (evt->pos().x() <= markWidth);
        if( hand != (xlinenum->cursor().shape() == Qt::PointingHandCursor)) {
            xlinenum->setCursor(hand ? Qt::PointingHandCursor : Qt::ArrowCursor);
        }
    }
    XFuncNode* fnCur = getEventFuncNode(config(), "onLineNumMouse");
    StrVar* sv=config()->GetVar("@snum");
    int selectNumber = SVCHK('0',0)? sv->getInt(4): -1;
    if( evt->type() == QEvent::MouseButtonPress || evt->type() == QEvent::MouseButtonDblClick) {
        bool bok=false;
        if( evt->button() == Qt::LeftButton) {
            bok=true;
            if( fnCur ) fnCur->GetVar("button")->set("left");
        } else if(  evt->button() == Qt::RightButton) {
            if( fnCur ) fnCur->GetVar("button")->set("right");
        }
        QTextCursor cursor = cursorForPosition(QPoint(0, evt->pos().y()));
        QTextCursor selection = cursor;
        if( fnCur ) {
            QTextBlock block=selection.block();
            int bsp=block.position(), bep=bsp+block.length();
            if( evt->type() == QEvent::MouseButtonPress ) fnCur->GetVar("type")->set("down"); else fnCur->GetVar("type")->set("doubleClick");
            fnCur->GetVar("mode")->setVar('1',0).addUL64((UL64)evt->modifiers());
            fnCur->GetVar("pos")->setVar('i',2).addInt(evt->pos().x()).addInt(evt->pos().y());
            fnCur->GetVar("range")->setVar('i',2).addInt(bsp).addInt(bep);
            fnCur->call();
            sv->reuse();
            LPCC result=toString(&(fnCur->xrst));
            if( ccmp(result,"ignore") ) bok=false;

        }
        if( bok) {
            selection.setVisualNavigation(true);
            config()->GetVar("@snum")->setVar('0',0).addInt( selection.blockNumber() );
            selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            setTextCursor(selection);
        }
    }  else if( selectNumber >= 0 ) {
        if( evt->type() == QEvent::MouseMove) {
            QTextCursor cursor = cursorForPosition(QPoint(0, evt->pos().y()));
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            QTextBlock anchorBlock = document()->findBlockByNumber(selectNumber);
            selection.setPosition(anchorBlock.position());
            if( cursor.blockNumber() < selectNumber) {
                selection.movePosition(QTextCursor::EndOfBlock);
                selection.movePosition(QTextCursor::Right);
            }
            selection.setPosition(cursor.block().position(), QTextCursor::KeepAnchor);
            if(  cursor.blockNumber() >= selectNumber) {
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            }
            setTextCursor(selection);
        } else {
            config()->GetVar("@snum")->setVar('0',0).addInt(-1);
            return;
        }

    }
}

bool GTextEdit::event(QEvent* evt) {
    if( procWidgetEvent(evt, config(), this) )
        return true;
    if( evt->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
        if( keyEvent->key() == Qt::Key_Tab) {
            QTextCursor c = textCursor();
            if( c.hasSelection()) {
                editorTabIndent(document(), c, true);
            } else {
                c.insertText("\t");
            }
            this->setTextCursor(c);
            return true;
        }
        else if( keyEvent->key() == Qt::Key_Backtab
            || (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() & Qt::ShiftModifier)) {
            QTextCursor c = textCursor();
            /*
            if( c.hasSelection() ) {
                StrVar* sv = config()->gv("@tps");
                if( SVCHK('a',0) ) {
                    XListArr* arr = (XListArr*)SVO;
                    arr->reuse();
                }
            }
            */
            if( c.hasSelection()) {
                editorTabIndent(document(), c, false);
                this->setTextCursor(c);
            }
            return true;
        }
        else if( keyEvent->key() == Qt::Key_Return ) {
            QTextCursor c = textCursor();
            QString text = c.block().text();
            int indent = firstNonSpace(text);
            if( indent) {
                c.beginEditBlock();
                c.insertBlock();
                c.insertText(text.mid(0,indent));
                c.endEditBlock();
            } else {
                c.insertBlock();
            }
            setTextCursor(c);
            return true;
        }
    }
    return QTextEdit::event(evt);
}

void GTextEdit::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    print(printer);
#endif
}

bool GTextEdit::canInsertFromMimeData(const QMimeData* source) const {
    return source->hasImage() || source->hasUrls() ||
        QTextEdit::canInsertFromMimeData(source);
}

void GTextEdit::insertFromMimeData(const QMimeData* source) {
    if( xcf->isBool("pasteStyleUse")) {
        if (source->hasImage()) {
            static int i = 1;
            QUrl url(QString("dropped_image_%1").arg(i++));
            dropImage(url, qvariant_cast<QImage>(source->imageData()));
        } else if (source->hasUrls()) {
            foreach (QUrl url, source->urls()) {
                QFileInfo info(url.toLocalFile());
                if (QImageReader::supportedImageFormats().contains(info.suffix().toLower().toLatin1()))
                    dropImage(url, QImage(info.filePath()));
                else
                    dropTextFile(url);
            }
        } else {
            QTextEdit::insertFromMimeData(source);
        }
    } else {
        QTextEdit::insertPlainText ( source->text() );
    }
}
void GTextEdit::dropImage(const QUrl& url, const QImage& image) {
    if (!image.isNull()) {
        document()->addResource(QTextDocument::ImageResource, url, image);
        textCursor().insertImage(url.toString());
    }
}

void GTextEdit::dropTextFile(const QUrl& url) {
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        textCursor().insertText(file.readAll());
    }
}

void GTextEdit::setCharFormat(LPCC code, TreeNode* cf) {
    setEditorCharFormat(xsyntax, code, cf, config(),(QWidget*)this );
}


XHighlighter::XHighlighter( QTextDocument *parent, TreeNode* cf) : QSyntaxHighlighter(parent), xcf(cf), xstringHighlightData(NULL) {
}
XHighlighter::~XHighlighter() {
    for( int n=0,size=xfmts.size();n<size;n++) {
        XHighlightData* h = xfmts.getvalue(n);
        SAFE_DELETE(h);
    }
    for( int n=0,size=xcommentFmts.size();n<size;n++) {
        XHighlightData* h = xcommentFmts.getvalue(n);
        SAFE_DELETE(h);
    }
    for( int n=0,size=xblockFmts.size();n<size;n++) {
        XHighlightData* h = xblockFmts.getvalue(n);
        SAFE_DELETE(h);
    }
    SAFE_DELETE(xstringHighlightData);
}

void XHighlighter::asHtml(QString& html) {
    // Create a new document from all the selected text document.
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    QTextDocument* tempDocument(new QTextDocument);
    Q_ASSERT(tempDocument);
    QTextCursor tempCursor(tempDocument);

    tempCursor.insertFragment(cursor.selection());
    tempCursor.select(QTextCursor::Document);
    // Set the default foreground for the inserted characters.
    QTextCharFormat textfmt = tempCursor.charFormat();
    textfmt.setForeground(Qt::gray);
    tempCursor.setCharFormat(textfmt);

    // Apply the additional formats set by the syntax highlighter
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    end = end.next();
    const int selectionStart = cursor.selectionStart();
    const int endOfDocument = tempDocument->characterCount() - 1;
    for(QTextBlock current = start; current.isValid() ; current = current.next()) {
        const QTextLayout* layout(current.layout());

        foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
            const int start = current.position() + range.start - selectionStart;
            const int end = start + range.length;
            if(end <= 0 || start >= endOfDocument)
                continue;
            tempCursor.setPosition(qMax(start, 0));
            tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
            tempCursor.setCharFormat(range.format);
        }
    }

    // Reset the user states since they are not interesting
    for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
        block.setUserState(-1);

    // Make sure the text appears pre-formatted, and set the background we want.
    tempCursor.select(QTextCursor::Document);
    QTextBlockFormat blockFormat = tempCursor.blockFormat();
    blockFormat.setNonBreakableLines(true);
    blockFormat.setBackground(Qt::black);
    tempCursor.setBlockFormat(blockFormat);

    // Finally retreive the syntax higlighted and formatted html.
    html = tempCursor.selection().toHtml();
    delete tempDocument;
} // asHtml

/*
inline int getHighlightStringPos( const QString& text, char ch ) {
    int ep=text.indexOf(ch, 0);
    if( ep==-1 ) return ep;
    bool bEnd=false;
    while( ep!=-1 ) {
        if( ep>0 && text.at(ep-1)==QChar('\\') ) {
            ep = text.indexOf(ch, ep+1);
            continue;
        }
        ep = text.indexOf(ch, ep+1);
        if( ep==-1 ) break;
        if( bEnd )
            bEnd=false;
        else
            bEnd=true;
    }
    return bEnd? -1 : ep;
}
*/

void XHighlighter::highlightBlock( const QString &text ) {
    /*
    static char prevChar='\0';
    static long prevCharTick=0;
    */
    int index=0,length=0, commentStart=-1, commentEnd=-1, commentPos=-1, stringEnd=0;
    int statePrev = previousBlockState(), stateCur=0;
    // ver 1.0.3

    if( statePrev==52 && xstringHighlightData ) {
        // long dist=GetTickCount() - prevCharTick;
        int sp=0, ep=0;
        char ch=xstringHighlightData->xtype&0x10000 ? '\'': '\"';
        ep=text.lastIndexOf( ch );
        /*
        if( dist<10 ) {
            ep=text.lastIndexOf( prevChar);
        } else {
            ep=getHighlightStringPos(text, '\'');
            if( ep==-1 ) ep=getHighlightStringPos(text, '\"');
        }
        */
        if( ep!=-1 ) {
            index = ep+1;
            stringEnd=index;
            setFormat(sp, index-sp, xstringHighlightData->charFormat());
        } else {
            ep=text.length();
            setCurrentBlockState(52);
            setFormat(sp, ep-sp, xstringHighlightData->charFormat());
            return;
        }
    }


    // 이전 상태가 문자열이 아니라면( ok==false 면 이전상태 문자열)
    for( int n=0,size=xfmts.size(); statePrev<50 && n<size; n++) {
        XHighlightData* h = xfmts.getvalue(n);
        int sp=stringEnd;
        U16 state = h->getState();
        if( state==10 ) {	// line comment
            index = h->exp.indexIn(text, sp);
            if( index >= 0 ) {
                commentStart=index;
                length = h->exp.matchedLength();
                setFormat(index, length, h->charFormat());
            }
        } else if( state==71 || state==72 ) {
            index = h->exp.indexIn(text, sp);
            while( index >= 0 ) {
                length=h->exp.matchedLength();
                int ep = h->expEnd.indexIn(text,index+length);
                if( ep>index ) {
                    if( state==71 ) {
                        ep+=h->expEnd.matchedLength();
                        setFormat(index, ep-index, h->charFormat());
                        sp=ep;
                    } else {
                        setFormat(index, length, h->charFormat());
                        setFormat(ep, h->expEnd.matchedLength(), h->charFormat());
                        sp=ep+h->expEnd.matchedLength();
                    }
                    index = h->exp.indexIn(text, sp);
                } else {
                    break;
                }
            }
        } else {
            index = h->exp.indexIn(text,sp);
            length = 0;
            while(index >= 0 ) {
                if( state==13 ) {			// SYNTAX.tag
                    index = h->exp.pos(1);
                    length = h->exp.cap(0).length();
                    // qDebug("--- tag index=%d, length=%d ---\n", index, length);
                    if( text.at(index-1)=='/' ) {
                        index-=2, length+=2;
                    } else {
                        index-=1, length+=1;
                    }
                } else {
                    length = h->exp.matchedLength();
                    if( length==0 )
                        break;
                    if( state==11 )
                        length-=1;// SYNTAX.func
                }
                if( index==-1 || length==0 ) {
                    break;
                }
                setFormat(index, length, h->charFormat());
                index = text.indexOf(h->exp, index+length); // expr.indexIn(text, index + length);
            }
        }
    }
    bool ok=true;
    for( int n=0,size=xblockFmts.size();n<size;n++) {
        XHighlightData* h = xblockFmts.getvalue(n);
        U16 state = h->getState();
        int startIndex=-1;
        length=0;
        if( statePrev==state ) {
            startIndex=0;
        } else {
            if( statePrev>100 ) continue;
            startIndex = text.indexOf(h->exp, stringEnd);
            if( startIndex>=0 ) {
                length=h->exp.matchedLength();
                if( commentStart!=-1 ) {
                    if( commentStart < startIndex ) {
                        startIndex=-1;
                    } else {
                        commentPos=commentStart;
                        commentStart=startIndex;
                    }
                } else {
                    commentStart=startIndex;
                }
            }
        }

        while( startIndex >= 0) {
            int endIndex = text.indexOf(h->expEnd, startIndex+length);
            int commentLength;
            if( endIndex == -1) {
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + h->expEnd.matchedLength();
            }
            if( commentLength==0 ) {
                startIndex = text.indexOf(h->exp, startIndex + 1);
            } else {
                setFormat(startIndex, commentLength, h->charFormat());
                startIndex = text.indexOf(h->exp, startIndex + commentLength);
            }
            if( endIndex != -1) {
                if( commentPos!=-1 ) commentStart=commentPos;
                commentEnd=endIndex;
                // statePrev=0;
                setCurrentBlockState(0);
                ok=true;
                break;
            } else {
                setCurrentBlockState(state);
                ok=false;
            }
        }
        if( statePrev==state ) {
            return;
        }
    }
    for( int n=0,size=xcommentFmts.size();ok && n<size ;n++) {
        XHighlightData* h = xcommentFmts.getvalue(n);
        U16 state = h->getState();
        index = h->exp.indexIn(text, 0);
        if( index==-1 ) continue;
        if( state==62 ) {
            if( index<2 ) {
                length = h->exp.matchedLength();
                if( length>0 ) setFormat(index, length, h->charFormat());
                setCurrentBlockState(0);
                return;
            }
        } else {
            length = h->exp.matchedLength();
            if( length>0 ) setFormat(index, length, h->charFormat());
            commentStart=index;
        }
    }

    if( xstringHighlightData  ) {
        U16 state = xstringHighlightData->getState();
        if( state==52  || state==51 ) {
            char ch=0;
            int sp=0, ep=0;
            index=stringEnd;
            for( int n=0; n<256; n++ ) {
                sp = text.indexOf('\"', index );
                if( sp!=-1 ) {
                    ep = text.indexOf('\'', index);
                    ch = '\"';
                    if( ep!=-1 && sp>ep ) {
                        ch = '\'';
                        sp = ep;
                    }
                } else {
                    sp = text.indexOf('\'', index);
                    ch= '\'';
                }
                if( sp==-1 ) break;

                if( commentStart!=-1 ) {
                    if( sp>commentStart ) {
                        if( commentEnd!=-1 ) {
                            if( sp<commentEnd ) {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else if( ok==false ) {
                        ok=true;
                    }
                }
                ep = text.indexOf(ch,sp+1);
                ok = ep!=-1 ? true: false;

                while( ch=='\"' && ep>0 && ok ) {
                    if( text.at(ep-1)!=QChar('\\') )
                        break;
                    if( ep>1 && text.at(ep-2)==QChar('\\') )
                        break;
                    ep = text.indexOf(ch, ep+1);
                    if( ep==-1 ) {
                        ok = false;
                    }
                }
                // if( cp==-1 ) cp = sp;
                if( ok ) {
                    index = ep+1;
                    setFormat(sp, index-sp, xstringHighlightData->charFormat());
                } else {
                    ep = text.size();
                    if( state==52 ) { // mode=='eps'
                        /*
                        prevChar = ch;
                        prevCharTick=GetTickCount();
                        */
                        xstringHighlightData->xtype&=~0xFF0000;
                        if( ch=='\'' ) {
                            xstringHighlightData->xtype|=0x10000;
                        } else {
                            xstringHighlightData->xtype|=0x20000;
                        }

                        setCurrentBlockState(state);
                    }
                    setFormat(sp, ep-sp, xstringHighlightData->charFormat());
                    break;
                }
            }
            if( state!=52 && ok==false ) ok = true;
        }
    }

    if( ok ) {
        setCurrentBlockState(0);
    }
}


/************************************************************************/
/*   TextLineNumberArea                                                     */
/************************************************************************/
TextLineNumberArea::TextLineNumberArea(GTextEdit *editor) : QWidget(editor), xflag(0)
{
    xedit = editor;
}

QSize TextLineNumberArea::sizeHint() const
{
    return QSize(xedit->lineNumberAreaWidth(), 0);
}
void TextLineNumberArea::paintEvent(QPaintEvent *event)
{
    xedit->lineNumberAreaPaintEvent(event);
}

void TextLineNumberArea::mouseMoveEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
    //QWidget::mouseMoveEvent(evt);
}

void TextLineNumberArea::mousePressEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
    //QWidget::mousePressEvent(evt);
}

void TextLineNumberArea::mouseReleaseEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
    //QWidget::mouseReleaseEvent(evt);
}


/************************************************************************/
/*   GProgress                                                     */
/************************************************************************/

GProgress::GProgress(TreeNode* cf, QWidget* p) : QProgressBar(p), xcf(cf) {
    connect(this, SIGNAL(ValueChanged( int value)), this, SLOT(slotValueChanged( int value)) );

}
void	GProgress::slotValueChanged( int value ) { uom.onProgressValueChanged(config(),value); }



ComboDelegate::ComboDelegate(TreeNode *cf, TreeProxyModel *proxy) : xcf(cf), xproxy(proxy) {
    StrVar* sv= NULL; // cf->gv("onDraw");
    xfnDraw = getEventFuncNode(cf, "onDraw");
    xproxy=NULL;
    xuse = true;
    xdraw=new TreeNode(2, true);
    xdraw->xstat=FNSTATE_DRAW;
    lineHeight = 24;
    sv = cf->gv("@lineHeight");
    if( isNumberVar(sv) ) {
        lineHeight = toInteger(sv);
    }
}

void ComboDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    XFuncNode* fnDraw = getEventFuncNode(xcf, "onDraw");

    if( fnDraw && xdraw &&  option.state!=1 ) {
        // onDraw( d, node, str, index, state, object);
        StrVar* sv=NULL;
        PARR arrs=getLocalParams(fnDraw);
        setVarRectF(xdraw->GetVar("@rect"), option.rect );
        xdraw->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        arrs->add()->setVar('n',0,(LPVOID)xdraw );
        /*
        sv=arrs->add();
        if( xproxy ) {
            TreeNode* node=static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
            if( node ) sv->setVar('n',0,(LPVOID)node);
        } else {
            QString str = index.data().toString();
            sv->set(Q2A(str));
        }
        */
        arrs->add()->setVar('0',0).addInt(index.row());
        arrs->add()->setVar('0',0).addInt((int)option.state);
        // arrs->add()->setVar('n',0,(LPVOID)xcf);
        setFuncNodeParams(fnDraw, arrs);
        fnDraw->call();
        return;
    }
    QItemDelegate::paint (painter, option, index);
}

QSize ComboDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    StrVar* sv = xcf->gv("@lineHeight");
    int comboLineHeight=lineHeight;
    if( isNumberVar(sv) ) {
        comboLineHeight= toInteger(sv);
    }
    return QSize(1, comboLineHeight); // the row height is now 40
}

GCombo::GCombo(TreeNode* cf, QWidget* p) : QComboBox(p), xcf(cf) {
    view()->viewport()->installEventFilter(this);

    connect(this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)) );
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)) );

}
void	GCombo::slotActivated ( int index )					{ uom.onComboActivated(config(),index); }
void	GCombo::slotCurrentIndexChanged ( int index )		{ uom.onComboCurrentChanged(config(),index);  }
void	GCombo::showPopup() {
    QComboBox::showPopup();
    XFuncNode* fnCur = getEventFuncNode(config(), "onPopup");
    if( fnCur ) {
        fnCur->call();
    }
}

bool	GCombo::eventFilter(QObject* object, QEvent* evt) {
    switch(evt->type() ){
    case QEvent::MouseButtonPress: {
        if( object == view()->viewport() ) {
            QMouseEvent* e = static_cast<QMouseEvent*>(evt);
            xcf->GetVar("@mpos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
        }
    } break;
    case QEvent::MouseButtonRelease: {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        if( object == view()->viewport() ) {
            StrVar* sv=xcf->gv("@mpos");
            if( SVCHK('i',2) ) {
                QRect rc(sv->getInt(4),sv->getInt(8),8,8);
                sv->reuse();
                if( rc.contains(e->pos()) ) {
                    sv=xcf->gv("onViewClick");
                }
            }
            if( SVCHK('f',0) ) {
                XFuncNode* fn=(XFuncNode*)SVO;
                fn->GetVar("@pos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
                fn->GetVar("@keys")->setVar('1',0).addUL64((UL64)e->modifiers());
                fn->GetVar("@button")->set(e->button()==Qt::LeftButton?"left":"right");
                fn->call();
                LPCC rtn = toString(&(fn->xrst));
                if( ccmp(rtn,"ignore") ) return true;
            }
        } else if( this->isEditable() && object==this->lineEdit() ) {
            if( this->lineEdit()->hasSelectedText() ) {
                qDebug("########### combo focus (%d,%d) #############\n",e->pos().x(), e->pos().y() );
            } else {
                this->lineEdit()->selectAll();
                qDebug("combo focus (%d,%d)\n",e->pos().x(), e->pos().y() );
                return true;
            }
        }
    } break;
    default: break;
    }

    return false;
}


GHeaderView::GHeaderView(TreeNode* cf, Qt::Orientation orientation, QWidget * parent) : QHeaderView(orientation,parent), xcf(cf) {
    xidx=0;
}
QSize GHeaderView::sizeHint() const {
    // Get the base implementation size.
    QSize baseSize = QHeaderView::sizeHint();
    StrVar* sv=xcf->gv("@headerHeight");
    if( isNumberVar(sv) ) {
        baseSize.setHeight( toInteger(sv) );
    } else {
        baseSize.setHeight(32);
    }
    return baseSize;
}
void GHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
    XFuncNode* fnDraw = getEventFuncNode(xcf, "onDrawHeader");
    if( fnDraw ) {
        // onDrawHeader(d, text, index, order);
        QString text = model()->headerData( logicalIndex, orientation(), Qt::DisplayRole).toString();
        TreeNode* d=NULL;
        StrVar* sv=fnDraw->gv("@draw");
        if( SVCHK('n',0) ) {
            d=(TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            fnDraw->GetVar("@draw")->setVar('n',0,(LPVOID)d);
        }
        PARR arrs=getLocalParams(fnDraw);
        setVarRectF(d->GetVar("@rect"), rect);
        d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        // ### version 1.0.4
        arrs->add()->setVar('n',0,(LPVOID)d);
        arrs->add()->set(Q2A(text));
        arrs->add()->setVar('0',0).addInt(logicalIndex);
        arrs->add()->setVar('0',0).addInt(xidx);
        arrs->add()->setVar('0',0).addInt(sortIndicatorOrder());
        setFuncNodeParams(fnDraw, arrs);
        fnDraw->call();
    } else {
        // version 1.0.3
        QString text = model()->headerData( logicalIndex, orientation(), Qt::DisplayRole).toString();
        StrVar* sv=xcf->gv("@headerFontSize");
        if( isNumberVar(sv) ) {
            QFont font=painter->font();
            font.setPointSize(toInteger(sv) );
            painter->setFont(font);
        }
        painter->fillRect(rect, Qt::white);
        if( text.isEmpty() ) return;

        text = painter->fontMetrics().elidedText(text, Qt::ElideRight, rect.width());
        int textWidth = painter->fontMetrics().width(text);

        painter->save();
        if( logicalIndex == xidx ) {
            painter->setPen(QColor(62, 32, 32));
            painter->drawText(rect, Qt::AlignCenter, text);
            QPixmap* pm=NULL;
            if(Qt::DescendingOrder == sortIndicatorOrder() ) {
                pm = getPixmap("list_icon:down",true);
                if( pm )
                painter->drawPixmap(
                    ((rect.right() - (rect.width()/2) ) + (textWidth / 2)) + 5, (rect.bottom() - rect.top() - 4 )/2 ,
                    6,4,*pm
                );
            } else if(Qt::AscendingOrder == sortIndicatorOrder()) {
                pm = getPixmap("list_icon:up",true);
                if( pm )
                painter->drawPixmap(
                    ((rect.right() - (rect.width()/2) ) + (textWidth / 2)) + 5, (rect.bottom() - rect.top() - 4 )/2 ,
                    6, 4, *pm
                );
            }
        } else {
            painter->setPen(QColor(38, 38, 38));
            painter->drawText(rect, Qt::AlignCenter, text);
        }
        painter->restore();
        painter->save();
        painter->setPen(QColor(126, 126, 126));
        painter->drawLine(rect.right(), (rect.bottom()-rect.top())/2, rect.right(), rect.bottom());
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        painter->restore();
    }
}
void GHeaderView::clickDetailHeader(int index) {
    xidx = index;
    xcf->GetVar("@sortOrder")->setVar('0',0).addInt( sortIndicatorOrder() );
    xcf->GetVar("@clickIndex")->setVar('0',0).addInt(index);
    XFuncNode* fnCur = getEventFuncNode(xcf, "onHeaderClick");
    if( fnCur ) {
        fnCur->call();
    }
}
void GHeaderView::sectionClickedSlot(int index) {
    // varsion 1.0.3
    StrVar* sv=xcf->gv("sortIndex");
    if( SVCHK('0',0) ) {
        sv->setVar('9',0);
    }
    xidx = index;
    XFuncNode* fnCur = getEventFuncNode(xcf, "onHeaderClick");
    if( fnCur ) {
        fnCur->GetVar("@sortOrder")->setVar('0',0).addInt( sortIndicatorOrder() );
        fnCur->GetVar("@sortIndex")->setVar('0',0).addInt(index);
        fnCur->call();
    }
}


GTree::GTree(TreeNode* cf, QWidget* p) : QTreeView(p), xcf(cf) {
    xcf->xtype=1;
    setMouseTracking(true);
    viewport()->installEventFilter(new GViewEventFilter(viewport(),xcf));
}


void	GTree::slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous )					{ uom.onViewCurrentChanged(config(),current,previous); }
void	GTree::slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous )			{ uom.onViewCurrentColumnChanged(config(),current,previous);  }
void	GTree::slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous )				{ uom.onViewCurrentRowChanged(config(),current,previous);  }
void	GTree::slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )		{ uom.onViewSelectionChanged(config(),selected,deselected);  }
void	GTree::slotDoubleClicked(const QModelIndex & index)														{ uom.onViewDoubleClicked(config(),index); }
void	GTree::slotClicked ( const QModelIndex & index )														{ uom.onViewClicked(config(),index); }
void	GTree::slotCollapsed(const QModelIndex & index)															{ }
void	GTree::slotClickHeader(int index )																		{
    GHeaderView* hv = qobject_cast<GHeaderView*>(header());
    if( hv ) {
        hv->clickDetailHeader(index);
    }
}

void	GTree::mousePressEvent(QMouseEvent * event) {
    /*
    if (event->button() == Qt::RightButton) {
        QModelIndex index(indexAt(event->pos()));
        if (index.isValid()) {
            setCurrentIndex(index);
        }
    } else {
        setDragDropMode(QAbstractItemView::InternalMove);
        QTreeView::mousePressEvent(event);
    }

    */
    XFuncNode* fn = getEventFuncNode(config(), "onMouseDown");
    if( fn ) {
        QPoint pt = event->pos();
        config()->GetVar("@mousePos")->setVar('i',2).addInt(pt.x()).addInt(pt.y());
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
        arrs->add()->setVar('1',0).addUL64((UL64)event->modifiers());
        arrs->add()->set(event->button()==Qt::LeftButton?"left":"right");
        setFuncNodeParams(fn, arrs);
        fn->call();
        LPCC result=toString(fn->getResult());
        if( ccmp("ignore",result) ) {
            return;
        }
    }
    QTreeView::mousePressEvent(event);
}
void GTree::mouseReleaseEvent(QMouseEvent * event) {
    QPoint pt = event->pos();
    StrVar* sv=config()->gv("@mousePos");
    XFuncNode* fn =NULL;
    if( SVCHK('i',2) ) {
        int x=sv->getInt(4)-8,y=sv->getInt(8)-8;
        QRect rc(x,y,16,16);
        sv->reuse();
        if( rc.contains(event->pos()) ) {
            fn=getEventFuncNode(config(), "onMouseClick");
        }
        if( fn==NULL ) {
            fn=getEventFuncNode(config(), "onMouseUp");
        }
        config()->GetVar("@mousePos")->reuse();
    }
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
        arrs->add()->setVar('1',0).addUL64((UL64)event->modifiers());
        arrs->add()->set(event->button()==Qt::LeftButton?"left":"right");
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
    QTreeView::mouseReleaseEvent(event);
}
void GTree::mouseMoveEvent(QMouseEvent * event) {
    XFuncNode* fn = getEventFuncNode(config(), "onMouseMove");
    if( fn ) {
        QPoint pt = event->pos();
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('i',2).addInt(pt.x()).addInt(pt.y());
        arrs->add()->setVar('1',0).addUL64((UL64)event->modifiers());
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
    QTreeView::mouseMoveEvent(event);
}

void GTree::columnResized(int logicalIndex, int oldSize, int newSize) {
    QTreeView::columnResized(logicalIndex, oldSize, newSize);
    /*
    StrVar* sv=config()->gv("@proxy");					if( SVCHK('m',1)==false ) return;
    TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return;
    sv= logicalIndex<proxy->xfields->childCount() ? proxy->xfields->child(logicalIndex)->gv("width") : NULL;
    if( sv ) {
        sv->setVar('0',0).addInt(newSize);
    }
    */
}

void GTree::dragEnterEvent(QDragEnterEvent *event) {
   //  event->acceptProposedAction();
    XFuncNode* fn = getEventFuncNode(config(), "onDrag");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set("start");
        arrs->add()->setVar('m',2,(LPVOID)event->mimeData());
        setFuncNodeParams(fn, arrs);
        fn->call();
        LPCC ty=toString(fn->getResult());
        if( ccmp(ty,"accept") ) {
            event->accept();
        } else if( ccmp(ty,"move") ) {
            event->setDropAction(Qt::MoveAction); event->accept();
        } else if( ccmp(ty,"copy") ) {
            event->setDropAction(Qt::CopyAction); event->accept();
        } else if( ccmp(ty,"proposed") ) {
            event->acceptProposedAction();
        } else
            event->ignore();
    }

}
void	GTree::dragMoveEvent(QDragMoveEvent* event) {
    XFuncNode* fn = getEventFuncNode(config(), "onDragMove");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('i',2).addInt(event->pos().x()).addInt(event->pos().y());
        arrs->add()->setVar('m',2,(LPVOID)event->mimeData());
        setFuncNodeParams(fn, arrs);
        fn->call();
        LPCC ty=toString(fn->getResult());
        if( ccmp(ty,"accept") ) {
            event->accept();
        } else if( ccmp(ty,"move") ) {
            event->setDropAction(Qt::MoveAction); event->accept();
        } else if( ccmp(ty,"copy") ) {
            event->setDropAction(Qt::CopyAction); event->accept();
        } else if( ccmp(ty,"proposed") ) {
            event->acceptProposedAction();
        } else
            event->ignore();
    }

}
void	GTree::startDrag(Qt::DropActions act) {
    QTreeView::startDrag(act);
}
void GTree::dropEvent(QDropEvent *event) {
    XFuncNode* fn = getEventFuncNode(config(), "onDrop");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('i',2).addInt(event->pos().x()).addInt(event->pos().y());
        arrs->add()->setVar('m',2,(LPVOID)event->mimeData());
        setFuncNodeParams(fn, arrs);
        fn->call();
        LPCC ty=toString(fn->getResult());
        if( ccmp(ty,"accept") ) {
            event->accept();
        } else if( ccmp(ty,"move") ) {
            event->setDropAction(Qt::MoveAction); event->accept();
        } else if( ccmp(ty,"copy") ) {
            event->setDropAction(Qt::CopyAction); event->accept();
        } else if( ccmp(ty,"proposed") ) {
            event->acceptProposedAction();
        } else
            event->ignore();
    }
}

GListVIew::GListVIew(TreeNode* cf, QWidget* p) : QListView(p), xcf(cf) {
}

void	GListVIew::slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous )					{ uom.onViewCurrentChanged(config(),current,previous); }
void	GListVIew::slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous )			{ uom.onViewCurrentColumnChanged(config(),current,previous);  }
void	GListVIew::slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous )				{ uom.onViewCurrentRowChanged(config(),current,previous);  }
void	GListVIew::slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )		{ uom.onViewSelectionChanged(config(),selected,deselected);  }
void	GListVIew::slotDoubleClicked(const QModelIndex & index)														{ uom.onViewDoubleClicked(config(),index); }
void	GListVIew::slotClicked ( const QModelIndex & index )														{ uom.onViewClicked(config(),index); }

GTable::GTable(TreeNode* cf, QWidget* p) : QTableView(p), xcf(cf) {
}

void	GTable::slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous )				{ uom.onViewCurrentChanged(config(),current,previous); }
void	GTable::slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous )			{ uom.onViewCurrentColumnChanged(config(),current,previous);  }
void	GTable::slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous )				{ uom.onViewCurrentRowChanged(config(),current,previous);  }
void	GTable::slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )		{ uom.onViewSelectionChanged(config(),selected,deselected);  }
void	GTable::slotDoubleClicked(const QModelIndex & index)													{ uom.onViewDoubleClicked(config(),index); }
void	GTable::slotClicked ( const QModelIndex & index )														{ uom.onViewClicked(config(),index); }


GTab::GTab(TreeNode* cf, QWidget* p) : QTabWidget(p), xcf(cf) {
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)) );
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(slotTabCloseRequested(int)) );
}
void	GTab::slotCurrentChanged (int index) { uom.onTabCurrentChanged(config(), index); }
void	GTab::slotTabCloseRequested (int index) { uom.onTabCloseRequested(config(), index); }


GSplitter::GSplitter(TreeNode* cf, QWidget* p) : QSplitter(p), xcf(cf) {
}
void	GSplitter::slotSplitterMoved ( int pos,int index) {
    uom.onSplitterMoved(config(), pos, index);
}
QSplitterHandle *GSplitter::createHandle() {
    return new GSplitterHandle(xcf, orientation(), this);
}

GSplitterHandle::GSplitterHandle(TreeNode* cf, Qt::Orientation ori, QSplitter* p)
    :QSplitterHandle(ori,p), xcf(cf)
{

}
void GSplitterHandle::paintEvent(QPaintEvent *e) {
    XFuncNode* fn = getEventFuncNode(config(), "onDrawHandle");
    if( fn ) {
        // ex) onDrawHandle(d, type)
        TreeNode* d=NULL;
        StrVar* sv=fn->GetVar("@draw");
        QPainter p(this);
        if( SVCHK('n',0) ) {
            d=(TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            sv->setVar('n',0,(LPVOID)d);
        }
        PARR arrs=getLocalParams(fn);
        d->GetVar("@g")->setVar('g',0,(LPVOID)&p);
        setVarRectF(d->GetVar("@rect"), e->rect());
        arrs->add()->setVar('n',0,(LPVOID)d);
        if( orientation() == Qt::Horizontal) {
            arrs->add()->set("hbox");
        } else {
            arrs->add()->set("vbox");
        }
        setFuncNodeParams(fn, arrs);
        fn->call();
        return;
    }
    QSplitterHandle::paintEvent(e);
}


GStacked::GStacked(TreeNode* cf, QWidget* p) : QStackedWidget(p), xcf(cf) {
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotCurrentChanged(int)) );
    connect(this, SIGNAL(widgetRemoved(int)), this, SLOT(slotWidgetRemoved(int)) );

}
void	GStacked::slotCurrentChanged(int index) { uom.onStackedCurrentChanged(config(),index); }
void	GStacked::slotWidgetRemoved(int index) { uom.onStackedWidgetRemoved(config(), index);  }



GToolButton::GToolButton(TreeNode* cf, QWidget * p) : QToolButton(p), xcf(cf) {
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)) );
}

void	GToolButton::slotClicked (bool checked) { uom.onButtonClick(config(), checked); }
void	GToolButton::slotToggled (bool checked) { uom.onButtonTrigger(config(), checked); }


GButton::GButton(TreeNode* cf, QWidget * p) : QPushButton(p), xcf(cf) {
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()) );
}
void	GButton::slotClicked () { uom.onButtonClick(config()); }


GCheck::GCheck(TreeNode* cf, QWidget* p) : QCheckBox(p), xcf(cf) {
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)) );
}
void	GCheck::slotClicked (bool checked) { uom.onButtonClick(config(), checked); }
void	GCheck::slotToggled (bool checked) { uom.onButtonTrigger(config(), checked);}

GRadio::GRadio(TreeNode* cf, LPCC txt, QWidget* p) : QRadioButton(KR(txt),p), xcf(cf) {
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)) );
}
void	GRadio::slotClicked (bool checked) { uom.onButtonClick(config(), checked); }
void	GRadio::slotToggled (bool checked) { uom.onButtonTrigger(config(), checked); }

GSpin::GSpin(TreeNode* cf, QWidget* p) : QSpinBox(p), xcf(cf) {
    connect(this, SIGNAL(valueChanged(QString)), this, SLOT(slotValueChanged(QString)) );
}
void	GSpin::slotValueChanged(const QString & text) { uom.onSpinValueChanged(config(),text); }


/************************************************************************/
/*  GTray implemenet                                               */
/************************************************************************/
GTray::GTray(TreeNode* cf, QObject* p): QSystemTrayIcon(p), xcf(cf) {
    connect(this, SIGNAL(messageClicked()), this, SLOT(trayMessageClicked()));
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));
}

GTray::~GTray() {
    xcf->reuse();
}

void GTray::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    StrVar* sv=config()->gv("onTrayEvent");
    if( SVCHK('f',0) ) {
        XFuncNode* fn=(XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        sv=arrs->add();
        switch (reason) {
        case QSystemTrayIcon::Trigger:
            sv->set("click");
            break;
        case QSystemTrayIcon::DoubleClick:
            sv->set("doubleClick");
            break;
        case QSystemTrayIcon::MiddleClick:
            sv->set("middleClick");
            break;
        case QSystemTrayIcon::Context:
            sv->set("context");
            break;
        default: break;
        }
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void GTray::trayMessageClicked() {
    StrVar* sv=config()->gv("onTrayEvent");
    if( SVCHK('f',0) ) {
        XFuncNode* fn=(XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        arrs->add()->set("messageClick");
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

/************************************************************************/
/*  GAction implemenet                                               */
/************************************************************************/
GAction::GAction(TreeNode* cf, QObject* p) : QAction(p), xcf(cf) {
    connect(this,SIGNAL(triggered()),this, SLOT(slotActionClick()));
}

void GAction::slotActionClick() {
    XFuncNode* fn = getEventFuncNode(config(), "onClick");
    if( fn ) {
        fn->GetVar("@action")->setVar('w',11,(LPVOID)this);
        fn->call();
    } else {
        uom.onActionClick(xcf);
    }
}

void GMenu::mouseReleaseEvent(QMouseEvent *e){
    QAction *action = activeAction();
    if (action && action->isEnabled()) {
        action->setEnabled(false);
        QMenu::mouseReleaseEvent(e);
        action->setEnabled(true);
        action->trigger();
        this->close();
    }
    else
        QMenu::mouseReleaseEvent(e);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  GCanvas
//
//////////////////////////////////////////////////////////////////////////////////////////////////
inline bool itemInvalidCheck(CanvasItem* ci, TreeNode* cf ) {
    if( ci==NULL ) return false;
    if( ci->state()==0 ) ci->invalidate();
    if( ci->state()==0 ) {
        XFuncNode* fn=ci->state()==0? getEventFuncNode(cf, "onRange"): NULL;
        if( fn ) {
            fn->call();
        }
        ci->setState(1);
    }
    return true;
}

GCanvas::GCanvas(TreeNode *cf, QWidget *p) : QLabel(p), xcf(cf) {
    xcurrentTimeline=NULL;
    if( xcf->isTrue("overlay") ) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    xcf->xstat=WTYPE_CANVAS;
    xdrawObject.xstat=FNSTATE_DRAW;
    xwidgetConf.xwidget=this;
    xcanvasItems=NULL;
    xpageItem=NULL;
    xpopupItem=NULL;
    xdivItem=NULL;
    setLayout();
}

GCanvas::~GCanvas() {
    xcf->reuse();
}

bool GCanvas::startTimeLine(LPCC code, int frameEnd, int duration, LPCC mode, bool bBack) {
    if( xcurrentTimeline==NULL ) return false;
    if( xcurrentTimeline->state()== QTimeLine::Running  ) {
        // return false;
        stopTimeLine();
    }
    connect( xcurrentTimeline, SIGNAL(frameChanged(int)), this, SLOT(updateCanvas(int)) );
    connect( xcurrentTimeline, SIGNAL(finished()), this, SLOT(slotTimeLineFinish()) );
    xcurrentTimeline->setCode(code);
    xcurrentTimeline->setFrameRange(0,frameEnd);
    xcurrentTimeline->setDuration(duration );
    if( bBack ) {
        xcurrentTimeline->setDirection(QTimeLine::Backward);
    } else {
        xcurrentTimeline->setDirection(QTimeLine::Forward);
    }
    if( ccmp(mode,"in") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::EaseInCurve);
    } else if( ccmp(mode,"out") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::EaseOutCurve);
    } else if( ccmp(mode,"inout") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::EaseInOutCurve);
    } else if( ccmp(mode,"linear") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::LinearCurve);
    } else if( ccmp(mode,"sine") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::SineCurve);
    } else if( ccmp(mode,"cosine") ) {
        xcurrentTimeline->setCurveShape(QTimeLine::CosineCurve);
    } else {
        xcurrentTimeline->setCurveShape(QTimeLine::LinearCurve);
    }
    xcurrentTimeline->start();
    return true;
}
MyTimeLine* GCanvas::setTimeline(LPCC code, StrVar* rst) {
    xcurrentTimeline=NULL;
    if(slen(code)==0) return NULL;
    if( !findPairKey(&xtimelines, code,rst) ) {
        MyTimeLine* timeline=new MyTimeLine(widgetConf(), this);
        timeline->xcanvasWidget=this;
        timeline->setCode(code);
        connect( timeline, SIGNAL(frameChanged(int)), this, SLOT(updateCanvas(int)) );
        connect( timeline, SIGNAL(finished()), this, SLOT(slotTimeLineFinish()) );
        rst->setVar('t',20,(LPVOID)timeline);
        xtimelines.add()->setVar('x',21).add(code).add('\f').add(rst);
    }
    StrVar* sv=rst;
    if( SVCHK('t',20) ) {
        xcurrentTimeline=(MyTimeLine*)SVO;
    }
    return xcurrentTimeline;
}

MyTimeLine* GCanvas::nextTimeline(StrVar* rst, bool rewind) {
    int sz=xtimelines.size();
    if( sz==0 ) return NULL;
    if( xcurrentTimeline ) {
        LPCC code=xcurrentTimeline->getCode();
        int idx=findPairIndex(&xtimelines, code);
        xcurrentTimeline=NULL;
        if(idx!=-1 ) {
            idx++;
            if( idx>=sz ) {
                if( rewind==false ) return NULL;
                idx=0;
            }
            if( getPairValue(&xtimelines, idx, rst)  ) {
                StrVar* sv=rst;
                if( SVCHK('t',20) ) {
                    xcurrentTimeline=(MyTimeLine*)SVO;
                    return xcurrentTimeline;
                }
            }
        }
    }
    return NULL;
}
MyTimeLine* GCanvas::prevTimeline(StrVar* rst, bool rewind) {
    int sz=xtimelines.size();
    if( sz==0 ) return NULL;
    if( xcurrentTimeline ) {
        LPCC code=xcurrentTimeline->getCode();
        int idx=findPairIndex(&xtimelines, code);
        xcurrentTimeline=NULL;
        if(idx!=-1 ) {
            idx--;
            if( idx<0 && rewind ) {
                if( rewind==false ) return NULL;
                idx=sz-1;
            }
            if( getPairValue(&xtimelines, idx, rst)  ) {
                StrVar* sv=rst;
                if( SVCHK('t',20) ) {
                    xcurrentTimeline=(MyTimeLine*)SVO;
                    return xcurrentTimeline;
                }
            }
        }
    }
    return NULL;
}

bool GCanvas::stopTimeLine() {
    if( xcurrentTimeline==NULL ) return false;
    if( xcurrentTimeline->state()==QTimeLine::Running  ) {
        xcurrentTimeline->stop();
        return true;
    }
    return false;
}

CanvasItem* GCanvas::canvasItems(WidgetConf* root) {
#ifdef CANVAS_ITEM_USE
    bool addMode=false;
    if( root==NULL ) root=widgetConf();
    if( addMode ) {
        for( int n=0,sz=root->childCount(); n<sz; n++ ) {
            TreeNode* cur=root->child(n);
            if( cur->isNodeFlag(FLAG_INIT) ) {
                continue;
            }
            addMode=true;
        }
    }
    if( xcanvasItems==NULL ) {        
        xcanvasItems=new CanvasItem(NULL, root, config());
    } else if(addMode==false ) {
        return xcanvasItems;
    }
    for( int n=0; n<root->childCount(); n++ ) {
        WidgetConf* wcf=getWidgetConf(root->child(n));
        if( wcf ) {
            wcf->xwidget=this;
            wcf->setNodeFlag(FLAG_INIT);
            if( wcf->cmp("tag","widgets") ) {
                continue;
            }
            xcanvasItems->addChild( wcf );
        }
    }
#endif
    return xcanvasItems;
}

bool GCanvas::addWidgets(TreeNode* root, StrVar* var, int sp, int ep ) {
#ifdef CANVAS_ITEM_USE
    LPCC mode=NULL;
    XListArr arrWidgets;
    TreeNode* target=getTreeNodeTemp();
    XParseVar pv(var, sp, ep);
    if( root==NULL ) {
        root=config()->addNode("@widgets");
    }
    LPCC classId=NULL;
    TreeNode* ctrl=NULL;
    StrVar* sv=config()->gv("@targetControl");
    if( SVCHK('n',0) ) {
        ctrl=(TreeNode*)SVO;
        classId=ctrl->get("classId");
    }
    while( pv.valid() ) {
        char ch=pv.ch();
        if( ch==0 ) break;
        if( ch=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) {
                break;
            }
            ch=pv.ch();
        }
        sp=pv.start;
        if( ch!='<' ) {
            break;
        }
        LPCC tag=pv.incr().NextWord();
        qDebug("xxx canvas add widgets (tag:%s)", tag);
        if( slen(tag)==0 ) {
            break;
        }
        LPCC widgetId=NULL;
        pv.setPos(sp);
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            TreeNode* node=NULL;
            LPCC rootAddId=NULL;
            bool pageCheck=ccmp(tag,"page") || ccmp(tag,"dialog") || ccmp(tag,"main");
            if( pageCheck ) {
                if(slen(classId)) {
                    cfNode("@funcInfo")->set("classId",classId);
                }
                baroSourceApply(var, sp, pv.start);
                sv=cfNode("@funcInfo")->gv("classId");
                if(sv) sv->reuse();
                continue;
            }
            target->set("id","");
            target->set("mode","");
            parseProp(target, var, pv.prev,pv.cur, PROP_GETID );
            mode=target->get("mode");
            if( ccmp(mode,"skip") ) continue;
            widgetId=target->get("id");
            if(slen(widgetId)==0) widgetId=tag;
            if(slen(classId)) {
                rootAddId=FMT("%s:%s",classId, widgetId);
            } else {
                rootAddId=widgetId;
            }
            qDebug("#0#canvas widget add %s %s", rootAddId, widgetId);
            node=root->getNode(rootAddId);
            if( node==NULL ) {
                node=root->addNode(rootAddId);
                node->set("id", widgetId);
                node->set("tag", tag);
            }
            sp=parseProp(node, var, pv.prev,pv.cur );
            if( sp==-1 ) {
                break;
            }

            QWidget* widget=gwidget(node);
            if( widget==NULL ) {
                node->setNodeFlag(FLAG_INIT);
                if(ccmp(mode,"sub") ) {
                    widget=createWidget(tag, node);
                } else {
                    widget=createWidget(tag, node, this);
                }
                if( widget ) {
                    arrWidgets.add()->setVar('n',0,(LPVOID)node);
                    widget->hide();
                    if( ccmp(mode,"sub") ) {
                    } else {
                        widget->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
                    }
                }
            }
            if( widget ) {
                node->GetVar("@canvasNode")->setVar('n',0,(LPVOID)config() );
                setModifyClassFunc(node, var, sp, pv.cur, NULL, NULL, false, "func");
            }
            if( ccmp(mode,"break") ) break;
        } else {
            break;
        }
    }
    for(int n=0,sz=arrWidgets.size(); n<sz; n++) {
        StrVar* sv=arrWidgets.get(n);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            sv=node->gv("onInit");
            if( SVCHK('f',0) ) {
                XFuncNode* fn=(XFuncNode*)SVO;
                if( ctrl) {
                    fn->GetVar("targetControl")->setVar('n',0,(LPVOID)ctrl);
                }
                getCf()->set("@currentFuncName","onInit");
                fn->call();
            }
        }
    }
#endif
    return true;
}
bool GCanvas::setLayout(StrVar *sv, int sp, int ep) {
#ifdef CANVAS_ITEM_USE
    if( sv==NULL ) {
        sv=config()->gv("@layout");
        if( sv ) {
            if( xcanvasItems ) return true;
            sp=0, ep=sv->length();
        } else {
            if( xcanvasItems==NULL ) {
                xcanvasItems=new CanvasItem(NULL, widgetConf(), config());
            }
            WidgetConf* wcf=widgetConf()->addWidgetConf();
            wcf->set("tag","main");
            wcf->xwidget=this;
            xpageItem=xcanvasItems->addChild(wcf);
            return true;
        }
    }
    if( ep<=0 ) {
        ep=sv->length();
    }
    if( sp>=ep ) {
        qDebug("#0#canvas layout source is empty (%d, %d)", sp, ep);
        return false;
    }
    WidgetConf* root=widgetConf();
    StrVar* var=sv;
    XParseVar pv(var, sp, ep);
    while( pv.valid() ) {
        char ch=pv.ch();
        if( ch==0 ) break;
        if( ch==';' ) {
            ch=pv.incr().ch();
            if( ch==0 ) break;
        }
        if( ch=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) break;
            ch=pv.ch();
        }
        sp=pv.start;
        if( ch=='<' ) {
            LPCC tag=pv.incr().NextWord();
            if( slen(tag)==0 ) {
                break;
            }
            pv.setPos(sp);
            if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
                TreeNode* node=NULL;
                if( ccmp(tag,"widgets") ) {
                    node=config()->addNode("@widgets");
                } else {
                    WidgetConf* wcf=root->addWidgetConf();
                    node=static_cast<TreeNode*>(wcf);
                }
                if( node ) {
                    sp=parseProp(node, var, pv.prev,pv.cur );
                    if( sp==-1 ) {
                        qDebug("canvas layout source not match (%s, %s)", tag, node->log());
                        break;
                    }
                    node->set("tag", tag);
                    if( ccmp(tag,"widgets") ) {
                        addWidgets(node, var, sp, pv.cur);
                    } else {
                        node->GetVar("@canvasNode")->setVar('n',0,(LPVOID)config() );
                        setModifyClassFunc(node, var, sp, pv.cur, NULL, NULL, false, "event");
                    }
                }
            } else {
                qDebug("#9#canvas item match error (tag:%s)", tag);
                break;
            }
        } else {
            ch=pv.moveNext().ch();
            if( ch=='(' ) {
                if( !pv.match("(",")",FIND_SKIP) ) break;
            }
            ch=pv.ch();
            if( ch=='{' ) {
                if( !pv.match("{","}",FIND_SKIP) ) break;
                setModifyClassFunc(config(), var, sp, pv.start, NULL, NULL, false, "event");
            }
        }
    }
    canvasItems();
    if( xcanvasItems ) {
        if( xcanvasItems->childCount()==0 ) {
            WidgetConf* wcf=widgetConf()->addWidgetConf();
            wcf->set("tag","main");
            wcf->xwidget=this;
            xpageItem=xcanvasItems->addChild(wcf);
        } else {
            xpageItem=xcanvasItems->findTag("main");
        }
    }
    // qDebug("... setLayout ...");
#endif
    return xpageItem? true: false;
}

bool GCanvas::invalidate(bool recalc, bool all) {
#ifdef CANVAS_ITEM_USE
    if( xpageItem==NULL ) {
        return false;
    }
    if( xcurrentTimeline ) {
        if( xcurrentTimeline->state()!= QTimeLine::Running ) {
            xcurrentTimeline=NULL;
        }
    }
    QRect rc=rect();
    if( all ) {
        if( xcanvasItems ) {
            for( int n=0,sz=xcanvasItems->childCount(); n<sz; n++ ) {
                CanvasItem* ci=xcanvasItems->child(n);
                if( ci ) {
                    if( ci->tagNode()->cmp("tag","main") ) {
                        ci->setRect(0,0,rc.width(), rc.height() );
                    }
                    ci->setState(0);
                }
            }
        }
    } else if( xpageItem ) {
        xpageItem->setRect(0,0,rc.width(), rc.height() );
        if( recalc ) {
            xpageItem->setState(0);
            if( xdivItem ) xdivItem->setState(0);
            if( xpopupItem ) xpopupItem->setState(0);
        }
    }
#endif
    update();
    return true;
}

bool GCanvas::setTouchMode(bool on) {
    StrVar* sv = xcf->GetVar("@scrollArea");
    if( SVCHK('Q',20) ) {
        QScrollArea* scroll=(QScrollArea*)SVO;
        FlickCharm* fc=NULL;
        sv=xcf->GetVar("@touchScroll");
        if( SVCHK('Q',21) ) {
            fc=(FlickCharm*)SVO;
        } else {
            fc=new FlickCharm();
            sv->setVar('Q',21,(LPVOID)fc);
        }
        if( on ) {
            fc->activateOn(scroll);
        } else {
            fc->deactivateFrom(scroll);
        }
        return true;
    }
    return false;
}
bool GCanvas::drawDefault(QPainter* painter, const QRectF& rect) {
#ifdef CANVAS_ITEM_USE
    if( xpageItem ) {
        xpageItem->draw(painter, rect);
        if( xdivItem) {
            xdivItem->draw(painter, rect);
        }
    }
    if( xpopupItem ) {
        xpopupItem->draw(painter, rect);
    }
#endif
    return true;
}

bool GCanvas::draw(QPainter* painter, const QRectF& rect) {
    XFuncNode* fn=getEventFuncNode(config(), "onDraw");
    // bool drawCheck=false;
    // drawDefault(painter, rect);
    if( fn ) {
        TreeNode* d=drawObject();
        PARR arrs=getLocalParams(fn);
        LPCC state=xcurrentTimeline? xcurrentTimeline->getCode(): NULL;
        setVarRectF(d->GetVar("@rect"), rect );
        d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        /* draw param setting */
        arrs->add()->setVar('n',0,(LPVOID)d);
        setVarRectF(arrs->add(), rect );
        if( slen(state) ) arrs->add()->set(state);
        setFuncNodeParams(fn, arrs);
        fn->call();
    } else if(xpageItem) {
        drawDefault(painter, rect);
    }
    return true;
}

bool GCanvas::showPopup(LPCC id, QRectF rect) {
#ifdef CANVAS_ITEM_USE
    if( xcanvasItems==NULL ) return false;
    CanvasItem* ci=NULL;
    for( int n=0,sz=xcanvasItems->childCount(); n<sz; n++ ) {
        CanvasItem* cur=xcanvasItems->child(n);
        if( cur->cmp("tag","popup") && cur->cmp("id",id) ) {
            ci=cur;
            break;
        }
    }
    if( ci ) {
        if( !rect.isEmpty() ) {
            ci->setRect(rect.x(),rect.y(), rect.width(), rect.height() );
        }
        xpopupItem=ci;
        update();
        return true;
    }
#endif
    return false;
}

bool GCanvas::showDiv(LPCC id, QRectF rect) {
#ifdef CANVAS_ITEM_USE
    if( xcanvasItems==NULL ) return false;
    CanvasItem* ci=NULL;
    for( int n=0,sz=xcanvasItems->childCount(); n<sz; n++ ) {
        CanvasItem* cur=xcanvasItems->child(n);
        if( cur->cmp("tag","popup") && cur->cmp("id",id) ) {
            ci=cur;
            break;
        }
    }
    if( ci ) {
        if( !rect.isEmpty() ) {
            ci->setRect(rect.x(),rect.y(), rect.width(), rect.height() );
        }
        xdivItem=ci;
        update();
        return true;
    }
#endif
    return false;
}

void GCanvas::hidePopup() {
    if( xpopupItem ) {
        xpopupItem=NULL;
        update();
    }
}
void GCanvas::hideDiv() {
    if( xdivItem ) {
        xdivItem=NULL;
        update();
    }
}

TreeNode* GCanvas::widgets() {
    TreeNode* root=static_cast<TreeNode*>(widgetConf());
    if( root ) {
        for( int n=0; n<root->childCount(); n++ ) {
            TreeNode* cur=root->child(n);
            if( cur && cur->cmp("tag","widgets") ) {
                return cur;
            }
        }
    }
    return NULL;
}

void GCanvas::updateCanvas(int frame ) {
    MyTimeLine* timeline=qobject_cast<MyTimeLine*>(sender());
    if( timeline ) {
        TreeNode* tn=timeline->info();
        StrVar* sv=tn->gv("@updateRect");
        if(SVCHK('i',5) || SVCHK('i',4)) {
            update(setQRect(sv));
        } else {
            update();
        }
    }
}

void GCanvas::slotTimeLineFinish() {
    xcurrentTimeline=NULL;
    update();
}



//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Widget UTIL
//
//////////////////////////////////////////////////////////////////////////////////////////////////

int DirFileInfo::setDir(U32 sortFlag, U32 fliterFlag) {
    dir.setFilter((QDir::Filters)fliterFlag);
    dir.setSorting((QDir::SortFlags)sortFlag);
    list=dir.entryInfoList();
    infoIndex=0;
    return list.size();
}
void DirFileInfo::nameFilters(StrVar* sv) {
    if( sv==NULL ) return;
    QStringList sa;
    XParseVar pv(sv,0,sv->length() );
    while( pv.valid() ) {
        LPCC type=pv.findEnd(",").v();
        sa.append(KR(type));
    }
    dir.setNameFilters(sa);
}
QFileInfo* DirFileInfo::next() {
    if( infoIndex< list.size() ) {
        info=list.at(infoIndex++);
        return &info;
    }
    return NULL;
}

GInputCompletion::GInputCompletion(QLineEdit *parent, TreeNode* cf, GTree* tree ) : QObject(parent), editor(parent), xcf(cf) {
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(200);
    connect(timer, SIGNAL(timeout()), SLOT(autoSuggest()));
    connect(editor, SIGNAL(textEdited(QString)), timer, SLOT(start()));

    if( tree->config() ) {
        StrVar* sv=tree->config()->gv("@treeMode");
        if( sv ) {
            sv->reuse();
        }
    }
    popup = tree;
    popup->setWindowFlags(Qt::Popup);
    popup->setFocusPolicy(Qt::NoFocus);
    popup->setMouseTracking(true);

    popup->setUniformRowHeights(true);
    popup->setRootIsDecorated(false);
    popup->setEditTriggers(QTreeWidget::NoEditTriggers);
    popup->setSelectionBehavior(QTreeWidget::SelectRows);
    popup->setFrameStyle(QFrame::Box | QFrame::Plain);
    popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popup->header()->hide();
    connect(popup, SIGNAL(clicked(QModelIndex)), this, SLOT(doneCompletion()) );
    connect(popup, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doneCompletion) );
    // connect(popup, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(doneCompletion()));
    popup->installEventFilter(this);

}
GInputCompletion::~GInputCompletion() {

}
void GInputCompletion::setPopup(GTree* tree) {
    if( tree==NULL ) return;
}

bool GInputCompletion::eventFilter(QObject *obj, QEvent *ev) {
    if( obj != popup )
        return false;

    if (ev->type() == QEvent::MouseButtonPress) {
        popup->hide();
        editor->setFocus();
        return true;
    }

    if (ev->type() == QEvent::KeyPress) {

        bool consumed = false;
        int key = static_cast<QKeyEvent*>(ev)->key();
        switch (key) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            doneCompletion();
            consumed = true;
        case Qt::Key_Escape:
            editor->setFocus();
            popup->hide();
            if( key==Qt::Key_Escape ) {
                editor->setText("");
            }
            consumed = true;
        case Qt::Key_Down:
        case Qt::Key_Up:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            break;

        default:
            editor->setFocus();
            editor->event(ev);
            popup->hide();
            break;
        }

        return consumed;
    }
    return false;
}


void GInputCompletion::showPopup() {
    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->setFocus();
    popup->show();
}
inline
void currentTreeNode(GTree* tree, TreeNode* node, StrVar* rst) {
    TreeNode* tn=tree->config();
    StrVar* sv=tn->gv("@model");
    rst->reuse();
    if( !SVCHK('m',0) ) {
        return;
    }
    TreeModel* m=(TreeModel*)SVO;
    TreeProxyModel* proxy=NULL;
    sv=tn->gv("@proxy");
    if( SVCHK('m',1) ) {
        proxy=(TreeProxyModel*)SVO;
    }
    if( node ) {
        QModelIndex src = m->getIndex(node);
        if( src.isValid() && proxy ) {
            QModelIndex index = proxy->mapFromSource(src);
            if( index.isValid() ) {
                tree->scrollTo(index);
                tree->setCurrentIndex(index);
            }
        }
    } else {
        QItemSelectionModel* im=tree->selectionModel();
        if( im==NULL ) {
            rst->setVar('3',0);
            return;
        }
        QModelIndex index = im->currentIndex();
        if( index.isValid() ) {
            TreeNode* node=static_cast<TreeNode*>(proxy->mapToSource(index).internalPointer());
            if( node ) {
                rst->setVar('n',0,(LPVOID)node );
            }
        }
    }
}

void GInputCompletion::doneCompletion() {
    if( popup==NULL ) return;
    timer->stop();
    LPCC text=NULL;
    TreeNode* cur=NULL;
    StrVar* sv=xrst.reuse();
    currentTreeNode(popup, NULL, sv);
    if( SVCHK('n',0) ) {
        cur=(TreeNode*)SVO;
        sv=cur->gv("text");
        if( sv ) text=toString(sv);
    }

    bool ok=false;
    if( sender() ) {
        QMetaMethod metaMethod = sender()->metaObject()->method(senderSignalIndex());
        /*
        if( ccmp(metaMethod.signature(),"clicked(QModelIndex)") ) {
            ok=true;
        }
        */
    }
    if( ok || !editor->text().isEmpty() ) {
        ok=true;
    } else {
        text=NULL;
    }
    popup->clearSelection();
    popup->hide();
    editor->setFocus();
    if( slen(text) ) {
        editor->setText(KR(text) );
    }
    QMetaObject::invokeMethod(editor, "returnPressed");
}

void GInputCompletion::autoSuggest() {
    if( popup==NULL ) return;

    const QPalette &pal = editor->palette();
    QColor color = pal.color(QPalette::Disabled, QPalette::WindowText);
    TreeNode* popupNode=popup->config();
    QString val=editor->text();
    int row=14;
    xparam.reuse();
    xparam.add()->set(Q2A(val));
    execMemberFunc( popupNode->gv("setFilter"), &xparam, xrst.reuse(), NULL, popupNode, "setFilter");
    if( isNumberVar(&xrst) ) {
        row=toInteger(&xrst);
    }
    if( row==0 ) {
        row=1;
    } else {
        StrVar* sv=popupNode->gv("@currentSelectUse");
        if( !SVCHK('3',1) ) {
            StrVar* sv=popupNode->gv("@proxy");
            if( SVCHK('m',1) ) {
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                QModelIndex idx = proxy->index(0,0);
                if( idx.isValid() ) {
                    popup->setCurrentIndex(idx);
                }
            }
        }
    }


    popup->resizeColumnToContents(0);
    popup->resizeColumnToContents(1);
    popup->adjustSize();
    popup->setUpdatesEnabled(true);

    int rowHeight=popup->sizeHintForRow(0);
    if( rowHeight<=0 ) rowHeight=28;
    int h = rowHeight * qMin(10, row) + 3;
    popup->resize(popup->width(), h);
    popup->move(editor->mapToGlobal(QPoint(0, editor->height())));
    popup->setFocus();
    popup->show();

}

void GInputCompletion::preventSuggest() {
    timer->stop();
}

void setEditorCharFormat(XHighlighter* syntax, LPCC code, TreeNode* cf, TreeNode* config, QWidget* widget ) {
    if( cf==NULL ) return;
    StrVar* sv=NULL;
    QTextCharFormat fmt;
    // U32 type=gum.getFlagType("SYNTAX",code);
    // qDebug()<<"SYNTAX code="<<code<< ", type="<<type;
    U32 type =
        ccmpl(code,6,"string") ? 51 :
        ccmpl(code,7,"comment") ? 61 :
        ccmpl(code,7,"command") ? 62 :
        ccmpl(code,5,"match") ? 71 :
        ccmpl(code,5,"block") ? 101 :
        ccmpl(code,4,"func") ? 11 : ccmpl(code,7,"subfunc") ? 11 : ccmpl(code,5,"event") ? 11 :
        ccmpl(code,3,"tag") ? 13 : 0;
    // string : 51, eps=>52, js=>53, java=>54
    // comment : js=>61, lineStart=>62
    fmt.setObjectType(type);

    sv=cf->gv("weight");
    if( isNumberVar(sv) ) {
        fmt.setFontWeight(toInteger(sv));
    } else if( sv) {
        LPCC c= toString(sv);
        if( ccmp(c,"bold") ) fmt.setFontWeight(75);
        else if( ccmp(c,"strong") ) fmt.setFontWeight(99);
    }
    sv=cf->gv("color");
    if( sv ) {
        fmt.setForeground( QBrush(getGlobalColor(sv)) );
    }
    sv=cf->gv("background");
    if( sv ) {
        fmt.setBackground( QBrush(getGlobalColor(sv)) );
    }
    sv=cf->gv("italic");
    if( sv ) {
        fmt.setFontItalic( isVarTrue(sv) );
    }
    sv=cf->gv("fontName");
    if( sv ) {
        LPCC name=toString(sv);
        fmt.setFontFamily(KR(name));
    }
    sv=cf->gv("fontSize");
    if( isNumberVar(sv) ) {
        fmt.setFontPointSize((qreal)toDouble(sv));
    }

    sv=cf->gv("keyword");
    XListArr* a=cfArray("@syntax", true);
    if( sv ) {
        XParseVar pv(sv);
        while(pv.valid() ) {
            pv.findEnd("|");
            LPCC key=pv.v();
            a->add()->set(key);
        }
    }
    LPCC ptn=toString(cf->gv("pattern"));
    if( type>50) {
        if( type==51 ) {
            LPCC mode=toString(cf->gv("mode"));
            if( ccmp(mode,"all") ) {
                type = 52;
            }
            XHighlightData* hd = new XHighlightData(fmt,type);
            syntax->xstringHighlightData = hd;
        } else if( type==71 ) {
            LPCC mode=toString(cf->gv("mode"));
            if( ccmp(mode,"each") ) {
                type = 72;
            }
            StrVar *sp=cf->gv("start"), *ep=cf->gv("end");
            if( sp && ep) {
                XHighlightData* hd = new XHighlightData(fmt, type);
                hd->setBlockPattern(toString(sp),toString(ep));
                syntax->xfmts.add(hd);
            }
        } else if( type>100 ) {
            type+= config->incrNum("@blockIdx",1);
            StrVar *sp=cf->gv("start"), *ep=cf->gv("end");
            if( sp && ep) {
                XHighlightData* hd = new XHighlightData(fmt, SYNTAX_BLOCK|type);
                hd->setBlockPattern(toString(sp),toString(ep));
                syntax->xblockFmts.add(hd);
            }
        } else if( type>60 ) {
            LPCC mode=toString(cf->gv("mode"));
            if( ccmp(mode,"command") ) {
                type = 62;
            }
            XHighlightData* hd = new XHighlightData(fmt,type);
            hd->exp.setPattern(KR(ptn));
            syntax->xcommentFmts.add(hd);
        }
    } else if( slen(ptn) ) {
        int size=a->size();
        if( size>0 ) {
            sv = cf->gv("case");
            for( int n=0, size=a->size();n<size;n++ ) {
                XHighlightData* hd = new XHighlightData(fmt,type);
                LPCC keyword = toString(a->get(n));
                hd->exp.setPattern(KR(ptn).arg(keyword));
                if( sv ) {
                    if( isVarTrue(sv) )
                        hd->exp.setCaseSensitivity(Qt::CaseSensitive);
                    else
                        hd->exp.setCaseSensitivity(Qt::CaseInsensitive);
                }
                syntax->xfmts.add(hd);
            }
        } else {
            XHighlightData* hd = new XHighlightData(fmt,type);
            hd->exp.setPattern(KR(ptn));
            syntax->xfmts.add(hd);
        }
    }
    // gtrees.removeArr(a);
}
