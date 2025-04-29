#include "func_util.h"
#include "../util/user_event.h"
#include "../webserver/ConnectNode.h"
#include "../webserver/HttpNode.h"
#include <QApplication>
#include <QSound>
#include <QProcess>
#include <QScreen>
#include <QCryptographicHash>
#include <QColorDialog>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QScrollBar>
#include <QFileDialog>

#ifdef WEBTOOL_USE
#include <QHostInfo>
#include <QNetworkInterface>
#endif
#ifdef _WIN32
#include <mmsystem.h>
#include <shellapi.h>

#include "../fobject/FMutex.h"

FMutex gMutexSubfunc;

inline void W2UTF8(wchar_t* str, StrVar* rst) {
    if( str==NULL || rst==NULL ) return;
    int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
    std::string buf(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, -1, &buf[0], len,0,0);
    rst->add(buf.c_str());
}
#endif

QIcon gCommIcon;
QIcon& iconPixmap(QIcon& icon, QPixmap* img, LPCC mode, LPCC stat) {
    QIcon::Mode m=QIcon::Normal;
    QIcon::State s=QIcon::Off;
    if( mode ) {
        if( ccmp(mode,"disable") )
            m= QIcon::Disabled;
        else if( ccmp(mode,"select") )
            m= QIcon::Selected;
    }
    if( stat ) {
        if( ccmp(stat,"on") ) s=QIcon::On;
    }
    icon.addPixmap(*img, m, s);
    return icon;
}

void regArrayFuncs() {
    if( isHashKey("array") )
        return;
    PHashKey hash = getHashKey("array");
    U16 uid = 1;
    hash->add("get", uid);			uid++;
    hash->add("add", uid);			uid++;
    hash->add("insert", uid);		uid++;
    hash->add("index", uid);			uid++;
    hash->add("delete", uid);		uid++;
    hash->add("set", uid);			uid++;
    hash->add("pop", uid);			uid++;
    hash->add("reuse", uid);			uid++;
    hash->add("dist", uid);			uid++;
    uid = 20;
    hash->add("size", uid);			uid++;
    hash->add("remove", uid);		uid++;
    hash->add("join", uid);			uid++;
    hash->add("sum", uid);			uid++;
    hash->add("avg", uid);			uid++;
    hash->add("sort", uid);			uid++;
    hash->add("recalc", uid);		uid++;
    hash->add("find", uid);			uid++;
	hash->add("finds", uid);		uid++;
	hash->add("findPair", uid);		uid++;
	hash->add("findPairValue", uid);uid++;
    uid = 40;
	hash->add("removeAll", uid);	uid++;
    hash->add("copy", uid);			uid++;
    hash->add("reverse", uid);		uid++;
    hash->add("decode", uid);		uid++;
    hash->add("addPair", uid);		uid++;
    hash->add("div", uid);          uid++; //
    hash->add("toString", uid);		uid++;
    hash->add("addU16", uid);		uid++;
    hash->add("addHex", uid);		uid++;
    hash->add("toHex", uid);		uid++;  // 49
	hash->add("useArray", uid);		uid++;  // 50
	hash->add("swap", uid);			uid++;  // 51
	hash->add("filter", uid);		uid++;  // 52
	hash->add("map", uid);			uid++;  // 53
	uid=991;
    hash->add("with", uid);			uid++;
    hash->add("inject", uid);		uid++;
    hash->add("put", uid);			uid++;
}

void regSystemFuncs() {
    if( isHashKey("system") )
        return;
    PHashKey hash = getHashKey("system");
    U16 uid = 1;
    hash->add("tick", uid);				uid++;		// 1
    hash->add("path", uid);		uid++;	// 2
    hash->add("networkInfo", uid);		uid++;	//
    hash->add("readFile", uid);			uid++;	//
    hash->add("saveFile", uid);			uid++;	//
    hash->add("config", uid);				uid++;	//
    hash->add("localtime", uid);		uid++;	//
    hash->add("proc", uid);				uid++;	//
    hash->add("saveAllExt", uid);		uid++;	//
    hash->add("driveList", uid);		uid++;	// 10
    hash->add("trashList", uid);		uid++;	//
    hash->add("filePath", uid);			uid++;	//
    hash->add("lock", uid);				uid++;	//
    hash->add("unlock", uid);			uid++;	//
    hash->add("execApp", uid);			uid++;	//
    hash->add("screenCapture", uid);	uid++;	//
    uid = 20;
    hash->add("funcList", uid);			uid++;	//
    hash->add("tickCheck", uid);		uid++;	//
    hash->add("openExplore", uid);		uid++;	//
    hash->add("rand", uid);				uid++;	//
    hash->add("isRun", uid);			uid++;	//
    hash->add("run", uid);				uid++;	//
    hash->add("memoryStatus", uid);		uid++;	//
    hash->add("info", uid);				uid++;	//
    uid = 31;
    hash->add("watcherFolder", uid);	uid++;	//
    hash->add("inode", uid);			uid++;	//
    hash->add("date", uid);				uid++;	//
    hash->add("isConnect", uid);		uid++;	//
    hash->add("beep", uid);				uid++;	//
    hash->add("processCheck", uid);		uid++;	//
    hash->add("sleep", uid);			uid++;	//
    hash->add("wait", uid);				uid++;	//
    hash->add("copyText", uid);			uid++;	//
    hash->add("copyClipboard", uid);			uid++;	// 40
    hash->add("playWave", uid);			uid++;	//
    hash->add("processInfo", uid);		uid++;	//
    hash->add("kill", uid);				uid++;	//
    hash->add("worker", uid);			uid++;	//
    hash->add("reboot", uid);			uid++;	//
    hash->add("shutdown", uid);			uid++;	//
    hash->add("shortcut", uid);			uid++;	//
    hash->add("activatePage", uid);		uid++;	//
    hash->add("version", uid);			uid++;	//
	hash->add("cursorPos", uid);		uid++;	// 50
    hash->add("keyEventProc", uid);		uid++;	//
    hash->add("desktopCapture", uid);	uid++;	//
    hash->add("globalTimer", uid);	uid++;	//
    hash->add("clipboard", uid);		uid++;	//
    hash->add("watcherFile", uid);		uid++;	//
	hash->add("watcherClipboard", uid);		uid++;	//
}
void regMathFuncs() {
    if( isHashKey("math") )
        return;
    PHashKey hash = getHashKey("math");
    U16 uid = 1;
    hash->add("random", uid);		uid++;	// 1
    hash->add("sin", uid);			uid++;	// 2
    hash->add("cos", uid);			uid++;	// 3
    hash->add("pi", uid);			uid++;	// 4
    hash->add("tan", uid);			uid++;	// 5
    hash->add("atan", uid);			uid++;	// 6
    hash->add("exp", uid);			uid++;	// 7
    hash->add("log", uid);			uid++;	// 8
	hash->add("acos", uid);			uid++;	// 9
	hash->add("asin", uid);			uid++;	// 10
	uid = 21;
    hash->add("abs", uid);			uid++;	// 21
    hash->add("mod", uid);			uid++;	// 22
    hash->add("pow", uid);			uid++;	// 23
    hash->add("sqrt", uid);			uid++;	// 24
    hash->add("round", uid);		uid++;	// 25
    hash->add("floor", uid);		uid++;	// 26
    hash->add("ceil", uid);			uid++;	// 27
    hash->add("distance", uid);			uid++;	// 28
}
void regWasFuncs() {
    if( isHashKey("was") )
        return;
    PHashKey hash = getHashKey("was");
    U16 uid = 1;
    hash->add("addPage", uid);		uid++;	// 1
    hash->add("template", uid);		uid++;	// 2
    hash->add("tpl", uid);			uid++;	// 3
    hash->add("conf", uid);			uid++;	// 4
    hash->add("templateVars", uid);	uid++;	// 5
    hash->add("addFunc", uid);		uid++;	// 6
    hash->add("call", uid);			uid++;	// 7
}

void regConfigFuncs() {
    if( isHashKey("cf") )
        return;
    PHashKey hash = getHashKey("cf");
    U16 uid = 1;
    uid++;	// 1 hash->add("exec", uid);
    hash->add("get", uid);			uid++;	// 2
    hash->add("set", uid);			uid++;	// 3
    hash->add("define", uid);		uid++;	// 4
    hash->add("convert", uid);		uid++;	// 5
    hash->add("printer", uid);		uid++;	// 6
    hash->add("include", uid);      uid++;	// 7
    hash->add("var", uid);          uid++;
    hash->add("createWidget", uid);
    uid = 10;
    hash->add("objectList", uid);   uid++;	// 10
    hash->add("log", uid);			uid++;	// 11
    hash->add("env", uid);			uid++;	// 12
    hash->add("newNode", uid);
    uid = 20;
    hash->add("newArray", uid);		uid++;	// 20
    hash->add("destoryNewObjects", uid);  uid++;	//
    hash->add("call", uid);			uid++;	//
    hash->add("postEvent", uid);	uid++;	// hash->add("cms", uid);
    hash->add("map", uid);			uid++;	//
    hash->add("sourceApply", uid);	uid++;	// ;
    hash->add("sourceUpdate", uid);	uid++;	//
    hash->add("sourceCanvas", uid);	uid++;	// hash->add("widget", uid);
    hash->add("debug", uid);			uid++;	//
    hash->add("sourceCall", uid);	uid++;	//
    hash->add("idle", uid);			uid++;	//
    hash->add("loadPage", uid);		uid++;	//
    hash->add("clipText", uid);		uid++;	//
    hash->add("clipImage", uid);		uid++;	//
    hash->add("loadImage", uid);		uid++;	//
    hash->add("loadSvg", uid);
    uid = 41;
    hash->add("exit", uid);			uid++;	//
    hash->add("selectFile", uid);	uid++;	//
    hash->add("selectFolder", uid);
    uid = 51;
    hash->add("readSetting", uid);	uid++;	//
    hash->add("writeSetting", uid);	uid++;	//
    hash->add("handshakeKey", uid);	uid++;	// hash->add("setClass", uid);
    hash->add("info", uid);			uid++;	//
    hash->add("getObject", uid);		uid++;	//
    hash->add("recvData", uid);      uid++;	//
    uid++;	// hash->add("class", uid);
    hash->add("funcNode", uid);		uid++;	//
    hash->add("funcParam", uid);	uid++;	//
    hash->add("pageNode", uid);     uid++;	//
    hash->add("timeLine", uid);		uid++;	//
    hash->add("automata", uid);		uid++;	//
    hash->add("encode", uid);	uid++;	//
    hash->add("addFunc", uid);		uid++;	//
    hash->add("addEvent", uid);		uid++;	//
    hash->add("direction", uid);		uid++;	//
    hash->add("imageLoad", uid);		uid++;	//
    hash->add("jsValue", uid);	uid++;	//
    hash->add("val", uid);	uid++;	//
    hash->add("error", uid);	uid++;	//
    hash->add("confInfo", uid);	uid++;	//
    hash->add("classCheck", uid);	uid++;	//
    hash->add("toData", uid);	uid++;	//
    hash->add("toBinary", uid);	uid++;	//
    hash->add("node", uid);	uid++;	//
    hash->add("array", uid);	uid++;	//
    hash->add("rootNode", uid);	uid++;	//
    hash->add("ref", uid);      uid++;	//
    hash->add("getColor", uid);      uid++;	//
    hash->add("setEvent", uid);      uid++;	//
    hash->add("setFunc", uid);      uid++;	//
    hash->add("format", uid);      uid++;	//
}

//#> exec

#ifdef WIDGET_USE
inline bool callActionFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GAction* act) {
    StrVar* sv=NULL;
    if( act==NULL ) {
        return false;
    }
    int cnt=arrs ? arrs->size():0;
    if( ccmp(fnm,"enable") ) {
        act->setEnabled( cnt==0? true: isVarTrue(arrs->get(0)));
    } else if( ccmp(fnm,"disable") ) {
        act->setDisabled( cnt==0? true: isVarTrue(arrs->get(0)));
    } else if( ccmp(fnm,"shortcut") ) {
        if( cnt==0 ) {
            QKeySequence ks=act->shortcut();
            QString name=ks.toString();
            rst->set(Q2A(name));
        } else {
            LPCC shortcut=AS(0);
            if( slen(shortcut) ) {
                int num=0; // toKeySequenceKey(KR(shortcut));
                if( num ) {
                    act->setShortcut(QKeySequence(num));
                }
            }
        }
        /*
        QList<QKeySequence> list;
        for( int n=0;n<cnt; n++ ) {
            LPCC keys= toString(arrs->get(0));
            list.append(QKeySequence(KR(keys)));
        }
        act->setShortcuts(list);
        */
    } else if( ccmp(fnm,"checked") ) {
        act->setChecked( cnt==0? true: isVarTrue(arrs->get(0)));
    } else if( ccmp(fnm,"is") ) {
        if( cnt==0 ) {
            rst->setVar('3', act->isEnabled()? false: true);
        } else {
            LPCC ty = AS(0);
            if( ccmp(ty,"disable") ) rst->setVar('3', act->isEnabled()? false: true);
            else if( ccmp(ty,"enable") ) rst->setVar('3', act->isEnabled()? true: false);
            else if( ccmp(ty,"checked") ) rst->setVar('3', act->isChecked() ? true: false);
            else if( ccmp(ty,"checkable") ) rst->setVar('3', act->isCheckable() ? true: false);
        }
    } else if( ccmp(fnm,"trigger") ) {
        if( cnt==0 ) {
            act->trigger();
            return true;
        }
        TreeNode* thisNode=NULL;
        sv = arrs->get(0);
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(1);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc = (XFuncSrc*)SVO;
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn, act->config(), NULL, tn->GetVar("@trigger"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
        }
    } else if( ccmp(fnm,"text") ) {
        if( cnt==0 )
            rst->set(Q2A(act->text()));
        else
            act->setText(KR(AS(0)));
    } else if( ccmp(fnm,"tooltip") ) {
        if( cnt==0 )
            rst->set(Q2A(act->toolTip()));
        else
            act->setToolTip(KR(AS(0)));
    } else if( ccmp(fnm,"status") ) {
        if( cnt==0 )
            rst->set(Q2A(act->statusTip()));
        else
            act->setStatusTip(KR(AS(0)));
    } else if( ccmp(fnm,"icon") ) {
        if( cnt==0 ) return false;
        QPixmap* img = getStrVarPixmap(arrs->get(0), true);
        if( img ) {
            act->setIcon(iconPixmap(gCommIcon,img,AS(1),AS(2)));
            act->setIconVisibleInMenu(true);
        }
    } else {
        return false;
    }
    return true;
}
#endif

