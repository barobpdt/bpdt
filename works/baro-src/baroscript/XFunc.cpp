#include "XFunc.h"
#include "func_util.h"
#include "../util/user_event.h"
#include "../fobject/FMutex.h"
// QT_BEGIN_NAMESPACE_XLSX
StrVar gReadBuffer;
XFunc gfc;
FMutex gMutexFuncSrc;
FMutex gMutexFunc;
FMutex gMutexFuncNode;

/************************************************************************/
/* XFuncNode                                             */
/************************************************************************/
inline bool isInPos(TextPos* tp, int pos)		{
    return pos>=tp->start && pos<=tp->end ? true : false;
}
inline int findParamPos(XParseVar& pv) {
    int pb = pv.FindPos("\b");
    int pf = pv.FindPos("\f", pb);
    return (pf>0) ? pf : (pb>0) ? pb : pv.wep;
}
inline bool cncmp(LPCC data, int datasize,  LPCC find, int findsize ) {
    if( findsize==-1 )
        findsize = slen(find);
    if( datasize==findsize ) {
        for(int n=0; n<datasize; n++ )
            if( *find++!=*data++ ) return false;
        return true;
    }
    return false;
}
inline LPCC addSpace( int depth, char ch='\t') {
    StrVar* rst=gReadBuffer.reuse();
    for( int i=0;i<depth;i++ ) rst->add( ch);
    return rst->get();
}

void XFuncNode::reuseFuncNode() {
    initVar();
}

XFuncNode* XFuncNode::addfuncNode(XFunc* fc) {
    XFuncNode* fn = gfns.getFuncNode(fc);
    fn->xparent = this;
    xchilds.add(fn);
    return fn;
}


void XFuncNode::initVar() {
    /*
    if( uom.getInfo()->isNodeFlag(CNF_DEBUG) ) {
        for( WBoxNode* bn = First(); bn ; bn = Next() ) {
            bn->value.reuse();
            bn->type=0;
            bn->uid=0;
        }
    } else {
    }
    */
    for( WBoxNode* bn = First(); bn ; bn = Next() ) {
        bn->value.close();
    }
    clearNodeFlag(FLAG_RUN|FLAG_CALL );
    xtype = xstat = 0;
    xfunc = NULL;
    // xnode = NULL;
    xchilds.reuse();
    xparent = NULL;
    reuse();
}


int XFuncNode::call(XFunc* fc, StrVar* rst) {
    // static long prevFnCall=0;
    if( isNodeFlag(FLAG_CALL) ) {
        // int dist=GetTickCount()-prevFnCall;
        int dist=GetTickCount()-xtick;
        if( dist > 200 ) {
            qDebug("#0## [%s] XFuncNode::call skip dist=%d\n", getCf()->get("@currentFuncName"), dist);
            clearNodeFlag(FLAG_CALL);
        }
        return 0;
    }
    xtick=GetTickCount();
    // prevFnCall=GetTickCount();
    xrst.reuse();
    if( fc==NULL )
        fc = xfunc;
    if( rst==NULL )
        rst = &xrst;
    if( fc==NULL ) {
        qDebug("\n=========================================\nXFuncNode::call function is null\n=========================================\n");
        return 0;
    }
    setNodeFlag(FLAG_CALL);
    /*
    if( isNodeFlag(FLAG_PERSIST) && callEventFunc(this, fc, rst) ) {
        clearNodeFlag(FLAG_CALL);
        return 0;
    }
    */
    if( slen(get("_funcName_"))==0 ) {
        set("_funcName_", getCf()->get("@currentFuncName") );
        // qDebug("func name : %s",  getCf()->get("@currentFuncName"));
    }
    if( fc->xfuncRef==0 || fc->xfuncRef==9001 ) {
        int callbackCnt=0;
        for( int n=0; n<fc->getFuncSize(); n++ ) {
            XFunc* fcCur=fc->getFunc(n);
            // version 1.0.2
            // if( fcCur->xftype==FTYPE_RETURN ) break;
            if( fcCur->xftype==FTYPE_SET && fcCur->xdotPos==0 ) {
                XFunc* p = fcCur->getParam(0);
                if( p && p->getType()==FTYPE_CALLBACK ) {
                    execFuncSet(fcCur, this, rst);
                    callbackCnt++;
                }
            }
        }
        if( fc->xfuncRef==0 ) {
            if( callbackCnt>0 ) {
                fc->xfuncRef=9001;
            } else {
                fc->xfuncRef=1;
            }
        }
    }
    int rtn = callSubFuncs(fc,this,rst);
    clearNodeFlag(FLAG_CALL);
    return rtn;
}

inline bool callIfTypeMap(XFunc* fcCur ) {
    U16 type=fcCur->getType();
    if( type==0 ) {
        fcCur->xftype=FTYPE_STR;
    } else if( type==FTYPE_SINGLEVAR ) {

        LPCC val=fcCur->getValue();
        int pos=IndexOf(val,'.');
        if( pos>0 ) {
            LPCC var=gBuf.add(val,pos);
            bool bupper = true;
            fcCur->xdotPos=pos;
            for( int n=0, len=slen(var); n<len; n++ ) {
                if( var[n]>90 || var[n]<65 ) {
                    bupper=false;
                    break;
                }
            }
            if( bupper ) {
                fcCur->xftype = FTYPE_DEFINE;
            } else if( ccmp(var,"this") ) {
                fcCur->xftype = FTYPE_THIS;
            } else  if( IS_DIGIT(var[0]) || var[0]=='-' ) {
                fcCur->xftype = FTYPE_NUMBER;
            } else {
                fcCur->xftype = FTYPE_VAR;
            }
        } else {
            if( ccmp(val,"true") || ccmp(val,"false") ) {
                fcCur->xftype = FTYPE_BOOL;
            } else  if( ccmp(val,"null") ) {
                fcCur->xftype = FTYPE_NULL;
            } else  if( IS_DIGIT(val[0]) || val[0]=='-' ) {
                int size=slen(val);
                if( size>1 && (val[1]=='x'|| val[1]=='X') ) {
                    LPCC hex = gBuf.add(val+2,size-2);
                    fcCur->xfuncRef = (U32)strtol(hex, NULL, 16);
                }
                fcCur->xftype = FTYPE_NUMBER;
            } else {
                fcCur->xftype = FTYPE_VAR;
            }
        }
    }
    return true;
}

int XFuncNode::callIf(XFunc* fc, StrVar* rst, bool& bcalled) {
    int rtn=0;
    if( funcTrueCheck(fc,this,rst) ) {
        rtn = callSubFuncs(fc,this,rst);
        bcalled = true;
    }
    return rtn;
#ifdef SETIF_USE
    XFunc* pfc=fc->parent();
    if( pfc && pfc->getType()==FTYPE_SET ) {
        XFunc* fcCheck=pfc->nextFunc();
        bool bctrl= ( fcCheck==NULL || fcCheck->getType()==FTYPE_CONTROL || pfc->getFuncSize() ) ? true: false;
        bool bok=false;
        XFunc* fcNext= bctrl ? pfc: fcCheck;
        rtn=100;
        if( bctrl==false ) {
            rtn++;
        }
        for(int n=0; n<512 && fcNext ; n++ ) {
            if( fcNext!=pfc ) {
                fcNext->xfflag|=0x80;
            }
            if( bchk && bok==false ) {
                if( bctrl ) {
                    callSubFuncs(fcNext,this,rst);
                } else {
                    if( n==0 ) {
                        callIfTypeMap( fcNext);
                        execParamFunc(fcNext, this, rst->reuse() );
                    } else {
                        if( fcNext->getFuncSize()==1 ) {
                            XFunc* fcCur=fcNext->getFunc(0);
                            if( fcCur ) {
                                callIfTypeMap( fcCur );
                                execParamFunc(fcCur, this, rst->reuse() );
                            }
                        } else {
                            callSubFuncs(fcNext,this,rst);
                        }
                    }
                }
                bok=true;
            }
            /*
            if( bctrl && n==0 ) {
                fcNext=pfc->nextFunc();
            } else {
                fcNext=fcNext->nextFunc();
            }*/
            fcNext=fcNext->nextFunc();
            if( fcNext && fcNext->getType()==FTYPE_CONTROL ) {
                U16 kind=fcNext->xfkind;
                if( kind==5 || kind==6 || kind==7 ) {
                    // fcNext->xfflag|=0x80;
                    rtn++;
                } else {
                    break;
                }
            } else {
                break;
            }
            if( bok==false ) {
                getControlFuncKind(fcNext);
                if( fcNext->xfkind==5 ) {
                    bchk=true;
                } else if( fcNext->xfkind==6 ) {	// else if
                    bchk = isFuncTrue(fcNext,this,rst);	// chk = checkExprValue(next,fn,rst); //
                } else if( fcNext->xfkind==7 ) {	// else ifThis
                    bchk = callIfCheck(fcNext,this,rst);
                }
            }
        }
        if( bok==false ) {
            rst->reuse();
        }
        return rtn;
    }
#endif

}

inline bool isNumberCheck(LPCC val, int num ) {
    return isnum(val) && atoi(val)==num ? true : false;
}

int XFuncNode::callSwitch(XFunc* fc, StrVar* rst) {
    int rtn = 0;
    if( fc->getParamSize()==0 )
        return 0;

    int cnt = fc->xsubs.size();
    LPCC val = NULL;
#ifdef SWITCH_SUBFUNC_USE
    val=toString(rst);
    if( slen(val)==0 ) {
        XFunc* cur = fc->getFunc(cnt-1);
        if( cur->xfkind==9 ) {
            return callSubFuncs(cur,this,rst);
        }
        /*
        else if( cur->xftype==0 && cur->xfkind==0 ) {
            StrVar* sv=this->gv("@default");
            if( !SVCHK('f',1) ) {
                sv=this->gv("default");
            }
            if( SVCHK('f',1) ) {
                callLocalFunction(sv, this, rst, NULL);
            }
        }
        */
        return 0;
    }
    // ### version 1.0.4
    if( cnt==0 || cnt==1 ) {
        XFunc* cur = cnt==0? NULL : fc->getFunc(0);
        if( cur==NULL || (cur->xftype==0 && cur->xfkind==0) ) {
            XFunc* pfc=fc->parent();
            if( pfc && pfc->xfuncRef==9001 ) {
                pfc->xfuncRef=9000;
            }

            bool bSub=false;
            /* ex) switch('iClassName::func'); */
            int pos=IndexOf(val,':');
            if( pos>0 ) {
                if( val[0]=='i' && isUpperCase(val[1]) ) {
                    bSub=true;
                }
                else {
                    StrVar* sv=this->gv("subClassFunc");
                    bSub=SVCHK('3',1);
                }
                if( !execCallSubfunc(val, NULL, this, NULL, rst, bSub, false, true ) ) {
                    rst->setVar('3',0);
                    /*
                    if( bSub ) {
                        execCallSubfunc("_class", "other", this, NULL, rst, bSub, false, true );
                    } else {
                        execCallSubfunc(gBuf.add(val,pos), "other", this, NULL, rst, bSub, false, true );
                    }
                    */
                }
            } else {
                StrVar* sv=this->gv("subClassFunc");
                bSub=SVCHK('3',1);
                if( !execCallSubfunc(toString(getFuncVar(this,"_funcName_")), val, this, NULL, rst, bSub, false, true ) ) {
                    rst->setVar('3',0);
                    // execCallSubfunc(toString(getFuncVar(this,"_funcName_")), "other", this, NULL, rst, bSub, false, true );
                }
            }
            // funcNodeDelete(this);

            return FTYPE_BREAK;
        }
    }
#endif
    XFunc* def = NULL;
    execParamFunc(fc->getParam(0),this,rst);
    // XFunc* cur = NULL;
    int num = -1;
    if( isNumberVar(rst)) {
        num=toInteger(rst);
    } else {
        val=toString(rst);
    }
    for( int n=0;n<cnt;n++ ) {
        XFunc* cur = fc->getFunc(n);
        if( cur->xfkind==9 ) {
            def = cur;
        } else {
            if( num!=-1 && (cur->xfkind==1 || cur->xfkind==6) ) {
                if( cur->xfuncRef==(U32)num ) {
                    def = cur;
                    break;
                }
            } else if( cur->xfkind==2 ) {
                if( ccmp(cur->getValue(),val) ) {
                    def = cur;
                    break;
                }
            } else if( cur->xfkind==5 ) {
                XParseVar* pv = &(cur->xpv);
                pv->setPos();
                pv->incr();
                int ep = pv->wep-1;
                while( pv->start< ep ) {
                    if( ccmp(pv->getNext(),val) ) {
                        def = cur;
                        break;
                    }
                    if( pv->ch()==',') pv->incr();
                    else break;
                }
                if( def ) break;
            }
        }
    }
    if( def ) {
        rtn = callSubFuncs(def,this,rst);
    }
    if( rtn==FTYPE_CONTINUE || rtn==FTYPE_BREAK )
        rtn = 0;
    return rtn;
}


