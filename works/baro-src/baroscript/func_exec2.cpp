#include "func_util.h"

DbUtil gMemDb;
StrVar gExprVar;

inline StrVar* getFuncNodeVar(XFuncNode* fn, LPCC code, U32 flag) {
    StrVar* sv=NULL;
    if( flag && fn ) fn = fn->xparent;
    while( fn ) {
        sv = fn->gv(code);
        if( sv ) break;
        fn= fn->xparent;
    }
    return sv;
}
//#> func param
void execParamFunc(XFunc* param, XFuncNode* fn, StrVar* var) {
    if( param==NULL || var==NULL ) return;

    int cnt=param->getParamSize();
    XFunc* fcOper=NULL;
    XFunc* fcSub=NULL;
    if( isFuncType(param) && cnt>0 ) {
        if( param->getParam(0)->xftype==FTYPE_OPER ) {
            fcOper=param->getParam(0);
        } else {
            fcSub=param->getParam(cnt-1);
            if( fcSub && fcSub->xfflag&0x10 ) {
                fcOper=fcSub->getParam(0);
            }
        }
    } else if( (param->xfflag&0x800)==0 ) {
        fcOper=param->getParam(0);
    }
    var->reuse();
    if( fcOper && fcOper->xftype==FTYPE_OPER ) {
        if( fcSub) {
            fcSub->xfflag|=0x800;
            execFunc(param,fn,var);
            fcSub->xfflag&=~0x800;
            getOperValue(fn, fcSub, fcSub->getParamSize(), var);
        } else {
            if(param->xftype!=FTYPE_SINGLEVAR) {
                execFunc(param,fn,var);
            }
            getOperValue(fn, param, param->getParamSize(), var);
        }
    } else {
        execFunc(param,fn,var);
    }
    /*
    if( param->getParamSize() && param->getParam(0)->xftype==FTYPE_OPER ) {

        if(param->xftype!=FTYPE_SINGLEVAR) {
            execFunc(param,fn,var);
        }
        getOperValue(fn, param, param->getParamSize(), var);
    } else {
        execFunc(param,fn,var);
    }
    */
}
XListArr* getParamArray(XFunc* func, XFuncNode* fn, StrVar* rst ) {
    int cnt = func->xparams.size();
    if( cnt==0 )
        return NULL;
    XListArr* list = NULL;
    if( cnt==1 )
        func->xfflag|=FLAG_CALL;
    for( int n=0;n<cnt;n++ ) {
        XFunc* param = func->xparams.getvalue(n);
        U16 ty = param->xftype;
        if( ty==FTYPE_OPER ) {
            break;
        }
        if( list==NULL ) {
            list=getLocalArray(true);
        }
        StrVar* var = list->add();
        execParamFunc(param, fn, var);
    }
    return list;
}
U32 makeLocalFunction( XFunc* func, bool bset, XFuncNode* fn, StrVar* rst, LPCC name ) {
    if( func==NULL || fn==NULL || rst==NULL ) return 0;
    XFunc* pfc=NULL;
    // LPCC fcode=NULL;
    StrVar* sv=NULL;
    XFuncSrc* fsrc=NULL;
    pfc=func->parent();

    if( name ) {
        sv = fn->gv(name);
        if( SVCHK('f',1) ) {
            fsrc = (XFuncSrc*)SVO;
            if( func->xfuncRef==0 ) {
                int idx=getCf()->incrNum("@cbidx");
                func->xfuncRef=idx;
            }
        }
    }
    if( func->xfuncRef==0 ) {
        if( fsrc ) {
            rst->setVar('f',1,(LPVOID)fsrc);
        } else {
            XParseVar* pv =&(func->xpv);
            pv->SetPosition();
            fsrc=gfsrcs.getFuncSrc();
            if( pv->ch()==';' ) {
                pv->incr();
                fsrc->xparam.reuse();
            } else {
                pv->findEnd(";");
                fsrc->xparam.set(pv->v());
            }
            fsrc->readBuffer(pv->GetVar(),pv->start,pv->wep );
            fsrc->makeFunc();
            if( fsrc->xfunc ) {
                int idx=getCf()->incrNum("@cbidx");
                if( idx==0 ) idx=getCf()->incrNum("@cbidx");
                func->xfuncRef=idx;
                LPCC code=gBuf.fmt("c%d", idx);
                if( bset ) {
                    XFuncNode* fnCur=setFuncNodeParent(fsrc, fn);
                    getStrVar("callback", code, 'f',0,(LPVOID)fnCur );
                    qDebug("callback set code: %s\n",code);
                    rst->setVar('f',0,(LPVOID)fnCur);
                } else {
                    getStrVar("func", code, 'f',1,(LPVOID)fsrc );
                    qDebug("func set code: %s\n",code);
                    rst->setVar('f',1,(LPVOID)fsrc);
                }
            } else {
                gfsrcs.deleteFuncSrc(fsrc);
                qDebug("#9##[%s] make func error (index:%d)\n", getBaseFuncName(fn), func->xfuncRef);
            }
        }
    } else {
        LPCC code=gBuf.fmt("c%d", func->xfuncRef);
        if( bset ) {
            sv=getStrVar("callback",code );
            if( SVCHK('f',0) ) {
                rst->reuse()->add(sv);
            } else {
                qDebug("local function callback not set !!! code: %s\n",code);
            }
        } else {
            if( fsrc ) {
                rst->setVar('f',1,(LPVOID)fsrc);
            } else {
                sv=getStrVar("func",code );
                if( SVCHK('f',1) ) {
                    rst->reuse()->add(sv);
                } else {
                    qDebug("local function func not set !!! code: %s\n",code);
                }
            }
        }
    }
    return func->xfuncRef;
}


int execParams(XFunc* func, XFuncNode* fn, StrVar* rst ) {
    XFunc* param=func->xparams.getvalue(0);
    execParamFunc(param, fn, rst);
    return 0;
    /*
    int rtn = 0;
    XFunc* param=NULL;
    for( int n=0,sz=func->xparams.size();n<sz;n++ ) {
        param=func->xparams.getvalue(n);
        if( param->getParamSize() && param->getParam(0)->xftype==FTYPE_OPER ) {
            if(param->xftype!=FTYPE_SINGLEVAR) {
                execFunc(param,fn,rst);
            }
            getOperValue(fn, param, param->getParamSize(), rst);
        } else {
            rtn = execFunc(param,fn,rst);
            if( rtn > 0 ) {
                n+=rtn;
            }
        }
    }
    return rtn;
    */
}
inline bool makeExprData(XParseVar& pv, XFuncNode* fn, StrVar* rst, XFunc* fc ) {
    char ch = 0;
    int sp=0,ep=0,end=0;
    StrVar* var = NULL;
    // bool fst = true;
    while( pv.valid() ) {
        ch = pv.ch();
        if( ch==0 )
            break;

        sp = pv.start;
        switch(ch) {
        case '(': {
            if( pv.MatchWordPos("(",")") ) {
                rst->add('(');
                XParseVar sub(pv.GetVar(),pv.prev, pv.cur);
                makeExprData(sub,fn,rst,fc);
                rst->add(')');
            } else return false;
        } break;
        case ',':
        case '+':
        case '-':
        case '=':
        case '!':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>': {
            rst->add(ch);
            pv.incr();
            // fst = false;
        } break;
        case '|':
        case '&': {
            if( ch=='&' && pv.CharAt(sp+1)=='&' ) {
                rst->add(" and ");
                pv.start+= 2;
            } else if( ch=='|' && pv.CharAt(sp+1)=='|' ) {
                rst->add(" or ");
                pv.start+= 2;
            } else {
                rst->add(ch);
                pv.incr();
            }
        } break;
        default: {
            if( ch=='#' ) {
               ch=pv.incr().ch();
               if( ch=='[' && pv.match("[","]", FIND_SKIP) ) {
                   callEvalFunc(pv.GetVar(),pv.prev,pv.cur,fn,gExprVar.reuse(),NULL,NULL);
                   if( gExprVar.length() ) {
                       rst->add(&gExprVar);
                   }
               } else {
                   return false;
               }
            } else if( ch=='0' && pv.CharAt(sp+1)=='x' ) {
                pv.start+= 2;
                rst->format(16,"%d",(int)strtol(pv.getNext(), NULL, 16));
                if( pv.ch()==0 ) {
                    break;
                }
            } else {
                pv.moveNext();
                end = ep = pv.start;
                ch = pv.CharAt(ep);
                bool fchk = false;
                if( ch=='(' || ch=='.' ) {
                    if( ch=='.' ) {
                        ch=pv.incr().ch();
                        if( ch=='(') {
                            fchk=true;
                        } else {
                            LPCC val=pv.Trim(sp, pv.start);
                            if( isnum(val) ) {
                                rst->add(val);
                                continue;
                            } else {
                                fchk=true;
                            }
                        }
                    }  else {
                        fchk=true;
                    }
                    if( fchk==false && pv.start!=end )
                        pv.start = end;
                }
                if( fchk ) {
                    if(ch=='(' ) {
                        if( !pv.match("(",")", FIND_SKIP) ) {
                            return false;
                        }
                    }
                    if( (ep-sp)==3 && ccmpl(pv.GetBuffer(sp),"not",3) ) {
                        LPCC val=pv.v();
                        var = getFuncVar(fn, val);
                        rst->add( isVarTrue(var) ? '1': '0');
                        //qDebug("Expr data call not function : %s : %s\n",val, toString(var));
                    } else  {
                        callEvalFunc(pv.GetVar(),sp, pv.start,fn, gExprVar.reuse(), NULL, NULL);
                        if( gExprVar.length() ) {
                            rst->add(&gExprVar);
                        }
                    }
                    continue;
                }

                char* val = Trim((char*)pv.GetChars(sp,end));
                if( slen(val)==0 )
                    break;
                pv.setPos(end);
                if( isnum(val) ) { // || ccmp(val,"AND") || ccmp(val,"OR")
                    // if( ccmp(val,"AND") || ccmp(val,"OR") ) fst = false;
                    rst->add(val);
                } else {
                    LPCC extr=NULL;
                    if( ch=='.' ) {
                        int pos = IndexOf(val,'.');
                        if( pos>0 ) {
                            extr = val+pos+1;
                            val[pos]=0;
                        }
                    }
                    // fst = false;
                    var = getFuncVar(fn, val);
                    if( var ) {
                        if( isNumberVar(var) ) {
                            toString(var,rst);
                            if( extr ) rst->add('.').add(extr);
                        } else {
                            rst->add(isVarTrue(var) ? "1": "0");
                            // qDebug("Expr data is invalid : %s\n",val);
                        }
                    } else {
                        rst->add('0');
                        qDebug("Expr data is null : %s\n",val);
                    }
                }
            }
        } break;
        } // end switch
    }
    return true;
}

