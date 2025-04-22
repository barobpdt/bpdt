#include "widget_util.h"
#include "../baroscript/func_util.h"
#include "../baroscript/widgets.h"

void initWidget(TreeNode* wcf, QWidget* widget) {
    if( wcf==NULL || widget==NULL ) return;
    StrVar* sv=NULL;
    if( isPageNode(wcf) ) {
        LPCC title=wcf->get("title");
        if( slen(title) ) widget->setWindowTitle(KR(title));
        LPCC icon=wcf->get("icon");
        if( slen(icon) ) {
            QPixmap* pic = getPixmap(icon,true);
            if( pic ) widget->setWindowIcon(QIcon(*pic));
        }
        sv=wcf->gv("state");
        if( sv ) {
            LPCC state=toString(sv);
            if( slen(state) ) {
                widget->setWindowState(
                ccmp(state,"min")? Qt::WindowMinimized : ccmp(state,"max") ? Qt::WindowMaximized :
                ccmp(state,"full")? Qt::WindowFullScreen : ccmp(state,"active")? Qt::WindowActive : Qt::WindowNoState);
            }
        }
        sv=wcf->gv("size");
        if( sv ) {
            XParseVar pv(sv);
            LPCC width=pv.findEnd(",").v();
            LPCC height=pv.findEnd(",").v();
            if( isnum(width) && isnum(height) ) {
                int w=atoi(width), h=atoi(height);
                if( w>0 && h>0 ) {
                    widget->resize(w,h);
                }
            }
        }
        sv=wcf->gv("pos");
        if( sv ) {
            XParseVar pv(sv);
            LPCC xpos=pv.findEnd(",").v();
            LPCC ypos=pv.findEnd(",").v();
            if( isnum(xpos) && isnum(ypos) ) {
                int x=atoi(xpos), y=atoi(ypos);
                widget->move(x,y);
            }
        }
    } else {
        GButton* btn=qobject_cast<GButton*>(widget);
        if( btn ) {
            LPCC text=wcf->get("text"), icon=wcf->get("icon");
            if( slen(text)) btn->setText(KR(text));
            if( slen(icon) ) {
                QPixmap* pic=getPixmap(icon,true);
                if( pic ) btn->setIcon(QIcon(*pic));
            }
        }
        sv=wcf->gv("font");
        if( sv==NULL ) {
            sv=cfVar("@widgetFont");
        }
        if( sv && sv->length() ) {
            QFont font=widget->font();
            if( isNumberVar(sv) ) {
                font.setPointSize(toInteger(sv));
            } else {
                getFontInfo(&font, toString(sv));
            }
            widget->setFont(font);
        }
    }
    sv=wcf->gv("style");
    if(sv) {
        LPCC sty=toString(sv);
        widget->setStyleSheet(KR(sty));
    }
    sv=wcf->gv("state");
    if( sv ) {
        qtSetWindowState(widget, toString(sv));
    }
    sv=wcf->gv("flags");
    if( sv ) {
        qtSetWindowFlags(widget, sv );
    }
    sv=wcf->gv("width");
    if( isNumberVar(sv) ) {
        widget->setFixedWidth(toInteger(sv));
    }
    sv=wcf->gv("height");
    if( isNumberVar(sv) ) {
        widget->setFixedHeight(toInteger(sv));
    }
    sv=wcf->gv("tip"); if( sv==NULL ) sv=wcf->gv("tooltip");
    if( sv ) {
        LPCC tooltip=toString(sv);
        widget->setToolTip(KR(tooltip));
    }
    sv=wcf->gv("style");
    if( sv ) {
        LPCC style=toString(sv);
        if( slen(style) ) widget->setStyleSheet(KR(style));
    }
    sv=wcf->gv("maxWidth");
    if( isNumberVar(sv) ) {
        widget->setMaximumWidth(toInteger(sv));
    }
    sv=wcf->gv("maxHeight");
    if( isNumberVar(sv) ) {
        widget->setMaximumHeight(toInteger(sv));
    }
    sv=wcf->gv("minWidth");
    if( isNumberVar(sv) ) {
        widget->setMinimumWidth(toInteger(sv));
    }
    sv=wcf->gv("minHeight");
    if( isNumberVar(sv) ) {
        widget->setMinimumHeight(toInteger(sv));
    }
    sv=wcf->gv("hide");
    if( isVarTrue(sv) ) {
        widget->hide();
    }
    sv=wcf->gv("margin");
    if( isNumberVar(sv) ) {
        int num=toInteger(sv);
        widget->setContentsMargins(num,num,num,num);
    } else if( sv ) {
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
        widget->setContentsMargins(l,t,r,b);
    }
    if( isVarTrue(wcf->gv("dropUse")) ) {
        widget->setAcceptDrops(true);
    }
    if( isVarTrue(wcf->gv("expand")) || isVarTrue(wcf->gv("expandAll")) ) {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else if( isVarTrue(wcf->gv("expandWidth")) ) {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    } else if( isVarTrue(wcf->gv("expandHeight")) ) {
        widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    }
}

#ifdef WIDGETMAP_USE
#include "editor_util.h"
#include "tree_util.h"
#include "treemodel.h"

#ifdef MPLAYER_USE
#include "widget_myplayer.h"
MyMPlayer* myPlayer(WidgetConf* wcf) {
    MyMPlayer* w=wcf->xwidget ? qobject_cast<MyMPlayer*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->player(wcf->isset("id") ? wcf->get("id"): "#player", wcf);
        }
    }
    return w;
}

