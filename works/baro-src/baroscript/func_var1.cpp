#include "func_util.h"
StrVar grst; 

//#> inline
inline TreeNode* getPageNode(TreeNode* node) {
    TreeNode* page=node;
    while( page ) {
        LPCC tag=page->get("tag");
        if( slen(tag) ) {
            if( ccmp(tag,"page") || ccmp(tag,"main") || ccmp(tag,"dialog") ) {
                return page;
            }
        }
        page=page->parent();
    }
    return NULL;
}
int toNum(StrVar* sv, int def) {
    int num=0;
    if( isNumberVar(sv) ) {
        num=toInteger(sv);
    } else {
        num=def;
    }
    return num;
}

int baroFuncEndPos( StrVar*sv, int sp, int ep ) {
    XParseVar sub( sv, sp, ep);
    bool sfc=false;
    if( sub.ch()=='@' ) {
        sfc=true;
        sub.incr();
    }

    char ch=sub.moveNext().ch();
    if( ch=='#' ) ch=sub.incr().moveNext().ch();
    if( ch=='.' ) {
        if( sfc==false ) {
            return -1;
        }
        ch=sub.incr().moveNext().ch();
        if( ch=='#' ) ch=sub.incr().moveNext().ch();
    }
    if( ch=='(' ) {
        if( sub.match("(",")",FIND_SKIP) )  {
            return sub.start;
        }
    }
    return -1;
}
XListArr* getLocalParams(XFuncNode* fn) {
    XListArr* params=NULL;
    StrVar* sv=fn->GetVar("@params");
    if( SVCHK('a',0) ) {
        params=(XListArr*)SVO;
    } else {
        params=new XListArr(true);
        sv->setVar('a',0,(LPVOID)params);
    }
    return params->reuse();
}
int setFuncNodeParams(XFuncNode* fn, XListArr* arrs) {
    StrVar* sv=fn->gv("@params");
    if( sv==NULL ) {
        fn->GetVar("@params")->setVar('a',0,(LPVOID)arrs);
    }
    sv=fn->getParams();
    if( sv && sv->length() ) {
        XParseVar pv(sv);
        int asize=arrs ? arrs->size(): 0;
        int num=0;
        for(int n=0; n<64 && pv.valid(); n++ ) {
            LPCC code=pv.findEnd(",").v();
            bool ref=code[0]=='&'? true: false;
            if( ref ) {
                code++;
            }
            StrVar* var=fn->GetVar(code)->reuse();
            if( n<asize ) {
                sv=arrs->get(n);
                if( sv==NULL ) break;
                if( ref && sv->charAt(0)!='\b' ) {
                    var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(sv->length()).addInt(0).addInt(0);
                } else {
                    var->add(sv);
                }
                num++;
            }
        }
        return num;
    }
    return 0;
}

