#include "func_util.h"
#include "tree_model.h"
#include "../util/widget_util.h"
#include <QApplication>
#include <QScrollArea>

inline void clearLayout(QLayout* layout, bool deleteWidgets = true)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}
bool callLayoutFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    QWidget* wd = NULL;
    int cnt=arrs? arrs->size() : 0;
    bool call=true;
    if( ccmp(fnm,"hideAll") || ccmp(fnm,"showAll") ) {
        XListArr* a=NULL;
        StrVar* sv=arrs ? arrs->get(0) : NULL;
        if( SVCHK('a',0) ) {
            a=(XListArr*)SVO;
        } else if( sv ) {
            int sp=0, ep=0;
            sv=getStrVarPointer(sv, &sp, &ep);
            XParseVar pv(sv, sp, ep);
            a=getListArrTemp();
            while( pv.valid() ) {
                pv.findEnd(",");
                a->add()->add(pv.v());
            }
        }
        if( ccmp(fnm,"hideAll") ) {
            showHideWidgets(tn, false, a);
        } else {
            showHideWidgets(tn, true, a);
        }
        rst->setVar('a',0,(LPVOID)a);
    } else if( ccmp(fnm,"hide") ) {
        QLayout* box = qobject_cast<QLayout*>(getLayout(tn));
        if( box ) {
            wd = qobject_cast<QWidget*>(box->parent());
            if( wd ) wd->hide();
        }
    } else if( ccmp(fnm,"show") ) {
        QLayout* box = qobject_cast<QLayout*>(getLayout(tn));
        if( box ) {
            wd = qobject_cast<QWidget*>(box->parent());
            if( wd ) wd->show();
        }
    } else if( ccmp(fnm,"margin") ) {
        if( cnt==0 ) return false;
        QLayout* box = qobject_cast<QLayout*>(getLayout(tn));
        int size = arrs->size();
        if( box ) {
            if( size==1 ) {
                int num = toInteger(arrs->get(0));
                box->setMargin(num);
            } else if( size==4 ) {
                box->setContentsMargins(toInteger(arrs->get(0)), toInteger(arrs->get(1)), toInteger(arrs->get(2)), toInteger(arrs->get(3)) );
            }
            rst->setVar('n',0,(LPVOID)tn);
        }
    } else if( ccmp(fnm,"spacing") ) {
        if( cnt==0 ) return false;
        switch(tn->xstat) {
        case LTYPE_GRID: {
            QGridLayout* grid = qobject_cast<QGridLayout*>(getLayout(tn));
            if( grid ) {
                if(isNumberVar(arrs->get(0)) ) {
                    grid->setSpacing(toInteger(arrs->get(0)));
                } else {
                    LPCC ty=AS(0);
                    if(ccmp(ty,"vertical")) {
                        grid->setVerticalSpacing(toInteger(arrs->get(1)) );
                    } else if(ccmp(ty,"minHeight")) {
                        grid->setRowMinimumHeight(toInteger(arrs->get(1)), toInteger(arrs->get(2)));
                    } else if(ccmp(ty,"minWidth")) {
                        grid->setColumnMinimumWidth(toInteger(arrs->get(1)), toInteger(arrs->get(2)));
                    } else { // horizontal
                        grid->setHorizontalSpacing(toInteger(arrs->get(1)) );
                    }
                }
            }
        } break;
        case LTYPE_HBOX:
        case LTYPE_VBOX: {
            QBoxLayout* box = qobject_cast<QBoxLayout*>(getLayout(tn));
            if(box) {
                box->setSpacing(toInteger(arrs->get(0)) );
            }
        } break;
        default: break;
        }
    } else if( ccmp(fnm,"stretch") ) {
        switch(tn->xstat) {
        case LTYPE_GRID: {
            QGridLayout* grid = qobject_cast<QGridLayout*>(getLayout(tn));
            if( grid ) {
                LPCC ty=AS(0);
                if(ccmp(ty,"row")) {
                    grid->setRowStretch(toInteger(arrs->get(1)), toInteger(arrs->get(2)));
                } else {
                    grid->setColumnStretch(toInteger(arrs->get(1)), toInteger(arrs->get(2)));
                }
            }
        } break;
        case LTYPE_HBOX:
        case LTYPE_VBOX: {
            QBoxLayout* box = qobject_cast<QBoxLayout*>(getLayout(tn));
            box->setStretch(toInteger(arrs->get(0)), toInteger(arrs->get(1)) );
        } break;
        }
    } else if( ccmp(fnm,"removeChild") ) {
        if(cnt==0) return true;
        StrVar* sv=arrs->get(0);
        TreeNode* widgetNode=NULL;
        if(SVCHK('n',0)) {
            widgetNode=(TreeNode*)SVO;
        }
        bool check=false;
        QLayout* box = qobject_cast<QLayout*>(getLayout(tn));
        if( box && widgetNode ) {
            QWidget* qw=gwidget(widgetNode);
            if(qw) {
                box->removeWidget(qw);
            } else {
                qDebug("layout remove widget error !!!");
            }
            /*
            else {
                clearLayout(box, SVCHK('3',1));
            }
            */
        } else {
            qDebug("layout remove widget node error !!!");
        }
        rst->setVar('3', check? 1: 0);
    } else if( ccmp(fnm,"addChild") ) {
        if(cnt==0) return true;
        StrVar* sv=arrs->get(0);
        QScrollArea* scroll=NULL;
        QLayout* layout=NULL;
        QWidget* widget=NULL;
        QWidget* parent=NULL;
        int idx=1;
        TreeNode* cf=NULL;
        rst->reuse();
        if(SVCHK('w',20)) {
            scroll=(QScrollArea*)SVO;
            sv=arrs->get(idx++);
            if(SVCHK('n',0)) {
                cf=(TreeNode*)SVO;
            }
        } else if( SVCHK('n',0)) {
            cf=(TreeNode*)SVO;
        } else if(sv) {
            LPCC tag=toString(sv);
            cf=tn->addNode();
            cf->set("tag", tag);
        }
        if(cf==NULL ) {
            qDebug("#0#layout add node not defined");
            return true;
        }
        if( scroll==NULL ) {
            sv=arrs->get(idx);
            if( SVCHK('n',0)) {
                idx++;
                cf->xparent=(TreeNode*)SVO;
            }
            if( cf->xparent==NULL ) {
                cf->xparent=tn; //findPageNode(tn);
            }
            LPCC tag=cf->get("tag");
            parent=gwidget(cf->xparent);
            if( ccmp(tag,"hbox")|| ccmp(tag,"vbox") || ccmp(tag,"form") ) {
                layout=createLayout(cf, parent);
            } else {
                widget=createWidget(tag, cf, parent);
            }
        }

        if(layout || widget || scroll) {
            switch(tn->xstat) {
            case LTYPE_GRID: {
                QGridLayout* grid = qobject_cast<QGridLayout*>(getLayout(tn));
                if( grid ) {
                    int r=toInteger(arrs->get(idx++));
                    int c=toInteger(arrs->get(idx++));
                    int rs=toInteger(arrs->get(idx++));
                    int cs=toInteger(arrs->get(idx++));
                    if(rs==0) rs=1;
                    if(cs==0) cs=1;
                    if( layout ) {
                        grid->addLayout(layout, r, c, rs, cs );
                    } else {
                        grid->addWidget(widget?widget:scroll, r, c, rs, cs );
                    }
                }
            } break;
            case LTYPE_HBOX:
            case LTYPE_VBOX: {
                QBoxLayout* box = qobject_cast<QBoxLayout*>(getLayout(tn));
                if( box ) {
                    int pos=-1;
                    sv=arrs->get(idx++);
                    if( isNumberVar(sv)) {
                        pos=toInteger(sv);
                        sv=arrs->get(2);
                        if(SVCHK('n',0)) {
                            parent=gwidget((TreeNode*)SVO);
                        }
                    }
                    qDebug("xxxx layout child index: %d ", pos );
                    if(pos==-1) {
                        if( layout ) {
                            box->addLayout(layout, cf->getInt("stretch"));
                        } else {
                            box->addWidget(widget?widget:scroll, cf->getInt("stretch"));
                        }
                    } else {
                        if( layout ) {
                            box->insertLayout(pos, layout, cf->getInt("stretch"));
                        } else {
                            box->insertWidget(pos, widget?widget:scroll, cf->getInt("stretch"));
                        }
                    }
                }
            } break;
            default: break;
            }
        } else {
            qDebug("#0#layout add child not defind [node:%s]", toString(arrs->get(0)) );
        }
        if(layout || cf->cmp("tag","group")) {
            createWidgetChild(cf, parent, 1);
        }
        rst->setVar('n',0,(LPVOID)cf);
    } else if( ccmp(fnm,"widgets") ) {
        XListArr* a = getWidgetArr(tn, getListArrTemp());
        rst->setVar('a',0,(LPVOID)a);
    } else {
        call=false;
    }
    return call;
}