bool execObjectFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst, StrVar* origin, LPCC funcName ) {
    char ch=0;
    if( var->charAt(0)=='\b' ) {
        if( fc->xfkind==0  ) {
            fc->xfkind = (U16)var->charAt(1);
        }
        ch=var->charAt(1);
    }
    bool rtn = false;
    int cnt= arrs ? arrs->size(): 0;
    switch( ch ) {
    case 'n' : {
        TreeNode* cur=(TreeNode*)var->getObject(FUNC_HEADER_POS);
        rst->reuse();
        if( fc->xdotPos>0 || fc->xfuncRef==0 ) {
            LPCC fnm=funcName ? funcName : fc->getFuncName();
            rtn = callNodeFunc(cur, fnm, arrs, fn, rst, var,fc);
        } else {
            rtn = callNodeFunc(cur, NULL, arrs,fn, rst,var,fc);
        }
    } break;
    case 's' : {
        U16 stat=var->getU16(2);
        if(stat==0 || stat==1 ) {
            if( fc->xfuncRef==0 ) {
                regStrFuncs();
                fc->xfuncRef = getHashKeyVal("str", fc->getFuncName() );
            }
            rtn = execStrFunc(fc,arrs,fn,var,rst,origin);
        } else if( stat==11 ) {
            // 's',11 => QString
            QString* str=(QString*)var->getObject(FUNC_HEADER_POS);
            rtn=true;
            if( fc->xfuncRef==0 ) {
                LPCC fnm=funcName? funcName : fc->getFuncName();
                fc->xfuncRef =
                    ccmp(fnm,"indexOf") ? 1 :
                    ccmp(fnm,"replace") ? 2 :
                    ccmp(fnm,"substr") ? 3 :
                    0;
            }
            switch( fc->xfuncRef ) {
            case 1: {	// indexOf
                if( cnt==0 ) return true;
                LPCC find=AS(0);
                rst->setVar('0',0).addInt(-1);
                if( slen(find) ) {
                    int pos=0;
                    if( isNumberVar(arrs->get(1)) ) {
                        pos=toInteger(arrs->get(1));
                    }
                    StrVar* sv=arrs->get(1);
                    if( SVCHK('3',0) ) {
                        pos=str->indexOf(KR(find), pos, Qt::CaseInsensitive );
                    } else {
                        LPCC sty=toString(sv);
                        if( ccmpl(sty,3,"exp") ) {
                            pos=str->indexOf(QRegExp(find), pos );
                        } else {
                            pos=str->indexOf(KR(find), pos );
                        }
                    }
                    rst->setVar('0',0).addInt(pos);
                }
            } break;
            case 2: {	// replace
                if( cnt==0 ) return true;
                LPCC find=AS(0);
                LPCC rep=AS(1);
                if( slen(find) ) {
                    StrVar* sv=arrs->get(2);
                    if( SVCHK('3',0) ) {
                        str->replace(KR(find), KR(rep), Qt::CaseInsensitive);
                    } else {
                        LPCC sty=toString(sv);
                        if( ccmpl(sty,3,"exp") ) {
                            str->replace(QRegExp(KR(find)), KR(rep) );
                        } else {
                            str->replace(KR(find), KR(rep));
                        }
                    }
                }
            } break;
            case 3: {	// substr
                if( cnt==0 ) return true;
                int sp=0, ep=-1;
                if( isNumberVar(arrs->get(0)) ) {
                    sp=toInteger(arrs->get(0));
                    if( isNumberVar(arrs->get(1)) ) {
                        ep=toInteger(arrs->get(1));
                        if( ep>str->length() ) {
                            ep=str->length();
                        }
                    }
                }
                QString sub=str->mid(sp, ep);
                rst->set(Q2A(sub));
            } break;
            default: break;
            }
        } else if( stat==12 ) {
            // 's',12
            QRegExp* exp=(QRegExp*)var->getObject(FUNC_HEADER_POS);
            rtn=true;
            if( fc->xfuncRef==0 ) {
                LPCC fnm=funcName? funcName : fc->getFuncName();
                fc->xfuncRef =
                    ccmp(fnm,"indexOf") ? 1 :
                    ccmp(fnm,"indexIn") ? 2 :
                    ccmp(fnm,"match") ? 3 :
                    ccmp(fnm,"length") ? 4 :
                    ccmp(fnm,"cap") ? 5 :
                    ccmp(fnm,"caseSensitive") ? 6 :
                    0;
            }
            StrVar* sv=NULL;
            switch( fc->xfuncRef ) {
            case 1:
            case 2: {	// indexIn
                if( cnt==0 ) return true;
                rst->setVar('0',0).addInt(-1);
                sv=fn->gv("@str");
                if( SVCHK('s',11) ) {
                    QString* str=(QString*)SVO;
                    int pos=0;
                    if( isNumberVar(arrs->get(0)) ) {
                        pos=toInteger(arrs->get(0));
                    }
                    pos=exp->indexIn(*str,pos);
                    rst->setVar('0',0).addInt(pos);
                }
            } break;
            case 3: {	// match
                sv=fn->gv("@str");
                rst->reuse();
                if( SVCHK('s',11) ) {
                    QString* str=(QString*)SVO;
                    if( exp->exactMatch(*str) ) {
                        rst->setVar('3',1);
                    } else {
                        rst->setVar('3',0);
                    }
                }
            } break;
            case 4: {	// length
                rst->setVar('0',0).addInt( exp->matchedLength() );
            } break;
            case 5: {	// cap
                int pos=arrs && isNumberVar(arrs->get(0)) ? toInteger(arrs->get(0)) : 1;
                if( pos<0 ) pos=1;
                QString str=exp->cap(pos);
                rst->set(Q2A(str));
            } break;
            case 6: {	// caseSensitive
                bool chk=arrs && !isVarTrue(arrs->get(0)) ? false: true;
                if( chk ) exp->setCaseSensitivity(Qt::CaseInsensitive);
                else exp->setCaseSensitivity(Qt::CaseInsensitive);
            } break;
            default: {
              rtn=false;
            } break;
            }
        }
    } break;
    case 'i' : {
        if( fc->xfuncRef==0 ) {
            regRangeFuncs();
            fc->xfuncRef = getHashKeyVal("range", funcName? funcName : fc->getFuncName() );
        }
        rtn = execRangeFunc(fc,arrs,fn,var,rst);
    } break;
    case 'd' : {
        U16 stat=var->getU16(2);
        switch(stat) {
        case 12: { // path
        } break;
        case 13: { // region
            /*
            QRegion* reg=(QRegion*)var->getObject(FUNC_HEADER_POS);
            LPCC fnm=funcName? funcName : fc->getFuncName();
            if( fc->xfuncRef ) {
                fc->xfuncRef=ccmp(fnm,"unite")? 1: ccmp(fnm,"intersect")? 2: ccmp(fnm,"subtract")? 3:
                      ccmp(fnm,"or")? 4: ccmp(fnm,"contains")? 5: ccmp(fnm,"bound")? 6: 99;
            }
            StrVar* sv=NULL;
            switch( fc->xfuncRef ) {
            case 1: {   // unite
                if( cnt==0 ) return true;
                sv=arrs->get(0);
                if( SVCHK('d',13)) {
                    QRegion* regOther=(QRegion*)SVO;
                    reg->unite(*regOther);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    QRect rc=setQRect(sv);
                    reg->unite(rc);
                }
            } break;
            case 2: {   // intersect
                if( cnt==0 ) return true;
                sv=arrs->get(0);
                if( SVCHK('d',13)) {
                    QRegion* regOther=(QRegion*)SVO;
                    reg->intersect(*regOther);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    QRect rc=setQRect(sv);
                    reg->intersect(rc);
                }
            } break;
            case 3: {   // subtract
                if( cnt==0 ) return true;
                sv=arrs->get(0);
                if( SVCHK('d',13)) {
                    QRegion* regOther=(QRegion*)SVO;
                    reg->subtract(*regOther);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    QRect rc=setQRect(sv);
                    reg->subtract(rc);
                }
            } break;
            case 4: {   // or
                if( cnt==0 ) return true;
                sv=arrs->get(0);
                if( SVCHK('d',13)) {
                    QRegion* regOther=(QRegion*)SVO;
                    reg->eor(*regOther);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    QRect rc=setQRect(sv);
                    reg->eor(rc);
                }
            } break;
            case 5: {   // contains
                if( cnt==0 ) return true;
                sv=arrs->get(0);
                if( isNumberVar(sv) ) {
                    int x=toInteger(sv), y=toInteger(arrs->get(1));
                    rst->setVar('3',reg->contains(QPoint(x,y))?1:0);
                } else if( SVCHK('i',2) ) {
                    int x=sv->getInt(4), y=sv->getInt(8);
                    rst->setVar('3',reg->contains(QPoint(x,y))?1:0);
                } else if( SVCHK('i',20) ) {
                    int x=sv->getDouble(4), y=sv->getDouble(4+sizeof(double));
                    rst->setVar('3',reg->contains(QPoint(x,y))?1:0);
                } else if( SVCHK('i',5) || SVCHK('i',4) ) {
                    QRect rc=setQRect(sv);
                    rst->setVar('3',reg->contains(rc)?1:0);
                }
            } break;
            case 6: {   // bound
                QRect rc=reg->boundingRect();
                setVarRectF(rst, rc);
            } break;
            default: return true;
            }
            */
        } break;
        default: break;
        }
    } break;
    case 'a' : {
        U16 stat=var->getU16(2);
        switch(stat) {
        case 0: { // array
            if( fc->xfuncRef==0 ) {
                regArrayFuncs();
                fc->xfuncRef = getHashKeyVal("array", funcName? funcName : fc->getFuncName() );
            }
            rtn=execArrayFunc(fc,arrs,fn,var,rst);
        } break;
        case 11: { // automata
            rtn=callAutomataFunc(fc,arrs,fn,rst,var);
        } break;
        default: break;
        }
    } break;
    case 'f' : {
        LPCC fnm=funcName? funcName : fc->getFuncName();
        U16 stat=var->getU16(2);
        if( stat==0 ) {
            XFuncNode* fnCur=(XFuncNode*)var->getObject(FUNC_HEADER_POS);
            StrVar* sv=NULL;
            rst->reuse();
            int pos=IndexOf(fnm,'.');
            if( pos>0 ) {
                fnm+=pos+1;
            }
            rtn=true;
            if( ccmp(fnm,"isset") ) {
                LPCC code=arrs ? AS(0): NULL;
                if( slen(code) ) {
                    sv=fnCur->gv(code);
                    if( sv ) {
                        if( SVCHK('9',0) ) {
                            rst->setVar('3',0);
                        } else {
                            rst->setVar('3',1);
                        }
                    } else {
                        rst->setVar('3',0);
                    }
                } else {
                    rst->setVar('3',0);
                }
            } else if( ccmp(fnm,"funcSrc") ) {
                XFuncSrc* fsrc=fnCur->xfunc->getFuncSrc();
                if(fsrc==NULL ) {
                    fsrc=&(fnCur->xsrc);
                }
                rst->setVar('f',1,(LPVOID)fsrc);
            } else if( ccmp(fnm,"append") ) {
                if( cnt==0 ) {
                    return true;
                }
                StrVar* val=NULL;
                sv=fnCur->gv("@appendVar");
                if(sv) {
                    int sp=0, ep=0;
                    val=getStrVarPointer(sv,&sp,&ep);
                }
                for(int n=0,sz=arrs->size(); n<sz; n++ ) {
                    if( n==0 && val==NULL ) {
                        LPCC code=toString(arrs->get(n));
                        if( slen(code) ) {
                            val=fnCur->GetVar(code);
                        }
                    } else {
                        if( val ) {
                            int sp=0, ep=0;
                            sv=getStrVarPointer(arrs->get(n), &sp, &ep);
                            if( sp<ep ) {
                                val->add(sv, sp, ep);
                            }
                        } else {
                            qDebug("#9#funcNode append error (append var not define) \n");
                            break;
                        }
                    }
                }
                rst->setVar('3', val? 1: 0 );
            } else if( ccmp(fnm,"eventFuncList") ) {
                rst->reuse();
                sv=fnCur->gv("@eventFuncList");
                if(sv) {
                    rst->add(sv);
                }
            } else if( ccmp(fnm,"addFuncSrc") ) {
                if( cnt==0 ) return true;
                XFuncSrc* fsrc=NULL;
                sv=arrs->get(0);
                if(SVCHK('f',0)) {
                    XFuncNode* fnDest=(XFuncNode*)SVO;
                    fsrc=fnDest->getFuncSrc();
                } else if(SVCHK('f',1)) {
                    fsrc=(XFuncSrc*)SVO;
                }

                if( fnCur->isNodeFlag(FLAG_PERSIST) ) {
                    XListArr* list=NULL;
                    sv=fnCur->gv("@eventFuncList");
                    if(SVCHK('a',0)) {
                        list=(XListArr*)SVO;
                    } else {
                        list=new XListArr(true);
                        fnCur->GetVar("@eventFuncList")->setVar('a',0,(LPVOID)list);
                    }
                    if(fsrc) {
                        int size=list->size(), idx=-1;
                        for(int n=0; n<size; n++) {
                            sv=list->get(n);
                            if(SVCHK('f',1)) {
                                XFuncSrc* fsrcCur=(XFuncSrc*)SVO;
                                if(fsrcCur==fsrc) {
                                    idx=n;
                                    break;
                                }
                            }
                        }
                        if(idx==-1) {
                            list->add()->setVar('f',1,(LPVOID)fsrc);
                        } else {
                            qDebug("addFuncSrc error [already add function]");
                        }
                    }
                    rst->setVar('a',0,(LPVOID)list);
                } else {
                   qDebug("addFuncSrc error [function list not defined]");
                }
            } else if( ccmp(fnm,"callFuncParams") ) {
                XListArr* parr = NULL;
                sv=fnCur->gv("@params");
                if(SVCHK('a',0)) {
                    parr = (XListArr*)SVO;
                } else {
                    parr = new XListArr(true);
                    fnCur->GetVar("@params")->setVar('a',0,(LPVOID)parr);
                }
                parr->reuse();
                if( arrs ) {
                    for(int n=0; n<arrs->size(); n++) {
                        parr->add()->add(arrs->get(n));
                    }
                }
                rst->setVar('a',0,(LPVOID)parr);
            } else if( ccmp(fnm,"callFuncSrc") ) {
                sv=fnCur->gv("@eventFuncList");
                // qDebug("@@ callFuncSrc start");
                if( SVCHK('a',0) ) {
                    XListArr* list=(XListArr*)SVO;
                    XFuncSrc* fsrcCur = NULL;
                    TreeNode* prevThisNode = NULL;
                    TreeNode* curThisNode = NULL;
                    int size=list->size();
                    sv=fnCur->gv("@this");
                    if(SVCHK('n',0)) {
                        prevThisNode = (TreeNode*)SVO;
                    }
                    sv=arrs!=NULL ? arrs->get(0):NULL;
                    if(SVCHK('f',1) ) {
                        fsrcCur=(XFuncSrc*)SVO;
                        sv=arrs->get(1);
                    }
                    if(SVCHK('n',0)) {
                        curThisNode = (TreeNode*)SVO;
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)curThisNode);
                    }
                    // qDebug("@@ callFuncSrc size:%d",size);
                    for(int n=0; n<size; n++) {
                        sv=list->get(n);
                        if(SVCHK('f',1)) {
                            XFuncSrc* fsrc=(XFuncSrc*)SVO;
                            if(fsrcCur && fsrcCur!=fsrc ) {
                                continue;
                            }
                            XListArr* params=NULL;
                            sv=fnCur->gv("@params");
                            if(SVCHK('a',0)) {
                                params=(XListArr*)SVO;
                                XParseVar pv(&(fsrc->xparam));
                                for(int p=0; pv.valid() && p<256; p++ ) {
                                    LPCC code = pv.findEnd(",").v();
                                    if( slen(code)==0 ) break;
                                    fnCur->GetVar(code)->reuse()->add(params->get(p));
                                }
                            }
                            if( fnCur->isNodeFlag(FLAG_CALL) ) {
                                // qDebug("==> funcNode callFuncSrc already called");
                                fnCur->clearNodeFlag(FLAG_CALL);
                            }

                            fnCur->call(fsrc->xfunc, rst->reuse());
                            if( checkFuncObject(rst,'3',1) ||
                                ccmp("ignore",rst->get()) ) {
                                break;
                            }
                        } else {
                            qDebug("@@ callFuncSrc n==%d not function", n);
                        }
                    }
                    if( prevThisNode && curThisNode ) {
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)prevThisNode);
                    }
                } else {
                    qDebug("callFuncSrc error [function list not defined]");
                }
            } else if( ccmp(fnm,"removeFuncSrc") ) {
                if( cnt==0 ) return true;
                XFuncSrc* fsrc=NULL;
                sv=arrs->get(0);
                if(SVCHK('f',0)) {
                    XFuncNode* fnDest=(XFuncNode*)SVO;
                    if( fnDest->isNodeFlag(FLAG_PERSIST)) {
                        fsrc=fnDest->getFuncSrc();
                    }
                } else if(SVCHK('f',1)) {
                    fsrc=(XFuncSrc*)SVO;
                }
                sv=fnCur->gv("@eventFuncList");
                if(fsrc && SVCHK('a',0) ) {
                    XListArr* list=(XListArr*)SVO;
                    int idx=-1, size=list->size();
                    for(int n=0; n<size; n++) {
                        sv=list->get(n);
                        if(SVCHK('f',1)) {
                            XFuncSrc* fsrcCur=(XFuncSrc*)SVO;
                            if(fsrc==fsrcCur) {
                                idx=n;
                                break;
                            }
                        }
                    }
                    if(idx!=-1 ) {
                        list->remove(idx);
                    }
                } else {
                    qDebug("removeFuncSrc error [function list not defined]");
                }
            } else if( ccmp(fnm,"flags") ) {
                rst->setVar('1',0).addUL64((UL64)fnCur->xflag);
            } else if( ccmp(fnm,"setPersist") ) {
                if(!fnCur->isNodeFlag(FLAG_PERSIST) ) {
                    fnCur->setNodeFlag(FLAG_PERSIST);
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            } else if( ccmp(fnm,"setEvent") ) {
                 NULL;
                bool checkSet = true;
                sv= arrs ? arrs->get(0): NULL;
                if(sv==NULL ) {
                    sv=fnCur->gv("@this");
                } else if( SVCHK('n',0) ) {
                    if(cnt==2 ) {
                        checkSet = isVarTrue(arrs->get(1));
                    }
                } else {
                    checkSet = isVarTrue(sv);
                    sv=fnCur->gv("@this");
                }
                if( SVCHK('n',0)) {
                    TreeNode* thisNode = (TreeNode*)SVO;
                    if( checkSet ) {
                        thisNode->setNodeFlag(FLAG_SETEVENT);
                    } else {
                        thisNode->clearNodeFlag(FLAG_SETEVENT);
                    }
                }
            } else if( ccmp(fnm,"isPersist") ) {
                rst->setVar('3', fnCur->isNodeFlag(FLAG_PERSIST)?1:0 );
            } else if( ccmp(fnm,"get") ) {
                rst->reuse();
                if( cnt==0 ) {
                    XParseVar pv;
                    int n=0;
                    for( WBoxNode* bn=fnCur->First(); bn; bn=fnCur->Next() ) {
                        LPCC code=fnCur->getCurCode();
                        if( ccmp(code,"@this") || ccmp(code,"@page") ) continue;
                        if( n++ ) rst->add(",\n");
                        rst->add(code).add("=");
                        sv=fnCur->gv(code);
                        if(sv==NULL) {
                        } else if( isNumberVar(sv) ) {
                            toString(sv, rst);
                        } else if( sv->charAt(0)=='\b' ) {
                            getObjectType(sv,rst);
                        } else {
                            pv.SetVar(sv,0,sv->length());
                            if(pv.ch()) {
                                pv.findEnd("\n");
                                if(pv.valid() ) {
                                    int sp=pv.prev, ep=pv.cur;
                                    if(sp<ep) rst->add(sv, sp, ep);
                                } else {
                                    rst->add(sv);
                                }
                            } else {
                                if(pv.start>0) rst->add("(blank)"); else rst->add("(empty)");
                            }
                        }
                    }
                } else {
                    if( isVarTrue(arrs->get(1)) ) {
                        int sp=0, ep=0;
                        sv=getStrVarPointer(arrs->get(0), &sp, &ep);
                        if( sp<ep ) {
                            XParseVar pv(sv, sp, ep);
                            setResultVal(&pv, fnCur, rst );
                        }
                    } else {
                        LPCC code=AS(0);
                        sv=fnCur->gv(code);
                    }
                    rst->reuse()->add(sv);
                }
            } else if( ccmp(fnm,"set") ) {
                LPCC code=arrs ? AS(0): NULL;
                if( slen(code) ) {
                    // qDebug("---------- func node set var:%s\n", code);
                    StrVar* var=fnCur->GetVar(code);
                    int sp=0,ep=0;
                    sv=getStrVarPointer(arrs->get(1),&sp,&ep);
                    if(var) {
                        var->reuse();
                        if(sp<ep) var->set(sv,sp,ep);
                    }
                }
            } else if( ccmp(fnm,"delete") ) {
                if( fnCur->isNodeFlag(FLAG_PERSIST) ) {
                    fnCur->clearNodeFlag(FLAG_PERSIST);
                }
                funcNodeDelete(fnCur);
            } else if( ccmp(fnm,"inject") ) {
                for( int n=0;n<fc->getParamSize();n++) {
                    XFunc* fcParam=fc->getParam(n);
                    LPCC code = fcParam->getVarName();
                    bool bref=false;
                    if( code[0]=='&' ) {
                        bref=true;
                        code+=1;
                    }
                    LPCC name=NULL;
                    int pos=IndexOf(code,':');
                    if(pos>0) {
                        name=gBuf.add(code,pos);
                        code+=pos+1;
                    } else {
                        name=code;
                    }
                    sv=fnCur->gv(code);
                    // qDebug("code:%s %s", code, sv?sv->get():"undefined");
                    if( bref ) {
                        if( sv ) {
                            if( sv->charAt(0)=='\b' ) {
                                fn->GetVar(name)->reuse()->add(sv);
                            } else if( sv ) {
                                int sp=0, ep=sv->length();
                                fn->GetVar(name)->setVar('s',0, (LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                            } else {
                                fn->GetVar(name)->reuse();
                            }
                        } else {
                            fn->GetVar(name)->reuse();
                        }
                    } else {
                        if( SVCHK('s',0) ) {
                            StrVar* var=fn->GetVar(name)->reuse();
                            int sp=0,ep=0;
                            sv=getStrVarPointer(sv,&sp,&ep);
                            if( sp<ep ) var->add(var->get(sp), ep-sp );
                        } else {
                            fn->GetVar(name)->reuse()->add(sv);
                        }
                    }
                }
            } else if( ccmp(fnm,"parentFunc") ) {
                XFuncNode* fnParent=fnCur->xparent;
                if( fnParent ) rst->setVar('f', 0, (LPVOID)fnParent );
            } else {
                qDebug("#0#[%s] not valid (name:%s Please funcNode inline function check)\n", getBaseFuncName(fn), fnm);
                rtn=false;
            }
        } else if( stat==11 ) {
            // 'f',11
            rtn=callFileInfoFunc(fn, fc, var, arrs, rst);
        } else if( stat==12 ) {
            // 'f',12
        }
        return true;
    } break;
    case 'c' : {
#ifdef WIDGET_USE
        U16 stat=var->getU16(2);
        if( stat==1 ) {
            // 'c',1 CanvasItem
            rtn=callCanvasItemFunc(funcName, fn, fc, var, arrs, rst);

        } else if( stat==2 ) {
            // 'c',2 textcursor
            rtn=callTextCursorFunc(funcName, fn, fc, var, arrs, rst);
        } else if( stat==11 ) {
            // diskcache
        }
#endif
    } break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4' : {
        if( fc->xfuncRef==0 ) {
            LPCC fnm=funcName? funcName : fc->getFuncName();
            fc->xfuncRef = ccmp(fnm,"eq") ? 806: ccmp(fnm,"lt") ? 2:ccmp(fnm,"le") ? 3: ccmp(fnm,"gt") ? 4:ccmp(fnm,"ge") ? 5: ccmp(fnm,"ne") ? 807:
                ccmp(fnm,"date")? 7:
                ccmp(fnm,"round") ? 8 :
                ccmp(fnm,"shift") ? 9 :
                ccmp(fnm,"toInt") ? 802 :
                ccmp(fnm,"toDouble") ? 803 :
                ccmp(fnm,"toLong") ? 804 :
                ccmp(fnm,"toString") ? 805 :
                ccmp(fnm,"toHex") ? 810 :
				ccmp(fnm,"between") ? 820 :
                ccmp(fnm,"incr") ? 808 :0;
        }
        rtn=true;
        switch( fc->xfuncRef ) {
		case 820: {
            if( cnt==0 ) {
				return false;
			}
			bool bchk = false;
			if( ch=='4' ) {
				double a=toDouble(var);
				double sp=toDouble(arrs->get(0));
				double ep=toDouble(arrs->get(1));
				bchk=sp<=a && a<ep;
			} else if( ch=='0' ) {
				int a=toInteger(var);
				int sp=toInteger(arrs->get(0));
				int ep=toInteger(arrs->get(1));
				bchk=sp<=a && a<ep;
			} else {
				UL64 a=toUL64(var);
				UL64 sp=toUL64(arrs->get(0));
				UL64 ep=toUL64(arrs->get(1));
				bchk=sp<=a && a<ep;
			}
			rst->setVar('3',bchk?1:0);
		} break;
        case 806:	// eq, ne
        case 807: {
            if( cnt==0 ) {
                return false;
            }
            bool bchk = false;
            if( ch=='4' ) {
                double a=toDouble(var);
                for( int n=0,sz=arrs->size();n<sz;n++ ) {
                    if( a==toDouble(arrs->get(n)) ) {
                        bchk = true;
                        break;
                    }
                }
            } else if( ch=='3' ) {
                U16 stat=var->getU16(2);
                StrVar* sv=arrs->get(0);
                if( stat==1  ) {
                    if( SVCHK('3',1) ) bchk=true;
                } else {
                    if( SVCHK('3',0) ) bchk=true;
                }
            } else if( ch=='0' ) {
                int a=toInteger(var);
                for( int n=0,sz=arrs->size();n<sz;n++ ) {
                    if( a==toInteger(arrs->get(n)) ) {
                        bchk = true;
                        break;
                    }
                }
            } else {
                UL64 a=toUL64(var);
                for( int n=0,sz=arrs->size();n<sz;n++ ) {
                    if( a==toUL64(arrs->get(n)) ) {
                        bchk = true;
                        break;
                    }
                }
            }
            if( fc->xfuncRef==806 )
                rst->setVar('3',bchk?1:0);
            else
                rst->setVar('3',bchk?0:1);
        } break;
        case 2:
        case 3: {
            if( cnt==0 ) return false;
            if( ch=='4' ) {
                double a=toDouble(var);
                if( fc->xfuncRef==2 )
                    rst->setVar('3',a<toDouble(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a<=toDouble(arrs->get(0))?1:0 );
            } else if( ch=='0' || ch=='3' ) {
                int a=toInteger(var);
                if( fc->xfuncRef==2 )
                    rst->setVar('3',a<toInteger(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a<=toInteger(arrs->get(0))?1:0 );
            } else {
                UL64 a=toUL64(var);
                if( fc->xfuncRef==2 )
                    rst->setVar('3',a<toUL64(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a<=toUL64(arrs->get(0))?1:0 );
            }
        } break;
        case 4:
        case 5: {
            if( cnt==0 ) return false;
            if( ch=='4' ) {
                double a=toDouble(var);
                if( fc->xfuncRef==4 )
                    rst->setVar('3',a>toDouble(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a>=toDouble(arrs->get(0))?1:0 );
            } else if( ch=='0' || ch=='3' ) {
                int a=toInteger(var);
                if( fc->xfuncRef==4 )
                    rst->setVar('3',a>toInteger(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a>=toInteger(arrs->get(0))?1:0 );
            } else {
                UL64 a=toUL64(var);
                if( fc->xfuncRef==4 )
                    rst->setVar('3',a>toUL64(arrs->get(0))?1:0 );
                else
                    rst->setVar('3',a>=toUL64(arrs->get(0))?1:0 );
            }
        } break;
        case 7: {	// date
            UL64 dt=toUL64(var);
            if( dt>0 ) {

                rst->ftime("%Y%m%dT%H%M%S",localtime((const time_t*)&dt));
                LPCC fmt=arrs?AS(0):"yyyy-MM-dd hh:mm:ss";
                QString str = getDateTime(rst->get()).toString(KR(fmt));
                rst->reuse()->add(Q2A(str));
            }
        } break;
        case 8: {	// round
            int p=0;
            if( arrs ) {
                p=toInteger(arrs->get(0));
            }
            float a=(float)toDouble(var);
            double v=rounding(a, p);
            if( p==0 ) {
                rst->setVar('0',0).addInt((int)v);
            } else {
                rst->setVar('4',0).addDouble(v);
            }

        } break;
        case 9: {	// shift
            UL64 dt=toUL64(var), val=0;
            int p=0;
            if( arrs ) {
                p=toInteger(arrs->get(0));
            }
            if( p<0 ) {
				val=dt<<p;
            } else {
				p*=-1;
				val=dt>>p;
            }
            rst->setVar('1',0).addUL64(val);
        } break;
        case 802: { // toInt
            int num = toInteger(var);
            rst->setVar('0',0).addInt(num);
        } break;
        case 803: { // toDouble
            double num = toDouble(var);
            rst->setVar('4',0).addDouble(num);
        } break;
        case 804: { // toLong
            UL64 num = toUL64(var);
            rst->setVar('1',0).addUL64(num);
        } break;
        case 805: { // toString
            rst->set( toString(var) );
        } break;
        case 810: { // toHex
            U32 num = toInteger(var);
            rst->format(16,"0X%X",num);
        } break;
        case 808: { // incr
            StrVar* sv=arrs ? arrs->get(1): NULL;
            bool positive= SVCHK('3',0) ? false: true;
            if( ch=='0' ) {
                int num = toInteger(var), p=1;
                if( arrs ) {
                    p=toInteger(arrs->get(0));
                }
                rst->setVar('0',0).addInt(num);
                if(positive) num+=p;
                else num-=p;
                var->setVar('0',0).addInt(num);
            } else if( ch=='1') {
                qint64 num = (qint64)toUL64(var), p=1;
                if( arrs ) {
                    p=toUL64(arrs->get(0));
                }
                rst->setVar('1',0).addUL64(num);
                if(positive) num+=p;
                else num-=p;
                var->setVar('1',0).addUL64(num);
            } else if( ch=='4') {
                double num = var->getInt(4), p=1;
                if( arrs ) {
                    p=toDouble(arrs->get(0));
                }
                rst->setVar('4',0).addInt(num);
                if(positive) num+=p;
                else num-=p;
                var->setVar('4',0).addDouble(num);
            }
        } break;
        default: {
            rtn=false;
        } break;
        }
    } break;
    case '9': {
        if( fc->xfuncRef==0 ) {
            regStrFuncs();
            fc->xfuncRef = getHashKeyVal("str", funcName? funcName : fc->getFuncName() );
        }
        var->reuse();
        rtn=execStrFunc(fc,arrs,fn,var,rst,origin);
        fc->xfuncRef=0;
    } break;
    case 'w' : {
#ifdef WIDGET_USE
        U16 stat=var->getU16(2);
        if( stat==11 ) {
            // 'w',11
            GAction* action=(GAction*)var->getObject(FUNC_HEADER_POS);
            LPCC fnm=funcName? funcName : fc->getFuncName();
            rtn=callActionFunc(fnm, arrs, action->config(), fn, rst, action);

         } else if( stat==12 ) {
            // 'w',12
            QMenu* menu=(QMenu*)var->getObject(FUNC_HEADER_POS);
            LPCC fnm=funcName? funcName : fc->getFuncName();
            if( ccmp(fnm,"close") ) {
                menu->close();
            }
            rtn=true;
        } else if( stat==20) {
            QScrollArea* scroll=(QScrollArea*)var->getObject(FUNC_HEADER_POS);
            LPCC fnm=funcName? funcName : fc->getFuncName();
            if(ccmp(fnm,"widgetResizable") ) {
                if(arrs==NULL) {
                    return 0;
                }
                scroll->setWidgetResizable(isVarTrue(arrs->get(0)) );
            } else if(ccmp(fnm,"scrollPolicy") ) {
                if(arrs==NULL) {
                    return 0;
                    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                }
                LPCC ty=AS(0), val=AS(1);
                if(ccmp(ty,"hscroll") ) {
                    if(ccmp(val,"on")) {
                        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                    } else if(ccmp(val,"off")) {
                        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    } else {
                        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

                    }
                } else if(ccmp(ty,"vscroll") ) {
                    if(ccmp(val,"on")) {
                        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                    } else if(ccmp(val,"off")) {
                        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
                    } else {
                        scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                    }
                }
            } else if(ccmp(fnm,"position") ) {
                if(arrs==NULL) {
                    rst->setVar('0',0).addInt(scroll->verticalScrollBar()->maximum());
                    // scrollBar->maxi();
                    return 0;
                }
                LPCC ty=AS(0);
                if(ccmp(ty,"max") || ccmp(ty,"vmax") ) {
                     rst->setVar('0',0).addInt(scroll->verticalScrollBar()->maximum());
                } else if(ccmp(ty,"hmax") ) {
                    rst->setVar('0',0).addInt(scroll->horizontalScrollBar()->maximum());
                } else if(ccmp(ty,"move") || ccmp(ty,"vmove") ) {
                    int pos=toInteger(arrs->get(1));
                    scroll->verticalScrollBar()->setValue(pos);
                } else if(ccmp(ty,"hmove") ) {
                    int pos=toInteger(arrs->get(1));
                    scroll->horizontalScrollBar()->setValue(pos);
                }
            }
        } else if( stat==21) {
            // 'w',21
            GTray* tray =(GTray*)var->getObject(FUNC_HEADER_POS);
            LPCC fnm=funcName? funcName : fc->getFuncName();
            rtn=callTrayFunc(fnm,arrs,tray->config(),fn,rst,fc);
        }
#endif
    } break;
    case 'v' : {
        U16 stat=var->getU16(2);
        switch(stat) {
        case 1: { // ConnectNode
            ConnectNode* conn=(ConnectNode*)var->getObject(FUNC_HEADER_POS);
            TreeNode* tn=conn->config();
            if( tn==NULL ) {
                tn=new TreeNode(2, true);
                conn->xcf=tn;
            }
            StrVar* sv=tn->GetVar("@c");
            if( !SVCHK('v',1) ) {
                sv->setVar('v',1,(LPVOID)conn);
            }
            rtn=callConnectNodeFunc(fc, arrs, tn, fn, rst);
        } break;
        case 22: { // Message Client
            rtn=callMessageClientFunc(var, fc, funcName, arrs, NULL, fn, rst);
        } break;
        case 0: { // HttpServerThread
            LPCC fnm=funcName? funcName : fc->getFuncName();
            HttpServerThread* hst=(HttpServerThread*)var->getObject(FUNC_HEADER_POS);
            TreeNode* tn=hst->config();
            if( tn==NULL ) {
                tn=new TreeNode(2, true);
                hst->xcf=tn;
            }
            StrVar* sv=tn->GetVar("@c");
            if( !SVCHK('v',1) ) {
                sv->setVar('v',0,(LPVOID)hst);
            }
            rtn=callWasFunc(fnm, arrs, tn, fn, rst, fc );
        } break;
        default: break;
        }
        return true;
    } break;
    case 'x' : {
        U16 stat=var->getU16(2);
        if( stat==21 ) {
            // 'x',21 pair
            int pos=var->find("\f", FUNC_HEADER_POS);
            if( pos==-1 ) {
                return true;
            }
            if( fc->xfuncRef==0 ) {
                LPCC fnm=funcName? funcName : fc->getFuncName();
                fc->xfuncRef =
                    ccmp(fnm,"inject") ? 992 : ccmp(fnm,"key") ? 2 :
                    ccmp(fnm,"value") ? 3 : 0;
            }
            rtn=true;
            switch( fc->xfuncRef ) {
            case 992: {	// inject
                StrVar* sv=NULL;
                int cnt=fc->getParamSize();
                LPCC key=var->get(FUNC_HEADER_POS,pos++);
                StrVar* val=getStrVarTemp();
                if( pos < var->length() ) {
                    val->add(var, pos, var->length() );
                }
                for( int n=0;n<cnt;n++) {
                    XFunc* fcParam=fc->getParam(n);
                    if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
                        StrVar* textVar=cfVar(NULL);
                        takeParseVar(textVar,fcParam,fn);
                        XParseVar pv(textVar );
                        while( pv.valid() ) {
                            LPCC code=pv.NextWord();
                            if( slen(code)==0 ) break;
                            sv=fn->GetVar(code)->reuse();
                            if(n==0 ) {
                                sv->set(key);
                            } else {
                                sv->add(val);
                            }
                            if( pv.ch()==',' ) pv.incr(); else break;
                            n++;
                        }
                        break;
                    }
                    LPCC code = fcParam->getVarName();
                    sv=fn->GetVar(code)->reuse();
                    if(n==0 ) {
                        sv->set(key);
                    } else {
                        sv->add(val);
                    }
                }
            } break;
            case 2: {	// key
                rst->set(var->get(FUNC_HEADER_POS,pos));
            } break;
            case 3: {	// value
                int len=var->length();
                if( len>pos ) {
                    rst->add( var, pos+1, len );
                }
            } break;
            default: break;
            }
        }
    } break;
    case 'm' : {
        LPCC fnm=funcName? funcName : fc->getFuncName();
        U16 stat=var->getU16(2);
        if( stat==2 ) {
          QMimeData* mime=(QMimeData*)var->getObject(FUNC_HEADER_POS);
          if(ccmp(fnm,"text")) {
            rst->set(Q2A(mime->text()));
          } else if( ccmp(fnm,"image")) {
            QByteArray ba;
            QPixmap img = qvariant_cast<QPixmap>(mime->imageData());
            QBuffer buffer( &ba );
            buffer.open( QIODevice::WriteOnly );
            img.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format
            if( img.isNull() ) {
                qDebug()<< "image is null";
            } else if(ba.size() ) {
                rst->setVar('i',7).addInt(ba.size()).add(ba.constData(),ba.size());
            }
          } else if( ccmp(fnm,"html")) {
            rst->set(Q2A(mime->html()));
          } else if( ccmp(fnm,"hasText")) {
            rst->setVar('3', mime->hasText()? 1:0);
          } else if( ccmp(fnm,"hasHtml") ) {
                rst->setVar('3', mime->hasHtml()? 1: 0);
          } else if( ccmp(fnm,"hasImage") ) {
                rst->setVar('3', mime->hasImage()? 1: 0);
          } else if( ccmp(fnm,"data")) {
            LPCC type = cnt==0 ? "application/nodeData": AS(0);
            QByteArray ba = mime->data(type);
            rst->set(ba.constData(), ba.size());
          }
        } else if( stat==3 ) {
        }
        return true;
    } break;
    case 't' : {
#ifdef WIDGET_USE
        U16 stat=var->getU16(2);
        if(stat==21) {
            WatcherFile* watcher=(WatcherFile*)var->getObject(FUNC_HEADER_POS);
            TreeNode* cur=watcher->xcf;
            LPCC funcNm = fc->getFuncName();
            qDebug("watcher file func: %s", funcNm);
            if(ccmp(funcNm,"start")) {
                LPCC path=AS(0);
                if( watcher->start(path) ) {
                    rst->setVar('3',1);
                } else {
                    cur->set("target","");
                    rst->setVar('3',0);
                }
            } else if(ccmp(funcNm,"config")) {
                rst->setVar('n',0,(LPVOID)cur);
            } else if(ccmp(funcNm,"close")) {
                StrVar* sv=cur->gv("@watcher");
                if( SVCHK('f',1) ) {
                    cur->set("target","");
                    SAFE_DELETE(watcher);
                    sv->reuse();
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            } else if(ccmp(funcNm,"callback")) {
                StrVar* sv=arrs->get(1);
                TreeNode* thisNode=NULL;
                if( SVCHK('n',0)) {
                    thisNode=(TreeNode*)SVO;
                    sv = arrs->get(2);
                }
                if( SVCHK('f',1) ) {
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    XFuncNode* fnCur=setCallbackFunction(fsrc, fn, cur, NULL, cur->GetVar("@callback"));
                    if(thisNode) {
                        fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)cur);
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                    }
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            }
            return true;
        }
        if( stat==20 ) { // 't',20 timeline
            MyTimeLine* timeline=(MyTimeLine*)var->getObject(FUNC_HEADER_POS);
            TreeNode* tn=timeline->info();
            if( tn ) {
                rtn=callTimelineFunc(fc->xfuncRef==0?fc->getFuncName():NULL, arrs, tn, fn, rst, fc,timeline);
            }
        }

#endif
    } break;
    case 'e' : {
        U16 stat=var->getU16(2);
        if( stat==11 ) {
            QEventLoop* loop=(QEventLoop*)var->getObject(FUNC_HEADER_POS);
            if( loop ) {
                loop->quit();
            }
        }
        return true;
    } break;
    default: {
        if( ch ) {
            qDebug()<<"#0##execObjectFunc error | ch="<< var->charAt(1) << " funcNm:" << fc->getFuncName();
        }
        if( fc->xfuncRef==0 ) {
            LPCC funcNm = fc->getFuncName();
            regStrFuncs();
            fc->xfuncRef = getHashKeyVal("str", funcNm );
        }
        if( fc->xfuncRef ) {
            // PARR arrs=getParamArray(fc,fn,rst);
            rtn=execStrFunc(fc,arrs,fn,var,rst);
        }
    } break;
    }
    releaseLocalArray(arrs);
    return rtn;
}

inline int getArraySum(XListArr* a) {
    int tot = 0;
    if( a==NULL )
        return tot;
    for(int n=0,sz=a->size();n<sz;n++ ) {
        StrVar* sv = a->get(n);
        if( SVCHK('a',0) ) {
            tot+=getArraySum((XListArr*)SVO);
        } else {
            tot+=toInteger(sv);
        }
    }
    return tot;
}
inline int findArrayEntry(XListArr* a, StrVar* var, StrVar* rst) {
    if(a==NULL || var==NULL ) return -1;
    LPCC find=NULL;
    if( var->charAt(0)=='\b' ) {
        if( checkFuncObject(var,'n',0) || checkFuncObject(var,'a',0) || checkFuncObject(var,'f',1) || checkFuncObject(var,'f',0) ||
            checkFuncObject(var,'v',1) || checkFuncObject(var,'i',6) || checkFuncObject(var,'i',9) ) {
            char ch=var->charAt(1);
            U16 state=var->getU16(2);
            LPVOID find=var->getObject(FUNC_HEADER_POS);
            for(int n=0;n<a->size();n++) {
                StrVar* sv=a->get(n);
                if(SVCHK(ch,state)) {
                    if( find==SVO ) return n;
                }
            }
            return -1;
        }
        if( checkFuncObject(var, 'x',21)) {
            int pos=var->find("\f", FUNC_HEADER_POS);
            if( pos>FUNC_HEADER_POS ) {
                LPCC key=var->get(FUNC_HEADER_POS,pos);
                for(int n=0;n<a->size();n++) {
                    StrVar* sv=a->get(n);
                    if(SVCHK('x',21)) {
                        pos=sv->find("\f", FUNC_HEADER_POS);
                        if( pos>FUNC_HEADER_POS && ccmp(key,sv->get(FUNC_HEADER_POS,pos)) ) {
                            return n;
                        }
                    }
                }
            }
            return -1;
        }
        find=toString(var,rst);
    } else {
       find=toString(var,rst);
    }
    if(slen(find)) {
       for( int n=0; n<a->size(); n++ ) {
           if(ccmp(toString(a->get(n)),find)) {
              return n;
           }
       }
    }
    return -1;
}

bool execArrayFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* sv, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;
    PARR a = SVCHK('a',0) ? (PARR)SVO: NULL;
    if( a==NULL )
        return false;
    /*
    U32 flag= fc->xfflag&0xFF;
    if( flag==0x8 ) {}
    */
    rst->reuse();
    switch(fc->xfuncRef) {
    case 1: { // get
        rst->reuse();
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        int n=isNumberVar(sv) ? toInteger(sv):0;
        if( n<0 ) {
            n+= a->size();
            if( n>=0 ) {
                sv=a->get(n);
                rst->add(sv);
            }
        } else if( n<a->size() ) {
            sv=a->get(n);
            rst->add(sv);
        }
    } break;
    case 2:	{  // add
        if( cnt==0 )
            return false;

        int sp=0, ep=0;
        for( int n=0;n<cnt;n++ ) {
            StrVar* var=a->add();
            sv=arrs->get(n);
            if( SVCHK('s',0) ) {
                sv=getStrVarPointer(sv,&sp,&ep);
                if( sp<ep)
                    var->add(sv->get(sp),ep-sp);
            } else {
                var->add(sv);
            }
            if( n==0 ) {
                rst->reuse()->add(var);
            }
        }
        if( cnt>1 ) {
            rst->setVar('a',0,(LPVOID)a);
        }

    } break;
    case 6: { // set
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int num=toInteger(sv);
            int size=a->size();
            sv=arrs->get(1);
            StrVar* cur=NULL;
            if( num<size ) {
                cur=a->get(num);
            } else {
                cur=a->add();
            }
            if( cur ) {
                XFunc* parent=fc->parent();
                cur->reuse()->add(sv);
                if( parent && parent->getType()==FTYPE_SET ) {
                    rst->reuse()->add(cur);
                }
            }
        }
    } break;
    case 3: { // insert
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int idx=toInteger(sv);
            int sz1=a->size(), sz2=0;
            a->add();
            sz2=a->size();
            if( sz2==(sz1+1) && idx>=0 && idx<sz1 ) {
                for( int n=sz1-1;n>=idx; n-- ) {
                    int next=n+1;
                    StrVar* var=a->get(next);
                    var->reuse()->add(a->get(n));
                }
                setStrVar(a->get(idx), arrs->get(1));
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 8: { // reuse
        a->reuse();
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 9: { // dist
        int last=a->size()-1;
        if( cnt==0 ) {
            if( last>0 ) {
                double start=toDouble(a->get(0)), end=toDouble(a->get(last));
                if( start<end ) rst->setVar('4',0).addDouble(end-start);
            }
        }
        int cnt=arrs->size(), sz=a->size();
        if( cnt==1 ) {
            int sp=toInteger(arrs->get(0));
            if( sp<sz ) {
                double start=toDouble(a->get(0)), end=toDouble(a->get(last));
                if( start<end ) rst->setVar('4',0).addDouble(end-start);
            }
        } else {
            int sp=toInteger(arrs->get(0)), ep=toInteger(arrs->get(1));
            if( sp<sz ) {
                if( ep<=0 ) {
                    ep+=last;
                } else {
                    ep+=sp;
                }
                if( sp<ep ) {
                    if( ep>last ) ep=last;
                    double start=toDouble(a->get(sp)), end=toDouble(a->get(ep));
                    if( start<end ) rst->setVar('4',0).addDouble(end-start);
                }
            }
        }
        if( rst->length()==0 ) {
            rst->setVar('9',0);
        }
    } break;
    case 5: { // delete
        if( cnt && isVarTrue(arrs->get(0)) )
            deleteAllArray(a);
        if( a->isNodeFlag(FLAG_NEW) ) {
            SAFE_DELETE(a);
        }
    } break;
    case 7: { // pop
        rst->reuse();
        sv=a->get(0);
        if( sv ) {
            rst->add(sv);
            a->remove(0);
        }
    } break;
    case 4: { // index
        if( cnt==0 ) return false;

    } break;
    case 20: { // size
        rst->setVar('0',0).addInt(a->size());
    } break;
    case 21: { // remove
        if( cnt==0 ) {
            if( a->isNodeFlag(FLAG_NEW) ) {
                // SAFE_DELETE(a)
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('3',1)) {
            if( a->isNodeFlag(FLAG_NEW) ) {
                SAFE_DELETE(a)
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
                qDebug("@@ array remove error new instance check false !!!");
            }
            // rst->setVar('a',0,(LPVOID)a);
            return true;
        }
        // version 1.0.3
        int rtn=0;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int sp=toInteger(sv), ep=-1;
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                int last=a->size();
                ep=toInteger(sv);
                if( ep>last ) ep=last;
            }
            if( ep==-1 ) {
                rtn=a->remove(sp);
            } else if(sp<=ep ) {
                rtn=a->remove(sp, ep);
            }
            // rst->setVar('0',0).addInt(rtn); //, rtn>0? 1:0);
            rst->setVar('3', rtn>0 ? 1: 0);
            return true;
        }
        int find=findArrayEntry(a, sv, rst->reuse());
        if(find!=-1 ) {
            rtn=a->remove(find);
            rst->setVar('3', rtn>0 ? 1: 0);
        } else {
            rst->setVar('3', 0);
        }
    } break;
    case 22: { // join
        rst->reuse();
        LPCC sep = arrs? toString(arrs->get(0)): ",";
        LPCC id=NULL;
        int flag=0;
        if( sep && sep[0]=='#') {
            id=sep+1;
            if(ccmp(id,"#s")) flag=1;
            else if(ccmp(id,"#q")) flag=2;
            else if(ccmp(id,"#b")) flag=3;
            else flag=9;
            sep=",";
        }
        for(int n=0,sz=a->size();n<sz;n++) {
            if( n && sep ) rst->add(sep);
            if(flag) {
                switch(flag) {
                case 1:
                    rst->add('\"').add(toString(a->get(n))).add('\"');
                    break;
                case 2:
                    rst->add('\'').add(toString(a->get(n))).add('\'');
                    break;
                case 3:
                    rst->add("#{").add(toString(a->get(n))).add("}");
                    break;
                case 9:
                    sv=a->get(n);
                    if(SVCHK('n',0)) {
                        TreeNode* node=(TreeNode*)SVO;
                        sv=node->gv(id);
                        rst->add("#{").add(toString(sv)).add("}");
                    }
                default: break;
                }
            } else {
                rst->add(toString(a->get(n)));
            }
        }
    } break;
    case 23: { // sum
        if( cnt==0 ) {
            // rst->setVar('0',0).addInt(getArraySum(a));
            double tot=0;
            for(int n=0; n<a->size(); n++ ) {
                sv=a->get(n);
                tot+=toDouble(sv);
            }
            rst->setVar('4',0).addDouble(tot);
        } else {
            int sp=toInteger(arrs->get(0)), ep=a->size();
            if( arrs->get(1) ) {
                ep=toInteger(arrs->get(1));
                if( ep > a->size() ) ep=a->size();
            }
            double tot=0;
            for(;sp<ep ;sp++) {
                sv=a->get(sp);
                tot+=toDouble(sv);
            }
            rst->setVar('4',0).addDouble(tot);
        }
    } break;
    case 24: { // avg
        if( a->size()==0 ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        double tot=0;
        for(int n=0; n<a->size(); n++ ) {
            sv=a->get(n);
            tot+=toDouble(sv);
        }
        rst->setVar('4',0).addDouble(tot/a->size());
    } break;
    case 25: { // sort
        LPCC ord = NULL;
        LPCC field = NULL;
        LPCC subField = NULL;
        if( cnt==1 ) {
            ord=AS(0);
        } else if( cnt==2 ) {
            field=AS(0), ord=AS(1);
        } else if( cnt==3 ) {
            field=AS(0), subField=AS(1), ord=AS(2);
        }
        if( slen(ord)==0 ) ord="asc";
        bubbleSort(a,  ccmp(ord,"asc")?0 : ccmp(ord,"desc")?1 : ccmp(ord,"alphanum")? 2 : ccmp(ord,"alphanum_desc")?3 :0, field, subField );
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 26: { // recalc
        if( cnt==0 ) {
            XListArr* tmp=getListArrTemp();
            for(int n=0;n<a->size();n++) {
               tmp->add()->add(a->get(n));
            }
            rst->setVar('a',0,(LPVOID)tmp);
            return true;
        }
        int sz=a->size();
        sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* src=(XListArr*)SVO;
            if( cnt==1 ) {
                // ex) a.recalc(arr)  -- copy array
                 a->reuse();
                 for(int n=0;n<src->size();n++) {
                    a->add()->add(src->get(n));
                 }
            } else {

                sv=arrs->get(1);
                if( isNumberVar(sv) ) {
                    int pointCnt=toInteger(sv);
                    LPCC ty=AS(2);
                    a->reuse();
                    if(slen(ty) ) {
                        pointCnt*=2;
                        for( int n=0;n<pointCnt;n++) a->add();
                        getBezier2D(src, a, pointCnt);
                        if(ccmp(ty,"curveX")|| ccmp(ty,"curveY") ) {
                            int idx=0;
                            double prev=0;
                            char buf[64];
                            if(ccmp(ty,"curveX")) {
                                for( int n=0;n<pointCnt;n++) {
                                    if(n%2==0) {
                                        if(idx>0 ) rst->add(",");
                                        double v=toDouble(a->get(n));
                                        int num=(int)(prev-v);
                                        if(num==0) num=1;
                                        sprintf(buf,"%d",num);
                                        rst->add(buf);
                                        prev=v;
                                        idx++;
                                    }
                                }
                            } else {
                                for( int n=0;n<pointCnt;n++) {
                                    if(n%2==1) {
                                        if(idx>0 ) rst->add(",");
                                        double v=toDouble(a->get(n));
                                        int num=(int)(prev-v);
                                        if(num==0) num=1;
                                        sprintf(buf,"%d",num);
                                        rst->add(buf);
                                        prev=v;
                                        idx++;
                                    }
                                }
                            }
                        } else if(ccmp(ty,"point")) {
                            XListArr* list = getListArrTemp();
                            double sp=0, ep=0;
                            for( int n=0;n<pointCnt;n++) {
                                if(n%2==0) {
                                    sp=toDouble(a->get(n));
                                } else {
                                    ep=toDouble(a->get(n));
                                    list->add()->setVar('i',20).addDouble(sp).addDouble(ep);
                                }
                            }
                            rst->setVar('a',0,(LPVOID)list);
                        }
                    } else {
                        for( int n=0;n<pointCnt;n++) a->add();
                        getBezier2D(src, a, pointCnt);
                        rst->setVar('a',0,(LPVOID)a);
                    }
                } else {
                    qDebug("#0#array recalc curve number error !!!");
                }
            }
            rst->setVar('a',0,(LPVOID)a);
            return true;
        }
        if( !isNumberVar(sv) ) {
            if(sz==0) {
                return true;
            }
            LPCC ty=toString(sv);
            if(ccmp(ty,"sum") ) {
                double sum=0, v=0;
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv);
                    sum+=v;
                    sv->setVar('4',0).addDouble(sum);
                }
            } if(ccmp(ty,"step") ) {
                double v=0, prev=0;
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv);
                    if(n==0) {
                        sv->setVar('4',0).addDouble(v);
                    } else {
                        double d = v>prev ? v-prev: prev-v;
                        sv->setVar('4',0).addDouble(d);
                    }
                    prev=v;
                }
            } else if(ccmp(ty,"toInt") ) {
                int v=0;
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toInteger(sv);
                    sv->setVar('0',0).addInt(v);
                }
            } if(ccmp(ty,"toRound") ) {
                double v=0;
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv);
                    sv->setVar('0',0).addInt((int)rounding(v,0));
                }
            } else if(ccmp(ty,"toDouble") ) {
                double v=0;
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv);
                    sv->setVar('4',0).addDouble(v);
                }
            } else if(ccmp(ty,"times") ) {
                double v=0, rate=toDouble(arrs->get(1));
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv) * rate;
                    sv->setVar('4',0).addDouble(v);
                }
            } else if(ccmp(ty,"plus") ) {
                double v=0, plus=toDouble(arrs->get(1));
                for(int n=0;n<sz;n++) {
                    sv=a->get(n);
                    v=toDouble(sv) + plus;
                    sv->setVar('4',0).addDouble(v);
                }
            }
            rst->setVar('a',0,(LPVOID)a);
            return true;
        }

        double wid=toDouble(sv);
        a->reuse();
        sv=arrs->get(1);
        if( isNumberVar(sv) ) {
            int num = toInteger(sv);
            if( num>0 ) {
                double rate= (double)(wid/num);
                for( int n=0;n<num;n++ ) {
                    a->add()->setVar('4',0).addDouble(rate);
                }
            }
        } else {
            int sp=0, ep=0;
            StrVar* var=getStrVarPointer(sv, &sp, &ep);;
            XParseVar pv(var,sp,ep);
            if( pv.find("px")>0 ) {
                sv=arrs->get(2);
                double rate= sv ? toDouble(sv) : 0.0;
                return recalcArrayPixcel(a, pv, (int)wid, rst, rate);
            }
            int emptyCnt=0;
            double tot=0;
            while( pv.valid() ) {
                pv.findEnd(",");
                LPCC val=pv.v();
                if( isnum(val) ) {
                    double num=atof(val);
                    tot+=num;
                    a->add()->addDouble(num);
                } else {
                    a->add();
                    emptyCnt++;
                }
            }
            sz=a->size();
            if( emptyCnt>0 ) {
                double ww=0;
                if( wid> tot ) {
                    ww=(wid-tot) / emptyCnt;
                } else {
                    ww=(tot-wid) / emptyCnt;
                }
                for( int n=0; n<sz; n++ ) {
                    sv=a->get(n);
                    if( sv->length()==0 ) {
                        sv->addDouble(ww);
                        tot+=ww;
                    }
                }
            }
            double val=0;
            for( int n=0;n<sz;n++) {
                sv = a->get(n);
                val=sv->getDouble(0);
                if( wid!=tot ) {
                    double c=(double)(val/(double)tot)*wid;
                    sv->setVar('4',0).addDouble(c);
                } else {
                    sv->setVar('4',0).addDouble(val);
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 27: { // find
        if( cnt==0 ) {
            return false;
        }
        // version 1.0.2
        // int cnt=arrs->size();
        XFunc* pfc=fc->parent();
        if( pfc && pfc->getType()==FTYPE_ARRAY ) {
            pfc = pfc->parent();
        }
        bool bctrl=isTrueCheck(fc);
        int find=findArrayEntry(a, arrs->get(0), rst->reuse());
        if(find==-1) {
            if( bctrl ) {
                rst->setVar('3',0);
            } else {
                rst->setVar('0',0).addInt(-1);
            }
        } else {
            if( bctrl ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('0',0).addInt(find);
            }
        }
    } break;
    case 28: { // finds
        if( cnt==0 ) return false;
        int asz=arrs->size(), bsz=a->size();
        if( asz==0 || bsz==0 ) {
            rst->setVar('3',0);
        } else {
            bool ok=false;
            for( int n=0; n<asz; n++ ) {
                int find=findArrayEntry(a,arrs->get(n),rst->reuse());
                if(find!=-1) {
                    ok=true;
                    break;
                }
            }
            rst->setVar('3',ok?1:0);
        }
    } break;
    case 29: { // findPair
		StrVar* sv=NULL;
        if( cnt==0 ) {
			XListArr* arr=getListArrTemp();
			for(int n=0,sz=a->size(); n<sz; n++ ) {
				sv=a->get(n);
				if( SVCHK('x',21) ) {
					int pos=sv->find("\f",FUNC_HEADER_POS,64);
					if( pos>FUNC_HEADER_POS ) {
						arr->add()->add(sv,FUNC_HEADER_POS,pos);
					}
				}
			}
			rst->setVar('a',0,(LPVOID)arr);
			return true;
		}
		LPCC key=AS(0);
		XFunc* pfc=fc->parent();
		if( pfc && pfc->getType()==FTYPE_ARRAY ) {
			pfc = pfc->parent();
		}
		bool bctrl=false;
		if( pfc && (pfc->getType()==FTYPE_CONTROL||pfc->getType()==FTYPE_FUNCCHECK) ) {
			bctrl=true;
		}
		if( bctrl ) {
			if( findPairKey(a, key, rst) ) {
				rst->setVar('3',1);
			} else {
				rst->setVar('3',0);
			}
		} else {
			findPairKey(a, key, rst);
		}
    } break;
    case 30: { // findPairValue
		StrVar* sv=NULL;
        if( cnt==0 ) {
			XListArr* arr=getListArrTemp();
			for(int n=0,sz=a->size(); n<sz; n++ ) {
				sv=a->get(n);
				if( SVCHK('x',21) ) {
					int pos=sv->find("\f",FUNC_HEADER_POS,64);
					if( pos>FUNC_HEADER_POS ) {
						arr->add()->add(sv,pos+1,sv->length());
					}
				}
			}
			rst->setVar('a',0,(LPVOID)arr);
			return true;
		}
		StrVar* var=arrs->get(0);
		XFunc* pfc=fc->parent();
		if( pfc && pfc->getType()==FTYPE_ARRAY ) {
			pfc = pfc->parent();
		}
		bool bctrl=false;
		if( pfc && (pfc->getType()==FTYPE_CONTROL||pfc->getType()==FTYPE_FUNCCHECK) ) {
			bctrl=true;
		}
		int idx=-1;
		for(int n=0,sz=a->size();n<sz;n++) {
			if( getPairValue(a,n,rst) ) {
				if( compareResult(rst, var, "==") ) {
					idx=n;
					break;
				}
			}
		}
		if( bctrl ) {
			if( idx!=-1 ) {
				rst->setVar('3',1);
			} else {
				rst->setVar('3',0);
			}
		} else if( idx!=-1 ) {
			getPairKey(a, idx, rst->reuse());
		} else {
			rst->setVar('9',0);
		}
    } break;
    case 40: { // removeAll
        deleteAllArray(a);
    } break;
    case 41: { // copy
        if( cnt==0 ) {
            XListArr* arr=getListArrTemp();
            for( int n=0,sz=a->size(); n<sz; n++ ) {
                arr->add()->add(a->get(n));
            }
            rst->setVar('a',0, (LPVOID)arr);
            return true;
        }
        a->reuse();
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            XListArr* arr=NULL;
            int sp=toInteger(sv), ep=a->size();
            if( sp>ep ) sp=ep;
            sv=arrs->get(1);
            if( SVCHK('a',0) ) {
                arr=(XListArr*)SVO;
            } else if( isNumberVar(sv) ) {
                int pos=toInteger(sv);
                if( pos<0 ) {
                    ep+=pos;
                } else if(pos<ep ) {
                    ep=pos;
                }
                if(ep<sp) ep=sp;
                sv=arrs->get(2);
                if( SVCHK('a',0) ) {
                    arr=(XListArr*)SVO;
                }
            }
            if( arr==NULL ) arr=getListArrTemp();
            for( int n=sp; n<ep; n++) {
                arr->add()->add(a->get(n));
            }
            rst->setVar('a',0, (LPVOID)arr);
            return true;
        }

        if( SVCHK('a',0) ) {
            XListArr* arr=(XListArr*)SVO;
            for( int n=0,sz=arr->size();n<sz;n++) {
                a->add()->add(arr->get(n));
            }
        } else if( SVCHK('n',0) ) {
            TreeNode* tn = (TreeNode*)SVO;
            for( int n=0,sz=tn->childCount();n<sz;n++) {
                a->add()->setVar('n',0,(LPVOID)tn->child(n));
            }
        }
        rst->setVar('a',0, (LPVOID)a);

    } break;
    case 42: { // reverse
        XListArr* arr = getListArrTemp(); // gtrees.getArr();
        for( int n=a->size()-1; n>=0; n-- ) arr->add()->add(a->get(n));
        if( arrs && isVarTrue(arrs->get(0)) ) {
            a->reuse();
            for( int n=0,sz=arr->size(); n<sz; n++ ) a->add()->add(arr->get(n));
            rst->setVar('a',0,(LPVOID)a);
        } else {
            rst->setVar('a',0,(LPVOID)arr);
        }
    } break;
    case 43: { // decode
        int sz=a->size();

        if( cnt==0 ) {
            for(int n=sz-1;n>=0;n--) {
                rst->add(a->get(n));
            }
            qDebug("arr decode cnt:%d len:%d", sz, rst->length());
            if( sz==1 ) {
                U16 val=0;
                memcpy(&val, rst->get(), sizeof(val));
                rst->setVar('2',0).addU32((U32)val);
            } else if(sz==2 ) {
                U32 val=0;
                memcpy(&val, rst->get(), sizeof(val));
                rst->setVar('2',0).addU32(val);
            }
            return true;
        }

        LPCC ty=AS(0);
        if( isVarTrue(arrs->get(1)) ) {
            for(int n=sz-1;n>=0;n--) {
                rst->add(a->get(n));
            }
        } else {
            for(int n=0;n<sz;n++) {
                rst->add(a->get(n));
            }
        }
        qDebug("arr decode ty:%s cnt:%d len:%d", ty, sz, rst->length() );
        if( ccmp(ty,"U16") ) {
            U16 val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('2',0).addU32(val);
        } else if( ccmp(ty,"SHORT") ) {
            short val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('0',0).addInt(val);
        } else if( ccmp(ty,"UINT") ) {
            U32 val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('2',0).addU32(val);
        } else if( ccmp(ty,"INT") ) {
            int val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('0',0).addInt(val);
        } else if( ccmp(ty,"FLOAT") ) {
            float val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('4',0).addDouble((double)val);
        } else if( ccmp(ty,"DOUBLE") ) {
            double val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('4',0).addDouble(val);
        } else if( ccmp(ty,"LONG") || ccmp(ty,"U64") ) {
            UL64 val=0;
            memcpy(&val, rst->get(), sizeof(val));
            rst->setVar('2',0).addUL64(val);
        }
    } break;
    case 44: { // addPair
        if( cnt==0 ) return false;
        int size=arrs->size();
        rst->setVar('x',21);
        if(size==3 && isNumberVar(arrs->get(0))) {
            int idx=toInteger(arrs->get(0));
            StrVar* var=a->get(idx);
            if( var) {
                LPCC key=AS(1);
                rst->add(key).add('\f').add(arrs->get(2));
                var->reuse()->add(rst);
            }
        } else {
            LPCC key=AS(0);
            rst->add(key).add('\f').add(arrs->get(1));
            a->add()->add(rst);
        }
    } break;
    case 45: { // div
        getDivArr(a, arrs, rst);
        // rst->setVar('a',0,(LPVOID)a);
    } break;
    case 46: { // toString
        for( int n=0,sz=a->size();n<sz;n++) {
            LPCC value=toString(a->get(n));
            if( n>0 ) rst->add(',');
            rst->add(value);
        }
    } break;
    case 47: { // addU16
        if( cnt==0 ) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            U16 val=(U16)toInteger(sv);
            a->add()->addU16(val);
        }
    } break;
    case 48: { // addHex
        if( cnt==0 ) {
            return false;
        }
        int sp=0, ep=0;
        StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        XParseVar pv(sv, sp, ep);
        LPCC sep=AS(1);
        if( slen(sep)==0 ) sep=" ";
        while( pv.valid() ) {
            U16 num=0;
            LPCC val=pv.findEnd(sep).v();
            sscanf(val,"%x",&num);
            a->add()->addU16(num);
            qDebug("addHex: %s %d", val, num);
        }
    } break;
    case 49: { // toHex
        LPCC sep=arrs?AS(0): NULL;
        if( slen(sep)==0 ) sep=" ";
        for( int n=0, sz=a->size(); n<sz; n++ ) {
            U16 val=a->get(n)->getU16();
            if( n>0 ) rst->add(' ');
            rst->format(4,"%04X",val);
        }
    } break;
	case 50: { // useArray
		int sz=a->size();
		qDebug("useArray ========== cnt:%d", cnt);
		for(int n=sz-1;n>=0;n--) {
			StrVar* sv=a->get(n);
			if(SVCHK('n',0)) {
				TreeNode* node=(TreeNode*)SVO;
				if(node->isNodeFlag(FLAG_DELETE)) {
					qDebug("remove ========== %d", n);
					a->remove(n);
				}
			} else if(SVCHK('9',0) || sv->length()==0 ) {
				qDebug("remove ========== %d", n);
				a->remove(n);
			}
		}
		rst->setVar('a',0,(LPVOID)a);
	} break;
	case 51: { // swap
        if( cnt==0 ) {
			return false;
		}
		int sp=toInteger(arrs->get(9)), ep=toInteger(arrs->get(1));
		if( sp>=0 && sp<ep && ep<a->size()-1 ) {
			rst->reuse()->add(a->get(ep));
			a->get(ep)->reuse()->add(a->get(sp));
			a->get(sp)->reuse()->add(rst);
			rst->setVar('3',1);
		} else {
			rst->setVar('3',0);
		}
	} break;
	case 52: { // filter
        if( cnt==0 ) {
			return false;
		}
		sv=arrs->get(0);
		if( SVCHK('f',1) ) {
			XFuncSrc* fsrc=(XFuncSrc*)SVO;
			XListArr* arr=NULL;
			sv=arrs->get(1);
			if(SVCHK('a',0)) {
				arr=(XListArr*)SVO;
			} else {
				arr=getListArrTemp();
			}
			int sz=a->size();
			XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            sv=arrs->get(1);
            if( !SVCHK('n',0) ) {
                sv=fn?fn->gv("@this"): NULL;
            }
			if(SVCHK('n',0)) {
				TreeNode* thisNode=(TreeNode*)SVO;
				fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                fnCur->GetVar("origin")->setVar('a',0,(LPVOID)a);
			}
            for(int n=0;n<sz;n++) {
                arrs->reuse();
                arrs->setNodeFlag(FLAG_LOCALARRAY_SET);
                arrs->add()->add(a->get(n));
                arrs->add()->setVar('0',0).addInt(n);
                setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
                fnCur->call();
                sv=fnCur->getResult();
                if(SVCHK('3',1)) {
                    arr->add()->add(a->get(n));
                }
            }
			funcNodeDelete(fnCur);
			rst->setVar('a',0,(LPVOID)arr);
		}
	} break;
	case 53: { // map
        if( cnt==0 ) {
			return false;
		}
		sv=arrs->get(0);
		if( SVCHK('f',1) ) {
			XFuncSrc* fsrc=(XFuncSrc*)SVO;
			XListArr* arr=NULL;
			sv=arrs->get(1);
			if(SVCHK('a',0)) {
				arr=(XListArr*)SVO;
			} else {
				arr=getListArrTemp();
			}
			int sz=a->size();
			XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            sv=arrs->get(1);
            if( !SVCHK('n',0) ) {
                sv=fn?fn->gv("@this"): NULL;
            }
            if(SVCHK('n',0)) {
                TreeNode* thisNode=(TreeNode*)SVO;
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                fnCur->GetVar("origin")->setVar('a',0,(LPVOID)a);
            }
			for(int n=0; n<sz;n++ ) {
				sv=a->get(n);
                arrs->reuse();
                arrs->add()->add(a->get(n));
                arrs->add()->setVar('0',0).addInt(n);
                setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
                fnCur->call();
                arr->add()->add(fnCur->getResult());
			}
			funcNodeDelete(fnCur);
			rst->setVar('a',0,(LPVOID)arr);
		}

	} break;
	case 991: { // with
        XFunc* func=fc->getParam(0);
        if( func==NULL ) return true;
        XParseVar* pv=NULL;
        XParseVar sub;
        if( func->xftype==FTYPE_TEXT || func->xftype==FTYPE_STR ) {
            StrVar* sv=cfVar(NULL);
            takeParseVar(sv,func,fn);
            sub.SetVar(sv, 0, sv->length() );
            pv=&sub;
        } else {
            pv=&(func->xpv);
            pv->SetPosition();
        }
        while( pv->valid() ) {
            char ch=pv->ch();
            if( ch==0 ) break;
            if( ch==',' ) {
                a->add();
                pv->incr();
                continue;
            }
            if( !setResultVal(pv,fn,a->add()) ) {
                break;
            }
            if( pv->ch()==',' ) {
                pv->incr();
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 992: { // inject
        cnt=fc->getParamSize();
        int sz=a->size();
        int num=0;
        for( int n=0;n<cnt;n++) {
            XFunc* fcParam=fc->getParam(n);
            if( fcParam->xftype==FTYPE_NUMBER ) {
                if( n==0 ) {
                    LPCC val=fcParam->getValue();
                    if( isnum(val) ) {
                        num=atoi(val);
                        if( num<0 || num>=sz ) break;
                        continue;
                    }
                } else break;
            } else if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
                StrVar* textVar=cfVar(NULL);
                takeParseVar(textVar,fcParam,fn);
                XParseVar pv(textVar );
                while( pv.valid() ) {
                    LPCC code=pv.NextWord();
                    if( slen(code)==0 ) break;
                    if( num < sz ) {
                        sv=a->get(num++);
                    } else {
                        sv=NULL;
                    }
                    fn->GetVar(code)->reuse()->add(sv);
                    n++;
                    if( pv.ch()==',' ) pv.incr();
                }
                break;
            }
            bool bref=false;
            LPCC code = fcParam->getVarName();
            if( code[0]=='&' ) {
                bref=true;
                code+=1;
            }
            if( num < sz ) {
                sv=a->get(num++);
            } else {
                sv=NULL;
            }
            if( bref ) {
                if( sv ) {
                    if( sv->charAt(0)=='\b' ) {
                        fn->GetVar(code)->reuse()->add(sv);
                    } else if( sv ) {
                        int sp=0, ep=sv->length();
                        fn->GetVar(code)->setVar('s',0, (LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                    } else {
                        fn->GetVar(code)->reuse();
                    }
                } else {
                    fn->GetVar(code)->reuse();
                }
            } else {
                //#baropage
                StrVar* var=fn->GetVar(code)->reuse();
                if( SVCHK('s',0)) {
                    int sp=0, ep=0;
                    sv=getStrVarPointer(sv,&sp,&ep);
                    if( sp<ep ) var->add(sv->get(sp), ep-sp );
                } else {
                    var->add(sv);
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 993: { // put
        cnt=fc->getParamSize();
        if( cnt==0 ) {
            return true;
        }
        XFunc* fcParam=fc->getParam(0);
        int num=0, sz=a->size(), n=0;
        if( fcParam->xftype==FTYPE_NUMBER ) {
            LPCC val=fcParam->getValue();
            if( isnum(val) ) {
                n=1;
                num=atoi(val);
                if( num<0 ) {
                    return true;
                }
                if( num> sz ) {
                    int dist=num-sz;
                    for( int x=0; x<dist; x++ ) a->add();
                    sz=a->size();
                }
            }
        }
        for( ; n<cnt; n++ ) {
            fcParam=fc->getParam(n);
            if( num<sz ) {
                sv=a->get(num);
            } else {
                sv=a->add();
            }
            execParamFunc(fcParam, fn, rst->reuse());
            setStrVar(sv, rst);
            num++;
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    default: return false;
    }
    return true;
}

bool execInternalFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    if( fc->xfkind==0 ) {
        LPCC vnm = fc->getVarName();
        fc->xfkind = ccmp(vnm,"System") ? FKIND_SYSTEM :
            ccmp(vnm,"Baro") ? FKIND_CLASS:
            ccmpl(vnm,2,"cf",2) ? FKIND_CF:
            ccmp(vnm,"Math") ? FKIND_MATH: 0;
        switch( fc->xfkind) {
        case FKIND_SYSTEM :	regSystemFuncs(); break;
        case FKIND_CF :		regConfigFuncs(); break;
        case FKIND_CLASS :	regClassFuncs(); break;
        case FKIND_MATH :	regMathFuncs(); break;
        }
    }
    if( fc->xfkind==0 )
        return false;

    if( fc->xfuncRef==0 ) {
        switch( fc->xfkind) {
        case FKIND_SYSTEM :	fc->xfuncRef=getHashKeyVal("system",fc->getFuncName()); break;
        case FKIND_CF :		fc->xfuncRef=getHashKeyVal("cf",fc->getFuncName()); break;
        case FKIND_CLASS :	fc->xfuncRef=getHashKeyVal("class",fc->getFuncName()); break;
        case FKIND_MATH :	fc->xfuncRef=getHashKeyVal("math",fc->getFuncName()); break;
        }
    }
    if( fc->xfuncRef==0 )
        return false;
    bool rtn = false;
    switch(fc->xfkind) {
    case FKIND_SYSTEM :		rtn = execSystemFunc(fc,arrs,fn,rst); break;
    case FKIND_CF :			rtn = execConfigFunc(fc,arrs,fn,rst); break;
    case FKIND_CLASS :		rtn = execClassFunc(fc,arrs,fn,rst); break;
    case FKIND_MATH :		rtn = execMathFunc(fc,arrs,fn,rst); break;
    default: break;
    }
    return rtn;
}


inline void SystemShutdown(UINT nSDType) {
#ifdef WINDOW_USE
    HANDLE           hToken;
    TOKEN_PRIVILEGES tkp   ;

    ::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);
    ::LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount          = 1                   ; // set 1 privilege
    tkp.Privileges[0].Attributes= SE_PRIVILEGE_ENABLED;

    // get the shutdown privilege for this process
    ::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    switch (nSDType)
    {
        case 0: ::ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCE, 0); break;
        case 1: ::ExitWindowsEx(EWX_POWEROFF|EWX_FORCE, 0); break;
        case 2: ::ExitWindowsEx(EWX_REBOOT  |EWX_FORCE, 0); break;
    }
#endif
}

#ifdef KEYHOOK_USE
HHOOK hHook=NULL;
inline bool setKeyEventClose() {
    if( hHook ) {
        UnhookWindowsHookEx(hHook);
        hHook=NULL;
        return true;
    }
    return false;
}
inline bool setKeyEventProc() {
    if( hHook ) {
        UnhookWindowsHookEx(hHook);
        hHook=NULL;
    }

    if( hHook==NULL ) {
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, MyKeyBoardProc, GetModuleHandle(NULL), 0);
        if (hHook == NULL) {
            return false;
        }
    }
    return true;
}
#endif

bool execSystemFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;
    switch(fc->xfuncRef) {
    case 20: { // funcList

    } break;
    case 21: { // tickCheck
    } break;
    case 22: { // openExplore
#ifdef WINDOW_USE
		if( cnt==0 )
			return false;
		StrVar* sv = arrs ? arrs->get(1): NULL;
		XListArr* a = NULL;
		if( SVCHK('a',0)) {
			a = (XListArr*)SVO;
		} else {
			int sp=0, ep=0;
			sv=getStrVarPointer(sv, &sp, &ep);
			XParseVar pv(sv, sp, ep);
            a=getListArrTemp();
			while( pv.valid() ) {
				a->add()->add( pv.findEnd(",").v() );
			}
		}
		LPCC path =AS(0);
		QString rootPath = QDir::toNativeSeparators(KR(path));
		int len=rootPath.length();
		if( rootPath.at(len-1)!=QChar('\\') ) rootPath+="\\";
		ITEMIDLIST* itemList[1024];
		ITEMIDLIST* dir = ILCreateFromPath(rootPath.toStdWString().data());
		if( dir==NULL )
			return false;
		bool rootAdd=false;
		int count = min( a?a->size():0, 1024);
		if( count==0 ) {
			itemList[0] = dir;
			count=1;
			rootAdd=true;
		} else {
			for (int n=0; n<count; n++) {
				sv = a->get(n);
				QString name=rootPath;
				LPCC fnm=NULL;
				if( SVCHK('n',0) ) {
					TreeNode* node=(TreeNode*)SVO;
					fnm=node->get("name");
				} else {
					fnm=toString(sv);
				}
				name+=KR(fnm);
				itemList[n] = ILCreateFromPath(name.toStdWString().data() );
			}
		}
		class ShellCallThread : public QThread
		{
		public:
			ShellCallThread(PCIDLIST_ABSOLUTE pidlFolder, UINT cidl, PCUITEMID_CHILD_ARRAY apidl)
				: pidlFolder(pidlFolder), cidl(cidl), apidl(apidl) {}
			PCIDLIST_ABSOLUTE pidlFolder;
			UINT cidl;
			PCUITEMID_CHILD_ARRAY apidl;

			virtual void run() {
				CoInitialize(NULL);
				HRESULT hresult = SHOpenFolderAndSelectItems(pidlFolder, cidl, apidl, 0);
				CoUninitialize();
			}
		};
		QEventLoop eventLoop;
		ShellCallThread *pThread = new ShellCallThread((PCIDLIST_ABSOLUTE)dir, count, (PCUITEMID_CHILD_ARRAY)itemList);
		pThread->start();
		while(!pThread->wait(1))
			eventLoop.processEvents();
		delete pThread;

		ILFree((PIDLIST_RELATIVE)dir);
		if( rootAdd==false ) {
			for (int n=0; n<count; n++) {
				ILFree((PIDLIST_RELATIVE)itemList[n]);
			}
		}
#endif
    } break;
    case 23: { // rand
        static int xseed = 256;
        if( xseed%2==0 )
            xseed+=11;
        else
            xseed-=7;
        srand( mixNum(clock(), time(NULL), xseed) );
        U32 r=rand();
        int maxNum=arrs ? toInteger(arrs->get(0)): 0;
        if( maxNum==0 ) maxNum=100;
        rst->setVar('0',0).addInt(r%maxNum);
    } break;
    case 24: { // isRun
        if( cnt==0) return true;
        LPCC processName=AS(0);
        if( slen(processName) ) {
            QProcess tasklist;
            QString process=KR(processName);
            tasklist.start( "tasklist",
                QStringList() << "/NH"
                              << "/FO" << "CSV"
                              << "/FI" << QString("IMAGENAME eq %1").arg(process));
            tasklist.waitForFinished();
            QString output = tasklist.readAllStandardOutput();
            output.startsWith(QString("\"%1").arg(process));
        }
    } break;
    case 25: { // run
        if( cnt==0 ) return true;
    } break;
    case 26: { // memoryStatus
#ifdef WINDOW_USE
        MEMORYSTATUSEX memory_status;
        ZeroMemory(&memory_status, sizeof(MEMORYSTATUSEX));
        memory_status.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memory_status)) {
            rst->set(Q2A(QString("RAM: %1 MB").arg(memory_status.ullTotalPhys / (1024 * 1024))));
        } else {
            rst->set("Unknown Memory status");
        }
#endif
    } break;
    case 27: { // info
        if( cnt==0 ) {
#ifdef _WIN32
        TCHAR  infoBuf[4096];
        DWORD  bufCharCount = 4096;
        if( GetComputerName( infoBuf, &bufCharCount ) ) {
            W2UTF8(infoBuf, rst);
        }
        rst->add("::");
        if( GetUserName( infoBuf, &bufCharCount ) ) {
            W2UTF8(infoBuf, rst);
        }
#else
        #include <unistd.h>
        #include <limits.h>

        char hostname[HOST_NAME_MAX];
        char username[LOGIN_NAME_MAX];
        gethostname(hostname, HOST_NAME_MAX);
        getlogin_r(username, LOGIN_NAME_MAX);
#endif
            return true;
        }
        LPCC ty=AS(0);
        if( ccmp(ty,"screenCount") ) {
            rst->setVar('0',0).addInt( QGuiApplication::screens().size()); // QGuiApplication::desktop()->screenCount() );
        } else if( ccmp(ty,"screenRect") ) {
            if( cnt==1 ) {
                QScreen* pScreen = QGuiApplication::screens().at(0);
                if(pScreen) setVarRectF(rst, pScreen->geometry());
            } else {
                StrVar* sv = arrs->get(1);
                if(isNumberVar(sv)) {
                    int idx=toInteger(sv);
                    QScreen* pScreen = QGuiApplication::screens().at(idx);
                    if(pScreen) setVarRectF(rst, pScreen->geometry());
                }
            }
        } else if( ccmp(ty,"cursor") ) {
            QPoint pt = QCursor::pos();
            StrVar* sv=arrs->get(1);
            if( SVCHK('i',2) ) {
                pt.setX(sv->getInt(4)), pt.setY(sv->getInt(8));
                QCursor::setPos(pt);
            } else {
                rst->setVar('i',2).addInt(pt.x()).addInt(pt.y());
            }
        } else if( ccmp(ty,"device") ) {
#ifdef WINDOW_USE
            TCHAR  infoBuf[4096];
            UINT devs = waveInGetNumDevs();
            devs = waveOutGetNumDevs();
            for (UINT dev = 0; dev < devs; dev++) {
                WAVEOUTCAPS caps = {};
                MMRESULT mmr = waveOutGetDevCaps(dev, &caps, sizeof(caps));
                if (MMSYSERR_NOERROR != mmr) {
                     break;
                }
                swprintf( infoBuf, 4096,
                    L"-- device #%u --\n"
                    L"Manufacturer ID: %u\n"
                    L"Product ID: %u\n"
                    L"Version: %u.%u\n"
                    L"Product Name: %s\n"
                    L"Formats: 0x%x\n"
                    L"Channels: %u\n"
                    L"Reserved: %u\n"
                    L"Support: 0x%x\n"
                    L"%s%s%s%s%s"
                    ,
                    dev,
                    caps.wMid,
                    caps.wPid,
                    caps.vDriverVersion / 256, caps.vDriverVersion % 256,
                    caps.szPname,
                    caps.dwFormats,
                    caps.wChannels,
                    caps.wReserved1,
                    caps.dwSupport,
                        ((caps.dwSupport & WAVECAPS_LRVOLUME) ?       L"tWAVECAPS_LRVOLUME\n" :       L""),
                        ((caps.dwSupport & WAVECAPS_PITCH) ?          L"tWAVECAPS_PITCH\n" :          L""),
                        ((caps.dwSupport & WAVECAPS_PLAYBACKRATE) ?   L"tWAVECAPS_PLAYBACKRATE\n" :   L""),
                        ((caps.dwSupport & WAVECAPS_VOLUME) ?         L"tWAVECAPS_VOLUME\n" :         L""),
                        ((caps.dwSupport & WAVECAPS_SAMPLEACCURATE) ? L"tWAVECAPS_SAMPLEACCURATE\n" : L"")
                );
                if(dev!=0) rst->add("\n\n");
                W2UTF8(infoBuf, rst);
            }
#endif
        } else if( ccmp(ty,"realSize") ) {
            int nWidth= 0, nHeight=0;
#ifdef WINDOW_USE
            if( cnt==1 ) {
                nWidth= GetSystemMetrics(SM_CXSCREEN), nHeight = GetSystemMetrics(SM_CYSCREEN);
            } else {
                nWidth= GetSystemMetrics(SM_CXVIRTUALSCREEN); nHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
            }
#endif
            rst->setVar('i',2).addInt(nWidth).addInt(nHeight);
        } else if( ccmp(ty,"titleHeight") ) {
            int height=QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);
            rst->setVar('0',0).addInt(height);
        }
    } break;
    case 1: { // tick
        rst->setVar('1',0).addUL64(GetTickCount());
    } break;
    case 2: { // path
        if( cnt==0 ) {
            rst->add( Q2A(QCoreApplication::applicationDirPath()) );
        }
     } break;
    case 3: { // networkInfo
        XListArr* a=getListArrTemp();
#ifdef WEBTOOL_USE
        QString localhostname =  QHostInfo::localHostName();
        QString localhostIP="";
        QString InterfaceName="";
        QString localMacAddress="";
        QString networkMask="";
        QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
        for(int i=0; i < ifaces.size(); i++) {
            unsigned int flags = ifaces[i].flags();
            bool isLoopback = (bool)(flags & QNetworkInterface::IsLoopBack);
            bool isP2P = (bool)(flags & QNetworkInterface::IsPointToPoint);
            bool isRunning = (bool)(flags & QNetworkInterface::IsRunning);

            // If this interface isn't running, we don't care about it
            if( !isRunning ) continue;
            // We only want valid interfaces that aren't loopback/virtual and not point to point
            if( !ifaces[i].isValid() || isLoopback || isP2P ) continue;
            foreach(const QNetworkAddressEntry &entry, ifaces[i].addressEntries()) {
                if( entry.ip().protocol() == QAbstractSocket::IPv4Protocol
                    && entry.ip() != QHostAddress(QHostAddress::LocalHost)
                    && entry.netmask().toString() != "")
                {
                    localhostIP = entry.ip().toString();
                    networkMask= entry.netmask().toString();
                    InterfaceName=ifaces[i].name();
                    localMacAddress = ifaces[i].hardwareAddress();
                    break;
                }
            }
        }
        /* host, ip, mac, interfaceName */
        a->add()->set(Q2A(localhostname));
        a->add()->set(Q2A(localhostIP));
        a->add()->set(Q2A(networkMask));
        a->add()->set(Q2A(localMacAddress));
        a->add()->set(Q2A(InterfaceName));
#endif
        rst->setVar('a',0,(LPVOID)a);
    } break;

    case 6: { // config
        TreeNode* info=uom.getInfo();
        if( cnt==0 ) {
            rst->setVar('n',0,(LPVOID)info);
        }

    } break;
    case 7: { // localtime
        // struct tm* tm = NULL;
        time_t tt;
        UL64 a=0;
        if( cnt==0 ) {
            rst->setVar('1',0).addUL64((UL64)getTm());
            return true;
        } else if( cnt==1) {
            // default day
            if( isNumberVar(arrs->get(0)) ) {
                int n=toInteger(arrs->get(0));
                QDateTime dt = QDateTime::currentDateTime();
                dt.addDays(n);
                a = dt.toLocalTime().toTime_t();
            } else {
                LPCC dtm = AS(0);
                if( ccmp(dtm,"utc") ) {
                    QDateTime dt = QDateTime::currentDateTimeUtc();
                    a = dt.toLocalTime().toTime_t();
                } else {
                    if( slen(dtm)==8 )
                        dtm=gBuf.fmt("%s000000", dtm);
                    else if( slen(dtm)==10 )
                        dtm=gBuf.fmt("%s 00:00:00", dtm);
                    QDateTime dt=getDateTime(dtm);
                    a = dt.toTime_t();
                }
            }
        } else if( cnt==2 ) {
            if( isNumberVar(arrs->get(0)) ) {
                QDateTime dt=QDateTime::currentDateTime();
                a=getTimet(AS(1), toInteger(arrs->get(0)), dt );
            } else {
                LPCC ty=AS(0);
                QDateTime dt = QDateTime::currentDateTime();
                int dist=toInteger(arrs->get(1));
                if( ccmp(ty,"incrYear") ) {
                   dt.addYears(dist);
                } else if( ccmp(ty,"incrMonth") ) {
                    dt.addMonths(dist);
                } else if( ccmp(ty, "incrDay") ) {
                    dt.addDays(dist);
                } else {
                    dt.addSecs(dist);
                }
                a = dt.toTime_t();
            }
            rst->setVar('1',0).addUL64((UL64)a);
        } else if( cnt==3 ) {
            StrVar* sv=arrs->get(0);
            if( SVCHK('1',0) ) {
                QDateTime dt;
                dt.setTime_t(toUL64(sv));
                a=getTimet(AS(2), toInteger(arrs->get(1)), dt );
            }
        }
        if( a ) rst->setVar('1',0).addUL64((UL64)a);
    } break;
    case 10: { // driveList
        XListArr* a = getListArrTemp();
        foreach( QFileInfo drive, QDir::drives() )  a->add()->set(Q2A(drive.absolutePath()));
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 11: { // trashList
        TreeNode* root = getTreeNodeTemp();
        root->reuse();
#ifdef WINDOW_USE
        /*
        typedef IShellDetails	FAR*	PSHELLDETAILS;
        typedef IShellFolder2	FAR*	LPSHELLFOLDER2;
        typedef IShellIcon		FAR*	LPSHELLICON;

        LPMALLOC pMalloc = NULL;
        LPSHELLFOLDER2	m_pFolder2;
        LPSHELLFOLDER	pDesktop		= NULL;
        LPITEMIDLIST	pidlRecycleBin	= NULL;
        STRRET			strRet;
        int iSubItem = 0;
        SHELLDETAILS sd;
        if( SUCCEEDED(SHGetDesktopFolder(&pDesktop)) &&
            SUCCEEDED(SHGetSpecialFolderLocation (NULL, CSIDL_BITBUCKET, &pidlRecycleBin)) &&
            SUCCEEDED(pDesktop->BindToObject(pidlRecycleBin, NULL, IID_IShellFolder2, (LPVOID *)&m_pFolder2)) &&
            SUCCEEDED(pDesktop->GetDisplayNameOf (pidlRecycleBin, SHGDN_NORMAL, &strRet)) ) {
        }
        SHGetMalloc(&pMalloc);
        // HRESULT hr = S_OK;
        StrVar* h = root->GetVar("headers")->reuse();
        while ( true ) {
            if( FAILED(m_pFolder2->GetDetailsOf (NULL , iSubItem, &sd)) ) break;
            if( iSubItem++>0 ) h->add(',');
            h->add(W2C(sd.str.pOleStr));
            pMalloc->Free(sd.str.pOleStr);
        }
        LPITEMIDLIST	pidl = NULL;
        LPENUMIDLIST	penumFiles;
        if( FAILED(m_pFolder2->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS| SHCONTF_INCLUDEHIDDEN, &penumFiles)) ) {

        }
        char code[32];
        while( SUCCEEDED(penumFiles->Next(1, &pidl, NULL)) ) {
            TreeNode* node = root->addNode();
            iSubItem = 0;
            while (SUCCEEDED(m_pFolder2->GetDetailsOf (pidl , iSubItem, &sd)) ) {
                sprintf(code, "", iSubItem);
                node->GetVar(code)->set(W2C(sd.str.pOleStr));
                pMalloc->Free(sd.str.pOleStr);
                iSubItem++;
            }
        }
        if( penumFiles ) penumFiles->Release();
        pMalloc->Release();
        */
#else
        QString trashPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation) + "/.local/share/Trash/files";
        // XDir dir(Q2A(trashPath));
        // makeDirModelData(&dir, root);
#endif
    } break;
    case 12: { // filePath
        if( cnt==0 ) {
            rst->add(Q2A(QCoreApplication::applicationDirPath()));
            return true;
        }
        LPCC fileName=AS(0);
        StrVar* sv=arrs->get(1);
        LPCC fullName=getFullFileName(fileName);
        if( SVCHK('3',1) ) {
            makeFileFolder(fullName, true, rst);
        }
        rst->set(fullName);
    } break;
    case 15: { // execApp
        if( cnt==0 ) {
            return true;
        }
#ifdef WIDGET_USE
        LPCC prog=AS(0);
        qDebug("#0#exec application : %s\n", prog);
        QString progName=KR(prog);
        STARTUPINFO si = { 0 };
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWNORMAL;

        PROCESS_INFORMATION pi = { 0 };

        BOOL rc = ::CreateProcess(NULL, (LPTSTR)progName.toStdWString().c_str(), NULL, NULL, FALSE,
                NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi);

        // close process and thread handles now (app will continue to run)
        if (pi.hProcess)
            ::CloseHandle(pi.hProcess);
        if (pi.hThread)
            ::CloseHandle(pi.hThread);
        rst->setVar('3', rc?1:0);
#endif
    } break;
    case 16: { // screenCapture
        LPCC path=NULL;
        int screenIndex=0;
        QRectF rect;
        StrVar* bufVar=NULL;
        StrVar* sv=NULL;
        // screenIndex, rect, savePath
        for(int n=0;n<cnt;n++) {
            sv=arrs->get(n);
            if(isNumberVar(sv)) {
                screenIndex=toInteger(sv);
            } else if(SVCHK('s',1)) {
                bufVar=(StrVar*)SVO;
            } else if(SVCHK('i',5)||SVCHK('i',4)) {
                rect=getQRectF(sv);
            } else if(SVCHK('9',0) || SVCHK('3',0) || SVCHK('3',1)) {

            } else {
                path=toString(sv);
            }
        }
        int screenCount=QGuiApplication::screens().size();
        if( screenIndex<screenCount ) {
            QScreen *screen = QApplication::screens().at(screenIndex);
            if( !rect.isValid()) {
                rect=screen->geometry();
            }
            WId wid=QApplication::desktop()->winId();
            QPixmap qPixmap= rect.isValid() ?
                    screen->grabWindow(wid, rect.x(), rect.y(), rect.width(), rect.height()):
                    screen->grabWindow(wid);

            if( slen(path)>0 ) {
                QImageWriter imgWriter(KR(path), "JPEG");
                imgWriter.write(qPixmap.toImage());
                rst->setVar('3',1);
            } else if(bufVar==NULL) {
                bufVar=rst;
            }
            if( bufVar ){
                QByteArray ba;
                QBuffer buffer( &ba );
                buffer.open( QIODevice::WriteOnly );
                qPixmap.save(&buffer, "JPEG");
                if( ba.size() ) {
                    bufVar->setVar('i',7).addInt(ba.size()).add(ba.constData(), ba.size());
                }
            }
        } else {
            qDebug("#9#screen capture error screen number not valid (screen number:%d)", screenIndex);
        }
    } break;
    case 32: { // inode
        if( cnt==0 )
            return false;
        LPCC path = AS(0);
        if( slen(path) )
            getFileInode(path, rst->reuse());
    } break;
    case 33: { // date
        time_t t = time(&t);
        LPCC fmt="yyyy-MM-dd hh:mm:ss";
        StrVar* sv = NULL;
        if( arrs ) {
            sv = arrs->get(0);
            if( isNumberVar(sv) ) {
                t = (time_t)toUL64(sv);
                if( cnt>1 ) {
                    fmt=AS(1);
                }
            } else {
                fmt=AS(0);
            }
        }
        if( t ) {
            if( ccmp(fmt,"GMT") ) {
                fmt="%a, %d %b %Y %H:%M:%S GMT";
                rst->ftime("%a, %d %b %Y %H:%M:%S GMT", localtime(&t) );
            } else {
                rst->ftime("%Y%m%dT%H%M%S",localtime(&t));
                if( ccmp(fmt,"dayOfWeek") ) {
                    QDateTime dt=getDateTime(rst->get());
                    rst->setVar('0',0).addInt( dt.date().dayOfWeek());
                } else if( ccmp(fmt,"daysInMonth") ) {
                    QDateTime dt=getDateTime(rst->get());
                    rst->setVar('0',0).addInt( dt.date().daysInMonth() );
                } else if( ccmp(fmt,"daysInYear") ) {
                    QDateTime dt=getDateTime(rst->get());
                    rst->setVar('0',0).addInt( dt.date().daysInYear() );
                } else if( slen(fmt) ) {
                    QString str = dateTimeString(rst->get(),fmt);
                    rst->reuse()->add(Q2A(str));
                }
            }
        }
    } break;
    case 34: { // isConnect
        LPCC addr=NULL;
        int port=80;
        if( cnt==0 ) {
            addr="www.google.com";
        } else {
            addr=AS(0);
            if( isNumberVar(arrs->get(1)) ) port=toInteger(arrs->get(1));
        }
        QTcpSocket conn;
        conn.connectToHost(addr, port);

        if( conn.waitForConnected(3000)) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }

    } break;
    case 35: { // beep
#ifdef WINDOW_USE
        if( cnt==0 ) {
            Beep(523,500);
        }
#endif
    } break;
    case 36: { // processCheck

    } break;
    case 37: { // sleep
        int tm=100;
        if( arrs && isNumberVar(arrs->get(0)) ) tm=toInteger(arrs->get(0));
#ifdef WINDOW_USE
        Sleep(tm);
#else
        usleep(tm*1000);
#endif
    } break;
    case 38: { // wait
		bool start=true;
		if( arrs ) {
			StrVar* sv=arrs->get(0);
			if( SVCHK('3',1) || SVCHK('3',0) ) {
				if( SVCHK('3',0) ) start=false;
			} else {
				LPCC val=toString(sv);
				if( ccmp(val,"end") || ccmp(val,"stop") || ccmp(val,"0") ) {
					start= false;
				}
			}
		}
		if( start ) {
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		} else {
			QApplication::restoreOverrideCursor();
		}

    } break;
    case 39: { // copyText
		if( arrs ) {
			LPCC text=AS(0);
			QClipboard *c = QApplication::clipboard();
			c->setText(KR(text));
		}
    } break;
    case 40: { // copyClipboard
        if( arrs ) {
            LPCC ty=AS(0);
            StrVar* sv=arrs->get(1);
            if(ccmp(ty,"image")) {
                if( SVCHK('i',7) ) {
                    QClipboard *c = QApplication::clipboard();
                    QImage image;
                    image.loadFromData((const uchar *)sv->get(8), sv->getInt(4));
                    c->setImage(image);
                }
            } else if(ccmp(ty,"text")) {
                QClipboard *c = QApplication::clipboard();
                // c->setMimeData()
                LPCC text=toString(sv);
                c->setText(KR(text));
            } else if(ccmp(ty,"file")) {
                if(SVCHK('a',0) ) {
                    XListArr* a=(XListArr*)SVO;
                    QList<QUrl> list;
                    for(int n=0;n<a->size(); n++) {
                        LPCC path=toString(a->get(n));
                        list.append(QUrl(KR(path)));
                    }
                    if(list.size()>0) {
                        QMimeData* mimeData = new QMimeData;
                        mimeData->setUrls(list);
                    }
                }
            }
        }
    } break;
    case 41: { // playWave
		if( cnt==1) {
		   LPCC path=AS(0);
		   QSound::play(KR(path) );
	   } else if( cnt==2 ) {

	   }
    } break;
    case 42: { // processInfo

    } break;
    case 43: { // kill
        if( cnt==1) {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                U32 pid=(U32)toInteger(sv);
                rst->fmt("taskkill /F /PID %d /T",pid);
            } else {
                LPCC name=AS(0);
                rst->fmt("taskkill /F /PID %s /T",name);

                rst->set("taskkill /F /IM ").add(name).add(" /T");

            }
            LPCC cmd=rst->get();
            if( slen(cmd) ) {
                QProcess::execute( KR(cmd) );
            }
        } else if( cnt==2 ) {

        }
    } break;
    case 44: { // worker

    } break;
    case 45: { // reboot
        if( arrs ) {
            // system("shutdown -r -t 0");
            system("shutdown -r -f -t 1");
        } else {
            SystemShutdown(2);
        }
    } break;
    case 46: { // shutdown
        if( arrs ) {
             system("shutdown -s -t 0");
        } else {
            SystemShutdown(0);
        }
    } break;
    case 47: { // shortcut

    } break;
    case 48: { // activatePage

    } break;
    case 49: { // version

    } break;
    case 50: { // cursorPos
		QPoint pt=QCursor::pos();
		rst->setVar('i',2).addInt(pt.x()).addInt(pt.y());
    } break;
    case 51: { // keyEventProc
#ifdef KEYHOOK_USE
        StrVar* sv=NULL;
        if( cnt==0 ) {
            sv=uom.getInfo()->gv("keyEventProc");
            rst->reuse()->add(sv);
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('3',1) ) {
            rst->setVar('3', setKeyEventProc()? 1 :0 );
        } else if( SVCHK('3',0) ) {
            rst->setVar('3', setKeyEventClose()? 1 :0 );
        } else if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, NULL);
            sv=uom.getInfo()->gv("keyEventProc");
            if( SVCHK('f',0) ) {
                XFuncNode* fnPrev=(XFuncNode*)SVO;
                fnPrev->clearNodeFlag(FLAG_PERSIST);
                funcNodeDelete(fnPrev);
                // gfns.deleteFuncNode(fnPrev);
            }
            fnCur->GetVar("keyArray")->setVar('a',0,(LPVOID)&gKeyArray);
            fnCur->GetVar("keyPrevTick")->setVar('1',0).addUL64(GetTickCount());
            uom.getInfo()->GetVar("keyEventProc")->setVar('f',0,(LPVOID)fnCur);
            rst->setVar('3', setKeyEventProc()? 1 :0 );
        }