inline void setChildFuncVar(XFunc* fcCur, StrVar* sv, XFuncNode* fn, StrVar* rst) {
    TreeNode* node=NULL;
    LPCC fnm=fcCur->getFuncName();
    if( SVCHK('n',0) ) {
        node=(TreeNode*)SVO;
    } else {
        qDebug("#0#[%s] func child var not valid (var:%s)\n", getBaseFuncName(fn), fnm);
        return;
    }
    if( fcCur->xdotPos>0 ) {
        int pos=IndexOf(fnm,'.');
        if( pos>0) {
            for( int n=0;n<8;n++ ) {
                LPCC cd=gBuf.add(fnm,pos);
                sv=node->gv(cd);
                if( SVCHK('n',0) ) {
                    node=(TreeNode*)SVO;
                } else {
                    qDebug("#0#[%s] func child node value error (var: %s)\n", getBaseFuncName(fn), fnm);
                    break;
                }
                fnm+=pos+1;
                pos=IndexOf(fnm,'.');
                if( pos<=0 ) break;
            }
        }
        if( slen(fnm) ) {
            sv=node->GetVar(fnm);
            rst->reuse()->add(sv);
        } else {
            qDebug("#0#[%s] func child node value error (var: %s)\n", getBaseFuncName(fn), fnm);
        }
    } else {
        sv=node->GetVar(fnm);
        rst->reuse()->add(sv);
    }
}
int execFunc(XFunc* func, XFuncNode* fn, StrVar* rst, StrVar* cur ) {
    if( func==NULL ) return 0;
    int rtn = 0, frtn = 0;
    int psize = func->getParamSize();
    switch( func->getType() ) {
        case FTYPE_USEGROUP: {
            execParams(func,fn,rst);
            qDebug()<< "FTYPE_USEGROUP = "<< KR(toString(rst));
        } break;
        case FTYPE_CALLBACK: {
            bool bset=func->xfkind==9 && func->parent() ? true : false;
            // XFunc* pfc=func->parent();
            makeLocalFunction(func, bset, fn, rst, NULL );
        } break;
        case FTYPE_TEMPLATE: {
        } break;
        case FTYPE_CONST: {
            rst->set(func->getValue());
        } break;
        case FTYPE_NULL: {
            // rst->reuse();
            rst->setVar('9',0);
            return 0;
        } break;
        case FTYPE_BOOL: {
            LPCC code=func->getValue();
            rst->setVar('3',ccmp(code,"true")?1:0);
            return 0;
        } break;
        case FTYPE_SINGLEVAR: {
            //version 1.0.3
            getOperValue(fn, func, func->getParamSize(), rst);
            return 0;
        } break;
        case FTYPE_VAR: {
            takeVarAdd( rst, func, fn);
        } break;
        case FTYPE_STATIC: {
            LPCC code = func->getFuncName();
            LPCC vnm = func->xpv.GetBuffer();
            if( ccmpl(vnm,2,"cf",2) ) {
                rst->add(getCf()->GetVar(code));
            } else {

            }
        } break;
        case FTYPE_TEXTVAR:
        case FTYPE_STR: {
            rst->add(func->getValue());
            return 0;
        } break;
        case FTYPE_EXPR: {

        } break;
        case FTYPE_DEFINE: {            
            if( func->xfuncRef==0 ) {
                if( func->xdotPos>0 ) {
                    func->xfuncRef = getHashFlagVal( func->getVarName(), func->getFuncName() );
                } else {
                    func->xfuncRef = getHashFlagVal( "FLAG", func->getValue() );
                }
            }
            rst->setVar('0',0).addInt((int)func->xfuncRef);
            return 0;
        } break;
        case FTYPE_TEXTDATA: {
            func->xpv.setPos();
            rst->add( func->xpv.get(), func->xpv.size() );
            return 0;
        } break;
        case FTYPE_TEXT: {
            takeParseVar(rst,func,fn);
            return 0;
        } break;
        case FTYPE_NUMBER: {
            LPCC val = func->getValue();
            if( val==NULL ) {
                rst->setVar('0',0).addInt(0);
                return 0;
            }
            bool bMinus=false;
            if( val[0]=='-' && !IS_DIGIT(val[1]) ) {
                StrVar* sv=getFuncVar(fn, val+1);
                if( SVCHK('4',0) ) {
                    double f=sv->getDouble(FUNC_HEADER_POS);
                    rst->setVar('4',0).addDouble(-1*f);
                } else if( SVCHK('0',0) ) {
                    int num=sv->getInt(FUNC_HEADER_POS);
                    rst->setVar('0',0).addInt(-1*num);
                } else if( SVCHK('1',0) ) {
                    int num=(int)sv->getUL64(FUNC_HEADER_POS);
                    rst->setVar('0',0).addInt(-1*num);
                } else {
                    val=toString(sv);
                    bMinus=true;
                }
                if( bMinus==false ) return 0;
            } else if( val[0]=='0' && toupper(val[1])=='X') {
                val+=2;
                U32 hex = 0;
                sscanf(val,"%x",&hex);
                rst->setVar('0',1).addU32(hex);
                return 0;
            }
            int len=0;
            bool dot=false;
            if( isnum(val, &len, &dot) ) {
                if( dot ) {
                    float f=(float)atof(val);
                    if( bMinus ) f*=-1;
                    rst->setVar('4',0).addDouble(f);
                } else {
                    int num = atoi(val);
                    if( bMinus ) num*=-1;
                    rst->setVar('0',0).addInt(num);
                }
            } else {

            }
        } break;
        case FTYPE_HASH: {
            TreeNode* root=cfNode("@objectNode");
            func->xpv.SetPosition();
            rst->reuse();
            if( func->xpv.ch()=='[' ) {
                if( func->xfuncRef==0 ) {
                    int idx=getCf()->incrNum("arrayIndex")+1;
                    func->xfuncRef=idx;
                }
                XListArr* arr =root->addArray(FMT("array%d",func->xfuncRef), true);
                func->xpv.incr();
                getCf()->setArrayValue(arr,func->xpv, fn);
                rst->setVar('a',0,(LPVOID)arr);
            } else {
                TreeNode* h = NULL;
                if( func->xfuncRef==0 ) {
                    int idx=getCf()->incrNum("hashIndex")+1;
                    func->xfuncRef=idx;
                }
                h=root->addNode(FMT("node%d",func->xfuncRef));
                h->setNodeFlag(FLAG_SKIP);
                h->setJson(func->xpv, fn);
                rst->setVar('n',0,(LPVOID)h);
            }
            return 0;
        } break;
        case 106:
        case 107:
        case 108:
        case FTYPE_SET:
        case FTYPE_PLUS:
        case FTYPE_MINUS:
        case FTYPE_DIV:
        case FTYPE_MULTI:
        case FTYPE_MOD: {
            func->xfuncRef = 0;
            rtn = execFuncSet(func,fn,rst);
            return 0;
        } break;
        case FTYPE_CONTROL: {
            int offset = 0;
            return callControl(func,fn,rst,&offset);
        } break;
        case FTYPE_BREAK: {
            return FTYPE_BREAK;
        } break;
        case FTYPE_RETURN: {
            frtn = FTYPE_RETURN;
            if( psize ) {
                XFunc* fparam=func->getParam(0);
                rst->reuse();
                execParamFunc(fparam,fn,rst);
            }
            return frtn;
        } break;
        case FTYPE_CONTINUE: {
            frtn = FTYPE_CONTINUE;
        } break;
        case FTYPE_FUNCCHECK:
        case FTYPE_FUNC_THIS:
        case FTYPE_FUNCCLASS:
        case FTYPE_FUNC_RESV:
        case FTYPE_FUNC: {
            int flag=0, nchk=1;
            if( func->getFuncSize()>0 ) {
                XFunc* pfc=func->parent();
                if( pfc && pfc->getFuncSize() && (pfc->xftype==FTYPE_SET||isFuncType(pfc)) ) {
                    flag=3;
                } else {
                    flag=1;
                }
            } else if( func->xfflag&FFLAG_PARAM ) {
                XFunc* pfc=func->parent();
                if( pfc && pfc->getFuncSize() ) {
                    if( pfc->xftype!=FTYPE_CONTROL && pfc->xftype!=FTYPE_CASE ) {
                        flag=2;
                    }
                }
            }
            if( func->xftype==FTYPE_FUNC_THIS ) {
                LPCC fnm=func->getFuncName();
                StrVar* sv = getFuncVar(fn,"@this"); // fn->gv("@this");
                if( SVCHK('n',0) ) {
                    TreeNode* thisNode=(TreeNode*)SVO;
                    XListArr* arrs=getParamArray(func,fn,rst);
                    if( !callNodeFunc(thisNode, fnm, arrs, fn, rst, NULL, func) ) {
                        qDebug("#9#[%s] %s.%s() this function not found (ox%x %d) \n",  getBaseFuncName(fn), "this", fnm, func->xfflag, func->xfuncRef);
                    }
                    releaseLocalArray(arrs);
                } else {
                    qDebug("#9#[%s] %s.%s() this function error \n",  getBaseFuncName(fn), "this", fnm);
                }
                nchk = 0;
            } else if( func->xftype==FTYPE_FUNCCHECK ) {
                if( func->xfkind==0 ) {
                    LPCC vnm = func->getValue();
                    func->xfkind =
                        ccmp(vnm,"print") ? FKIND_PRINT:
                        // ccmp(vnm,"expr") ? FKIND_EXPR:
                        ccmp(vnm,"use") ? FKIND_REUSE :
                        // ccmp(vnm,"class") ? 89 :
                        // ccmp(vnm,"config") ? FKIND_REGEXPR:
                        ccmp(vnm,"eq") ? 92 :
                        ccmp(vnm,"flags") ? 93 :
                        ccmp(vnm,"get") ? 101 :
                        // ccmp(vnm,"instance") ? 102 :
                        ccmp(vnm,"conf") ? 103 :
                        ccmp(vnm,"var") ? 104 :
                        ccmp(vnm,"call") ? 106 :                        
                        ccmp(vnm,"echo") ? 107 :
                        ccmp(vnm,"max") ? 108 :
                        ccmp(vnm,"min") ? 109 :
                        ccmp(vnm,"nvl") ? 112 :
                        ccmp(vnm,"not") ? 113 :
                        ccmp(vnm,"alert") ? 116 :
                        ccmp(vnm,"eval") ? 200 :
                        ccmp(vnm,"fmt") ? 201 :
                        ccmp(vnm,"args") ? 207 :
                        ccmp(vnm,"typeof") ? 208 :
                        ccmp(vnm,"when") ? 209 :
                        // ccmp(vnm,"sp") ? 207 :
                        // ccmp(vnm,"path") ? FKIND_PATH :
                        // ccmp(vnm,"array") ? FKIND_ARRAY :
                        // ccmp(vnm,"node") ? FKIND_JSON : 0;
                        // ccmp(vnm,"isnull") ? FKIND_ISNULL :
                        // ccmp(vnm,"alert") ? 90 :
                        // ccmp(vnm,"confirm") ? 91 :
                        // ccmp(vnm,"hash") ? FKIND_HASH : => json으로 대체
                        // ccmp(vnm,"event") ? FKIND_EVENT :
                        // ccmp(vnm,"range") ? FKIND_RANGE :
                        // ccmp(vnm,"use") ? FKIND_REUSE :
                        0;
                }

                if( func->xfkind==FKIND_REUSE ) {
                    setUseVars(func, fn, rst);
                } else if( func->xfkind==104 ) {
                    int cnt = func->xparams.size();
                    for( int n=0;n<cnt;n++ ) {
                        XFunc* param = func->xparams.getvalue(n);
                        if( param->getType()==FTYPE_VAR) {
                            LPCC name=param->getValue();
                            fn->GetVar(name)->reuse();
                        }
                    }
                } else if( func->xfkind==106 ) {
                    execCallFunc(func, fn, rst);
                } else if( func->xfkind>=200 ) {
                    execCheckFunc(func,NULL,fn,rst);
                } else {
                    XListArr* arrs=getParamArray(func,fn,rst);
                    execCheckFunc(func,arrs,fn,rst);
                    releaseLocalArray(arrs);
                }
                nchk = 0;
            } else if( func->xftype==FTYPE_FUNC_RESV ) {
                // System, Math, Cf ...
                /*
                U16 fchk = func->xflag >> 16;
                if( func->xdotPos < fchk ) {
                    LPCC vnm = func->getVarName();
                    if( ccmpl(vnm,2,"cf",2) ) {
                        int sp = func->xpv.wsp;
                        LPCC code = func->xpv.GetChars(sp+func->xdotPos+1,sp+fchk-1);
                        cur = getCf()->GetVar(code);
                        if( cur ) {
                            if( func->xfkind==0 )
                                func->xfkind = cur->charAt(1);
                            nchk = 2;
                        }
                    }
                } else {
                }
                */
                PARR arrs=getParamArray(func,fn,rst);
                execInternalFunc(func,arrs,fn,rst);
                if( flag && cur==NULL ) {
                    cur= fn->GetVar("@rst");
                    cur->reuse()->add(rst);
                }
                releaseLocalArray(arrs);
                nchk = 0;
            }

            if( nchk ) {
                /* ################# ver 1.0.3 */
                StrVar* sv=NULL;
                PARR arrs=NULL;
                bool berror=false;
                bool cc = ( cur==rst && cur->charAt(0)!='\b'); // cur && func->xftype==FTYPE_FUNC_THIS ||
                if( cc || (func->xdotPos>0 && nchk==1) ) {
                    LPCC varNameCode=NULL;
                    LPCC funcName=func->getFuncName();
                    bool bRef=false;
                    if( cc==false ) {
                        varNameCode=func->getVarName();
                        if( varNameCode[0]=='&' ) {
                            varNameCode+=1;
                            if( varNameCode[0]=='@' ) {
                                sv=getSubFuncSrc(varNameCode+1, funcName );
                                rst->reuse();
                                if( SVCHK('f',1) ) {
                                    rst->add(sv);
                                }
                                return frtn;
                            }
                            bRef=true;
                        } else {                            
                            cur=getFuncVar(fn, varNameCode);
                        }
                    }
                    if( cur ) {
                        if( cur->charAt(0)=='\b') {
                            if( varNameCode==NULL ) {
                                varNameCode=func->getVarName();
                                if( varNameCode[0]=='&' ) {
                                    varNameCode+=1;
                                    bRef=true;
                                }
                            }
                            /* ex) &this.aaa() */
                            if( bRef ) {
                                sv=cur;
                                if( SVCHK('n',0) ) {
                                    TreeNode* refNode=(TreeNode*)SVO;
                                    sv=refNode->gv(funcName);
                                    if( SVCHK('f',1) || SVCHK('f',0) ) {
                                        rst->reuse()->add(sv);
                                        return frtn;
                                    }
                                }
                                // releaseLocalArray(arrs);
                                return frtn;
                            }

                            if( !callObjectFunc(funcName, cur, fn, rst, func ) ) {
                                qDebug("#0#[%s] %s.%s() object function error (ox%x, %d) \n",  getBaseFuncName(fn), varNameCode, funcName, func->xfflag, func->xfuncRef);
                                berror=true;
                            }
                        } else {		// if( func->xftype!=FTYPE_FUNC_THIS )
                            if( bRef ) {
                                rst->reuse();
                                return frtn;
                            }
                            if( func->xfuncRef==0  ) {
                                regStrFuncs();
                                func->xfuncRef = getHashKeyVal("str", funcName );
                                if( func->xfuncRef ) {
                                    func->xfflag|= FFLAG_SAVE;
                                }
                                // qDebug("#0# %s maybe string function [value:%s] \n", funcName, cur->get() );
                            }
                            if( func->xfuncRef ) { // && (func->xdotPos==0 || func->xfflag&FFLAG_STRSET) func->xfflag==0 ||
                                arrs=getParamArray(func,fn,rst);
                                if( !execStrFunc(func,arrs,fn,cur,rst) ) {
                                    if( func->xfflag&FFLAG_SAVE ) {
                                        func->xfuncRef=0;
                                    }
                                }
                            } else {
                                qDebug("#9#[%s] function not define (name:%s, function:%s])\n", getBaseFuncName(fn), cur->get(), func->getFuncName());
                                berror=true;
                            }
                        }
                    } else if( varNameCode ) {
                        if( bRef ) {
                           sv=getSubFuncSrc(varNameCode, funcName );
                           rst->reuse();
                           if( SVCHK('f',1) ) {
                               rst->add(sv);
                           }
                           return frtn;
                        }
                        if( varNameCode[0]=='@' ) {
                            arrs=getParamArray(func,fn,rst);
                            execCallSubfunc(varNameCode+1, funcName, fn, arrs, rst, false, true);
                        } else if( func->xfuncRef>0 ) {
                            qDebug("#0#[%s] group function index error (%s.%s) \n", getBaseFuncName(fn), varNameCode, funcName );
                            berror=true;
                        } else {
                            berror=true;
                            qDebug("#0#[%s] group function is not define (name: %s.%s)  \n", getBaseFuncName(fn), varNameCode, funcName );
                        }
                    } else {
                        // version 1.0.4 ### error check ###
                        qDebug("#0#[%s] var not define (name: %s.%s) \n", getBaseFuncName(fn), "undefine", funcName );
                        berror=true;
                    }
                } else {		// may be user function
                    /*
                    if( nchk==2 && flag>0 ) {
                        func->xfflag|= FFLAG_PPARAM;
                    }
                    */
                    arrs=getParamArray(func,fn,rst);
                    if( cur ) {
                        if( func->xfuncRef==0 ) {
                            LPCC funcNm = func->getFuncName();
                            regStrFuncs();
                            func->xfuncRef = getHashKeyVal("str", funcNm );
                        }
                        execStrFunc(func,arrs,fn,cur,rst);
                    } else {
                        LPCC funcNm = func->getFuncName();
                        if(slen(funcNm)==0) {
                            return 0;
                        }
                        StrVar* sv=NULL;
                        if( funcNm[0]=='&' ) {
                            LPCC key=funcNm+1;
                            sv = getStrVar("fsrc",key);
                            if( SVCHK('f',0) || SVCHK('f',1) ) {
                                rst->reuse()->add(sv);
                            } else {
                                sv=fn->gv(key);
                                if( SVCHK('f',0) || SVCHK('f',1) ) {
                                    rst->reuse()->add(sv);
                                }
                            }
                            funcNm=NULL;
                        }
                        if( funcNm) {
                            TreeNode* thisNode=NULL;
                            sv=fn->gv("@this"); // gv("@inlineNode");
                            if( SVCHK('n',0) ) {
                                thisNode=(TreeNode*)SVO;
                                sv=thisNode->gv(funcNm);
                            }
                            if( thisNode && SVCHK('f',1) ) {
                                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                                XFunc* pfc=fsrc?fsrc->xfunc: NULL;
                                if( pfc ) {
                                    /*
                                    XFuncNode* fnParent=fn;
                                    U16 fflag=pfc?pfc->xflag: 0;
                                    sv=thisNode->gv("onInit");
                                    if( SVCHK('f',0)) fnParent=(XFuncNode*)SVO;
                                    */
                                    XFuncNode* fnCur = gfns.getFuncNode(pfc, fn );
                                    getCf()->set("@currentFuncName",funcNm);
                                    setFuncSrcParam(fnCur,arrs,fsrc,0,fn );
                                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                                    /*
                                    fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)thisNode);
                                    if( fn!=fnParent) {
                                        fnCur->GetVar("@funcNode")->setVar('n',0,(LPVOID)fn);
                                    }
                                    */
                                    fnCur->call(pfc,rst->reuse());
                                    funcNodeDelete(fnCur);
                                    // printCallLog(funcNm );
                                }
                            } else if( !funcCall( getFuncVar(fn,funcNm), fn, arrs, rst) ) {
                                if( !execUserFunc(func,arrs,fn,cur,rst,funcNm) ) {
                                    qDebug("#0#[%s] function call error (name: %s)\n", getBaseFuncName(fn), funcNm);
                                }
                            }
                        }
                    }
                    // execObjectFunc( func, getParamArray(func,fn,rst),fn,cur,rst );
                }
                releaseLocalArray(arrs);
                if( berror ) {
                    rst->reuse();
                    return frtn;
                }
            }

            if( flag ) {
                if( flag==3) {
                    XFunc* pfc=func->parent();
                    int ssize=func->getFuncSize();
                    int row = func->xflag;
                    qDebug("...flag==3 size:%d, row:%d",ssize, row);
                    for(int n=0;n<ssize;n++ ) {
                        XFunc* fcCur = func->xsubs.getvalue(n);
                        int flag = fcCur->xflag;
                        if( row < flag ) {
                            if( ssize > 1) {
                                qDebug("row=%d, flag=%d ssize=%d\n", row, flag, ssize);
                                continue;
                            }
                        } else if( row>flag ) {
                            break;
                        }
                        LPCC fnm=fcCur->getFuncName();
                        qDebug("xx0 %s xx",fnm);
                        if( isFuncType(fcCur) ) {
                            if( rst->charAt(0)=='\b') {
                                PARR arrs=getParamArray(fcCur,fn,rst);
                                execObjectFunc(fcCur, arrs, fn, rst, rst, cur);
                                releaseLocalArray(arrs);
                            } else {
                                execFunc(fcCur, fn, rst, rst );
                            }
                        } else if( fcCur->xftype==FTYPE_VAR ) {
                            qDebug("#0##### var[0] ###\n");
                            setChildFuncVar(fcCur, rst, fn, rst);
                        }
                    }
                    ssize=pfc->getFuncSize();
                    for(int n=0;n<ssize;n++ ) {
                        XFunc* fcCur = pfc->xsubs.getvalue(n);
                        if( fcCur==func )
                            break;
                        LPCC fnm=fcCur->getFuncName();
                        qDebug("xx1 %s xx",fnm);

                        if( isFuncType(fcCur) ) {
                            if( rst->charAt(0)=='\b') {
                                PARR arrs=getParamArray(fcCur,fn,rst);
                                execObjectFunc(fcCur, arrs, fn, rst, rst, cur);
                                releaseLocalArray(arrs);
                            } else {
                                execFunc(fcCur, fn, rst, rst );
                            }
                        } else if( fcCur->xftype==FTYPE_VAR ) {
                            qDebug("#0##### var [1] ###\n");
                            setChildFuncVar(fcCur, rst, fn, rst);
                        }
                    }
                } else {
                    XFunc* pfc= flag==1?func: func->parent();
                    int ssize=pfc->xsubs.size();
                    if( ssize>0 ) {
                        U16 ftype = func->parent() ? func->parent()->xftype: 0;
                        if( cur==NULL )
                            cur=rst;
                        if( ftype>FTYPE_FUNCSTART && ftype<FTYPE_FUNCEND ) {
                            int row = func->xflag;
                            for(int n=0;n<ssize;n++ ) {
                                XFunc* fcCur = pfc->xsubs.getvalue(n);
                                int flag = fcCur->xflag;
                                if( row < flag ) {
                                    if( ssize > 1) {
                                        qDebug("row=%d, flag=%d ssize=%d\n", row, flag, ssize);
                                        continue;
                                    }
                                } else if( row>flag ) {
                                    break;
                                }
                                if( isFuncType(fcCur) ) {
                                    if( rst->charAt(0)=='\b') {
                                        PARR arrs=getParamArray(fcCur,fn,rst);
                                        execObjectFunc(fcCur, arrs, fn, rst, rst, cur);
                                        releaseLocalArray(arrs);
                                    } else {
                                        execFunc(fcCur, fn, rst, rst );
                                    }
                                } else if( fcCur->xftype==FTYPE_VAR ) {
                                    qDebug("#0##### var[0] ###\n");
                                    setChildFuncVar(fcCur, rst, fn, rst);
                                }
                            }
                        } else {
                            for(int n=0;n<ssize;n++ ) {
                                XFunc* fcCur = pfc->xsubs.getvalue(n);
                                if( fcCur==func )
                                    break;
                                if( isFuncType(fcCur) ) {
                                    if( rst->charAt(0)=='\b') {
                                        PARR arrs=getParamArray(fcCur,fn,rst);
                                        execObjectFunc(fcCur, arrs, fn, rst, rst, cur);
                                        releaseLocalArray(arrs);
                                    } else {
                                        execFunc(fcCur, fn, rst, rst );
                                    }
                                } else if( fcCur->xftype==FTYPE_VAR ) {
                                    qDebug("#0##### var [1] ###\n");
                                    setChildFuncVar(fcCur, rst, fn, rst);
                                }
                            }
                        }
                    }
                }
            }

        } break;
        case FTYPE_SETARRAY: {
            func->xpv.SetPosition();
            int pos = func->xpv.findPos("[");
            LPCC left = pos>0 ? func->xpv.left(): NULL;
            if( left && func->xpv.MatchWordPos("[","]") ) {
                getFuncArrayVar(fn, left, func->xpv.GetVar(), func->xpv.prev, func->xpv.cur, rst, NULL, func->getParam(0) );
            }
        } break;
        case FTYPE_ARRAY: {
            func->xpv.SetPosition();
            int pos = func->xpv.findPos("[");
            LPCC left = pos>0 ? func->xpv.left(): NULL;
            if( left && func->xpv.MatchWordPos("[","]",FIND_SKIP) ) {
                bool bok=true;
                rst->reuse();
                if(slen(left)==1) {
                    if( ccmp(left,"#") ) {
                        XParseVar pv(func->xpv.GetVar(), func->xpv.prev, func->xpv.cur);
                        makeTextData(pv, fn, rst, 0x20000);
                        // rst->add(func->xpv.GetVar(), func->xpv.prev, func->xpv.cur);
                        bok=false;
                    } else if( ccmp(left,"$") ) {
                        StrVar* sv=fn->gv("@selfNode");
                        if( SVCHK('n',0) ) {
                            XParseVar pv(func->xpv.GetVar(), func->xpv.prev, func->xpv.cur);
                            TreeNode* node=(TreeNode*)SVO;
                            parseArrayVar(pv, node, fn, rst, 0x100, func );
                        } else {
                            qDebug("#0#self node not define (line:%s)\n", func->getValue() );
                        }
                    } else if( ccmp(left,"S") ) {
                        rst->add(func->xpv.GetVar(), func->xpv.prev, func->xpv.cur);
                        bok=false;
                    } else if( ccmp(left,"E") ) {
                        XParseVar sub( func->xpv.GetVar(), func->xpv.prev, func->xpv.cur );
                        if( makeExprData(sub, fn, rst, NULL) ) {
                            if( !gMemDb.isopen() ) {
                                gMemDb.open(":memory:");
                            }
                            LPCC sql=gMemDb.xvar.fmt("select %s", rst->get());
                            qDebug("expr value=>%s sql=>%s\n",rst->get(), sql);
                            if( gMemDb.prepare(sql) ) {
                                LPCC val=gMemDb.fetchValue(0);
                                int len=0;
                                bool chk=false;
                                if( isnum(val, &len, &chk) ) {
                                    if( chk ) {
                                        rst->setVar('4',0).addDouble( atof(val) );
                                    } else {
                                        rst->setVar('0',0).addInt( atoi(val) );
                                    }
                                } else {
                                    rst->set(val);
                                }
                            }
                        }
                        bok=false;
                    }
                }
                if( bok ) {
                    StrVar* var = getFuncArrayVar(fn, left, func->xpv.GetVar(), func->xpv.prev, func->xpv.cur, rst, rst, func);
                    if( var ) {
                        if( checkFuncObject(var,'0',9) ) {
                            return var->getInt(4);
                        }
                        int ssize=func->xsubs.size();
                        if( ssize>0 ) {
                            //(na) 20150714
                            U16 ftype = func->parent() ? func->parent()->xftype: 0;
                            if( ftype>FTYPE_FUNCSTART && ftype<FTYPE_FUNCEND ) {
                                int row = func->xflag;
                                for(int n=0;n<ssize;n++ ) {
                                    XFunc* fcCur = func->xsubs.getvalue(n);
                                    int flag = fcCur->xflag;
                                    if( row < flag )
                                        continue;
                                    else if( row>flag ) break;

                                    if( var->charAt(0)=='\b') {
                                        PARR arrs=getParamArray(fcCur,fn,rst);
                                        execObjectFunc(fcCur, arrs, fn, var, rst );
                                        releaseLocalArray(arrs);
                                    } else {
                                        execFunc(func->xsubs.getvalue(n), fn, rst, var );
                                    }
                                }
                            } else {
                                for(int n=0;n<ssize;n++ ) {
                                    XFunc* fcCur = func->xsubs.getvalue(n);
                                    if( var->charAt(0)=='\b') {
                                        PARR arrs=getParamArray(fcCur,fn,rst);
                                        execObjectFunc(fcCur, arrs, fn, var, rst );
                                        releaseLocalArray(arrs);
                                    } else {
                                        execFunc(func->xsubs.getvalue(n), fn, rst, var );
                                    }
                                    if( fcCur->xsubs.size()>0 ) {
                                        execSubObjectFunc(fcCur,fn,rst);
                                    }
                                    var = rst;
                                }
                            }
                        } else if( checkFuncObject(var,'f',0) ) {
                            XFuncNode* fnCur = (XFuncNode*)var->getObject(FUNC_HEADER_POS);
                            if( func->xfflag&FFLAG_PARAM ) {
                                rst->setVar('f',0,(LPVOID)fnCur);
                            } else {
                                fnCur->call(NULL,rst);
                            }
                        } else if( rst!=var ) {
                            rst->reuse()->add(var);
                        }
                    }
                }
            } else {
                qDebug()<<"FTYPE_ARRAY call error : "<<KR(func->getValue());
                return 0;
            }
        } break;
        case FTYPE_THIS: {
            StrVar* sv = getFuncVar(fn,"@this"); // fn->gv("@this");
            if( sv==NULL ) {
                if( fn->xfunc && fn->xfunc->xftype==FTYPE_CONTROL ) {
                    sv=getFuncNodeVar(fn,"@this",FTYPE_CONTROL);
                }
            }
            if( sv ) {
                if( func->xfkind==FTYPE_ARRAY ) {
                    func->xpv.SetPosition();
                    if( func->xpv.findPos("[")!=-1 && func->xpv.MatchWordPos("[","]") ) {
                        StrVar* var = getFuncArrayVar(fn, NULL, func->xpv.GetVar(), func->xpv.prev, func->xpv.cur, rst, sv, func); // getArrayKeyVar(sv,func->xpv.v(),fn);
                        if( var ) {
                            int sz = func->getFuncSize();
                            if( sz>0 ) {
                                XFunc* fc = NULL;
                                for(int x=0; x<sz; x++ ) {
                                    XFunc* fcCur = func->getFunc(x);
                                    PARR arrs=getParamArray(fcCur,fn,rst);
                                    if( x==0 ) {
                                        fc = fcCur;
                                        fcCur->xfflag|=FFLAG_SUBS;
                                        execObjectFunc(fcCur,arrs,fn,var,rst);
                                    } else {
                                        execObjectFunc(fcCur,arrs,fn,rst,rst);
                                    }
                                    releaseLocalArray(arrs);
                                }
                                if( fc && fc->getFuncSize() ) {
                                    sz = fc->getFuncSize();
                                    for(int x=0;x<sz; x++ ) {
                                        XFunc* fcCur = fc->getFunc(x);
                                        PARR arrs=getParamArray(fcCur,fn,rst);
                                        execObjectFunc(fcCur,arrs,fn,rst,rst);
                                        releaseLocalArray(arrs);
                                    }
                                }
                            } else if( checkFuncObject(var,'f',0) ) {
                                XFuncNode* fnCur = (XFuncNode*)var->getObject(FUNC_HEADER_POS);
                                if( func->xfflag&FFLAG_PARAM ) {
                                    rst->setVar('f',0,(LPVOID)fnCur);
                                } else {
                                    fnCur->call(NULL,rst);
                                }
                            } else if( rst!=var ) {
                                rst->reuse()->add(var);
                            }
                        }
                    }
                } else if( func->xfkind==FTYPE_SET ) {
                    // version 1.0.3 this.a.b=data 처리
                    LPCC varName = func->xpv.GetChars(func->xpv.wsp+5);
                    if( SVCHK('n',0) ) {
                        TreeNode* tn=(TreeNode*)SVO;
                        int pos=IndexOf(varName,'.');
                        if( pos>0 ) {
                            for( int n=0;n<8 ;n++ ) {
                                LPCC cd=gBuf.add(varName,pos);
                                sv=n==0 ? getVarValue(tn, cd) : tn->gv(cd); // sv=getVarValue(tn, cd);
                                if( sv==NULL ) {
                                    tn=NULL;
                                    break;
                                }
                                if( SVCHK('n',0) ) {
                                    tn=(TreeNode*)SVO;
                                }
                                varName+=pos+1;
                                pos=IndexOf(varName,'.');
                                if( pos<=0 ) break;
                            }
                        }
                        if( tn ) {
                            sv=tn->GetVar(varName);
                            if( SVCHK('f',0) ) {	// SVCHK('n',0) || SVCHK('a',0) || SVCHK('f',1) ||
                                varName = func->xpv.GetChars(func->xpv.wsp+5);
                                qDebug("#0#<<warning>> var already setting (var:%s, type:%s)\n", varName, getObjecTypeName(sv) );
                            } else {
                                execParams(func,fn,sv->reuse());
                            }
                        } else {
                            // ### version 1.0.4 ###
                            varName = func->xpv.GetChars(func->xpv.wsp+5);
                            qDebug("#0#[%s] var not define (var:%s)\n", getBaseFuncName(fn), varName );
                        }
                    }
                } else {
                    if( func->getFuncSize() ) {
                        XFunc* fc = func->getFunc(0);
                        PARR arrs=getParamArray(fc,fn,rst);
                        execObjectFunc(fc,arrs,fn,sv,rst);
                        releaseLocalArray(arrs);
                    } else if( func->xdotPos>0 ) {
                        if( SVCHK('n',0) ) {
                            rst->reuse()->add( getSubVarValue((TreeNode*)SVO, func->getFuncName()) );
                        } else {
                            LPCC var = func->getVarName();
                            if( slen(var) ) {
                                sv = fn->gv(gBuf.fmt("@%s",var));
                                if( sv ) rst->reuse()->add(sv);
                            } else
                                rst->reuse()->add(sv);
                        }
                    } else {
                        rst->reuse()->add(sv);
                    }
                }
            } else {
                LPCC var = func->xdotPos==4? func->getFuncName() : NULL;
                if( slen(var) ) {
                    StrVar* sv = fn->gv(gBuf.fmt("@%s",var));
                    if( sv ) rst->reuse()->add(sv);
                } else {
                    LPCC val=func->getValue();
                    qDebug("FTYPE_THIS call error : %s is null", val );
                    rst->setVar('e',1).format(256,"FTYPE_THIS call error : %s is null", val);
                }
            }
        } break;
        default: {
        } break;
    }

#ifdef FUNC_OPER_USE

    if( psize==0 || func->parent()==NULL ) {
        return frtn;
    }
    if( func->xfflag==0x32  && isFuncType(func) ) {
        XFunc* fparam=func->getParam(0);
        int psz = fparam->getParamSize();
        if( psz>1 ) {
            XFunc* sub=fparam->getParam(0);
            if( sub && sub->xftype==FTYPE_OPER ) {
                fparam->xfflag|=FFLAG_OPEFCALL;
                getOperValue(fn, fparam, psz, rst);
            }
        }
    } else if( func->xfflag==0xA || func->xfflag&FLAG_CALL ) {
        XFunc* param = func->getParam(psize-1);
        if( param==NULL ) {
            qDebug("#0#func check error(name:%s, param size:%d)\n", func->getValue(), psize);
            return frtn;
        }
        if( param->xfflag==0x1022 ) {
            int psz = param->getParamSize();
            if( psz>2 ) {
                XFunc* sub=param->getParam(2);
                if( sub && sub->xftype==FTYPE_OPER ) {
                    param->xfflag|=FFLAG_OPEFCALL;
                    getOperValue(fn, param, psz, rst);
                }
            }

        } else if( param->xfflag&FFLAG_USE_SUB ) {
            int psz = param->getParamSize();
            if( psz>0 && param->getParam(0)->xftype==FTYPE_OPER ) {
                param->xfflag|=FFLAG_OPEFCALL;
                if( (param->xfflag&FFLAG_REF)==0 ) {
                    param->xfflag|=FFLAG_OPEFCALL;
                    getOperValue(fn, param, psz, rst);
                }
            }
        }
    } else {
        XFunc* pfc =func->parent();
        if( pfc && (pfc->xfflag&FFLAG_REF)==0 ) {
            if( isFuncType(pfc) && pfc->xparams.size()>1 ) {
                // qDebug()<<"xxxxxxxxxxxxxx";
            } else {
                U16 ptype = pfc->xftype;
                bool bchk = ptype>100 && ptype<110;
                if( bchk || ptype==FTYPE_SET || ptype==FTYPE_CONTROL ) {	// || ptype==FTYPE_RETURN
                    XFunc* fcur = func;
                    XFunc* fparam=func->getParam(0);
                    if( psize>1 && fparam && fparam->xftype!=FTYPE_OPER ) {
                        /*
                        fcur = func->getParam(psize-1);
                        psize = fcur->xparams.size();
                        fparam= psize>0 ? fcur->getParam(0) : NULL;
                        */
                        fcur = func->getParam(psize-1);
                        psize = fcur ? fcur->xparams.size(): 0;
                        if( fcur && fcur->xfflag==0x1022 ) {
                            fparam= psize>2 ? fcur->getParam(2) : NULL;
                        } else {
                            fparam= psize>0 ? fcur->getParam(0) : NULL;
                        }
                    }
                    if( fcur && fparam && fparam->xftype==FTYPE_OPER ) {
                        int psz = fcur->getParamSize();
                        if( psz>1 && (fcur->xfflag&FFLAG_REF)==0 ) {
                            fcur->xfflag|=FFLAG_OPEFCALL;
                            getOperValue(fn, fcur, psz, rst);
                        }
                    }
                }
            }
        }
    }
#endif
    return frtn;
}
inline LPCC setNewObjectCode(LPCC var) {
    return FMT("%s:%d",var, getCf()->incrNum(FMT("@%s_index",var)) );
}
inline bool setNewObject( LPCC var, LPCC fnm, StrVar* sv, XFunc* func, XFuncNode* fn ) {
    TreeNode* tn=NULL;
    if( fnm && SVCHK('n',0) ) {
        // ex) obj.id=NEW.node
        tn=(TreeNode*)SVO;
        int pos=IndexOf(fnm,'.');
        if( pos>0) {
            for( int n=0;n<8;n++ ) {
                LPCC cd=gBuf.add(fnm,pos);
                if( slen(cd)==0 ) break;
                fnm+=pos+1;
                pos=IndexOf(fnm,'.');
                if( pos==-1 ) {
                    break;
                }
                sv=getVarValue(tn, cd);
                if( SVCHK('n',0) ) {
                    tn=(TreeNode*)SVO;
                } else {
                    tn=tn->addNode(cd);
                }
            }
        }
    }

    if( ccmp(var,"node") || ccmp(var,"array") ) {
        if( tn ) {
            if( slen(fnm)==0 ) {
                qDebug("#9#new object error(type:%s)", var);
                return false;
            }
            if( ccmp(var,"node") ) {
                tn=tn->addNode(fnm);
            } else {
                tn->addArray(fnm);
            }
        } else if(sv) {
            if( ccmp(var,"node") ) {
                tn=getTreeNodeTemp();
                if( tn ) sv->setVar('n',0,(LPVOID)tn);
            } else {
                XListArr* arr=getListArrTemp();
                sv->setVar('a',0,(LPVOID)arr);
            }
        }
        return true;
    }
    if( fnm ) {
        if( tn==NULL ) {
            qDebug("#9#new object error(type:%s, name:%s)", var, fnm);
            return false;
        }
        LPCC code=setNewObjectCode(var);
        sv=tn->GetVar(fnm);
        if( setClassTagObject(var, code) ) {
            tn=getBaroObject(var,code);
        }
        if( tn && sv ) {
            sv->setVar('n',0,(LPVOID)tn);
            return true;
        }
    } else {
        if( tn==NULL ) {
            LPCC code=setNewObjectCode(var);
            if( setClassTagObject(var, code) ) {
                tn=getBaroObject(var,code);
            }
        }
        if( tn && sv ) {
            sv->setVar('n',0,(LPVOID)tn);
            return true;
        }
    }
    return false;
}
int execFuncSet(XFunc* func, XFuncNode* fn, StrVar* origin ) {
    int cnt=func->xparams.size();
    if( cnt==0 )
        return 0;

    LPCC code = func->xdotPos>0 ? func->getVarName(): func->getValue();
    if( slen(code)==0 )
        return 0;
    StrVar* sv = NULL;
    StrVar* rst = fn->gv(code);
    XFunc* pfc=NULL;
    LPCC inlineCode=NULL;
    if( rst==NULL ) {
        XFuncSrc* fsrc=func->getFuncSrc();
        pfc=fsrc?fsrc->xfunc: NULL;
        if( pfc && pfc->xflag&FFLAG_EQ ) {
            sv=fn->gv("@inlineNode");
            if( SVCHK('n',0) ) {
                TreeNode* thisNode=(TreeNode*)SVO;
                rst = thisNode->gv(code);
                inlineCode=code;
                if( checkFuncObject(rst,'f',1) ) {
                    rst=NULL;
                }
            }
        }
    }
    if( rst==NULL ) {
        if( code[0]=='@' ) {
            XFunc* p = func->getParam(0);
            if( p && p->getType()==FTYPE_CALLBACK ) {	//  && func->getType()==FTYPE_SET
                bool bset=p->xfkind==9 ? true: false;
                rst=fn->GetVar(code);
                makeLocalFunction(p, bset, fn, rst, code+1);
                return 0;
            }

            // version 1.0.2
            if( func->xdotPos==0 ) {
                XFuncNode* pfn=fn->parent();
                code++;
                for(int n=0; n<8 && pfn; n++ ) {
                    rst = pfn->gv(code);
                    if( rst ) // || pfn->isNodeFlag(FLAG_CALL) )
                        break;
                    pfn = pfn->parent();
                }
            }
        } else if( code[0]=='&' ) {
            code++;
            rst = fn->gv(code);
            if( checkFuncObject(rst,'9',0) ) {
                rst->reuse();
            }
            if( rst && rst->length() ) return 1;
        } else if( func->xdotPos>0 ) {
            if( pfc && pfc->xflag&FUNC_INLINE ) {
                sv=fn->gv("@inlineNode");
                if( SVCHK('n',0) ) {
                    TreeNode* thisNode=(TreeNode*)SVO;
                    rst = thisNode->gv(code);
                    // qDebug("... code:[%s] (rst==%s)", code, rst?rst->get():"-" );
                }
            }
            if( rst==NULL ) {
                XFuncNode* pfn = fn->parent();
                for(int n=0; n<8 && pfn; n++ ) {
                    rst = pfn->gv(code);
                    if( rst ) // || pfn->isNodeFlag(FLAG_CALL) )
                        break;
                    pfn = pfn->parent();
                }
            } else if( checkFuncObject(rst,'f',1) ) {
                rst=NULL;
            }
            if( rst==NULL ) {
                qDebug("#0#[%s] set value error (var: %s)\n", getBaseFuncName(fn), func->getValue() );
                return 0;
            }
        }
    }

    if( rst==NULL ) {
        rst = fn->GetVar(code);
        if( rst==NULL ) return 0;
    }
    /*
    if( checkFuncObject(rst,'f',1) ) {
        qDebug()<<code<<"maybe callback";
    }
    */

    StrVar* prev=NULL;
    if( func->getParamSize()>0 ) {
        if( func->xfkind==0 ) {
            XFunc* p = func->getParam(0);
            if( p->getType()==FTYPE_DEFINE ) {
                LPCC kind=p->getVarName();
                func->xfkind = 1;
                if( ccmp(kind,"NEW") ) {
                    LPCC var=p->getFuncName();
                    setNewObject(var, func->xdotPos?func->getFuncName():NULL, rst, p, fn);
                    return 1;
                }
            } else if( func->getType()==FTYPE_SET ) {
                if( p->xdotPos>0 && ccmp(code, p->getVarName()) ) {
                    func->xfkind = 2;
                } else if( p->getType()>FTYPE_FUNCSTART && p->getType()<FTYPE_FUNCEND ) {
                    for( int x=0;x<p->getParamSize();x++) {
                        XFunc* pp=p->getParam(x);
                        if( ccmp(code, pp->getVarName()) ) {
                            func->xfkind = 2;
                            break;
                        }
                    }
                }
                if( func->xfkind != 2 )
                    func->xfkind = 1;
            } else {
                func->xfkind = func->getType();
            }
        }
        if( func->xdotPos>0 ) {
            sv=rst;
            if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                LPCC fnm=func->getFuncName();
                int pos=IndexOf(fnm,'.');
                if( pos>0) {
                    StrVar* sv=NULL;
                    for( int n=0;n<8;n++ ) {
                        LPCC cd=gBuf.add(fnm,pos);
                        sv=node->gv(cd);
                        if( SVCHK('n',0) ) {
                            node=(TreeNode*)SVO;
                        } else {
                            qDebug("#0#[%s] set node value error (var: %s)\n", getBaseFuncName(fn), fnm);
                            return 0;
                        }
                        fnm+=pos+1;
                        pos=IndexOf(fnm,'.');
                        if( pos<=0 ) break;
                    }
                }
                if( node && slen(fnm) ) {
                    bool vchk=true;
                    if(ccmp(fnm,inlineCode) ) {
                        sv=fn->gv("@inlineNode");
                        if( SVCHK('n',0) ) {
                            TreeNode* thisNode=(TreeNode*)SVO;
                            if(thisNode==node) {
                                rst=fn->GetVar(inlineCode);
                                vchk=false;
                            }
                        }
                    }
                    if( vchk ) {
                        rst=node->GetVar(fnm);
                    }
                } else {
                    return 0;
                }
            } else {
                LPCC fnm=func->getFuncName();
                qDebug("#0#[%s] set node value error (var:%s)\n", getBaseFuncName(fn), fnm);
                return 0;
            }
        }
        if( func->xfkind==1 ) {
            rst->reuse();
        } else if( func->xfkind==2 ) {
            prev=rst;
            rst=origin->reuse();
        }
    }

    if( func->getType()==FTYPE_SET ) {
        execParams(func,fn,rst);
        //#baropage
        StrVar* sv=NULL;
        if( checkFuncObject(rst,'s',0) ) {
            sv = (StrVar*)rst->getObject(FUNC_HEADER_POS);
        }
        if( prev && sv!=prev  ) {
            prev->reuse()->add(rst);
        } else if( sv && func->xdotPos>0 ) {
            int sp=0,ep=0;
            sv=getStrVarPointer(rst, &sp, &ep);
            if( sv!=rst ) {
                rst->reuse();
                if( sp<ep ) rst->add(sv->get(sp), ep-sp );
            }
        }

    } else if( func->getType()>100  ) {
        U16 ty = func->getType();
        if( isNumberVar(rst) ) {
            execParamFunc(func->getParam(0),fn,origin->reuse());
            if( rst->charAt(0)=='\b' && rst->charAt(1)=='4' ) {
                float a=(float)toDouble(rst);
                float c=(float)toDouble(origin);
                switch( ty ) {
                case FTYPE_PLUS:	{ rst->setVar('4',0).addDouble(a+c); } break;
                case FTYPE_MINUS:	{ rst->setVar('4',0).addDouble(a-c); } break;
                case FTYPE_DIV:		{ rst->setVar('4',0).addDouble(c? (double)(a/c):0); } break;
                case FTYPE_MULTI:	{ rst->setVar('4',0).addDouble(a*c); } break;
                }
            } else {
                if( origin->charAt(0)=='\b' && origin->charAt(1)=='4' ) {
                    float a=(float)toDouble(rst);
                    float c=(float)toDouble(origin);
                    switch( ty ) {
                    case FTYPE_PLUS:	{ rst->setVar('4',0).addDouble(a+c); } break;
                    case FTYPE_MINUS:	{ rst->setVar('4',0).addDouble(a-c); } break;
                    case FTYPE_DIV:		{ rst->setVar('4',0).addDouble(c? (double)(a/c):0); } break;
                    case FTYPE_MULTI:	{ rst->setVar('4',0).addDouble(a*c); } break;
                    }
                } else if( isNumberVar(origin) )  {
                    if( ty > 105 ) {
                        U32 a = (U32)toInteger(rst);
                        U32 b = (U32)toInteger(origin);
                        switch( ty ) {
                        case 106:			{ rst->setVar('0',0).addInt(a|b); } break;
                        case 107:			{ rst->setVar('0',0).addInt(a&b); } break;
                        case 108:			{ rst->setVar('0',0).addInt(a^b); } break;
                        }
                    } else {
                        int a = toInteger(rst);
                        int b = toInteger(origin);
                        switch( ty ) {
                        case FTYPE_PLUS:	{ rst->setVar('0',0).addInt(a+b); } break;
                        case FTYPE_MINUS:	{ rst->setVar('0',0).addInt(a-b); } break;
                        case FTYPE_DIV:		{ rst->setVar('4',0).addDouble(b? (double)(a/b):0); } break;
                        case FTYPE_MULTI:	{ rst->setVar('0',0).addInt(a*b); } break;
                        case FTYPE_MOD:		{ rst->setVar('0',0).addInt(a%b); } break;
                        }
                    }
                }
            }
        } else if( ty==FTYPE_PLUS ) {
            execFunc(func->getParam(0),fn,origin->reuse());
            if( ty==106 || rst->charAt(0)=='\b' )
                rst->reuse();
            rst->add(origin);
        } else {
            qDebug("#0#execFuncSet error var=%s, type=%d\n", code, ty);
        }
    }
    return 1;
}