int XFuncNode::callFor(XFunc* fc, StrVar* rst) {
    if( !fc )
        return 0;
    int psize=fc->xparams.size();
    int rtn = 0;
    if( psize==0 ) {
        qDebug("#9#while param error (param count:%d)", psize);
        return FTYPE_RETURN;
    }

    XFunc* param = fc->getParam(0);
    int sp=0, ep=0xFFFF;

    if( param->getParam(0) && param->getParam(0)->getType()==FTYPE_OPER  ) {
        // ex) while(n=0,n<sz,n++) or while(n=0, range(10))
        if( param->xftype!=FTYPE_VAR || param->xdotPos>0 ) {
            qDebug("#9#while var type error");
            return FTYPE_RETURN;
        }
        char vid[32]={0,};
        LPCC vnm=param->getValueBuf(vid,32);
        if( slen(vnm)==0 ) {
            qDebug("#9#while var error");
            return FTYPE_RETURN;
        }
        execParamFunc(param, this, rst);
        if( isNumberVar(rst) ) {
            sp=toInteger(rst);
        } else {
            sp=0;
            this->GetVar(vnm)->reuse()->add(rst);
        }
        // strncpy(vid, vnm, 32);
        if( psize==2) {
            StrVar* sv=rst->reuse();
            execFunc(fc->getParam(1),this,sv);
            if(isNumberVar(sv)) {
                ep=toInteger(sv);
                if(sp<ep ) {
                    for( int n=sp; n<ep; n++ ) {
                        GetVar(vnm)->setVar('0',0).addInt(n);
                        rtn = callSubFuncs(fc,this,rst);
                        // if( rtn==FTYPE_CONTINUE ) continue;
                        if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN )
                            break;
                    }
                } else if(sp>ep ) {
                    for( int n=sp; n>=ep; n-- ) {
                        GetVar(vnm)->setVar('0',0).addInt(n);
                        rtn = callSubFuncs(fc,this,rst);
                        // if( rtn==FTYPE_CONTINUE ) continue;
                        if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN )
                            break;
                    }
                }
            } else if(SVCHK('a',0) ) {
                XListArr* arr = (XListArr*)SVO;
                ep=arr->size();
                for( int n=sp; n<ep; n++ ) {
                    GetVar(vnm)->reuse()->add(arr->get(n));
                    rtn = callSubFuncs(fc,this,rst);
                    // if( rtn==FTYPE_CONTINUE ) continue;
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
            } else if(SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                ep=node->childCount();
                for( int n=sp; n<ep; n++ ) {
                    GetVar(vnm)->setVar('n',0,(LPVOID)node->child(n));
                    rtn = callSubFuncs(fc,this,rst);
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
            } else {
                sv=getStrVarPointer(sv,&sp,&ep);
                for( int n=sp; n<ep; n++ ) {
                    GetVar(vnm)->reuse()->add(sv->charAt(n));
                    rtn = callSubFuncs(fc,this,rst);
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
            }
        } else if( psize==3 ) {
            XFunc* fcCheck = fc->getParam(1);
            XFunc* fcIncr = fc->getParam(2);
            for( int n=sp; n<ep; n++ ) {
                if( !isFuncTrue(fcCheck, this, rst) ) {
                    break;
                }
                rtn = callSubFuncs(fc,this,rst);                
                if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN )
                    break;
                execParamFunc(fcIncr, this, rst);
            }
        } else {
            qDebug("#9#while param error (param count:%d)", psize);
            return FTYPE_RETURN;
        }
    } else if( param->xftype==FTYPE_FUNC ) {
        // ex) while(s.valid(), idx)
        XFunc* fcCheck=param;
        LPCC vnm=NULL;
        char vid[32]={0,};
        param = fc->getParam(1);
        if( param && param->xftype==FTYPE_VAR ) {
            vnm=param->getVarBuf(vid,32);
            GetVar(vnm)->setVar('0',0).addInt(0);
        }
        for( int n=sp;n<ep; n++ ) {
            if( vnm ) GetVar(vnm)->setVar('0',0).addInt(n);
            if( !isFuncTrue(fcCheck, this, rst) ) {
                break;
            }
            rtn = callSubFuncs(fc,this,rst);
            // if( rtn==FTYPE_CONTINUE ) continue;
            if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN )
                break;
        }
    } else if( param->xftype==FTYPE_VAR ) {
        if( psize==1) {
            if( param->getType()==FTYPE_VAR || param->getType()==FTYPE_NUMBER ) {
                execFunc(param,this,rst);
                if(isNumberVar(rst) ) {
                    ep=toInteger(rst);
                } else {
                    ep=0;
                }
            }
            for(int n=0; n<ep; n++ ) {
                rtn = callSubFuncs(fc,this,rst);
                // if( rtn==FTYPE_CONTINUE ) continue;
                if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN )
                    break;
            }
        } else {
            LPCC vnm=NULL, curNm=NULL;
            char cid[32]={0,};
            char vid[32]={0,};
            param = fc->getParam(0);
            if( param ) {
                curNm=param->getVarBuf(cid,32);
            }
            param = fc->getParam(2);
            if( param && param->xftype==FTYPE_VAR ) {
                vnm=param->getVarBuf(vid,32);
                GetVar(vnm)->setVar('0',0).addInt(0);
                ep=0xFFFFF;
            }
            StrVar* sv = rst->reuse();
            execFunc(fc->getParam(1), this, sv);
            if(SVCHK('a',0) ) {
                XListArr* arr = (XListArr*)SVO;
                ep=arr->size();
                for( int n=sp; n<ep; n++ ) {
                    if(slen(vnm)) GetVar(vnm)->setVar('0',0).addInt(n);
                    sv=GetVar(curNm)->reuse();
                    sv->add(arr->get(n));
                    rtn = callSubFuncs(fc,this,rst);
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
            } else if(SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                ep=node->childCount();
                for( int n=sp; n<ep; n++ ) {
                    TreeNode* cur=node->child(n);
                    if( vnm ) GetVar(vnm)->setVar('0',0).addInt(n);
                    sv=GetVar(curNm);
                    sv->setVar('n',0,(LPVOID)cur);
                    rtn = callSubFuncs(fc,this,rst);
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
            } else {
                /*
                sv=getStrVarPointer(sv,&sp,&ep);
                for( int n=sp; n<ep; n++ ) {
                    if( slen(vnm) ) GetVar(vnm)->setVar('0',0).addInt(n);
                    GetVar(curNm)->reuse()->add(sv->charAt(n));
                    rtn = callSubFuncs(fc,this,rst);
                    if( rtn==FTYPE_CONTINUE ) continue;
                    if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN ) break;
                }
                */
                qDebug("#0#while object error--> %s, %s [%s]", curNm, vnm, toString(sv));
                return 0;
            }
        }
    } else {
        qDebug("#9#while param error (param count:%d)", psize);
        return FTYPE_RETURN;
    }
    return rtn;
}


int XFuncNode::setJson(XParseVar& pv, XFuncNode* fn) {
    char ch = pv.ch();
    if( fn==NULL ) fn = this;
    if( ch=='{' ) {
        pv.start++;
        setJson(pv,fn);
    }
    int size = 0;
    LPCC next = NULL;
    while( pv.valid() ) {
        ch = pv.ch();
        if( (ch=='\'' || ch=='"')  && pv.MatchStringPos(ch) )
            next = pv.v();
        else
            next = pv.getNext();
        ch = pv.ch();
        if( ch==':' ) {
            pv.start++;
            ch = pv.ch();
            if( (ch=='\'' || ch=='"')  && pv.MatchStringPos(ch) ) {
                LPCC val = pv.left(&size);
                GetVar(next)->set(val, size);
            } else if( ch=='&' ) {
                pv.start++;
                LPCC code = pv.getNext();
                StrVar* rst = getFuncVar(fn, code);
                GetVar(next)->reuse()->add(rst);
            } else  { //if( pv.findPos(" \t\n",0,FIND_CHARS)!=-1 )
                pv.findEnd(",");
                GetVar(next)->set(pv.left());
                if( pv.cur<pv.wep ) pv.start = pv.cur;
            }
            ch = pv.ch();
            if( ch=='}' ) {
                pv.start++;
                break;
            }
            if( ch==',' || ch==';' ) pv.incr();
            else break;
        } else
            break;
    }
    return childCount();
}



/************************************************************************/
/* XFuncSrc                                                             */
/************************************************************************/

XFuncSrc::XFuncSrc() : xfunc(NULL) {
    xflag=0;
}


bool XFuncSrc::err(int sp, LPCC msg) {
    // version 1.0.4 ### error check ###
    LPCC src=get();
    qDebug()<<"#9##error : pos:"<< sp << " message:"<< QString::fromLocal8Bit(msg) << "src:\r\n"<< src <<"\r\n\r\n";
    return false;
}


bool XFuncSrc::readBuffer(StrVar* var, int sp, int ep) {
    XParseVar pv(var,sp,ep);
    if( ep==0 )
        ep=pv.wep;
    ParseText p(pv);
    int first = 0;
    p.AddComment("/*","*/",sp,ep);
    p.AddLineComment("// ",sp,ep);
    if( p.poss.size() > 0 ) {
        StrVar* sv = gReadBuffer.reuse(); //gum.getStrVar(true); // getCf()->GetVar("@src");
        p.MakeText(sv, first);
        parseData(sv,0,sv->length());
        // parseData(&src,0,src.length());
    } else {
        parseData(var,sp,ep);
    }
    return true;
}

bool XFuncSrc::readFile(LPCC filenm) {
    StrVar* tmp = getStrVarTemp();
    tmp->readFile(filenm);
    bool rtn = readBuffer((StrVar*)tmp);
    return rtn;
}

int XFuncSrc::isFunc(StrVar* data, int sp, char& ft ) {
    if( data==NULL || sp>=data->length() )
        return 0;
    xpv.SetVar( data, sp );
    if( !xpv.NextMove() )
        return 0;

    int ep = xpv.start;
    if( xpv.StartWith("else",xpv.prev) ) {
        if( xpv.ch()=='i' && xpv.ch(1)=='f' ) {
            xpv.NextMove();
            if( xpv.ch()!='(' ) {
                xpv.start = ep;
            }
        } else {
            xpv.start = ep;
        }
        return xpv.start;
    }
    char c = xpv.ch(); // xpv.NextChar();
    sp = xpv.start;
    // version 1.0.2 => xxx#1()
    /**/
    if( c=='#' ) {
        xpv.start++;
        xpv.NextMove();
        ep=xpv.start;
        c=xpv.ch();
        sp=xpv.start;
    }

    if( c=='(') {
        if( sp!=xpv.start ) {
            ft = 'X';
        }
        return ep;
    }
    else if( c=='.' ) {
        xpv.start++;
        bool chk = false;
        for( int n=0;n<4;n++) {
            if( !xpv.NextMove() )
                return 0;
            ep = xpv.start;
            c = xpv.ch(); // xpv.NextChar();
            if( ep!=xpv.start )
                chk = true;
            if( c=='#' ) {
                xpv.start++;
                xpv.NextMove();
                ep=xpv.start;
                c=xpv.ch();
            }
            if( c=='(') {
                if( chk )
                    ft = 'X';
                return ep;
            }
            else if( c!='.' )
                return 0;
            xpv.start++;
            sp = xpv.start;
            xpv.NextChar();
            if( sp!=xpv.start )
                chk = true;

        }
    }
    return 0;
}