bool callWidgetAll(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    bool call=true;
    // qDebug("callWidgetAll %s (%d, %d)", fnm, tn->xstat, fc->xfuncRef);
    if( slen(fnm)==0 ) fnm=fc->getFuncName();
    int cnt=arrs? arrs->size() : 0;
    switch( tn->xstat ) {
    case WTYPE_DATEEDIT: {
        GDateEdit* w=qobject_cast<GDateEdit*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"date") ? 1 :
                ccmp(fnm,"value") ? 2 :
                ccmp(fnm,"select") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 || ref==2 ) {
            if( arrs ) {
                LPCC ds = AS(0);
                LPCC fmt = AS(1);
                if( slen(fmt)==0 ) fmt = "yyyy-MM-dd";
                if( slen(ds) ) {
                    w->setDate(QDate::fromString(KR(ds), KR(fmt)) );
                } else {
                    if( !w->isNullable() ) {
                        w->setNullable(true);
                    }
                    QDateTime dt;
                    w->setDateTime(dt);
                }
            } else {
                rst->set( Q2A(w->date().toString("yyyy-MM-dd")) );
            }
        } else if( ref==3 ) {
            w->selectAll();
        } else {
            call=false;
        }
    } break;
    case WTYPE_CALENDAR: {
        GCalendar* w=qobject_cast<GCalendar*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"date") ? 1 :
                ccmp(fnm,"value") ? 2 :
                ccmp(fnm,"select") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 || ref==2 ) {
        } else if( ref==3 ) {
        } else {
            call=false;
        }
    } break;
    case WTYPE_TIMEEDIT: {
        GTimeEdit* w=qobject_cast<GTimeEdit*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"time") ? 1 :
                ccmp(fnm,"value") ? 2 :
                ccmp(fnm,"select") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 || ref==2 ) {
            if( arrs ) {
                LPCC ds = AS(0);
                LPCC fmt = AS(1);
                if( slen(fmt)==0 ) fmt = "HH:mm";
                w->setTime(QTime::fromString(KR(ds), KR(fmt)) );
            } else {
                rst->set( Q2A(w->time().toString("HH:mm")) );
            }
        } else if( ref==3 ) {
            w->selectAll();
        } else {
            call=false;
        }
    } break;
    case WTYPE_LABEL: {
        GLabel* w = qobject_cast<GLabel*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"text") ? 1 :
                ccmp(fnm,"value") ? 2 :
                ccmp(fnm,"playGif") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 || ref==2 ) {
            if( arrs ) {
                LPCC txt=AS(0); if( txt==NULL ) txt="";
                w->setText(KR(txt));
            } else {
                rst->set(Q2A(w->text()));
            }
        } else if( ref==3 ) {
            if( cnt==0 ) {

            } else {
                LPCC fileName=AS(0);
                QMovie *movie = new QMovie(KR(fileName));
                if( movie->isValid()) {
                    w->setMovie(movie);
                    movie->start();
                } else {
                    qDebug("#9#play GIF file not valid fileName %s", fileName);
                }
            }
        } else {
            call=false;
        }
    } break;
    case WTYPE_SPIN: {
        GSpin* w=qobject_cast<GSpin*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"value") ? 1 :
                ccmp(fnm,"select") ? 2 :
                ccmp(fnm,"range") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 ) {
            if( arrs ) {
                w->setValue(toInteger(arrs->get(0)));
            } else {
                rst->setVar('0',0).addInt(w->value());
            }
        } else if(ref==2 ) {
            w->selectAll();
        } else if( ref==3 ) {
            StrVar* sv=NULL;
            if( cnt==0 ) {

            } else {
                int cnt=arrs->size();
                if( cnt==1 ) {
                    sv=arrs->get(0);
                    if( isNumberVar(sv) ) w->setRange(0, toInteger(sv) );
                } else if( cnt==2 ) {
                    int sp=0, ep=0;
                    sv=arrs->get(0);
                    if( isNumberVar(sv) ) {
                        sp=toInteger(sv); sv=arrs->get(1);
                        ep=toInteger(sv);
                        if( sp<ep ) w->setRange(sp,ep);
                    }
                }
            }
        } else {
            call=false;
        }
    } break;
    case WTYPE_CONTEXT: {
        GContext* w=qobject_cast<GContext*>(widget);
        if( w==NULL ) {
            qDebug("#0#context canvas convert error name:%s", fnm);
            return false;
        }
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"interval") ? 1 :
                ccmp(fnm,"autoFill") ? 2 :
                ccmp(fnm,"overlay") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 ) {
            if( arrs ) {
                StrVar* sv=arrs->get(0);
                if(isNumberVar(sv)) {
                    w->startTimer(toInteger(sv));
                }
            } else {
                rst->setVar('0',0).addInt(w->_interval);
            }
        } else if( ref==2 ) {
            if( arrs ) {
                StrVar* sv=arrs->get(0);
                w->setAutoFillBackground(isVarTrue(sv));
            }
        } else if( ref==3 ) {
            if( arrs ) {
                int interval=0;
                StrVar* sv=arrs->get(0);
                bool chk=isVarTrue(sv);
                sv=arrs->get(1);
                if(SVCHK('n',0)) {
                    TreeNode* p=(TreeNode*)SVO;
                    QWidget* widget=gwidget(p);
                    if(widget ) {
                        w->setParent(widget);
                    }
                    sv=arrs->get(2);
                    if(isNumberVar(sv) ) {
                        interval=toInteger(sv);
                    } else {
                        interval=500;
                    }
                } else if(isNumberVar(sv) ) {
                    interval=toInteger(sv);
                }
                if( !tn->cmp("type","overlay") ) {
                    w->setAttribute(Qt::WA_TranslucentBackground, true);
                    w->setAttribute(Qt::WA_NoSystemBackground);
                    w->setAttribute(Qt::WA_OpaquePaintEvent);
                    w->setAttribute(Qt::WA_NoBackground);
                    w->setStyleSheet("background:transparent;");
                    if (w->parent()) {
                        w->parent()->installEventFilter(w);
                        w->raise();
                    }
                    tn->set("type","overlay");
                }
                if(chk) {
                   if(interval>0 ) w->openOverlay(500);
                }
                w->showOverlay(chk);
            } else {
                rst->add(tn->gv("type"));
            }
        } else {
            call=false;
        }

    } break;
    case WTYPE_BUTTON: {
        // LPCC type=tn->get("type");
        GButton* w=qobject_cast<GButton*>(widget);
        if( w==NULL ) {
            qDebug("#0#button convert error name:%s", fnm);
            return false;
        }
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"text") ? 1 :
                ccmp(fnm,"value") ? 2 :
                ccmp(fnm,"icon") ? 3 :
                ccmp(fnm,"down") ? 4 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 || ref==2 ) {
            if( arrs ) {
                LPCC txt=AS(0);
                w->setText(KR(txt));
            } else {
                rst->set( Q2A(w->text()) );
            }
        } else if( ref==3  ) {
            // version 1.0.4
            if( arrs ) {
                QPixmap* pm=getStrVarPixmap(arrs->get(0));
                QRect rc;
                if( pm && !pm->isNull() ) {
                    LPCC icon=AS(0);
                    rc= pm->rect();
                    w->setIcon(QIcon(*pm));
                    tn->set("icon", icon );
                }
                if( rc.isValid() ) {
                    w->setIconSize(rc.size());
                    LPCC ty=AS(1);
                    if( slen(ty) ) {
                        w->setFixedSize(rc.size());
                    }
                }
            } else {
                LPCC icon=tn->get("icon");
                rst->set(icon);
            }
        } if( ref==4 ) {
            if( cnt==0 || isVarTrue(arrs->get(0)) ) {
                w->setDown(true);
            } else {
                w->setDown(false);
            }
        } else {
            call=false;
        }
    } break;
    case WTYPE_TOOLBTN: {
        GToolButton* w=qobject_cast<GToolButton*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"text") ? 1 :
                ccmp(fnm,"icon") ? 2 :
                ccmp(fnm,"down") ? 3 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 ) {
            if( cnt==0 ) return false;
            LPCC txt = AS(0);
            QPixmap* pm=getStrVarPixmap(arrs->get(1));
            QRect rc;
            if( pm && !pm->isNull() ) {
                rc= pm->rect();
                w->setIcon(QIcon(*pm));
            }
            LPCC align = AS(2);
            if( ccmp(align,"under") ) w->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
            else if( ccmp(align,"right") ) w->setToolButtonStyle(Qt::ToolButtonFollowStyle);
            else w->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
            if( rc.isValid() && ccmp(align,"iconOnly") ) {
                w->setIconSize(rc.size());
                w->setFixedSize(rc.size());
            }
            w->setText(KR(txt));
        } else if( ref==2 ) {
            StrVar* sv=NULL;
            if( cnt==0 ) {
                // version 1.0.3
                sv=tn->gv("@icon");
                if( sv==NULL ) sv=tn->gv("icon");
                rst->set( toString(sv) );
            } else {
                tn->GetVar("@icon")->set(toString(arrs->get(0)));
                QPixmap* pm=getStrVarPixmap(arrs->get(0));
                QRect rc;
                if( pm && !pm->isNull() ) {
                    rc= pm->rect();
                    w->setIcon(QIcon(*pm));
                }
            }
        } else if( ref==3 ) {
            if( cnt==0 ) {
                rst->setVar('3', w->isDown()? 1: 0);
            }
            if( isVarTrue(arrs->get(0)) ) {
                w->setDown(true);
            } else {
                w->setDown(false);
            }
        } else {
            call=false;
        }
    } break;
    case WTYPE_GROUP: {
        GGroup* w=qobject_cast<GGroup*>(widget);
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"title") ? 1 :
                ccmp(fnm,"prop") ? 2 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 ) {
            if( arrs ) {
                LPCC title = AS(0);
                w->setTitle(KR(title));
            }
        } else if( ref==2 ) {
            if( arrs ) {
                LPCC key=AS(0), value=AS(1);
                tn->set(key, value);
            }
        }  else {
            call=false;
        }
    } break;
    case WTYPE_PROGRESS: {
        GProgress* w=qobject_cast<GProgress*>(widget);
        StrVar* sv=NULL;
        if( w==NULL ) return false;
        U32 ref=fc->xfuncRef;
        if( ref==0 ) {
            ref=
                ccmp(fnm,"range") ? 1 :
                ccmp(fnm,"value") ? 2 :
                0;
            fc->xfuncRef=ref;
        }
        if( ref==1 ) {
            int a=0, b=100;
            if( arrs ) {
                if( arrs->size()==1 ) {
                    sv = arrs->get(0);
                    if( isNumberVar(sv) ) b = toInteger(sv);
                } else if( arrs->size()==2 ) {
                }
            }
            if( a<b ) w->setRange(a,b);
        } else if( ref==2 ) {
            if( cnt==0 ) {
                rst->setVar('0',0).addInt(w->value());
            } else {
                int a = 0;
                sv = arrs->get(0);
                if( isNumberVar(sv) ) a = toInteger(sv);
                w->setValue(a);
            }
        } else {
            call=false;
        }
    } break;
    case WTYPE_SPLITTER: {
        call=callSplitterFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_TREE: {
        call=callTreeFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_MAIN: {
        call=callMainFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_COMBO: {
        call=callComboFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_DIV: {
        call=callDivFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_TAB: {
        call=callTabFunc(fnm, arrs, tn, fn, rst,widget, fc);
    } break;
    case WTYPE_LINEINPUT: {
        call=callInputFunc(fnm, arrs, tn, fn, rst, qobject_cast<GInput*>(widget), fc);
    } break;
    case WTYPE_CHECK: {
        call=callCheckFunc(fnm, arrs, tn, fn, rst,widget);
    } break;
    case WTYPE_RADIO: {
        call=callRadioFunc(fnm, arrs, tn, fn, rst,widget);
    } break;
    case WTYPE_PAGE: {
        call=callPageFunc(fnm, arrs, tn, fn, rst,qobject_cast<GPage*>(widget), fc);
    } break;
    case WTYPE_CANVAS: {
        call=callCanvasFunc(fnm, arrs, tn, fn, rst,qobject_cast<GCanvas*>(widget), fc);
    } break;
    case WTYPE_TEXTEDIT: {
        call=callTextEditFunc(fnm, arrs, tn, fn, rst,qobject_cast<GTextEdit*>(widget), fc);
    } break;
    case WTYPE_WEBVIEW: {
#ifdef WEBVIEW_USE
        call=callWebViewFunc(fnm, arrs, tn, fn, rst,qobject_cast<WebView*>(widget), fc);
#endif
    } break;
    case FNSTATE_MPPLAYER: {
        call=callMPlayerFunc(fnm, arrs, tn, fn, rst,qobject_cast<GMpWidget*>(widget), fc);
    } break;
    case WTYPE_VIDEO: {
        call=callVideoFunc(fnm, arrs, tn, fn, rst,qobject_cast<GVideo*>(widget), fc);
    } break;
    default: {
        call=false;
    } break;
    }
    return call;
}


TreeNode* getWidgetConfig(QWidget* w, U16 stat) {
    {
        GCanvas* widget=qobject_cast<GCanvas*>(w);
        if( widget )
            return widget->config();
    }
    {
        GPage* widget=qobject_cast<GPage*>(w);
        if( widget )
            return widget->config();
    }
    {
        GWidget* widget=qobject_cast<GWidget*>(w);
        if( widget )
            return widget->config();
    }
    {
        GSplitter* widget=qobject_cast<GSplitter*>(w);
        if( widget )
            return widget->config();
    }
    {
        GTree* widget=qobject_cast<GTree*>(w);
        if( widget )
            return widget->config();
    }
    {
        GTextEdit* widget=qobject_cast<GTextEdit*>(w);
        if( widget )
            return widget->config();
    }
    {
        GLabel* widget=qobject_cast<GLabel*>(w);
        if( widget )
            return widget->config();
    }
    {
        GCombo* widget=qobject_cast<GCombo*>(w);
        if( widget )
            return widget->config();
    }
    {
        GInput* widget=qobject_cast<GInput*>(w);
        if( widget )
            return widget->config();
    }
    return NULL;
}

void showHideWidgets(TreeNode* tn, bool show, XListArr* ids) {
    for( int n=0,sz=tn->childCount();n<sz;n++) {
        TreeNode* cur = tn->child(n);
        if( cur->xstat==LTYPE_GRID || cur->xstat==LTYPE_HBOX || cur->xstat==LTYPE_VBOX || cur->xstat==LTYPE_ROW ) {
            showHideWidgets(cur, show, ids);
        } else {
            QWidget* wd = gwidget(cur);
            if( wd ) {
                bool ok=true;
                if( ids ) {
                    LPCC cid=cur->get("id");
                    ok=false;
                    if( slen(cid) ) {
                        for( int x=0,cnt=ids->size();x<cnt;x++) {
                            LPCC id=toString(ids->get(x));
                            if( ccmp(cid, id) ) {
                                ok=true;
                                break;
                            }
                        }
                    }
                }
                if( ok ) {
                    if( show )
                        wd->show();
                    else
                        wd->hide();
                }
            }
        }
    }
}


bool callSplitterFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    if( widget==NULL )
        widget=gwidget(tn);
    GSplitter* w=qobject_cast<GSplitter*>(widget);
    if( w==NULL )
        return false;
    // ver 1.0.2
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"stretchFactor") ? 1 :
            ccmp(fnm,"addPage") ? 2 :
            ccmp(fnm,"handleWidth") ? 3 :
            ccmp(fnm,"sizes") ? 4 :
            ccmp(fnm,"widgetAt") ? 5 :
            ccmp(fnm,"orientation") ? 6 :
            ccmp(fnm,"maxWidth") ? 7 :
            ccmp(fnm,"maxHeight") ? 8 :
            ccmp(fnm,"minWidth") ? 9 :
            ccmp(fnm,"minHeight") ? 10 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {   // stretchFactor
        if( cnt==0 ) return false;
        if( isNumberVar(arrs->get(0)) ) {
            w->setStretchFactor(toInteger(arrs->get(0)),1);
        } else {
            LPCC id=AS(0);
            for(int n=0,sz=tn->childCount();n<sz;n++) {
                TreeNode* cur=tn->child(n);
                if( cur->cmp("id",id) ) {
                    w->setStretchFactor(n,1);
                }
            }
        }
    } break;
    case 2: {   // addPage
        if( cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if(SVCHK('n',0)) {
            TreeNode* node=(TreeNode*)SVO;
            QWidget* widget=gwidget(node);
            qDebug("splitter add widget :%s", widget?"ok":"no widget");
            if( widget ) {
                for( int n=0,sz=w->count();n<sz;n++ ) {
                    if( w->widget(n)==widget ) {
                        return true;
                    }
                }
                w->addWidget(widget);
                widget->show();
                tn->addChild(node);
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 3: {   // handleWidth
        if( cnt==0 ) {
            rst->setVar('0',0).addInt( w->handleWidth() );
        } else {
            StrVar* sv = arrs->get(0);
            int wid = isNumberVar(sv) ? toInteger(sv): 4;
            w->setHandleWidth(wid);
        }
    } break;
    case 4: {   // sizes
        QList<int> list=w->sizes();
        if( cnt==0 ) {
            XListArr* a=tn->addArray("@sizes");
            a->reuse();
            for(int n=0,sz=list.size();n<sz;n++) {
                a->add()->setVar('0',0).addInt(list.at(n));
            }
            rst->setVar('a',0,(LPVOID)a);
            return true;
        }
        // version 1.0.3
        StrVar* sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            for( int n=0;n<a->size() && n<list.size(); n++ ) {
                int wid=toInteger(a->get(n));
                list[n]=wid;
            }
            w->setSizes(list);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 5: {   // widgetAt
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            TreeNode* cur=tn->child(toInteger(sv));
            if( cur) rst->setVar('n',0,(LPVOID)cur);
        }
    } break;
    case 6: {   // orientation
        if(cnt==0) {
            if(w->orientation()==Qt::Vertical) {
                rst->set("vbox");
            } else {
                rst->set("hbox");
            }
            return true;
        }
        StrVar* sv=arrs->get(0);
        LPCC ty=toString(sv);
        if( ccmp(ty,"vbox") ) {
            w->setOrientation(Qt::Vertical);
        } else if(ccmp(ty,"hbox")) {
            w->setOrientation(Qt::Horizontal);
        }
    } break;
    case 7: {   // maxWidth
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            w->setMaximumWidth(toInteger(sv));
        }
    } break;
    case 8: {   // maxHeight
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            w->setMaximumHeight(toInteger(sv));
        }
    } break;
    case 9: {   // minWidth
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            w->setMinimumWidth(toInteger(sv));
        }
    } break;
    case 10: {   // minHeight
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            w->setMinimumHeight(toInteger(sv));
        }
    } break;
    default: { return false; }
    }
    return true;
}