int execCallFunc(XFunc* func, XFuncNode* fn, StrVar* rst) {
    // version 1.0.2
    StrVar* sv=NULL;
    int cnt= func->getParamSize();
    rst->reuse();
    if( cnt==0 ) {
        return 0;
    }
    sv=fn->gv("@callCheck");
    if( sv ) sv->reuse();
    PARR arrs=NULL;
    for( int n=0;n<cnt;n++ ) {
        XFunc* param = func->xparams.getvalue(n);
        if( arrs==NULL ) {
            arrs=getLocalArray(true);
        }
        StrVar* var = arrs->add();
        if( param->xdotPos>0 && param->xpv.ch()=='@' && !param->xpv.StartWith("@s[") ) {
            LPCC vnm=param->getVarName();
            sv=getSubFuncSrc(vnm+1, param->getFuncName() );
            if( SVCHK('f',1) ) {
                var->add(sv);
            }
        } else {
            execParamFunc(param, fn, var);
        }
    }
    if( arrs==NULL ) {
        return 0;
    }
    int ret=0;
    int sp=0, ep=0;
    sv=arrs->get(0);
    if( SVCHK('f',1) ) {
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        if( arrs->size() == 1 ) {
            // ### version 1.0.4 (ex) call(&func);
            TreeNode* thisNode=NULL;
            sv=fn->gv("@this");
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            }
            XFuncNode* pfn=NULL;
            if( thisNode ) {
                sv=thisNode->gv("onInit");
                if( SVCHK('f',0) ) {
                    pfn=(XFuncNode*)SVO;
                }
            }
            XFuncNode* fnCallback=gfns.getFuncNode(fsrc->xfunc,pfn);
            fnCallback->setNodeFlag(FLAG_PERSIST);
            rst->setVar('f',0,(LPVOID)fnCallback);
            releaseLocalArray(arrs);
            return 0;
        }
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            // ex) call( &test(a,b), page, button );
            TreeNode* thisNode=(TreeNode*)SVO;
            TreeNode* meNode=NULL;
            sv=arrs->get(2);
            if( SVCHK('n',0) ) {
                meNode=(TreeNode*)SVO;
            }

            XFunc* fcCur=func->getParam(0);
            if( fcCur ) {
                PARR arrsSub=getParamArray(fcCur,fn,rst);
                LPCC funcNm=fcCur->getValue();
                rst->set(funcNm);
                callFuncSrc(fsrc, fn, true, thisNode, meNode, NULL, arrsSub, rst );
                if( arrsSub ) {
                    releaseLocalArray(arrsSub);
                }
            }
        } else if( SVCHK('a',0) ) {
            XListArr* arrsSub=(XListArr*)SVO;
            sv=arrs->get(2);
            if( SVCHK('n',0) ) {
                TreeNode* thisNode=(TreeNode*)SVO;
                callFuncSrc(fsrc, fn, true, thisNode, NULL, NULL, arrsSub, rst->reuse() );
            }
        } else {
            // ex) call( class.func, src, param);
            sv=getStrVarPointer(arrs->get(1), &sp, &ep);
            if( sp<ep ) {
                XFunc* fcParam=func->getParam(0);
                LPCC fnm=fcParam->getValue();
                LPCC param=AS(2);
                fsrc->reuse();
                fsrc->xparam.reuse();
                fsrc->xflag=0;
                if( fsrc->xfunc ) {
                    deleteAllFunc(fsrc->xfunc);
                    fsrc->xfunc=NULL;
                }
                fsrc->readBuffer(sv, sp, ep);
                if( fsrc->makeFunc() && fsrc->xfunc ) {
                    if( slen(param) ) fsrc->xparam.set(param);
                    qDebug("#0# %s function modify ok\n", fnm);
                } else {
                    qDebug("#2# %s function modify fail !!!\n", fnm);
                }
            }
        }
        releaseLocalArray(arrs);
        return 0;
    } else if( SVCHK('f',0) ) {
        XFuncNode* fnParent=(XFuncNode*)SVO;
        // ex) call( parentFuncNode, subfunc, param) or call( node.onInit, src, true);
        sv=getStrVarPointer(arrs->get(1), &sp, &ep);
        return execFuncStr(fnParent, sv, sp, ep, rst, toString(arrs->get(2)) );
    } else if( SVCHK('n',0) ) {
        TreeNode* targetNode=(TreeNode*)SVO;
        sv=arrs->get(1);
        if( SVCHK('f',1) ) {
            // ex) call(page, &my.func(a,b)); inline func call
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFunc* fcCur=func->getParam(1);
            TreeNode* thisNode=NULL;
            XFuncNode* fnParent=fn;
            LPCC funcName=NULL;            
            sv=fn->GetVar("@this");
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
                sv=thisNode->gv("onInit");
                if( SVCHK('f',0) ) {
                    fnParent=(XFuncNode*)SVO;
                }
            }
            XFuncNode* fnCur = gfns.getFuncNode(fsrc->xfunc, fnParent);
            TreeNode* meNode=targetNode;
            PARR arrsSub=NULL;
            if( fcCur ) {
                funcName=fcCur->getValue();
                arrsSub=getParamArray(fcCur,fn,rst);
            }
            setFuncSrcParam(fnCur,arrsSub,fsrc,0,fn );
            fnCur->GetVar("@page")->setVar('n',0,(LPVOID)meNode);
            fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)targetNode);
            if( thisNode==NULL ) {
                thisNode=targetNode;
            }
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode );
            if( slen(funcName) ) {
                // getCf()->set("@currentFuncName", funcName);
            }
            fnCur->call(fsrc->xfunc,rst->reuse());
            funcNodeDelete(fnCur);
            if( arrsSub ) {
                releaseLocalArray(arrsSub);
            }
        } else {
            // ex) call(node, src, type);
            LPCC type = cnt>2 ? AS(2): NULL;
            sv=getStrVarPointer(arrs->get(1),&sp,&ep);
            if( ccmp(type,"inline") ) {
                XParseVar pv(sv, sp, ep);
                parseArrayVar(pv, targetNode, fn, rst, 0x400, func);
            } else {
                bool append = ccmp(type,"append") ? true: false;
                setModifyClassFunc(targetNode, sv, sp, ep, fn, rst, append);
            }
        }
        releaseLocalArray(arrs);
        return 0;
    }
    if( cnt>2 ) {
        // ex) call('test', 'localFunc', src, param );
        XFuncSrc* fsrc=NULL;
        LPCC localFuncName=AS(1);
        qDebug("####### localFuncName=%s\n", localFuncName);
        LPCC funcName=NULL;
        sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
        } else {
            funcName=AS(0);
            sv=getStrVar("fsrc",funcName);
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
        }
        if( fsrc ) {
            XFunc* fc=fsrc->xfunc;
            bool find=false;
            if( slen(funcName)==0 ) funcName="func";
            for( int x=0; x<fc->getFuncSize(); x++ ) {
                XFunc* fcCur=fc->getFunc(x);
                if( fcCur->xftype!=FTYPE_SET ) {
                    continue;
                }
                XFunc* fcParam = fcCur->getParam(0);
                if( fcParam && fcParam->getType()==FTYPE_CALLBACK ) {
                    LPCC varName=fcCur->getVarName();
                    if( ccmp(varName,localFuncName) ) {
                        XFuncSrc* fsrcCur=NULL;
                        if( fcParam->xfuncRef==0 ) {
                            int idx=getCf()->incrNum("@cbidx");
                            fsrcCur=gfsrcs.getFuncSrc();
                            fcParam->xfuncRef=idx;
                            LPCC fcode=gBuf.fmt("c%d",fcParam->xfuncRef);
                            getStrVar("func", fcode, 'f',1,(LPVOID)fsrcCur );
                        }
                        StrVar* src=getStrVarPointer(arrs->get(2), &sp, &ep);
                        qDebug("#0#call func => funcName=%s localFuncName=%s\n", funcName, localFuncName );
                        if( fsrcCur==NULL ) {
                            LPCC fcode=gBuf.fmt("c%d",fcParam->xfuncRef);
                            sv=getStrVar("func",fcode );
                            if( SVCHK('f',1) ) {
                                fsrcCur=(XFuncSrc*)SVO;
                            }
                        }
                        if( fsrcCur ) {
                            reloadFuncSrc(fsrcCur, src, sp, ep, varName);
                            if( cnt==4 ) {
                                LPCC param=toString(arrs->get(3));
                                fsrcCur->xparam.set(param);
                            }
                        }
                        find=true;
                        break;
                    }
                }
            }
            if( find==false ) {
                XFuncSrc* fsrcTemp=getFuncSrc();
                rst->set(localFuncName);
                if( cnt==4 ) {
                    LPCC param=toString(arrs->get(3));
                    if( slen(param) ) {
                        rst->add("=func(").add(param).add(") ");
                    }
                }
                StrVar* src=getStrVarPointer(arrs->get(2), &sp, &ep);
                rst->add("{").add(src, sp, ep).add("}");
                if( fsrcTemp->parseData(rst, 0, rst->length()) ) {
                    sp=fsrc->length();
                    fsrc->add((StrVar*)fsrcTemp);
                    ep=fsrc->length();
                    XParseVar pv((StrVar*)fsrc, sp, ep);
                    fsrc->makeFunc(pv, fc);
                }
                if( funcName && localFuncName ) {
                    qDebug("#0##modify localfunction error funcName %s must add localFuncName=%s \n", funcName, localFuncName);
                } else {
                    qDebug("#0##modify localfunction error");
                }
            }
        }
        releaseLocalArray(arrs);
        return 0;
    }

    sv=getStrVarPointer(arrs->get(0),&sp,&ep);
    if( sp>=ep ) {
        return 0;
    }
    XParseVar pv(sv, sp, ep);
    char c=pv.ch();
    bool sfunc=false;
    int sa=pv.start, ea=sa;
    if( c=='@' ) {
        pv.incr();
        sa++;
        sfunc=true;
    }
    pv.moveNext();
    ea=pv.start;
    c=pv.ch();
    if( c=='#' ) {
        pv.incr().moveNext();
        ea=pv.start;
        c=pv.ch();
    }
    LPCC funcNm=NULL;
    if( c=='.' ) {
        pv.incr();
        int fa=pv.start,fb=0;
        pv.moveNext();
        if( pv.ch()=='#' ) {
            pv.incr().moveNext();
        }
        c=pv.ch();
        if( c==0 ) {
            fb=pv.start;
            if( fa<fb ) {
                funcNm=gBuf.fmt("%s:%s", pv.Trim(sa, ea), pv.Trim(fa,fb) );
            }
        }
        sfunc=true;
    } else if( c==0 ) {
        funcNm=pv.Trim(sa, ea);
    }
    if( c==0 && slen(funcNm) ) {
        if( sfunc ) {
            sv=getSubFuncSrc(funcNm,NULL);
        } else {
            sv=fn->gv( funcNm ); // getFuncVar(fn, funcNm); //
            if( !SVCHK('f',1) ) {
                sv=getStrVar("fsrc",funcNm);
            }
        }
        if( SVCHK('f',1) || SVCHK('f',0) ) {
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                if( fsrc->xfunc ) {
                    LPCC key=AS(1);
                    if( ccmp(key,"inline") ) {
                        fsrc->xfunc->xflag|=FUNC_INLINE;
                    }
                }
            }
            rst->reuse()->add(sv);
        } else {
            rst->setVar('3',0);
        }
    } else {
        ret=setFuncSource(sv, sp, ep);
        rst->setVar('3', ret==0? 0: 1 );
    }
    releaseLocalArray(arrs);
    return ret;
}
void execSubObjectFunc(XFunc* func, XFuncNode* fn, StrVar* rst, int depth) {
    int ssize=func->xsubs.size();
    U16 ftype = func->parent() ? func->parent()->xftype: 0;
    if( ftype>FTYPE_FUNCSTART && ftype<FTYPE_FUNCEND ) {
        int row = func->xflag;
        for(int n=0;n<ssize;n++ ) {
            XFunc* fcCur = func->xsubs.getvalue(n);
            if( fcCur==func )
                continue;
            int flag = fcCur->xflag;
            if( row < flag )
                continue;
            else if( row>flag ) break;

            if( rst->charAt(0)=='\b') {
                PARR arrs=getParamArray(fcCur,fn,rst);
                execObjectFunc(fcCur, arrs, fn, rst, rst  );
                releaseLocalArray(arrs);
            } else {
                execFunc(fcCur, fn, rst, rst );
            }
        }
    } else {
        for(int n=0;n<ssize;n++ ) {
            XFunc* fcCur = func->xsubs.getvalue(n);
            if( fcCur==func )
                break;
            if( rst->charAt(0)=='\b') {
                PARR arrs=getParamArray(fcCur,fn,rst);
                execObjectFunc(fcCur, arrs, fn, rst, rst  );
                releaseLocalArray(arrs);
            } else {
                execFunc(fcCur, fn, rst, rst );
            }
        }
    }

}