int XFuncSrc::endVarPos(StrVar* data, int sp ) {
    if( data==NULL || sp>=data->length() )
        return 0;
    xpv.SetVar( data, sp );
    if( !xpv.NextMove() )
        return 0;
    sp = xpv.start;
    char c = xpv.CharAt(sp);
    // version 1.0.2 => xxx#1()
    if( c=='#' ) {
        xpv.start++;
        xpv.NextMove();
        sp=xpv.start;
        c=xpv.ch();
    }
    if( c=='.' ) {
        xpv.start++;
        for( int n=0;n<3;n++) {
            if( !xpv.NextMove() ) return 0;
            sp = xpv.start;
            c = xpv.CharAt(sp);
            if( c=='#' ) {
                xpv.start++;
                xpv.NextMove();
                sp=xpv.start;
                c=xpv.ch();
            }
            if( c!='.' ) return sp;
            xpv.start++;
        }
    }
    return sp;
}

int XFuncSrc::subOper(StrVar* data, char c, int sp, int ep, char ft, U32* endchk) {
    XParseVar pv(data, sp, ep);
    bool rtn = true;
    bool end = false;
    if( endchk ) *endchk = 0;
    if( c=='\'' || c=='\"' || c=='`' ) {
        if( pv.MatchStringPos(c) ) {
            if( c=='\'' || c=='`' )
                add("\b=").add(pv.left()).add("\b");
            else
                add("\bT").add(pv.left()).add("\b");
        } else {
            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
            rtn = err(pv.start, gBuf.fmt("%s string match error",xx) );
        }
        c = pv.NextChar();
        if( !isoper(c) ) end = true;
    } else if( c=='(') {
        if( pv.MatchWordPos("(",")",FIND_SKIP) ) {
            add("\b(");
            XParseVar sub(data, pv.prev,pv.cur);
            makeSub(sub, ft);
            add("\b)");
        } else {
            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
            rtn = err(pv.start, gBuf.fmt("%s () braket match error",xx) );
        }
        c = pv.NextChar();
        if( !isoper(c) ) end = true;
    } else if( c=='[') {

        if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
            add("\bh[").add(pv.GetValue(pv.prev), (pv.cur-pv.prev)).add("]\b");
        } else {
            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
            rtn = err(pv.start, gBuf.fmt("oprercompare error %s [] match error",xx) );
        }
        c = pv.NextChar();
        if( c!=';' ) end = true;
    } else if( c=='{') {
        if( pv.MatchWordPos("{","}",FIND_SKIP) ) {
            add("\bh").add( pv.GetValue(pv.prev), (pv.cur-pv.prev) ).add("\b");
        } else {
            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
            rtn = err(pv.start, gBuf.fmt("compare error %s {} hash match error",xx) );
        }
        c = pv.NextChar();
        if( c!=';' ) end = true;
    } else {
        // int p = length()-1;
        if( (c=='-'||c=='+') && charAt(-1)=='\f' ) {
            pv.start++;
            sp=pv.start;
            if( pv.moveNext().ch()=='.' ) {
                pv.incr().moveNext();
            }
            LPCC val=pv.Trim(sp,pv.start );
            add("\bn").add(c).add(val).add("\b");
        } else {
            char next = pv.CharAt(pv.start+1);
            if( isoper(c) && isoper(next) ) {
                add("\b@").add(c).add(next).add("\b"); pv.start+=2;
            } else {
                if( ft=='C' ) {
                    if( c==':' || c==';' )
                        add('\f');
                    else
                        add("\b@").add(c).add("\b");
                    pv.start++;
                } else {
                    bool badd=true;
                    if( c=='&' ) {
                        char ft=0;
                        sp = pv.start;
                        ep = isFunc(data, sp+1, ft);
                        if( ep>0 ) {
                            pv.start = ep;
                            c = pv.NextChar();
                            if( c=='(' && pv.MatchWordPos("(",")",FIND_SKIP) ) {
                                makeFunc(pv.GetVar(), sp, ep, pv.prev, pv.cur,ft);
                                badd=false;
                            } else if(sp<ep ) {
                                LPCC xx=gBuf.add(data->get(sp), ep-sp);
                                return err(ep, gBuf.fmt("compare error func:%s",xx) );
                            }
                        } else {
                            sp=pv.start;
                            pv.incr();
                            LPCC funcNm=pv.NextWord();
                            StrVar* sv=getStrVar("fsrc", funcNm);
                            if( SVCHK('f',1) ) {
                                add("\bV&").add(funcNm);
                                badd=false;
                            } else {
                                pv.start=sp;
                            }
                        }
                    }
                    if( badd ) {
                        add("\b@").add(c).add("\b");
                        pv.start++;
                    }
                }

            }
        }
    }
    if( endchk && end )
        *endchk = 1;

    return (rtn ) ? pv.start : -1;
}

inline LPCC getFuncNameValue(StrVar* data, int sp, int ep) {
    int size = ep-sp;
    if( ep<=sp || size>256 )
        return NULL;
    char* buf = gBuf.get(size);
    int n = 0;
    for( ;sp<ep; sp++ ) {
        if( ISBLANK(data->charAt(sp)) ) continue;
        buf[n++] = data->charAt(sp);
    }
    buf[n] = 0;
    return (LPCC)buf;
}

bool XFuncSrc::makeFunc(StrVar* data, int sp, int ep, int psp, int pep, char& ft ) {
    // LPCC fnm = ft=='X'? getFuncNameValue(data,sp,ep): data->get(sp,ep);
    if( sp>=ep )
        return false;

    LPCC fnm = data->trimv(sp,ep);
    int pos=IndexOf(fnm,'.');
    if( fnm[0]=='&') {
        fnm++;
        format(64,"\bU&%s", fnm);
        ft='U';
    } else if( fnm[0]=='~') {
        ft='i';
        format(64,"\binot" );
    } else if( fnm[0]=='@') {
        fnm++;
        if(ccmp(fnm,"not") ) {
            ft='i';
            format(64,"\bi%s", fnm);
        } else {
            ft='U';
            format(64,"\bU@%s", fnm);
        }
    } else {
        if( ft=='n' ) {
            ft=0;
        } else if( pos==-1) {
            if( ft=='p' ) {
                ft='U';
            } else {
                if( ccmp(fnm,"fmt") || ccmp(fnm,"print") || ccmp(fnm,"args") || ccmp(fnm,"conf") || ccmp(fnm,"eq") ||
                     ccmp(fnm,"when") || ccmp(fnm,"typeof") || ccmp(fnm,"eval") || ccmp(fnm,"alert") || ccmp(fnm,"confirm") || ccmp(fnm,"flags") ||
                     ccmp(fnm,"nvl") || ccmp(fnm,"max") || ccmp(fnm,"min") || ccmp(fnm,"and") || ccmp(fnm,"or") ) ft='i';
                else if( ccmp(fnm,"func") || ccmp(fnm,"function") ) ft='b';
                else if( STARTWITH(fnm,"else",size) || ccmp(fnm,"if") || ccmp(fnm,"not") || ccmp(fnm,"for")
                           || ccmp(fnm,"while") || ccmp(fnm,"switch")) ft='C';
            }
        }
        if( !ft ) {
            if( pos>0 ) {
                if( isUpperCase(fnm[0]) ) {
                    ft=( ccmpl(fnm,3,"cf.",3) || ccmpl(fnm,"System.",7) || ccmpl(fnm,"Baro.",5) || ccmpl(fnm,"Math.",5) ) ? 'p' : 'U';
                } else {
                    ft='U';
                }
            } else {
                ft = getFuncType(fnm);
            }

        }
        if( ft=='b' ) { //  || ft=='c' || ft=='^'
            // callback, func, template
            return true;
        }
        format(64,"\b%c%s", ft, fnm);
    }
    if( psp && pep ) {
        XParseVar pv(data, psp, pep);
        char c = pv.NextChar();
        if( c==0 ) {
            add("\b/");
            return true;
        }

        add('\f');
        bool bchk=true;
        if( pos>0 ) {
            fnm+=pos+1;
            if( ccmp(fnm,"with") ) {
                if( c=='\'' && c=='\"' && c=='`' ) {

                } else {
                    add("\bt").add(pv.GetVar(),psp, pep).add("\b");
                    bchk=false;
                }
            }
        } else if( ccmp(fnm,"with") ) { //
            if( c=='\'' && c=='\"' && c=='`' ) {

            } else {
                add("\bt").add(pv.GetVar(),psp, pep).add("\b");
                bchk=false;
            }
        }
        if( bchk && !makeSub(pv, ft) ) {
            return false;
        }
    }
    if( ft!='M' )
        add("\b/");
    return true;
}
bool XFuncSrc::makeSub(XParseVar& pv, char ft) {
    // version 1.0.2
    char c;
    int sp, ep;
    while( pv.start<pv.wep ) {
        c = pv.ch(); //(na) pv.NextChar();
        if( c==0 ) break;
        if( c==';' && ft=='C' ) {
            add("\f"); pv.start++;
        } else if( c==',' ) {
            add("\f"); pv.start++;

        } else if( // c!='.' &&  c!='@' && IS_OPERATION(c)
            c == '-' || c == '+' || c == '\'' || c == '\"'|| c == '`'||
            c == '(' || c == '{' || c == '['  || c=='&' || c=='|' || c=='<' || c=='>' || c=='=' || c=='!' ||
            c == '*' || c == '/' || c == '%' )
        {
            bool bchk=true;
            if( c=='+' || c=='-' || c=='&' ) {
                char prev = pv.prevChar();
                if( prev=='(' || prev==',' || prev=='=' ) {
                    sp = pv.start;
                    pv.start++;
                    pv.moveNext();
                    char ch=pv.ch();
                    if( ch=='.') {
                        pv.start++;
                        pv.moveNext();
                        ch=pv.ch();
                        if( ch=='#' ) {
                            ch=pv.incr().moveNext().ch();
                        }
                    }
                    if( ch=='(' || ch=='[' ) {
                        pv.start=sp;
                    } else {
                        ep = pv.start;
                        if( c=='&' ) {
                            LPCC fnm=pv.Trim(sp,ep);
                            format(64,"\bU%s\b/", fnm);
                        } else {
                            add("\b0").add(pv.GetChars(sp,ep)).add("\b");
                        }
                        bchk=false;
                    }
                }
            }
            if( bchk ) {
                ep = subOper(pv.GetVar(), c, pv.start, pv.wep, ft);
                if( ep==-1 )
                    return false;
                pv.start = ep;
            }
        } else {
            bool chkfn = true;
            if( c=='.' ) {
                add(".");
                chkfn = false;
                ft = 'U';
                pv.start++;
            }
            sp = pv.startPos();
            ep = isFunc(pv.GetVar(),sp,ft);
            if( ep>0 ) {
                pv.start = ep;
                c = pv.NextChar();
                if( c=='(' && pv.MatchWordPos("(",")",FIND_SKIP) ) {
                    if( chkfn ) {
                        ft = 0;
                    }
                    makeFunc(pv.GetVar(), sp, ep, pv.prev, pv.cur,ft);
                    if( ft=='b' ) { // || ft=='c' || ft=='^'
                        LPCC param=pv.v();
                        c=pv.ch();
                        if( c=='{' && pv.MatchWordPos("{","}",FIND_SKIP) ) {
                            add("\b").add(ft).add(param).add(';').add(pv.v()).add("\b");
                        }
                    }
                }
                else {
                    LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                    return err(pv.start, gBuf.fmt("%s make sub error",xx) );
                }
            } else {
                //(na) 2020
                ft = 0;
                if( c=='@' ) {
                    ep = endVarPos(pv.GetVar(), sp+1);
                } else {
                    ep = endVarPos(pv.GetVar(), sp);
                }
                pv.start = ep;
                c = pv.NextChar();
                if( c==':' ) {
                    c=pv.incr().moveNext().ch();
                    if(c=='.') pv.incr().moveNext();
                    LPCC var = pv.Trim(sp,pv.start);
                    qDebug("func param var ================ %s %c\n", var, c);
                    add("\bV").add(var).add("\b");
                } else if( c=='[' ) {
                    if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                        LPCC var = pv.Trim(sp,pv.start);
                        add("\ba").add(var).add("\b");
                    } else {
                        LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                        return err(pv.start, gBuf.fmt("%s make sub error",xx) );
                    }
                } else if(sp < ep) {
                    LPCC var = pv.Trim(sp,ep);
                    /*
                    if( ft=='e' ) {
                        add("\bE").add(var).add("\b");
                    } else
                    */
                    if( isUpperCase(var[0]) ) {
                        if( ep-sp>2 && isUpperCase(var[1]) ) {
                            add("\bD").add(var).add("\b");
                        } else {
                            add("\bV").add(var).add("\b");
                        }
                    } else if( isnum(var) ) {
                        add("\bn").add(var).add("\b");
                    } else {
                        add("\bV").add(var).add("\b");
                    }
                }
            }
        }
    }
    return true;
}


