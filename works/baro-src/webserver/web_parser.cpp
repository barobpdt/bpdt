
{#test(id)#}

bool makeEvalValue(StrVar* var, int sp, int ep, ConnectNode* cn, TreeNode* webPageNode, XFuncNode* fnInit, StrVar* rst, LPCC incName) {
    XParseVar pv(var, sp, ep);
    char ch=pv.ch();
    if( ch==0 ) return false;
    StrVar a,  b;
    while( pv.valid() ) {
        bool bcheck=false;
        sp=pv.start;
        if(slen(incName)==0) incName="main";
        evalVarCompare(pv, cn, webPageNode, fnInit, &a, NULL, NULL);
        if(ch=='?') {
            if( a.length() && isVarTrue(&a)) {
                bcheck=true;
            }
            pv.incr();
        } else {
            LPCC oper=NULL;
            int osp=0;
            ch=pv.ch();
            if( ch=='=' || ch=='<' || ch=='>' || ch=='&' || ch=='|' ) {
                osp=pv.start;
                ch=pv.incr().ch();
                if( ch=='=' || ch=='<' || ch=='>' || ch=='&' || ch=='|' ) ch=pv.incr().ch();
            }
            if( osp ) {
                oper=pv.Trim(osp, pv.start );
            }
            bcheck=evalVarCompare(pv, cn, webPageNode, fnInit, &b, &a, oper);
            if( pv.ch()!='?' ) return false;
            pv.incr();
        }
        makeEvalValueResult(pv, cn, webPageNode, fnInit, incName, bcheck, rst);
        if( bcheck ) {
            return true;
        }
        if( pv.ch()==':' ) {
            ch=pv.incr().ch();
            if( pv.StartWith("${") || ch=='<' || ch=='\'' || ch=='\"'  ) {
                return makeEvalValueResult(pv, cn, webPageNode, fnInit, incName, true, rst);
            }
        } else {
            qDebug("#0#makeEvalValue error start char:%c (a:%s,b:%s)\n",pv.ch(), toString(&a), toString(&b));
        }
    }
    return true;
}

inline bool makeEvalValueResult(XParseVar& pv, ConnectNode* cn, TreeNode* webPageNode, XFuncNode* fnInit, LPCC incName, bool bcheck, StrVar* rst) {
    StrVar* sv=NULL;
    TreeNode* obj=NULL;
    TreeNode* param=cn->config();
    char ch=pv.ch();
    // qDebug("------------ makeEvalValueResult (ch:%c) %s %s ----------------\n",ch, incName, bcheck?"true":"false");
    if( pv.StartWith("{`") ) {
        if( pv.match("{`", "`}", FIND_SKIP) ) {
            if( bcheck ) {
                char ch=checkEvalCase(pv.GetVar(), pv.prev, pv.cur);
                if( ch==0 || ch=='=' ) {
                    XParseVar sub(pv.GetVar(), pv.prev, pv.cur);
                    evalVarValue(sub, cn, webPageNode, fnInit, rst, ch);
                } else if( ch=='#' ) {
                    // {`#inc file`}
                    makeEvalSub(pv.GetVar(), pv.prev, pv.cur, cn, webPageNode, fnInit, rst, incName);
                } else {
                    LPCC fnm=FMT("%s_%d",incName,pv.cur);
                    callWebEvalFun(fnm, cn, webPageNode, fnInit, pv.GetVar(), pv.prev, pv.cur, rst, false);
                }
            }
        } else {
            qDebug("#0#not match eval value {`...`}\n");
        }
    } else if( ch=='<') {
        int sp=pv.start;
        LPCC tag=pv.incr().NextWord();
        pv.setPos(sp);
        if( ccmp(tag,"text") ) {
            if( pv.match("<text>", "</text>") ) {
                if( bcheck ) {
                    makeWebEvalText(pv.GetVar(), pv.prev, pv.cur, cn, webPageNode, fnInit, rst, FMT("%s_%d",incName, pv.cur), true );
                }
            } else {
                LPCC line=pv.findEnd("\n").v();
                qDebug("#0#eval web text tag match error (include:%s, pos:%d, line:%s)\n", incName, sp, line);
                return false;
            }
        } else {
            if( pv.match(FMT("<%s",tag), FMT("</%s>",tag)) ) {
                if( bcheck ) {
                    XParseVar sub(pv.GetVar(), pv.prev, pv.cur);
                    sub.findEnd(">");
                    if( sub.valid() ) {
                        rst->format(256,"<%s %s>\r\n", tag, sub.v());
                        makeWebEvalText(pv.GetVar(), sub.start, pv.cur, cn, webPageNode, fnInit, rst, FMT("%s_%d",incName, pv.cur) );
                        rst->format(32,"</%s>\r\n", tag);
                    }
                }
            } else {
                LPCC line=pv.findEnd("\n").v();
                qDebug("#0#eval tag match error (tag:%s, include:%s, pos:%d, line:%s)\n", tag, incName, sp, line);
                return false;
            }
        }

    } else if( ch=='#') {
        if( pv.StartWith("#[") ) {
            if( pv.match("#[", "]", FIND_SKIP) ) {
                if( bcheck ) {
                    XParseVar sub(pv.GetVar(), pv.prev, pv.cur);
                    makeTextData(sub, fnInit, rst, 0x20000, param);
                }
            } else {
                qDebug("#0#not match eval value #[...]\n");
            }
        } else if( pv.StartWith("#{") ) {
            if( pv.match("#{", "}", FIND_SKIP) ) {
                if( bcheck ) {
                    rst->add(pv.GetVar(), pv.prev, pv.cur);
                }
            } else {
                qDebug("#0#not match eval value #{...}\n");
            }
        } else {
            qDebug("#0#eval value not define keyword (use: #[] or #{}) \n");
        }
    } else if( ch=='\'' || ch=='\"') {
        pv.MatchStringPos(ch);
        if( bcheck ) {
            rst->add(pv.GetVar(), pv.prev, pv.cur);
        }
    } else {
        int sp=pv.start;
        ch=pv.moveNext().ch();
        if( ch=='#') ch=pv.incr().moveNext().ch();
        if( ch=='.') {
            int num=0;
            sv=NULL;
            for(; num<8 ; num++ ) {
                ch=pv.incr().ch();
                if( pv.ch()==0 ) break;
                ch=pv.moveNext().ch();
                if( ch=='#') ch=pv.moveNext().ch();
                if( ch!='.') {                   
                    break;
                }
            }
        }
        if(bcheck ) {
            XParseVar sub(pv.GetVar(), sp, pv.start);
            evalVarValue(sub, cn, webPageNode, fnInit, rst,'=');
        }
    }
    return bcheck;
}


checkEvalCase(StrVar* var, int sp, int ep) {
    XParseVar pv(var,sp,ep);
    char ch=pv.ch();
    if( ch==0 ) return ch;
    if( ch=='#' || ch=='=' || ch=='+' ) return ch;
    if( ch=='\'' || ch=='\"') {
        pv.MatchStringPos(ch);
    } else {        
        if(ch=='@') pv.incr();
        ch=pv.moveNext().ch();
        if( ch=='#' || ch=='-' ) ch=pv.incr().moveNext().ch();
        if( ch=='.' ) {
            ch=pv.incr().moveNext().ch();
            if( ch=='#' || ch=='-' ) ch=pv.incr().moveNext().ch();
        }
        if( ch=='(') {
            if(pv.match("(",")") ) {
                ch=pv.ch();
            } else {
                return 'e';
            }
            if( ch==';' ) {
                ch=pv.incr().ch();
            }
            if( ch==0 ) {
                return '&';
            }
        }
        if(ch=='?') return ch;
    }
    if( ch==0 ) return ch;

    if( ch=='=' || ch=='<' || ch=='>' || ch=='&' || ch=='|' ) {
        ch=pv.incr().ch();
        if( ch=='=' || ch=='<' || ch=='>' || ch=='&' || ch=='|' ) ch=pv.incr().ch();
    }
    if( ch=='\'' || ch=='\"') {
        pv.MatchStringPos(ch);
    } else if( ch=='[' ) {
        pv.match("[","]");
    } else {
        ch=pv.moveNext().ch();
        if( ch=='#') ch=pv.incr().moveNext().ch();
        while( ch=='.' ) {
            ch=pv.incr().moveNext().ch();
        }
        if( ch=='(') {
            if(pv.match("(",")") ) {
                ch=pv.ch();
            } else {
                return 'e';
            }
        }
    }
    if(ch=='?') {
        return ch;
    }
    return 'f';
}


        ch=checkEvalCase(pv.GetVar(), pv.prev, pv.cur);
        // qDebug("...checkEvalCase ch=%c\n", ch);

        if( ch==0 || ch=='=' ) {
            sub.SetVar(pv.GetVar(), pv.prev, pv.cur);
            evalVarValue(sub, cn, webPageNode, fnInit, var, ch);
        } else if( ch=='#' ) {
            // {`#inc file`}
            makeEvalSub(pv.GetVar(), pv.prev, pv.cur, cn, webPageNode, fnInit, var, incName);
        } else if( ch=='f' ) {
            // {` a(); b();`}
            LPCC fnm=FMT("%s_%d",incName,pv.cur);
            callWebEvalFun(fnm, cn, webPageNode, fnInit, pv.GetVar(), pv.prev, pv.cur, var, false);
        } else if( ch=='&' || ch=='+' ) {
            // {`appendText()`]
            sub.SetVar(pv.GetVar(), pv.prev, pv.cur);
            if(sub.ch()=='@') sub.incr();
            LPCC fnm=sub.findEnd("(").v();
            sub.start-=1;
            if( (sub.ch()=='(') && sub.match("(",")", FIND_SKIP) && slen(fnm) ) {
                if( ccmp(fnm,"conf") ) {
                    makeEvalSub(sub.GetVar(), sub.prev, sub.cur, cn, webPageNode, fnInit, var, incName);
                } else {
                    XFuncSrc* fsrc=NULL;
                    sv=webPageNode->gv(fnm);
                    if(SVCHK('f',1)) {
                        fsrc=(XFuncSrc*)SVO;
                    } else {
                        int pos=IndexOf(fnm,'.');
                        if(pos>0) {
                            LPCC ref=gBuf.add(fnm,pos);
                            fnm+=pos+1;
                            TreeNode* base=getTreeNode("ctrl", ref,false);
                            if(base) {
                                sv=base->gv(fnm);
                                if( SVCHK('f',1) ) {
                                    fsrc=(XFuncSrc*)SVO;
                                }
                            }
                            if(fsrc==NULL) {
                                sv=getSubFuncSrc(ref, fnm);
                                if( SVCHK('f',1) ) {
                                    fsrc=(XFuncSrc*)SVO;
                                }
                            }
                        } else {
                            sv=getStrVar("func", fnm);
                            if(SVCHK('f',1)) {
                                fsrc=(XFuncSrc*)SVO;
                            }
                        }
                    }
                    int sp=sub.prev, ep=sub.cur;
                    if( fsrc && var && sp<ep ) {
                        XListArr* arrs=NULL;                        
                        sub.SetVar(pv.GetVar(), sp, ep);
                        while(sub.valid()) {
                            char c=sub.ch();
                            if(c==',') {
                                sub.incr();
                                continue;
                            }
                            if(c=='\0') break;
                            if(arrs==NULL) arrs=getLocalArray(true);
                            if(c=='\'' || c=='\"') {
                                if( sub.MatchStringPos(c) ) {
                                    LPCC str=getTextVarCheck(cn->config(), sub.GetVar(), sub.prev, sub.cur);
                                    if(str) {
                                        arrs->add()->add(str);
                                    } else {
                                        arrs->add()->add(sub.GetVar(), sub.prev, sub.cur);
                                    }
                                    if(sub.ch()==',') sub.incr();
                                } else {
                                    qDebug("#9# web impl function param string match error(func name:%s)\n", fnm);
                                    break;
                                }
                            } else {
                                sub.findEnd(",",FIND_SKIP);
                                XParseVar params(sub.GetVar(),sub.prev, sub.cur);
                                evalVarValue(params, cn, webPageNode, fnInit, arrs->add());
                            }
                        }
                        TreeNode* param=cn->config();
                        XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc,fnInit);
                        if(arrs) setFuncSrcParam(fnCur,arrs,fsrc,0,fnInit);
                        int len=var->length();
                        fnCur->GetVar("req")->setVar('v',1,(LPVOID)cn);
                        fnCur->GetVar("param")->setVar('n',0,(LPVOID)param);
                        fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)param);
                        fnCur->GetVar("@evalResult")->setVar('s',0,(LPVOID)var).addInt(0).addInt(len).addInt(0).addInt(len);
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)webPageNode);
                        fnCur->call();
                        if( ch=='+' && len==var->length() ) {
                            var->add(toString(fnCur->getResult()));
                        }
                        funcNodeDelete(fnCur);
                        releaseLocalArray(arrs);
                    } else {
                        qDebug("#0#web page fuction call error (function:%s)\n", fnm);
                    }
                }
            } else {
                qDebug("#0#web page fuction not match params (function:%s)\n", fnm);
            }
        } else if( ch=='?' ) {
            makeEvalValue(pv.GetVar(), pv.prev, pv.cur, cn, webPageNode, fnInit, var, incName);
        } else {
            qDebug("#9#not define eval case type (%c)\n", ch);
        }