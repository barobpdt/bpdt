#include "func_util.h"
#include "../util/user_event.h"

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
        // qDebug("#0#getLocalParams new param add!!!");
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
StrVar* getFuncVar(XFuncNode* fn, LPCC code, bool memberCheck) {
    if( slen(code)==0 ) return NULL;
    /*
    StrVar* sv = uom.getInfo()->gv("userFuncVars");
    if(SVCHK('n',0)) {
        TreeNode* ufm=(TreeNode*)SVO;
        sv = ufm->gv(code);
        if( sv && !SVCHK('9',0) ) {
            return sv;
        }
    }
    */
    if( fn==NULL )
		return NULL;
	if(ccmp(code,"this")) {
		return fn->gv("@this");
	}
    StrVar* sv=fn->gv(code);
    if( sv==NULL ) {
        XFuncNode* fnCur=fn->parent();
        for(int n=0; fnCur && n<16; n++ ) {
			sv = fnCur->gv(code);
            if( sv ) return sv;
            fnCur = fnCur->parent();
        }
    }
    if( sv==NULL && memberCheck ) {
#ifdef INLINENODE_USE
        sv=fn->gv("@inlineNode");
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
        } else {
            sv=fn->gv("@this");
            if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
            }
        }
#endif
		TreeNode* thisNode=NULL;
		sv=fn->gv("@this");
		if( SVCHK('n',0) ) {
			thisNode=(TreeNode*)SVO;
		}
        if(thisNode ) {
            sv=thisNode->gv("onInit");
            if(SVCHK('f',0)) {
                XFuncNode* fnInit=(XFuncNode*)SVO;
                sv=fnInit->gv(code);
            } else {
                sv=NULL;
            }
        } else {
            sv=NULL;
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
    if( cur && cur!=rst ) {
        rst->reuse()->add(cur);
    }
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
inline bool isFuncPos(LPCC vnm, XParseVar& sub) {
	char ch=sub.incr().moveNext().ch();
	if( ch==0 ) return 0;
	while( ch=='.' ) {
		ch=sub.incr().moveNext().ch();
	}
	if( ch=='(') {
		if( sub.match("(",")") ) {
			return true;
		}
		qDebug("#0#parseArray eval call error(funcRef:%s)\n", vnm );
	}
	return false;
}

StrVar* parseArrayVar(XParseVar& sub, TreeNode* node, XFuncNode* fn, StrVar* rst, U32 flag, XFunc* fc ) {
    StrVar codeVar;
    StrVar* svVar=NULL;
    StrVar* sv=uom.getInfo()->gv("@callFuncNode");
	bool bfloat=false, bInt=false, endCheck=false, bInitFunc=false;
    int n=0, aa=0, bb=0;
    float af=0.0, bf=0.0;
    int oper=0, operNext=0, findex=(int)flag&0xFF;
    bool self=false, beq=false;
    if(SVCHK('f',0)) {
        fn=(XFuncNode*)SVO;
        sv=NULL;
    }

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
        }
        if( ch==0 || ch==';' || ch==',' || endCheck ) {
            if( n && oper ) {
                // qDebug("...var:%s af:%f, aa:%d, bb:%d %s (n:%d, oper:%d )", codeVar.get(), af, aa, bb, bfloat?"float":"int", n, oper  );
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
			n++;
			oper=8;
			endCheck=true;
			continue;
		}
		if( ch=='@' ) {
			// ex) @(10/3)+1 == 4
			bInt=true;
			sub.incr();
			continue;
		}
		bool bMinus=false, bshap=false;
		if( ch=='-' ) {
			bMinus=true;
			ch=sub.incr().ch();
		}
		if( ch=='#' ) {
			bshap=true;
			ch=sub.incr().ch();
		}
		if( ch==0 ) continue;
		bool svchk=false, achk=false, fchk=false;
		if( ch=='(' ) {
			if( oper>20 ) {
				return NULL;
			}
			sv=NULL;
			if( sub.match("(",")") ) {
				XParseVar pv(sub.GetVar(), sub.prev, sub.cur);
				sv=parseArrayVar(pv, node, fn, getStrVarTemp(), flag|0x800);
			} else {
				qDebug("#0#array var parse error(not match function %s)\n", sub.get() );
				return NULL;
			}
			svchk=true;
		} else {
			int sp=sub.start, ep=0;
			if( beq || n==0 ) {
				beq=false;
				achk=true;
			}
			ch=sub.moveNext().ch();
			if( ch=='#' ) ch=sub.incr().moveNext().ch();
			if( achk && oper==0 && (ch==','||ch==';' ) ) {
				continue;
			}
			ep=sub.start;
			LPCC vnm=sub.Trim(sp, ep);
            LPCC funcType=NULL;
            if( ccmp(vnm,"public") || ccmp(vnm,"virtual") || ccmp(vnm,"private") ) {
                funcType=vnm;
                sub.ch();
                sp=sub.start;
                ch=sub.moveNext().ch();
                if( ch=='#' ) ch=sub.incr().moveNext().ch();
                ep=sub.start;
                vnm=sub.Trim(sp, ep);
            }

            sv=NULL;
            if( ch=='(' ) {
                int psp=0, pep=0;
                if( sub.match("(", ")",FIND_SKIP) ) {
                    psp=sub.prev, pep=sub.cur;
                } else {
                    qDebug("#0#node inline function match error(function:%s, name:%s)\n", getBaseFuncName(fn), vnm);
                    return NULL;
                }
                sv=rst->reuse();
                if( sub.ch()=='{' && sub.match("{","}",FIND_SKIP) ) {
                    ep=sub.start;

                    if(bInt) {
                        sp-=1;
                        bInt=false;
                    }
                    if( !setModifyClassFunc(node, sub.GetVar(), sp, ep, fn, sv, false, funcType) ) {
                        qDebug("#0#node inline function make error(function:%s )\n", vnm );
                    }
                    // if(ccmp(vnm,"onInit") ) bInitFunc=true;
                    svVar=NULL;
                    oper=0;
                    endCheck=true;
                    continue;
                } else {
                    qDebug("#0#node inline function match error(function:%s, name:%s)\n", getBaseFuncName(fn), vnm);
                }
                return NULL;
            } else if( isnum(vnm) ) {
                bf=0.0, bb=0;
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
				if(isFuncPos(vnm, sub)) {
					sv=rst->reuse();
					callEvalFunc(sub.GetVar(), sp, sub.start, fn, sv, NULL, NULL );
					svchk=true;
				} else {
					bool ok=true;
					TreeNode* cur=NULL;
					sub.start=ep;
					if(bshap) {
						sv=getFuncVar(fn,vnm);
					} else if(flag&0x400) {
						sv=getFuncVar(fn,vnm);
						if(!sv) {
							sv=node->gv(vnm);
						}
					} else {
						sv=node->gv(vnm);
						if(!sv) {
							sv=getFuncVar(fn,vnm);
						}
					}
					while(ch=='.') {
						if(ok && SVCHK('n',0)) {
							cur=(TreeNode*)SVO;
						} else {
							ok=false;
						}
						sub.incr();
						vnm=sub.NextWord();
						ch=sub.ch();
						if(ch!='.') {
							if(ok && cur && slen(vnm) ) {
								sv=rst->reuse();
								sv->add(cur->gv(vnm));
							}
							break;
						}
					}
					if(!ok) {
						sv=rst->reuse();
						sv->setVar('0',0).addInt(0);
					}
					svchk=true;
					// qDebug("parse var %s [%c] %s %s", vnm, ch, toString(sv), ok?"ok":"*" );
				}
            } else {
				LPCC key = vnm;
				if( ccmp(key,"null") || ccmp(key,"true") || ccmp(key,"false") ) {
					LPCC code=codeVar.get();
					sv=slen(code) ? node->GetVar(code): codeVar.reuse();
					if( ccmp(key,"null") ) {
						sv->setVar('9',0);
					} else {
						sv->setVar('3',ccmp(key,"true")?1:0 );
					}
					continue;
				}
				if(bshap) {
					sv=getFuncVar(fn,vnm);
				} else if(flag&0x400) {
					sv=getFuncVar(fn,key);
					if(!sv) {
						sv=node->gv(key);
					}
				} else {
					sv=node->gv(key);
					if(!sv) sv=fn->gv(key);
				}
				svchk=true;
				if( n==0 ) {
					codeVar.set(key);
				}
				// qDebug("var %s == %s (n:%d)",vnm, toString(sv), n );
			}
		} // if c!='('

		if( n==0 || beq ) {
			achk=true;
			if(svVar==NULL) svVar=sv;
			ch = sub.ch();
			if( ch==0 ) continue;
			n++;
		}
		if( svchk ) {
			if( SVCHK('3',0) || SVCHK('3',1) ) {
				bInt=true;
				fchk=false;
				bb=SVCHK('3',0)?0:1;
			} else if( SVCHK('4',0) ) {
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
			} else {
				bInt=true;
				fchk=false;
				bb=0;
				bf=0.0;
			}
		}
		if(bMinus) {
			if(fchk) bf*=-1;
			else bb*=-1;
		}
		if( achk ) {
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

        ch = sub.ch();
		/*
		qDebug(">> %s oper:%d (af:%f,aa:%d bf:%f, bb:%d) (c:%c)",
			   achk?"a":"b", oper,
			   af, aa, bf, bb, ch  );
		*/
        if( ch==0 || ch==';' || ch==',' ) {
            if( oper==0 ) {
                svVar=sv;
            }
            continue;
        }
        if( ch==':' || (ch=='=' && sub.ch(1)!='=') ) {
            aa=0, bb=0;
            af=0.0, bf=0.0;
            self=true;
            oper = 8;
            beq=true;
			bInt = bfloat=false;
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
    /*
	if(bInitFunc) {
		sv= node->gv("onInit");
		qDebug("parse node init function call !!!");
		if(SVCHK('f',0) ) {
			XFuncNode* fnInit=(XFuncNode*)SVO;
			fnInit->xparent=fn;
            fnInit->GetVar("funcNode")->setVar('f',0,(LPVOID)fn);
			fnInit->call();
            fnInit->GetVar("funcNode")->reuse();
			fnInit->xparent=NULL;
			for(WBoxNode* bn=node->First(); bn; bn=node->Next() ) {
				sv=&(bn->value);
				if(SVCHK('f',0) ) {
					XFuncNode* fnCur=(XFuncNode*)SVO;
					if(fnCur!=fnInit) {
						fnCur->xparent=fnInit;
					}
				}
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
    */
    return rst;
}

bool makeTextData(XParseVar& pv, XFuncNode* fn, StrVar* rst, U32 flag, TreeNode* tn ) {
    // flag & 0x10000
    // flag & 0x20000
    // flag & 0x40000
    char ch = 0;
    // int sp=0,ep=0; //,end=0;

    while( pv.valid() ) {
        pv.findEnd("$",FIND_FORWORD);
        if( pv.cur>pv.prev ) rst->add(pv.GetVar(),pv.prev,pv.cur);
        ch = pv.ch();
        if( ch==0 )
            break;

        if( ch=='$') {
            pv.incr();
            rst->add("$$");
            continue;
        }
        if( (flag & 0x20000) && (ch!='{') ) {
            rst->add('$');
            continue;
        }
        bool cchk=ch=='{'? true: false;

        if( cchk ) {
            if( pv.match("{","}", FIND_SKIP) ) {
                bool evalCheck=false;
                int sp=pv.prev, ep=pv.cur;
                XParseVar sub(pv.GetVar(), sp, ep);
                LPCC var=sub.NextWord();
                if(slen(var)==0 ) {
                    rst->add(" ");
                    continue;
                }
                StrVar* sv = tn ? tn->gv(var): NULL;
                if( sv==NULL ) {
                    sv=getFuncVar(fn, var);
                }
                ch=sub.ch();
                if(ch==0) {
                    if(sv) rst->add(toString(sv) );
                    continue;
                }
                if(ch=='(') {
                    evalCheck=true;
                } else {
                    if(ch=='.') {
                        TreeNode* cur=NULL;
                        while(ch=='.') {
                            if( !SVCHK('n',0)) {
                                evalCheck=true;
                                break;
                            }
                            cur=(TreeNode*)SVO;
                            sub.incr();
                            var=sub.NextWord();
                            ch=sub.ch();
                            if( cur && slen(var) ) {
                                sv=cur->gv(var);
                                if( sv==NULL) {
                                    sv=cur->gv("onInit");
                                    if(SVCHK('f',0)) {
                                        XFuncNode* fnInit=(XFuncNode*)SVO;
                                        sv=fnInit->gv(var);
                                    } else {
                                        sv=NULL;
                                    }
                                }
                            }
                            if(ch==0) {
                                if(sv) rst->add(toString(sv) );
                                break;
                            }
                            if(ch=='.') {
                                continue;
                            }
                            if(ch=='(' || isoper(ch)) {
                                evalCheck=true;
                                break;
                            }

                        }
                    }
                }
                if(evalCheck ) {
                    XParseVar fsub(pv.GetVar(), sp, ep);
                    XFuncSrc* fsrc = getFuncSrc();
                    fsrc->parseData(pv.GetVar(), sp, ep);
                    if( fsrc->makeFunc() ) {
                        XFunc* fc=fsrc->xfunc;
                        StrVar* var=getStrVarTemp();
                        if(fc->getFuncSize()==1) {
                            execFunc(fc->getFunc(0), fn, var);
                        } else {
                            qDebug("makeTextData eval call %d %d (size:%d)", sp, ep, fc->getFuncSize() );
                        }
                        /*
                        fn->clearNodeFlag(FLAG_CALL);
                        fn->call(fsrc->xfunc, var );
                        */
                        rst->add(toString(var));
                    }
                }
            } else {
                pv.incr();
            }
        } else {
            LPCC var = pv.NextWord();
            if(slen(var)==0 ) {
                continue;
            }
            StrVar* sv = tn ? tn->gv(var): NULL;
            if( sv==NULL ) {
                sv=getFuncVar(fn, var);
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
    LPCC code=func->getVarName();
    if( slen(code)==0 ) return NULL;
    if( func->xdotPos==0 ) {
        if(code[0]=='@') {
            return getStrVar("fsrc", code+1);
        }
        return getFuncVar(fn, func->getValue(), true);
    }
    StrVar* sv = NULL;
    if(code[0]=='@') {
        return getSubFuncSrc(code+1, func->getFuncName());
    }
    LPCC funcName=func->getFuncName();
    if( isUpperCase(code[0]) && ccmpl(code,"Cf") ) {
        return getCf()->GetVar(funcName);
    }
    sv=getFuncVar(fn, code);
    if( SVCHK('n',0) ) {
        return getSubVarValue((TreeNode*)SVO, funcName);
    }
    qDebug("#0#object var value %s not defined !!!", code);
    return NULL;
}

void getOperValue(XFuncNode* fn, XFunc* param, int size, StrVar* rst) {
    if( param==NULL ) return;
    if( size==1 ) {
        if( param->getType()==FTYPE_NUMBER ) return;
        LPCC nodeVar=NULL;
        LPCC code=param->getValue();
		/*
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
		*/
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

    int sa=0, ea=0;
    char c=pv.ch();
    while( pv.valid() ) {
        c=pv.ch();
        if( c==0 || c=='>' ) break;
        if( c==';' ) {
            c=pv.incr().ch();
        }
        if( c=='/' ) {
            c=pv.ch(1);
            if( c=='/') {
                pv.findEnd("\n");
            } else if( c=='*' ) {
                if( !pv.match("/*", "*/", FIND_SKIP) ) {
                    break;
                }
            } else {
                pv.incr();
            }
            continue;
        }
        LPCC funcMode=funcLine;
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
#ifdef WIDGET_USE
            target=findChildId(node,subId);
#endif
            pv.incr().ch();
            sa=pv.start;
            pv.moveNext();
            if( pv.ch()=='#' ) {
                pv.incr().moveNext();
            }
            ea=pv.start;
        }
        funcNm=pv.Trim(sa,ea);
        if( slen(funcNm)==0 ) {
            qDebug("#0##class function error (name: %s)\n", funcNm);
            break;
        }
        if( rst ) {
            if( rst->length() ) rst->add(",");
            rst->add(funcNm);
        }
        c=pv.ch();
        if( c=='=' ) {
			// aa=private() {} >> funcMode=='private'
            funcMode=pv.incr().NextWord();
            c=pv.ch();
        }
        if( c=='(' && pv.match("(",")") ) {
            param=pv.v();
            c=pv.ch();
        }
        if( c!='{' ) {
            qDebug("#0##class function start error (name: %s)\n", funcNm);
            break;
        }
        if( pv.match("{","}",FIND_SKIP)==false ) {
            qDebug("#0##class function match error (name: %s)\n", funcNm);
            break;
        }

        if( target==NULL ) {
            qDebug("#9#class fuction target error (%s:%s)", subId, funcNm);
            c=pv.ch();
            if( c==';' ) pv.incr();
            continue;
        }
        bool addMode=false;
        if( !target->isNodeFlag(FLAG_SETEVENT) ) {
            target->setNodeFlag(FLAG_SETEVENT);
            qDebug("#0#class target FLAG_SETEVENT add");
        }
        sv=target->gv(funcNm);
        if( SVCHK('f',1) ) {
            fsrc = (XFuncSrc*)SVO;
            if( fsrc->xflag & FLAG_REF ) {
                fsrc=gfsrcs.getFuncSrc();
                if(ccmp(funcMode,"virtual") ) {
                    fsrc->xflag|=FLAG_REF;
                }
                addMode=true;
            }
        } else if( SVCHK('f',0) ) {
            XFuncNode* fnCur=(XFuncNode*)SVO;
            XFunc* fc=fnCur->xfunc;
            fsrc=fc ? fc->getFuncSrc(): NULL;
        } else {
            fsrc=gfsrcs.getFuncSrc();
            if(ccmp(funcMode,"virtual") ) {
                fsrc->xflag|=FLAG_REF;
            }
            addMode=true;
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

            if( StartWith(funcNm,"on") && slen(funcNm)>2 && isUpperCase(funcNm[2]) ) {
                if( !ccmp(funcMode,"event") ) funcMode="event";
            }
            /*
            XFunc* func=fsrc->xfunc;
            if( ccmp(funcLine,"class") ) {
                // LPCC classId=node->get("classId");
                if( ccmp(funcMode,"event") || ccmp(funcNm,"@class") ) {
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
			} else if( ccmp(funcMode,"inline") || ccmp(funcMode,"private") ) { //  || ccmp(funcMode,"func")
                if( func ) {
                    func->xflag|=FUNC_INLINE;
					func->xflag|=FFLAG_EQ;
					if( ccmp(funcMode,"private") ) func->xflag|=FFLAG_PSET;
                }
            }
            */

            qDebug("#0# function add:%s (%s: %s)\n", funcNm, funcMode, addMode?"add":"modify");
            if( addMode ) {
                if( ccmp(funcMode,"event") ) {
                    XFuncNode* fnParent=NULL;
                    sv=target->gv("onInit");
                    if( SVCHK('f',0) ) {
                        fnParent=(XFuncNode*)SVO;
                    }
                    XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fnParent);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)target);
                    fnCur->setNodeFlag(FLAG_PERSIST);
                    target->GetVar(funcNm)->setVar('f',0,(LPVOID)fnCur );
                    if( ccmp(funcNm,"onInit") ) {
                        fnInit=fnCur;
                    }
                } else {
                    target->GetVar(funcNm)->setVar('f',1,(LPVOID)fsrc );
                }
            }
        }        
    }
    if( fnInit && node ) {
        fnInit->xparent = NULL;
        // fnInit->call();
        for(WBoxNode* bn=node->First(); bn; bn=node->Next()) {
            sv=&(bn->value);
            if(SVCHK('f',0)) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                if(fnCur!=fnInit) {
                    if(fnCur->xparent && fnCur->xparent!=fnInit) {
                        qDebug("#0#class init function reset parent name:%s\n", node->getCurCode());
                    }
                    fnCur->xparent=fnInit;
                }
            }
        }
        node->setNodeFlag(FLAG_INIT);
    }
    return true;
}

XFuncNode* setFuncNodeParent(XFuncSrc* fsrc, XFuncNode* fn, TreeNode* thisNode, TreeNode* meNode) {
	// TreeNode* pageNode=NULL;
	XFuncNode* pfn=fn;
	StrVar* sv=NULL;
	if( fsrc->xfunc ) {
		if( fn && thisNode==NULL ) {
			sv=fn->gv("@this");
			if( sv==NULL ) {
				sv=getFuncVar(fn,"@this");
			}
			if( SVCHK('n',0) ) {
				thisNode=(TreeNode*)SVO;
				sv=thisNode->gv("onInit");
				if( SVCHK('f',0) ) {
					pfn=(XFuncNode*)SVO;
				}
			}
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
        if(fn && fn->isNodeFlag(FLAG_PERSIST)) fnCur->xparent=fn;
    } else {
        fnCur=setFuncNodeParent(fsrc, (fn && fn->isNodeFlag(FLAG_PERSIST))?fn:NULL, thisNode);
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