#endif
    } break;
    case 52: { // desktopCapture
    } break;
    case 53: { // globalTimer
        if( cnt==0 ) {
            rst->setVar('3',uom.xtimer.isActive()?1:0);
        } else {
            StrVar* sv=arrs->get(0);
            if(SVCHK('3',1)) {
                uom.startTimer();
            } else if(SVCHK('3',0)) {
                uom.stopTimer();
            } else if(isNumberVar(sv)) {
                int delay=toInteger(sv);
                TreeNode* tn=uom.getInfo();
                tn->GetVar("@timerDelay")->setVar('0',0).addInt(delay);
                sv=arrs->get(1);
                if( SVCHK('f',1) ) {
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    XFuncNode* fnCur = setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("onTimeout"));
                    TreeNode* thisNode=NULL;
                    sv=arrs->get(2);
                    if(SVCHK('n',0)) {
                        thisNode=(TreeNode*)SVO;
                    }
                    if(thisNode) {
                        fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                    }
                    tn->GetVar("onTimeout")->setVar('f', 0, fnCur);
                }
                uom.startTimer();
            } else if(sv) {
                LPCC ty=toString(sv);
                if( ccmp(ty,"run") || ccmp(ty,"isRun") ) {
                    rst->setVar('3', uom.xtimer.isActive()? 1: 0);
                } else if( ccmp(ty,"start") ) {
                    uom.startTimer();
                } else if( ccmp(ty,"stop") ) {
                    uom.stopTimer();
                }
            }
        }
    } break;
    case 54: { // clipboard
		LPCC type=NULL;
        if( cnt==0 ) {
			type="text";
		} else {
			type=AS(0);
		}
		QClipboard *c = QApplication::clipboard();
		const QMimeData *md = c? c->mimeData(): NULL;
		if( md==NULL ) {
			return true;
		}

		qDebug("clipboard type==%s", type);
		if( ccmp(type,"text") ) {
			rst->set(Q2A(md->text()) );
		} else if( ccmp(type,"html") ) {
			rst->set(Q2A(md->html()) );
		} else if( ccmp(type,"image") ) {
			if( md->hasImage()) {
				QByteArray ba;
				QPixmap img = qvariant_cast<QPixmap>(md->imageData());
				QBuffer buffer( &ba );
				buffer.open( QIODevice::WriteOnly );
				img.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format
				if( img.isNull() ) {
					qDebug()<< "image is null";
				} else if(ba.size() ) {
					rst->setVar('i',7).addInt(ba.size()).add(ba.constData(),ba.size());
				}
			}
		} else if( ccmp(type,"hasText") ) {
			rst->setVar('3', md->hasText()? 1: 0);
		} else if( ccmp(type,"hasHtml") ) {
			rst->setVar('3', md->hasHtml()? 1: 0);
		} else if( ccmp(type,"hasImage") ) {
			rst->setVar('3', md->hasImage()? 1: 0);
		}
    } break;
    case 55: {  // watcherFile
        if(cnt==0 ) {
			return true;
        }
#ifdef WINDOW_USE
        TreeNode* cur=NULL;
        StrVar* sv=arrs->get(0);
        LPCC code=NULL;
        if(SVCHK('n',0)) {
            cur=(TreeNode*)SVO;
        } else {
            TreeNode* node=uom.getInfo()->addNode("@watcherFiles");
            code=AS(0);
            cur=node->addNode(code);
        }
        WatcherFile* watcher=NULL;
        sv=cur->GetVar("@watcher");
        if( SVCHK('t',21) ) {
            watcher=(WatcherFile*)SVO;
        } else {
            watcher=new WatcherFile(cur);
            sv->setVar('t',21,(LPVOID)watcher);
            if( slen(code)) cur->set("code",code);
        }
        sv=arrs->get(1);
        TreeNode* thisNode=NULL;
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(2);
        }
		if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode*fnCur = setCallbackFunction(fsrc, fn, cur, NULL, cur->GetVar("@callback"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)cur);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
		}
        rst->setVar('t',21,(LPVOID)watcher);