bool callMPlayerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GMpWidget* w, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    StrVar* sv=NULL;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"start") ? 1 :
            ccmp(fnm,"openUrl") ? 2 :
            ccmp(fnm,"playState") ? 3 :
            ccmp(fnm,"play") ? 4 :
            ccmp(fnm,"stop") ? 5 :
            ccmp(fnm,"quit") ? 6 :
            ccmp(fnm,"pause") ? 7 :
            ccmp(fnm,"seek") ? 8 :
            ccmp(fnm,"command") ? 9 :
            ccmp(fnm,"isRun") ? 9 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {	// start
        StrVar* sv=NULL;
        LPCC path=toString(confVar("path.mplayer"));
#ifdef WINDOW_USE
        if( slen(path) ) {
            w->setMPlayerPath(KR(path));
        } else {
            rst->set( Q2A(QApplication::applicationDirPath()) );
            rst->add("/app/mplayer/mplayer.exe");
            rst->replace('/','\\');
            path=rst->get();
            w->setMPlayerPath(KR(path));
        }
        int sp=0, ep=0;
        StrVar* var=NULL;
        QStringList list;
        if( cnt==0 ) {
        } else {
            sv=arrs->get(0);
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                for( int n=0,sz=a->size();n<sz; n++ ) {
                    LPCC val=toString(a->get(n));
                    if( slen(val) ) list.append(KR(val));
                }
            } else {
                rst->set( toString(sv) );
                var=rst;
            }
        }
        if( var ) {
            XParseVar pv(var,sp,ep);
            for(int n=0; n<64 && pv.valid(); ) {
                char ch=pv.ch();
                if( ch==0 )
                    break;
                LPCC val=NULL;
                if( ch=='\'' || ch=='"' ) {
                    val=pv.v();
                } else {
                    pv.findEnd(" \t",FIND_CHARS);
                    val=pv.v();
                }
                if( slen(val) ) {
                    list.append(KR(val));
                }
            }
        }
        w->start(list);
#else
        if( slen(path) ) {
            w->setMPlayerPath(KR(path));
        }
        int sp=0, ep=0;
        StrVar* var=NULL;

        QStringList list;
        if( cnt==0 ) {
        } else {
            sv=arrs->get(0);
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                for( int n=0,sz=a->size();n<sz; n++ ) {
                    LPCC val=toString(a->get(n));
                    if( slen(val) ) list.append(KR(val));
                }
            } else {
                LPCC cmd=AS(1);
                if( slen(cmd) ) {
                    var=NULL;
                    list.append("bash");
                    list.append(cmd);
                } else {
                    rst->set( toString(sv) );
                    var=rst;
                    // var->set("-slave -idle -noquiet -identify -nodouble -nomouseinput -nokeepaspect -monitorpxelaspect 1 -input nodefault-bindings:conf=/dev/null -vo direct3d ");
                }
            }
        }
        if( var ) {
            XParseVar pv(var,sp,ep);
            for(int n=0; n<64 && pv.valid(); ) {
                char ch=pv.ch();
                if( ch==0 )
                    break;
                LPCC val=NULL;
                if( ch=='\'' || ch=='"' ) {
                    val=pv.v();
                } else {
                    pv.findEnd(" \t",FIND_CHARS);
                    val=pv.v();
                }
                if( slen(val) ) {
                    list.append(KR(val));
                }
            }
        }
        w->start(list);