MyMPlayer* initPlayer(WidgetConf* wcf) {
    MyMPlayer* w=myPlayer(wcf);
    return w;
}
#endif

#ifdef WEBVIEW_USE
#include "widget_webview.h"
WebView* myWebView(WidgetConf* wcf) {
    WebView* w=wcf->xwidget ? qobject_cast<WebView*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->webview(wcf->isset("id") ? wcf->get("id"): "#player", wcf);
        }
    }
    return w;
}
WebView* initWebView(WidgetConf* wcf) {
    WebView* w=myWebView(wcf);
    return w;
}
#endif

//#> page init
inline bool initModelData(WidgetConf* wcf) {
    return wcf->isNode("data") && setTreeModelHeader(wcf)>0 ? true: false;
}
bool initPage(WidgetMap* ui) {
    if( ui==NULL ) return false;
    TreeNode* map=ui->pageMap();
    TreeNode* pageConf=ui->pageConf();
    return true;
}


bool setEventFunc_initPage(WidgetMap* ui ) {
    return initPage(ui);
}
bool setEventFunc_change(WidgetConf* wcf, PWIDGET_CHANGE fpChange ) {
    if( fpChange) {
        switch(wcf->xtype ) {
        case 22: myProgress(wcf)->onChange=fpChange; break;
        case 23: myCombo(wcf)->onChange=fpChange; break;
        case 90: myTab(wcf)->onChange=fpChange; break;
        case 93: myDiv(wcf)->onChange=fpChange; break;
        case 28: myCheck(wcf)->onChange=fpChange; break;
        case 29: mySpin(wcf)->onChange=fpChange; break;
        case 32: myInput(wcf)->onChange=fpChange; break;
        case 35: myDate(wcf)->onChange=fpChange; break;
        case 36: myTime(wcf)->onChange=fpChange; break;
        case 37: myCalendar(wcf)->onChange=fpChange; break;
        case 38: myToolbar(wcf)->onChange=fpChange; break;
        case 39: myToolbox(wcf)->onChange=fpChange; break;
        case 100: myAction(wcf)->onChange=fpChange; break;
        default: return false;
        }
        return true;
    }
    return false;
}