#endif
    } break;
	case 56: { // watcherClipboard
        if( cnt==0 ) {
            uom.connect( QApplication::clipboard(), SIGNAL(dataChanged()), &uom, SLOT(onChanageClipboard()) );
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('3',1) || SVCHK('3',0) ) {
            if( SVCHK('3',1)) {
                uom.connect( QApplication::clipboard(), SIGNAL(dataChanged()), &uom, SLOT(onChanageClipboard()) );
            } else {
                uom.disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), 0, 0 );
            }
            return true;
        }
        XFuncNode* pfn=NULL;
        TreeNode* thisNode=NULL;
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(1);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            pfn=setCallbackFunction(fsrc, fn, NULL, NULL, uom.getInfo()->GetVar("onChanageClipboard") );
            if( pfn ) {
                if(thisNode) pfn->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                uom.connect( QApplication::clipboard(), SIGNAL(dataChanged()), &uom, SLOT(onChanageClipboard()) );
            }
        }
        rst->setVar('3', pfn?1:0 );
	} break;
    default: break;
    }
    return false;
}


bool execConfigFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    StrVar* sv=NULL;
    int cnt=arrs? arrs->size(): 0;
    switch(fc->xfuncRef) {
    case 1: { // exec

    } break;
    case 2: { // get
        if( cnt==0 ) return true;
        LPCC code=AS(0);
        sv=uom.getInfo()->gv(code);
        rst->reuse()->add(sv);
    } break;
    case 3: { // set
        if( cnt==0 ) return true;
        int sp=0, ep=0;
        LPCC code=AS(0);
        if(arrs->get(1)) {
            StrVar* var=getStrVarPointer( arrs->get(1), &sp, &ep);
            sv=uom.getInfo()->GetVar(code);
            qDebug("... config set %s (%d,%d) ...\n", code, sp, ep);
            if(sv) {
                sv->reuse();
                if(sp<ep) sv->add(var, sp, ep);
            }
            rst->add(var, sp, ep);
        }
    } break;
    case 4: { // define
        if( cnt==0 ) return true;
        DbUtil* db=getConfDb();
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            db->select("select type,def,value from global_define");
            while(db->fetchNode()) {
                getHashFlagVal(db->get("type"), db->get("def"), (U32)toUL64(db->GetVar("value")) );
            }
            return true;
        }
        LPCC type = NULL, def = NULL;
        UL64 tm=0;
        U32 value=0, maxNum=0;
        bool update = false;
        if( cnt>3 ) {
            type = AS(0), def=AS(1);
            if( isNumberVar(arrs->get(2)) )
                value = (U32)toInteger(arrs->get(2));
            else {
                LPCC chk = AS(2);
                if( ccmp(chk,"max") ) maxNum = 1;
            }
            if( cnt==4 ) update = true;
        } else {
            LPCC code = AS(0);
            int pos=IndexOf(code,'.');
            if( pos==-1 ) {
                rst->reuse();
                if( db->prepare("select def,value from global_define where type=?") ) {
                    db->bindStr(code).exec();
                    while( db->fetchNode() ) {
                        rst->format(64,"[%s.%s=\t%s]\n", code, db->get("def"), toString(db->GetVar("value")) );
                    }
                }
                return true;
            } else {
                type = gBuf.add(code,pos), def=code+pos+1;
                if( cnt==1 ) {
                    rst->setVar('0',0).addInt( getHashFlagVal(type,def) );
                    return true;
                } else if( isNumberVar(arrs->get(1)) ) {
                    value = (U32)toInteger(arrs->get(1));
                } else {
                    maxNum=1;
                }
            }
        }

        if( tm==0 ) tm = (UL64)getTm();
        if( slen(type) && slen(def) ) {
            if( update || maxNum==0 ) {
                getHashFlagVal(type,def,(U32)value);
            }
            int findCnt = db->query("select count(1) from global_define where type=? and def=?").bindStr(type).bindStr(def).exec().fetchInt(0);
            if( findCnt>0 && maxNum>0 ) {
                maxNum = db->query("select max(value) from global_define where type=?").bindStr(type).exec().fetchInt(0);
                value = maxNum + 1;
                getHashFlagVal(type,def,(U32)value);
                update = true;
            } else if( maxNum>0 ) {
                value = maxNum;
            }
            if( findCnt==0 )
                db->query("insert into global_define (type,def,value,tm) values (?,?,?,?)").bindStr(type).bindStr(def).bindLong(value).bindLong(tm).exec();
            else if( update )
                db->query("update global_define set value=?,tm=? where type=? and def=?").bindLong(value).bindLong(tm).bindStr(type).bindStr(def).exec();
            rst->setVar('0',0).addInt(value);
        } else
            rst->setVar('0',0).addInt(0);
    } break;
    case 5: { // convert
        if( cnt==0 ) return true;
        LPCC fmt = toString(arrs->get(0));
        int cnt = arrs->size();
        if( cnt==1 ) {
            rst->add('\'').add(fmt).add('\'');
        } else if( ccmp(fmt,"percent") && cnt==3 ) {
            UL64 done=toUL64(arrs->get(1)), total=toUL64(arrs->get(2));
            if( total >0 )
                rst->set(Q2A(QString::number( double(done)*100/double(total), 'f', 1 )));
            else
                rst->set("0");
        } else if( ccmp(fmt,"eval") ) {
            StrVar* sv = arrs->get(1);
            XParseVar pv(sv);
            sv = arrs->get(2);
            makeTextData(pv, fn, rst, 1, SVCHK('n',0) ? (TreeNode*)SVO: NULL );
        } else if( ccmp(fmt,"decode") ) {
            StrVar* sv = arrs->get(1);
            if( sv ) {
                ZipVar zv(sv);
                if( zv.Decode() )
                    rst->add(sv);
            }
        } else if( ccmp(fmt,"encode") ) {
            StrVar* sv = arrs->get(1);
            if( sv ) {
                ZipVar zv(sv);
                if( zv.Encode() )
                    rst->add(sv);
            }
        }
    } break;
    case 6: { // printer
        if( cnt==0 ) {
            return true;
        }
    } break;
    case 7: { // include
        if( cnt==0 ) return true;
        LPCC fileName=AS(0);
        rst->reuse();
        if( slen(fileName) ) {
            bool ok=true;
            TreeNode* cf=cfNode("@includeNode");
            LPCC fileNameFull=getFullFileName(fileName);
            QFileInfo fi(KR(fileNameFull));
            if( fi.isFile() ) {
                UL64 modifyDate=(UL64)fi.lastModified().toLocalTime().toTime_t(), prev=cf->getLong(fileName);
                if( prev && modifyDate==prev) {
                    ok=false;
                }
                if( ok ) {
                    TreeNode* funcInfo=cfNode("@funcInfo");
                    LPCC ref=AS(1), baseCode=AS(2);
                    funcInfo->set("ref", ref);
                    funcInfo->set("baseCode", baseCode);
                    funcInfo->set("includeFileName", fileName);
                    funcInfo->setBool("@include", true);
                    includeBaro(fileNameFull);
                    funcInfo->setBool("@include", false);
                    cf->setLong(fileName, modifyDate);
                    rst->setVar('3',1);
                } else {
                    qDebug("#0#include %s already setting\n", fileNameFull);
                }
            } else {
                 qDebug("#0#include %s file not founc\n",fileNameFull);
            }
        }
    } break;
    case 8: { // var

    } break;
    case 9: {  // createWidget
        if( cnt==0 ) return true;
        StrVar* sv = arrs->get(0);
        rst->reuse();
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            LPCC tag=node->get("tag");
            if(slen(tag)==0) {
                tag="page";
                node->set("tag", tag);
            }
            QWidget* page=createWidget(tag, node, NULL);
            if(page) {
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 11: { // log
        XFuncNode* fnCur=fn;
        if( arrs ) {
            sv=arrs->get(0);
            if( SVCHK('f',0) ) fnCur=(XFuncNode*)SVO;
        }
        XFuncSrc* fsrc=fnCur?fnCur->getFuncSrc(): NULL;
        if( fsrc && fsrc->xfunc ) {
            StrVar log;
            fsrc->xfunc->printFunc(log,0);
            qDebug("#0#callScriptFunc:%s\n", log.get());
        }
    } break;
    case 12: { // env
        int cnt=arrs->size();
        LPCC type=cnt>0? AS(0):NULL, name=cnt>1? AS(1):NULL;
        bool overwrite=cnt>2? isVarTrue(arrs->get(2)): false;
        qDebug("system env %s %s", type, name);
        getSystemEnv(rst, type, name, overwrite);
    } break;
    case 13: { // newNode
        /*
        TreeNode* root=uom.getInfo()->addNode("@newNode");
        LPCC code=NULL;
        int hashNum=2;
        if( arrs) {
            code=AS(0);
            sv=arrs->get(1);
            if(isNumberVar(sv)) {
                hashNum=toInteger(sv);
            }
        } else {
            code="@main";
        }
        TreeNode* node=NULL; //
        if(slen(code) ) {
            LPCC newId=code; // gBuf.fmt("@newNode.%s",code);
            if(hashNum>2) {
                sv=root->gv(newId);
                if(SVCHK('n',0)) {
                    node=(TreeNode*)SVO;
                } else {
                    node=new TreeNode(hashNum, true);
                    root->GetVar(newId)->setVar('n',0,(LPVOID)node);
                }
            } else {
                node=root->addNode(newId);
            }
        }
        if(node) {
            rst->setVar('n',0,(LPVOID)node);
        }
        */
        TreeNode* node = new TreeNode(2, true);
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 20: { // newArray
        /*
        TreeNode* root=uom.getInfo()->addNode("@newArray");
        LPCC code=NULL;
        if( arrs) {
            code=AS(0);
        } else {
            code="@main";
        }
        XListArr* arr=NULL;
        if(slen(code) ) {
            arr=root->addArray(code);
        }
        if(arr) {
            rst->setVar('a',0,(LPVOID)arr);
        }
        */
        XListArr* arr = new XListArr(true);
        rst->setVar('a',0,(LPVOID)arr);
    } break;
    case 21: { // destoryNewObject
        if(cnt==0) {
            return true;
        }
    } break;
    case 22: { // call
        TreeNode* callNode=cfNode("@callVars");
        if( cnt==0 ) {
            rst->setVar('n',0,(LPVOID)callNode);
            return true;
        }
        int sp=0, ep=0;
        // src, inlineMode, debug
        XFuncNode* fnCur=fn;
        callNode->GetVar("@inlineMode")->reuse();
        for(int n=1; n<arrs->size(); n++) {
            sv=arrs->get(1);
            if(SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
            } else if(SVCHK('n',0) ) {
                callNode->GetVar("@thisNode")->setVar('n',0,SVO);
            } else if(SVCHK('3',1) || SVCHK('3',0) ) {
                callNode->GetVar("@inlineMode")->set(SVCHK('3',1)?"debug":"append");
            } else if(!isNumberVar(sv) && !SVCHK('9',0) ) {
                callNode->set("@inlineMode", toString(sv));
            }
        }
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        qDebug("Cf.call source %d, %d", sp, ep);
        if(sp<ep) {
            callScriptFunc(sv,sp,ep,fnCur);
        }
    } break;
    case 23: {	// postEvent
        if( cnt==0 ) return true;
        StrVar* sv=NULL;
        XFuncSrc* fsrc=NULL;
        rst->reuse();
        sv=arrs->get(0);
        if( SVCHK('n',0) || SVCHK('f',1) ) {
            TreeNode* thisNode=NULL;
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(1);
            }
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
            TreeNode* root=uom.getInfo();
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn, root, NULL, root->GetVar("@postEvent"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)root);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
            return true;
        }
        XFuncNode* fnCur=NULL;
        TreeNode* eventNode=NULL;
        LPCC type=AS(0);
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            eventNode=(TreeNode*)SVO;
        }
        sv=uom.getInfo()->GetVar("@postEvent");
        if(SVCHK('f',0) ) {
            fnCur=(XFuncNode*)SVO;
        } else {
            qDebug("poseEvent funcNode not defined [type:%s]", type);
        }
        if( eventNode && fnCur ) {
            eventNode->set("@eventType", type);
            UserEvent* ue=new UserEvent(eventNode,1,rst);
            ue->setCallback(fnCur);
            QCoreApplication::postEvent(&uom, ue);
        } else {
            qDebug("poseEvent event node not defined [type:%s]", type);
        }
    } break;
    case 24: { // map
    } break;
    case 25: { // sourceApply
        if(cnt==0 ) return true;
        int sp=0,ep=0;
        TreeNode* funcInfo=cfNode("@funcInfo");
        LPCC baseCode=NULL;
        StrVar* sv=arrs->get(1);
        if( sv) {
            baseCode=toString(sv);
        }
		bool bInc=false;
        U32 flags=0;
        if( slen(baseCode)) {
			sv=arrs->get(2);
            if(SVCHK('3',1)) {
                bInc=true;
                funcInfo->set("includeFileName", baseCode);
            } else {  
				funcInfo->set("baseCode", baseCode);
			}
        } else {
            funcInfo->set("baseCode", "sourceApply:test");
        }
		if( bInc==false ) {			
			if( arrs->get(2)) {
				flags|=0x2;
				sv=getStrVarPointer(arrs->get(2),&sp,&ep);
				if(sp<ep) funcInfo->GetVar("@inlineSource")->set(sv,sp,ep);
			}
			if( arrs->get(3)) {
				flags|=0x4;
				sv=getStrVarPointer(arrs->get(3),&sp,&ep);
				if(sp<ep) funcInfo->GetVar("@confSource")->set(sv,sp,ep);
			}
		}
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        XListArr* a=baroSourceApply(sv,sp,ep,rst);
        if(a ) {
            rst->setVar('a',0,(LPVOID)a);
        }
        funcInfo->GetVar("baseCode")->reuse();
		if( bInc) funcInfo->GetVar("includeFileName")->reuse(); 
        if( flags&0x2 ) funcInfo->GetVar("@inlineSource")->reuse();
        if( flags&0x4 ) funcInfo->GetVar("@confSource")->reuse();
    } break;
    case 26: { // sourceUpdate
        if(cnt==0 ) return true;
        int sp=0,ep=0;
        StrVar* sv=NULL;
        /*
        arrs->get(1);
        if( SVCHK('n',0) ) {
            TreeNode* page=(TreeNode*)SVO;
            sv=cfVar("@currentObject");
            if( sv ) {
                sv->setVar('n',0,(LPVOID)page);
            }
        }
        */
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        qDebug("#0#sourceUpdate\n%s\nsource size(%d,%d)\n", sv->get(sp), sp, ep);
        rst->setVar('3',baroSourceUpdate(sv,sp,ep,rst) ?1:0);
    } break;
    case 27: { // sourceCanvas
        if(cnt==0 ) return true;
        LPCC pageId=AS(0), canvasId=AS(1);
        int sp=0,ep=0;
        StrVar* sv=getStrVarPointer(arrs->get(3),&sp,&ep);
        LPCC target=slen(pageId)?FMT("page=%s",pageId):"";
        LPCC id=slen(canvasId)?FMT("id=%s",canvasId):"canvas";
        rst->add(FMT("<canvas %s %s>",target,id)).add(sv,sp,ep).add("</canvas>");
        bool ret=baroSourceUpdate(rst, 0, rst->length(),NULL);
        rst->setVar('3',ret ?1:0);

    } break;
    case 28: { // debug
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            LPCC logfile=uom.getInfo()->get("debugLogPath");
            rst->add(FMT("LogFile:%s, LogFlag:0x%X", logfile, uom.getInfo()->xflag));
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('3',1) ) {
            uom.getInfo()->setNodeFlag(CNF_DEBUG);
#ifdef WINDOW_USE
            if(uom.xconsole) uom.xconsole->Show(true);
#endif
        } else if( SVCHK('3',0) ) {
            uom.getInfo()->clearNodeFlag(CNF_DEBUG );
#ifdef WINDOW_USE
            if(uom.xconsole) uom.xconsole->Show(false);
#endif
        } else {
#ifdef WINDOW_USE
            LPCC ty=toString(sv);
            if( ccmp(ty,"clear") ) {
                if(uom.xconsole) uom.xconsole->Clear();
            }
#endif
        }
        if( cnt>1 ) {
            // debug(true, [console or "logfile"], query );
            sv=arrs->get(1);
            if( SVCHK('3',1) || SVCHK('3',0) ) {
                if( SVCHK('3',1) ) {
                    uom.getInfo()->setNodeFlag(CNF_OUTPUT_QUERY);
                } else if( SVCHK('3',0) ) {
                    uom.getInfo()->clearNodeFlag(CNF_OUTPUT_QUERY);
                }
            } else {
                LPCC logPath=uom.getInfo()->get("debugLogPath");
                if( slen(logPath)==0 ) {
                    logPath=getFullFileName(toString(sv));
                    QFileInfo fi(KR(logPath) );
                    if( fi.isDir() ) {
                        uom.getInfo()->setNodeFlag(CNF_LOGFILE);
                        uom.getInfo()->set("debugLogPath", logPath);
                    } else {
                        qDebug("#9#log path is not folder (path:%s)",logPath);
                    }
                } else {
                    qDebug("#0#log path already set (path:%s)",logPath);
                }
            }
            sv=arrs->get(2);
            if( SVCHK('3',1) || SVCHK('3',0) ) {
                if( SVCHK('3',1) ) {
                     uom.getInfo()->setNodeFlag(CNF_LOGFILE);
                } else {
                    uom.getInfo()->clearNodeFlag(CNF_LOGFILE);
                }
            }
        }
    } break;
    case 29: { // sourceCall
        if( cnt==0 ) {
            callScriptFunc(NULL,0,0);
            sv=cfVar("@scriptFuncNode");
            if( SVCHK('f',0) ) {
                rst->reuse()->add(sv);
            }
            return true;
        }
        int sp=0, ep=0;
        // src, inlineMode, debug
        sv=arrs->get(1);
        if(SVCHK('3',1)) {
            cfVar("@inlineMode")->set("append");
        } else {
            cfVar("@inlineMode")->reuse();
        }
        sv=arrs->get(2);
        if(SVCHK('3',1)) {
            TreeNode* callNode=cfNode("@callVars");
            if(callNode) {
                callNode->set("debug", "true");
            }
        }
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        if(sp<ep) {
            callScriptFunc(sv,sp,ep);
        }
    } break;
    case 30: { // idle
    } break;
    case 31: { // loadPage
    } break;
    case 32: { // clipText        
    } break;
    case 33: { // clipImage

    } break;
    case 34: { // loadImage
    } break;
    case 35: { // loadSvg

    } break;
    case 41: { // exit
        TreeNode* info=uom.getInfo();
        qApp->closeAllWindows();
        SAFE_DELETE(info);
        qApp->exit();
    } break;
    case 42: { // selectFile
        LPCC title = AS(0);
        LPCC path =AS(1);
        LPCC filter = AS(2);
        QString filterString=QString();
        if(slen(filter) ) filterString=KR(filter);
        QStringList files=QFileDialog::getOpenFileNames(NULL,
            slen(title) ? KR(title): QString("File Select"),
            slen(path) ? KR(path): QDir::currentPath(),
            filterString );
        if( files.size()==1 ) {
            rst->set(Q2A(files.at(0)));
        } else if( files.size()>1 ) {
            XListArr* a=getListArrTemp();
            for( int n=0,sz=files.size();n<sz;n++) {
                a->add()->set(Q2A(files.at(n)));
            }
            rst->setVar('a',0,(LPVOID)a);
        }
    } break;
    case 43: { // selectFolder
        if( cnt==0 ) return true;
        LPCC title = AS(0);
        LPCC path =  AS(1);
        qDebug("select folder path==%s\n", path);
        QFileDialog *fd = new QFileDialog(NULL,
            slen(title) ? KR(title) : QString("Folder Select"),
            path? KR(path): QDir::currentPath()
        );
        QTreeView *tree = fd->findChild <QTreeView*>();
        if(tree) {
            tree->setRootIsDecorated(true);
            tree->setItemsExpandable(true);
        }
        qDebug("slect folder ==========1\n");
        fd->setFileMode(QFileDialog::Directory);
        fd->setOption(QFileDialog::ShowDirsOnly);
        fd->setViewMode(QFileDialog::Detail);
        if( isVarTrue(arrs->get(2)) )
            fd->setWindowFlags( fd->windowFlags()|Qt::WindowStaysOnTopHint);
        if( fd->exec() ) {
            QString directory = fd->selectedFiles()[0];
            rst->set(Q2A(directory));
        }
    } break;
    case 51: { // readSetting
        if( cnt==0 ) return false;
        // type, code, key, def
    } break;
    case 52: { // writeSetting

    } break;
    case 53: { // handshakeKey
        if( cnt==0 ) {
            return true;
        }
        QByteArray  ba;
        LPCC key=AS(0);
        rst->add(key); //.add("258EAFA5-E914-47DA-95CA-C50BARO85B11");
        ba.append(rst->get(), rst->length());
        QByteArray sh1=QCryptographicHash::hash(ba,QCryptographicHash::Sha1).toBase64();
        rst->set(sh1.constData(), sh1.length() );
    } break;
    case 54: { // info
        if( cnt==0 ) {
            sv =fn->gv("@this");
            if( SVCHK('n',0) ) {
                TreeNode* tn=(TreeNode*)SVO;
                LPCC tag=tn->get("tag");
				LPCC id=tn->get("id");
                rst->add("this=>").add(tag).add('.').add(id);
            }
            return true;
        }
        XFuncNode* fnCur = NULL;
        LPCC ty = NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* tn = (TreeNode*)SVO;
            sv = tn->gv("onInit");
            if( SVCHK('f',0) ) {
                fnCur = (XFuncNode*)SVO;
            }
            ty=AS(1);
            if( slen(ty)==0 ) ty="value";
        } else if( SVCHK('f',0) ) {
            fnCur = (XFuncNode*)SVO;
            ty=AS(1);
            if( slen(ty)==0 ) ty="value";
        } else {
            ty=toString(sv);
            if( ccmp(ty,"funcVar") ) {
                LPCC kind = NULL;
                sv = arrs->get(1);
                if( SVCHK('n',0) ) {
                    TreeNode* tn = (TreeNode*)SVO;
                    kind = AS(2);
                    if( slen(kind)==0 ) kind="init";
                    if( ccmp(kind,"init") ) {
                        sv = tn->gv("onInit");
                    } else {
                        sv = tn->gv(kind);
                    }
                    if( SVCHK('f',0) ) {
                        fnCur = (XFuncNode*)SVO;
                    }
                } else {
                    fnCur = fn;
                }
                if( fnCur==NULL )
                    return false;

                LPCC cd = AS(3);
                if( slen(cd) ) {
                    rst->reuse()->add(fnCur->gv(cd));
                    return true;
                }
            }
        }
        if( fnCur ) {
            for(WBoxNode* bn=fnCur->First(); bn; bn = fnCur->Next()) {
                LPCC code = fnCur->getCurCode();
                sv = &(bn->value);
                rst->add(code).add("=");
                if( ccmp(ty,"value") ) {
                    rst->add(toString(sv)).add("\r\n");
                    continue;
                }
                char ch = sv->charAt(0);
                if( ch==0 ) rst->add("null\r\n");
                else if( ch!='\b' ) rst->add("string\r\n");
                else if( SVCHK('f',0) || SVCHK('f',1) ) rst->add("function\r\n");
                else if( SVCHK('n',0) ) {
                    TreeNode* node=(TreeNode*)SVO;
                    {
                        rst->add("node");
                        switch( node->xstat ) {
                        case FNSTATE_DRAW:			rst->add("(draw)"); break;
                        case FNSTATE_DB:			rst->add("(db)"); break;
                        case FNSTATE_MODEL:			rst->add("(dataModel)"); break;
                        case FNSTATE_FILE:			rst->add("(file)"); break;
                        case FNSTATE_FILEFIND:		rst->add("(fileFind)"); break;
                        case FNSTATE_PROCESS:		rst->add("(process)"); break;
                        case FNSTATE_JOB:			rst->add("(job)"); break;
                        case FNSTATE_CRON:			rst->add("(cron)"); break;
                        case FNSTATE_WORKER:		rst->add("(worker)"); break;
                        case FNSTATE_WEBCLIENT:		rst->add("(web)"); break;
                        case FNSTATE_WEBSERVER:		rst->add("(webServer)"); break;
                        case FNSTATE_EXCEL:			rst->add("(excel)"); break;
                        case FNSTATE_AUDIO:			rst->add("(audio)"); break;
                        case FNSTATE_GOOGLE:		rst->add("(google)"); break;
                        case FNSTATE_CANVASITEM:	rst->add("(drawItem)"); break;
                        case FNSTATE_WATCHER:		rst->add("(watcher)"); break;
                        case FNSTATE_PDF:			rst->add("(pdf)"); break;
                        case FNSTATE_ZIP:			rst->add("(zip)"); break;
                        case FNSTATE_SOCKET:		rst->add("(socket)"); break;
#ifdef WIDGET_USE
                        case FNSTATE_ACTION:		rst->add("(action)"); break;
                        case FNSTATE_MENU:			rst->add("(menu)"); break;
#endif
                        default: break;
                        }
                        rst->add("\r\n");
                    }
                }
                else if( SVCHK('a',0) ) rst->add("array\r\n");
                else if( SVCHK('s',0) ) rst->add("string ref\r\n");
                else if( SVCHK('9',0) ) rst->add("null\r\n");
                else if( SVCHK('3',1) ) rst->add("bool(true)\r\n");
                else if( SVCHK('3',0) ) rst->add("bool(false)\r\n");
                else if( SVCHK('0',0) || SVCHK('0',1) || SVCHK('1',0) || SVCHK('2',0) || SVCHK('4',0) ) rst->add("number\r\n");
                else if( SVCHK('i',4) ) rst->add("rect\r\n");
                else if( SVCHK('i',2) ) rst->add("point\r\n");
                else if( SVCHK('i',3) ) rst->add("color\r\n");
                else rst->add("common object\n");
            }
        }
    } break;
    case 55: { // getObject
        TreeNode* tree=NULL;
        if( cnt==0 ) {
            tree=getTreeNode(NULL,NULL);
            if( tree ) rst->setVar('n',0,(LPVOID)tree);
            return true;
        }
        LPCC tag=AS(0), name=AS(1);
        sv=arrs->get(2);
        tree=getTreeNode(tag,name,SVCHK('3',1) );
        if( tree ) rst->setVar('n',0,(LPVOID)tree);
    } break;
    case 56: { // recvData

    } break;
    case 57: { // class

    } break;
    case 58: { // funcNode	// version 1.0.3
        if( cnt==0 ) {
            rst->setVar('f',0,(LPVOID)fn);
            return true;
        }
        XFuncNode* fnTarget=NULL;
        sv=arrs->get(0);
        if(SVCHK('n',0)) {
            TreeNode* thisNode=(TreeNode*)SVO;
            rst->reuse();
            if( thisNode ) {
                sv=thisNode->gv("onInit");
                if( SVCHK('f',0) ) {
                    fnTarget=(XFuncNode*)SVO;
                    sv=arrs->get(1);
                    if(SVCHK('3',1)) {
                        for(WBoxNode* bn=thisNode->First(); bn; bn=thisNode->Next()) {
                            sv=&(bn->value);
                            if( !SVCHK('f',0) ) continue;
                            LPCC fnm=thisNode->getCurCode();
                            if(!ccmp(fnm,"onInit") ) {
                                XFuncNode* fnCur=(XFuncNode*)SVO;
                                fnCur->xparent=fnTarget;
                            }
                        }
                    } else if(SVCHK('f',1)) {
                        XFuncSrc* fsrc=(XFuncSrc*)SVO;
                        XFunc* fcCur=fsrc->xfunc;
                        XFuncNode* fnCallback=gfns.getFuncNode(fcCur,fnTarget);
                        fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                        fnCallback->call(fcCur);
                        for(WBoxNode* bn=fnCallback->First(); bn ; bn=fnCallback->Next() ) {
                            LPCC varNm=fnCallback->getCurCode();
                            if( varNm[0]=='@' ) continue;
                            fnTarget->GetVar(varNm)->set(bn->value );
                        }
                        funcNodeDelete(fnCallback);
                    }
                    rst->setVar('f',0,(LPVOID)fnTarget);
                } else {
                    sv=arrs->get(1);
                    if(SVCHK('f',1)) {
                        XFuncSrc* fsrc=(XFuncSrc*)SVO;
                        XFunc* fcCur=fsrc->xfunc;
                        XFuncNode* fnCallback=gfns.getFuncNode(fcCur,NULL);
                        fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                        fnCallback->setNodeFlag(FLAG_PERSIST);
                        // fnCallback->call(fcCur);
                        thisNode->GetVar("onInit")->setVar('f',0,(LPVOID)fnCallback);
                        for(WBoxNode* bn=thisNode->First(); bn; bn=thisNode->Next()) {
                            sv=&(bn->value);
                            if( !SVCHK('f',0) ) continue;
                            LPCC fnm=thisNode->getCurCode();
                            if(!ccmp(fnm,"onInit") ) {
                                XFuncNode* fnCur=(XFuncNode*)SVO;
                                fnCur->xparent=fnTarget;
                            }
                        }
                        rst->setVar('f',0,(LPVOID)fnCallback);
                    }
                }
            }
        } else if(SVCHK('f',1)) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnInit = NULL;
            TreeNode* thisNode=NULL;
            sv=arrs->get(1);
            if(!SVCHK('n',0)) {
                sv=fn->gv("@this");
            }
            if(SVCHK('n',0)) {
                thisNode = (TreeNode*)SVO;
                sv=thisNode->gv("onInit");
                if( SVCHK('f',0) ) {
                    fnInit=(XFuncNode*)SVO;
                }
            }
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc,fnInit);
            if( thisNode ) {
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                fnCur->setNodeFlag(FLAG_PERSIST);
                thisNode->setNodeFlag(FLAG_SETEVENT);
            }
            rst->setVar('f',0,(LPVOID)fnCur);
        } else {
            LPCC type=AS(0);
            rst->reuse();
            if( ccmp(type,"parent") ) {
                fnTarget=fn->parent();
            } else if( ccmp(type,"pp") ) {
                fnTarget=fn->parent();
                if( fnTarget && fnTarget->parent() ) {
                    fnTarget=fnTarget->parent();
                }
            } else if( ccmp(type,"onInit") ) {
                TreeNode* thisNode=NULL;
                sv=arrs->get(1);
                if( SVCHK('n',0) ) {
                    thisNode=(TreeNode*)SVO;
                } else {
                    sv=fn->gv("@this");
                    if( SVCHK('n',0) ) {
                        thisNode=(TreeNode*)SVO;
                    }
                }
                if( thisNode ) {
                    sv=thisNode->gv("onInit");
                    if( SVCHK('f',0) ) {
                        fnTarget=(XFuncNode*)SVO;
                    }
                }
            }
        }
        if( fnTarget ) {
            rst->setVar('f',0,(LPVOID)fnTarget);
        }
    } break;
    case 59: { // funcParam
        if( cnt==0 ) return true;
        sv=arrs->get(0);
        rst->reuse();
        XFuncSrc* fsrc=NULL;
        if( SVCHK('f',0) ) {
            XFuncNode* fnCur=(XFuncNode*)SVO;
            if( fnCur->xfunc ) {
                fsrc=fnCur->xfunc->getFuncSrc();
            }
        } else if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
        }
        if( fsrc ) {
            StrVar* param=&(fsrc->xparam);
            rst->add(param);
        }
    } break;
    case 60: { // pageNode
#ifdef WIDGET_USE
        TreeNode* thisNode=NULL;
        TreeNode* pageNode=NULL;
        sv=fn->gv("@this");
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
        }
        if( arrs ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            }
        }
        pageNode=findPageNode(thisNode);
        if( pageNode ) {
            rst->setVar('n',0,(LPVOID)pageNode);
        }