#endif
    } break;
    case 2: {	// openUrl
        if( cnt==0 ) return false;
        LPCC url=NULL;
        sv=arrs->get(0);
        sv->replace('\\','/');
        url=toString(sv);
        if( slen(url) ) w->load(KR(url));
    } break;
    case 3: {	// playState
        w->writeCommand(QString("state"));
        rst->reuse()->add( tn->gv("@state") );
    } break;
    case 4: {	// play
        w->play();
    } break;
    case 5: {	// stop
        w->stop();
    } break;
    case 6: {	// quit
        w->writeCommand(QString("quit"));
    } break;
    case 7: {	// pause
        w->pause();
    } break;
    case 8: {	// seek

    } break;
    case 9: {	// command
        LPCC cmd=arrs ? AS(0): NULL;
        if( slen(cmd) ) w->writeCommand(KR(cmd));
    } break;
    case 10: {	// isRun
        w->writeCommand(QString("isRunning"));
        LPCC state=toString(tn->gv("@state"));
        rst->setVar('3', ccmp(state,"running") ? 1 :0 );
    } break;
    default: return false;
    }
    return true;
}

bool callVideoFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GVideo* w, XFunc* fc) {
    StrVar* sv=NULL;
    if( w==NULL ) {
        QWidget* widget=gwidget(tn);
        if( widget ) {
            w=qobject_cast<GVideo*>(widget);
            if( w==NULL ) return true;
        }
    }
    int cnt=arrs? arrs->size() : 0;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"start") ? 1 :
            ccmp(fnm,"openUrl") ? 2 :
            ccmp(fnm,"playState") ? 3 :
            ccmp(fnm,"play") ? 4 :
            ccmp(fnm,"stop") ? 5 :
            ccmp(fnm,"quit") ? 6 :
            ccmp(fnm,"pause") ? 7 :
            ccmp(fnm,"seek") ? 8 :
            ccmp(fnm,"command") ? 9 :
            ccmp(fnm,"isRun") ? 10 :
            ccmp(fnm,"canvas") ? 11 :
            ccmp(fnm,"fullscreen") ? 12 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    qDebug("#### callVideoFunc %s (%d)", fnm, ref);
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {	// start
        StrVar* sv=NULL;
        LPCC path=toString(confVar("path.mplayer"));
#ifdef WINDOW_USE
        if( slen(path) ) {
            w->setMPlayerPath(KR(path));
        } else {
            rst->set( Q2A(QApplication::applicationDirPath()) );
            rst->add("/app/mplayer/mplayer.exe");
            rst->replace('/','\\');
            path=rst->get();
            w->setMPlayerPath(KR(path));
        }
        int sp=0, ep=0;
        StrVar* var=NULL;
        QStringList list;
        if( cnt==0 ) {

        } else {
            sv=arrs->get(0);
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                for( int n=0,sz=a->size();n<sz; n++ ) {
                    LPCC val=toString(a->get(n));
                    if( slen(val) ) list.append(KR(val));
                }
            } else {
                rst->set( toString(sv) );
                var=rst;
            }
        }
        if( var ) {
            XParseVar pv(var,sp,ep);
            for(int n=0; n<64 && pv.valid(); ) {
                char ch=pv.ch();
                if( ch==0 )
                    break;
                LPCC val=NULL;
                if( ch=='\'' || ch=='"' ) {
                    val=pv.v();
                } else {
                    pv.findEnd(" \t",FIND_CHARS);
                    val=pv.v();
                }
                if( slen(val) ) {
                    list.append(KR(val));
                }
            }
        }
        w->start(list);
#else
        if( slen(path) ) {
            w->setMPlayerPath(KR(path));
        }
        int sp=0, ep=0;
        StrVar* var=NULL;

        QStringList list;
        if( cnt==0 ) {

        } else {
            sv=arrs->get(0);
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                for( int n=0,sz=a->size();n<sz; n++ ) {
                    LPCC val=toString(a->get(n));
                    if( slen(val) ) list.append(KR(val));
                }
            } else {
                LPCC cmd=AS(1);
                if( slen(cmd) ) {
                    var=NULL;
                    list.append("bash");
                    list.append(cmd);
                } else {
                    rst->set( toString(sv) );
                    var=rst;
                    // var->set("-slave -idle -noquiet -identify -nodouble -nomouseinput -nokeepaspect -monitorpxelaspect 1 -input nodefault-bindings:conf=/dev/null -vo direct3d ");
                }
            }
        }
        if( var ) {
            XParseVar pv(var,sp,ep);
            for(int n=0; n<64 && pv.valid(); ) {
                char ch=pv.ch();
                if( ch==0 )
                    break;
                LPCC val=NULL;
                if( ch=='\'' || ch=='"' ) {
                    val=pv.v();
                } else {
                    pv.findEnd(" \t",FIND_CHARS);
                    val=pv.v();
                }
                if( slen(val) ) {
                    list.append(KR(val));
                }
            }
        }
        w->start(list);