bool XFuncSrc::parseData(StrVar* data, int sp, int ep, bool sub) {
    if( data->length()==0 )
        return false;
    XParseVar pv(data, sp, ep);
    bool y = false;
    bool z = false;
    bool dot = false;
    // bool islib = false;
    // bool iscls = false;
    U32 endchk = 0;
    char c,ft=0;
    while( pv.start<pv.wep ) {
        c = pv.NextChar();
        if( c==0 ) {
            if( z && !sub ) {
                add("\b>");
                y = z = false;
            }
            break;
        }

        if( c=='=') {
            if( !z ) {
                return err(pv.start,"parse error (char: =)");
            }
            y = false;
            pv.start++;
            add('\f');
        } else if( c==';' || c==',' ) {
            pv.start++;
            if( z && !sub ) {
                add("\b>");
                y = z = false;
            }
        } else if( c=='.') {
            ft=0;
            dot = true;
            pv.start++;
            add('.');
        } else if( c=='{') {
            if( pv.MatchWordPos("{","}",FIND_SKIP) ) {
                // add("\binode\f\b#").add(pv.v()).add("\b\b/");
                add("\bh{").add(pv.v()).add("}\b");
                c=pv.ch();
                if( c==';' ) pv.start++;
                if( z && !sub ) {
                    add("\b>");
                    y = z = false;
                }
            } else {
                int size=pv.wep-pv.start;
                LPCC xx=gBuf.add(pv.GetValue(pv.start), size ); // min(32,size)
                return err(pv.start, gBuf.fmt("%s JSON not match(size:%d)",xx,size) );
            }
        } else if( c=='[') {
            if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                add("\bh[").add(pv.v()).add("]\b");
                c=pv.ch();
                if( c==';' ) pv.start++;
                if( z && !sub ) {
                    add("\b>");
                    y = z = false;
                }
            } else {
                LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                return err(pv.start, gBuf.fmt("%s array not match",xx) );
            }
        } else if( c=='\'' || c=='\"' || c == '`'|| c=='(' || c==')' || c=='+' ||  c=='-' || c=='*' || c=='/' || c=='%' || c=='|' ) { // c!='&' && c!='@' && IS_OPERATION(c) ) {
            if( !z && !sub ) {
                z = true;
                add("\b<");
                if( c=='-'||c=='+' ) {
                    pv.start++;
                    sp=pv.start;
                    if( pv.moveNext().ch()=='.' ) {
                        pv.incr().moveNext();
                    }
                    LPCC val=pv.Trim(sp,pv.start );
                    add("\bn").add(c).add(val).add("\b");
                    c=pv.ch();
                    if( c=='+' ||  c=='-' || c=='*' || c=='/' || c=='%' || c=='|' ) {
                        qDebug("operation check (oper:%c)\n", c);
                    } else {
                        add("\b>");
                        y = z = false;
                        continue;
                    }
                }
            }


#ifdef TEST_USE
            if( sub && c==',' || c==':' ) {
                if( c==',' )
                    add('\f');
                else
                    add('=');
                ep++;
            } else
#endif
            {
                ep = subOper(pv.GetVar(), c,pv.start, pv.wep, (char)0, &endchk );
                if( ep==-1 ) return false;
                if( endchk==1 ) {
                    add("\b>");
                    y = z = false;
                }
            }
            pv.start = ep;
            y = false;
        } else {
            bool bset=false;
            if( !z && !sub ) {
                add("\b<");
                bset = z = true;
            } else if( c=='&' ) {
                ep = subOper(pv.GetVar(), c,pv.start, pv.wep, (char)0, &endchk );
                if( ep==-1 ) return false;
                if( endchk==1 ) {
                    add("\b>");
                    y = z = false;
                }
                pv.start = ep;
                y = false;
                continue;
            }
            sp = pv.startPos();
            if( dot )
                ft = 'U';
            ep = isFunc(data, sp, ft);
            if( ep>0 ) {
                pv.start = ep;
                if( isEquals(pv.GetBuffer(sp),"return",ep-sp) ) {
                    add("\bR2\f");
                } else {
                    c = pv.NextChar();
                    // char ft = (islib ) ? 'L' : (iscls ) ? 'O'  : 0;
                    if( c=='(' && pv.MatchWordPos("(",")",FIND_SKIP) ) {
                        if( !bset && !ft ) {
                            ft='n';
                        }
                        makeFunc(data, sp, ep, pv.prev, pv.cur, ft);
                        if( ft=='b' ) { //  || ft=='c' || ft=='^'
                            LPCC param=pv.v();
                            c=pv.ch();
                            if( c=='{' && pv.MatchWordPos("{","}",FIND_SKIP) ) {
                                add("\b").add(ft).add(param).add(';').add(pv.v()).add("\b");
                                c=pv.ch();
                                if( c==';' ) pv.start++;
                                if( z && !sub ) {
                                    add("\b>");
                                    y = z = false;
                                }
                            }
                        }
                        if( c=='.') {
                            pv.start++;
                        }
                    }
                    else {
                        makeFunc(data, sp, ep, 0, 0, ft);
                    }
                    c = pv.NextChar();
                    if( z && ( ( c=='@' && ft!='C' )|| c==0) ) {
                        add("\b>");
                        if( c==0 ) {
                            break;
                        } else {
                            z = false;
                        }
                    }

                    if( c=='{' ) {
                        if( pv.MatchWordPos("{","}",FIND_SKIP) ) {
                            if( ft=='M' ) {
                                z=true;
                                add("\f\bh").add( pv.GetValue(pv.prev), (pv.cur-pv.prev) ).add("\b").add("\b/");
                            } else {
                                add("\b>\b{");
                                parseData(pv.GetVar(),pv.prev, pv.cur);
                                add("\b}");
                                z = false;
                            }
                        } else {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s function match error",xx) );
                        }
                    } else if( c=='[' ) {
                        if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                            add("\b>\b[").add( pv.GetValue(pv.prev), (pv.cur-pv.prev) ).add("\b]");
                            z = false;
                        } else {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s inline command match error",xx) );
                        }
                    } else if( ft=='C' ) {
                        z = false;
                        add("\b>");
                        if( c==';' ) {
                            add("\b<\bx\b/\b>");
                            pv.start++;
                        }
                    } else if( ft=='M' ) {
                        add("\b/");
                    } else if( z ) {
                        if( c!=';' && c!='.' ) {
                            if( !IS_OPERATION(c) ) {
                                add("\b>");
                                y = z = false;
                            }
                        }
                    }
                    ft = 0;
                    // islib = iscls = false;
                }
            } else {
                // char chk = c;
                // U32 ftype=0;
                if( (pv.CharAt(sp+1)=='[') && (c=='#' || c=='$') ) {
                    ep=sp+1;
                } else if( c=='@' ) {
                    ep = endVarPos(data,sp+1);
                } else {
                    ep = endVarPos(data,sp);
                }
                if( ep<=sp ) {
                    LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                    return err(pv.start, gBuf.fmt("%s function match error",xx) );
                }
                pv.start = ep;
                c = pv.NextChar();
                if( c=='=' ) {
                    add('$');
                    add(pv.Trim(sp,ep));
                    y = false;
                    pv.start++;
                    add('\f');
                    if( dot ) dot=false;
                    continue;
                } else if( c=='+' || c=='-' || c=='*' || c=='/' || c=='%' || c=='&' || c=='|' ) { //  || c==':'
                    char ch=pv.CharAt(pv.start+1);
                    if( ch=='=' ) {
                        add(c);
                        add(pv.Trim(sp,ep));
                        pv.start++;
                    } else if( ch=='+' || ch=='-') {
                        add("\bV").add(pv.Trim(sp,ep)).add("\b");
                        y = true;
                        add("\b@").add(c).add(ch).add("\b");
                        pv.start+=2;
                        ch=pv.ch();
                        // if( ch!=';' && ch!=',' && !IS_OPERATION(ch) && z && !sub ) {
                        if( z && !sub && !IS_OPERATION(ch) ) {
                            add("\b>");
                            y = z = false;
                        }
                    } else {
                        add("\bV").add(pv.Trim(sp,ep)).add("\b");
                        y = true;
                    }
                } else if( c=='{' ) {
                    LPCC var=pv.Trim(sp,ep);
                    add('$').add(var);
                    // qDebug("#0# local function : %s\n\n", var);
                    z=true;
                    if( pv.MatchWordPos("{","}",FIND_SKIP) ) {
                        add("\f\bb;").add(pv.GetValue(pv.prev), (pv.cur-pv.prev) ).add("\b\b>");
                        if( pv.ch()==';' ) {
                            pv.start++;
                        }
                        y = z = false;
                    } else {
                        LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                        return err(pv.start, gBuf.fmt("%s function match error",xx) );
                    }
                } else if( c=='[' ) {
                    if( ep<pv.start  ) {	// || (ftype==1)
                        LPCC var = pv.Trim(sp,ep);
                        sp = pv.start;
                        if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                            if( ccmp(var,"@")) {
                                add("\bb@call;").add(pv.GetValue(pv.prev), (pv.cur-pv.prev) ).add("\b\b>");
                                if( pv.ch()==';' ) {
                                    pv.start++;
                                }
                                y = z = false;
                            } else if( ccmp(var,"return")) {
                                add("\bR1\f").add("\ba").add(pv.Trim(sp,pv.start)).add('\b');
                            } else if( ccmp(var,"case") ) {
                                c = pv.NextChar();
                                if( c=='.') {
                                    pv.start++;
                                    pv.NextMove();
                                }
                                add("\bW").add(pv.GetBuffer(sp), pv.start-sp).add("\b");
                                if( c!=':' )  {
                                    return err(pv.start,"case match error");
                                }
                                add("\b>"); y = z = false;
                                pv.start++;
                            } else {
                                add("\ba").add(pv.Trim(sp,pv.start)).add('\b');
                                if( c!=';' && c!=','  && c!='.' ) {
                                    if( z && !IS_OPERATION(c) ) {
                                        add("\b>");
                                        y = z = false;
                                    }
                                }
                            }
                        } else {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s inline command match error",xx) );
                        }
                    } else {
                        if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                            c = pv.NextChar();
                            if( c=='=' ) {
                                add('A').add(pv.Trim(sp,pv.start));
                            } else {
                                add("\ba").add(pv.Trim(sp,pv.start)).add('\b');
                                if( c!=';' && c!=','  && c!='.' ) {
                                    if( z && !IS_OPERATION(c) ) {
                                        add("\b>");
                                        y = z = false;
                                    }
                                }
                            }
                        } else {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s inline command match error",xx) );
                        }
                    }
                } else {
                    if( y ) {
                        return err(ep,"parse data error(start)");
                    }
                    LPCC var = pv.Trim(sp,ep);
                    bool xchk=false;
                    if( slen(var)==0 ) {
                        LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                        return err(pv.start, gBuf.fmt("%s parse data error",xx) );
                    }
                    if( ccmp(var,"case") ) {
                        if( c=='\'') {
                            if( pv.MatchStringPos(c) ) {
                                add("\bw").add(pv.GetBuffer(pv.prev), pv.cur-pv.prev).add("\b");
                            } else {
                                LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                                return err(pv.start, gBuf.fmt("%s case parse data error",xx) );
                            }
                        } else if( c=='[') {
                            if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                                add("\bW").add(pv.GetBuffer(pv.prev), pv.cur-pv.prev).add("\b");
                            } else {
                                LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                                return err(pv.start, gBuf.fmt("%s case parse data error",xx) );
                            }
                        } else {
                            bool fchk = true;
                            sp = pv.start;
                            pv.NextMove();
                            c = pv.NextChar();
                            if( c=='.' ) {
                                pv.NextMove();
                            } else if( c=='(' ) {
                                while( pv.valid() ) {
                                    if( pv.ch()=='(' && pv.MatchWordPos("(",")",FIND_SKIP) ) {
                                        if( pv.ch()!='.') break;
                                        c = pv.ch();
                                    } else break;
                                    pv.NextMove();
                                }
                                fchk = false;
                                ft = 0;
                                add("\bW\f");
                                XParseVar pvSub(data,sp,pv.start);
                                makeSub(pvSub,ft);
                                add("\b");
                            }
                            if( fchk )
                                add("\bW").add(pv.GetBuffer(sp), pv.start-sp ).add("\b");
                        }
                        c = pv.NextChar();
                        if( c!=':' ) {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s case parse data error",xx) );
                        }
                        add("\b>"); y = z = false;
                        pv.start++;
                    } else if( ccmp(var,"default") ) {
                        add("\bW").add(var).add("\b");
                        c = pv.NextChar();
                        if( c!=':' ) {
                            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                            return err(pv.start, gBuf.fmt("%s default parse data error",xx) );
                        }
                        add("\b>"); y = z = false;
                        pv.start++;
                    } else if( ccmp(var,"break") ) {
                        add("\bB0\b");
                        xchk=true;
                    } else if( ccmp(var,"null") ) {
                        add("\bL0\b");
                        xchk=true;
                    } else if( ccmp(var,"continue") ) {
                        add("\bc0\b");
                        xchk=true;
                    } else if( ccmp(var,"return")) {
                        add("\bR1\f");
                    } else {
                        add("\bV").add(var).add("\b");
                        xchk=true;
                        /*
                        y = true;
                        c = pv.NextChar();
                        if( c!=';' && c!=',' && !IS_OPERATION(c) ) {
                            if( z && !sub ) {
                                add("\b>");
                                y = z = false;
                            }
                        }
                        */
                    }
                    c = pv.NextChar();
                    if( xchk ) {
                        y = true;
                        c = pv.NextChar();
                        if( c=='@' || c=='$' || !IS_OPERATION(c) ) {
                            if( z && !sub ) {
                                add("\b>");
                                y = z = false;
                            }
                        }
                    } else if( c==0 && z && !sub ) {
                        add("\b>");
                        break;
                    }
                }
            }
        }
        if( dot && c!='.' )
            dot = false;
    }
    return true;
}