#endif
    } break;
    case 61: { // timeLine

    } break;
    case 62: { // automata
        sv=uom.getInfo()->GetVar("@automata");
        CKoreanAutomata* automata=NULL;
        if( SVCHK('a',11) ) {
            automata=(CKoreanAutomata*)SVO;
        } else {
            automata=new CKoreanAutomata();
            sv->setVar('a',11,(LPVOID)automata);
        }
        rst->setVar('a',11,automata);
    } break;
    case 63: { // encode
        if( cnt==0 ) return true;
        XFuncSrc fsrc;
        int sp=0,ep=0;
        sv=getStrVarPointer(arrs->get(0),&sp,&ep);
        fsrc.readBuffer(sv, sp, ep);
        rst->add((StrVar*)&fsrc);
        if( rst->length() ) {
            ZipVar z(rst);
            z.Encode();
        }
    } break;
    case 64: { // addFunc
        if( cnt==0 ) return true;
        int sp=0, ep=0;
        StrVar* var=getStrVarPointer(arrs->get(0), &sp, &ep);
        if( sp<ep ) {
			TreeNode* funcInfo=NULL;
			LPCC ref=AS(1);
			LPCC funcLine=AS(2);
			if(slen(ref)) {
				 funcInfo=cfNode("@funcInfo");
				 if(funcInfo) funcInfo->set("refName", ref);
			}
            if( slen(funcLine)==0 ) funcLine="common";
            setFuncSource(var, sp, ep, funcLine);
			if(funcInfo) funcInfo->set("refName", "");
        }
    } break;
    case 65: { // addEvent
        if( cnt==0 ) return true;
        int sp=0, ep=0;
        TreeNode* node=NULL;
        StrVar* var=NULL;
        LPCC funcLine=NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            node=(TreeNode*)SVO;
            var=getStrVarPointer(arrs->get(1), &sp, &ep);
            funcLine=AS(2);
        } else {
            node=uom.getInfo();
            var=getStrVarPointer(arrs->get(0), &sp, &ep);
            funcLine=AS(1);
        }
        if( node) {
            if( slen(funcLine)==0 ) funcLine="common";
            if( setModifyClassFunc(node, var, sp, ep, fn, rst, false, funcLine) ) {
                // node->setNodeFlag(FLAG_PERSIST);
                return true;
            }
        }
    } break;
    case 66: { // direction
        if( cnt==0 ) return true;
        XListArr* result=uom.getInfo()->addArray("@direction", true);
        sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            StrVar* svStart=NULL;
            StrVar* svPrev=NULL;
            int sameCount=0;
            float base=0.3f;
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                base=(float)toDouble(sv);
            }
            sv=NULL;
            U32 prevFlag=0;
            for( int n=0,sz=a->size(); n<sz ; n++ ) {
                sv=a->get(n);
                if( !SVCHK('i',2) ) continue;
                if( svPrev==NULL ) {
                    svPrev=sv;
                    svStart=sv;
                    continue;
                }
                U32 flag=getDirection(svPrev, sv, base);
                if( flag==0 ) {
                    continue;
                }
                if( prevFlag==0 ) {
                    prevFlag=flag;
                } else if( prevFlag!=flag ) {
                    if( getDirection(svStart, sv, base)==flag || sameCount>5 ) {
                        sameCount=0;
                        svStart=sv;
                        result->add()->add(getDirectionValue(prevFlag, rst));
                        prevFlag=flag;
                    } else {
                        sameCount++;
                        continue;
                    }
                } else {
                    sameCount++;
                }
                svPrev=sv;
            }
            if( sameCount>3 ) {
                result->add()->add(getDirectionValue(prevFlag, rst));
            }
        }
        rst->setVar('a',0,(LPVOID)result);
    } break;
    case 67: { // imageLoad
        if( cnt==0 ) return true;
        StrVar* sv=arrs->get(1);
        bool alpha=sv && SVCHK('3',1);
        QPixmap* pm=getStrVarPixmap(arrs->get(0),alpha);
        if(pm) {
            rst->setVar('i',6,(LPVOID)pm);
        }
    } break;
    case 68: { // jsValue
        if( cnt==0 ) return true;
		int sp=0, ep=0;
        StrVar* sv=arrs->get(0);
        StrVar* var=NULL;
        if(sv->charAt(0)=='\b' && sv->charAt(1)!='s' ) {
            var=getStrVarTemp();
            toString(sv, var);
            ep=var->length();
        }
        if( var==NULL ) {
            var=getStrVarPointer(sv, &sp, &ep);
        }
        /*
        sv=arrs->get(1);
        bool skip=SVCHK('3',1);
        */
        rst->reuse();
        sv=arrs->get(1);
        if(SVCHK('3',1)) {
            rst->add('\'');
            if(sp<ep) jsValueString(var, sp, ep, rst);
            rst->add('\'');
        } else {
            rst->add('\"');
            if(sp<ep) jsValueString(var, sp, ep, rst);
            rst->add('\"');
        }
    } break;
    case 69: { // val
        if( cnt==0 ) return true;
        rst->reuse();
        for(int n=0,sz=arrs->size();n<sz; n++) {
            rst->add(toString(arrs->get(n)));
        }
    } break;
    case 70: { // error
        if( cnt==0 ) {
            uom.getInfo()->clearNodeFlag(CNF_FUNC_LOG);
            rst->add(cfVar("@errorMessage"));
        } else {
            sv=arrs->get(0);
            if( SVCHK('3',1) ) {
                uom.getInfo()->setNodeFlag(CNF_FUNC_LOG);
                cfVar("@errorMessage",true);
            }
        }
    } break;
    case 71: { // confInfo
        TreeNode* cf=NULL;
        sv=confVar(NULL);
        if( SVCHK('n',0) ) {
            cf=(TreeNode*)SVO;
        }
        if( cf==NULL ) {
            qDebug("#9#confInfo error (object not define)");
            return true;
        }
        if( cnt==0 ) {
            rst->setVar('n',0,(LPVOID)cf);
        } else {
            StrVar* var=NULL;
            LPCC code=AS(0);
            rst->reuse();
            if( slen(code)==0 ) {
                qDebug("#9#confInfo error (code not define)");
                return true;
            }
            if( code[0]=='#' ) {
                var=confVar(code);
                if( var ) rst->add(var);
                return true;
            }
            sv=arrs->get(1);
            if( sv ) {
                int sp=0, ep=0;
                var=getStrVarPointer(sv, &sp, &ep);
                cf->GetVar(code)->set(var,sp,ep);
            } else {
                sv=cf->gv(code);
                if( sv ) {
                    int sp=0, ep=0;
                    var=getStrVarPointer(sv, &sp, &ep);
                    rst->add(var,sp,ep);
                }
            }
        }
    } break;
    case 72: { // classCheck

    } break;
    case 73: { // toData
        if( cnt==0 ) return true;
        sv = arrs->get(0);
        if( SVCHK('i',7) ) {
            // sv->get(FUNC_HEADER_POS+4)
            rst->add((LPCC)sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
        } else {

        }
    } break;
    case 74: { // toBinary
        rst->reuse();
        if( cnt==0 ) {
            return true;
        }
        sv = arrs->get(0);
        if( SVCHK('i',6) || SVCHK('i',9) ) {
            QByteArray ba;
            QBuffer buffer( &ba );
            LPCC ext=toString(arrs->get(1));
            if(slen(ext)==0 ) ext="PNG";
            buffer.open( QIODevice::WriteOnly );
            if( SVCHK('i',6) ) {
                QPixmap* pix = (QPixmap*)SVO;
                if( pix ) {
                    pix->save( &buffer, ext);
                }
            } else if( SVCHK('i',9) ) {
                QImage* pix = (QImage*)SVO;
                if( pix ) {
                    pix->save( &buffer, ext);
                }
            }
            if( ba.size() ) {
                rst->setVar('i',7).addInt(ba.size()).add(ba.constData(), ba.size());
            }
        } else if( SVCHK('i',7) ) {
            rst->add(sv);
        } else if( sv ) {
            int sp=0, ep=0;
            sv=getStrVarPointer(sv,&sp,&ep);
            int len=ep-sp;
            if( len >0 ) {
                rst->setVar('i',7).addInt(len).add(sv->get(sp),len);
            }
        }
    } break;
    case 75: { // node
        TreeNode* node=getTreeNodeTemp();
        if( node ) {
            rst->setVar('n',0,(LPVOID)node);
        } else {
            qDebug("#9#common node error\n");
        }
    } break;
    case 76: { // array
        XListArr* arr=getListArrTemp();
        if( arr ) {
            rst->setVar('a',0,(LPVOID)arr);
        } else {
            qDebug("#9#Common array error\n");
        }
    } break;
    case 77: { // rootNode
        if(cnt==0) {
            rst->setVar('n',0,(LPVOID)uom.getInfo() );
            return true;
        }
        int cnt=arrs->size();
        LPCC code=AS(0);
        rst->reuse();
        if(cnt==1) {
            rst->add(uom.getInfo()->gv(code));
        } else {
            sv=uom.getInfo()->GetVar(code);
            if( SVCHK('n',0)) {
                if(isVarTrue(arrs->get(2)) ) {
                    sv=arrs->get(1);
                    if(SVCHK('n',0)) {
                        TreeNode* node=(TreeNode*)node;
                        if(node) uom.getInfo()->GetVar(code)->setVar('n',0,(LPVOID)node);
                    }
                    rst->setVar('3',1);
                }
            } else {
                int sp=0, ep=0;
                StrVar* var=getStrVarPointer(arrs->get(1),&sp,&ep);
                sv->reuse();
                if(sp<ep) {
                    sv->add(var,sp,ep);
                    rst->setVar('3',1);
                }
            }
        }
    } break;
    case 78: { // ref
        StrVar* var=uom.getStrVar();
        int sp=0, ep=0;
        if( var==NULL ) {
            qDebug("#9#CF.ref() error");
            return true;
        }
        if( arrs ) {
            sv=getStrVarPointer( arrs->get(0), &sp, &ep);
            var->add(sv, sp, ep);
            sp=0, ep=var->length();
        }
        rst->setVar('s',0,(LPVOID)var).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
    } break;
    case 79: { // getColor
#ifdef WIDGET_USE
        QColor prev;
        TreeNode* node=NULL;
        if( arrs) {
            prev=getQColor(arrs->get(0));
            sv=arrs->get(1);
            if( SVCHK('n',0)) {
                node=(TreeNode*)SVO;
            }
        }
        QColor clr=QColorDialog::getColor(prev,gwidget(node));
        rst->setVar('i',3).addInt(clr.red()).addInt(clr.green()).addInt(clr.blue());
#endif
    } break;
    case 80: {  // setEvent
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* target=(TreeNode*)SVO;
            LPCC eventName=AS(1);
            qDebug("setEvent start %s", eventName);
            sv=arrs->get(2);
            if(slen(eventName) && SVCHK('f',1)) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnParent=NULL;
                TreeNode* page=NULL; // findPageNode(target);
                if( ccmp(eventName,"onInit") && target!=page ) {
                    if(page) {
                        sv=page->gv("onInit");
                        if( SVCHK('f',0) ) {
                            fnParent=(XFuncNode*)SVO;
                        }
                    }
                }
                if(fnParent==NULL ){
                    sv=target->gv("onInit");
                    if( SVCHK('f',0) ) {
                        fnParent=(XFuncNode*)SVO;
                    } else if(page) {
                        sv=page->gv("onInit");
                        if( SVCHK('f',0) ) {
                            fnParent=(XFuncNode*)SVO;
                        }
                    }
                }
                XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fnParent);
                if(page) fnCur->GetVar("@page")->setVar('n',0,(LPVOID)page);
                // fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)target);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)target);
                fnCur->setNode(FLAG_PERSIST);
                target->GetVar(eventName)->setVar('f',0,(LPVOID)fnCur );
                qDebug("setEvent %s ok", eventName);
            }
        }
    } break;
    case 81: {  // setFunc
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* target=(TreeNode*)SVO;
            LPCC funcName=AS(1);
            sv=arrs->get(2);
            if(slen(funcName) && SVCHK('f',1)) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                target->GetVar(funcName)->setVar('f',1,(LPVOID)fsrc);
            }
        }
    } break;
    case 82: {  // format
        if(cnt==0) {
            return true;
        }
        StrVar* var=cfVar("formatVar", true);
        var->setVar('a',0,(LPVOID)arrs);
        parseTextFormat(arrs->get(0),0,0,rst,var);
    } break;
    default: return false;
    }
    return true;
}