StrVar* getNodeVar(TreeNode* node, XFuncNode* fn, LPCC key, bool vchk, U32 flag, bool bset ) {
    if( key==NULL || key[0]=='\b' )
        return NULL;
    /*
    if( ccmp(key,"this") ) {
        sv=getFuncVar(fn,"@this");
    } else if( key[0]=='$' ) {
        sv= bset ? fn->GetVar(key+1) : getFuncVar(fn,key+1);
    } else if( key[0]=='#' ) {
        sv= bset ? node->GetVar(key+1) : node->gv(key+1);
    } else {
        bool localCheck=flag&0x100;
        if( key[0]=='&' ) {
            key+=1;
        }
        if( localCheck ) {
            sv=bset ? fn->GetVar(key) : getFuncVar(fn,key);
        } else {
            sv= bset ? node->GetVar(key) : node->gv(key);
        }
    }
    */
    StrVar* sv=fn->gv(key);
    // qDebug("[getNodeVar] key:%s (%s)", key, node->log());
    if( sv==NULL ) {
        sv=node->gv(key);
        if(sv==NULL) {
            fn=fn->parent();
            for(int n=0;n<4;n++) {
                if(fn==NULL) break;
                sv=fn->gv(key);
                if(sv) break;
                fn=fn->parent();
            }
        }
    }
    return sv;
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

//#> func var
StrVar* getFuncVar(XFuncNode* fn, LPCC code) {
    if( fn==NULL )
        return NULL;
    if( slen(code)==0 ) return NULL;
    StrVar* sv=fn->gv(code);
    if( sv==NULL ) {
        sv=fn->gv("@inlineNode");
        if( SVCHK('n',0) ) {
            TreeNode* thisNode=(TreeNode*)SVO;
            sv=thisNode->gv(code);
            if(sv==NULL) {
                sv=thisNode->gv("onInit");
                if(SVCHK('f',0)) {
                    XFuncNode* fnInit=(XFuncNode*)SVO;
                    if(fnInit && fnInit!=fn->parent() ) {
                        sv=fnInit->gv(code);
                    } else {
                        sv=NULL;
                    }
                }
            }
        } else {
            sv=NULL;
        }
    }
    if( sv==NULL ) {
        XFuncNode* fnCur=fn->parent();
        char ch=code[0];
        for(int n=0; fnCur && n<8; n++ ) {
            /*
            if( n>0 && ch=='@' ) {
                if( !ccmp(code,"@page") ) {
                    code+=1;
                }
            }
            */
            sv = fnCur->gv(code);
            if( sv ) return sv;
            fnCur = fnCur->parent();
        }
    }
    return sv;
}

StrVar* getVarValue( TreeNode* tn, LPCC cd, StrVar* rst) {
    if( tn==NULL )
        return NULL;
    StrVar* sv = tn->gv(cd);
    if( sv ) return sv;
    sv=tn->gv("onInit");
    if( SVCHK('f',0) ) {
        XFuncNode* fnCur = (XFuncNode*)SVO;
        return fnCur->gv(cd);
    }
    return NULL;
}

StrVar* getSubVarValue( TreeNode* tn, LPCC fnm) {
    int pos=IndexOf(fnm,'.');
    if( pos>0) {
        StrVar* sv=NULL;
        for( int n=0;n<8;n++ ) {
            LPCC cd=gBuf.add(fnm,pos);
            sv=getVarValue(tn, cd);
            if( SVCHK('n',0) ) {
                tn=(TreeNode*)SVO;
            } else {
                return NULL;
            }
            fnm+=pos+1;
            pos=IndexOf(fnm,'.');
            if( pos<=0 ) break;
        }
    }
    return getVarValue(tn, fnm);
}
XFuncNode* getClassFnInit(XFuncNode* fn) {
    XFuncNode* fnCur=fn;
    StrVar* sv=NULL;
    for(int n=0; fnCur && n<8; n++ ) {
        sv=fnCur->gv("@this");
        if( SVCHK('n',0) ) {
            TreeNode* tn=(TreeNode*)SVO;
            sv=tn->gv("onInit"); // ### version 1.0.4 ("@fnInit");
            if( SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
                return fnCur;
            }
        }
        fnCur=fnCur->parent();
    }
    return fn;
}

bool takeVarAdd(StrVar* rst, XFunc* func, XFuncNode* fn) {
    StrVar* cur = getFuncStrVarValue(func,fn, rst->reuse());
    if( cur && cur!=rst ) rst->add(cur);
    return true;
}
bool takeParseVar(StrVar* rst, XFunc* func, XFuncNode* fn) {
    if( func==NULL )
        return false;
    func->xpv.setPos();
    if( func->xpv.find("$",NULL,FIND_FORWORD)>0 ) {
        makeTextData(func->xpv,fn,rst);
    } else
        rst->add( func->xpv.get(), func->xpv.size() );
    return true;
}

StrVar* getFuncArrayVar(XFuncNode* fn, LPCC left, StrVar* kv, int sp, int ep, StrVar* rst, StrVar* sv, XFunc* param ) {
    bool bLocal=false;
    StrVar* nodeVar=sv;
    TreeNode* node = NULL;
    XParseVar sub(kv, sp, ep );
    if( SVCHK('n',0) && rst!=sv ) {
        node = (TreeNode*)SVO;
    } else if( left ) {
        if( ccmp(left,"$") ) {
            sv=getFuncVar(fn,"@this");
            if( SVCHK('n',0) ) {
                node = (TreeNode*)SVO;
            }
            if( sub.ch()=='>' ) {
                return NULL;
            }
            sub.start=sp;
            if( sub.ch()=='{' ) {
                if(  sub.match("{","}", FIND_SKIP) ) {
                    sv=rst->reuse(); // gum.getStrVar();
                    XFuncSrc* fsrc=nodeInlineFunc( param, 1, sub.GetVar(), 0, 0, sub.prev, sub.cur, rst->reuse() );
                    if( fsrc ) {
                        LPCC code="inline";
                        rst->reuse();
                        nodeFuncCall(node, fn, fsrc, rst, NULL, false, false, false);
                        fn->GetVar(code)->setVar('f',1,(LPVOID)fsrc);
                    } else {
                        qDebug("#0#node inline function call error(function:%s, name:%s)\n", getBaseFuncName(fn), left);
                    }
                } else {
                    qDebug("#0#node inline function match error(function:%s, name:%s)\n", getBaseFuncName(fn), left);
                }
                return rst;
            } else {
                bLocal=true;
            }
        } else if( ccmp(left,"Cf") || ccmp(left,"CF") ) {
            node = getCf();
        } else {
            if( ccmp(left,"this") ) {
                sv=getFuncVar(fn,"@this");
            } else {
                sv= getFuncVar(fn, left);
                // param ? getFuncVar(fn,ccmp(left,"this")?"@this":left): getFuncVar(fn,left); // sv=getFuncVar(fn,ccmp(left,"this")?"@this":left);
            }
            if( SVCHK('n',0) ) {
                node = (TreeNode*)SVO;
            }
        }
    }

    LPCC key = NULL;
    if( funcArrayVarCheck(kv,sp,ep) ) {
        StrVar* gsv = getStrVarTemp();
        makeTextData(sub,fn,gsv,0x40000);
        if( nodeVar && node && funcArrayVarOperCheck(gsv, 0, gsv->length()) ) {
            /*
            StrVar var;
            var.add(gsv);
            sub.SetVar(&var);
            */
            sub.SetVar(gsv);
            if( param->xfuncRef ) {
                param->xfuncRef=0;
            }
            return parseArrayVar(sub, node, fn, rst, bLocal? 0x100:0, param);
        }
        key =Trim(gsv->get());
    } else {
        if( nodeVar && node && funcArrayVarOperCheck(kv, sp, ep) ) {
            return parseArrayVar(sub, node, fn, rst, bLocal? 0x100:0, param);
        }
        key = kv->trimv(sp,ep); //kv->trimv(sp,ep); // gBuf.add(kv->get(sp), ep-sp);
    }

    StrVar* var = NULL;
    if( node ) {
        if( bLocal ) {
            var=getFuncVar(fn, key);
        } else {
            var = getVarValue(node,key);
            if( var==NULL ) {
                sv=node->gv("module");
                if( SVCHK('n',0) ) {
                    TreeNode* module=(TreeNode*)SVO;
                    var=module->gv(key);
                }
            }
        }
    } else if( SVCHK('a',0) ) {
        var = getArrayKeyVar(sv,key,fn,NULL);
        if( nodeVar && var==NULL  ) {
            XListArr* a = (XListArr*)SVO;
            if( isnum(key) ) {
                int num = atoi(key);
                if( num==a->size() ) {
                    var = a->add();
                }
            }
        }
    }
    /* case : a[n]=xxx */
    if( nodeVar==NULL && param ) {
        if( node && var==NULL ) {
            if( bLocal ) {
                var = fn->GetVar(key);
            } else {
                var = node->GetVar(key);
            }
        }
        if( var ) {
            execParamFunc(param,fn,var->reuse());
            sv = var;
            if( SVCHK('s',0) ) {
                sv=getStrVarPointer(sv,&sp,&ep);
                if( sv!=var ) {
                    var->reuse()->add(sv->get(sp), ep-sp);
                }
            }
        }
    }
    return var;
}

StrVar* parseArrayVar(XParseVar& sub, TreeNode* node, XFuncNode* fn, StrVar* rst, U32 flag, XFunc* fc ) {
    StrVar codeVar;
    StrVar* sv=NULL;
    StrVar* svVar=NULL;
    bool bfloat=false, bInt=false, endCheck=false;
    int n=0, aa=0, bb=0;
    float af=0.0, bf=0.0;
    int oper=0, operNext=0, findex=(int)flag&0xFF;
    bool self=false, beq=false;

    if( flag&0x800 ) {
        n=1;
        beq=true;
    }
    char ch=0;
    XFuncSrc* fsrcInit=NULL;
    for(int x=0; x<256; x++) {
        ch=sub.ch();
        if( ch=='/' ) {
            if( sub.ch(1)=='*' ) {
                sub.match("/*","*/",FIND_SKIP);
            } else {
                sub.findEnd("\n");
            }
            continue;
        } else if( ch==0 || ch==';' || ch==',' || endCheck ) {
            if( n && oper ) {
                // qDebug("... af:%f, aa:%d, bb:%d %s (n:%d, oper:%d )", af, aa, bb, bfloat?"float":"int", n, oper  );
                if( bfloat ) {
                    if( oper>10 ) bfloat=false;
                    switch( oper ) {
                    case 1: af++;		break;
                    case 2: af--;		break;
                    case 3: af+=bf;		break;
                    case 4: af-=bf;		break;
                    case 5: af*=bf;		break;
                    case 6: if(bf!=0.0) af/=bf;		break;
                    case 7: {
                        aa=(int)af, bb=(int)bf;
                        if(bb<=0) {
                            aa=0;
                        } else {
                            aa%=bb;
                        }
                        bfloat=false;
                    } break;
                    // case 8: af=bf;	break;
                    // case 9: aa=(af==bf); break;
                    case 10: aa=(af!=bf); break;
                    case 11: aa=(af<bf); break;
                    case 12: aa=(af>bf); break;
                    case 13: aa=(af<=bf); break;
                    case 14: aa=(af>=bf); break;
                    case 16: aa=((int)af&(int)bf); break;
                    case 17: aa=((int)af|(int)bf); break;
                    case 18: aa=(af&&bf); break;
                    case 19: aa=(af||bf); break;
                    default: break;
                    }
                } else {
                    switch( oper ) {
                    case 1: aa++;		break;
                    case 2: aa--;		break;
                    case 3: aa+=bb;		break;
                    case 4: aa-=bb;		break;
                    case 5: aa*=bb;		break;
                    case 6: {
                        if(bb!=0) {
                            bfloat=true;
                            af=(float)(aa/(float)bb);
                            // aa/=bb;
                        } else {
                            aa=0;
                        }
                    } break;
                    case 7: {
                        if(bb<=0) {
                            aa=0;
                        } else {
                            aa%=bb;
                        }
                    } break;
                    // case 8: aa=bb;		break;
                    // case 9: aa=(aa==bb); break;
                    case 10: aa=(aa!=bb); break;
                    case 11: aa=(aa<bb); break;
                    case 12: aa=(aa>bb); break;
                    case 13: aa=(aa<=bb); break;
                    case 14: aa=(aa>=bb); break;
                    case 16: aa=(aa&bb); break;
                    case 17: aa=(aa|bb); break;
                    case 18: aa=(aa&&bb); break;
                    case 19: aa=(aa||bb); break;
                    default: break;
                    }
                }
                if( ch==0 || ch==';' || ch==',' ) {
                    if( operNext ) {
                        operNext=0;
                    }
                }
                if( endCheck && oper && operNext ) {
                    oper=operNext;
                    endCheck=false;
                    continue;
                }

                LPCC code=codeVar.get();
                if( self && slen(code) ) {
                    sv=node->GetVar(code);
                    if( endCheck && svVar ) {
                        sv->reuse()->add(svVar);
                    } else if( bfloat ) {
                        sv->setVar('4',0).addDouble(af);
                    } else {
                        sv->setVar('0',0).addInt(aa);
                    }
                }
                if( endCheck==false && ch ) {
                    ch=sub.incr().ch();
                }
            }


            if( ch==0 ) {
                if( oper==0 ) {
                    if( svVar && rst!=svVar ) {
                        rst->reuse()->add(svVar);
                    }
                } else if( oper==9  ) {
                    if( bfloat ) {
                        rst->setVar('3',af==bf ? 1: 0);
                    } else {
                        rst->setVar('3',aa==bb ? 1: 0);
                    }
                } else if( oper==1 ) {
                    if( bfloat )
                        rst->setVar('4',0).addDouble(af-1);
                    else
                        rst->setVar('0',0).addInt(aa-1);
                } else if( oper==2 ) {
                    if( bfloat )
                        rst->setVar('4',0).addDouble(af+1);
                    else
                        rst->setVar('0',0).addInt(aa+1);
                } else {
                    if( oper>9 ) {
                        if( bfloat )
                            rst->setVar('3', af>0? true: false);
                        else
                            rst->setVar('3', aa>0? true: false);
                    } else {
                        if( bfloat )
                            rst->setVar('4',0).addDouble(af);
                        else
                            rst->setVar('0',0).addInt(aa);
                    }
                }
                break;
            }
            self=bfloat=bInt=endCheck=false;
            codeVar.reuse();
            svVar=NULL;
            n=0, aa=0, bb=0;
            af=0.0, bf=0.0;
            oper=operNext=0;
            if( ch==',' || ch==';' ) {
                sub.incr();
            }
            continue;
        }
        if( ch ) {
            if( ch=='\'' || ch=='\"' ) {
                if( n==0 ) {
                    qDebug("#0#array var error(string %s)\n", sub.get() );
                    return NULL;
                }
                sv=rst->reuse();
                if( sub.MatchStringPos(ch) ) {
                    if( ch=='\'') {
                        sv->add(sub.GetBuffer(sub.prev),sub.cur-sub.prev);
                    } else {
                        XParseVar pv(sub.GetVar(),sub.prev,sub.cur);
                        makeTextData(pv,fn,sv, 0, node);
                    }
                    svVar=sv;
                } else {
                    qDebug("#0#array var parse error(not match string %s)\n", sub.get() );
                    return NULL;
                }
                oper=0;
                endCheck=true;
                continue;
            } else if( ch=='@' ) {
                // ex) @(10/3)+1 == 4
                bInt=true;
                sub.incr();
                continue;
            } else if( ch=='(' ) {
                if( oper>20 ) {
                    return NULL;
                }
                sv=NULL;
                if( sub.match("(",")") ) {
                    // StrVar var;
                    XParseVar pv(sub.GetVar(), sub.prev, sub.cur);
                    // sv=parseArrayVar(pv, node, fn, &var, true);
                    sv=parseArrayVar(pv, node, fn, getStrVarTemp(), flag|0x800);
                } else {
                    qDebug("#0#array var parse error(not match function %s)\n", sub.get() );
                    return NULL;
                }
                bool fchk=false;
                if( sv ) {
                    if( SVCHK('4',0) ) {
                        if( bInt ) {
                            bb=toInteger(sv);
                        } else {
                            bf=(float)toDouble(sv);
                            fchk=true;
                        }
                    } else if( SVCHK('0',0) || SVCHK('1',0) ) {
                        bb=toInteger(sv);
                    } else if( isNumberVar(sv) ) {
                        if( bInt ) {
                            bb=toInteger(sv);
                        } else {
                            if( sv->find(".")>0 ) {
                                fchk=true;
                                bf=toDouble(sv);
                            } else {
                                bb=toInteger(sv);
                            }
                        }
                    }
                } else {
                    bf=0.0;
                    bb=0;
                }
                if( beq || n==0 ) {
                    beq=false;
                    if( fchk ) {
                        af=bf;
                        bfloat=true;
                    } else {
                        aa=bb;
                    }
                } else {
                    if( bInt && bfloat ) {
                        aa=(int)af;
                    } else if( fchk && !bfloat ) {
                        af=(float)aa;
                        bfloat=true;
                    } else if(!fchk && bfloat ) {
                        bf=(float)bb;
                    }
                }
                // qDebug("---- n:%d af:%f,aa:%d (%s, %s)", n, af, aa, bInt?"int":"", bfloat?"float":"");
                bInt=false;
                n++;
            } else {
                bool fchk=false, achk=false;
                int sp=sub.start, ep=0;
                if( beq || n==0 ) {
                    beq=false;
                    achk=true;
                }
                ch=sub.ch();
                if( ch=='@' || ch=='-' || ch=='+' ) {
                    sub.incr();
                }
                ch=sub.moveNext().ch();
                if( ch=='#' ) ch=sub.incr().moveNext().ch();
                if( achk && oper==0 && (ch==','||ch==';' ) ) {
                    continue;
                }
                LPCC vnm=sub.Trim(sp, sub.start);
                bool vchk=false;
                sv=NULL;
                bf=0.0, bb=0;
                if( isnum(vnm) ) {
                    if( ch=='.' ) {
                        sub.incr().moveNext();
                        vnm=sub.Trim(sp,sub.start);
                        if( isnum(vnm) ) {
                            fchk=true;
                            bf=atof(vnm);
                        } else {
                            bf=0.0, bb=0;
                        }
                        ep=sub.start;
                    } else {
                        bb=atoi(vnm);
                    }
                } else if( ch=='.' ) {
                    ch=sub.incr().moveNext().ch();
                    if( ch==0 ) {
                        break;
                    }
                    if( ch=='#' ) {
                        ch=sub.incr().moveNext().ch();
                    }
                    while( ch=='.' ) {
                        ch=sub.incr().moveNext().ch();
                    }
                    if( ch=='(') {
                        if( !sub.match("(",")") ) {
                            qDebug("#0#parseArray eval call error(funcRef:%s)\n", vnm );
                        }
                        ch=sub.ch();
                    }
                    for(int xx=0;xx<8;xx++) {
                        if( ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' ) {
                            ch=sub.incr().moveNext().ch();
                            if( ch=='#' ) ch=sub.incr().moveNext().ch();
                            while( ch=='.' ) {
                                ch=sub.incr().moveNext().ch();
                            }
                            if( ch=='[') {
                                if( !sub.match("(",")",FIND_SKIP) ) {
                                    qDebug("#0#parseArray eval call error(funcRef:%s)\n", vnm );
                                }
                                ch=sub.ch();
                            } else if( ch=='(' ) {
                                if( !sub.match("(",")") ) {
                                    qDebug("#0#parseArray eval call error(funcRef:%s)\n", vnm );
                                }
                                ch=sub.ch();
                            }
                        } else {
                            break;
                        }
                    }
                    sv=rst->reuse();
                    callEvalFunc(sub.GetVar(), sp, sub.start, fn, sv, NULL, NULL );
                    LPCC code=codeVar.get();
                    if(oper==0 ) {
                        vchk=false;
                        svVar=sv;
                    } else if( slen(code) ) {
                        if(oper==8 ) {
                            node->GetVar(code)->reuse()->add(sv);
                        } else {
                            StrVar* var=node->GetVar(code);
                            if( checkFuncObject(var,'4',0)) {
                                af=var->getDouble(4);
                                bfloat=true;
                            } else {
                                aa=toInteger(var);
                            }
                            if( SVCHK('4',0) ) {
                                bf=(float)sv->getDouble(4);
                                if(!bfloat) af=aa;
                                fchk=true;
                            } else if( isNumberVar(sv) ) {
                                if( bfloat ) {
                                    bf=toDouble(sv);
                                    fchk=true;
                                } else {
                                    bb=toInteger(sv);
                                }
                            } else {
                               bf=0.0, bb=0;
                            }
                            if(oper==3) {
                                if(fchk) af+=bf; else aa+=bb;
                            } else if(oper==4) {
                                if(fchk) af-=bf; else aa-=bb;
                            } else if(oper==5) {
                                if(fchk) af*=bf; else aa*=bb;
                            } else if(oper==6) {
                                if(fchk) {
                                    if(bf!=0) af/=bf;
                                } else {
                                    if(bb!=0) aa/=bb;
                                }
                            } else if(oper==7) {
                                if(fchk) {
                                    if( bfloat) aa=af; else bb=bf;
                                    fchk=false;
                                }
                                aa%=bb;
                            }
                            if(fchk) {
                                node->GetVar(code)->setVar('4',0).addDouble(af);
                            } else {
                                qDebug("parseArrayVar set func %s.%s (oper:%d) result:%s, [%d,%f] %s\n",
                                       code, vnm, oper, toString(sv), aa, af, fchk?"float":"int");
                                node->GetVar(code)->setVar('0',0).addInt(aa);
                            }
                        }
                    }
                    oper=0;
                    svVar=NULL;
                    endCheck=true;
                    continue;
                } else if( ch=='(' ) {
                    int psp=0, pep=0;
                    if( sub.match("(", ")",FIND_SKIP) ) {
                        psp=sub.prev, pep=sub.cur;
                    } else {
                        qDebug("#0#node inline function match error(function:%s, name:%s)\n", getBaseFuncName(fn), vnm);
                        return NULL;
                    }
                    sv=rst->reuse();
                    if( sub.ch()=='{'  ) {
                        LPCC code=codeVar.get();
                        if( sub.match("{","}",FIND_SKIP) ) {
                            ep=sub.start;
                            if( ccmp(vnm,"class") ) {
                                if( !setModifyClassFunc(node, sub.GetVar(), sp, ep, fn, sv, false, "class") ) {
                                    qDebug("#0#node class function make error(name:%s)\n", vnm);
                                }
                            } else if( ccmp(vnm,"inline") || ccmp(vnm,"func") || ccmp(vnm,"private") || ccmp(vnm,"event") ) {
                                if( slen(code) ) {
                                    StrVar* src=grst.reuse();
                                    if(ccmp(vnm,"private")) vnm="inline";
                                    src->add(code).add("(").add(sub.GetVar(), psp, pep).add(") {").add(sub.GetVar(), sub.prev, sub.cur).add("}");
                                    if( setModifyClassFunc(node, src, 0, src->length(), fn, sv, false, vnm) ) {
                                        qDebug("#0#node function modify(name:%s, type:%s)\n", code, vnm);
                                    } else {
                                        qDebug("#9#node function make error(function:%s, name:%s)\n", getBaseFuncName(fn), code);
                                    }
                                } else {
                                    qDebug("#9#node inline function define error(function:%s, name:%s)\n", getBaseFuncName(fn), code);
                                }
                            } else if( ccmp(vnm,"call") || ccmp(vnm,"init") ) {
                                XFuncSrc* fsrc=nodeInlineFunc( fc, findex++, sub.GetVar(), psp, pep, sub.prev, sub.cur, rst );
                                if( fsrc && fsrc->xfunc ) {
                                    XFunc* fcCur=fsrc->xfunc;
                                    fcCur->xflag|=FFLAG_EQ;
                                    if( ccmp(vnm,"init") ) {
                                        fsrcInit=fsrc;

                                    } else if( fsrc && fsrc->xfunc ) {
                                        XFuncNode* fnCur= gfns.getFuncNode(fcCur, fn);
                                        sv=fn->gv("@this");
                                        if( SVCHK('n',0) ) {
                                            TreeNode* thisNode=(TreeNode*)SVO;
                                            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                                        }
                                        fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)node);
                                        fnCur->call();
                                        funcNodeDelete(fnCur);
                                    }
                                }
                            } else {
                                if(bInt) {
                                    sp-=1;
                                    bInt=false;
                                }
                                if( !setModifyClassFunc(node, sub.GetVar(), sp, ep, fn, sv, false, "func") ) {
                                    qDebug("#0#node inline function make error(function:%s, name:%s)\n", vnm, code);
                                }
                            }
                            svVar=NULL;
                            oper=0;
                            endCheck=true;
                            continue;
                        } else {
                            qDebug("#0#node inline function match error(function:%s, name:%s)\n", getBaseFuncName(fn), vnm);
                            return NULL;
                        }
                    } else if(ccmp(vnm,"not") || ccmp(vnm,"if") ) {
                        XParseVar pv(sub.GetVar(), psp, pep);
                        sv=parseArrayVar(pv, node, fn, getStrVarTemp(), flag|0x800);
                        if(isVarTrue(sv)) {
                            sv->setVar('0',0).addInt(ccmp(vnm,"not")?0:1);
                        } else {
                            sv->setVar('0',0).addInt(ccmp(vnm,"not")?1:0);
                        }
                        svVar=sv;
                        vchk=false;
                        // qDebug("parse not check: %s\n", toString(sv));
                    } else {
                        XFuncSrc* fsrc=NULL;
                        bool member=false;
                        sv=node->gv(vnm);
                        if( SVCHK('f',1) ) {
                            member=true;
                            fsrc=(XFuncSrc*)SVO;
                        } else {
                            sv=fn->gv(vnm);
                            if( SVCHK('f',1) ) {
                                fsrc=(XFuncSrc*)SVO;
                            } else {
                                sv=getStrVar("fsrc", vnm);
                                if( SVCHK('f',1) ) {
                                    fsrc=(XFuncSrc*)SVO;
                                }
                            }
                        }
                        sv=rst->reuse();
                        if( fsrc ) {
                            XParseVar pv(sub.GetVar(), psp, pep);
                            if( fsrc->xflag & FUNC_INLINE ) {
                                nodeFuncCall(node, fn, fsrc, sv, &pv, false, false, false);
                            } else {
                                nodeFuncCall(node, fn, fsrc, sv, &pv, false, false, member);
                            }
                        } else {
                            callEvalFunc(sub.GetVar(), sp, sub.start, fn, sv, NULL, NULL );
                        }
                        if( !isNumberVar(sv) ) {
                            LPCC code=codeVar.get();
                            if( slen(code) ) {
                                node->GetVar(code)->reuse()->add(sv);
                            }
                            ch=sub.ch();
                            if( ch==';' || ch==',' ) sub.incr();
                            // qDebug("xxx call --- func (%d) xxx code:%s result:%s c:%c", n, code, toString(sv), ch );
                            svVar=NULL;
                            oper=0;
                            endCheck=true;
                            continue;
                        }
                    }
                } else {
                    vchk=true;
                    ep=sub.start;
                }
                if( vchk ) {
                    if( n==0 ) {
                        LPCC code=NULL;
                        achk=true;
                        if( sv==NULL && sp<ep ) {
                            codeVar.set(sub.Trim(sp,ep) );
                            code=codeVar.get();
                            sv=getNodeVar(node, fn, code, false, flag);
                            if(svVar==NULL) svVar=sv;
                            ch = sub.ch();
                            if( ch==0 ) continue;
                        }
                        n++;
                    } else if( sv==NULL && sp<ep ) {
                        LPCC key = sub.Trim(sp,ep);
                        LPCC code=codeVar.get();
                        if( isnum(key, &bb, &fchk) ) {
                            if( fchk ) {
                                bf=atof(key);
                            } else {
                                bb=atoi(key);
                            }
                        } else if( ccmp(key,"null") || ccmp(key,"true") || ccmp(key,"false") ) {
                            sv=slen(code) ? node->GetVar(code): codeVar.reuse();
                            if( ccmp(key,"null") ) {
                                // sv->setVar('9',0);
                                bb=0;
                            } else {
                                // sv=node->GetVar(code);
                                // sv->setVar('3',ccmp(key,"true")?1:0 );
                                bb=SVCHK('3',1)? 1: 0;
                            }
                            if( bfloat ) {
                                aa=(int)af;
                            }
                        } else if( slen(key) ) {
                            sv=getNodeVar(node, fn, key, false, flag);
                        }
                    }
                }

                if( sv ) {
                    if( SVCHK('4',0) ) {
                        bf=(float)sv->getDouble(4);
                        fchk=true;
                    } else if( SVCHK('0',0) || SVCHK('1',0) || isNumberVar(sv) ) {
                        if( fchk ) {
                            bf=toDouble(sv);
                        } else {
                            bb=toInteger(sv);
                        }
                    } else {
                       bf=0.0, bb=0;
                    }
                }
                if( achk ) {
                    if( fchk ) {
                        af=bf;
                        bfloat=true;
                    } else {
                        aa=bb;
                    }
                } else if( fchk!=bfloat ) {
                    if( fchk ) {
                        af=(float)aa;
                        bfloat=true;
                    } else {
                        bf=(float)bb;
                    }
                }
                n++;
            } // else

        } // if(ch)

        ch = sub.ch();
        if( ch==0 || ch==';' || ch==',' ) {
            if( oper==0 ) {
                svVar=sv;
            }
            continue;
        } else if( ch==':' || (ch=='=' && sub.ch(1)!='=') ) {
            aa=0, bb=0;
            af=0.0, bf=0.0;
            self=true;
            oper = 8;
            beq=true;
            bfloat=false;
            svVar= NULL;
            sub.incr();
            continue;
        }
        if( ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' || ch=='=' ) {
            char c = sub.incr().ch();
            if( c=='+' || c=='-' || c=='=' ) {
                self=true;
                if( c=='+' && ch=='+' ) oper=1;
                else if( c=='-' && ch=='-' ) oper=2;
                else if( ch=='+' && c=='=' )
                    oper=3;
                else if( ch=='-' && c=='=' )
                    oper=4;
                else if( ch=='*' && c=='=' )
                    oper=5;
                else if( ch=='/' && c=='=' )
                    oper=6;
                else if( ch=='%' && c=='=' )
                    oper=7;
                else if( ch=='=' && c=='=' ) {
                    oper=9;
                    self=false;
                }
                sub.incr();
            } else {
                if( oper ) {
                    endCheck=true;
                    if( ch=='+' ) operNext = 3;
                    else if( ch=='-' ) operNext = 4;
                    else if( ch=='*' ) operNext = 5;
                    else if( ch=='/' ) operNext = 6;
                    else if( ch=='%' ) operNext = 7;
                } else {
                    if( ch=='+' ) oper = 3;
                    else if( ch=='-' ) oper = 4;
                    else if( ch=='*' ) oper = 5;
                    else if( ch=='/' ) oper = 6;
                    else if( ch=='%' ) oper = 7;
                }
            }
        } else if( ch=='<' || ch=='>' ) {
            char c = sub.incr().ch();
            if( c=='=' ) {
                if( ch=='<' ) {
                    oper=13;
                } else if( ch=='>' ) {
                    oper=14;
                }
                sub.incr();
            } else if( ch=='<' ) {
                oper=11;
            } else if( ch=='>' ) {
                oper=12;
            }
        } else if( ch=='&' || ch=='|' ) {
            char c = sub.incr().ch();
            if( c=='&' || c=='|' ) {
                if( ch=='&' ) {
                    oper=18;
                } else if( ch=='|' ) {
                    oper=19;
                }
                sub.incr();
            } else  if( ch=='&' ) {
                oper=16;
            } else if( ch=='|' ) {
                oper=17;
            }
        } else if( ch=='!' ) {
            char c = sub.incr().ch();
            if( c=='=' ) {
                oper=10;
                sub.incr();
            }
        } else {
            if( n && oper ) {
                endCheck=true;
            } else {
                qDebug("node inline var line end check (char=%c)\n", ch);
                break;
            }
        }
    }
    if( fsrcInit && fsrcInit->xfunc ) {
        XFuncNode* fnCur= gfns.getFuncNode(fsrcInit->xfunc, fn);
        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
        fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)node);
        fnCur->call();
        funcNodeDelete(fnCur);
    }
    return rst;
}