bool execUserFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst, LPCC funcName) {
    XFuncSrc* fsrc = NULL;
    if( funcName==NULL ) {
        funcName = var ? toString(var): fc->getFuncName();
    }
    StrVar* sv = getStrVar("fsrc",funcName);
    if( SVCHK('f',1) ) {
        fsrc = (XFuncSrc*)SVO;
    }
    bool rtn = false;
    // pushCallFuncName(funcName);
    if( fsrc && fsrc->xfunc  ) {
        callUserFunc(fsrc, arrs, fn, rst);
        rtn=true;
    }
    return rtn;
}

XFuncSrc* nodeInlineFunc( XFunc* fc, int findex, StrVar* var, int psp, int pep, int sp, int ep, StrVar* rst ) {
   XFuncSrc* fsrc=NULL;
   StrVar* sv=NULL;
   int ref=fc ? fc->xfuncRef: 0;
   if( fc && ref==0 ) {
       ref=getCf()->incrNum("@nodeFuncidx");
       if( ref==0 ) ref=getCf()->incrNum("@nodeFuncidx");
       fc->xfuncRef=ref;
   }
   if( ref==0 ) {
       return NULL;
   }
   LPCC code=gBuf.fmt("f%d_%d", ref, findex);
   sv=getStrVar("nodeFunc",code );
   if( SVCHK('f',1) ) {
       fsrc=(XFuncSrc*)SVO;
   } else {
       fsrc=gfsrcs.getFuncSrc();
       if( psp<pep ) {
           fsrc->xparam.set(var, psp, pep);
       }
       fsrc->readBuffer(var, sp, ep );
       fsrc->makeFunc();
       if( fsrc->xfunc ) {
           // fsrc->xflag|=0x200;
           getStrVar("nodeFunc", code, 'f',1,(LPVOID)fsrc );
       }
   }
   return fsrc;
}