#endif
    } break;
    case 2: {	// openUrl
        if( cnt==0 ) return false;
        LPCC url=NULL;
        sv=arrs->get(0);
        sv->replace('\\','/');
        url=toString(sv);
        qDebug("video open url: %s", url);
        if( slen(url) ) w->openViedo(KR(url));
    } break;
    case 3: {	// playState
        w->writeCommand(QString("state"));
        rst->reuse()->add( tn->gv("@state") );
    } break;
    case 4: {	// play
        w->play();
    } break;
    case 5: {	// stop
        w->stop();
    } break;
    case 6: {	// quit
        w->writeCommand(QString("quit"));
    } break;
    case 7: {	// pause
        w->pause();
    } break;
    case 8: {	// seek

    } break;
    case 9: {	// command
        LPCC cmd=arrs ? AS(0): NULL;
        if( slen(cmd) ) w->writeCommand(KR(cmd));
    } break;
    case 10: {	// isRun
        w->writeCommand(QString("isRunning"));
        LPCC state=toString(tn->gv("@state"));
        rst->setVar('3', ccmp(state,"running") ? 1 :0 );
    } break;
    case 11: {	// canvas
        GCanvas* canvas=w->canvas();
        if( canvas && canvas->config() ) {
            rst->setVar('n',0,(LPVOID)canvas->config() );
        }
    } break;
    case 12: {	// fullscreen
        if( cnt==0 || isVarTrue(arrs->get(0)) ) {
            if( !w->isFullScreen() ) w->toggleFullScreen();
        } else {
            if( w->isFullScreen() ) w->toggleFullScreen();
        }
    } break;
    default: return false;
    }
    return true;
}
#ifdef WEBVIEW_USE
bool callWebViewFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, WebView* wv, XFunc* fc) {
    if( wv==NULL ) return false;
    StrVar* sv=NULL;
    int cnt = arrs ? arrs->size(): 0;
    U32 fref=fc? fc->xfuncRef: 0;
    if( fref==0 ) {
        fref=
            ccmp(fnm, "url") ? 1 :
            ccmp(fnm, "html") ? 2 :
            ccmp(fnm, "scroll") ? 3 :
            ccmp(fnm, "fetchTagInfo") ? 4 :
            ccmp(fnm, "findAll") ? 5 :
            ccmp(fnm, "findParent") ? 6 :
            ccmp(fnm, "infoAt") ? 7 :
            ccmp(fnm, "clickPosition") ? 8 :
            ccmp(fnm, "hitTest") ? 9 :
            ccmp(fnm, "valueAt") ? 10 :
            ccmp(fnm, "attrAt") ? 11 :
            ccmp(fnm, "search") ? 12 :
            ccmp(fnm, "zoom") ? 13 :
            ccmp(fnm, "viewSize") ? 14 :
            ccmp(fnm, "capture") ? 15 :
            ccmp(fnm, "is") ? 16 :
            ccmp(fnm, "scollbarWidth") ? 17 :
            ccmp(fnm, "showKeyMap") ? 18 :
            ccmp(fnm, "callJs") ? 19 :
            0;
        if( fc ) fc->xfuncRef=fref;
    }
    switch(fref) {
    case 1: { // url
        if( cnt==0 ) {
            rst->set( Q2A(wv->url().toString()) );
        } else {
            QNetworkDiskCache* diskCache = NULL;
            sv = tn->GetVar("@diskcache");
            if( SVCHK('c',11) ) {
                diskCache = (QNetworkDiskCache*)SVO;
            } else {
                QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
                diskCache = new QNetworkDiskCache();
                diskCache->setCacheDirectory(location);
                sv->setVar('c',11,(LPVOID)diskCache);
            }
            LPCC url=AS(0);
            QUrl qurl(KR(url));
            wv->loadPage( qurl, diskCache );
            if( cnt==2 ) {
                sv=arrs->get(1);
                if( SVCHK('f',1) ) {
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
                }
            }
        }
    } break;
    case 2: { // html
        if( cnt==0 ) {
            wv->viewSource();
            sv=tn->GetVar("@refSource");
            if( SVCHK('s',0) ) {
                rst->reuse()->add(sv);
            }
            return true;
        }
        int sp=0,ep=0;
        sv = getStrVarPointer(arrs->get(0), &sp, &ep);
        LPCC html = sv->get(sp,ep);
        LPCC base= AS(1);
        if( slen(base) ) {
            wv->setHtml(KR(html), QUrl(KR(base)) );
        } else {
            wv->setHtml(KR(html));
        }
    } break;
    case 3: { // scroll
        QWebFrame* frame=wv->page()->mainFrame();
        if( cnt==0 ) {
            frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
            frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
            return true;
        }
        LPCC ty=AS(0);
        if( ccmp(ty,"hide") ) {
            frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
            frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        } else if( ccmp(ty,"show") ) {
            frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAsNeeded);
            frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAsNeeded);
        } else if( ccmp(ty,"hhide") ) {
            frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        } else if( ccmp(ty,"vhide") ) {
            frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        } else if( ccmp(ty,"bottom") ) {
            frame->setScrollBarValue(Qt::Vertical, frame->scrollBarMaximum(Qt::Vertical));
        } else if( ccmp(ty,"top") ) {
            frame->setScrollBarValue(Qt::Vertical, 0);
        } else if( ccmp(ty,"position") ) {
            StrVar* sv=arrs->get(1);
            if( SVCHK('i',2) ) {
                frame->setScrollBarValue(Qt::Horizontal, sv->getInt(4) );
                frame->setScrollBarValue(Qt::Vertical, sv->getInt(8) );
            } else if( isNumberVar(sv) ) {
                frame->setScrollBarValue(Qt::Vertical, toInteger(sv));
            }
        }
    } break;
    case 4: { // fetchTagInfo
        if( cnt==0 ) {
            return true;
        }
        LPCC tag=AS(0);
        TreeNode* node=getTreeNodeTemp();
        wv->tagInfo(tag, AS(1), node, isVarTrue(arrs->get(2)) );
        rst->setVar('n',0,(LPVOID)node);
    }  break;
    case 5: { // findAll
        if( cnt==0 ) {
            return true;
        }
        XListArr* a=getListArrTemp();
        int idx=-1;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
            wv->findAll( idx, AS(1), AS(2), a);
        } else {
            wv->findAll( idx, AS(0), AS(1), a);
        }
        rst->setVar('a',0,(LPVOID)a);
    }  break;
    case 6: { // findParent
        if( cnt==0 ) {
            return true;
        }
        int idx=-1;
        rst->reuse();
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
            wv->findParentTag(idx, AS(0), AS(1), rst);
        } else {
            wv->findParentTag(idx, AS(1), AS(2), rst);
        }
    }  break;
    case 7: { // infoAt
        if( cnt==0 ) {
            return true;
        }
        TreeNode* node=getTreeNodeTemp();
        int idx=-1;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
        }
        wv->getElementInfo( idx, node);
        rst->setVar('n',0,(LPVOID)node);

    }  break;
    case 8: // clickPosition
    case 9: { // hitTest
        if( cnt==0 ) {
            return true;
        }
        int x=-1, y=-1;
        sv=arrs->get(0);
        if( SVCHK('i',2) || SVCHK('i',4) ) {
            x=sv->getInt(4), y=sv->getInt(8);
        } else if( isNumberVar(sv) ) {
            if( isNumberVar(arrs->get(1)) ) {
                x=toInteger(sv), y=toInteger(arrs->get(1));
            }
        }
        if( x!=-1 && y!=-1 ) {
            if( fref==9 ) {
                wv->hitTest(QPoint(x,y), rst->reuse() );
            } else {
                wv->sendMouseDownEvent(QPoint(x,y), rst->reuse() );
            }
        }
    }  break;
    case 10: { // valueAt
        if( cnt==0 ) return true;
        int idx=-1;
        StrVar* data=NULL;
        rst->reuse();
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
            data=arrs->get(1);
        } else {
            data=arrs->get(0);
        }
        if( data) {
            int sp=0,ep=0;
            sv=getStrVarPointer(data, &sp, &ep);
            rst->set("this.value='");
            jsValueString(sv, sp, ep, rst );
            rst->add("'");
        } else {
            rst->set("this.value");
        }
        wv->setElementValue(idx, rst->get(), rst );
    }  break;
    case 11: { // attrAt
        if( cnt==0 ) return true;
        int idx=-1;
        LPCC name=NULL;
        StrVar* data=NULL;
        rst->reuse();
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
            name=AS(1);
            data=arrs->get(2);
        } else {
            name=AS(0);
            data=arrs->get(1);
        }
        wv->setElementAttribute(idx, name, data);
    }  break;
    case 12: { // search
        if( cnt==0 ) return true;
        LPCC text=AS(0);
        if( !isVarTrue(arrs->get(1)) ) {
            wv->page()->findText(QString(),QWebPage::HighlightAllOccurrences );
        }
        if( slen(text) ) {
            wv->page()->findText(KR(text),QWebPage::HighlightAllOccurrences );
        }
    }  break;
    case 13: { // zoom
        if( cnt==0 ) return true;
        int zoom=100;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            zoom=toInteger(arrs->get(0));
        }
        wv->setZoom( zoom );
    }  break;
    case 14: { // viewSize
        QWebFrame* frame=wv->page()->mainFrame();
        if( cnt==0 ) {
            wv->page()->setViewportSize(wv->geometry().size() );
            frame->setScrollBarValue(Qt::Vertical, 0);
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('i',4) ) {
            int w=sv->getInt(12), h=sv->getInt(16);
            wv->page()->setViewportSize( QSize(w,h) );
        } else {
            LPCC ty=toString(sv);
            if( ccmp(ty,"full") ) {
                frame->setScrollBarValue(Qt::Vertical, 0);
                wv->page()->setViewportSize( frame->contentsSize() );
            }
        }
    }  break;
    case 15: { // capture
        QWebFrame* frame=wv->page()->mainFrame();
        int idx=-1;
        LPCC type=NULL, output=NULL;
        if( cnt==0 ) {
            type="copy";
        }
        // capture( type, output);
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
            type=AS(1), output=AS(2);
            sv=arrs->get(3);
        } else {
            type=AS(0), output=AS(1);
            sv=arrs->get(2);
        }
        if( idx==-1 ) {
            frame->setScrollBarValue(Qt::Vertical, 0);
            wv->page()->setViewportSize( frame->contentsSize() );
        }
        if( ccmp(type,"copy") ) {
            output=NULL;
        } else if( slen(output)==0 ) {
            if( ccmp(type,"pdf") ) {
                output="data/webcapture.pdf";
            } else {
                output="data/webcapture.jpg";
            }
        }
        if( sv ) rst->reuse()->add(sv);
        wv->renderElement(idx, type, output, rst );
        if( ccmp(type,"copy") ) {
            QClipboard *c = QApplication::clipboard();
            QPixmap* img=getStrVarPixmap(rst);
            if( img ) {
                c->setPixmap(*img);
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        }
        if( idx==-1 ) {
            wv->page()->setViewportSize(wv->geometry().size() );
        }
    }  break;
    case 16: { // is
        if( cnt==0 ) {
            rst->set("linkClickUse, touchScroll, validIndex");
            return true;
        }
        LPCC ty=AS(0);
        rst->reuse();
        if( isWidgetCheck(ty, wv, rst) ) return true;
        if( ccmp(ty,"linkClickUse") ) {
            sv=arrs->get(1);
            if( sv ) {
                if( SVCHK('3',1) ) {
                    tn->GetVar("@linkClickUse")->setVar('3',1);
                } else if( SVCHK('3',0) ) {
                    tn->GetVar("@linkClickUse")->setVar('3',0);
                } else {
                    rst->add(tn->gv("@linkClickUse"));
                }
            } else {
                rst->add(tn->gv("@linkClickUse"));
            }
        } else if( ccmp(ty,"validIndex") ) {
            rst->reuse();
            if( isNumberVar(arrs->get(1)) ) {
                int idx=toInteger(arrs->get(1));
                int sz=wv->xlistFind.size();
                if( idx>=0 && idx<sz ) {
                    rst->setVar('3',1);
                }
            }
        } else if( ccmp(ty,"touchScroll") ) {
            rst->add(tn->gv("@touchScrollUse"));
        }
    }  break;
    case 17: { // scollbarWidth
        if( cnt==0 ) {

        } else {
            int width=toInteger(arrs->get(0));
            if( width==0 ) width=16;
            qApp->setStyle(new WebViewStyle(width));
        }
    }  break;
    case 18: { // showKeyMap
        sv=arrs->get(1);
        if( sv==NULL || isVarTrue(sv) ) {
            wv->showAccessKeys();
        } else{
            wv->hideAccessKeys();
        }
        wv->setFocus();
    }  break;
    case 19: { // callJs
        if( cnt==0 ) {
             return true;
        } else {
            wv->call(AS(0),rst);
        }
    } break;
    default: return false;
    }
    return true;
}
#endif