bool execMathFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;
    StrVar* sv= NULL;
    switch(fc->xfuncRef) {
    case 1: { // random
        double fMin=0.0, fMax=1.0;
        if( arrs ) {
            cnt=arrs->size();
            if( cnt==1 && isNumberVar(arrs->get(0)) ) {
                fMax=toDouble(arrs->get(0));
            } else {
                fMin=toDouble(arrs->get(0));
                if( isNumberVar(arrs->get(1)) ) {
                    fMax=toDouble(arrs->get(1));
                }
            }
        }
        double f = (double)rand() / RAND_MAX;
        rst->setVar('4',0).addDouble( fMin + f * (fMax - fMin) );
    } break;
    case 2: { // sin
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( sin(x) );
    } break;
    case 3: { // cos
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( cos(x) );
    } break;
    case 4: { // pi
        rst->setVar('4',0).addDouble( M_PI );
    } break;

    case 5: { // tan
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( tan(x) );
    } break;
    case 6: { // atan
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( atan(x) );
    } break;
    case 7: { // exp
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( exp(x) );
    } break;
    case 8: { // log
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double x=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( log(x) );
    } break;
	case 9: { // acos
        if( cnt==0 ) {
			rst->setVar('4',0).addDouble(0);
			return true;
		}
		double x=toDouble(arrs->get(0));
		rst->setVar('4',0).addDouble( acos(x) );
	} break;
	case 10: { // asin
        if( cnt==0 ) {
			rst->setVar('4',0).addDouble(0);
			return true;
		}
		double x=toDouble(arrs->get(0));
		rst->setVar('4',0).addDouble( asin(x) );
	} break;
	case 21: { // abs
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('4',0) ) {
            rst->setVar('4',0).addDouble( abs(toDouble(sv)) );
        } else if( SVCHK('1',0) ) {
            rst->setVar('1',0).addUL64( labs(toUL64(sv)) );
        } else {
            rst->setVar('0',0).addInt( abs(toInteger(sv)) );
        }
    } break;
    case 22: { // mod
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        sv=arrs->get(0);
        if( SVCHK('4',0) ) {
            double div=toDouble(arrs->get(1));
            if( div>0.0 ) rst->setVar('4',0).addDouble( fmod(toDouble(sv), div) );
        } else if( SVCHK('1',0) ) {
            UL64 div=toUL64(arrs->get(1));
            if( div>0 ) {
                rst->setVar('1',0).addUL64(toUL64(sv)%div);
            }
        } else {
            int div=toInteger(arrs->get(1));
            if( div>0 ) {
                rst->setVar('0',0).addInt(toInteger(sv)%div);
            }
        }
    } break;
    case 23: { // pow
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double a=toDouble(arrs->get(0)), b=toDouble(arrs->get(1));
        rst->setVar('4',0).addDouble( pow(a,b) );
    } break;
    case 24: { // sqrt
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double a=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( sqrt(a) );
    } break;
    case 25: { // round
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        int cnt=arrs->size();
        double a=toDouble(arrs->get(0));
        if( cnt==1 ) {
            rst->setVar('0',0).addInt((int)a);
        } else {
            rst->setVar('4',0).addDouble( rounding(a, toInteger(arrs->get(1))) );
        }
    } break;
    case 26: { // floor
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double a=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( floor(a) );
    } break;
    case 27: { // ceil
        if( cnt==0 ) {
            rst->setVar('4',0).addDouble(0);
            return true;
        }
        double a=toDouble(arrs->get(0));
        rst->setVar('4',0).addDouble( ceil(a) );
    } break;
    case 28: { // distance
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        int cnt=arrs->size();
        int sz=sizeof(double);
        double x1=0, x2=0, y1=0, y2=0;
        if(cnt==2) {
            sv=arrs->get(0);
            if( SVCHK('i',20)) {
                x1=sv->getDouble(4), y1=sv->getDouble(4+sz);
            } else if( SVCHK('i',2)) {
                x1=sv->getInt(4), y1=sv->getInt(8);
            }
            sv=arrs->get(1);
            if( SVCHK('i',20)) {
                x2=sv->getDouble(4), y2=sv->getDouble(4+sz);
            } else if( SVCHK('i',2)) {
                x2=sv->getInt(4), y2=sv->getInt(8);
            }
        } else if(cnt==4) {
            x1=toDouble(arrs->get(0)), y1=toDouble(arrs->get(1)), x2=toDouble(arrs->get(2)), y2=toDouble(arrs->get(3));
        }
        x2-=x1;
        y2-=y1;
        x2*=2;
        y2*=2;
        rst->setVar('4',0).addDouble(std::sqrt(x2 + y2));
    } break;
    default: return false;
    }
    return true;
}