inline bool moveVarPos(XParseVar& sub ) {
    // bool cchk=false;
    sub.moveNext();
    char ch=sub.CharAt(sub.start);
    if( ch=='#' || ch=='@' || ch==' ' ) {
        // cchk=true;
        if( ch==' ' ) {
            U8 c=sub.ch(1);
            if( c>128 || isalnum(c) ) {
                for(int x=0;x<10;x++ ) {
                    sub.moveNext();
                    c=sub.CharAt(sub.start);
                    if( c==' ' && (c>128 || isalnum(sub.ch(1)) ) ) {
                        continue;
                    }
                    break;
                }
            } else {
                // cchk=false;
            }
            ch=sub.ch();
        } else {
            ch=sub.incr().moveNext().ch();
        }
    } else if( ISBLANK(ch) ) {
        ch=sub.ch();
    }
    return true;
}

inline StrVar* moveVar(TreeNode* node, XParseVar& sub, XFuncNode* fn, StrVar* rst  ) {
    char ch=sub.ch();
    if( ch=='#' || ch=='$' ) {
        if( sub.ch(1)=='[' ) {
            if( sub.incr().match("[","]") ) {
                StrVar* sv=NULL;
                LPCC vnm=sub.v();
                if( IndexOf(vnm,'.')>0 ) {
                    TreeNode* targetNode= ch=='#'? node: NULL;
                    StrVar* prevVar=NULL;
                    XParseVar pv(sub.GetVar(), sub.prev, sub.cur);
                    for( int n=0; n<10 && pv.valid(); n++ ) {
                        LPCC fnm=pv.findEnd(".").v();
                        if( n==0 ) {
                            sv=targetNode ? targetNode->gv(fnm): fn->gv(fnm);
                        } else if( targetNode ) {
                            sv=targetNode->gv(fnm);
                        } else {
                            qDebug("#0#inline var parse error(var:%s, name:%s)\n", vnm, fnm );
                            return NULL;
                        }
                        if( SVCHK('n',0) ) {
                            targetNode=(TreeNode*)SVO;
                        }
                        prevVar=sv;
                    }
                    sv=prevVar;
                } else {
                    rst->set(ch).add(vnm);
                    LPCC code=rst->get();
                    sv=getNodeVar(node, fn, code, false, ch=='$' ? 0x200: 0 );
                }
                return sv;
            } else {
                rst->set("@error");
                qDebug("#0#inline var match error(src:%s )\n", sub.get() );
                return rst;
            }
        } else {
            sub.incr();
        }
    } else if(  ch=='@' || ch=='-' ) {
        sub.incr();
    }
    moveVarPos(sub);
    if( sub.ch()=='.' ) {
        sub.incr().moveNext();
    }
    return NULL;
}