inline void addToolItem(GToolBar* toolbar, StrVar* sv, StrVar* rst) {
    if( rst==NULL || sv==NULL ) return;
    rst->reuse();
    if( SVCHK('n',0) ) {
        TreeNode* node=(TreeNode*)SVO;
        sv=node->gv("@c");
        if( SVCHK('w',12) ) {
            GMenu* menu=(GMenu*)SVO;
            QToolButton* toolButton = new QToolButton();
            toolButton->setMenu(menu);
            toolButton->setPopupMode(QToolButton::InstantPopup);
            node->GetVar("@toolButton")->setVar('w',14,(LPVOID)toolButton );
            sv=node->gv("icon");
            if( SVCHK('i',6) ) {
                QPixmap* px=(QPixmap*)SVO;
                toolButton->setIcon( QIcon(*px) );
            } else if( sv && sv->length() ) {
                QPixmap* px=getPixmap(sv->get());
                if( px ) toolButton->setIcon( QIcon(*px) );
            }
            sv=node->gv("tooltip");
            if( sv ) {
                LPCC tooltip = toString(sv);
                toolButton->setToolTip(KR(tooltip));
            }
            toolbar->addWidget(toolButton);
            rst->setVar('3',1);
        } else if( SVCHK('w',11) ) {
            GAction* act=(GAction*)SVO;
            toolbar->addAction(act);
            rst->setVar('3',1);
        } else {
            QWidget* widget=gwidget(node);
            if( widget ) {
                toolbar->addWidget(widget);
                rst->setVar('3',1);
            }
        }
    } else if( ccmp(sv->get(),"-") ) {
        toolbar->addSeparator();
        rst->setVar('3',1);
    } else {
        LPCC aid=toString(sv);
        GAction* act=getAction(aid,NULL);
        if( act ) {
            toolbar->addAction(act);
            rst->setVar('3',1);
        }
    }
}
inline void makeMenu(GMenu* menu, StrVar* sv) {
    if( menu==NULL || sv==NULL )
        return;
    if( !SVCHK('a',0) ) {
        GAction* act=getAction(sv->get(),NULL);
        if( act )
            menu->addAction(act);
        return;
    }
    TreeNode* cf=menu->config();
    XListArr* a=(XListArr*)SVO;
    sv=menu->config()->gv("@page");
    TreeNode* page = SVCHK('n',0) ? (TreeNode*)SVO:NULL;
    for(int n=0,sz=a->size();n<sz;n++) {
        GMenu* sub=NULL;
        GAction* act=NULL;
        sv=a->get(n);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            if( node->cmp("type","menu") ) {
                sv=node->GetVar("@c");
                if( SVCHK('w',12) ) {
                    sub=(GMenu*)SVO;
                    sub->clear();
                } else {
                    sub=new GMenu(node,menu);
                    node->xstat=WTYPE_MENU;
                    if( page ) {
                        node->GetVar("@page")->setVar('n',0,(LPVOID)page);
                    }
                    sv->setVar('w',12,(LPVOID)sub);
                }
                makeMenu(sub,node->gv("actions"));
                LPCC text = node->get("text");
                QPixmap* icon=NULL;
                if( slen(text) )
                    sub->setTitle(KR(text));
                sv=sub->config()->gv("icon");
                if( SVCHK('i',6) ) {
                    icon=(QPixmap*)SVO;
                } else if( sv && sv->length() ) {
                    icon=getPixmap(toString(sv));
                }
                if( icon ) sub->setIcon(QIcon(*icon));
                sv=cf->gv("tooltip");
                if( sv ) {
                    LPCC tooltip = toString(sv);
                    sub->setToolTip(KR(tooltip));
                }
            } else {
                LPCC text = node->get("text");
                if( ccmp(text,"-") ) {
                    menu->addSeparator();
                } else {
                    LPCC id=node->get("id");
                    if( IndexOf(id,'.')==-1 ) {
                        LPCC pageId=cf->get("id");
                        if( slen(pageId) ) pageId="global";
                        node->set("id", FMT("%s.%s",pageId,id)  );
                    }
                    act=getAction(node->get("id"),node);
                }
            }
        } else if( ccmp(sv->get(),"-") ) {
            menu->addSeparator();
        } else {
            LPCC cd=toString(sv);
            act=getAction(cd,NULL);
        }
        if( act ) {
            menu->addAction(act);
            if( page ) {
                act->config()->GetVar("@page")->setVar('n',0,(LPVOID)page);
            }
        } else if( sub ) {
            menu->addMenu(sub);
        }
    }
}
bool callMainFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    if( widget==NULL )
        widget=gwidget(tn);
    GMainPage* w=qobject_cast<GMainPage*>(widget);
    if( w==NULL )
        return false;
    StrVar* sv=NULL;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"toolBar") ? 1 :
            ccmp(fnm,"menuBar") ? 2 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {   // toolBar
        TreeNode* cf = NULL;
        GToolBar* toolbar=NULL;
        sv=tn->GetVar("@toolbar");
        if( SVCHK('n',0) ) {
            cf=(TreeNode*)SVO;
        } else {
            cf=new TreeNode(2, true);
            sv->setVar('n',0,(LPVOID)cf);
        }
        sv=cf->GetVar("@c");
        if( SVCHK('w',13) ) {
            toolbar=(GToolBar*)SVO;
        } else {
            toolbar=new GToolBar(cf, w);
            sv->setVar('w',13,(LPVOID)toolbar);
        }
        cf->xstat=FNSTATE_TOOLBAR;
        LPCC ty=NULL;
        sv = arrs ? arrs->get(0): NULL;
        if( SVCHK('a',0) ) {
            // toolbar('open,save', 'left');
            XListArr* a=(XListArr*)SVO;
            ty=cnt==2? AS(1): "top";
            for(int n=0,sz=a->size();n<sz;n++) {
                addToolItem(toolbar, a->get(n), rst );
            }
        } else if( cnt==1 ) {
            ty=AS(0);
        }
        w->addToolBar(
            ccmp(ty,"top") ? Qt::TopToolBarArea :
            ccmp(ty,"left") ? Qt::LeftToolBarArea :
            ccmp(ty,"right") ? Qt::RightToolBarArea :
            ccmp(ty,"bottom") ? Qt::BottomToolBarArea :
            Qt::TopToolBarArea, toolbar);
        rst->setVar('n',0,(LPVOID)cf);
    } break;
    case 2: {   // menuBar
        if( cnt==0 ) {
            sv=tn->gv("@menus");
            if( SVCHK('a',0) ) {
                rst->reuse()->add(sv);
            }
            return true;
        }
        TreeNode* node = NULL;
        sv= arrs->get(0);
        if( SVCHK('n',0) ) {
            node = (TreeNode*)SVO;
            sv = node->gv("actions");
        }
        /*
        QList<QAction*> list = w->menuBar()->actions();
        for( int n=0;n<list.size();n++ ) {
            w->menuBar()->removeAction(list.at(n));
        }
        */
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            for(int n=0,sz=a->size();n<sz;n++) {
                GMenu* menu=NULL;
                sv=a->get(n);
                LPCC sep=NULL;
                if( SVCHK('n',0) ) {
                    TreeNode* cur=(TreeNode*)SVO;
                    sv=cur->GetVar("@c");
                    if( SVCHK('w',12) ) {
                        menu=(GMenu*)SVO;
                        menu->clear();
                    } else {
                        menu=new GMenu(cur,w);
                        cur->xstat=WTYPE_MENU;
                        sv->setVar('w',12,(LPVOID)menu);
                    }
                    if( menu ) {
                        menu->config()->GetVar("@page")->setVar('n',0,(LPVOID)w->config());
                        makeMenu(menu,cur->gv("actions"));
                        LPCC text = menu->config()->get("text");
                        if( slen(text) )
                            menu->setTitle(KR(text));
                    }
                } else {
                    sep=toString(sv);
                }
                /*
                QMenuBar* mb=w->menuBar();
                if( menu )
                    mb->addMenu(menu);
                else if( ccmp(sep,"-") )
                    mb->addSeparator();
                */
            }
            tn->GetVar("@menus")->setVar('a',0,(LPVOID)a);
        }
    } break;
    default: { return false; }
    }
    return true;

}

bool callDivFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    if( widget==NULL )
        widget=gwidget(tn);
    GStacked* w=qobject_cast<GStacked*>(widget);
    if( w==NULL )
        return false;
    StrVar* sv=NULL;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"addPage") ? 1 :
            ccmp(fnm,"removePage") ? 2 :
            ccmp(fnm,"isPage") ? 3 :
            ccmp(fnm,"pages") ? 4 :
            ccmp(fnm,"current") ? 5 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {   // addPage
        if( cnt==0 )
            return false;
        QWidget* widget=NULL;
        TreeNode* cf= NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            cf = (TreeNode*)SVO;
            widget=gwidget(cf);
        }
        if( widget ) {
            bool ok = true;
            for( int n=0;n<w->count();n++) {
                if( w->widget(n)==widget ) {
                    ok=false;
                    break;
                }
            }
            if( ok ) {
                w->addWidget(widget);
                tn->addChild(cf);
            }
            if( isVarTrue(arrs->get(1)) ) {
                w->setCurrentWidget(widget);
            }
            rst->setVar('n',0,(LPVOID)cf);
        }
    } break;
    case 2: {   // removePage
        if( cnt==0 )
            return false;
        QWidget* widget=NULL;
        TreeNode* cf= NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            cf = (TreeNode*)SVO;
            widget=gwidget(cf);
            if( widget ) {
                tn->remove(cf);
                w->removeWidget(widget);
            }
        }
    } break;
    case 3: {   // isPage
        QWidget* widget=NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* cf=(TreeNode*)SVO;
            widget=gwidget(cf);
        }
        rst->setVar('3',widget&&w->indexOf(widget)!=-1 ? 1:0 );

    } break;
    case 4: {   // pages
        if( cnt==0 ) {
            XListArr* a =tn->addArray("@widgetArray", true);
            for( int n=0;n<w->count();n++ ) {
                TreeNode* cf = getWidgetConfig(w->widget(n));
                if( cf ) {
                    a->add()->setVar('n',0, (LPVOID)cf);
                }
            }
            rst->setVar('a',0,(LPVOID)a);
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                int n=toInteger(arrs->get(0));
                if( n < w->count() ) {
                    TreeNode* cf = getWidgetConfig(w->widget(n) );
                    rst->setVar('n',0,(LPVOID)cf);
                }
            } else {
                LPCC code=AS(0);
                for( int n=0;n<w->count();n++ ) {
                    TreeNode* cf = getWidgetConfig(w->widget(n) );
                    if( cf ) {
                        LPCC id=cf->get("id");
                        if( ccmp(id,code) ) {
                            rst->setVar('n',0,(LPVOID)cf);
                        }
                    }
                }
            }
        }
    } break;
    case 5: {   // current
        QWidget* widget=NULL;
        if( cnt==1 ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                TreeNode* cf=(TreeNode*)SVO;
                widget=gwidget(cf);
                if( widget ) {
                    w->setCurrentWidget(widget);
                    rst->setVar('n',0, (LPVOID)cf);
                }
            }
        } else {
            TreeNode* cf = getWidgetConfig(w->currentWidget() );
            if(cf) {
                rst->setVar('n',0,(LPVOID)cf);
            }
        }
    } break;
    default: { return false; }
    }
    return true;
}