bool execMemberFunc(StrVar* sv, PARR arr, StrVar* rst, XFuncNode* fn, TreeNode* tn, LPCC funcName ) {
    if( sv==NULL ) return false;
    if( SVCHK('f',1) ) {
        bool ok=true;
        XFuncSrc* fsrc = (XFuncSrc*)SVO;
        XFuncNode* fnParent=NULL;
        TreeNode* thisNode=tn;
        if( thisNode==NULL ) {
            sv=fn?fn->gv("@this"): NULL;
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            }
        }
        if(thisNode==NULL) {
            qDebug("#9#execMemberFunc error");
            return false;
        }
        sv=thisNode->gv("onInit");
        if(SVCHK('f',0)) {
            fnParent=(XFuncNode*)SVO;
        }
        /*
        else {
            thisNode=findPageNode(thisNode);
            sv=thisNode?thisNode->gv("onInit"):NULL;
            if(SVCHK('f',0)) {
                fnParent=(XFuncNode*)SVO;
            }
        }
        TreeNode* meNode=findPageNode(thisNode);
        if( meNode ) {
            sv=meNode->gv("onInit");
            if( SVCHK('f',0) ) {
                fnParent=(XFuncNode*)SVO;
            }
        }
        */
        XFuncNode* fnPrev=NULL;
        if( fnParent ) {
            fnPrev = fnParent->xparent==NULL ? fn : fnParent->xparent;
            fnParent->xparent=fn;
        } else {
            // fn->xparent=fnParent;
            fnParent=fn;
        }
        XFunc* func=fsrc->xfunc;
        XFuncNode* fnCur = gfns.getFuncNode(func, fnParent);

        if( fnCur ) {            
            setFuncSrcParam(fnCur,arr,fsrc,0,fn);
#ifdef INLINENODE_USE
            if( func && func->xflag & FUNC_INLINE ) {
                fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)tn);
            }