int nodeFuncCall(TreeNode* node, XFuncNode* fn, XFuncSrc* fsrc, StrVar* rst, XParseVar* param, bool bParam, bool bSubfunc, bool bMember, XListArr* arr) {
    if( fsrc==NULL || fsrc->xfunc==NULL ) {
        return 0;
    }
    int sp=0,ep=0;
    char ch=0;
    StrVar* sv=NULL;
    XFuncNode* fnParent=fn;
    TreeNode* thisNode=NULL;
    sv = fn->gv("@this");
    if( SVCHK('n',0) ) {
        thisNode=(TreeNode*)SVO;
    }
    if( bSubfunc ) {
        if( fsrc->xflag & 0x400 ) {
            bMember=true;
        }
        if( (fsrc->xflag & 0x1800)==0 && arr==NULL  ) {
            arr=getLocalArray(true);
            arr->add()->setVar('9',0);
        }
    }
    if( bMember ) {
        sv=NULL;
        // call
        if( bParam==false ) {
            sv=node->gv("onInit");
        }
        if( SVCHK('f',0) ) {
            fnParent=(XFuncNode*)SVO;
        } else {
            sv=thisNode->gv("onInit");
            if( SVCHK('f',0) ) {
                fnParent=(XFuncNode*)SVO;
            }
        }
    }
    if( arr==NULL && param ) {
        while( param->valid() ) {
            LPCC code=NULL;
            int sp=param->start;
            sv=moveVar(node, *param, fn, rst->reuse() );
            if( sv==NULL ) {
                int num=0;
                bool fchk=false;
                code=param->Trim(sp, param->start);
                if( isnum(code,&num,&fchk) ) {
                    if(fchk ) {
                        rst->setVar('4',0).addDouble(atof(code));
                    } else {
                        rst->setVar('0',0).addInt(atoi(code));
                    }
                    sv=rst;
                } else {
                    sv=getNodeVar(node, fn, code, false, bMember? 0: 0x100 );
                }
            }
            ch=param->ch();
            if( ch==0 || ch==',' ) {
                if( arr==NULL ) {
                    arr=getLocalArray(true);
                }
                arr->add()->add(sv);
            }
            if( ch==',' ) param->incr(); else break;
        }
    }
    XFuncNode* fnCur=bParam ? fn: gfns.getFuncNode(fsrc->xfunc, fnParent);
    XParseVar pv(&(fsrc->xparam));
    int idx=0;
    while( pv.valid() ) {
        LPCC code=NULL;
        ch=pv.ch();
        sp=pv.start;
        if( ch=='&' ) {
            pv.incr();
        }
        ch=pv.moveNext().ch();
        if( ch=='#' ) {
            code=pv.NextWord();
            ch=pv.ch();
        }
        if( ch==0 || ch==',' ) {
            LPCC key=pv.Trim(sp, pv.start);
            LPCC nm=key;
            StrVar* var= arr ? arr->get(idx): NULL;
            if( key[0]=='&' ) nm+=1;
            if( slen(nm) ) {
                if( var ) {
                    if( key[0]=='&' ) {
                        if( var->charAt(0)=='\b' ) {
                            fnCur->GetVar(nm)->reuse()->add(var);
                        } else {
                            sp=0, ep=var->length();
                            fnCur->GetVar(nm)->setVar('s',0, (LPVOID)var).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                        }
                    } else {
                        fnCur->GetVar(nm)->reuse()->add(var);
                    }
                } else {
                    fnCur->GetVar(nm)->setVar('9',0);
                }
            }
            idx++;
        }
        if( pv.ch()==',' ) pv.incr(); else break;
    }
    int ret=0;
    if( bParam && fsrc ) {
        ret=callSubFuncs(fsrc->xfunc,fnCur,rst->reuse());
    } else {
        if( thisNode ) {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
        }
        if( arr ) {
            fnCur->GetVar("@params")->setVar('a',0,(LPVOID)arr);
        }
        fnCur->call(NULL,rst->reuse() );
        funcNodeDelete(fnCur);
    }
    releaseLocalArray(arr);
    return ret;
}
bool funcCall(StrVar* sv, XFuncNode* fn, XListArr* arrs, StrVar* rst, bool bmap) {
    if( sv==NULL )
        return false;
    bool blocal=false;
    XFuncNode* fnCur=NULL;
    rst->reuse();
    if( SVCHK('f',1) ) {
        TreeNode* thisNode=NULL;
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        sv = fn->gv("@this");
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
        }
        if( fsrc->xflag & 0x200 ) {
            nodeFuncCall(thisNode, fn, fsrc, rst, NULL, false, false, false, arrs);
            return true;
        } else {
            fnCur=gfns.getFuncNode(fsrc->xfunc,fn);
            setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
            // sv = fn->gv("sql");
            if( thisNode ) fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            // pushCallFuncName(funcNm);
            fnCur->call(NULL,rst);
            blocal=true;
            // popCallFuncName();
            // gfns.deleteFuncNode(fnCur);
        }

    } else if( SVCHK('f',0) ) {
        fnCur=(XFuncNode*)SVO;
        // pushCallFuncName(funcNm);
        setFuncSrcParam(fnCur,arrs);
        // fnCur->set("@funcName",funcNm);
        fnCur->call(NULL,rst);
        // popCallFuncName();
    }
    if( fnCur ) {
        if( bmap ) {
            TreeNode* tempNode=getTreeNodeTemp();
            for( WBoxNode* bn=fnCur->First(); bn; bn=fnCur->Next() ) {
                LPCC varNm=fnCur->getCurCode();
                if( ccmp(varNm,"@this") ) continue;
                tempNode->GetVar(varNm)->set(bn->value );
            }
            rst->setVar('n',0,(LPVOID)tempNode);
        }
        if( blocal ) {
            funcNodeDelete(fnCur);
        }
        // releaseLocalArray(arrs);
        return true;
    }
    return false;
}
bool callUserFunc(XFuncSrc* fsrc, PARR arrs, XFuncNode* fn, StrVar* rst, int startParam, U32 flag) {
    if( fsrc==NULL ) return false;
    XFunc* fc=fsrc->xfunc;
    XFuncNode* fnCur = gfns.getFuncNode(fc, fn);
    /*
    if( fsrc->xflag&FUNC_INLINE ) {
        fnCur=fn;
    } else {
        fnCur=gfns.getFuncNode(fc, fn);
    }
    */
    if( fnCur ) {
        setFuncSrcParam(fnCur,arrs,fsrc,startParam,fn);
        StrVar* sv=getFuncVar(fn, "@this"); // fn->gv("@this");
        if( SVCHK('n',0) ) {
            fnCur->GetVar("@this")->reuse()->add(sv);
        }
        fnCur->call(fc, rst->reuse());
        funcNodeDelete(fnCur);
        // releaseLocalArray(arrs);
        return true;
    }
    return false;
}