bool setEventFunc_click(WidgetConf* wcf, PWIDGET_CLICK fpClick ) {
    if( fpClick ) {
        switch(wcf->xtype ) {
        case 10: myCanvas(wcf)->onClick=fpClick; break;
        case 25: myButton(wcf)->onClick=fpClick; break;
        case 26: myToolButton(wcf)->onClick=fpClick; break;
        case 27: myRadio(wcf)->onClick=fpClick; break;
        case 28: myCheck(wcf)->onClick=fpClick; break;
        case 37: myCalendar(wcf)->onClick=fpClick; break;
        case 102: myTray(wcf)->onClick=fpClick; break;
        default: return false;
        }
        return true;
    }
    return false;

}

//#> widget create

MyCanvas *myCanvas(WidgetConf *wcf)
{
    MyCanvas* w=wcf->xwidget ? qobject_cast<MyCanvas*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->canvas(wcf->isset("id") ? wcf->get("id"): "#canvas", wcf);
        }
    }
    return w;
}

MyPushButton* myButton(WidgetConf* wcf) {
    MyPushButton* w=wcf->xwidget ? qobject_cast<MyPushButton*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->button(wcf->isset("id") ? wcf->get("id"): "#button", wcf);
        }
    }
    return w;
}
MyLabel* myLabel(WidgetConf* wcf) {
    MyLabel* w=wcf->xwidget ? qobject_cast<MyLabel*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->label(wcf->isset("id") ? wcf->get("id"): "#label", wcf);
        }
    }
    return w;
}
MyTabWidget* myTab(WidgetConf* wcf) {
    MyTabWidget* w=wcf->xwidget ? qobject_cast<MyTabWidget*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->tab(wcf->isset("id") ? wcf->get("id"): "#tab", wcf);
            _LOG("xxxxxxxxxxxxxx %s xxxxxxxxxx", wcf->log() );
        }
    }
    return w;
}
MySplitter* mySplitter(WidgetConf* wcf) {
    MySplitter* w=wcf->xwidget ? qobject_cast<MySplitter*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->splitter(wcf->isset("id") ? wcf->get("id"): "#splitter", wcf);
        }
    }
    return w;
}
MyGroupBox* myGroup(WidgetConf* wcf) {
    MyGroupBox* w=wcf->xwidget ? qobject_cast<MyGroupBox*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->group(wcf->isset("id") ? wcf->get("id"): "#group", wcf);
        }
    }
    return w;
}
MyStackedWidget* myDiv(WidgetConf* wcf) {
    MyStackedWidget* w=wcf->xwidget ? qobject_cast<MyStackedWidget*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->div(wcf->isset("id") ? wcf->get("id"): "#div", wcf);
        }
    }
    return w;
}
MyToolButton* myToolButton(WidgetConf* wcf) {
    MyToolButton* w=wcf->xwidget ? qobject_cast<MyToolButton*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->tool(wcf->isset("id") ? wcf->get("id"): "#tool", wcf);
        }
    }
    return w;
}
MyRadioButton* myRadio(WidgetConf* wcf) {
    MyRadioButton* w=wcf->xwidget ? qobject_cast<MyRadioButton*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->radio(wcf->isset("id") ? wcf->get("id"): "#radio", wcf);
        }
    }
    return w;
}
MyCheckBox* myCheck(WidgetConf* wcf) {
    MyCheckBox* w=wcf->xwidget ? qobject_cast<MyCheckBox*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->check(wcf->isset("id") ? wcf->get("id"): "#check", wcf);
        }
    }
    return w;
}
MySpinBox* mySpin(WidgetConf* wcf) {
    MySpinBox* w=wcf->xwidget ? qobject_cast<MySpinBox*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->spin(wcf->isset("id") ? wcf->get("id"): "#spin", wcf);
        }
    }
    return w;
}
MyLineEdit* myInput(WidgetConf* wcf) {
    MyLineEdit* w=wcf->xwidget ? qobject_cast<MyLineEdit*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->input(wcf->isset("id") ? wcf->get("id"): "#input", wcf);
        }
    }
    return w;
}
MyPlainTextEdit* myTextArea(WidgetConf* wcf) {
    MyPlainTextEdit* w=wcf->xwidget ? qobject_cast<MyPlainTextEdit*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->textarea(wcf->isset("id") ? wcf->get("id"): "#textarea", wcf);
        }
    }
    return w;
}
MyDateEdit* myDate(WidgetConf* wcf) {
    MyDateEdit* w=wcf->xwidget ? qobject_cast<MyDateEdit*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->date(wcf->isset("id") ? wcf->get("id"): "#date", wcf);
        }
    }
    return w;
}
MyTimeEdit* myTime(WidgetConf* wcf) {
    MyTimeEdit* w=wcf->xwidget ? qobject_cast<MyTimeEdit*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->time(wcf->isset("id") ? wcf->get("id"): "#time", wcf);
        }
    }
    return w;
}
MyCalendarWidget* myCalendar(WidgetConf* wcf) {
    MyCalendarWidget* w=wcf->xwidget ? qobject_cast<MyCalendarWidget*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->calendar(wcf->isset("id") ? wcf->get("id"): "#calendar", wcf);
        }
    }
    return w;
}
MyToolBar* myToolbar(WidgetConf* wcf) {
    MyToolBar* w=wcf->xwidget ? qobject_cast<MyToolBar*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->toolbar(wcf->isset("id") ? wcf->get("id"): "#toolbar", wcf);
        }
    }
    return w;
}
MyToolBox* myToolbox(WidgetConf* wcf) {
    MyToolBox* w=wcf->xwidget ? qobject_cast<MyToolBox*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->toolbox(wcf->isset("id") ? wcf->get("id"): "#toolbox", wcf);
        }
    }
    return w;
}
MyProgressBar* myProgress(WidgetConf* wcf) {
    MyProgressBar* w=wcf->xwidget ? qobject_cast<MyProgressBar*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->progress(wcf->isset("id") ? wcf->get("id"): "#progress", wcf);
        }
    }
    return w;
}
MyListView* myList(WidgetConf* wcf) {
    MyListView* w=wcf->xwidget ? qobject_cast<MyListView*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->list(wcf->isset("id") ? wcf->get("id"): "#list", wcf);
        }
    }
    return w;
}
MyComboBox* myCombo(WidgetConf* wcf) {
    MyComboBox* w=wcf->xwidget ? qobject_cast<MyComboBox*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->combo(wcf->isset("id") ? wcf->get("id"): "#combo", wcf);
        }
    }
    return w;
}
MyTree* myTree(WidgetConf* wcf) {
    MyTree* w=wcf->xwidget ? qobject_cast<MyTree*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->tree(wcf->isset("id") ? wcf->get("id"): "#tree", wcf);
        }
    }
    return w;
}
MyTextEdit* myEditor(WidgetConf* wcf) {
    MyTextEdit* w=wcf->xwidget ? qobject_cast<MyTextEdit*>(wcf->xwidget): NULL;
    if( w==NULL ) {
        if( wcf->xwidgetMap ) {
            w=wcf->xwidgetMap->editor(wcf->isset("id") ? wcf->get("id"): "#editor", wcf);
        }
    }
    return w;
}
MyAction* myAction(WidgetConf* wcf) {
    MyAction* w=NULL;
    if( wcf->xwidgetMap ) {
        w=wcf->xwidgetMap->action(wcf->isset("id") ? wcf->get("id"): "#action", wcf );
    }
    return w;
}
MyMenu* myMenu(WidgetConf* wcf) {
    MyMenu* w=NULL;
    if( wcf->xwidgetMap ) {
        w=wcf->xwidgetMap->menu(wcf->isset("id") ? wcf->get("id"): "#menu", wcf );
    }
    return w;
}
MySystemTrayIcon* myTray(WidgetConf* wcf) {
    MySystemTrayIcon* w=NULL;
    if( wcf->xwidgetMap ) {
        w=wcf->xwidgetMap->tray(wcf->isset("id") ? wcf->get("id"): "#tray", wcf );
    }
    return w;
}
//#> widget init