bool makeTextData(XParseVar& pv, XFuncNode* fn, StrVar* rst, U32 flag, TreeNode* tn ) {
    // flag & 0x10000
    // flag & 0x20000
    // flag & 0x40000
    char ch = 0;
    int sp=0,ep=0; //,end=0;

    StrVar* sv = NULL;
    while( pv.valid() ) {
        pv.findEnd("$",FIND_FORWORD);
        if( pv.cur>pv.prev ) rst->add(pv.GetVar(),pv.prev,pv.cur);
        ch = pv.ch();
        if( ch==0 )
            break;
        if( ch=='[' ) {
            rst->add(ch);
            continue;
        }
        bool cchk=false;
        if( ch=='$') {
            sp = pv.start;
            pv.incr();
            ch=pv.ch();
            if( ch=='{' ) {
                if( pv.match("{","}",FIND_SKIP) ) {
                    LPCC vnm=pv.GetChars(pv.prev, pv.cur);
                    rst->add("${").add(vnm).add("}");
                } else {
                    pv.incr();
                }
            } else {
                if( (flag & 0x20000)==0 ) {
                    LPCC val=pv.NextWord();
                    rst->add("$").add(val);
                } else {
                    rst->add("$$");
                }
            }
            continue;
        }
        if( (flag & 0x20000) && (ch!='{') ) {
            rst->add('$');
            continue;
        }

        if( ch=='{' ) {
            sp = pv.start;
            pv.incr();
            ch=pv.ch();
            if( ch=='+' || ch=='#' ) {
                pv.start=sp;
                if( pv.match("{","}",FIND_SKIP) ) {
                    callEvalFunc(pv.GetVar(), pv.prev+1, pv.cur, fn, getStrVarTemp(), NULL, rst, ch=='#');
                } else {
                    pv.incr();
                }
                continue;
            } else if( ch=='=' ) {
                //#baropage
                pv.start=sp;
                if( pv.match("{","}",FIND_SKIP) ) {
                    XParseVar sub(pv.GetVar(), pv.prev+1, pv.cur);
                    if( tn ) {
                        StrVar* val=getStrVarTemp();
                        parseArrayVar(sub, tn,fn,val);
                        rst->add(toString(val) );
                    } else {
                        execFuncStr(fn, pv.GetVar(), pv.prev+1, pv.cur, rst, "@page" );
                    }
                } else {
                    pv.incr();
                }
                continue;
            } else {
                int pos=baroFuncEndPos(pv.GetVar(),pv.start, pv.wep );
                if( pos!=-1 ) {
                    execFuncStr(fn, pv.GetVar(), pv.start, pos, rst, "@page" );
                    pv.start=pos;
                    ch=pv.ch();
                    if( ch==','|| ch==';' ) {
                        ch=pv.incr().ch();
                    }
                    if( ch=='}' ) {
                        pv.incr();
                    }
                    continue;
                }
            }
            cchk=true;
        }
        sp = pv.start;
        pv.moveNext();
        ep = pv.start;
        ch = pv.CharAt(ep);
        if( cchk && ch=='#' ) {
            pv.incr().moveNext();
            ep = pv.start;
            ch = pv.CharAt(ep);
        }
        bool fchk = false;
        if( (flag & 0x40000)==0 && (ch=='(' || ch=='.' || ch=='[') ) {
            if( ch=='.' ) {
                bool vchk=false; // (ex)$aaa.$bbb
                int n=0;
                pv.incr();
                for( ; n<8; n++) {
                    ch=pv.CharAt(pv.start);
                    if( ch=='@' ) {
                        pv.incr();
                    } else if( ch=='$' ) {
                        if( n==0 ) {
                            vchk=true;
                        }
                        break;
                    }
                    pv.moveNext();
                    ch=pv.CharAt(pv.start);
                    if( ch!='.' ) break;
                    pv.incr();
                }
                if( vchk ) {
                    LPCC var = pv.GetChars(sp,ep);
                    sv = tn ? tn->gv(var): NULL;
                    if( sv==NULL ) sv=ccmp(var,"this") ? fn->gv("@this"):getFuncVar(fn, var);
                    if( sv ) {
                        toString( sv, rst );
                    }
                    pv.start=ep;
                    continue;
                }
                if( ch=='(') {
                    fchk=true;
                } else {
                    LPCC var = pv.Trim(sp,ep);
                    sv = tn ? tn->gv(var): NULL;
                    if( sv==NULL ) sv=ccmp(var,"this") ? fn->gv("@this"):getFuncVar(fn, var);
                    TreeNode* sub=NULL;
                    if( SVCHK('n',0) ) {
                        sub=(TreeNode*)SVO;
                    }
                    pv.start=ep+1;
                    while( sub ) {
                        ch=pv.CharAt(pv.start);
                        if( ch=='@' ) {
                            pv.incr();
                            var=gBuf.fmt("@%s",pv.NextWord() );
                        } else {
                            var=pv.NextWord();
                        }
                        sv=sub->gv(var);
                        ch=pv.CharAt(pv.start);
                        if( ch=='.' ) {
                            pv.incr();
                            ch=pv.CharAt(pv.start);
                            if( ch=='$' ) {
                                vchk=true;
                                break;
                            }
                        } else {
                            break;
                        }

                        if( SVCHK('n',0) ) {
                            sub=(TreeNode*)SVO;
                        } else {
                            break;
                        }
                    }
                    if( rst!=sv ) {
                        rst->add( toString(sv) );
                    }
                    if( vchk ) {
                        pv.start-=1;
                    }
                }
            } else if( ch=='[') {
                if( pv.match("[","]",FIND_SKIP) ) {
                    LPCC left = pv.GetChars(sp,ep);
                    sv=getFuncVar(fn, ccmp(left,"this")? "@this": left);
                    if( SVCHK('n',0) ) {
                        TreeNode* node=(TreeNode*)SVO;
                        sv=node->gv(pv.v());
                    } else if( SVCHK('a',0) ) {
                        XListArr* a=(XListArr*)SVO;
                        left=pv.v();
                        if( isnum(left) ) {
                            sv=a->get(atoi(left));
                        }
                    } else {
                        sv=NULL;
                    }
                    if( sv && rst!=sv ) rst->add(toString(sv));
                } else {
                    return false;
                }
            } else {
                fchk=true;
            }
            if( fchk ) {
                if( cchk ) {
                    int pos=pv.findPos("}");
                    if( pos==-1 ) {
                        return false;
                    }
                    ep=pos;
                    pv.start=ep;
                } else if( pv.MatchWordPos("(",")") ) {
                    ch=pv.CharAt(pv.start);
                    while( ch=='.' ) {
                        pv.incr().moveNext();
                        ch=pv.CharAt(pv.start);
                        if( ch=='(' && pv.MatchWordPos("(",")") ) {
                            ch=pv.CharAt(pv.start);
                        } else return false;
                    }
                    ep=pv.start;
                } else {
                    return false;
                }
                sv=grst.reuse();
                callEvalFunc(pv.GetVar(), sp, ep, fn, sv, NULL, NULL);
                if( SVCHK('i',3) ) {
                    int len=sv->length();
                    if( len==16 ) {
                        rst->format(64,"#%02X%02X%02X", sv->getInt(4), sv->getInt(8), sv->getInt(12) );
                    } else if( len==20 ) {
                        rst->format(64,"#%02X%02X%02X%02X", sv->getInt(4), sv->getInt(8), sv->getInt(12), sv->getInt(16));
                    }
                } else {
                    rst->add(toString(sv) );
                }
            }
        } else {
            if( cchk ) {
                int pos=pv.findPos("}");
                if( pos==-1 ) {
                    return false;
                }
                ep=pos;
                pv.start=ep;
            }
            LPCC var = pv.Trim(sp,ep);
            sv = tn ? tn->gv(var): NULL;
            if( sv==NULL ) {
                sv=ccmp(var,"this") ? fn->gv("@this"):getFuncVar(fn, var);
            }
            if( SVCHK('i',3) ) {
                int len=sv->length();
                if( len==16 ) {
                    rst->format(64,"#%02X%02X%02X", sv->getInt(4), sv->getInt(8), sv->getInt(12) );
                } else if( len==20 ) {
                    rst->format(64,"#%02X%02X%02X%02X", sv->getInt(4), sv->getInt(8), sv->getInt(12), sv->getInt(16));
                }
            } else {
                rst->add(toString(sv) );
            }
        }
        if( cchk ) {
            if( pv.ch()=='}' ) pv.incr();
        }
    }
    return true;
}
StrVar* getMeVar(XFuncNode* fn ) {
    if( fn==NULL ) return NULL;
    StrVar* sv=fn->gv("@page");
    if( sv==NULL ) {
        XFuncNode* fnCur=fn->parent();
        for(int n=0; fnCur && n<4; n++ ) {
            sv=fnCur->gv("@page");
            if( sv ) break;
            fnCur=fnCur->parent();
        }
    }
    return sv;
}

