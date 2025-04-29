#include "func_util.h"
#include "widgets.h"
#include "../util/widget_util.h"
#include "../util/user_event.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QToolTip>
#include <QScrollArea>
#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>

void regWidgetFuncs() {
    if( isHashKey("widget") )
        return;
    PHashKey hash = getHashKey("widget");
    U16 uid = 1;
    hash->add("close", uid);				uid++;	//
    hash->add("hide", uid);				uid++;	//
    hash->add("enable", uid);			uid++;	//
    hash->add("show", uid);				uid++;	//
    hash->add("focus", uid);				uid++;	//
    hash->add("menu", uid);				uid++;	//
    hash->add("open", uid);				uid++;	//
	hash->add("setOverlay", uid);		uid++;	//
	hash->add("showOverlay", uid);		uid++;	//
	uid = 20;
    hash->add("rect", uid);				uid++;	// 20
    hash->add("globalVar", uid);		uid++;
    hash->add("pageNode", uid);             uid++;	//
    hash->add("visible", uid);			uid++;	//
    hash->add("cursor", uid);			uid++;	//
    uid++;	// 25
    hash->add("maxWidth", uid);			uid++;	//
    hash->add("maxHeight", uid);		uid++;	//
    hash->add("styleSheet", uid);		uid++;	//
    hash->add("tooltip", uid);			uid++;	//
    hash->add("flags", uid);			uid++;	// 30
    hash->add("icon", uid);				uid++;	//
    hash->add("pageState", uid);		uid++;	//
    uid++;	// // hash->add("opacity", uid);
    hash->add("title", uid);			uid++;	//
    hash->add("size", uid);				uid++;	// 35
    hash->add("offset", uid);			uid++;	//
    hash->add("mapGlobal", uid);		uid++;	//
    hash->add("resize", uid);			uid++;	//
    hash->add("alert", uid);			uid++;	//
    hash->add("timer", uid);			uid++;	//
    hash->add("killTimer", uid);		uid++;	//
    hash->add("cursorPos", uid);		uid++;	//
    hash->add("mouseTracking", uid);    uid++;
    hash->add("winId", uid);
    uid = 48;
    hash->add("fireEvent", uid);		uid++;	//
    hash->add("postEvent", uid);		uid++;	//

    uid = 50;
    hash->add("fixedHeight", uid);		uid++;	//
    hash->add("fixedWidth", uid);		uid++;	//
    hash->add("margin", uid);			uid++;	//
    hash->add("textSize", uid);			uid++;	//
    uid++;	// hash->add("pageValue", uid);
    uid++;	// hash->add("pageCallback", uid);

    uid = 60;
    hash->add("disable", uid);			uid++;	//
    uid++;	//
    hash->add("minSize", uid);			uid++;	//
    hash->add("maxSize", uid);			uid++;	//
    uid++;	// hash->add("layout", uid);
    uid++;	// hash->add("update", uid);
    uid = 71;
    hash->add("findWidget", uid);       uid++;	//
    hash->add("findTag", uid);			uid++;	//
    hash->add("palette", uid);			uid++;	//
    hash->add("updateWidget", uid);     uid++;	//
    hash->add("widgets", uid);			uid++;	//
    hash->add("widget", uid);			uid++;	//
    uid++;	// hash->add("key", uid);

    uid = 100;
    hash->add("parentWidget", uid);		uid++;	//
    hash->add("active", uid);			uid++;	//
    hash->add("action", uid);			uid++;	// 106
    hash->add("selectFolder", uid);		uid++;	//
    hash->add("expandWidth", uid);		uid++;	//
    hash->add("expandHeight", uid);		uid++;	//
    hash->add("tray", uid);				uid++;	//
    uid = 110;
    hash->add("selectFile", uid);		uid++;	//
    hash->add("readSetting", uid);		uid++;	//
    hash->add("writeSetting", uid);		uid++;	//
    hash->add("createWidget", uid);			uid++;	//
    uid++;	// hash->add("config", uid);
    uid++;	// hash->add("modal", uid);
    hash->add("geo", uid);				uid++;	//
    uid++;	// hash->add("selectFiles", uid);
    hash->add("fontWidth", uid);			uid++;	//
    hash->add("font", uid);				uid++;	//
    hash->add("confirm", uid);			uid++;	//
    uid = 121;
    uid++;	// hash->add("delay", uid);
    hash->add("redraw", uid);			uid++;	//
    hash->add("acceptDrops", uid);		uid++;	//
    hash->add("dragProcess", uid);			uid++;	//
    hash->add("activate", uid);			uid++;	//
    hash->add("setScroll", uid);			uid++;	//
    hash->add("findLayout", uid);		uid++;	//
    hash->add("touchScroll", uid);		uid++;
    uid++;	// hash->add("attr", uid);
    uid++;	//
    uid++;	// hash->add("dialog", uid);
    uid++;	//
    uid++;	// hash->add("pageRef", uid);
    hash->add("destroy", uid);			uid++;	//
    hash->add("showTop", uid);			uid++;	//
    hash->add("transBackground", uid);	uid++;	//
    hash->add("getColor", uid);			uid++;	//
    uid++;  // hash->add("eventFuncs", uid);
    hash->add("opacity", uid);			uid++;	//
    hash->add("actionPage", uid);		uid++;	//
    uid=171;
    uid++;  // hash->add("pos", uid);
    hash->add("move", uid);				uid++;
    hash->add("is", uid);				uid++;
    uid=891;
    hash->add("eventMap", uid);			uid++;
}
inline TreeNode* findLayoutTreeNode(TreeNode* node) {
    while( node ) {
        node=node->parent();
        if( node==NULL )
            break;
        if( node->xstat==LTYPE_GRID || node->xstat==LTYPE_HBOX || node->xstat==LTYPE_VBOX  ) return node;
    }
    return NULL;
}
bool isWidgetCheck(LPCC ty, QWidget* w, StrVar* rst) {
    if( ccmp(ty,"disable") )		rst->setVar('3', w->isEnabled()? false: true);
    else if( ccmp(ty,"enable") )	rst->setVar('3', w->isEnabled()? true: false);
    else if( ccmp(ty,"visible") )	rst->setVar('3', w->isVisible()? true: false);
    else if( ccmp(ty,"active") )	rst->setVar('3', w->isActiveWindow() ? true: false);
    else if( ccmp(ty,"full") || ccmp(ty,"fullscreen") ) rst->setVar('3', w->isFullScreen() ? true: false);
    else if( ccmp(ty,"max") || ccmp(ty,"maximized") ) rst->setVar('3', w->isMaximized() ? true: false);
    else if( ccmp(ty,"min") || ccmp(ty,"minimized") ) rst->setVar('3', w->isMinimized() ? true: false);
    else if( ccmp(ty,"hidden") )	rst->setVar('3', w->isHidden() ? true: false);
    else if( ccmp(ty,"modal") )		rst->setVar('3', w->isModal() ? true: false);
    else if( ccmp(ty,"toplevel") )	rst->setVar('3', w->isTopLevel() ? true: false);
    else if( ccmp(ty,"focusWidget") )	{
        GInput* input=qobject_cast<GInput*>(w->focusWidget());
        if( input ) {
            rst->setVar('n',0,(LPVOID)input->config());
        } else {
            rst->setVar('3', 0);
        }
    } else if( ccmp(ty,"bold") ) {
        QFont font=w->font();
        rst->setVar('3', font.bold() );
    } else {
        return false;
    }
    return true;
}