bool callTabFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
   int cnt = arrs ? arrs->size(): 0;
   if( widget==NULL )
       widget=gwidget(tn);

   GTab* w=qobject_cast<GTab*>(widget);
   if( w==NULL )
       return false;
   StrVar* sv=NULL;
   U32 ref = fc ? fc->xfuncRef: 0;
   if( ref==0 ) {
       ref=
           ccmp(fnm,"addPage") ? 1 :
           ccmp(fnm,"current") ? 2 :
           ccmp(fnm,"index") ? 3 :
           ccmp(fnm,"tabButton") ? 4 :
           ccmp(fnm,"tabEnable") ? 5 :
           ccmp(fnm,"removePage") ? 6 :
           ccmp(fnm,"is") ? 173 :
           ccmp(fnm,"tabPosition") ? 8 :
           ccmp(fnm,"tabCount") ? 9 :
           0;
       if( fc ) fc->xfuncRef=ref;
   }
   if( ref==0 )
       return false;
   switch( ref ) {
   case 1: {   // addPage
       QWidget* widget=NULL;
       TreeNode* cf = NULL;
       sv=arrs->get(0);
       if( SVCHK('n',0) ) {
           cf = (TreeNode*)SVO;
           widget=gwidget(cf);
       }
       if( widget ) {
           for( int n=0,sz=w->count();n<sz;n++ ) {
               if( w->widget(n)==widget ) {
                   w->setCurrentWidget(widget);
                   return true;
               }
           }
       }
       if( widget && cnt>1 ) {
            if( cnt==2 ) {
               w->addTab(widget,KR(AS(1)));
            } else {
               QPixmap* pm = getStrVarPixmap(arrs->get(2));
               if( pm ) {
                   w->addTab(widget, QIcon(*pm), KR(AS(1)));
               } else {
                   w->addTab(widget,KR(AS(1)));
               }
            }
            if( isVarTrue(arrs->get(3)) ) {
                w->setCurrentWidget(widget);
            }
            tn->addChild(cf);
            rst->setVar('n',0,(LPVOID)cf);
       }
   } break;
   case 2: {   // current
       TreeNode* cf=NULL;
       QWidget* widget=NULL;
       if( cnt==1 ) {
           sv=arrs->get(0);
           if( SVCHK('n',0) ) {
               cf=(TreeNode*)SVO;
               widget=gwidget(cf);
           } else if( isNumberVar(sv) ) {
               int idx=toInteger(sv);
               if( idx>=0 && idx<w->count() ) {
                   widget=w->widget(idx);
                   cf = getWidgetConfig(widget,0);
               }
           }
           if( widget ){
               w->setCurrentWidget(widget);
           }
       } else {
           cf = getWidgetConfig(w->currentWidget(),0);
       }
       if( cf ) {
           rst->setVar('n',0, (LPVOID)cf);
       }
   } break;
   case 3: {   // index
       if( cnt==0 ) return false;
       TreeNode* cur=NULL;
       int idx=-1;
       if( SVCHK('n',0) ) {
           cur=(TreeNode*)SVO;
           for( int n=0;n<w->count();n++ ) {
               TreeNode* cf = getWidgetConfig(w->widget(n),0);
               if( cf==cur ) {
                   idx=n;
                   break;
               }
           }
       }
       rst->setVar('0',0).addInt(idx);
   } break;
   case 4: {   // tabButton
       if( cnt==0 ) return false;
       QTabBar *tabBar = w->findChild<QTabBar*>();

       sv=arrs->get(0);
       TreeNode* cur=NULL;
       int idx=-1;
       if( SVCHK('n',0) ) {
           cur=(TreeNode*)SVO;
           for( int n=0;n<w->count();n++ ) {
               TreeNode* cf = getWidgetConfig(w->widget(n),0);
               if( cf==cur ) {
                    idx=n;
                    break;
               }
           }
       }
       if( idx!=-1 ) {
           QWidget* button=NULL;
           TreeNode* buttonNode=NULL;
           sv=arrs->get(1);
           if( SVCHK('n',0) ) {
               buttonNode=(TreeNode*)SVO;
               button=gwidget(buttonNode);
           }
           if( button ) {
               LPCC side=AS(2);
               if( cur && buttonNode ) {
                   cur->GetVar("@tabButton")->setVar('n',0,(LPVOID)buttonNode);
               }
               tabBar->setTabButton(idx, ccmp(side,"left") ? QTabBar::LeftSide: QTabBar::RightSide, button);
           }
       }

   } break;
   case 5: {   // tabEnable
       sv=arrs->get(0);
       if( isNumberVar(sv) ) {
           int idx=toInteger(sv);
           w->setTabEnabled(idx, isVarTrue(arrs->get(1)) );
       }

   } break;
   case 6: {   // removePage
       if( cnt==0 ) {
           return true;
       }
       TreeNode* cur=NULL;
       int idx=-1;
       sv=arrs->get(0);
       if( SVCHK('n',0) ) {
           cur=(TreeNode*)SVO;
           for( int n=0;n<w->count();n++ ) {
               TreeNode* cf = getWidgetConfig(w->widget(n) );
               if( cf==cur ) {
                    idx=n;
                    break;
               }
           }
       } else if( SVCHK('3',1) ) {
           for( int n=w->count()-1;n>=0;n-- ) {
               TreeNode* page = getWidgetConfig(w->widget(n),0);
               if( page ) {
                   w->removeTab(n);
               }
           }
           tn->xchilds.reuse();
       } else if( isNumberVar(sv) ) {
           idx=toInteger(sv);
       }
       if( idx!=-1 && idx<w->count() ) {
           QWidget* widget=w->widget(idx);
           if( widget ) {
               cur=getWidgetConfig(widget );
           }
           w->removeTab(idx);
       }
       if( cur ) {
           tn->remove(cur);
       }
   } break;
   case 173: {   // is
       if(cnt==0) {
           return false;
       }
       LPCC ty=AS(0);
       if(ccmp(ty,"movable")) {
           sv=arrs->get(1);
           if(sv==NULL) {
               rst->setVar('3', w->isMovable()?1:0);
           } else {
               w->setMovable(SVCHK('3',1));
           }
       } else if(fc) {
           return false;
       }
   } break;
   case 8: {   // tabPosition
       LPCC p=cnt==0 ? "north": AS(0);
       w->setTabPosition(
           ccmp(p,"north") ? QTabWidget::TabPosition::North :
           ccmp(p,"south") ? QTabWidget::TabPosition::South :
           ccmp(p,"west") ? QTabWidget::TabPosition::West :
           ccmp(p,"east") ? QTabWidget::TabPosition::East : QTabWidget::TabPosition::North);

   } break;
   case 9: {   // tabCount
       rst->setVar('0',0).addInt(w->count());
   } break;
   default: { return false; }
   }
   return true;
}