StrVar* getFuncStrVarValue(XFunc* func, XFuncNode* fn, StrVar* rst) {
    // version 1.0.3 getPageId
    LPCC code=NULL;
    LPCC funcName=NULL;
    StrVar* sv = NULL;
    if( func->xdotPos>0 ) {
        code=func->getVarName();
        if( slen(code)==0 ) return NULL;
        funcName=func->getFuncName();
        if( ccmp(code,"@page") ) {
            sv=getMeVar(fn);
        } else if( isUpperCase(code[0]) ) {
            if( ccmpl(code,"Cf") || ccmp(code,"CF") ) {
                return getCf()->GetVar(funcName);
            }
        } else {
            sv=getFuncVar(fn, code);
        }
    } else {
        code=func->getValue();
        sv=getFuncVar(fn, code);
        if( sv ) {
            return sv;
        }
    }


    /*(na) a=a.tag; */
    // ### version 1.0.4
    if( funcName && sv ) {
        if( SVCHK('n',0) ) {
            sv=getSubVarValue((TreeNode*)SVO, funcName);
        } else {
            if( SVCHK('x',21) ) {
                int pos=sv->find("\f", FUNC_HEADER_POS);
                if( pos>0 ) {
                    if( ccmp(funcName,"key") ) {
                        rst->set(sv->get(FUNC_HEADER_POS,pos));
                    } else {
                        int len=sv->length();
                        if( len>pos ) {
                            rst->add( sv, pos+1, len );
                        }
                    }
                }
            } else if( SVCHK('i',20) ) {
                double num=0;
                if( ccmp(funcName,"x") ) {
                    num=sv->getDouble(4);
                } else if( ccmp(funcName,"y") ) {
                    num=sv->getInt(4+sizeof(double));
                }
                rst->setVar('4',0).addDouble(num);
            } else if( SVCHK('i',5) ) {
                int sz=sizeof(double);
                double num=0;
                if( ccmp(funcName,"x") ) {
                    num=sv->getDouble(4);
                } else if( ccmp(funcName,"y") ) {
                    num=sv->getDouble(4+sz);
                } else if( ccmp(funcName,"width") ) {
                    num=sv->getDouble(4+(2*sz));
                } else if( ccmp(funcName,"height") ) {
                    num=sv->getDouble(4+(3*sz));
                }
                rst->setVar('4',0).addDouble(num);
            } else {
                qDebug("%s var not define (%s)\n", code, funcName );
            }
            return NULL;
        }
    }    
    return sv;
}