XFuncNode* setEventMapUserFunction(TreeNode* cf, XParseVar& pv, XFunc* fc, LPCC eventName, XFuncNode* fn, StrVar* rst, XFunc* fcParent) {
    if( rst==NULL || fc==NULL ) {
        return NULL;
    }
    XFuncNode* fnCur=NULL;
    StrVar* sv=NULL;
    if( fc->getType()==FTYPE_CALLBACK ) {
        makeLocalFunction(fc, false, fn, rst, NULL);
        sv=rst;
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            fnCur=setCallbackFunction(fsrc, fn, cf, NULL, cf->GetVar(eventName) );
        }
    } else {
        char c=0;
        rst->reuse();
        pv.SetPosition();
        c=pv.ch();
        // &func()
        bool bSubFunc=false;
        if( c=='&' ) {
            c=pv.incr().ch();
        }
        if( c=='@' ) {
            bSubFunc=true;
        }
        LPCC userFunc=pv.findEnd("(").v();
        if( bSubFunc ) {
            LPCC funcNm=userFunc+1;
            int pos=IndexOf(funcNm, '.');
            if( pos>0 ) {
                getSubFuncSrc(gBuf.add(funcNm,pos), funcNm+pos+1 );
            }
        }

        /*
        if( ccmp(eventName,"onDraw") ) {
            if( cf->xstat==WTYPE_COMBO ) {
                rst->add("@draw, @index, @state");
            } else if( cf->xstat==WTYPE_TREE ) {
                rst->add("@draw, @node, @over");
            } else {                
                rst->add("@draw");
            }
        } else if( ccmp(eventName,"onDrawHandle") ) {
            rst->add("@draw");
        } else if( ccmp(eventName,"onChange") ) {
            if( cf->xstat==WTYPE_TREE ) {
                rst->add("@node");
            } else if( cf->xstat==WTYPE_COMBO ) {
                rst->add("");
            } else if( cf->xstat==WTYPE_TAB || cf->xstat==WTYPE_DIV ) {
                rst->add("@index, @page");
            }
        } else if( ccmp(eventName,"onStateChange") ) {
            rst->add("@state, @value");
        } else if( ccmp(eventName,"onMessage") ) {
            rst->add("@line");
        } else if( ccmp(eventName,"onKeyDown") ) {
            rst->add("@key, @mode");
        } else if( ccmp(eventName,"onAction") ) {
            rst->add("@id, @action");
        } else if( ccmp(eventName,"onContextMenu") ) {
            rst->add("@pos");
        } else if( ccmp(eventName,"onMouseDown") || ccmp(eventName,"onMouseUp") || ccmp(eventName,"onMouseMove") || ccmp(eventName,"onDoubleClick") || ccmp(eventName,"onMouseClick") ) {
            rst->add("@pos, @mode, @button");
        } else if( ccmp(eventName,"onDragStart")  ) {
            if( cf->xstat==WTYPE_TREE ) {
                rst->add("@nodes");
            } else {
                rst->add("@pos");
            }
        } else if( ccmp(eventName,"onDrag")  ) {
            rst->add("@type, @data");
        } else if( ccmp(eventName,"onDragMove") || ccmp(eventName,"onDrop") ) {
            rst->add("@pos, @data");
        } else if( ccmp(eventName,"onMouseWheel") ) {
            rst->add("@pos, @delta, @mode");
        } else if( ccmp(eventName,"onEvent") ) {
            rst->add("@type, @node");
        } else if( ccmp(eventName,"onMove")  ) {
            rst->add("@pos");
        } else if( ccmp(eventName,"onPopup") || ccmp(eventName,"onPopupEnd") ) {
            rst->add("@tree");
        } else {
            if( cf->xstat==WTYPE_TREE ) {
                if( ccmp(eventName,"onChildData") || ccmp(eventName,"onFetchMore") || ccmp(eventName,"onFilter") ) {
                    rst->add("@node");
                } else if( ccmp(eventName,"onClicked") || ccmp(eventName,"onDoubleClicked") ) {
                    rst->add("@node, @column");
                } else if( ccmp(eventName,"onRowSize") ) {
                    rst->add("@node, @column");
                } else if( ccmp(eventName,"onEditEvent") ) {
                    rst->add("type, node, data, index");
                } else if( ccmp(eventName,"onDrawHeader") ) {
                    // version 1.0.3
                    rst->add("@draw, @index, @sortIndex, @order, @text");
                } else if( ccmp(eventName,"onSort") ) {
                    rst->add("@left, @right, @index");
                } else if( ccmp(eventName,"onTip") ) {
                    rst->add("@node, @index");
                }
            } else if( cf->xstat==WTYPE_EDIT || cf->xstat==WTYPE_TEXTEDIT ) {
                if( ccmp(eventName,"onContentChange") ) {
                    rst->add("@pos, @add, @remove");
                } else if( ccmp(eventName,"onLineNumDraw") ) {
                    rst->add("draw, rect, lineNum, startPos, endPos");
                } else if( ccmp(eventName,"onLineNumMouse") ) {
                    rst->add("pos, mode, button, type, range");
                }
            }
        }
        */
        LPCC params=pv.findEnd(")").v();
        rst->set("return ").add( userFunc ).add("(").add(params).add(")");
        qDebug("#0#id=%s eventMap : %s\n", cf->get("id"), rst->get() );
        XFuncSrc* fsrc= gfsrcs.getFuncSrc();
        fsrc->parseData(rst,0,rst->length());
        // ### version 1.0.4
        if( fsrc->makeFunc() ) {
            fsrc->xparam.set(params);
            fnCur=setCallbackFunction(fsrc, fn, cf, NULL, cf->GetVar(eventName) );
        } else {
            qDebug("#9#eventMap apply error (id=%s eventMap:%s)\n", cf->get("id"), rst->get() );
        }
    }
    return fnCur;
}


void writeSetting(LPCC type, LPCC code, LPCC key, LPCC def, StrVar* rst, QWidget* w ) {
    QSettings settings(KR(type), KR(code));
    if( ccmp(code,"position") && w ) {
        settings.setValue("pos", w->pos());
        settings.setValue("size", w->size());

        if( w->windowState()==Qt::WindowMaximized ) {
            settings.setValue(KR("state"),KR("maximized"));
        } else {
            settings.setValue(KR("state"),KR("saved"));
        }
    } else {
        if( key ) {
            LPCC val = def;
            settings.setValue(KR(key),KR(val) );
        }
    }
}
void readSetting(LPCC type, LPCC code, LPCC key, LPCC def, StrVar* rst, QWidget* w ) {
    QSettings settings(KR(type), KR(code));
    if( ccmp(code,"position") && w ) {
        QDesktopWidget dt;
        QPoint pos = settings.value("pos", QPoint(0, 0)).toPoint();
        QSize size = settings.value("size", QSize(800, 600)).toSize();
        QString val = settings.value("state").toString();
        LPCC state = gBuf.add(Q2A(val));
        if( slen(def) && slen(state)==0 ) {
            state = def;
            w->setWindowState(
                ccmp(state,"minimized")? Qt::WindowMinimized : ccmp(state,"maximized") ? Qt::WindowMaximized :
                ccmp(state,"fullscreen")? Qt::WindowFullScreen : ccmp(state,"active")? Qt::WindowActive : Qt::WindowNoState);
        } else if( ccmp(state,"saved") ) {
            int num = dt.screenNumber(pos);
            QRect rc = dt.screenGeometry(num);
            qDebug()<<"readSetting rect ====== "<<rc;
            if( rc.isValid() && (rc.x()<pos.x() && rc.right()>pos.x()) && (rc.y()<pos.y() && rc.bottom()>pos.y()) ) {
                w->move(pos);
                w->resize(size);
            } else {
                rc = QApplication::desktop()->screenGeometry();
                if( rc.width()<size.width() || rc.height()<size.height() ) {
                    size.setWidth(rc.width()-100);
                    size.setHeight(rc.height()-100);
                }
                QPoint pt = rc.center();
                w->move( pt.x()-(size.width()*0.5), pt.y()-(size.height()*0.5) );
                w->resize(size);
            }
        } else if( slen(state) ) {
            w->setWindowState(
                ccmp(state,"minimized")? Qt::WindowMinimized : ccmp(state,"maximized") ? Qt::WindowMaximized :
                ccmp(state,"fullscreen")? Qt::WindowFullScreen : ccmp(state,"active")? Qt::WindowActive : Qt::WindowNoState);
        }
    } else if( key ) {
        QString val = settings.value(key).toString();
        if( val.isNull() || val.isEmpty() ) {
            if( slen(def) ) rst->set(def);
        } else {
            rst->set( Q2A(val) );
        }
    }
}

inline int toKeySequenceKey(QString const & str) {
    QKeySequence seq(str);
    int keyCode=0;

    // We should only working with a single key here
    if( seq.count() == 1 )
        keyCode = seq[0];
    else {
        // Should be here only if a modifier key (e.g. Ctrl, Alt) is pressed.
        assert(seq.count() == 0);

        // Add a non-modifier key "A" to the picture because QKeySequence
        // seems to need that to acknowledge the modifier. We know that A has
        // a keyCode of 65 (or 0x41 in hex)
        seq = QKeySequence(str + "+A");
        if( seq.count() == 1 && seq[0] > 65 ) {
            keyCode = seq[0] - 65;
        }
    }

    return keyCode;
}
void setActionValue(GAction* act, TreeNode* cur) {
    LPCC text=cur->get("text");
    if( slen(text) ) act->setText(KR(text));
    QPixmap* icon=NULL;
    StrVar* sv=cur->gv("icon");
    if( SVCHK('i',6) ) {
        icon=(QPixmap*)SVO;
    } else if( sv && sv->length() ) {
        icon=getStrVarPixmap(sv, true );
    }
    if( icon ) {
        act->setIcon(QIcon(*icon));
    }
    sv=cur->gv("tooltip");
    if( sv ) {
        LPCC tooltip = toString(sv);
        act->setToolTip(KR(tooltip));
    }
    sv=cur->gv("shortcut");
    if( sv ) {
        LPCC shortcut=toString(sv);
        int num=toKeySequenceKey(KR(shortcut));
        if( num ) {
            act->setShortcut(QKeySequence(num));
        }
    }
}

GAction* getAction(LPCC id, TreeNode* cf, bool overwrite) {
    if( slen(id)==0 )
        return NULL;
    GAction* act = NULL;    
    StrVar* sv=getStrVar("action",id);
    if( SVCHK('w',11) ) {
        act=(GAction*)SVO;
    }
    if( act==NULL ) {
        if( cf==NULL )
            return NULL;
        act = new GAction(cf);
        getStrVar("action",id, 'w',11,(LPVOID)act );
        setActionValue(act, cf);        
    }
    if( cf ) {
        cf->GetVar("@action")->setVar('w',11,(LPVOID)act);
        cf->xstat=WTYPE_ACTION;
    }    
    return act;
}