MyCanvas* initCanvas(WidgetConf* wcf) {
    MyCanvas* w=myCanvas(wcf);
    return w;
}
MyListView* initList(WidgetConf* wcf) {
    MyListView* w=myList(wcf);
    if( initModelData(wcf) ) w->initModel();
    return w;
}
MyComboBox* initCombo(WidgetConf* wcf) {
    MyComboBox* w=myCombo(wcf);
    if( initModelData(wcf) ) w->initModel();
    return w;
}
MyTree* initTree(WidgetConf* wcf) {
    MyTree* w=myTree(wcf);
    if( initModelData(wcf) ) w->initModel();
    return w;
}
MyTextEdit* initEditor(WidgetConf* wcf) {
    MyTextEdit* w=myEditor(wcf);
    editor_syntaxApply(w, wcf->gv("@syntax"));
    return w;
}
MyPushButton* initButton(WidgetConf* wcf) {
    MyPushButton* w=myButton(wcf);
    w->setText( WCFSS("text",wcf->get("id")) );
    return w;
}
MyLabel* initLabel(WidgetConf* wcf) {
    MyLabel* w=myLabel(wcf);
    w->setText( WCFSS("text",wcf->get("id")));
    return w;
}
MyTabWidget* initTab(WidgetConf* wcf) {
    MyTabWidget* w=myTab(wcf);
    return w;
}
MySplitter* initSplitter(WidgetConf* wcf) {
    MySplitter* w=mySplitter(wcf);
    return w;
}
MyGroupBox* initGroup(WidgetConf* wcf) {
    MyGroupBox* w=myGroup(wcf);
    return w;
}
MyStackedWidget* initDiv(WidgetConf* wcf) {
    MyStackedWidget* w=myDiv(wcf);
    return w;
}
MyToolButton* initToolButton(WidgetConf* wcf) {
    MyToolButton* w=myToolButton(wcf);
    return w;
}
MyRadioButton* initRadio(WidgetConf* wcf) {
    MyRadioButton* w=myRadio(wcf);
    return w;
}
MyCheckBox* initCheck(WidgetConf* wcf) {
    MyCheckBox* w=myCheck(wcf);
    return w;
}
MySpinBox* initSpin(WidgetConf* wcf) {
    MySpinBox* w=mySpin(wcf);
    return w;
}
MyLineEdit* initInput(WidgetConf* wcf) {
    MyLineEdit* w=myInput(wcf);
    return w;
}
MyPlainTextEdit* initTextArea(WidgetConf* wcf) {
    MyPlainTextEdit* w=myTextArea(wcf);
    return w;
}
MyDateEdit* initDate(WidgetConf* wcf) {
    MyDateEdit* w=myDate(wcf);
    return w;
}
MyTimeEdit* initTime(WidgetConf* wcf) {
    MyTimeEdit* w=myTime(wcf);
    return w;
}
MyCalendarWidget* initCalendar(WidgetConf* wcf) {
    MyCalendarWidget* w=myCalendar(wcf);
    return w;
}
MyToolBar* initToolbar(WidgetConf* wcf) {
    MyToolBar* w=myToolbar(wcf);
    return w;
}
MyToolBox* initToolbox(WidgetConf* wcf) {
    MyToolBox* w=myToolbox(wcf);
    return w;
}

MyProgressBar* initProgress(WidgetConf* wcf) {
    MyProgressBar* w=myProgress(wcf);
    w->setRange( WCFNUM("range-start",0), WCFNUM("range-end",100) );
    w->setValue( WCFNUM("value",0) );
    return w;
}
MyAction* initAction(WidgetConf* wcf) {
    MyAction* w=myAction(wcf);
    return w;
}
MyMenu* initMenu(WidgetConf* wcf) {
    MyMenu* w=myMenu(wcf);
    return w;
}
MySystemTrayIcon* initTray(WidgetConf* wcf) {
    MySystemTrayIcon* w=myTray(wcf);
    return w;
}



#endif