#endif
            if( slen(funcName) ) getCf()->set("@currentFuncName", funcName);
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)tn );
            fnCur->call(func,rst);
            funcNodeDelete(fnCur);
        } else {
            ok=false;
            qDebug("member function (%s) funcNode not defined !!!", funcName);
        }
        if( fnPrev) {
            fnParent->xparent = fnPrev==fn? NULL: fnPrev;
        }
        return ok;
    } else if( SVCHK('f',0) ) {
        XFuncNode* fnCur=(XFuncNode*)SVO;
        if( fnCur && fnCur->xfunc ) {
            XFuncSrc* fsrc = fnCur->xfunc->getFuncSrc();
            if( fsrc ) {
                setFuncSrcParam(fnCur,arr,fsrc,0,fn);
                fnCur->call(NULL,rst);
                return true;
            }
        } else if( funcName ) {
            qDebug("#0# %s memberFunc funcName is called error", funcName);
        }
    }
    return false;
}


bool execCallSubfunc(LPCC funcNm, LPCC subNm, XFuncNode* fn, XListArr* arrs, StrVar* rst, bool bSubClass, bool bExtern, bool bSwitch) {
    XFuncNode* fnParent=fn;
    XFuncSrc* fsrc=NULL;
	StrVar* sv = NULL;
	/*
	if( bExtern ) {
		sv=getSubFuncSrc(funcNm, subNm);
		if( SVCHK('f',1) ) {
			fsrc=(XFuncSrc*)SVO;
			if( fsrc->xflag & 0x1800 ) {
				bExtern=false;
			} else {
				sv = getStrVar("fsrc",funcNm);
				if( SVCHK('f',1) ) {
					fsrc = (XFuncSrc*)SVO;
				} else {
					bExtern=false;
				}
			}
		} else {
			sv = getStrVar("fsrc",funcNm);
			if( SVCHK('f',1) ) {
				fsrc = (XFuncSrc*)SVO;
			}
		}
	} else {

		sv=getSubFuncSrc(funcNm, subNm);
		if( SVCHK('f',1) ) {
			fsrc=(XFuncSrc*)SVO;
		}
	}
	*/
    sv=getSubFuncSrc(funcNm, subNm);
    if( SVCHK('f',1) ) {
        fsrc=(XFuncSrc*)SVO;
    }
    if( fsrc==NULL ) {
        qDebug("#9##[%s] sub function not define [@%s.%s]\n", getBaseFuncName(fn), funcNm, subNm);
		return false;
	}
	XFunc* func=fsrc->xfunc;
	XFuncNode* fnCur=gfns.getFuncNode( func,fnParent );
	TreeNode* thisNode=NULL;
	sv = fn->gv("@this");
	if( SVCHK('n',0) ) {
		thisNode=(TreeNode*)SVO;
	}

	/*
    // interface
    if( fsrc->xflag & 0x400 ) {
        bSubClass=true;
    }
	if( (fsrc->xflag & 0x1800)==0 && bSwitch==false ) {
        if( cnt==0 ) {
			arrs=getLocalArray(true);
			arrs->add()->set(subNm);
		} else {
			sv=arrs->insert(0);
			if( sv ) sv->set(subNm);
		}
	}
	TreeNode* meNode=NULL;
	sv=fn->gv("@page");
	if( SVCHK('n',0)) {
		meNode=(TreeNode*)SVO;
		sv=meNode->gv("onInit");
		if( SVCHK('f',0) ) {
			fnParent=(XFuncNode*)SVO;
		}
	} else {
		meNode=thisNode;
	}
	if( meNode ) {
		fnCur->GetVar("@page")->setVar('n',0,(LPVOID)meNode);
	}
	*/


	if( thisNode ) {
		fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
#ifdef INLINENODE_USE
		if(  func && func->xflag & FUNC_INLINE ) {
			fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)thisNode);
		}
#endif
	}
	/*
	else if( fn) {
		sv=fn->gv("@inlineNode");
		if( SVCHK('n',0) ) {
			TreeNode* inlineNode=(TreeNode*)SVO;
			fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)inlineNode);
		}
	}
	if( bExtern ) {
		fnCur->GetVar("_funcName_")->set(funcNm);
		fnCur->GetVar("type")->set(subNm);
	} else {
		sv=fnCur->GetVar("_funcName_");
		sv->set(funcNm);
		if( slen(subNm) ) sv->add("::").add(subNm);
	}
	*/
	// printCallLog(fnCur->get("_funcName_") );
    // gMutexSubfunc.EnterMutex();
	setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
	fnCur->call(NULL,rst->reuse() );
	funcNodeDelete(fnCur);
    // gMutexSubfunc.LeaveMutex();
	// releaseLocalArray(arrs);
	return true;
}

#ifdef WIDGET_USE
void makeChildMenus(XListArr* a, QMenu* menu, TreeNode* pageNode) {
    // (JWas)
    TreeNode* actions=getTreeNode("action","nodes", true);
    for(int n=0,sz=a->size();n<sz;n++) {
        StrVar* sv=a->get(n);
        if( SVCHK('n',0) ) {
            TreeNode* cur = (TreeNode*)SVO;
            LPCC type=cur->get("type");
            if(ccmp(type,"separator")) {
                menu->addSeparator();
                continue;
            }
            sv = cur->gv("menus");
            if( SVCHK('a',0) ) {
                XListArr* arr =(XListArr*)SVO;
                QMenu* sub=NULL;
                LPCC icon=cur->get("icon"), text=cur->get("text");
                QPixmap* px=NULL;
                if( slen(icon)) {
                    px=getPixmap(icon, true);
                }
                if( slen(text) ) {
                    if( px )
                        sub = menu->addMenu(QIcon(*px), KR(text) );
                    else
                        sub = menu->addMenu(KR(text));
                }
                if( sub ) {
                    makeChildMenus(arr, sub, pageNode);
                }
            } else {
                LPCC actionId = cur->get("actionId");
                if( slen(actionId)==0 ) {
                    actionId = cur->get("id");
                }
                GAction* action = NULL;
                TreeNode* actionNode=actions->getNode(actionId);
                if( actionNode ) {     // SVCHK('w',11) ) {
                    sv=actionNode->gv("@action");
                    if( SVCHK('w',11) ) {
                        action = (GAction*)SVO;
                        setActionValue(action, cur);
                    }
                }
                if( action==NULL) {
                    LPCC text=cur->get("text");
                    if( slen(text) ) {
                        if( actionNode==NULL ) {
                            actionNode=actions->addNode(actionId);
                        }
                        action = new GAction(actionNode);
                        actionNode->set("id", actionId);
                        actionNode->GetVar("@action")->setVar('w',11,(LPVOID)action);
                        actionNode->xstat=WTYPE_ACTION;
                        setActionValue(action, cur);
                    }
                }
                if( actionNode ) {
                    actionNode->GetVar("@page")->setVar('n',0,(LPVOID)pageNode);
                }
                if( action ) {
                    menu->addAction(action);
                }
            }
        } else {
            TreeNode* cur=NULL;
            LPCC actionId= toString(sv); // sv->get();
            if( ccmp(actionId,"-") ) {
                menu->addSeparator();
            } else if( slen(actionId) ) {
                TreeNode* actionNode=actions->getNode(actionId);
                if( actionNode ) {     // SVCHK('w',11) ) {
                    sv=actionNode->gv("@action");
                    if( SVCHK('w',11) ) {
                        GAction* action = (GAction*)SVO;
                        menu->addAction(action);
                        cur=action->config();
                        if(cur) cur->GetVar("@page")->setVar('n',0,(LPVOID)pageNode);
                    }
                }
            }
        }
    }
}

void makeNodeChildMenus(TreeNode* a, QMenu* menu, TreeNode* pageNode) {
    TreeNode* actions=getTreeNode("action","nodes", true);
    StrVar* sv=NULL;
    for(int n=0,sz=a->childCount();n<sz;n++) {
        TreeNode* cur = a->child(n);
        if(cur->isNodeFlag(FLAG_VARCHECK) ) {
            continue;
        }
        LPCC type=cur->get("type");
        if( ccmp(type,"separator")) {
            menu->addSeparator();
            continue;
        }
        LPCC menuText=cur->get("menuText");
        if( slen(menuText) ) {
            LPCC icon=cur->get("icon");
            QPixmap* px=NULL;
            QMenu* sub=NULL;
            if( slen(icon)) {
                px=getPixmap(icon, true);
            }
            if( px )
                sub = menu->addMenu(QIcon(*px), KR(menuText) );
            else
                sub = menu->addMenu(KR(menuText));
            if( sub ) {
                makeNodeChildMenus(cur, sub, pageNode);
            }
        } else {
            LPCC actionId = cur->get("actionId");
            if( slen(actionId)==0 ) {
                actionId = cur->get("id");
            }
            GAction* action = NULL;
            TreeNode* actionNode=actions->getNode(actionId);
            if( actionNode ) {
                sv=actionNode->gv("@action");
                if( SVCHK('w',11) ) {
                    action = (GAction*)SVO;
                    setActionValue(action, cur);
                }
            }
            if( action==NULL) {
                LPCC text=cur->get("text");
                if( slen(text) ) {
                    if( actionNode==NULL ) {
                        actionNode=actions->addNode(actionId);
                    }
                    action = new GAction(actionNode);
                    actionNode->set("id", actionId);
                    actionNode->GetVar("@action")->setVar('w',11,(LPVOID)action);
                    actionNode->xstat=WTYPE_ACTION;
                    setActionValue(action, cur);
                }
            }
            if( actionNode ) {
                actionNode->GetVar("@page")->setVar('n',0,(LPVOID)pageNode);
            }
            if( action ) {
                menu->addAction(action);
            }
        }
    }
}
#endif

#ifdef WIDGET_USE
bool callTrayFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    StrVar* sv=tn->GetVar("@tray");
    GTray* tray = NULL;
    if( SVCHK('w',21) ) {
        tray=(GTray*)SVO;
    } else {
        return false;
    }
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0  ) {
        ref =
            ccmp(fnm,"icon") ? 1:
            ccmp(fnm,"action") ? 2:
            ccmp(fnm,"contextMenu") ? 3:
            ccmp(fnm,"pageNode") ? 4:
            ccmp(fnm,"show") ? 5:
            ccmp(fnm,"hide") ? 6:
            ccmp(fnm,"message") ? 7:
            ccmp(fnm,"callback") ? 8: 0;
        if( fc ) fc->xfuncRef = ref;
    }
    if( ref==0 )
        return false;

    switch( ref ) {
    case 1: {		// icon
        QPixmap* img=getStrVarPixmap(arrs->get(0), true);
        if( img ) {
            tray->setIcon(iconPixmap(gCommIcon,img,AS(1),AS(2)));
        }
    } break;
    case 2: {		// action

    } break;
    case 3: {		// contextMenu
        if( cnt==0 ) return false;
        TreeNode* pageNode=findPageNode(tn);
        XListArr* a=NULL;
        sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            a=(XListArr*)SVO;
        } else {
            XParseVar pv(sv);
            a=getListArrTemp();
            while(pv.valid() ) {
                if(!pv.ch()) break;
                a->add()->add(pv.findEnd(",").v());
            }
        }
        GMenu* menu = NULL;
        sv = tn->GetVar("@contextMenu");
        if( SVCHK('w',12) ) {
            menu = (GMenu*)SVO;
            SAFE_DELETE(menu);
        }
        menu = new GMenu(pageNode);
        makeChildMenus(a,menu,pageNode);
        tray->setContextMenu(menu);
        rst->setVar('a',0,(LPVOID)a);
        // gtrees.removeArr(a);
    } break;
    case 4: {		// pageNode
        rst->setVar('n',0,(LPVOID)tray->config());
    } break;
    case 5: {		// show
        tray->show();
    } break;
    case 6: {		// hide
        tray->hide();
    } break;
    case 7: {		// message
        LPCC title = AS(0);
        LPCC msg = AS(1);
        LPCC icon = AS(2);
        QSystemTrayIcon::MessageIcon mi = slen(icon)==0? QSystemTrayIcon::Information :
            ccmp(icon,"warning")? QSystemTrayIcon::Warning :
            ccmp(icon,"critical")? QSystemTrayIcon::Critical: QSystemTrayIcon::Information;
        tray->showMessage(KR(title), KR(msg), mi);
    } break;
    case 8: {		// callback
        rst->reuse();
        if( cnt==0 ) {
            sv=tn->gv("onTrayEvent");
            if(sv) rst->add(sv);
            return true;
        }
        sv=arrs->get(0);
        TreeNode* thisNode=NULL;
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(2);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("onTrayEvent"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
        } else {
            sv=tn->gv("onTrayEvent");
            if( SVCHK('f',0) ) {
                XFuncNode* fn=(XFuncNode*)SVO;
                PARR arrs=getLocalParams(fn);
                arrs->add()->set(AS(0));
                setFuncNodeParams(fn, arrs);
                fn->call();
            }
        }
    } break;
    default:
        return false;
    }
    return true;
}
#endif
