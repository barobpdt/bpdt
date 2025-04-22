#include "func_util.h"
#include "../util/user_event.h"

#include <QApplication>
#include <QMessageBox>
#include <QDesktopServices>

StrVar gOperVar;

inline int confirm(LPCC msg, LPCC title, QWidget* parent) {
    QMessageBox mb(parent );
    int icon=2;
    mb.setIcon((QMessageBox::Icon)icon);
    mb.setStandardButtons(QMessageBox::Ok| QMessageBox::Cancel);
    mb.button(QMessageBox::Cancel)->setText(KR(conf("msg#confirm/cancel","cancel")));
    mb.button(QMessageBox::Ok)->setText(KR(conf("msg#confirm/ok","ok")));
    if( slen(msg) ) { mb.setText(KR(msg)); }
    if( slen(title)>0 ) mb.setWindowTitle(KR(title));
    return mb.exec();
}

bool execCheckFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    if( fc==NULL ) {
        if( arrs ) {
            arrs->xflag&=~FLAG_LOCALARRAY_SET;
        }
        return false;
    }
    bool rtn = false;
    switch(fc->xfkind) {
    case 92 :	{	// eq
        int cnt= arrs? arrs->size():0;
        rst->reuse();
        if( cnt==0 ) {
            return true;
        }
        rst->setVar('3',0);
        StrVar* sv=arrs->get(0);
        int n=1;
        for( ;n<cnt;n++) {
            StrVar* var=arrs->get(n);
            if( var->charAt(0)=='\b' ) {
                if( SVCHK('i',5) || SVCHK('i',20) ) {
                    if( !checkFuncObject(var,'i',5) ) break;
                    int sz=sizeof(double);
                    double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                    if( x==sv->getDouble(4) && y==sv->getDouble(4+sz) ) {
                        if( SVCHK('i',5) ) {
                            if( w==sv->getInt(4+(2*sz)) && h==sv->getInt(4+(3*sz)) ) rst->setVar('3',1);
                        } else {
                            rst->setVar('3',1);
                        }
                    }
                } else if( SVCHK('i',4) ) {
                    if( !checkFuncObject(var,'i',4) ) break;
                    int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
                    if( x==sv->getInt(4) && y==sv->getInt(8) && w==sv->getInt(12) && h==sv->getInt(16) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('i',2) ) {
                    if( !checkFuncObject(var,'i',2) ) break;
                    int x=var->getInt(4), y=var->getInt(8);
                    if( x==sv->getInt(4) && y==sv->getInt(8) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('0',0) ) {
                    if( sv->getInt(4)==toInteger(var) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('1',0) ) {
                    if( sv->getUL64(4)==toUL64(var) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('2',0) ) {
                    if( sv->getU32(4)==(U32)toInteger(var) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('4',0) ) {
                    if( sv->getDouble(4)==toDouble(var) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('9',0) ) {
                    if( checkFuncObject(var,'9',0) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('s',0) ) {
                    if( ccmp(toString(sv), toString(var)) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('3',0) ) {
                    if( checkFuncObject(var,'3',0) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('3',1) ) {
                    if( checkFuncObject(var,'3',1) ) {
                        rst->setVar('3',1);
                    }
                } else if( SVCHK('9',0) ) {
                    if( checkFuncObject(var,'9',0) ) {
                        rst->setVar('3',1);
                    }
                } else {
                    if( SVO==var->getObject(FUNC_HEADER_POS) ) {
                        rst->setVar('3',1);
                    }
                }
            } else {
                LPCC a=sv->get(), b=toString(var);
                if( slen(a)==0 && slen(b)==0 ) {
                    rst->setVar('3',1);
                } else if( ccmp(a,b) ) {
                    rst->setVar('3',1);
                }
            }
            if( checkFuncObject(rst,'3',1) ) {
                break;
            }
        }
    } break;
    case 103 :	{	// conf
        if(arrs==NULL) {
            rst->reuse()->add(confVar(NULL));
            return true;
        }
        int cnt= arrs? arrs->size():0;
        StrVar* var=confVar(AS(0), cnt>1 ? arrs->get(1):NULL, isVarTrue(arrs->get(2)) );
        rst->reuse();
        if(var && var->length() ) {
            bool bctrl=false;
            XFunc* pfc=fc->parent();
            if( pfc && pfc->getType()==FTYPE_ARRAY ) {
                pfc = pfc->parent();
            }
            if( pfc && (pfc->getType()==FTYPE_CONTROL||pfc->getType()==FTYPE_FUNCCHECK) ) {
                bctrl=true;
            }
            if(bctrl) {
                rst->setVar('3',1);
            } else {
                rst->add(var);
            }
        }
    } break;
    case FKIND_EXPR :	{		// expr
    } break;
    case FKIND_PATH :	{       // path
        rst->reuse();
        int cnt = fc->getParamSize();
        if( cnt==0 ) {
            rst->add( Q2A(QApplication::applicationDirPath()) );
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                /*
                int num = toInteger(arrs->get(0));
                rst->add( Q2A(QDesktopServices::storageLocation((QDesktopServices::StandardLocation)num)) );
                for( int n=1;n<cnt;n++ ) {
                    rst->add('/').add(AS(n));
                }
                QDir dir;
                dir.mkpath(KR(rst->get()));
                */
            } else {
                if( cnt==2 ) {
                    LPCC ty=AS(0);
                    LPCC path=AS(1);
                    if( ccmp(ty,"@") ) {
                        rst->add( Q2A(QApplication::applicationDirPath()) ).add(path);
                    }
                    if( rst->length() ) {
                        QDir dir(KR(path));
                        if( !dir.exists() ) dir.mkpath(KR(rst->get()));
                    }
                } else {
                    LPCC path=AS(0);
                    if( path && path[0]=='/') {
                        rst->add( Q2A(QApplication::applicationDirPath()) ).add(path);
                    } else {
                        rst->add(path);
                    }
                }
            }
        }
    } break;
    case FKIND_ISNULL :	{
        if( !isVarTrue(arrs->get(0)) ) {
            if( rst->length()>0 )
                toString(arrs->get(1),rst);
            else
                rst->add( arrs->get(1) );
        }
    } break;
    case FKIND_PRINT :	{ // print
        bool error=false;
        XFunc* pfc=fc->parent();
        if( pfc && pfc->xftype==FTYPE_RETURN ) {
            error=true;
        }
        rst->reuse();
        rst->add("[").add(getBaseFuncName(fn)).add("] ");
        if( arrs==NULL ) {
            printBox(fn,rst);
        } else {
            int cnt=arrs->size();
            for( int n=0; n<cnt; n++ ) {
                XFunc* param = fc->getParam(n);
                if( param==NULL ) break;
                if( n>0 ) {
                    rst->add("\n\t");
                }
                if( param->xftype==FTYPE_TEXT || param->xftype==FTYPE_STR || param->xftype==FTYPE_NUMBER ) {
                    LPCC val=toString(arrs->get(n));
                    rst->add(val);
                } else {
                    LPCC var = param->getValue();
                    if( var[0]=='@' ) {
                        getPrintInfo(var, fn, rst, arrs);
                    } else {
                        StrVar* sv=arrs->get(n);
                        rst->add(var).add("=");
                        toString(sv, rst);
                    }
                }
            }
        }
        if( rst->length() ) {            
            if( error ) {
                qDebug("#9# %s\n", rst->get());
            } else {
                qDebug("#0# %s\n", rst->get());
            }
        }
        if( error ) {
            rst->reuse();
        }
    } break;
    case 90:	{	// alert
        if( arrs==NULL ) return true;
        rst->reuse();
        int cnt=arrs->size();
        for( int n=0; n<cnt; n++ ) {
            StrVar* sv=arrs->get(n);
            XFunc* param = fc->getParam(n);
            if( param==NULL ) break;
            if( rst->length() ) rst->add(", ");
            if( param->xftype==FTYPE_TEXT || param->xftype==FTYPE_STR || param->xftype==FTYPE_NUMBER ) {
                rst->add(toString(sv));
            } else {
                LPCC data=toString(sv);
                QString str=KR(data);
                if( str.length()>256 ) {
                    str=str.mid(0,256);
                    str.append("...\n");
                    rst->add( param->getValue() ).add("=").add(Q2A(str));
                } else {
                    rst->add( param->getValue() ).add("=").add(data);
                }
            }
        }
        QString msg=KR(rst->get());
        if( msg.length()>2048 ) {
            msg=msg.mid(0,2048);
            msg.append("...");
        }
        int icon=2;
        LPCC title=conf("alert.title","info");
        QMessageBox mb;
        mb.setIcon((QMessageBox::Icon)icon);
        mb.setText(msg);
        if( slen(title)>0 ) mb.setWindowTitle(KR(title));
        mb.exec();
        rst->reuse();
    } break;
    case 91 :	{	// confirm
        if( arrs ) {
            return true;
        }
        LPCC msg=AS(0), title=AS(1), info=AS(2);
        if( slen(title)==0 ) {
            title=toString(confVar("confirm.title"));
        }
        int ret=confirm(msg,info,title);
        rst->setVar('3',ret==QMessageBox::Ok?1:0);
    } break;
    case 93 :	{	// flags
        if( arrs==NULL) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            U32 a=toUL64(sv), b=0;
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                b=toUL64(sv);
            }
            if(isVarTrue(arrs->get(2))) {
                rst->setVar('3', a|b?1: 0);
            } else {
                rst->setVar('3', a&b? 1: 0);
            }
        }
    } break;
    case 108 :		// max
    case 109 :	{	// min
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            rst->setVar('3',0);
        } else {
            StrVar* sv=NULL;
            if( cnt==1 ) {
                sv = arrs->get(0);
                if( isNumberVar(sv) ) {
                    rst->setVar('0',0).addInt(toInteger(sv));
                } else {
                    rst->setVar('3',0);
                }
            } else {
                sv = arrs->get(0);
                if( isNumberVar(sv) ) {
                    if( isNumberVar(arrs->get(1)) ) {
                        rst->setVar('0',0).addInt( fc->xfkind==108 ? max(toInteger(sv),toInteger(arrs->get(1))) : min(toInteger(sv),toInteger(arrs->get(1))) );
                    } else {
                        rst->setVar('0',0).addInt(toInteger(sv));
                    }
                } else if( isNumberVar(arrs->get(1)) ) {
                    rst->setVar('0',0).addInt(toInteger(arrs->get(1)));
                } else {
                    rst->setVar('3',0);
                }
            }
        }
    } break;
    case 111 :	{	// not
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==1 ) {
            StrVar* sv=arrs->get(0);
            // XFunc* pfc=fc->parent();
            if( SVCHK('0',1) ) {
                U32 flag=sv->getU32(4);
                U32 val=~flag;
                rst->setVar('0',0).addInt((int)val);
            } else {
                rst->setVar('3', isVarTrue(sv) ? 0:1 );
            }
        }
    } break;
    case 112 :	{	// nvl
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==2 ) {
            StrVar* sv=arrs->get(0);
            if( SVCHK('9',0) || (sv && sv->length()) ) {
                rst->reuse()->add(sv);
            } else {
                rst->reuse()->add(arrs->get(1));
            }
        } else {
            StrVar* sv=arrs->get(0);
            if( isVarTrue(sv) ) {
                rst->reuse()->add(sv);
            } else {
                rst->reuse()->add(arrs->get(1));
            }
        }
    } break;
    case 113 :	{	// and
        int cnt = arrs ? arrs->size(): 0;
        StrVar* sv=NULL;
        rst->setVar('3',1);
        for(int n=0; n<cnt; n++) {
            sv=arrs->get(n);
            if( !isVarTrue(sv)) {
                rst->setVar('3',0);
                break;
            }
        }
    } break;
    case 115 :	{	// or
        int cnt = arrs ? arrs->size(): 0;
        StrVar* sv=NULL;
        rst->setVar('3',0);
        for(int n=0; n<cnt; n++) {
            sv=arrs->get(n);
            if( isVarTrue(sv)) {
                rst->setVar('3',1);
                break;
            }
        }
    } break;
    case 116 :	{	// eval
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            return true;
        }
        bool bset=false;
        int sp=0, ep=0;
        XFuncNode* fnCur=fn;
        TreeNode* thisNode=NULL;
        StrVar* appendVar=NULL;
        StrVar* var=getStrVarPointer(arrs->get(0),&sp,&ep);
        for(int n=1; n<cnt; n++) {
            StrVar* sv=arrs->get(n);
            if(SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
            } else if(SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            } else if(SVCHK('3',1) || SVCHK('3',0) ) {
                bset=SVCHK('3',1);
            } else if(!isNumberVar(sv) && !SVCHK('9',0) ) {
                appendVar=sv;
            }
        }
        callEvalFunc(var,sp,ep,fnCur,rst,thisNode,appendVar,bset);
    } break;
    case 201 :	{	// fmt
        int cnt=fc->getParamSize();
        if( cnt==0 ) return true;
        StrVar* sv=uom.getInfo()->gv("@callFuncNode");
        if(SVCHK('f',0)) {
            fn=(XFuncNode*)SVO;
            sv=NULL;
        }
        StrVar* var=NULL;
        XFuncNode* fnCur=fn;
        TreeNode* curNode=NULL;
        XFunc* fcParam=fc->getParam(1);
        if( fcParam ) {
            execParamFunc(fcParam, fn, rst->reuse());
            sv=rst;
            if( SVCHK('n',0)  ) {
                curNode=(TreeNode*)SVO;
                fcParam=fc->getParam(2);
                if( fcParam) {
                    execParamFunc(fcParam, fn, rst->reuse());
                    sv=rst;
                    if( SVCHK('f',0) ) {
                        fnCur=(XFuncNode*)SVO;
                    }
                }
            } else if( SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
            }
        }

        rst->reuse();
        fcParam=fc->getParam(0);
        if( fcParam ) {
            if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
                makeTextData(fcParam->xpv, fnCur, rst, 0x20000, curNode );
            } else if( fcParam->xftype==FTYPE_VAR && fcParam->xdotPos==0 ) {
                LPCC code=fcParam->getValue();
                sv=fn->gv(code);
                if( sv==NULL && fn!=fnCur) {
                    sv=fnCur->gv(code);
                }
                if(sv) {
                    int sp=0, ep=0;
                    var=getStrVarPointer(sv, &sp, &ep);
                    XParseVar pv(var, sp, ep);
                    makeTextData(pv, fnCur, rst, 0x20000, curNode );
                }
            } else {
                if( arrs==NULL) {
                   arrs=getLocalArray(true);
                }
                int sp=0, ep=0;
                sv=arrs->add();
                execParamFunc(fcParam, fnCur, sv);
                var=getStrVarPointer(sv, &sp, &ep);
                if(sp<ep) {
                    XParseVar pv(var, sp, ep);
                    makeTextData(pv, fnCur, rst, 0x20000, curNode );
                }
            }
        }
        if( arrs ) {
            releaseLocalArray(arrs);
        }
    } break;
    case 207 :	{	// args
        int cnt = fc->getParamSize();
        StrVar* sv= fn->gv("@params");
        XListArr* a=NULL;
        if( SVCHK('a',0) ) {
            a=(XListArr*)SVO;
        } else {
            a=getListArrTemp();
        }
        if( cnt==0 ) {            
            rst->setVar('a',0,(LPVOID)a);
            return true;
        }
        int x=0, sp=0;
        XFunc* param = fc->getParam(0);
        if( param->xftype==FTYPE_NUMBER ) {
            LPCC val = param->getValue();
            if( isnum(val) ) {
                sp=atoi(val);
                x++;
                if( cnt==1 ) {
                    XListArr* arr=getListArrTemp();
                    int sz=a->size();
                    for( int n=sp; n<sz; n++ ) {
                        arr->add()->add(a->get(n));
                    }
                    rst->setVar('a',0,(LPVOID)arr);
                    return true;
                }
            }
            int sz=a->size();
            for( ; x<cnt; x++ ) {
                XFunc* param = fc->getParam(x);
                if( param==NULL ) break;
                LPCC val = param->getValue();
                if( sp<sz ) {
                    if( val[0]=='&' ) {
                        val++;
                        sv=a->get(sp);
                        if( sv->charAt(0)=='\b' ) {
                            fn->GetVar(val)->reuse()->add(sv);
                        } else if( sv ) {
                            int sp=0, ep=sv->length();
                            fn->GetVar(val)->setVar('s',0, (LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                        } else {
                            fn->GetVar(val)->reuse();
                        }
                    } else {
                        fn->GetVar(val)->reuse()->add(a->get(sp));
                    }
                } else {
                    fn->GetVar(val)->reuse();
                }
                sp++;
            }
        } else {
            int sz=a->size();
            for( x=0; x<cnt; x++ ) {
                param = fc->getParam(x);
                if( param==NULL ) break;
                LPCC code = param->getValue();
                if( x<sz ) {
                    if( code[0]=='&' ) {
                        code++;
                        sv=a->get(x);
                        if( sv->charAt(0)=='\b' ) {
                            fn->GetVar(code)->reuse()->add(sv);
                        } else if( sv ) {
                            int sp=0, ep=sv->length();
                            fn->GetVar(code)->setVar('s',0, (LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                        } else {
                            fn->GetVar(code)->reuse();
                        }
                    } else {
                        fn->GetVar(code)->reuse()->add(a->get(x));
                    }
                } else {
                    fn->GetVar(code)->reuse();
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 208:	{	// typeof
        getTypeOf(fc, rst->reuse(), fn);
    } break;
    case 209 :	{	// when
        int cnt=fc->getParamSize();
        if( cnt==0 ) return false;
        // StrVar* sv=NULL;
        if( cnt==2 ) {
            XFunc* param = fc->getParam(0);
            execParamFunc(param, fn, rst->reuse() ); // execFunc(param, fn, rst->reuse() );
            if( isVarTrue(rst) ) {
                execParamFunc(fc->getParam(1), fn, rst->reuse() );
                return true;
            }
        } else {
            int last=cnt-1;
            for( int n=0;n<cnt;) {
                if( n%2==0 ) {
                    XFunc* param = fc->getParam(n++);
                    execParamFunc(param, fn, rst->reuse() );
                    if( isVarTrue(rst) ) {
                        execParamFunc(fc->getParam(n), fn, rst->reuse() );
                        return true;
                    } else {
                        n++;
                        if( n==last ) {
                            execParamFunc(fc->getParam(n), fn, rst->reuse() );
                            return true;
                        }
                    }
                } else {
                    qDebug("when function not match !!!");
                }
            }
        }
        rst->reuse();
    } break;
    /*
    case 207 :	{	// sp
        if( fc->getParamSize() ) {
            XFunc* param = fc->getParam(0);
            parseSpCommand(&(param->xpv),fn, rst);
        }
    } break;
    */
    default: break;
    }
#ifdef FUNC_ARRAY_USE
    if( arrs && arrs->xflag&FLAG_LOCALARRAY_SET ) {
        arrs->xflag&=~FLAG_LOCALARRAY_SET;
        // gtrees.removeArr(arrs);
    }
#endif
    return rtn;
}


bool isUpperCase(char c) {
    int nc=(int)c;
    if( nc<0 || !isalpha(nc) ) return false;
    return isupper(nc) ? true: false;
}
char getFuncType(LPCC fnm) {
    int size = slen(fnm);
    if( size==0 ) {
        return (char)0;
    }
    if( isUpperCase(fnm[0]) ) {
        return ( ccmpl(fnm,"Cf.",3) || ccmpl(fnm,"System.",7) || ccmpl(fnm,"Baro.",5) || ccmpl(fnm,"Math.",5) ) ? 'p' : 'U';
    }
    return
        (ccmp(fnm,"func") || ccmp(fnm,"function"))  ? 'b' :
        ( STARTWITH(fnm,"else",size) || ccmp(fnm,"if") || ccmp(fnm,"not") || ccmp(fnm,"for")
            || ccmp(fnm,"while") || ccmp(fnm,"switch")
#ifdef IFTRUE_USE
            //  || ccmp(fnm,"ifdef")
            || ccmp(fnm,"ifThis") || ccmp(fnm,"notThis")
            || ccmp(fnm,"ifFalse") || ccmp(fnm,"ifTrue")
#endif
        ) ? 'C' :			//  || ccmp(fnm,"loop") || ccmp(fnm,"foreach") || ccmp(fnm,"each") ccmp(fnm,"or")|| ccmp(fnm,"expr") ||
        ( ccmp(fnm,"fmt") || ccmp(fnm,"print") || ccmp(fnm,"args") || ccmp(fnm,"conf") || ccmp(fnm,"eq") ||
          ccmp(fnm,"use") || ccmp(fnm,"var") || ccmp(fnm,"call") || ccmp(fnm,"when") || ccmp(fnm,"echo") || ccmp(fnm,"alert") ||
          ccmp(fnm,"typeof") || ccmp(fnm,"eval") ||
          ccmp(fnm,"nvl") || ccmp(fnm,"max") || ccmp(fnm,"min")

          // ccmp(fnm,"var") || ccmp(fnm,"flags") || ccmp(fnm,"get") || ccmp(fnm,"sp") || ccmp(fnm,"path") || ccmp(fnm,"instance")
          // ccmp(fnm,"confirm") || ccmp(fnm,"between") || ccmp(fnm,"throw") ||
          // ccmp(fnm,"array") || ccmp(fnm,"node") || ccmp(fnm,"event") || ccmp(fnm,"func") || ccmp(fnm,"isnull") || ccmp(fnm,"json") ||
        ) ? 'i' : 'U';
}

int getControlFuncKind(XFunc* func) {
    if( func && func->xfkind==0 ) {
        LPCC fname = func->getValue();
        func->xfkind =
            ccmp(fname,"if") ? 1:
            ccmp(fname,"not") ? 10:
            ccmp(fname,"while") || ccmp(fname,"for") ? 13:
            ccmp(fname,"switch") ? 4:
            ccmp(fname,"else") ? 5:
            ccmp(fname,"else if") ? 6:
            99;
    }
    return func->xfkind;
}

bool funcArrayVarOperCheck(StrVar* kv, int sp, int ep) {
    if( kv==NULL || sp>=ep ) return false;
    char* buf=kv->get();
    char ch=0;
    for(int x=sp; x<ep; x++ ) {
        ch=buf[x];
        if( ch==',' || ch==';' || ch=='=' || ch=='(' || ch==':' || ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' || ch=='<' || ch=='>' || ch=='&' || ch=='|' || ch=='^' || ch=='!' ) {
            return true;
        }
    }
    return false;
}
bool funcArrayVarCheck(StrVar* kv, int sp, int ep) {
    if( kv==NULL || sp>=ep ) return false;
    char* buf=kv->get();
    char ch=0;
    for(int x=sp; x<ep; x++ ) {
        ch=buf[x];
        if( ch=='$' ) {
            if( (x+1)<ep ) {
                if( buf[x+1]!='[' ) return true;
            }
        } else if( ch=='=' || ch=='(' || ch==':' ) {
            return false;
        }
    }
    return false;
}

StrVar* getArrayKeyVar(StrVar* var, LPCC key, XFuncNode* fn, StrVar* rst ) {
    if( var==NULL || fn==NULL )
        return NULL;
    if( var && var->charAt(0)=='\b' ) {
        char ch = var->charAt(1);
        U16 stat = var->getU16(2);
        switch(ch) {
        case 'w': {
        } break;
        case 'd': {
            DbUtil* db = (DbUtil*)var->getObject(FUNC_HEADER_POS);
            var = db ? db->GetVar(key): NULL;
        } break;
        case 'a': {
            XListArr* arr = (XListArr*)var->getObject(FUNC_HEADER_POS);
            var = arr ? arr->get( isnum(key) ? atoi(key): 0): NULL;
        } break;
        case 'n': {
            if( stat==0 ) {
                TreeNode* node = (TreeNode*)var->getObject(FUNC_HEADER_POS);
                var = getVarValue(node, key, rst);
            }
        } break;
        default:
            qDebug()<<"FTYPE_SETARRAY call error : "<<KR(var->get()); break;
            var = NULL;
        }
    }
    if( rst ) {
        rst->reuse();
        rst->add(var);
    }
    return var;
}

bool compareResult(StrVar* lr, StrVar* rr, LPCC op ) {
    if( op==NULL || lr==NULL || rr==NULL )
        return false;
    switch(op[0]) {
    case '&': {
        return op[1]=='&' ? isVarTrue(lr)&&isVarTrue(rr) : toUL64(lr) & toUL64(rr);
    } break;
    case '|': {
        return op[1]=='|' ? isVarTrue(lr)||isVarTrue(rr) : toUL64(lr) | toUL64(rr);
    } break;
    }
    if( isNumberVar(lr) && isNumberVar(rr) ) {
        UL64 a=toUL64(lr), b=toUL64(rr);
        switch(op[0]) {
        case '!': { return a!=b; } break;
        case '=': { return a==b; } break;
        case '<': { return op[1]=='='? a<=b: a<b; } break;
        case '>': { return op[1]=='='? a>=b: a>b; } break;
        }
    } else if( lr->charAt(0)=='\b' && rr->charAt(0)=='\b' ) {
        if( lr->charAt(1)!='s' && rr->charAt(1)!='s' ) {
            if( lr->charAt(1) == rr->charAt(1) ) {
                LPVOID lv=lr->getObject(4);
                LPVOID rv=rr->getObject(4);
                switch(op[0]) {
                case '!': { return lv!=rv; } break;
                case '=': { return lv==rv; } break;
                }
            } else {
                switch(op[0]) {
                case '!': { return true; } break;
                case '=': { return false; } break;
                }
            }
        }
    } else if( (lr->charAt(0)=='\b' && lr->charAt(1)!='s')  && (rr->charAt(0)=='\b' && rr->charAt(1)!='s')  ) {
        switch(op[0]) {
        case '!': return true;
        case '=': return false;
        }
    } else {
        int lrlen=lr->length(), rrlen=rr->length();
        if( lrlen==0 || rrlen==0 ) {
            if( lrlen==0 && rrlen==0 ) {
                switch(op[0]) {
                case '!': return false;
                case '=': return true;
                }
            } else {
                switch(op[0]) {
                case '!': return true;
                case '=': return false;
                }
            }
        } else {
            LPCC a=toString(lr,lr), b=toString(rr,rr);
            if( a && b ) {
                switch(op[0]) {
                case '!': { return !ccmp(a,b); } break;
                case '=': { return ccmp(a,b); } break;
                case '<': {
                    if( isnum(a) && isnum(b) ) {
                        if( op[1]=='=' ) {
                            return atoi(a)<=atoi(b);
                        } else {
                            return atoi(a)<atoi(b);
                        }
                    }
                } break;
                case '>': {
                    if( isnum(a) && isnum(b) ) {
                        if( op[1]=='=' ) {
                            return atoi(a)>=atoi(b);
                        } else {
                            return atoi(a)>atoi(b);
                        }
                    }
                } break;
                }
            }
        }
    }
    return false;
}

void getObjectType(StrVar* var, StrVar* rst) {
    if( var==NULL ) return;
    if( rst==NULL ) rst=gOperVar.reuse();
    if( var->charAt(0)=='\b' ) {
        switch( var->charAt(1) ) {
        case '0' : rst->add("int");			break;
        case '1' : rst->add("int64");		break;
        case '2' : rst->add("define");		break;
        case '3' : rst->add("bool");		break;
        case '4' : rst->add("double");		break;
        case '9' : rst->add("null");		break;
        // case 'S' : rst->add("string");		break;
        // case 'p' : rst->add("string object");	 break;
        case 'w' : {
            U16 stat = var->getU16(2);
            switch( stat) {
            case 0: rst->add(""); break;
            case 2: rst->add("action"); break;
            case 11: rst->add("action"); break;
            case 12: rst->add("menu"); break;
            case 13: rst->add("toolbar"); break;
            case 31: rst->add("graphic"); break;
            case 20: rst->add("scroll"); break;
            case 21: rst->add("tray"); break;
            case 51: rst->add("toolbutton"); break;
            }
        } break;
        case 'i' : {
            switch( var->getU16(2) ) {
            case 2: rst->add("point"); break;
            case 4: rst->add("rect"); break;
            case 20: rst->add("pointf"); break;
            case 5: rst->add("rectf"); break;
            case 6: rst->add("png"); break;
            case 7: rst->add("buffer"); break;
            case 9: rst->add("image"); break;
            }
        } break;
        case 'd' : {
            switch( var->getU16(2) ) {
            case 0: rst->add("sqlite");	break;
            case 1: rst->add("db");	break;
            }
        } break;
        case 's' : {
            switch( var->getU16(2) ) {
            case 0:		rst->add("stringRef");	break;
            case 1:		rst->add("stringRef");	break;
            case 11:	rst->add("stringObject");	break;
            case 12:	rst->add("regExp");	break;
            }
        } break;
        case 'a' : {
            rst->add("array");
        } break;
        case 'n' : {
            // version 1.0.2
            TreeNode* node=(TreeNode*)var->getObject(FUNC_HEADER_POS);
            {
				switch( node->xstat ) {
				case LTYPE_GRID:
				case LTYPE_ROW:
				case LTYPE_HBOX:
				case LTYPE_VBOX:			rst->add("layout"); break;
                case FNSTATE_DRAW:			rst->add("node(draw)"); break;
                case FNSTATE_DB:			rst->add("node(db)"); break;
                case FNSTATE_MODEL:			rst->add("node(dataModel)"); break;
                case FNSTATE_FILE:			rst->add("node(file)"); break;
                case FNSTATE_PROCESS:		rst->add("node(process)"); break;
                case FNSTATE_EXCEL:         rst->add("node(excel)"); break;
                case FNSTATE_JOB:			rst->add("node(job)"); break;
                case FNSTATE_CRON:			rst->add("node(cron)"); break;
                case FNSTATE_WORKER:		rst->add("node(worker)"); break;
                case FNSTATE_WEBCLIENT:		rst->add("node(web)"); break;
                case FNSTATE_WEBSERVER:		rst->add("node(webServer)"); break;
                case FNSTATE_WATCHER:		rst->add("node(watcher)"); break;
                case FNSTATE_PDF:			rst->add("node(printer)"); break;
                case FNSTATE_ZIP:			rst->add("node(zip)"); break;
                case FNSTATE_SOCKET:		rst->add("node(socket)"); break;
                case FNSTATE_ACTION:		rst->add("node(action)"); break;
                case FNSTATE_MENU:			rst->add("node(menu)"); break;
                case FNSTATE_CANVASITEM:	rst->add("node(drawItem)"); break;
                default: rst->add("node"); break;
                }
            }
        } break;
        case 'm' : {
            switch( var->getU16(2) ) {
            case 0: rst->add("model");		break;
            case 1: rst->add("proxy");		break;
            case 2: rst->add("mimeData");	break;
            }
        } break;
        case 'f' : {
            switch( var->getU16(2) ) {
            case 0: rst->add("func");		break;
            case 1: rst->add("funcRef");	break;
            }

        } break;
        case 'c' : {
            switch( var->getU16(2) ) {
            case 1: rst->add("drawItem");	break;
            case 2: rst->add("cursor");		break;
            case 3: rst->add("block");		break;
            }
        } break;
        case 't' : {
            switch( var->getU16(2) ) {
            case 11: rst->add("entry");	break;
            case 20: rst->add("timeline");	break;
            }
        } break;
        case 'v' : {
            switch( var->getU16(2) ) {
            case 0: rst->add("httpServer");	break;
            case 1: rst->add("socket");		break;
            }
        } break;
        case 'x' : {
            switch( var->getU16(2) ) {
            case 11: rst->add("watcher");	break;
            case 21: rst->add("pair");		break;
            }
        } break;
        case 'g' : {
            rst->add("painter");
        } break;
        case 'l' : {
            rst->add("layout");
        } break;
        case 'P' : {
            switch( var->getU16(2) ) {
            case 0: rst->add("process");	break;
            case 1: rst->add("job");		break;
            }
        } break;
        default:
            rst->add("undefine");
            break;
        }
    } else {
        if( var->length()<64 ) {
            rst->add("string");
        } else {
            rst->format(32,"string(%d)",var->length() );
        }
    }
}

LPCC getObjecTypeName(StrVar* sv) {
    StrVar* rst=gOperVar.reuse(); //getStrVarTemp();
    getObjectType(sv,rst);
    return rst->get();
}
bool getTypeOfResult(LPCC ty, StrVar* sv, StrVar* rst, XFunc* fc) {
    if( rst==NULL ) return false;
    rst->setVar('3',0);
    if( slen(ty)==0 ) return false;
    if( ccmp(ty,"null") ) {
        rst->setVar('3', sv==NULL||SVCHK('9',0) ? 1: 0 ); // notset ||
    } else if( ccmp(ty,"pointer") ) {
        rst->setVar('3', SVCHK('s',1) ? 1 : 0);
    } else if( ccmp(ty,"stringRef") ) {
        rst->setVar('3', SVCHK('s',0) ? 1 : 0);
    } else if( ccmp(ty,"string") ) {
        rst->setVar('3', SVCHK('s',0) || (sv->charAt(0)!='\b' && sv->length()) ? 1: 0);
    } else if( ccmp(ty,"bool") ) {
        rst->setVar('3', SVCHK('3',0) || SVCHK('3',1) ? 1: 0);
	} else if( ccmp(ty,"layout") ) {
		if( SVCHK('n',0) ) {
			TreeNode* node=(TreeNode*)SVO;
			switch( node->xstat ) {
			case LTYPE_GRID:
			case LTYPE_ROW:
			case LTYPE_HBOX:
			case LTYPE_VBOX:			rst->setVar('3', 1); break;
			default:					rst->setVar('3', 0); break;
			}
		}

	} else if( ccmp(ty,"widget") ) {
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            rst->setVar('3', gwidget(node)? 1: 0);
        }
    } else if( ccmp(ty,"node") ) {
        rst->setVar('3', SVCHK('n',0) ? 1:0);
    } else if( ccmp(ty,"array") ) {
        rst->setVar('3', SVCHK('a',0) ? 1:0);
    } else if( ccmp(ty,"number") || ccmp(ty,"num") ) {
        if( SVCHK('0',0) || SVCHK('1',0) || SVCHK('4',0) ) {
            rst->setVar('3', 1);
        } else if( ccmp(ty,"num") ) {
            LPCC num=toString(sv);
            if( isnum(num) ) rst->setVar('3', 1);
        }
    } else if( ccmp(ty,"funcRef") ) {
        if( SVCHK('f',1) ) {
            rst->setVar('3', 1);
        }
    } else if( ccmp(ty,"funcNode") ) {
        if( SVCHK('f',0) ) {
            rst->setVar('3', 1);
        }
    } else if( ccmp(ty,"func") || ccmp(ty,"function") ) {
        // qDebug("typeof type=%s, %s\n", ty, toString(sv) );
        if( SVCHK('f',0) || SVCHK('f',1) ) {
            if( ccmp(ty,"function") && SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                if( fsrc->xfunc && fsrc->xfunc->getFuncSize()>0 ) {
                    rst->setVar('3', 1 );
                } else {
                    rst->setVar('3', 0 );
                }
            } else {
                rst->setVar('3', 1 );
            }
        }
        /*
        else if( fc ) {
            LPCC func = fc->getParam(0)->getValue();
            sv = getStrVar("fsrc",func);
            if( SVCHK('f',1) ) {
                rst->setVar('3', 1 );
            }
        }
        */
    } else if( ccmp(ty,"rect") ) {
        rst->setVar('3', SVCHK('i',5)||SVCHK('i',4) ? 1: 0);
    } else if( ccmp(ty,"point") || ccmp(ty,"size") ) {
        rst->setVar('3', SVCHK('i',20)||SVCHK('i',2) ? 1: 0);
    } else if( ccmp(ty,"color") ) {
        rst->setVar('3', SVCHK('i',3) ? 1: 0);
    } else if( ccmp(ty,"pair") ) {
        rst->setVar('3', SVCHK('x',21) ? 1: 0);
    } else if( ccmp(ty,"image") ) {
        rst->setVar('3', SVCHK('i',9)||SVCHK('i',6) ? 1: 0);
    } else if( ccmp(ty,"png") ) {
        rst->setVar('3', SVCHK('i',6) ? 1: 0);
    } else if( ccmp(ty,"ref") ) {
        rst->setVar('3', SVCHK('s',0) ? 1: 0);
    } else if( ccmp(ty,"timeline") ) {
        rst->setVar('3', SVCHK('t',20) ? 1: 0);
    } else if( ccmp(ty,"menu") ) {
        rst->setVar('3', SVCHK('w',12) ? 1: 0);
    }

    if( checkFuncObject(rst,'3',1) ) {
        return true;
    }
    return false;
}

void getTypeOf(XFunc* fc, StrVar* rst, XFuncNode* fn ) {
    if( fc==NULL ) return;
    int cnt=fc->getParamSize();
    if( cnt==0 ) return;

    XFunc* param=fc->getParam(0);
    StrVar* sv=gOperVar.reuse(); //getStrVarTemp();
    execParamFunc(param, fn, sv);

    if( cnt>1 ) {
        XFunc* param=fc->getParam(1);
        LPCC varType=param->getValue();
        if( fn && ccmp(varType,"pointer") && cnt==3 ) {
            LPCC varName=toString(sv);
            sv=fn->gv("@this");
            if(SVCHK('n',0)) {
                TreeNode* thisNode=(TreeNode*)SVO;
                sv=thisNode->GetVar(varName);
                rst->setVar('s',1,(LPVOID)sv);
            } else {
                qDebug("#9#typeOf pointer error this object not define");
                rst->setVar('9',0);
            }
            return;
        }

        rst->setVar('3',0);
        for( int n=1;n<cnt;n++ ) {
            param=fc->getParam(n);
            if( getTypeOfResult(param->getValue(), sv, rst, fc) ) {
                break;
            }
        }
        return;
    } else {
        if( SVCHK('9',0) ) {
            rst->set("null");
            return;
        }
        getObjectType(sv,rst);
    }
}

void nodeInjectVar(XParseVar* pv, TreeNode* node, XFuncNode* fn, XFuncNode* fnInit ) {
    if( pv==NULL ) return;
    StrVar* sv=NULL;
    int sp=0,ep=0;
    if( fnInit==NULL && isPageNode(node) ) {
        sv=node->gv("onInit");
        if( SVCHK('f',0)) fnInit=(XFuncNode*)SVO;
    }
    while( pv->valid() ) {
        char ch=pv->ch();
        if( ch==0 || IS_OPERATION(pv->ch()) ) {
            break;
        }
        bool bok=true;
        LPCC code=NULL, name=NULL;
        sp=pv->start, ep=0;
        ch=pv->moveNext().ch();
        if( ch=='@' || ch=='#' ) {
            bok=false;
            code=pv->incr().NextWord();
            ep=pv->start;
            name=pv->Trim(sp,ep);
        } else {
            ep=pv->start;
            if( sp<ep ) {
                code=pv->Trim(sp,ep);
            }
        }
        ch=pv->ch();
        if( ch==':' || ch=='=' ) {
            pv->incr().ch();
            sp=pv->start, ep=0;
            ch=pv->moveNext().ch();
            if( ch=='@' || ch=='#' ) {
                bok=false;
                pv->incr().moveNext();
            }
            ep=pv->start;
            if( sp<ep ) {
                name=pv->Trim(sp,ep);
            }
        } else if( name==NULL ) {
            name=code;
        }
        sv=fnInit && bok ? fnInit->gv(name): NULL;
        if( sv==NULL ) {
            sv = node->gv(name);
            if( sv==NULL ) {
                sv=node->gv(gBuf.fmt("@%s",name));
            }
        }
        fn->GetVar(code)->reuse()->add(sv);
        if( pv->ch()==',' ) pv->incr();
    }
}