void getOperValue(XFuncNode* fn, XFunc* param, int size, StrVar* rst) {
    if( param==NULL ) return;
    if( size==1 ) {
        if( param->getType()==FTYPE_NUMBER ) return;
        LPCC nodeVar=NULL;
        LPCC code=param->getValue();
        if( code[0]=='$' ) {
            StrVar* sv=NULL;
            code++;
            if( ccmp(code,"this")) {
                sv=fn->gv("@this");
            } else {
                sv=getFuncVar(fn, code);
            }
            if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                fn->GetVar("@selfNode")->setVar('n',0,(LPVOID)node);
            } else {
                qDebug("#0#self node set error(code:%s)\n", code);
            }
            return;
        }
        int pos=IndexOf(code,'.');
        if( pos>0 ) {
            nodeVar=code+pos+1;
            code=gBuf.add(code,pos);
        }
        StrVar* sv=getFuncVar(fn, code);
        if( nodeVar && SVCHK('n',0) ) {
            sv=getSubVarValue((TreeNode*)SVO, nodeVar);
        }
        XFunc* fc = param->getParam(0);
        if( fc && fc->getType()==FTYPE_OPER ) {
            LPCC op = fc->getValue();
            if( isNumberVar(sv) ) {
                int num = toInteger(sv);
                if( ccmp(op,"++") )
                    sv->setVar('0',0).addInt(num+1);
                else if( ccmp(op,"--") )
                    sv->setVar('0',0).addInt(num-1);
                else
                    qDebug("#9#[%s] getOperValue not support opration(code:%s, oper:%s)\n", getBaseFuncName(fn), code, op);
                rst->setVar('0',0).addInt(num);
            } else {
                qDebug("#0#[%s] getOperValue not number value (code:%s, oper:%s)\n",getBaseFuncName(fn), code, op);
                rst->reuse()->add(sv);
            }
        }
        return;
    }
    bool bset=false;
    for( int n=0;n<size;n+=2 ) {
        XFunc* oper = param->getParam(n);
        if(oper==NULL ) return;
        if( oper->getType()!=FTYPE_OPER ) {
            break;
        }
        LPCC op = oper->getValue();
        char ch = op? op[0]: 0;
        if( ch=='&' && op[1]=='&' && !isVarTrue(rst) ) {
            rst->setVar('3',0);
            break;
        } else if( ch=='|' && op[1]=='|' && isVarTrue(rst) ) {
            rst->setVar('3',1);
            break;
        }

        XFunc* fcCur=param->getParam(n+1);
        if( fcCur==NULL ) break;

        if( ch=='=' && slen(op)==1 ) {
            bset=true;
            execParamFunc( fcCur,fn,rst);
            continue;
        }
        StrVar* var=&grst;
        if( var==rst) {
            // qDebug("--------[%s] multi operation %s\n", getBaseFuncName(fn), toString(rst));
            var=getStrVarTemp();
        } else {
            var->reuse();
        }
        execParamFunc( fcCur,fn,var);
        if( ch=='=' || ch=='!' || ch=='<' || ch=='>') {
            // func->xflag = n<<24; break;
            if( compareResult(rst, var, op) ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        } else if( ch=='&' || ch=='|' ) {
            char c = op[1];
            if( c=='&' || c=='|') {
                // qDebug("xxxxxx %s %s %s xxxxx\n",toString(rst), op, toString(var));
                if( compareResult(rst, var, op) ) {
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            } else {
                int a = toInteger(rst), b=toInteger(var);
                if( ch=='&' ) {
                    rst->setVar('0',0).addInt(a&b);
                } else if( ch=='|' ) {
                    rst->setVar('0',0).addInt(a|b);
                }
            }
        } else {
            char type=rst->charAt(1);
            if( (rst->charAt(0)=='\b' && type=='4') ||
                (var->charAt(0)=='\b' && var->charAt(1)=='4') ) {
                double a = toDouble(rst), b=toDouble(var);
                switch(ch) {
                case '+': { rst->setVar('4',0).addDouble(a+b); } break;
                case '-': { rst->setVar('4',0).addDouble(a-b); } break;
                case '*': { rst->setVar('4',0).addDouble(a*b); } break;
                case '/': { rst->setVar('4',0).addDouble(b!=0.0? a/b:0); } break;
                case '%': { rst->setVar('4',0).addDouble(fmod(a,b)); } break;
                }
            } else if( (rst->charAt(0)=='\b' && type=='1') || (var->charAt(0)=='\b' && var->charAt(1)=='1') ) {
                UL64 a=toUL64(rst), b=toUL64(var);
                switch(ch) {
                case '+': { rst->setVar('1',0).addUL64(a+b); } break;
                case '-': { rst->setVar('1',0).addUL64(a-b); } break;
                case '*': { rst->setVar('1',0).addUL64(a*b); } break;
                case '/': { rst->setVar('1',0).addUL64(b!=0? a/b:0); } break;
                case '%': { rst->setVar('1',0).addUL64(a%b); } break;
                }
            } else {
                int a = toInteger(rst), b=toInteger(var);
                switch(ch) {
                case '+': { rst->setVar('0',0).addInt(a+b); } break;
                case '-': { rst->setVar('0',0).addInt(a-b); } break;
                case '*': { rst->setVar('0',0).addInt(a*b); } break;
                case '/': { rst->setVar('0',0).addInt(b!=0? a/b:0); } break;
                case '%': { rst->setVar('0',0).addInt(b!=0? a%b:0); } break;
                }
            }
        }        
        if( param->xfflag==0x1022 && (fcCur->xfflag&0x10) ) {
            break;
        }       
    }
    if( bset ) {
        LPCC vnm=param->getValue();
        fn->GetVar(vnm)->reuse()->add(rst);
    }
}

bool setResultVal(XParseVar* pv, XFuncNode* fn, StrVar* result, TreeNode* target) {
    StrVar* sv=NULL;
    // TreeNode* thisNode=NULL;
    char ch=pv->ch();
    int sp=pv->start;
    int mp=sp, ep=0;
    bool ref=false;

    if( ch=='&' ) {
        ref=true;
        pv->incr().ch();
        sp=pv->start;
    } else if( ch=='{') {
        if( pv->match("{","}",FIND_SKIP) ) {
            XParseVar pvCur(pv->GetVar(), pv->prev, pv->cur);
            result->setVar('n',0,(LPVOID)setJsonVal(&pvCur, NULL, fn) );
        } else {
            return false;
        }
        return true;
    } else if( ch=='[') {
        if( pv->match("[","]",FIND_SKIP) ) {
            XParseVar pvCur(pv->GetVar(), pv->prev, pv->cur);
            result->setVar('a',0,(LPVOID)setArrayVal(&pvCur, NULL, fn) );
        } else {
            return false;
        }
        return true;
    } else if( ch=='\'' || ch=='\"' ) {
        if( pv->MatchStringPos(ch) ) {
            if( ch=='\"' ) {
                XParseVar pvCur(pv->GetVar(), pv->prev, pv->cur);
                makeTextData(pvCur,fn,result, 0, NULL);
            } else {
                result->add(pv->GetVar(), pv->prev, pv->cur);
            }
        }
        return true;
    } else if( ch=='+' || ch=='-' ) {
        pv->incr();
    }
    ch=pv->moveNext().ch();
    if( ch=='#' || ch=='@' ) {
        ch=pv->moveNext().ch();
    }
    if( ch=='.' ) {
        mp=pv->start;
        pv->incr().moveNext();
        ep=pv->start;
        ch=pv->ch();
        while( ch=='.' ) {
            ch=pv->incr().moveNext().ch();
        }
        if( ch!='(' ) {
            pv->start=ep;
        }
    }
    if( ch=='(' ) {
        ep=pv->start;
        if( !pv->match("(",")") ) {
            return false;
        }
        for( int x=0;x<8;x++ ) {
            ch=pv->ch();
            if( ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' ) {
                pv->incr();
                ch=pv->moveNext().ch();
                while(ch=='.' ) ch=pv->incr().moveNext().ch();
                if( ch=='(' ) {
                    if( !pv->match("(",")") ) return false;
                }
            } else {
                break;
            }
        }
        XFuncSrc* fsrc=NULL;
        if( ref ) {
            if( mp>sp ) {
                LPCC nodeName=pv->Trim(sp,mp);
                sv=ccmp(nodeName,"this") ? fn->gv("@this"): getFuncVar(fn, nodeName);
                if( SVCHK('n',0) ) {
                    LPCC funcName=pv->Trim(mp+1,ep);
                }
            } else {
                LPCC funcName=pv->Trim(sp,ep);
                sv=fn->gv(funcName);
                if( SVCHK('f',1) ) {
                    fsrc=(XFuncSrc*)SVO;
                } else {
                    sv=getStrVar("fsrc", funcName);
                    if( SVCHK('f',1) ) {
                        fsrc=(XFuncSrc*)SVO;
                    }
                }
            }
            if( fsrc ) {
                result->setVar('f',1,(LPVOID)fsrc);
            }
        } else {
            fsrc=getFuncSrc();
            ep=pv->start;
            fsrc->parseData(pv->GetVar(), sp, ep);
            fsrc->makeFunc();
            XFunc* func = fsrc->xfunc;
            if( func && func->getFuncSize()==1 ) {
                XFunc* fc = func->getFunc(0);
                fc->xfflag|=FFLAG_PARAM;
                execParamFunc(fc, fn, result);
            }
        }
        return true;
    }

    int sa=0, ea=0, x=0;
    if( ch=='[' ) {
        ep=pv->start;
        if( pv->match("[","]") ) {
            sa=pv->prev, ea=pv->cur;
        } else {
            return false;
        }
    }
    for( ;x<8;x++ ) {
        ch=pv->ch();
        if( ch=='+' || ch=='-' || ch=='*' || ch=='/' || ch=='%' ) {
            pv->incr();
            ch=pv->moveNext().ch();
            while(ch=='.' ) ch=pv->incr().moveNext().ch();
            if( ch=='(' ) {
                if( !pv->match("(",")") ) return false;
            }
        } else {
            break;
        }
    }
    if( x>0 ) {
        StrVar gsrc;
        XFuncSrc* fsrc=getFuncSrc();
        ep=pv->start;
        gsrc.set("return ").add(pv->GetVar(), sp, ep);

        fsrc->parseData(&gsrc, 0, gsrc.length() );
        fsrc->makeFunc();
        XFunc* func = fsrc->xfunc;
        if( func && func->getFuncSize()==1 ) {
            XFunc* fc = func->getFunc(0);
            execParamFunc(fc, fn, result);
        }
        return true;
    }

    if( sa<ea ) {
        LPCC left=pv->Trim(sp,ep);
        sv=getFuncArrayVar(fn, left, pv->GetVar(), sa, ea, result, result );
        if( sv!=result ) {
            result->reuse()->add(sv);
        }
    } else if( sp<mp ) {
        TreeNode* sub=NULL;
        LPCC varName=pv->Trim(sp,mp);
        if( ccmp(varName,"Cf") || ccmp(varName,"CF") ) {
            sub=getCf();
        } else {
            /*
            if( thisNode && ccmp(varName,"thisNode") ) {
                sub=thisNode;
            } else {
            }
            */
            sv=ccmp(varName,"this") ? fn->gv("@this"): getFuncVar(fn,varName);
            if( SVCHK('n',0) ) {
                sub=(TreeNode*)SVO;
            } else if( SVCHK('x',21) ) {
                int pos=sv->find("\f", FUNC_HEADER_POS);
                if( pos>0 ) {
                    LPCC val=pv->Trim(mp+1,pv->start);
                    if( ccmp(val,"key") ) {
                        result->set(sv->get(FUNC_HEADER_POS,pos));
                    } else {
                        int len=sv->length();
                        if( len>pos ) {
                            result->add( sv, pos+1, len );
                        }
                    }
                }
            }
        }
        if( sub==NULL ) {
            return false;
        }
        LPCC val=pv->Trim(mp+1,pv->start);
        sv=sub->gv(val);
        if( sv==NULL ) {
            return false;
        }
        ch=pv->ch();
        if( ch=='.' ) {
            pv->incr();
            while( pv->valid() ) {
                if( SVCHK('n',0) ) {
                    sub=(TreeNode*)SVO;
                } else {
                    return false;
                }
                val=pv->NextWord();
                sv=sub->gv(val);
                if( sv==NULL ) {
                    return false;
                }
                if( ch!='.' ) break;
            }
        }
        result->add(sv);
    } else {
        LPCC val=pv->Trim(sp,pv->start);
        if( ccmp(val,"true") ) {
            result->setVar('3',1);
        } else if( ccmp(val,"false") ) {
            result->setVar('3',0);
        } else if( isnum(val) ) {
            result->setVar('0',0).addInt(atoi(val));
        } else {
            sv=(target) ? target->gv(val): NULL;
            if( sv && sv!=result ) {
                setStrVar(result, sv );
            } else {
                sv=getFuncVar(fn,val);
                if( sv && sv->charAt(0)!='\b' && sv->find("\\") !=-1 ) {
                    decodeUnicode(sv, 0, sv->length(), result);
                } else {
                    setStrVar(result, ccmp(val,"this") ? fn->gv("@this"): sv );
                }
            }
        }
    }
    return true;
}

bool setModifyClassFunc(TreeNode* node, StrVar* sv, int sp, int ep, XFuncNode* fn, StrVar* rst, bool append, LPCC funcLine) {
    if( node==NULL ) return false;
    XParseVar pv(sv, sp, ep);
    XFuncNode* fnInit=NULL;
    XFuncSrc* fsrc=NULL;
    LPCC extendId=NULL;
    int pos=IndexOf(funcLine,'.');
    if(pos>0) {
        extendId=funcLine+pos+1;
        funcLine=gBuf.add(funcLine,pos);
    }
    int sa=0, ea=0;
    char c=pv.ch();
    while( pv.valid() ) {
        c=pv.ch();
        if( c==0 || c=='>' ) break;
        if( c==';' ) {
            c=pv.incr().ch();
        }
        LPCC funcMode=funcLine;
        /*
        if( c=='#' ) {
            funcMode=pv.incr().findEnd("\n").v();
        }
        */
        if( c=='/' ) {
            c=pv.ch(1);
            if( c=='/') {
                pv.findEnd("\n");
            } else if( c=='*' ) {
                pv.match("/*", "*/", FIND_SKIP);
            }
            c=pv.ch();
        }
        sa=pv.start;
        pv.moveNext();
        if( pv.ch()=='#' ) {
            pv.incr().moveNext();
        }
        ea=pv.start;
        TreeNode* target=node;
        LPCC subId=NULL, funcNm=NULL, param=NULL;
        if( pv.ch()=='.' ) {
            subId=pv.Trim(sa,ea);
            target=findChildId(node,subId);
            pv.incr().ch();
            sa=pv.start;
            pv.moveNext();
            if( pv.ch()=='#' ) {
                pv.incr().moveNext();
            }
            ea=pv.start;
        }
        funcNm=pv.Trim(sa,ea);
        if( rst ) {
            if( rst->length() ) rst->add(",");
            rst->add(funcNm);
        }
        c=pv.ch();
        if( c=='=' ) {
            funcMode=pv.incr().NextWord();
            c=pv.ch();
        }
        if( c=='(' && pv.match("(",")") ) {
            param=pv.v();
            c=pv.ch();
        }
        if( c!='{' ) break;
        if( pv.match("{","}",FIND_SKIP)==false ) break;
        if( slen(funcNm)==0 ) {
            qDebug("#9##class function error (name: %s)\n", funcNm);
            break;
        }
        if( target==NULL ) {
            qDebug("#9#fuction modify error (%s:%s)", subId, funcNm);
            c=pv.ch();
            if( c==';' ) pv.incr();
            continue;
        }
        bool addMode=false;
        if(ccmp(funcNm,"class")) {
            funcNm="@class";
            fsrc=gfsrcs.getFuncSrc();
            addMode=true;
        } else {
            sv=target->gv(funcNm);
            if( SVCHK('f',1) ) {
                fsrc = (XFuncSrc*)SVO;
            } else if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                XFunc* fc=fnCur->xfunc;
                fsrc=fc ? fc->getFuncSrc(): NULL;
                if( fsrc && ccmp(funcNm,"onInit") ) {
                    fnInit=fnCur;
                }
            } else {
                fsrc=gfsrcs.getFuncSrc();
                addMode=true;
            }
        }
        if( fsrc ) {
            if( addMode==false  ) {
                if(append) {
                    fsrc=gfsrcs.getFuncSrc();
                } else {
                    fsrc->reuse();
                    fsrc->xparam.reuse();
                    fsrc->xflag=0;
                    if( fsrc->xfunc ) {
                        gfuncs.deleteFunc(fsrc->xfunc);
                        fsrc->xfunc=NULL;
                    }
                }
            }
            if( slen(param) ) fsrc->xparam.set(param);
            fsrc->readBuffer(pv.GetVar(), pv.prev, pv.cur);
            if( append ) {
                XParseVar pv((StrVar*)fsrc, ep, fsrc->length() );
                if( fsrc->makeFunc(pv, NULL, 0) && fsrc->xfunc ) {
                    qDebug("#0## object function append ok (name: %s)", funcNm);
                } else {
                    qDebug("#9## object function append error (name: %s)\n", funcNm);
                }
                // qDebug("func count end : %d\n", fsrc->xfunc->getFuncSize());
            } else {
                if( fsrc->makeFunc() && fsrc->xfunc ) {
                    // qDebug("#0## object function make ok (name: %s)", funcNm);
                } else {
                    qDebug("#9## object function make error (name: %s)\n", funcNm);
                }
            }
            XFunc* func=fsrc->xfunc;
            if( StartWith(funcNm,"on") && slen(funcNm)>2 && isUpperCase(funcNm[2]) ) {
                if( !ccmp(funcMode,"event") ) funcMode="event";
            }
            if( ccmp(funcLine,"class") ) {
                // LPCC classId=node->get("classId");
                if( ccmp(funcMode,"event") || ccmp(funcNm,"@class") ) { /*&& slen(classId)==0 ) {*/
                    funcMode="class";
                    if( func ) {
                        func->xflag|=FUNC_INLINE|FFLAG_REF;
                    }
                } else if(ccmp(funcMode,"class") ) {
                    funcMode="func";
                }
            } else if( ccmp(funcMode,"static") ) {
                funcMode="func";
            }
            if( ccmp(funcNm,"init") ) {
                if( slen(extendId) ) {
                    funcNm=FMT("init#%s",extendId);
                }
                if( ccmp(funcMode,"inline") || ccmp(funcMode,"func") ) {
                    if( func ) {
                        func->xflag|=FUNC_INLINE|FFLAG_EQ;
                    }
                }
            } else if( ccmp(funcMode,"inline") || ccmp(funcMode,"private") || ccmp(funcMode,"func") ) {
                if( func ) {
                    func->xflag|=FUNC_INLINE;
                    if( ccmp(funcMode,"inline") || ccmp(funcMode,"private") ) {
                        func->xflag|=FFLAG_EQ;
                        if( ccmp(funcMode,"private") ) func->xflag|=FFLAG_PSET;
                    }
                }
            }
            qDebug("#0# function add:%s (%s: %s)\n", funcNm, funcMode, addMode?"add":"modify");
            if( addMode ) {
                if( ccmp(funcMode,"event") ) {
                    XFuncNode* fnParent=NULL;
                    TreeNode* page=getPageNode(target);
                    /*
                    sv=node->gv("@canvasNode");
                    if( SVCHK('n',0) ) {
                        TreeNode* canvas=(TreeNode*)SVO;
                        sv=canvas->gv("onInit");
                        if( SVCHK('f',0) ) {
                            fnParent=(XFuncNode*)SVO;
                        }
                        page=getPageNode(canvas);
                    } else {
                        page=getPageNode(target);
                    }
                    */
                    if( ccmp(funcNm,"onInit") && target!=page ) {
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
                        target->setNodeFlag(FLAG_SETEVENT);
                    }
                    if( slen(extendId)==0 ) {
                        XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fnParent);
                        if(page) fnCur->GetVar("@page")->setVar('n',0,(LPVOID)page);
                        fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)target);
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)target);
                        fnCur->setNode(FLAG_PERSIST);
                        target->GetVar(funcNm)->setVar('f',0,(LPVOID)fnCur );
                    }
                } else {
                    if( slen(extendId) ) {
                        sv=target->gv(funcNm);
                        if(sv) {
                            target->GetVar(FMT("%s#%s",funcNm,extendId))->setVar('f',1,(LPVOID)fsrc );
                        } else {
                            target->GetVar(funcNm)->setVar('f',1,(LPVOID)fsrc );
                        }
                    } else {
                        target->GetVar(funcNm)->setVar('f',1,(LPVOID)fsrc );
                    }
                }
            }
        }        
    }
    if( fnInit && node->isNodeFlag(FLAG_START) ) {
        fnInit->call();
        node->setNodeFlag(FLAG_INIT);
        qDebug("...[setModifyClassFunc] onInit function call (node: %s)",node->log() );
    }
    return true;
}