int XFuncSrc::makeFuncCase(XParseVar& pv, U32 flag, XFunc** ppfunc, XFunc* pfunc, XFunc* pval ) {
    XFunc* func = NULL;
    XFunc* cur = NULL;
    U32 cf = 0;
    char c;
    int sp, ep;
    // bool not = false;
    while( pv.start<pv.wep ) {
        c = pv.NextChar();
        if( c==0 )
            break;
        if( c=='\f' ) {
            // int p = pv.start;
            // qlog("param %s ==== %c %c\n",pfunc?pfunc->getValue():"NULL", pv.CharAt(p+1), pv.CharAt(p+2));
            if( (pv.start+1)<pv.wep ) {
                // cur = pval ? pval:  (flag&FPARAM_PARAM) ? pfunc->addParam() : pfunc;
                // cur = (flag&FPARAM_PARAM) ? cur: cur->addParam();
                // LPCC var = pfunc->getVarName();
                cur = pfunc->addParam();
                if( pval==NULL ) {
                    if( pfunc->xftype==FTYPE_CONTROL ) { // || pfunc->xftype==FTYPE_SET
                        pval = cur;
                    } else {
                        pval = pfunc;
                    }
                }
            }
            pv.start++;
            continue;
        }
        else if( c=='.' ) {
            pv.start++;
            c = pv.NextChar();
            if( c=='\b') {
                pv.start++;
                c = pv.NextChar();
                XFunc* prev = cur;
                if( cur && cur->xfflag==0x2 && isFuncType(cur) ) {
                    // qDebug("xxxxxxxxxxxxxx\n");
                } else {
                    cur = pval;
                }
                if( cur==NULL )
                    cur = pfunc;

                sp = pv.start + 1;
                ep = findParamPos(pv);
                pv.start = ep;
                if( c=='U' ) {
                    c = pv.NextChar();
                    /*
                    if( cur->xftype==FTYPE_FUNC && cur->xfflag&FFLAG_PARAM  ) {
                        XFunc* pfc=cur->parent();
                        // if( pfc && (pfc->xfflag&FFLAG_PARAM)==0 && pfc->xftype<FTYPE_DEFINE )
                        if( pfc && pfc->xdotPos==0 && (pfc->xfflag&FFLAG_PARAM)==0 && pfc->xftype<FTYPE_DEFINE )
                            cur = cur->parent();
                    }
                    */
                    func = cur->addFunc();
                    func->xftype = FTYPE_FUNC;
                    //(na) 20150714

                    if( prev && cur->xparams.size() ) {
                        int row = cur->xparams.size() - 1;
                        U16 ftype = cur->xftype;
                        if( ftype>FTYPE_FUNCSTART && ftype<FTYPE_FUNCEND ) {
                            cur->xflag = row;
                        } else {
                            prev->xflag = row;
                        }
                        func->xflag = row;
                    }
                    func->setValuePos(sp, ep);
                    if( c=='\f' ) {
                        int ret=makeFuncCase(pv, FPARAM_FUNC|FPARAM_PARAM, ppfunc, func, pval);
                        if( ret==0 )
                            return 0;
                        if( ret==2 ) {
                            XFunc* pfc=pfunc->parent();
                            if( pfc && (pfc->xftype>FTYPE_FUNCSTART && pfc->xftype<FTYPE_FUNCEND) ) {
                                pfunc = pfunc->parent();
                            }
                        }
                    } else if( c=='\b' && pv.CharAt(ep+1)=='/' ) { // flag&FPARAM_FUNC &&
                        pv.start+=2;
                        c = pv.NextChar();
                        if( c=='\f' ) {
                            c = pv.incr().ch();
                            U16 ftype = cur->xftype;
                            if( c=='\b' ) {
                                if( ftype==FTYPE_ARRAY || ftype==FTYPE_THIS ) {
                                    cur = cur->parent()->addParam();
                                } else {
                                    cur = cur->addParam();
                                }
                                continue;
                            }
                            else
                                break;
                        } if( c=='\b') {
                            // ----------------------------------------------------------------------------------
                            while( c=='\b' ) {
                                if( pv.GetChar(1)=='/' )
                                    pv.start+=2;
                                else
                                    break;
                                c = pv.NextChar();
                            }
                        }
                        if( flag&FPARAM_FUNCSUB ) break;
                    }
                    // if( pv.NextChar()=='\b' && pv.CharAt(pv.start+1)=='/' ) pv.start+=2;
                } else if( c=='V') {
                    func = cur->addFunc();
                    func->xftype = FTYPE_VAR;
                    func->setValuePos(sp, ep++);
                    pv.start = ep;
                } else if( c=='a') {
                    func = cur->addFunc();
                    func->xftype = FTYPE_ARRAY;
                    func->setValuePos(sp, ep++);
                    pv.start = ep;
                }
            }
            continue;
        } else if( c=='\b' ) {
            if( cur==NULL )
                cur = pfunc;
            pv.start++;
            c = pv.NextChar();
            if( c==0 || c=='/' )
                break;
            sp = pv.start + 1;
            ep = findParamPos(pv);

            // func = (flag&FPARAM_LIST) ? getListData(lval, cuid) : getParam(cuid);
            func = cur;
            if( func==NULL ) {
                err(pv.start, "make func error");
                return 0;
            }
            cf = flag;

            if( flag&FPARAM_GROUP || func->xfflag&FFLAG_USE_GROUP ) {
                func = func->addParam();
                cf|= FPARAM_PARAM;
            }
            /*
            if( not ) {
                func->xfflag|= FFLAG_NOT;
                not = false;
            }
            */

            if( ppfunc == NULL)
                ppfunc = &func;

            switch( c ) {
            /*
            case '(':  {
                if( pv.MatchWordPos("\b(","\b)") ) {
                    ep = pv.start;
                    func->xftype = FTYPE_USEGROUP;
                    func->xfflag |= FFLAG_USE_GROUP;
                    func->setValuePos(pv.prev, pv.prev);
                    func = func->addParam();
                    func->xftype = FTYPE_GROUP;
                    func->setValuePos(pv.prev, pv.prev);
                    XParseVar var(pv.GetVar(), pv.prev-1, pv.cur);
                    makeFuncCase(var, FPARAM_GROUP|FPARAM_PARAM, ppfunc, func );
                } else {
                    return 0;
                }
            } break;
            */
            case 'V': {
                func->xftype = FTYPE_VAR;
                func->setValuePos(sp, ep++);
            } break;
            case '0':
            case 'n': {
                func->xftype = FTYPE_NUMBER;
                func->setValuePos(sp, ep++);
            } break;
            case 'D': {					// define
                func->xftype = FTYPE_DEFINE;
                func->setValuePos(sp, ep++);
            } break;
            case 'P': {
                func->xftype = FTYPE_STATIC;
                func->setValuePos(sp, ep++);
            } break;
            case '=': {
                func->xftype = FTYPE_STR;
                func->setValuePos(sp, ep++);
            } break;
            case '#': {
                func->xftype = FTYPE_TEXTVAR;
                func->setValuePos(sp, ep++);
            } break;
            case 'T': {					// TEXT
                func->xftype = FTYPE_TEXT;
                func->setValuePos(sp, ep++ );
            } break;
            case 't': {					// TEXT
                func->xftype = FTYPE_TEXTDATA;
                func->setValuePos(sp, ep++ );
            } break;
            case 'L': {					// null
                func->xftype = FTYPE_NULL;
                func->setValuePos(sp, ep++ );
            } break;
            case 'a': {					// array
                func->xftype = FTYPE_ARRAY;
                func->setValuePos(sp, ep++ );
                pv.start = ep;
                c = pv.NextChar();
                if( c=='.' ) {
                    if(!makeFuncCase(pv, FPARAM_FUNC|FPARAM_PARAM, ppfunc, func) ) return 0;
                    ep = pv.start;
                }

            } break;
            case 'l': {					// list
                flag = FPARAM_LIST|FPARAM_PARAM;
                if( cf&FPARAM_LIST ) flag|= FPARAM_PLIST;
                func->xftype = FTYPE_LIST;
                pv.start = ep;
                if( !makeFuncCase(pv, flag, ppfunc, func ) ) {
                    err(sp, "make func error");
                    return 0;
                }
                ep = pv.start;
            } break;
            case 'h': {					// list
                func->xftype = FTYPE_HASH;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
            case 'e': {					// expr
                func->xftype = FTYPE_EXPR;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
            case 'b': {					// callback - func
                func->xftype = FTYPE_CALLBACK;
                func->xfkind = 0;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
            case 'E': {                 //
                func->xftype = FTYPE_CONST;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
            case '@': {
                if( func->xftype ) {
                    func->xfflag |= FFLAG_USE_GROUP;
                    func = func->addParam();
                }
                // int len = ep - sp;
                func->xftype = FTYPE_OPER;
                func->setValuePos(sp, ep++ );
                /*
                LPCC src = get(sp);
                if( isEquals(src,"?",len) ) {
                    XFunc* prev = func->prevParam();
                    if( prev ) prev->xfflag |= FFLAG_CASE;
                }
                else if( isEquals(src,":",len) ) {
                    XFunc* prev = func->prevParam();
                    if( prev ) prev->xfflag |= FFLAG_CASETRUE;
                }
                else if( isEquals(src,"!",len) ) {
                    not = true;
                }
                else if ( isEquals(src,"=",len) ) {
                    func->parent()->xfflag |= FFLAG_SET;
                }
                */
            } break;
            case 'p': {					// local
                func->xftype = FTYPE_FUNC_RESV;
                func->setValuePos(sp, ep );
                pv.start = ep;
                c = pv.NextChar();
                if( c=='\f' ) {
                    if(!makeFuncCase(pv, FPARAM_FUNC | FPARAM_PARAM | FPARAM_FUNCSUB, ppfunc, func) ) return 0;
                    ep = pv.start;
                }
            } break;
            case '&': {
                func->xftype = FTYPE_SINGLEVAR;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
#ifdef CHECHFUNC_USE
            case 'z': {
                func->xftype = FTYPE_VARCHECK;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
            case 'Z': {
                func->xftype = FTYPE_CATCH;
                pv.start = ep;
                func->setValuePos(sp, ep++ );
            } break;
#endif
            case 'i':
            case 'U':
            {
                flag|= FPARAM_FUNC;
                func->xftype =
                    (c=='U') ? FTYPE_FUNC :
                    (c=='i') ? FTYPE_FUNCCHECK:  FTYPE_FUNC;
                    // (c=='M') ? FTYPE_FUNCCLASS :(c=='L') ? FTYPE_FUNCLIB :
                func->setValuePos(sp, ep );
                pv.start = ep;
                c = pv.NextChar();
                if( c=='\f' ) {
                    if(!makeFuncCase(pv, FPARAM_FUNC | FPARAM_PARAM, ppfunc, func) ) return 0;
                    ep = pv.start;
                }
            } break;
            case 'C': {
                func->xftype = FTYPE_CONTROL;
                func->setValuePos(sp, ep );
                pv.start = ep;
                c = pv.NextChar();
                if( c=='\f' ) {
                    if(!makeFuncCase(pv, FPARAM_FUNC|FPARAM_PARAM, ppfunc, func) ) return 0;
                    ep = pv.start;
                }
            } break;
            default:
                ep = pv.start+1; break;
            }
            if( ep<pv.start || ep>=pv.wep ) break;  // return err("PARSE DATA ");
            pv.start = ep;
            c = pv.NextChar();
            if( c=='\b' && pv.CharAt(ep+1)=='/' ) { // flag&FPARAM_FUNC &&
                char prevChar = pv.CharAt(ep-1);
                pv.start+=2;
                c = pv.NextChar();
                if( c=='\f') {
                    if( prevChar=='\b' ) {
                        if( func->xfflag&FFLAG_PARAM ) {
                            return 2;
                        }
                        break;
                    }
                }
                while( c=='\b' ) {
                    if( pv.GetChar(1)=='/' ) {
                        pv.start+=2;
                    } else {
                        func->xfflag|=FFLAG_USE_SUB;
                        break;
                    }
                    c = pv.NextChar();

                    // draw.drawText( rc.pos(rcText.rt()), "($cnt)", 'center');
                    if( c=='\f' || c=='.' ) {
                        return func->xfflag&FFLAG_PARAM ? 1: 0;
                    }
                }
                // if( flag&FPARAM_FUNCSUB ) break; //(na)
            }
            else if( flag&FPARAM_LIST ) {
                if( c!='\f') {
                    if( c=='\b')
                        pv.start++;
                    break;
                }
            }
        } else if( c=='V') {
            break;
        } else {
            // LPCC txt = pv.get(pv.start);
            // qDebug()<<"PARSE DATA error"<<KR(txt);
            err( pv.start, "PARSE DATA error");
            return 0;
        }
    }
    return 1;
}

bool XFuncSrc::setLine(XParseVar& pv, XFunc** ppfunc, XFunc* pfunc, U32 flag ) {
    char c = pv.NextChar();
    pv.start++;
    XFunc* func = pfunc->addFunc();
    *ppfunc = func;
#ifdef DEBUG_USE
    if( pv.CharAt(pv.start)=='x' ) {
        (*ppfunc)->xfflag|= FFLAG_PSET;
        return true;
    }
    if( c=='~' ) {
        func->xfflag|=FFLAG_DEBUG;
        c = pv.NextChar();
        if( c=='(' ) {
            if( pv.MatchWordPos("(",")",FIND_SKIP) ) {
                func->setValuePos(pv.prev,pv.cur);
            }
        }
        pv.start++;
    }
#endif
    int ep = findParamPos(pv);
    if( ep==-1)
        return err( pv.start, "set line error" );
    switch(c ) {
        case '$': func->xftype = FTYPE_SET; break;
        case '+': func->xftype = FTYPE_PLUS; break;
        case '-': func->xftype = FTYPE_MINUS; break;
        case '*': func->xftype = FTYPE_MULTI; break;
        case '/': func->xftype = FTYPE_DIV; break;
        case '%': func->xftype = FTYPE_MOD; break;
        case '|':
            func->xftype = 106; break;
        case '&': func->xftype = 107; break;
        case '^': func->xftype = 107; break;
        case 'A': func->xftype = FTYPE_SETARRAY; break;
        case '#': func->xftype = FTYPE_TEXTVAR; break;
        case 'D': func->xftype = FTYPE_DEFINE; break;
        case 'P': func->xftype = FTYPE_RESERVE; break;
        case 'n': func->xftype = FTYPE_NUMBER; break;
        case '\b': {
            char ch = pv.CharAt(pv.start);
            switch(ch) {
            case 'a': func->xftype = FTYPE_ARRAY; break;
            case 'w': func->xftype = FTYPE_CASEVAR; break;
            case 'W': func->xftype = FTYPE_CASE; break;
            case 'R': func->xftype = FTYPE_RETURN; break;
            case 'c': func->xftype = FTYPE_CONTINUE; break;
            case 'B': func->xftype = FTYPE_BREAK; break;
            case 'L': func->xftype = FTYPE_NULL; break;
            case 'V': {
                func->xftype = FTYPE_SINGLEVAR;
                func->setValuePos(pv.start+1, ep++ );
                pv.start = ep;
                return makeFuncCase(pv, flag, ppfunc, func->addParam() );
            } break;
            case 'b': {
                int sp=pv.start+1;
                func->xftype = FTYPE_CALLBACK;
                func->xfkind = 0;
                func->setValuePos(sp, ep++ );
                pv.start = ep;
                return true;
            } break;
            case 'T': func->xftype = FTYPE_TEXT; break;
            case 't': func->xftype = FTYPE_TEXTDATA; break;
            case 'U': func->xftype = FTYPE_FUNC; break;
            case 'C': func->xftype = FTYPE_CONTROL; break;
            case 'i': func->xftype = FTYPE_FUNCCHECK; break;
            case 'M': func->xftype = FTYPE_FUNCCLASS; break;
            // case 'I': func->xftype = FTYPE_FUNCLOCAL; break;
            // case 'L': func->xftype = FTYPE_FUNCLIB; break;
            case 'p':
                func->xftype = FTYPE_FUNC_RESV; break;
            default: break;
            }
            pv.start++;
            if( func->xftype>FTYPE_FUNCSTART &&
                func->xftype<FTYPE_FUNCEND )
                flag |= FPARAM_FUNC;
        } break;
        default: {
            qDebug("not match func ===================== %c\n",c);
        }	break;
    }
    func->setValuePos(pv.start, ep );
    if( func->xftype==FTYPE_CASE || func->xftype==FTYPE_CASEVAR ) {

        // xfkind 1:  2:, 5: array, 6: define, 7: func,  9:default
        if( func->xftype==FTYPE_CASEVAR ) {
            func->xftype = FTYPE_CASE;
            func->xfkind = 2;
        } else {
            char* ctype = (char*)func->getValue();
            if( ctype==NULL ) {
                func->xfkind = 7;
            } else if( ctype[0]=='[' && slen(ctype)>2 ) {
                func->xfkind = 5;
            } else if( isUpperCase(ctype[0]) ) {
                int p = IndexOf(ctype,'.');
                if( p>0 ) {
                    ctype[p] = 0;
                    func->xfkind = 6;
                    func->xfuncRef = getHashFlagVal(ctype,ctype+p+1);
                } else {
                    func->xfkind = 2;
                }
            } else if( ccmp(ctype,"default") ) {
                func->xfkind = 9;
            } else if( isnum(ctype) ) {
                func->xfkind = 1;
                func->xfuncRef = atoi(ctype);
            } else
                func->xfkind = 2;
        }
    } else if( func->xftype==FTYPE_CONTROL ) {
        if( func->xfkind==0 )
            func->xfkind = getControlFuncKind(func);
    }
    pv.start = ep;
    c = pv.NextChar();
    if( c=='\b' ) {
        pv.start++;
        if( flag&FPARAM_FUNC && pv.CharAt(ep+1)=='/' )
            pv.start++;
        c = pv.NextChar();
    }
    return (c=='\f' || c=='.') ? makeFuncCase(pv, flag, ppfunc, func) : true;
}



bool XFuncSrc::makeFunc() {
    XParseVar pv(this);
    if( xfunc==NULL )
        xfunc = gfuncs.getFunc(this);
    return makeFunc(pv, xfunc, 0);
}

bool XFuncSrc::makeFunc(XParseVar& pv, XFunc* pfunc, U32 flag) {
    char c;
    bool isctrl = false;
    bool iscase = false;
    int sp;
    XFunc* rfunc = NULL;
    XFunc* func = NULL;
    if( pfunc==NULL ) {
        if( xfunc==NULL ) {
            xfunc = gfuncs.getFunc(this);
        }
        pfunc=xfunc;
    }
    while( pv.valid() ) { // pv.start<pv.wep ) {
        c = pv.NextChar();
        sp = pv.start;
        if( c=='\b') {
            c = pv.CharAt(sp+1);
            if( c=='<' ) {
                if( flag&FPARAM_CASE ) {
                    c = pv.CharAt(sp+2);
                    if( c=='\b' ) {
                        c = pv.CharAt(sp+3);
                        if( (c=='W'||c=='w') && (pfunc->xftype!=FTYPE_CONTROL && pfunc->xfkind!=4) )
                            return true;
                    }
                    // pv.start = sp;
                }
                if( pv.MatchWordPos("\b<","\b>") ) {
                    if( iscase ) {
                        XParseVar cur(pv.GetVar(), sp, pv.wep);
                        if( func && !makeFunc(cur, func, FPARAM_CASE) )
                            return false;
                        iscase = false;
                        pv.start = cur.start;
                    } else {
                        XParseVar cur(pv.GetVar(), pv.prev, pv.cur);
                        if( isctrl ) {
                            isctrl = false;
                            if( func && !setLine(cur, &rfunc, func, flag) )
                                return false;
                            /*
                            if( rfunc->xfflag& FFLAG_PSET ) {
                                rfunc = pfunc;
                                continue;
                            }
                            */
                        } else {
                            if( !setLine(cur, &rfunc, pfunc, flag) )
                                return false;
                        }
                        if( pv.start>=pv.wep )
                            break;

                        if( rfunc )
                            func = rfunc;

                        if( func->xftype == FTYPE_CONTROL ) {
                            isctrl = true;
                        } else if( func->xftype == FTYPE_CASE) {
                            iscase = true;
                        } else if( func->xftype == FTYPE_BREAK) {
                            iscase = false;
                            // if( flag&FPARAM_CASE )return true;
                        }
                    }
                } else {
                    LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                    return err(pv.start, gBuf.fmt("%s function parse line error",xx) );
                }
            } else if( c=='[' ) {
                if( pv.MatchWordPos("\b[","\b]") && rfunc ) {
                    XFunc* func = pfunc->addFunc();
                    func->setValuePos(pv.prev, pv.cur );
                    func->xfkind = 11;
                    rfunc->xfkind = 11;
                    func->xftype = FTYPE_ARRAY;
                } else {
                    LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                    return err(pv.start, gBuf.fmt("%s inline command match error", xx) );
                }
            } else if( c=='{' ) {
                if( pv.MatchWordPos("\b{","\b}") ) {
                    isctrl = false;
                    XParseVar sub(pv);
                    if( !makeFunc(sub, rfunc, flag) )
                        return false;
                } else {
                    LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                    return err(pv.start, gBuf.fmt("%s sub function match error",xx) );
                }
            } else {
                LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
                return err(pv.start, gBuf.fmt("%s unknown parse command",xx) );
            }
        } else {
            LPCC xx=gBuf.add(pv.GetValue(pv.start), min(32,(pv.wep-pv.start)) );
            return err(sp, gBuf.fmt("%s fuction start string error",xx) );
        }
    }
    return true;
}


/************************************************************************/
/* XFunc                                                                */
/************************************************************************/

XFunc::XFunc() :
    XType(), xftype(0), xfflag(0), xdotPos(0), xfkind(0), xfuncRef(0),
    xparent(NULL), xpv((StrVar*)this) {
}


XFuncSrc* XFunc::getFuncSrc() {
    return (XFuncSrc*)xpv.GetVar();
}

XFunc* XFunc::addFunc() {
    XFunc* fn = gfuncs.getFunc((XFuncSrc*)xpv.GetVar());
    if( fn==NULL ) return &gfc;
    fn->xparent = this;
    xsubs.add(fn);
    return fn;
}
XFunc* XFunc::addParam() {
    XFunc* fn = gfuncs.getFunc((XFuncSrc*)xpv.GetVar());
    if( fn==NULL ) return &gfc;
    fn->xparent = this;
    fn->xfflag = FFLAG_PARAM;
    xparams.add(fn);
    return fn;
}
XFunc* XFunc::getFunc(int n) {
    return n<xsubs.size() ? xsubs.getvalue(n): NULL;
}
XFunc* XFunc::getParam(int n) {
    return n<xparams.size() ? xparams.getvalue(n): NULL;
}
XFunc* XFunc::prevParam() {
    XFunc* p = parent();
    if( p==NULL )
        return this;
    XFunc* prev = NULL;
    for(int n=0,size=p->xparams.size();n<size;n++) {
        if( p->xparams.getvalue(n)==this )
            return prev;
        prev = p->xparams.getvalue(n);
    }
    return prev;
}

XFunc* XFunc::prevFunc() {
    XFunc* p = parent();
    if( p==NULL )
        return this;
    XFunc* prev = NULL;
    for(int n=0,size=p->xsubs.size();n<size;n++) {
        if( p->xsubs.getvalue(n)==this )
            return prev;
        prev = p->xsubs.getvalue(n);
    }
    return prev;
}

XFunc* XFunc::nextParam() {
    XFunc* p = parent();
    if( p==NULL )
        return NULL;
    for(int n=0,size=p->xparams.size();n<size-1;n++) {
        if( p->xparams.getvalue(n)==this )
            return p->xparams.getvalue(n+1);
    }
    return NULL;
}

XFunc* XFunc::nextFunc() {
    XFunc* p = parent();
    if( p==NULL )
        return NULL;
    for(int n=0,size=p->xsubs.size();n<size-1;n++) {
        if( p->xsubs.getvalue(n)==this )
            return p->xsubs.getvalue(n+1);
    }
    return NULL;
}

int XFunc::getFuncPos() {
    XFunc* p = parent();
    if( p==NULL )
        return 0;
    for(int n=0,size=p->xsubs.size();n<size;n++) {
        if( p->xsubs.getvalue(n)==this ) return n;
    }
    return -1;
}
int XFunc::getParamPos() {
    XFunc* p = parent();
    if( p==NULL )
        return 0;
    for(int n=0,size=p->xparams.size();n<size;n++) {
        if( p->xparams.getvalue(n)==this ) return n;
    }
    return -1;
}


void XFunc::setSrc(XFuncSrc* src) {
    xftype = xfflag = 0;
    xfuncRef = 0;
    xdotPos = xfkind = 0;
    xparams.reuse();
    xsubs.reuse();
    xpv.SetVar((StrVar*)src);
}


void XFunc::deleteFuncSrc() {
    if( xpv.pvar ) {
        gfsrcs.deleteFuncSrc((XFuncSrc*)xpv.pvar);
    }
}

void XFunc::setValuePos( int sp, int ep) {
    int size = ep-sp;
    if( size<0 ) return;
    xpv.SetPosition(sp,ep);
    if( size==0 ) return;
    LPCC p = xpv.GetBuffer();
    if( p==NULL ) return;
    switch(xftype ) {
    case FTYPE_TEXT : {
        if( IndexOf(p,'\\',size)!=-1) {
            int n=0, x=0;
            char* b = (char*)p;
            for( ;n<size;n++,x++ ) {
                if(b[n]=='\\') {
                    n++;
                    switch(b[n]) {
                    case '\\':	b[x] = '\\'; break;
                    case 'r':	b[x] = '\r'; break;
                    case 'n':	b[x] = '\n'; break;
                    case 't':	b[x] = '\t'; break;
                    case 'b':	b[x] = '\b'; break;
                    case 'f':	b[x] = '\f'; break;
                    case '\"':	b[x] = '\"'; break;
                    case '\'':	b[x] = '\''; break;
                    default:	n--; break;
                    }
                }
                else if( x<n ) b[x] = b[n];
            }
            b[x] = 0;
            if( n!=x && x>0 ) {
                xpv.SetPosition(sp, sp+x);
            }
        }
    } break;
    case FTYPE_RESERVE:
    case FTYPE_SET:
    case FTYPE_VAR:
    case FTYPE_ARRAY:
    case FTYPE_DEFINE:
    case FTYPE_FUNC:
    case FTYPE_FUNC_RESV:
    case FTYPE_FUNCCHECK:
    case FTYPE_FUNCEND:
    case FTYPE_STATIC: {
        bool bupper = true;
        for( int n=0;n<size;n++ ) {
            U8 ch=(U8)p[n];
            if( ch=='.' ) {
                xdotPos = (U8)n;
                xfflag|= FFLAG_DOTPOS;
                break;
            }
            if( bupper && (ch>90 || ch<65) )
                bupper = false;
        }
        if( StartWith(p,"this",4,size) && (p[4]=='.' || p[4]=='\b') ) {
            // version 1.0.4
            if( xftype==FTYPE_FUNC  ) {
                xftype = FTYPE_FUNC_THIS;
            } else {
                if( xftype==FTYPE_ARRAY )
                    xfkind = FTYPE_ARRAY;
                else if( xftype==FTYPE_SET )
                    xfkind = FTYPE_SET;
                xftype = FTYPE_THIS;
            }
        } else if( xdotPos>0 ) {
            if( ccmp(p,"Cf.") || ccmp(p,"CF.") ) { // StartWith(p,"Cf.",3,size)
                if( xftype==FTYPE_FUNC_RESV ) {
                    int pos = LastIndexOf(p,'.',size);
                    if( pos>0 && pos!=xdotPos ) {
                        pos++;
                        xflag|= FTYPE_FUNC_RESV<<8;
                        xflag|= pos<<16;
                    }
                } else if( xftype==FTYPE_VAR ) {
                    xftype = FTYPE_STATIC;
                }
            } else if( xftype==FTYPE_VAR ) {
                if( bupper ) {
                    xftype = FTYPE_DEFINE;
                } else if( IS_DIGIT(p[0]) || p[0]=='-' || p[0]=='+' ) {
                    xftype = FTYPE_NUMBER;
                }
            }
        } else {
            if( cncmp(p,size,"null",4) ) {
                xftype = FTYPE_NULL;
            } else if( cncmp(p,size,"true",4) || cncmp(p,size,"false",5)  ) {
                xftype = FTYPE_BOOL;
            } else if( IS_DIGIT(p[0]) || p[0]=='-' || p[0]=='+' ) {
                if( size>1 && (p[1]=='x'|| p[1]=='X') ) {
                    LPCC hex = gBuf.add(p+2,size-2);
                    xfuncRef = (U32)strtol(hex, NULL, 16);
                    // xftype = FTYPE_DEFINE;
                }
                xftype = FTYPE_NUMBER;
            }
        }
    } break;
    default: break;
    }
}

LPCC XFunc::getValue() {
    return xpv.value();
}

LPCC XFunc::getVarName() {
    return xdotPos>0 ? xpv.GetChars(xpv.wsp,xpv.wsp+xdotPos): getValue();
}
LPCC XFunc::getFuncName() {
    if( xftype==FTYPE_FUNC_RESV ) {
        int pos = xflag>>16;
        if( pos<xpv.wep && pos>xdotPos ) {
            U16 chk = xflag&0xFF00;
            U16 kind = chk>>8;
            if( kind==FTYPE_FUNC_RESV )
                return xpv.GetChars(xpv.wsp+pos,xpv.wep);
        }
    }
    return xpv.GetChars(xdotPos>0? xpv.wsp + xdotPos +1 : xpv.wsp, xpv.wep);
}
LPCC XFunc::getValueBuf(char* buf, int buflen) {
    return xpv.getBuffer(buf, buflen, xpv.wsp,xpv.wep);
}
LPCC XFunc::getVarBuf(char* buf, int buflen) {
    return xdotPos>0 ? xpv.getBuffer(buf, buflen, xpv.wsp,xpv.wsp+xdotPos):
                       xpv.getBuffer(buf, buflen, xpv.wsp,xpv.wep);
}

LPCC XFunc::getFuncBuf(char* buf, int buflen) {
    return xdotPos>0 ? xpv.getBuffer(buf, buflen, xpv.wsp + xdotPos +1, xpv.wep):
                       xpv.getBuffer(buf, buflen, xpv.wsp,xpv.wep);
}

inline LPCC getFuncTypeNm(U16 type) {
    return	(type==FTYPE_SET) ? "SET"			: (type==FTYPE_FUNC) ? "FUNC" :
            (type==FTYPE_TEXT) ? "TEXT"			: (type==FTYPE_TEXTVAR) ? "TEXTVAR"	:
            (type==FTYPE_CONTROL) ? "CONTROL" :
            (type==FTYPE_VAR) ? "VAR"			: (type==FTYPE_STR) ? "STR" :
            // (type==FTYPE_GROUP) ? "GROUP"		: (type==FTYPE_FUNCGROUP) ? "FUNCGROUP" :
            (type==FTYPE_USEGROUP) ? "USERGROUP"	: (type==FTYPE_CASE) ? "CASE"	:
            (type==FTYPE_OPER) ? "OPER"			: (type==FTYPE_RESERVE) ? "RESERVE" :
            (type==FTYPE_BREAK) ? "BREAK"		: (type==FTYPE_RETURN) ? "RETURN" :
            (type==FTYPE_CONTROL) ? "CONTORL"	: gBuf.fmt("FTYPE=%d",(int)type);
}

bool XFunc::printFunc(StrVar& var, int depth) {
    if( depth==0 )
        var.reuse();
    printParam(var, depth, 1);
    for( int n=0, size=xsubs.size(); n<size; n++ )
        xsubs.getvalue(n)->printFunc(var, depth+1);
    return false;
}

bool XFunc::printParam(StrVar& var, int depth, U32 flag) {
    if( flag )
        if( xftype==0 )
            var.format(512,"\n%s%s(OX%X,%d)", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef  );
        else {
            if( xpv.size()<256 )
                var.format(512,"\n%s%s(OX%X,%d) : %s", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef, xpv.GetValue() );
            else
                var.format(512,"\n%s%s(OX%X,%d) : %s...", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef, xpv.GetChars(xpv.wsp,xpv.wsp+64) );
        }
    else {
        if( xftype==0 )
            var.format(512,"\n%s%s(OX%X,%d)", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef  );
        else {
            if( xpv.size()<256 )
                var.format(512,"\n%s[%s(OX%X,%d) : %s] ", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef, xpv.GetValue() );
            else
                var.format(512,"\n%s%s(OX%X,%d) : %s...]", addSpace(depth), getFuncTypeNm(xftype), xfflag, xfuncRef, xpv.GetChars(xpv.wsp,xpv.wsp+64)  );
        }
        for( int n=0, size=xsubs.size(); n<size; n++ )
            xsubs.getvalue(n)->printFunc(var, depth+1);
    }
    int size = xparams.size();
    if( size>0 )
        var.add("(");
    for( int n=0 ; n<size; n++ )
        xparams.getvalue(n)->printParam(var, depth+1, 0);
    if( size>0 )
        var.add(")");
    return true;
}


/************************************************************************/
/* XFuncSrcBuffer                                                       */
/************************************************************************/

XFuncSrc* XFuncSrcBuffer::getFuncSrc() {
    // gMutexFuncSrc.EnterMutex();
    XFuncSrc* fs = xnodes.getNode();
    if( fs->xfunc ) {
        gfuncs.deleteFunc(fs->xfunc);
        fs->xfunc = NULL;
    }
    fs->reuse();
    fs->xparam.reuse();
    // gMutexFuncSrc.LeaveMutex();
    return fs;
}

void XFuncSrcBuffer::deleteFuncSrc(XFuncSrc* fs){
    if( fs ) {
        // gMutexFuncSrc.EnterMutex();
        if( fs->xfunc ) {
            deleteAllFunc(fs->xfunc);
            fs->xfunc=NULL;
        }
        xnodes.deleteNode(fs);
        // gMutexFuncSrc.LeaveMutex();
    }
}
/************************************************************************/
/* XFuncBuffer                                                       */
/************************************************************************/

XFunc* XFuncBuffer::getFunc(XFuncSrc* src)  {
    XFunc* func = xnodes.getNode();
    if( func ) {
        // gMutexFunc.EnterMutex();
        func->setSrc(src);
        func->xparams.reuse();
        func->xsubs.reuse();
        func->xftype = 0;
        func->xfflag = 0;
        func->xdotPos = 0;
        func->xfkind = 0;
        func->xfuncRef = 0;
        // gMutexFunc.LeaveMutex();
    }
    return func;
}
void XFuncBuffer::deleteFunc(XFunc* func)  {
    if( func ) {
        // gMutexFunc.EnterMutex();
        xnodes.deleteNode(func);
        // gMutexFunc.LeaveMutex();
    }
}

/************************************************************************/
/* XFuncNodeBuffer                                                       */
/************************************************************************/

XFuncNode* XFuncNodeBuffer::getFuncNode(XFunc* fc, XFuncNode* pfn )  {
    XFuncNode* fn = NULL;
    // gMutexFuncNode.EnterMutex();
    if( pfn ) {
        fn = pfn->addfuncNode(fc);		
	}
    if( fn==NULL ) {
        fn = xnodes.getNode();
    }
    /*
	if( pfn && fn ) {
		StrVar* sv=pfn->gv("@echoRef");
		if(SVCHK('f',0)) {
			fn->GetVar("@echoRef")->add(sv);
		}
	}
    */
    fn->xtype=pfn? pfn->xtype: 0;
    fn->xfunc=fc;
    fn->xflag=0;
    fn->xparent=pfn;
    fn->xchilds.reuse();
    // gMutexFuncNode.LeaveMutex();
    return fn;
}

void XFuncNodeBuffer::deleteFuncNode(XFuncNode* fn)  {
    if( fn && !fn->isNodeFlag(FLAG_PERSIST) ) {
        // gMutexFuncNode.EnterMutex();
        fn->reuseFuncNode();
        xnodes.deleteNode(fn);
        // gMutexFuncNode.LeaveMutex();
    }
}


/************************************************************************/
/* Func Object                                       */
/************************************************************************/

inline void SetBitMask(char* buf, int buflen, int sp, int ep) {
    int s = sp/8, e = ep/8;
    int sm = sp%8, em = ep%8;
    if( s==e) {
        for( int n=sm; n<em;n++) buf[s] |= 1<<n;
    } else {
        for( int n=s; n<=e;n++) {
            if( n==s) {
                for( int m=sm; m<8; m++) buf[n] |= 1<<m;
            } else if( n==e) {
                for( int m=0; m<em; m++) buf[n] |= 1<<m;
            } else {
                buf[n] = 0xFF;
            }
        }
    }
}

XType::XType() : xpos(), xflag(0) {}
void XType::setNode(U16 r, U16 c) {
    xpos.set(r,c);
}
U16	XType::getXRow()	{ return xpos.xr; }
U16	XType::getXCol()	{ return xpos.xc; }

XStr::XStr(LPCC value) : StrVar(value), xpos() {}
void XStr::setNode(U16 r, U16 c) {
    xpos.set(r,c);
}
U16 XStr::getXRow()		{ return xpos.xr; }
U16 XStr::getXCol()		{ return xpos.xc; }

XNode::XNode(int size) : WBox(size), xpos(), xtick(0), xflag(0) {
}
void XNode::setNode(U16 r, U16 c) {
    xpos.set(r,c);
}
U16	XNode::getXRow()	{ return xpos.xr; }
U16	XNode::getXCol()    { return xpos.xc; }
void XNode::setTick()   { xtick = GetTickCount(); }
long XNode::getTick()   { return GetTickCount()-xtick; }

int ParseText::Find( LPCC find, int ep, U32 flag) {
    if( ep==-1)
        ep = wep;
    int pos = -1;
    if(pvar) {
        pos = pvar->findPos(find, start, ep, flag);
        if( pos!=-1 )
        {
            prev = start;
            cur = pos;
            start = pos + slen(find);
        }
    }
    return pos;
}
BOOL ParseText::FindCheck(int pos, int* endpos) {
    int size = poss.size();
    if( pos==-1 ) pos = start;
    for( int m=0;m<size; m++)
    {
        if( poss[m]->IsIn(pos) )
        {
            if( endpos ) *endpos = poss[m]->end;
            return TRUE;
        }
    }
    return FALSE;
}

int ParseText::AddComment( LPCC st, LPCC et, int sp, int ep ) {
    if( ep==-1)
        ep=wep;
    else
        wep = ep;
    start = sp;
    for( int n=0; start<wep && n<1024; n++ ) {
        if( Find(st)==-1 ) break;
        if( !FindCheck() ) {
            sp = cur;
            if( MatchWordPos(st, et, FIND_SKIP) ) {
                poss.add(TextPos(sp,start) );
            }
        } else {
            start++;
        }
    }
    return poss.size();
}

int ParseText::AddLineComment( LPCC st, int sp, int ep) {
    if( ep==-1)
        ep=wep;
    else
        wep = ep;
    start = sp;
    for( int n=0; start<wep && n<1024; n++ )
    {
        if( Find(st)==-1 ) break;
        if( !FindCheck() ) {
            sp = cur;
            if( Find("\n")==-1 )
                poss.add(TextPos(sp, wep) );
            else
                poss.add(TextPos(sp, cur) );
        }
        else
            start++;
    }
    return poss.size();
}


StrVar* ParseText::MakeText(StrVar* rtn, int first) {
    if( rtn==NULL ) return NULL;
    int len = size();
    int masklen = (len/8)+1;
    char* mask = gBuf.get(masklen);
    memset(mask, 0 , masklen);
    for( int k=first; k<poss.size(); k++) {
        SetBitMask(mask, masklen, poss[k]->start-wsp, poss[k]->end-wsp);
    }
    rtn->reuse();
    for( int n=wsp,x=0; n<wep; n++,x++) {
        if( mask[x/8] & (1<<(x%8)) )
            continue;
        rtn->add( CharAt(n) );
    }
    return rtn;
}

ZipVar::ZipVar(StrVar* var, int sp, int ep) : XParseVar(var, sp, ep) {
}

BOOL ZipVar::Encode()
{
    if( pvar==NULL ) return FALSE;
    unsigned char* zipbuf=NULL, *basebuf=NULL;
    int ziplen = 0, baselen=0;
    if( xzip.Compress((unsigned char *)pvar->get(), pvar->length(), &zipbuf, &ziplen) &&
        CLZ77Lib::BASE64_Encode(zipbuf, ziplen, &basebuf, &baselen) )
    {
        pvar->set((LPCC)basebuf, baselen);
        return TRUE;
    }
    return FALSE;
}
BOOL ZipVar::Decode()
{
    if( pvar==NULL ) return FALSE;
    unsigned char* buf=NULL, *basebuf=NULL;
    int buflen = 0, baselen=0;
    if( CLZ77Lib::BASE64_Decode((unsigned char *)pvar->get(), pvar->length(), &basebuf, &baselen) &&
        xzip.Decompress(basebuf, baselen, &buf, &buflen) )
    {
        pvar->set((LPCC)buf, buflen);
        return TRUE;
    }
    return FALSE;
}

StrVar* ZipVar::JEncode(StrVar* var, int key )
{
    xrst.reuse();
    int srclen=var->length(), i=0;
    char ch=0, sum=0;
    for(; i < srclen; i++ ) {
        ch=var->charAt(i);
        sum+=ch;
        if( ((key%7) == (i%5)) || ((key%2) == (i%2)) ) {
            ch = ~ch;
        }
        xrst.add(ch);
    }
    int len=xrst.length();
    for( i = 0; i <len; i++ ) {
        if( key%srclen > i ) {
            ch = xrst.charAt(i) + sum*((i*i)%3);
        } else if( key%srclen == i ) {
            ch = sum;
        } else if( key%srclen < i ) {
            ch = xrst.charAt(i-1) + sum*((i*i)%7);
        }
        xrst.setAt(i, ch, len);
    }
    // xrst.setAt(i, '\0', len);
    return &xrst;
}

StrVar* ZipVar::JDecode(StrVar* var, int key ) {
    int srclen=var->length(), i=0, dlen=srclen-1, idx=abs(key%dlen);
    char ch=0, sum=var->charAt(idx);
    xrst.reuse();
    xrst.memalloc(srclen);
    for( i=0; i < srclen; i++ ) {
        if( idx > i ) {
            xrst.setAt(i, var->charAt(i) - sum*((i*i)%3), srclen);
        } else if( idx < i ) {
            xrst.setAt(i-1, var->charAt(i) - sum*((i*i)%7), srclen);
        }
    }
    for( i = 0; i<srclen; i++ ) {
        if( ((key%7) == (i%5)) || ((key%2) == (i%2)) )  {
            ch=xrst.charAt(i);
            xrst.setAt(i, ~ch, srclen);
        }
    }
    // xrst.setAt(i, '\0', srclen);
    return &xrst;
}
BOOL ZipVar::baseEncode(StrVar* var)
{
    /*
    if( pvar==NULL ) return FALSE;
    unsigned char* basebuf=NULL;
    int baselen=0;
    if(  CLZ77Lib::BASE64_Encode((unsigned char *)pvar->get(), pvar->length(), &basebuf, &baselen) )
    {
        var->set((LPCC)basebuf, baselen);
        return TRUE;
    }
    */
    return FALSE;
}

BOOL ZipVar::baseDecode(StrVar* var)
{
    /*
    if( pvar==NULL ) return FALSE;
    unsigned char* basebuf=NULL;
    int baselen=0;
    if( CLZ77Lib::BASE64_Decode((unsigned char *)pvar->get(), pvar->length(), &basebuf, &baselen) )
    {
        var->set((LPCC)basebuf, baselen);
        return TRUE;
    }
    */
    return FALSE;
}