bool callWidgetFunc(TreeNode* cf, XFunc* fc, QWidget* w, PARR arrs, XFuncNode* fn, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;
    if( w==NULL )
        return false;
    if( rst->charAt(0)=='\b')
        rst->reuse();
    switch(fc->xfuncRef) {
    case 1: {	// close        
        QPoint pt=w->pos();
        QSize sz=w->size();
        if( arrs ) {
            cf->GetVar("@close")->setVar('i',4).addInt(pt.x()).addInt(pt.y()).addInt(sz.width()).addInt(sz.height());
        } else {
            cf->GetVar("@close")->reuse();
        }
        w->close();
    } break;
    case 2: {	// hide
        if( arrs ) {
            if( fc && fc->xfuncRef ) fc->xfuncRef=0;
            return false;
        }
        w->hide();
    } break;
    case 3: {	// enable
        if( cnt==0 ) {
            w->setEnabled(true);
        } else if( cnt==1 ) {
            w->setEnabled(isVarTrue(arrs->get(0)));
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 4: {	// show
        if( arrs ) {
            if( fc && fc->xfuncRef ) fc->xfuncRef=0;
            return false;
        }
        w->show();
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 5: {	// focus
        if( arrs ) {
            if( fc && fc->xfuncRef ) fc->xfuncRef=0;
            return false;
        }
        w->setFocus();
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 6: {	// menu
        if( cnt==0 ) {
            return false;
        }
        int idx=0;
        StrVar* sv=arrs->get(idx++);       
        TreeNode* pageNode=NULL;        
        TreeNode* menuNode=cf->addNode("@menuNode");
        GMenu menu(menuNode);
        menuNode->GetVar("@menuObject")->setVar('w',12,(LPVOID)&menu);
        if( SVCHK('n',0)) {
            pageNode=(TreeNode*)SVO;
            sv=arrs->get(idx++);
        }
        if( pageNode==NULL ) {
            pageNode=findPageNode(cf);
            if( pageNode==NULL ) {
                sv=fn->gv("@this");
                if( SVCHK('n',0)) {
                    pageNode=(TreeNode*)SVO;
                } else {
                    pageNode=cf;
                }
            }
        }
        if(SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            makeNodeChildMenus(node, &menu, pageNode);
        } else if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            makeChildMenus(a,&menu,pageNode);
        }
        sv=arrs->get(idx++);
        if( SVCHK('i',2) ) {
            menu.exec( w->mapToGlobal(QPoint(sv->getInt(4),sv->getInt(8))) );
        } else if( SVCHK('i',20) ) {
            QPoint pt( (int)sv->getDouble(4), (int)sv->getDouble(4+sizeof(double)) );
            menu.exec( w->mapToGlobal(pt) );
        } else if( isNumberVar(sv) ) {
            QPoint pt=w->cursor().pos();
            int y=toInteger(sv);
            pt.setY( pt.y()+y);
            menu.exec(pt);
        } else {
            menu.exec(w->cursor().pos());
        }
        if( menuNode ) {
            sv=menuNode->gv("@menuObject");
            if( sv ) sv->reuse();
        }
    } break;
    case 115:
    case 7: {	// open
        if( cnt==0 ) {
            if( cf->cmp("tag","dialog") ) {
                GDialog* dialog = qobject_cast<GDialog*>(w);
                if( dialog ) {
                    dialog->exec();
                    return true;
                }
            } else {
                w->show();
            }
            rst->setVar('n',0,(LPVOID)cf);
            return true;
        }
        w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
        w->raise();
        w->activateWindow();
        w->setWindowState(Qt::WindowActive);
        StrVar* sv=cf->gv("@prevPosition");
        if( SVCHK('i',4) ) {
            w->move( QPoint(sv->getInt(4),sv->getInt(8)) );
            w->resize( QSize(sv->getInt(12), sv->getInt(16)) );
            w->show();
            return true;
        }
        sv=cf->gv("@ok");
        if( sv ) {
            sv->reuse();
        }
        // open(style or rect, callback, parent);
        QWidget* widget=NULL;
        LPCC sty=NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* cur=(TreeNode*)SVO;
            cur=findPageNode(cur);
            widget=gwidget(cur);
            sty=AS(1);
        } else if( SVCHK('i',5) || SVCHK('i',20) ) {
            int sz=sizeof(double);
            int x=(int)sv->getDouble(4), y=(int)sv->getDouble(4+sz);
            w->move(x,y);
            if( SVCHK('i',5) ) {
                int wid=sv->getDouble(4+(sz*2)), hei=sv->getDouble(4+(sz*3));
                w->resize(wid,hei);
            }
        } else if( SVCHK('i',4) || SVCHK('i',2) ) {
            int x = sv->getInt(4), y=sv->getInt(8);
            w->move(x,y);
            if( SVCHK('i',4) ) {
                int wid=sv->getInt(12), hei=sv->getInt(16);
                w->resize(wid,hei);
            }
        } else if(sv) {
            sty=toString(sv);
        }
        if( ccmp(sty,"center") ) {
            if( widget ) {
                QRect geo = widget->geometry();
                QPoint center = geo.center();                
                w->adjustSize();
                int x=center.x()-(w->width()*0.5),y = center.y()-(w->height()*0.5);
                qDebug()<<geo<<" "<<center<<" "<<w->geometry()<<" "<<x<<" "<<y;
                w->move(x, y+12 );
            } else {
                // int cnt = QApplication::desktop()->screenCount();
                QRect rc = QApplication::desktop()->screenGeometry(QCursor::pos());
                w->move(rc.center() - w->rect().center());
            }
        }
        if( cf->cmp("tag","dialog") ) {
            GDialog* dialog = qobject_cast<GDialog*>(w);
            if( dialog ) {
                dialog->exec();
                return true;
            }
        } else {
            w->show();
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
	case 8: {	// setOverlay
		GWidget* widget=static_cast<GWidget*>(w);
		if(widget) {
			int delay=arrs? toInteger(arrs->get(0)): 1000;
			widget->openOverlay(delay);
		}
	} break;
	case 9: {	// showOverlay
		GWidget* widget=static_cast<GWidget*>(w);
		if(widget) {
			bool show=arrs? isVarTrue(arrs->get(0)): true;
			widget->showOverlay(show);
		}
	} break;
	case 20: {	// rect
        bool chk=arrs && isVarTrue(arrs->get(0))? true: false;
        if( chk ) {
            setVarRect(rst, w->rect());
        } else {
            setVarRectF(rst, w->rect());
        }
    } break;
    case 21: {	// globalVar
        if( cnt==0 ) {
            return true;
        }
        TreeNode* pageNode=findPageNode(cf);
        if( pageNode ) {
            StrVar* sv=cf->gv("onInit");
            if( SVCHK('f',0) ) {
                XFuncNode* fn=(XFuncNode*)SVO;
                LPCC key=AS(0);
                if( cnt==1 ) {
                    sv=fn->gv(key);
                    rst->add(sv);
                } else {
                    int sp=0,ep=0;
                    sv=getStrVarPointer(arrs->get(1),&sp,&ep);
                    fn->GetVar(key)->set(sv, sp, ep);
                }
            }
        }
    } break;
    case 22: {	// pageNode
        TreeNode* pageNode=findPageNode(cf);
        if( pageNode ) {
            rst->setVar('n',0,(LPVOID)pageNode);
        }
    } break;
    case 23: {	// visible
        if( cnt==0 ) {
            rst->setVar('3',w->isVisible()?1:0 );
        } else {
            w->setVisible(isVarTrue(arrs->get(0)));
            rst->setVar('n',0,(LPVOID)cf);
        }
    } break;
    case 24: {	// cursor
        if( cnt==0 ) {
            rst->setVar('0',0).addInt((int)w->cursor().shape());
        } else if( isNumberVar(arrs->get(0)) ) {
            int num = toInteger(arrs->get(0));
            w->setCursor(QCursor((Qt::CursorShape)num));
            rst->setVar('n',0,(LPVOID)cf);
        }
    } break;
    case 26: if(cnt==0) rst->setVar('3',w->maximumWidth()); else w->setMaximumWidth(toInteger(arrs->get(0))); break;
    case 27: if(cnt==0) rst->setVar('3',w->maximumHeight()); else w->setMaximumHeight(toInteger(arrs->get(0))); break;
    case 28: {	// styleSheet
        if( cnt==0 ) {
            rst->set(Q2A(w->styleSheet()));
        } else {
            LPCC style=AS(0);
            w->setStyleSheet(KR(style));
            rst->setVar('n',0,(LPVOID)cf);
        }
    } break;
    case 29: {	// tooltip
        if( cnt==0 ) {
            rst->set(Q2A(w->toolTip()));
            return true;
        }
        LPCC text=AS(0);
        if( text==NULL ) text="";
        if( cnt==2 ) {
            StrVar* sv=arrs->get(1);
            if( SVCHK('i',2) ) {
                QPoint pt(sv->getInt(4), sv->getInt(8));
                QToolTip::showText( pt, KR(text), w );
            } else if( SVCHK('i',20) ) {
                int sz=sizeof(double);
                QPoint pt((int)sv->getDouble(4), (int)sv->getDouble(4+sz));
                QToolTip::showText( pt, KR(text), w );
            } else {
                if( SVCHK('3',0) ) {
                    w->setToolTip(KR(text));
                    QToolTip::hideText();
                } else {
                    QToolTip::showText( QCursor::pos(), KR(text), w );
                }
            }
        } else {
            w->setToolTip(KR(text));
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 30: {	// flags
        if( cnt==0 ) {
            rst->setVar('0',0).addInt((int)w->windowFlags());
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            U32 flag = (U32)toInteger(arrs->get(0));
            w->setWindowFlags((Qt::WindowFlags)flag);
            return true;
        }
        StrVar* sv=arrs->get(1);
        qtSetWindowFlags(w, arrs->get(0), SVCHK('3',1) );
        rst->setVar('n',0,(LPVOID)cf);
    } break;

    case 31: {	// icon
        if( cnt==0 ) return false;
        QPixmap* p = getStrVarPixmap(arrs->get(0));
        if( p ) {
            w->setWindowIcon(QIcon(*p));
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 32: {	// pageState
        if( cnt==0 ) {
            switch( w->windowState() ) {
            case Qt::WindowMinimized :	rst->set("minimized"); break;
            case Qt::WindowMaximized :	rst->set("maximized"); break;
            case Qt::WindowFullScreen :	rst->set("fullscreen"); break;
            case Qt::WindowActive :		rst->set("active"); break;
            }
            return true;
        }
        LPCC state = toString(arrs->get(0));
        GMainPage* main = qobject_cast<GMainPage*>(w);
        if( main ) {
            if( ccmp(state,"fullscreen") ) {
                main->menuBar()->hide();
                QList<QToolBar *> toolbars = main->findChildren<QToolBar *>();
                for( int n=0;n<toolbars.size();n++) {
                    toolbars.at(n)->hide();
                }
            } else if( ccmp(state,"active") ) {
                main->menuBar()->show();
                QList<QToolBar *> toolbars = main->findChildren<QToolBar *>();
                for( int n=0;n<toolbars.size();n++) {
                    toolbars.at(n)->show();
                }
            }
        }
        w->setWindowState(
            ccmp(state,"minimized")? Qt::WindowMinimized : ccmp(state,"maximized") ? Qt::WindowMaximized :
            ccmp(state,"fullscreen")? Qt::WindowFullScreen : ccmp(state,"active")? Qt::WindowActive : Qt::WindowNoState);
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 33: {	//
    } break;
    case 34: {	// title
        if( cnt==0 ) return false;
        LPCC title = toString(arrs->get(0));
        if( slen(title) ) {
            cf->set("title", title);
            w->setWindowTitle(KR(title));
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 35: {	// size
        if( cnt==0 ) {
            // QRect rc = w->rect();
            QSize sz = w->size();
            rst->setVar('i',2).addInt(sz.width()).addInt(sz.height());
            return true;
        }
        if( cnt==1 ) {
            StrVar* sv = arrs->get(0);
            if( SVCHK('i',2) ) {
                w->resize(sv->getInt(4), sv->getInt(8));
            } else if( SVCHK('i',5) ) {
                int sz=sizeof(double);
                w->resize((int)sv->getDouble(4+(2*sz)), (int)sv->getDouble(4+(3*sz)));
            } else if( SVCHK('i',4) ) {
                w->resize(sv->getInt(12), sv->getInt(16));
            } else {
                QSize sz = w->size();
                w->resize(toInteger(arrs->get(0)), sz.height());
            }
        } else if( cnt==2 ) {
            QSize sz = w->size();
            int wid=toInteger(arrs->get(0)), hei=toInteger(arrs->get(1));
            if( wid==-1) wid=sz.width();
            if( hei==-1) hei=sz.height();
            w->resize(wid,hei);
            rst->setVar('n',0,(LPVOID)cf);
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 36: {	// offset
        QWidget* widget=NULL;
        StrVar* sv = NULL;
        int idx=0;
        if( cnt==0 ) {
            widget = gwidget(findWidgetNode(cf,0));
        } else {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                widget = gwidget((TreeNode*)SVO);
                idx++;
            } else if( SVCHK('i',20) || SVCHK('i',5) || SVCHK('i',2) || SVCHK('i',4) ) {
                widget = gwidget(findWidgetNode(cf,0));
            }
        }
        if( widget ) {
            QPoint ptBase=QPoint(0,0);
            sv=arrs->get(idx);
            if( SVCHK('i',2) || SVCHK('i',4) ) {
                ptBase.setX(sv->getInt(4)), ptBase.setY(sv->getInt(8));
            } else if( SVCHK('i',20) || SVCHK('i',5) ) {
                ptBase.setX((int)sv->getDouble(4)), ptBase.setY((int)sv->getDouble(4+sizeof(double)));
            }
            QPoint pt = w->mapFrom(widget, ptBase);
            rst->setVar('i',2).addInt(pt.x()).addInt(pt.y());
            // qAbs(pt.x())).addInt(qAbs(pt.y()));
            return true;
        }
        rst->setVar('i',2).addInt(0).addInt(0);
    } break;
    case 37: {	// mapGlobal
        if( cnt==0 ) {
            QPoint pt = w->mapToGlobal(QPoint(0,0));
            rst->setVar('i',2).addInt(qAbs(pt.x())).addInt(qAbs(pt.y()));
            return true;
        }
        StrVar* sv = arrs->get(0);
        if( SVCHK('i',2) || SVCHK('i',4) || SVCHK('i',5) || SVCHK('i',20) ) {
            QPoint pt = w->mapToGlobal(QPoint(0,0));
            int cx=pt.x(), cy=pt.y(), cw=0, ch=0;
            if( SVCHK('i',2) || SVCHK('i',4) ) {
                int rx=sv->getInt(4), ry=sv->getInt(8);
                if( isVarTrue(arrs->get(1)) ) {
                    cx=rx, cy=ry;
                } else {
                    cx+=rx, cy+=ry;
                }
                if( SVCHK('i',4) ) {
                    cw=sv->getInt(12), ch=sv->getInt(16);
                }
            } else {
                int sz=sizeof(double);
                int rx=(int)sv->getDouble(4), ry=(int)sv->getDouble(4+sz);
                if( isVarTrue(arrs->get(1)) ) {
                    cx=rx, cy=ry;
                } else {
                    cx+=rx, cy+=ry;
                }
                if( SVCHK('i',5) ) {
                    cw=(int)sv->getDouble(4+(2*sz)), ch=(int)sv->getDouble(4+(3*sz));
                }
            }
            if( SVCHK('i',2) || SVCHK('i',20) )
                rst->setVar('i',20).addDouble(cx).addDouble(cy);
            else
                rst->setVar('i',5).addDouble(cx).addDouble(cy).addDouble(cw).addDouble(ch);
        } else if( isVarTrue(sv) ) {
            QPoint pt = w->mapFromGlobal(QCursor::pos());
            rst->setVar('i',2).addInt(pt.x()).addInt(pt.y());
        } else {
            rst->setVar('i',2).addInt(0).addInt(0);
        }
    } break;
    case 38: {	// visible
        if( cnt==0 ) return false;
        w->setVisible(true);
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 39: {	// alert
        if( cnt==0 ) {
            return true;
        }
        // version 1.0.3
        rst->reuse();
        StrVar* sv=arrs->get(0);
        LPCC data=toString(sv);
        QString str=KR(data);
        if( str.length()>512 ) {
            str=str.mid(0,512);
            str.append("...");
            rst->add(Q2A(str));
        } else {
            rst->add(data);
        }
        QString msg=KR(rst->get());
        int icon=2;
        LPCC title=toString(confVar("alert.title"));
        sv=arrs->get(1);
        if( sv) title=toString(sv);
        sv=arrs->get(2);
        if( isNumberVar(sv)) icon=toInteger(sv);
        QMessageBox mb(w);
        mb.setIcon((QMessageBox::Icon)icon);
        mb.setText(msg);
        if( slen(title)>0 ) mb.setWindowTitle(KR(title));
        mb.exec();
        rst->reuse();
    } break;
    case 40: {	// timer
        XListArr* arr=cf->addArray("@timers");
        if( cnt==0 ) {
            rst->setVar('a',0,(LPVOID)arr);
            return true;
        }
        StrVar* sv=arrs->get(0);
        int interval = 0;
        if( isNumberVar(sv) ) {
            if(cnt==1) {
                interval = toInteger(sv);
            } else if(cnt==2) {
                interval = toInteger(sv);
                sv=arrs->get(1);
                if(SVCHK('f',1) && interval>0 ) {
                    int tid=w->startTimer(interval);
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    qDebug("#0#single timer tid=%d", tid);
                    cf->GetVar(FMT("@timer_%d",tid))->setVar('f',1,(LPVOID)fsrc);
                    cf->setNodeFlag(FLAG_RUN|FLAG_SINGLE|FLAG_SETEVENT);
                    return true;
                }
            }
        }
        if( interval>0 ) {
            int tid=w->startTimer(interval);            
            rst->setVar('0',0).addInt(tid);
            arr->add()->setVar('0',0).addInt(tid);
            cf->setNodeFlag(FLAG_RUN|FLAG_SETEVENT);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 41: {	// killTimer
        XListArr* arr=cf->addArray("@timers");
        if( cnt==0 ) {
            for(int n=0, sz=arr->size(); n<sz; n++ ) {
                int tid=toInteger(arr->get(n));
                if(tid>0 ) {
                    w->killTimer(tid);
                }
            }
            arr->reuse();
            return true;
        }
        int tid=toInteger(arrs->get(0));
        if( tid>0 ) {
            int idx=-1;
            for(int n=0, sz=arr->size(); n<sz; n++ ) {
                if(tid==toInteger(arr->get(n))) {
                    idx=n;
                    break;
                }
            }
            w->killTimer(tid);
            if( idx!=-1) {
                arr->remove(idx);
            }
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 42: {	// cursorPos
         QPoint pt = w->cursor().pos();
         rst->setVar('i',20).addDouble(pt.x()).addDouble(pt.y());
    } break;
    case 43: {	// mouseTracking
        if(cnt==0 || isVarTrue(arrs->get(0)) ) {
            w->setMouseTracking(true);
        } else {
            w->setMouseTracking(false);
        }
    } break;
    case 44: {	// winId
        WId wid=w->winId();
        HWND hwnd = (HWND)wid; //reinterpret_cast<long>();
        rst->setVar('1',0).addUL64((UL64)hwnd);

    } break;
    case 48: {	// fireEvent
        if( cnt==1 ) {
            XParseVar pv(arrs->get(0));
            LPCC next=pv.NextWord();
            TreeNode* cur=findWidgetId(cf,next);
            if( cur && pv.ch()=='.' ) {
                next=pv.incr().NextWord();
                StrVar* sv=cur->gv(next);
                if( SVCHK('f',0) ) {
                    XFuncNode* fn=(XFuncNode*)SVO;
                    fn->call();
                }
            }
        }
    } break;
    case 49: {	// postEvent
        // ### version 1.0.4
        if( cnt==0 ) return false;
        XFuncNode* fnCur=NULL;
        TreeNode* eventNode=NULL;
        LPCC type=NULL;
        StrVar* sv=arrs->get(0);
        // postEvent type, node|class, func
        //           type, func
        type=toString(sv);
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            eventNode=(TreeNode*)SVO;
            sv=arrs->get(2);
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                fnCur=setCallbackFunction(fsrc, fn);
                sv=eventNode->gv("@useClass");
                if( SVCHK('3',1)) {
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)eventNode);
                }
            }
        } else {
            eventNode=cf;
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                fnCur=setCallbackFunction(fsrc, fn);
            }
        }
        eventNode->set("eventType", type);
        rst->set(type);
        UserEvent* ue=new UserEvent(eventNode,getCf()->incrNum("@userEventIndex"),rst);
        if( fnCur ) {
            ue->setCallback(fnCur);
        }
        QApplication::postEvent(w, ue);
    } break;
    case 118: {	// fontWidth
        LPCC txt=arrs? AS(0):NULL;
        if( slen(txt) ) {
            rst->setVar('0',0).addInt( w->fontMetrics().width(KR(txt)) );
        } else {
            rst->setVar('0',0).addInt(0);
        }
    } break;
    case 119: {	// font
        StrVar* sv=arrs ? arrs->get(0): NULL;
        QFont font=w->font();
        if( sv==NULL ) {
            // setFontInfo(&font);
            TreeNode* info=getFontInfo(NULL, NULL);
            rst->setVar('n',0,(LPVOID)info);
        } else if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            QWidget* widget=gwidget(node);
            if( widget ) {
                w->setFont(widget->font());
            }
            return true;
        } else {
            if( isVarTrue(arrs->get(1)) ) setFontInfo(&font);
            getFontInfo(&font, toString(sv));
            w->setFont(font);
        }
    } break;
    case 120: {	// confirm
        if( cnt==0 ) {
            return true;
        }
        LPCC msg=AS(0), title=AS(1), info=AS(2);
        if( slen(title)==0 ) {
            title=toString(confVar("confirm.title"));
        }
        // int ret=confirm(msg,info,title);
        QMessageBox msgBox(w);
        if( slen(title)==0 ) title="confirm";
        msgBox.setWindowTitle(KR(title));
        msgBox.setText(KR(msg));
        if( slen(info) ) msgBox.setInformativeText(KR(info));
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();
        rst->setVar('3',ret==QMessageBox::Ok?1:0);
    } break;
    case 50: {	// fixedHeight
        if( arrs ) w->setFixedHeight(toInteger(arrs->get(0)));
    } break;
    case 51: {	// fixedWidth
        if( arrs ) w->setFixedWidth(toInteger(arrs->get(0)));
    } break;
    case 52: {	// margin
        if( cnt==4 ) {
            w->setContentsMargins(toInteger(arrs->get(0)),toInteger(arrs->get(1)),toInteger(arrs->get(2)),toInteger(arrs->get(3)));
        } else if(cnt==1 ) {
            int m = toInteger(arrs->get(0));
            w->setContentsMargins(m,m,m,m);
        }
    } break;
    case 53: {	// textSize
        if( cnt==0 ) return false;
        QFontMetrics fm(w->font());
        LPCC txt = AS(0);
        if( isVarTrue(arrs->get(1)) )
            rst->setVar('i',2).addInt(fm.width(KR(txt))).addInt( fm.height());
        else
            rst->setVar('0',0).addInt(fm.width(KR(txt)));
    } break;
    case 54: {	// pageValue

    } break;
    case 55: {	// pageCallback

    } break;
    case 60: {	// disable
        w->setEnabled( arrs && !isVarTrue(arrs->get(0)) ? true: false);
    } break;
    case 62: {	// minSize
        if( cnt==0 )
            return false;
        int wid = toInteger(arrs->get(0));
        if( cnt==1 ) {
            w->setMinimumWidth(wid);
        } else if( cnt==2 ) {
            if( wid==0 ) {
                w->setMinimumHeight( toInteger(arrs->get(1)) );
            } else {
                w->setMinimumSize(wid,toInteger(arrs->get(1)) );
            }
        }
    } break;
    case 63: {	// maxSize
        if( cnt==0 )
            return false;
        int wid = toInteger(arrs->get(0));
        if( cnt==1 ) {
            w->setMaximumWidth(wid);
        } else if( cnt==2 ) {
            if( wid==0 ) {
                w->setMaximumHeight( toInteger(arrs->get(1)) );
            } else {
                w->setMaximumSize(wid,toInteger(arrs->get(1)) );
            }
        }
    } break;
    case 71: {	// findWidget
        if( cnt==0 ) {
            return false;
        }
        LPCC id=AS(0);
        StrVar* sv=arrs->get(1);
        rst->setVar('9',0);
        if( SVCHK('3',1)) {
            XListArr* arr=getListArrTemp();
            findWidgetTag(cf,id, arr);
            if( arr->size() ) rst->setVar('a',0,(LPVOID)arr);
        } else {
            int mod=isNumberVar(arrs->get(1))? toInteger(arrs->get(1)): 0;
            TreeNode* cur=findWidgetId(cf, id, mod);
            if( cur==NULL && mod==0 ) {
                cur=findWidgetTag(cf,id, NULL);
            }
            if(cur) {
                rst->setVar('n',0, (LPVOID)cur);
            }
        }
    } break;
    case 72: {	// findTag
        if( cnt==0 ) {
            return false;
        }
        LPCC tag=AS(0);
        XListArr* arr=getListArrTemp();
        findWidgetTag(cf,tag, arr);
        if( arr->size() ) {
            if( arr->size()==1 ) {
                rst->add(arr->get(0));
            } else {
                rst->setVar('a',0, (LPVOID)arr);
            }
        } else {
            rst->setVar('9',0);
        }
    } break;
    case 73: {	// palette
        if( cnt==0 ) {
            return false;
        } else {
            QPixmap* img=getStrVarPixmap(arrs->get(0));
            if( img ) {
                QPalette* palette = new QPalette();
                palette->setBrush(QPalette::Background, QBrush(*img) );
                w->setPalette(*palette);
            }
        }
    } break;
    case 74: {	// updateWidget
        if( cnt==0 ) {
            w->update();
        } else {
            QRectF rc=getQRectF(arrs->get(0));
            if( rc.isValid() ) {
                w->update(rc.toRect());
            }
        }
    } break;
    case 75: {	// widgets
        // ### version 1.0.4
        U32 flag=0;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            flag=(U32)toInteger(arrs->get(0));
        }
        XListArr* a=getListArrTemp();
        getWidgetArr(cf, a );
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 76: {	// widget
        if( cnt==0 )
            return false;
        LPCC id=AS(0);
        TreeNode* node=findWidgetId(cf,id);
        if( node ) rst->setVar('n',0,(LPVOID)node);
        return true;
    } break;
    case 77: {	// key
    } break;
    case 100: {	// parentWidget
        TreeNode* p=NULL;
        StrVar* sv=cf->gv("@parent");        
        if( SVCHK('n',0) ) {
            p=(TreeNode*)SVO;
        } else {
            p = cf->parent();
        }
        QWidget* widget=gwidget(p);
        if( cnt==0 ) {
            if( widget )
                rst->setVar('n',0,(LPVOID)p);
            else
                rst->setVar('3',0);
        } else {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                QWidget* parentWidget=gwidget( (TreeNode*)SVO );
                if( parentWidget ) {
                    w->setParent(parentWidget);
                    cf->GetVar("@parent")->reuse()->add(sv);
                }
            } else if( SVCHK('9',0) ) {
                w->setParent(NULL);
                sv=cf->gv("@parent");
                if( sv ) sv->reuse();
            }
        }
    } break;
    case 101: {	// active
        if( w ) w->activateWindow();
    } break;
    case 102: {	// action
        StrVar* sv= NULL;
        if( cnt==0 ) {
            sv = cf->gv("@actions");
            if( SVCHK('n',0) ) {
                TreeNode* root=(TreeNode*)SVO;
                rst->setVar('n',0,(LPVOID)root);
            }
            return true;
        }

        TreeNode* pageNode=cf;
        if( pageNode==NULL ) {
            sv=fn->gv("@this");
            if( SVCHK('n',0) ) {
                pageNode=(TreeNode*)SVO;
            } else {
                pageNode=cf;
            }
        }
        TreeNode* actions=getTreeNode("action","nodes", true);
        sv = arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            for( int n=0,sz=a->size();n<sz;n++ ) {
                sv=a->get(n);
                if( SVCHK('n',0) ) {
                    GAction* act = NULL;
                    TreeNode* cur=(TreeNode*)SVO;
                    LPCC actionId=cur->get("id");
                    TreeNode* actionNode=actions->getNode(actionId);
                    if( actionNode ) {     // SVCHK('w',11) ) {
                        sv=actionNode->gv("@action");
                        if( SVCHK('w',11) ) {
                            act = (GAction*)SVO;
                            setActionValue(act, cur);
                        }
                    }
                    if(act==NULL) {
                        if( actionNode==NULL ) {
                            actionNode=actions->addNode(actionId);
                        }
                        act = new GAction(actionNode);
                        actionNode->clone(cur,true);
                        // actionNode->set("id", actionId);
                        actionNode->GetVar("@action")->setVar('w',11,(LPVOID)act);
                        actionNode->xstat=WTYPE_ACTION;
                        setActionValue(act, cur);
                    }
                    actionNode->GetVar("@page")->setVar('n',0,(LPVOID)pageNode);
                }
            }
        }
    } break;
    case 103: {	// selectFolder
        LPCC title = cnt ? AS(0): "Select Folder";
        LPCC def= cnt? AS(1): NULL;
        if( slen(def)==0 )  {
            StrVar* sv=getCf()->gv("defaultPath");
            if( sv ) {
                def=sv->get();
            } else {
                def=toString(confVar("setup.defaultPath" ));
                if( slen(def) ) getCf()->GetVar("defaultPath")->set(def);
            }
        }
        QString path = QFileDialog::getExistingDirectory(
            w,
            KR(title),
            slen(def) ? KR(def): QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks );

        if( path.length()>0 ) {
            StrVar* sv=getCf()->GetVar("defaultPath");
            sv->set(Q2A(path));
            if( !ccmp(def,sv->get()) ) {
                confVar("setup.defaultPath", sv, true);
            }
            rst->reuse()->add(sv);
        }
    } break;
    case 104: {	// expandWidth
        if( cnt==0 )
            w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        else if( cnt==1)
            w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    } break;
    case 105: {	// expandSize
        if( cnt==0 )
            w->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);
        else if( cnt==1 )
            w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
        else if( cnt==2)
            w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    } break;
    case 106: {	// tray
        GTray* tray = NULL;
        StrVar* sv = cf->gv("@tray");
        if( SVCHK('w',21) ) {
            tray = (GTray*)SVO;
        } else {
            tray = new GTray(cf,w);
            // TreeNode* h =new TreeNode(2, true);
            // setClassFuncNode(h,fn,'n',NULL,FNSTATE_TRAY);
            cf->GetVar("@tray")->setVar('w',21,(LPVOID)tray);
        }
        if( tray ) {
            rst->setVar('w',21,(LPVOID)tray);
        }
    } break;

    case 110: {	// selectFile
        if( cnt==1 && checkFuncObject(arrs->get(0),'n',0) ) {
            // filter, defaultPath,
        } else {
            // filter "Music files (*.mp3);;Text files (*.txt);;All files (*.*)"...
            // defualt Text files (*.txt)
            StrVar* defaultPath = getCf()->GetVar("defaultPath");
            if( defaultPath->length()==0 ) {                
                LPCC def=toString(confVar("setup.defaultPath"));
                if( slen(def) ) defaultPath->set(def);
            }
            if( cnt==2 ) {
                LPCC filter = AS(1);
                if( slen(filter)==0 || ccmp(filter,"all") ) {
                    filter = "All files (*.*)";
                }
                QStringList files=QFileDialog::getOpenFileNames(w,KR(AS(0)),
                    defaultPath->length()? KR(defaultPath->get()): QDir::currentPath(),
                    KR(filter));
                if( files.size()==1 ) {
                    rst->set(Q2A(files.at(0)));
                    LPCC path = rst->get();
                    int pos = LastIndexOf(path,'/');
                    if( pos>0 ) {
                        getCf()->GetVar("defaultPath")->set(path,pos);
                    }
                } else if( files.size()>1 ) {
                    XListArr* a=getListArrTemp();
                    for( int n=0,sz=files.size();n<sz;n++) {
                        a->add()->set(Q2A(files.at(n)));
                    }
                    rst->setVar('a',0,(LPVOID)a);
                }
            } else {
                LPCC title = arrs ? AS(0): "save file select";
                QString fileName = QFileDialog::getSaveFileName(w,
                        KR(title),
                        defaultPath->length()? KR(defaultPath->get()): QDir::currentPath(),
                        cnt>1? KR(AS(1)): QString(), NULL);
                if( fileName.length() ) {
                    QFileInfo fi(fileName);
                    QString path=fi.absolutePath();
                    rst->set( getCf()->GetVar("defaultPath")->set(Q2A(path)).get() );
                    confVar("setup.defaultPath", rst, true );
                }
                rst->set(Q2A(fileName));
            }
        }
    } break;
    case 111: { // readSetting
        if( cnt==0 ) return false;
        LPCC code = AS(1);
        if( ccmp(code,"position") ) {
            readSetting(AS(0), code, code, AS(2), rst, w);
        } else {
            readSetting(AS(0), AS(1), AS(2), AS(3), rst, w);
        }
    } break;
    case 112: { // writeSetting
        if( cnt==0 ) return false;
        LPCC code = AS(1);
        if( ccmp(code,"position") ) {
            writeSetting(AS(0), code, code, AS(2), rst, w);
        } else {
            writeSetting(AS(0), AS(1), AS(2), AS(3), rst, w);
        }
    } break;
    case 113: { // createWidget
        if( cnt==0 ) return false;
        StrVar* sv = arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* wcf=(TreeNode*)SVO;
            LPCC tag=wcf->get("tag");
            createWidget(tag, wcf, w);
            rst->setVar('n',0,(LPVOID)wcf);
        }
    } break;
    case 116: { // geo
        if( w ) {
            StrVar* sv = arrs? arrs->get(0): NULL;
            if( SVCHK('i',4) ) {
                w->setGeometry(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16));
            } else if( SVCHK('i',5) ) {
                int sz=sizeof(double);
                w->setGeometry((int)sv->getDouble(4),(int)sv->getDouble(4+sz),(int)sv->getDouble(4+(2*sz)),(int)sv->getDouble(4+(3*sz)));
            } else if( cnt==4 ) {
                w->setGeometry(toInteger(arrs->get(0)),toInteger(arrs->get(1)),toInteger(arrs->get(2)),toInteger(arrs->get(3)));
            } else {
                QRect r = w->geometry();
                rst->setVar('i',5).addDouble(r.x()).addDouble(r.y()).addDouble(r.width()).addDouble(r.height());
            }
        }
    } break;
    case 121: { // delay
        if( cnt==0 ) return true;
    } break;
    case 122: { // redraw
        if(cnt==0 ) {
            w->update();
        } else {
            StrVar* sv=arrs->get(0);
            if( SVCHK('i',4) || SVCHK('i',5) ) {
                w->update(setQRect(sv));
            } else {
                w->update();
            }
        }
    } break;
    case 123: { // acceptDrops
        bool chk=cnt==0||isVarTrue(arrs->get(0)) ? true: false;
        w->setAcceptDrops(chk);
    } break;
    case 124: { // dragProcess
        if( cnt==0 ) return false;
        // 0: data, 1: icon, 2: callback
        StrVar* sv = arrs->get(2);
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* cfn=gfns.getFuncNode(fsrc->xfunc, fn);
            if( cfn ) {
                QDrag drag(w);
                QMimeData* mime=NULL;
                LPCC ty = NULL;
                sv = arrs->get(0);
                if( SVCHK('m',2) ) {
                    mime = (QMimeData*)SVO;
                } else {
                    ty = "application/nodeData";
                    QByteArray ba;
                    mime = new QMimeData();
                    ba.append(sv->get(), sv->length());
                    mime->setData(KR(ty), ba);
                }
                cf->GetVar("@drag")->setVar('m',3,(LPVOID)&drag);
                drag.setMimeData(mime);
                QPixmap* pix = getStrVarPixmap( arrs->get(1));
                if( pix ) {
                    drag.setPixmap(*pix);
                }
                sv = arrs->get(3);
                if( SVCHK('i',2) ) {
                    drag.setHotSpot(QPoint(sv->getInt(4), sv->getInt(8)) );
                }
                XParseVar pv(&(fsrc->xparam));
                arrs->reuse();
                while( pv.valid() ) {
                    pv.findEnd(",");
                    arrs->add()->add(pv.v());
                }
                ty = AS(0);
                if( drag.exec(Qt::CopyAction | Qt::MoveAction ) ) {
                    cfn->GetVar(ty)->set("ok");
                } else {
                    cfn->GetVar(ty)->set("ignor");
                }
                cfn->call();
                cf->GetVar("@drag")->reuse();
                // fnCur->clearNodeFlag(FLAG_PERSIST);
                funcNodeDelete(cfn);
                // gfns.deleteFuncNode(cfn);
            }
        }
    } break;
    case 125: { // activate
        if( cnt==0 || isVarTrue(arrs->get(0)) ) {
            w->setWindowState(w->windowState() & ~Qt::WindowMinimized);
            w->raise();
            w->activateWindow();
            w->setWindowState(Qt::WindowActive);
            rst->setVar('3', w->isActiveWindow() ? 1: 0);
        } else {
            w->setWindowState(Qt::WindowMinimized);
        }
    } break;
    case 126: {	// setScroll
        setScroll(cf);
        rst->reuse()->add(cf->gv("@scroll"));
    } break;
    case 127: { // findLayout
        TreeNode* node = findLayoutTreeNode(cf);
        if( node ) rst->setVar('n',0,(LPVOID)node);
    } break;
    case 128: { // touchScroll
        // w->setAttribute(Qt::WA_TranslucentBackground,true);
        FlickCharm* fc=NULL;
        StrVar* sv=cf->GetVar("@touchScroll");
        if( SVCHK('w',23) ) {
            fc=(FlickCharm*)SVO;
        } else {
            fc=new FlickCharm();
            sv->setVar('w',23,(LPVOID)fc);
        }
        QScrollArea* scroll=NULL;
        sv=cf->gv("@scroll");
        if( SVCHK('w',20)) {
            scroll=(QScrollArea*)SVO;
        }
        if(scroll) {
            if( cnt==0 || isVarTrue(arrs->get(0)) ) {
                cf->GetVar("@touchScrollUse")->setVar('3',1);
                fc->activateOn(scroll);
            } else {
                cf->GetVar("@touchScrollUse")->setVar('3',0);
                fc->deactivateFrom(scroll);
            }
        } else {
            qDebug("#0#not define scroll widget (touch mode fail)\n");
        }
    } break;
    case 134: { // destroy
        if( cf->isArray("@timers") ) {
            XListArr* arr=cf->addArray("@timers");
            for(int n=0, sz=arr->size(); n<sz; n++ ) {
                int tid=toInteger(arr->get(n));
                if(tid>0 ) {
                    w->killTimer(tid);
                }
            }
            arr->reuse();
        }
        w->setAttribute( Qt::WA_DeleteOnClose );
        w->close();
        /*
        QEventLoop loop;
        w->connect(w, SIGNAL(destroyed()), &loop, SLOT(quit()));
        loop.exec();
        */
        TreeNode* parent=cf->parent();
        if( parent ) {
            parent->remove(cf);
        }
        for( WBoxNode* bn=cf->First(); bn; bn=cf->Next() ) {
            LPCC name=cf->getCurCode();
            StrVar* sv=&(bn->value);
            if( SVCHK('f',0) ) {
                XFuncNode* fnEvent=(XFuncNode*)SVO;
                fnEvent->clearNodeFlag(FLAG_PERSIST);
                funcNodeDelete(fnEvent);
            }
        }
        cf->reset(true);
        w->deleteLater();
    } break;
    case 135: { // showTop
#ifdef WINDOW_USE
        bool top=true;
        if( arrs ) {
            top=isVarTrue(arrs->get(0));
        }

        if( top )
            SetWindowPos((HWND)w->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); //SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
        else
            SetWindowPos((HWND)w->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

        LONG lStyle = GetWindowLong((HWND)w->winId(), GWL_STYLE);
        LONG lExStyle = GetWindowLong((HWND)w->winId(), GWL_EXSTYLE);

        lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
        lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

        SetWindowLong((HWND)w->winId(), GWL_STYLE, lStyle);
        SetWindowLong((HWND)w->winId(), GWL_EXSTYLE, lExStyle);

#else
        Qt::WindowFlags flags = this->windowFlags();
        if (checked) {
            this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
            this->show();
        } else {
            this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
            this->show();
        }
#endif
    } break;
    case 136: { // transBackground

        QPalette p = w->palette();
        p.setColor(QPalette::Base, Qt::transparent);
        w->setStyleSheet("background: transparent");  // background-color: rgba(0,0,0,0)
        w->setPalette(p);
        w->setAttribute(Qt::WA_TranslucentBackground, true);
        w->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        qDebug("... label transBackground ok");
    } break;
    case 137: { // getColor
        QColor color;
        StrVar* sv=arrs ? arrs->get(0): NULL;
        if( sv ) {
            if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
                color=getQColor(sv);
            }
        }
        QColor clr = QColorDialog::getColor(color, w);
        rst->setVar('i',3).addInt(clr.red()).addInt(clr.green()).addInt(clr.blue());
        if( clr.alpha()>=0 ) rst->addInt(clr.alpha());
    } break;
    case 138: { // eventFuncs        
    } break;
    case 139: { // opacity
        if( cnt==0 ) {
            rst->setVar('4',0).addInt(w->windowOpacity());
        } else {
            int opa = toInteger(arrs->get(0));
            // int n=toInteger(arrs->get(0));
            if( isVarTrue(arrs->get(1)) ) {
                int op=w->windowOpacity()*100;
                op+=opa;
                qreal val=(qreal)(op/100.0);
                w->setWindowOpacity(val);
            } else {
                qreal val=(qreal)(opa/100.0);
                w->setWindowOpacity(val);
            }
        }
    } break;
    case 140: { // actionPage

    } break;
    case 172: {	// move
        if( cnt==0 ) {
            QPoint pt = w->pos();
            rst->setVar('i',2).addInt(pt.x()).addInt(pt.y());
            return true;
        }
        if( cnt==1 ) {
            StrVar* sv=arrs->get(0);
            if( SVCHK('i',4) ) {
                // w->resize(sv->getInt(12), sv->getInt(16));
                w->setGeometry( sv->getInt(4), sv->getInt(8), sv->getInt(12), sv->getInt(16) );
            } else if( SVCHK('i',2) ) {
                w->move(sv->getInt(4), sv->getInt(8));
            } else if( SVCHK('i',20) || SVCHK('i',5) ) {
                int sz=sizeof(double);
                int cx=(int)sv->getDouble(4), cy=(int)sv->getDouble(4+sz);
                if( SVCHK('i',20) ) {
                    w->move(cx, cy);
                } else {
                    int cw=(int)sv->getDouble(4+(2*sz)), ch=(int)sv->getDouble(4+(3*sz));
                    w->setGeometry( cx, cy, cw, ch);
                }
                sv=arrs->get(1);
                if( SVCHK('3',1) ) {
                    w->show();
                }
            } else {
                LPCC ty=toString(sv);
                if( ccmp(ty,"center") ) {
                    QPoint center = QApplication::desktop()->availableGeometry(w).center();
                    w->move(max(0.0,center.x()-(w->width()*0.5)), max(0.0,center.y()-(w->height()*0.5)) );
                }
            }
        } else if( cnt==2 ) {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                w->move(toInteger(sv),toInteger(arrs->get(1)));
            } else if( SVCHK('n',0) ) {
                QWidget* widget = gwidget((TreeNode*)SVO);
                if( widget ) {
                    QPoint center = widget->geometry().center();
                    w->move(max(0.0,center.x()-(w->width()*0.5)), max(0.0,center.y()-(w->height()*0.5)) );
                }
            }
        }
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 173: { // is
        if( cnt==0 ) {
            rst->set("visible, disable, enable, hidden, active, full, max, min, model, topleval");
            return true;
        }
        LPCC ty=AS(0);
        /*
        if( ccmp(ty,"ok") || ccmp(ty,"cancel") ) {
            StrVar* sv=cf->gv("@buttonStatus");
            rst->reuse();
            if( sv ) {
                LPCC val=toString(sv);
                rst->setVar('3', ccmp(ty,val)? 1: 0);
            }
            return true;
        }
        */
        if( isWidgetCheck(ty, w, rst) ) {
            return true;
        } else {
            return false;
        }
    } break;
    case 891: { // eventMap
        XFunc* param=fc->getParam(0);
        if( param && param->xftype==FTYPE_VAR ) {
            LPCC eventNm=param->getValue();
            if( slen(eventNm)==0 ) return true;
            XFunc* func=fc->getParam(1);
            if( func ) {
                setEventMapUserFunction(cf, func->xpv, func, eventNm, fn, rst, fc);
            }
        }
    } break;
    default:
        return false;
    }
    return true;
}

TreeNode* findPageNode(TreeNode* node) {
    if( node==NULL ) return NULL;
    for(int n=0; n<16 && node; n++ ) {
        if( isPageNode(node) ) return node;
        if( node->parent() ) {
            node=node->parent();
        } else {
            return node;
        }
    }
    return NULL;
}

bool callInitFunc(TreeNode* cur) {
    if( cur==NULL) {
        return false;
    }
    StrVar* sv=cur->gv("onInit");
    if(!cur->cmp("tag","worker") ) {
        cur->setNodeFlag(FLAG_START);
    }
    if( SVCHK('f',0) ) {
        XFuncNode* fnInit=(XFuncNode*)SVO;
        getCf()->set("@currentFuncName","onInit");
        fnInit->setNodeFlag(FLAG_PERSIST);
        fnInit->call();
        cur->setNodeFlag(FLAG_INIT);
        /*
        for( WBoxNode* bn=cur->First(); bn; bn=cur->Next() ) {
            sv=&(bn->value);
            if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                if( fnInit!=fnCur ) {
                    LPCC code=cur->getCurCode();
                    int pos = IndexOf(code,'.');
                    if( pos>0 ) {
                        LPCC wid = gBuf.add(code,pos);
                        TreeNode* sub=findWidgetId(cur,wid);
                        if( sub ) {
                            code+=pos+1;
                            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)sub);
                            fnCur->GetVar("@page")->setVar('n',0,(LPVOID)cur);
                            sub->GetVar(code)->setVar('f',0,(LPVOID)fnCur);
                        }
                    }
                    fnCur->xparent=fnInit;
                }
                fnCur->setNodeFlag(FLAG_PERSIST);
            }
        }
        */

        return true;
    }
    return false;
}

bool parsePageVar(TreeNode* root, StrVar* var, int sp, int ep, TreeNode* base ) {
    XParseVar pv(var, sp, ep);
    char c=pv.ch();
    sp=pv.start;
    if( c=='<' ) {
        LPCC tag=pv.incr().NextWord();
        pv.start=sp;
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag)) ) {
            if( !root->cmp("tag",tag) ) {
                root->set("tag", tag);
            }
            sp=parseProp(root, var, pv.prev,pv.cur);
            if( sp==-1 ) {
                _LOG("#9#parsePageVar error pageNode: %s\n", root->log() );
                return false;
            }
            return parsePageNode(root, var, sp, pv.cur, base);
        }
    }
    return false;
}

bool callPageFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GPage* page, XFunc* fc) {
    if( page==NULL ) {
        U32 ref = fc ? fc->xfuncRef: 0;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"readFile") ? 1 :
                ccmp(fnm,"readBuffer") ? 2 :
                ccmp(fnm,"create") ? 3 :
                ccmp(fnm,"createPage") ? 4 :
                0;
            if( fc ) fc->xfuncRef=ref;
        }
        if( ref==1 ) {  // readFile
            LPCC fileName=AS(0);
            if( fileReadAll(fileName, rst) ) {
                if( parsePageVar(tn, rst) ) {
                    rst->setVar('n',0,(LPVOID)tn);
                }
            } else {

            }
        } else if( ref==2 ) {   // readBuffer
            int sp=0,ep=0;
            StrVar* sv=arrs->get(0);
            sv=getStrVarPointer(sv, &sp, &ep);
            if( parsePageVar(tn, sv, sp, ep, tn) ) {
                rst->setVar('n',0,(LPVOID)tn);
            }
        } else if( ref==3 ) {   // create
            QWidget* parent=NULL;
            StrVar* sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                parent=gwidget((TreeNode*)SVO);
            }
            if( createWidget("page", tn, parent ) ) {
                rst->setVar('n',0,(LPVOID)tn);
            }
        } else if( ref==4 ) { // createPage
            QWidget* parent=NULL;
            StrVar* sv=arrs->get(1);
            if( SVCHK('n',0) ) {
                parent=gwidget((TreeNode*)SVO);
            }
            LPCC fileName=AS(0);
            if( fileReadAll(fileName, rst) ) {
                if( parsePageVar(tn, rst) ) {
                    LPCC tag=tn->get("tag");
                    if( slen(tag) ) {
                        tag="page";
                        tn->set("tag", tag);
                    }
                    if( createWidget(tag, tn, parent ) ) {
                        rst->setVar('3',1);
                    }
                    rst->setVar('n',0,(LPVOID)tn);
                }
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}


bool qtSetWindowFlags(QWidget* w, StrVar* sv, bool badd) {
    int sp=0, ep=0;
    StrVar* var=getStrVarPointer(sv, &sp, &ep);
    XParseVar pv(var, sp, ep);
    U32 flag = w->windowFlags();
    if( badd==false ) {
        flag&=~( Qt::Popup | Qt::Window | Qt::Sheet | Qt::Tool | Qt::ToolTip | Qt::SplashScreen | Qt::Dialog | Qt::Drawer );
    }

    while(pv.valid() ) {
        LPCC type=pv.findEnd(",").v();
        if( ccmp(type,"child") ) {
            w->setWindowFlags( Qt::Window | Qt::FramelessWindowHint );
            return true;
        } else if( ccmp(type,"childTop") ) {
            qDebug("... windows flag childTop ...\n");
            w->setWindowFlags( Qt::Window | Qt::FramelessWindowHint| Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint );
            return true;
        }
        if(ccmp(type,"fullscreen")|| ccmp(type,"minimized")|| ccmp(type,"maximized")|| ccmp(type,"active") ) {
            qtSetWindowState(w,type);
            return true;
        }
        if( badd ) {
            if( ccmp(type,"popup") ) flag|= Qt::Popup;
            else if( ccmp(type,"window") ) flag|=Qt::Window;
            else if( ccmp(type,"sheet") ) flag|=Qt::Sheet;
            else if( ccmp(type,"tool") ) flag|=Qt::Tool;
            else if( ccmp(type,"tooltip") ) flag|=Qt::ToolTip;
            else if( ccmp(type,"splash") ) flag|=Qt::SplashScreen;
            else if( ccmp(type,"dialog") ) flag|=Qt::Dialog;
            else if( ccmp(type,"frameless") ) flag|= Qt::FramelessWindowHint;
            else if( ccmp(type,"top") ) flag|= Qt::WindowStaysOnTopHint;
            else if( ccmp(type,"bottom") ) flag|= Qt::WindowStaysOnBottomHint;
            else if( ccmp(type,"maximize") ) flag |= Qt::WindowMaximizeButtonHint;
            else if( ccmp(type,"minimize") ) flag|=Qt::WindowMinimizeButtonHint;
            // else if( ccmp(type,"soft") ) flag|= Qt::WindowSoftkeysVisibleHint;
            else if( ccmp(type,"nobutton") ) flag &=~(Qt::WindowMaximizeButtonHint|Qt::WindowMinimizeButtonHint);
            else if( ccmp(type,"title") ) flag |= Qt::WindowTitleHint;
            else if( ccmp(type,"notitle") ) flag &=~Qt::WindowTitleHint;
            else if( ccmp(type,"custom") ) flag |= Qt::CustomizeWindowHint;
            else flag|= Qt::Window;
        } else {
            if( ccmp(type,"popup") ) flag&=~Qt::Popup;
            else if( ccmp(type,"window") ) flag|=Qt::Window;
            else if( ccmp(type,"sheet") ) flag|=Qt::Sheet;
            else if( ccmp(type,"tool") ) flag|=Qt::Tool;
            else if( ccmp(type,"tooltip") ) flag|=Qt::ToolTip;
            else if( ccmp(type,"splash") ) flag|=Qt::SplashScreen;
            else if( ccmp(type,"dialog") ) flag|=Qt::Dialog;
            else if( ccmp(type,"frameless") ) flag&=~Qt::FramelessWindowHint;
            else if( ccmp(type,"top") ) flag&=~Qt::WindowStaysOnTopHint;
            else if( ccmp(type,"bottom") ) flag&=~Qt::WindowStaysOnBottomHint;
            else if( ccmp(type,"maximize") ) flag &=~Qt::WindowMaximizeButtonHint;
            else if( ccmp(type,"minimize") ) flag|=Qt::WindowMinimizeButtonHint;
            else if( ccmp(type,"title") ) flag &=~Qt::WindowTitleHint;
        }
    }
    w->setWindowFlags((Qt::WindowFlags)flag);
    return true;
}

void qtSetWindowState(QWidget* w, LPCC state) {
    w->setWindowState(
        ccmp(state,"minimized")? Qt::WindowMinimized : ccmp(state,"maximized") ? Qt::WindowMaximized :
        ccmp(state,"fullscreen")? Qt::WindowFullScreen : ccmp(state,"active")? Qt::WindowActive : Qt::WindowNoState);
}