XFuncNode* setFuncNodeParent(XFuncSrc* fsrc, XFuncNode* fn, TreeNode* thisNode, TreeNode* meNode) {
    TreeNode* pageNode=NULL;
    XFuncNode* pfn=NULL;
    StrVar* sv=NULL;
    if( thisNode ) {
        pageNode=findPageNode(thisNode);
    } else {
        sv=fn->gv("@this");
        if( sv==NULL ) {
            sv=getFuncVar(fn,"@this");
        }
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
        }
        if( thisNode ) {
            pageNode=findPageNode(thisNode);
        }
    }
    if( thisNode && fsrc->xfunc ) {
        if( pageNode==NULL )
            pageNode=thisNode;
        sv=pageNode->gv("onInit");
        if( SVCHK('f',0) ) {
            pfn=(XFuncNode*)SVO;
        }
        XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, pfn);
        fnCur->setNodeFlag(FLAG_PERSIST);

        if( thisNode ) {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
        }
        return fnCur;
    }
    return NULL;
}

XFuncNode* setCallbackFunction(XFuncSrc* fsrc, XFuncNode* fn, TreeNode* thisNode, XFuncNode* fnPrev, StrVar* sv) {
    XFuncNode* fnCur=NULL;
    if( fnPrev==NULL && SVCHK('f',0) )  {
        fnPrev=(XFuncNode*)SVO;
    }
    XFuncSrc* fsrcPrev = ( fnPrev && fnPrev->xfunc ) ? fnPrev->xfunc->getFuncSrc(): NULL;
    if( fnPrev && fsrc==fsrcPrev ) {
        fnCur=fnPrev;
    } else {
        fnCur=setFuncNodeParent(fsrc, fn, thisNode);
        if( fnPrev && thisNode ) {
            fnPrev->clearNodeFlag(FLAG_PERSIST);
            funcNodeDelete(fnPrev);
        }
    }
    if( sv ) {
        sv->setVar('f',0,(LPVOID)fnCur);
    }
    return fnCur;
}