bool callEvalFunc(StrVar* sv, int sp, int ep, XFuncNode* fn, StrVar* rst, TreeNode* thisNode, StrVar* appendVar, bool bset ) {
    XParseVar pv(sv,sp,ep);
    XFuncSrc* src = getFuncSrc();
    if( rst ) rst->reuse();
    src->readBuffer(pv.GetVar(), sp, ep);
    if( src->makeFunc() && src->xfunc ) {
        XFunc* func = src->xfunc;
        XFuncNode* pfn=fn;
        if( func==NULL ) {
            qDebug("#9#callEvalFunc error (func not define)");
            return false;
        }
        if( func->getFuncSize()==1 ) {
            XFunc* fc = func->getFunc(0);
            if( fc==NULL ) {
                return false;
            }
            if( fc->xftype!=FTYPE_SET ) {
                fc->xfflag|=FFLAG_PARAM;
                execFunc(fc, fn, rst, NULL );
                if(appendVar) appendVar->add(toString(rst));
                return true;
            }
        }
        XFuncNode* fnCur=gfns.getFuncNode(func, pfn);
        if( thisNode==NULL ) {
            if( fn ) {
                StrVar* sv=fn->gv("@this");
                if( SVCHK('n',0) ) {
                    thisNode=(TreeNode*)SVO;
                }
            }
        }
        if( thisNode ) {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
        }
        if( appendVar ) {
            int len=appendVar->length();
            fnCur->GetVar("@evalResult")->setVar('s',0,(LPVOID)appendVar).addInt(0).addInt(len).addInt(0).addInt(len);
        }
        fnCur->call(func, rst );
        if( bset && fn ) {
            for( WBoxNode* bn=fnCur->First(); bn; bn=fnCur->Next() ) {
                LPCC varNm=fnCur->getCurCode();
                if( varNm && varNm[0]!='@' ) {
                    sv=fn->GetVar(varNm);
                    if( !SVCHK('n',0) ) {
                        sv->reuse()->add(fnCur->gv(varNm));
                    }
                }
            }
        }
        funcNodeDelete(fnCur);
        return true;
    }
    return false;
}