bool callInputFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GInput* widget, XFunc* fc) {
    if( widget==NULL )
        return false;
    StrVar* sv=NULL;
    U32 fref=fc->xfuncRef;
    if( fref==0 ) {
        fref=
            ccmp(fnm, "value") ? 811 :
            ccmp(fnm, "select") ? 2 :
            ccmp(fnm, "append") ? 3 :
            ccmp(fnm, "at") ? 4 :
            ccmp(fnm, "pos") ? 5 :
            ccmp(fnm, "defaultText") ? 6 :
            ccmp(fnm, "inlineButton") ? 7 :
            ccmp(fnm, "complete") ? 8 :
            ccmp(fnm, "align") ? 9 :
            ccmp(fnm, "format") ? 10 :
            ccmp(fnm, "mask") ? 11 :
            ccmp(fnm, "maxLength") ? 12 :
            ccmp(fnm, "readOnly") ? 13 :
            ccmp(fnm, "style") ? 14 :
            ccmp(fnm, "mode") ? 15 :
            ccmp(fnm, "googleCompleter") ? 16 :
            ccmp(fnm, "treePopup") ? 17 :
            ccmp(fnm, "isPopup") ? 18 :
            0;
        fc->xfuncRef=fref;
    }
    int cnt=arrs? arrs->size(): 0;
    switch(fref ) {
    case 811: { // value
        if( cnt==0 ) {
            rst->set(Q2A(widget->text()) );
        } else {
            LPCC txt=AS(0);
            widget->setText(KR(txt));
        }

    } break;
    case 2: { // select
        if( cnt==0 ) {
            widget->selectAll();
        } else {
            LPCC type=AS(0);
            if( ccmp(type,"text") ) {
                rst->set(Q2A(widget->selectedText()));
            } else if( ccmp(type,"pos") ) {
                if( cnt==3 ) {
                    widget->setSelection(toInteger(arrs->get(1)),toInteger(arrs->get(2)) );
                } else {
                    rst->setVar('0',0).addInt( widget->selectionStart() );

                }
            }
        }
    } break;
    case 3: { // append
        if( cnt==0 ) return true;
        QString txt=widget->text();
        txt+=KR(AS(0));
        widget->setText(txt);
    } break;
    case 4: { // at
        if( cnt==1 ) {
            sv=arrs->get(0);
            if( SVCHK('i',2) ) {
                rst->setVar('0',0).addInt( widget->cursorPositionAt(QPoint(sv->getInt(4),sv->getInt(8))) );
            }
        } else if( cnt==2 ) {
            int x=toInteger(arrs->get(0)), y=toInteger(arrs->get(1));
            rst->setVar('0',0).addInt( widget->cursorPositionAt(QPoint(x,y)) );
        }
    } break;
    case 5: { // pos
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(widget->cursorPosition());
        } else if( isNumberVar(arrs->get(0)) ) {
            widget->setCursorPosition(toInteger(arrs->get(0)));
        }
    } break;
    case 6: { // defaultText
        if( cnt ) {
            LPCC def = AS(0);
            widget->setPlaceholderText(KR(def));
        }
    } break;
    case 7: { // inlineButton
        if( cnt==0 ) {
            return false;
        }
        sv = tn->gv("@inlineButton");
        GToolButton* btn = NULL;
        if( SVCHK('w',51) ) {
            btn = (GToolButton*)SVO;
        }
        LPCC ty = AS(0);
        if( ccmp(ty,"create") && btn==NULL ) {
            StrVar* sv=arrs->get(1);
            if( SVCHK('n',0) ) {
                TreeNode* node = (TreeNode*)SVO;
                if( node->xstat!=WTYPE_TOOLBTN ) {
                    createWidget("toolbutton", node, widget);//(node, widget, NULL, rst, 0 );
                    btn = qobject_cast<GToolButton*>(gwidget(node));
                    if( btn ) {
                        sv = node->gv("icon");
                        if( sv ) {
                            QPixmap* pixmap = getStrVarPixmap(sv);
                            if( pixmap ) {
                                btn->setIcon(QIcon(*pixmap));
                            }
                        }
                        int hei=24, padding=2;
                        sv = node->gv("height");
                        if( isNumberVar(sv) ) hei=toInteger(sv);
                        sv = node->gv("padding");
                        if( isNumberVar(sv) ) padding=toInteger(sv);
                        LPCC sty=FMT("QToolButton { border: none; padding: %dpx}",padding);
                        btn->setCursor(Qt::ArrowCursor);
                        btn->setStyleSheet(sty);
                        // widget->setFixedHeight(h+2);
                        // widget->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(hei-frameWidth));
                        int frameWidth = widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
                        btn->setFixedSize(hei, hei);
                        btn->move(widget->width() - btn->width() - frameWidth, 0);
                        btn->show();
                        tn->GetVar("@inlineButton")->setVar('w',51,(LPVOID)btn);
                    }
                }
                return true;
            }
        }
        if( btn==NULL ) {
            return false;
        }
        if( ccmp(ty,"icon") ) {
            QPixmap* pixmap = getStrVarPixmap(arrs->get(1));
            if( pixmap ) {
                btn->setIcon(QIcon(*pixmap));
            }
        } else if( ccmp(ty,"show") ) {
            int h = btn->height() - 5;
            int frameWidth = widget->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
            btn->show();
            widget->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(h - frameWidth));
        } else if( ccmp(ty,"hide") ) {
            btn->hide();
            widget->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(0));
        } else if( ccmp(ty,"disable") ) {
            btn->setDisabled(true);
        } else if( ccmp(ty,"enable") ) {
            btn->setEnabled(true);
        } else if( ccmp(ty,"onClick") ) {
            sv = arrs->get(1);
            TreeNode* thisNode=NULL;
            if( SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(2);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                TreeNode* node = btn->config();
                if( node ) {
                   XFuncNode* fnCur = setCallbackFunction(fsrc, fn, node, NULL, node->GetVar("onClick"));
                   node->GetVar("@input")->setVar('n',0,(LPVOID)tn);
                   if(thisNode) {
                       thisNode->GetVar("@target")->setVar('n',0,(LPVOID)node);
                       fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                   }
                }
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 8: { // complete
        if( cnt==0 ) {
            QCompleter* c = widget->completer();
            rst->setVar('3', c==NULL? 0: 1);
            return false;
        }
        LPCC ty = AS(0);
        if( ccmp(ty,"create") ) {
            QCompleter* c = widget->completer();
            if( c==NULL ) {
                c = new QCompleter(widget);
            }
            StrVar* sv = arrs->get(1);
            bool ok = true;
            if( SVCHK('n',0) ) {
                TreeNode* cur = (TreeNode*)SVO;
                sv=cur->GetVar("@c");
                if( SVCHK('m',0) ) {
                    c->setModel((TreeModel*)SVO);
                    c->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
                } else {
                    LPCC field = AS(2);
                    if( slen(field)==0 ) field = "text";
                    QStringList list;
                    QStringListModel* model = new QStringListModel(c);
                    for( int n=0,sz=cur->childCount(); n<sz; n++ ) {
                        LPCC val = cur->child(n)->get(field);
                        list.append(KR(val));
                    }
                    model->setStringList(list);
                    c->setModel(model);
                }
            } else if( SVCHK('a',0) ) {
                XListArr* a = (XListArr*)SVO;
                QStringList list;
                QStringListModel* model = new QStringListModel(c);
                for( int n=0,sz=a->size(); n<sz; n++ ) {
                    LPCC val = toString(a->get(n));
                    list.append(KR(val));
                }
                model->setStringList(list);
                c->setModel(model);
            } else {
                LPCC val = toString(sv);
                QStringListModel* model = qobject_cast<QStringListModel*>(c->model());
                if( model==NULL ) {
                    model = new QStringListModel(c);
                    c->setModel(model);
                } else {
                    ok = false;
                }
                int cnt = model->rowCount();
                QModelIndex index = model->index(cnt, 0);
                if( slen(val) ) model->setData(index, KR(val));
            }
            if( ok ) widget->setCompleter(c);
        } else if( ccmp(ty,"add") ) {
            // version 1.0.3
            QCompleter* c = widget->completer();
            if( c==NULL ) {
                return true;
            }            
            QStringListModel* model = qobject_cast<QStringListModel*>(c->model());
            if( model==NULL ) {
                model = new QStringListModel(c);
                c->setModel(model);
            }
            sv=arrs->get(1);
            if( SVCHK('a',0) ) {
                XListArr* a = (XListArr*)SVO;
                for(int n=0;n<a->size();n++) {
                    LPCC val=toString(a->get(n));
                    if( slen(val) ) { 
                        QString str=KR(val);
                        if( model->stringList().indexOf(str)==-1 ) {
                            model->insertRow(0);
                            model->setData(model->index(0), str);
                        }
                    }
                }
            } else {
                LPCC val=toString(sv);
                if( slen(val) ) {
                    // int cnt = model->rowCount();
                    QString str=KR(val);
                    if( model->stringList().indexOf(str)==-1 ) {
                        model->insertRow(0);
                        model->setData(model->index(0), str);
                    }
                }
            }
        } else if( ccmp(ty,"mode") ) {
            LPCC kind=AS(1);
            QCompleter* c = widget->completer();
            if( c==NULL ) return false;
            if( ccmp(kind,"inline") ) c->setCompletionMode(QCompleter::InlineCompletion);
            else if( ccmp(kind,"popup") ) c->setCompletionMode(QCompleter::PopupCompletion);
            else if( ccmp(kind,"unfilter") ) c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
            else if( ccmp(kind,"caseSensitive") ) c->setCaseSensitivity(isVarTrue(arrs->get(2))?Qt::CaseSensitive: Qt::CaseInsensitive );
            else if( ccmp(kind,"maxVisibleItems") ) c->setMaxVisibleItems(toInteger(arrs->get(2)) );
            else if( ccmp(kind,"wrapAround") ) c->setWrapAround(isVarTrue(arrs->get(2)) );
            else if( ccmp(kind,"modelColumn") ) c->setCompletionColumn(toInteger(arrs->get(2)) );
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 9: { // align
        if( cnt==0 ) {
            return false;
        }
        LPCC align=AS(0);
        widget->setAlignment((Qt::Alignment)getAlignFlag(align));
    } break;
    case 10: { // format
        if( cnt==0 ) {
            widget->setValidator(NULL);
            return true;
        }
        LPCC fmt = AS(0);
        if( ccmp(fmt,"number") ) {
            widget->setValidator(new QIntValidator(widget));
        } else if( ccmp(fmt,"double") ) {
            widget->setValidator(new QDoubleValidator(widget));
        } else {
            widget->setValidator(new QRegExpValidator( QRegExp(KR(fmt)) ));
        }
    } break;
    case 11: { // mask
        if( cnt==0 ) {
            widget->setInputMask("");
            return true;
        }
        LPCC mask = AS(0);
        widget->setInputMask(KR(mask));
        if( cnt==2 ) {
            widget->setCursorPosition(toInteger(arrs->get(1)));
        }
    } break;
    case 12: { // maxLength
        if( cnt==1 && isNumberVar(arrs->get(0)) )
            widget->setMaxLength(toInteger(arrs->get(0)) );
    } break;
    case 13: { // readOnly
        if( cnt==1  )
            widget->setReadOnly(isVarTrue(arrs->get(0))?true:false);
    } break;
    case 14: { // style
        LPCC sty=cnt?AS(0):"QLineEdit { border: 2px solid gray; border-radius: 5px; }";
        widget->setStyleSheet(KR(sty));
    } break;
    case 15: { // mode
        if( cnt==1 ) {
            LPCC ty = AS(0);
            if( ccmp(ty,"password") ) {
                widget->setEchoMode(QLineEdit::Password);
            } else if( ccmp(ty,"passwordEco") ) {
                widget->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            } else if( ccmp(ty,"noEco") ) {
                widget->setEchoMode(QLineEdit::NoEcho);
            }
        } else {
            widget->setEchoMode(QLineEdit::Normal);
        }
    } break;
    case 16: { // googleCompleter
    } break;
    case 17: { // treePopup
        if( cnt==0 ) return true;
        GInputCompletion* completer = NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* tree=(TreeNode*)SVO;
            GTree* popup= static_cast<GTree*>(gwidget(tree));
            if( popup ) {
                sv=tn->GetVar("@treeCompleter");
                if( SVCHK('g',2) ) {
                    completer=(GInputCompletion*)SVO;
                    completer->setPopup(popup);
                } else {
                    completer=new GInputCompletion(widget, tn, popup);
                    sv->setVar('g',2,(LPVOID)completer);
                }
            }
        } else {
            sv=tn->GetVar("@treeCompleter");
            if( SVCHK('g',2) ) {
                completer=(GInputCompletion*)SVO;
            }
            if( completer ) {
                LPCC type=AS(0);
                if( ccmp(type,"stop") ) completer->preventSuggest();
                else if( ccmp(type,"popup") || ccmp(type,"start") ) completer->autoSuggest();
                else if( ccmp(type,"end") ) completer->doneCompletion();
                else if( ccmp(type,"show") ) completer->showPopup();
            }
        }
    } break;
    case 18: { // isPopup
        QCompleter* c = widget->completer();
        if( c && c->popup() && c->popup()->isVisible() ) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    default: return false;
    }
    return true;
}

bool callCheckFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    if( widget==NULL )
        widget=gwidget(tn);
    GCheck* w=qobject_cast<GCheck*>(widget);
    if( w==NULL )
        return false;
    int cnt=arrs? arrs->size() : 0;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"is") ? 173 :
            ccmp(fnm,"checked") ? 2 :
            ccmp(fnm,"text") ? 3 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 173: {   // is
        if( cnt==0) {
            rst->setVar('3',w->isChecked()? 1:0);
        } else {
            LPCC ty = AS(0);
            if( ccmp(ty,"disable") ) rst->setVar('3', w->isEnabled()? false: true);
            else if( ccmp(ty,"enable") ) rst->setVar('3', w->isEnabled()? true: false);
            else if( ccmp(ty,"checked") ) rst->setVar('3', w->isChecked() ? true: false);
            else if( ccmp(ty,"checkable") ) rst->setVar('3', w->isCheckable() ? true: false);
            else {
                return false;
            }
        }
    } break;
    case 2: {   // checked
        if( arrs ) {
            w->setChecked(isVarTrue(arrs->get(0)));
        }
        rst->setVar('3',w->isChecked()? 1:0);
    } break;
    case 3: {   // text
        if( arrs ) {
            LPCC txt = AS(0);
            w->setText(KR(txt));
        }
        rst->setVar('3',w->isChecked()? 1:0);
    } break;

    default: { return false; }
    }
    return true;
}

bool callRadioFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    if( widget==NULL )
        widget=gwidget(tn);
    GRadio* w=qobject_cast<GRadio*>(widget);
    if( w==NULL )
        return false;
    int cnt=arrs ? arrs->size(): 0;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"is") ? 173 :
            ccmp(fnm,"checked") ? 2 :
            ccmp(fnm,"text") ? 3 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 173: {   // is
        if( cnt==0) {
            rst->setVar('3',w->isChecked()? 1:0);
        } else {
            LPCC ty = AS(0);
            if( ccmp(ty,"disable") ) rst->setVar('3', w->isEnabled()? false: true);
            else if( ccmp(ty,"enable") ) rst->setVar('3', w->isEnabled()? true: false);
            else if( ccmp(ty,"checked") ) rst->setVar('3', w->isChecked() ? true: false);
            else if( ccmp(ty,"checkable") ) rst->setVar('3', w->isCheckable() ? true: false);
            else {
                return false;
            }
        }
    } break;
    case 2: {   // checked
        if( arrs ) {
            w->setChecked(isVarTrue(arrs->get(0)));
        }
        rst->setVar('3',w->isChecked()? 1:0);
    } break;
    case 3: {   // text
        if( arrs ) {
            LPCC txt = AS(0);
            w->setText(KR(txt));
        }
        rst->setVar('3',w->isChecked()? 1:0);
    } break;

    default: { return false; }
    }
    return true;

}