XFuncNode* getEventFuncNode(TreeNode* tn, LPCC eventName ) {
    StrVar* sv=tn?tn->gv(eventName):NULL;
    XFuncNode* fn=NULL;
    if( SVCHK('f',0) ) {
        fn=(XFuncNode*)SVO;
        XFunc* fcCur= fn->xfunc;
        if( fcCur==NULL ) {
            qDebug("#9#event not define (getEventFuncNode:%s)\n", eventName);
            return NULL;
        }
        sv=fn->gv("@this");
        if( !SVCHK('n',0) ) {
            fn->GetVar("@this")->setVar('n',0,(LPVOID)tn);
        }
        fn->set("_funcName_", eventName);
    }
    return fn;
}
XFuncNode* setFuncResult(XFuncNode* pfn, StrVar* var, int psp, int pep, int sp, int ep, StrVar* rst) {
    XFuncSrc* src = gfsrcs.getFuncSrc();
    src->readBuffer(var,sp,ep );
    src->makeFunc();
    if( src->xfunc ) {
        XFuncNode* fn=gfns.getFuncNode(src->xfunc,pfn);
        if( psp<pep ) {
            src->xparam.set(var->get(psp),pep-psp);
        }
        rst->setVar('f',0,(LPVOID)fn);
        return fn;
    }
    return NULL;
}
XListArr* setArrayVal(XParseVar* pv, XListArr* arr, XFuncNode* fn) {
    if( arr==NULL ) arr=getListArrTemp();
    while( pv->valid() ) {
        char ch=pv->ch();
        if( ch==0 ) break;
        if( ch==',' ) {
            pv->incr();
        }
        if( !setResultVal(pv,fn,arr->add(),NULL ) ) {
            break;
        }
    }
    return arr;
}
TreeNode* setJsonVal(XParseVar* pv, TreeNode* node, XFuncNode* fn) {
    StrVar* sv=NULL;
    if( node==NULL ) {
        node=getTreeNodeTemp();
    }

    while( pv->valid() ) {
        char ch=pv->ch();
        if( ch==0 ) break;
        if( ch==',' ) {
            ch=pv->incr().ch();
        }
        LPCC name=NULL, key=NULL;
        int sp=pv->start, ep=0;
        if( ch=='@') {
            pv->incr();
        }
        ch=pv->moveNext().ch();
        if( ch=='-' || ch=='#' ) {
            // name=pv->incr().NextWord();
            pv->incr().moveNext();
        }
        ep=pv->start;
        if( sp<ep ) {
            key=pv->Trim(sp,ep);
            name=key[0]=='@'? key+1: key;
        } else {
            break;
        }
        ch=pv->ch();
        if( ch!=':') {
            if( ch==',' || ch==0 ) {
                setStrVar(node->GetVar(key), getFuncVar(fn,name) );	// fn,name[0]=='@'? name+1:
            } else {
                break;
            }
            pv->incr();
        } else {
            pv->incr();
            if( !setResultVal(pv,fn,node->GetVar(key)->reuse(), node) ) {
                break;
            }
        }
    }
    return node;
}

StrVar* setStrVar(StrVar* rst, StrVar* sv) {
    if( rst==NULL ) return NULL;
    if( sv==NULL ) {
        rst->reuse();
    } else if( SVCHK('s',0) ) {
        int sp=0, ep=0;
        StrVar* val=getStrVarPointer(sv,&sp,&ep);
        int size=val->length();
        if( sv!=rst ) {
            int len=ep-sp;
            if( len  <= size ) {
                rst->reuse()->add(val->get(sp), len);
            } else {
                rst->reuse();
                qDebug("#0#string ref not valid (%d,%d)\n",sp, ep);
            }
        }
    } else {
        rst->reuse();
        if( sv) rst->add(sv);
    }
    return rst;
}



