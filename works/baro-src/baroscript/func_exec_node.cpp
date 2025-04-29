#include "func_util.h"

#define NEWVER_USE

char gVarBuf[256];

inline bool setClassFunc(TreeNode* target, StrVar* sv, int sp, int ep, XFuncNode* fn ) {
    if( target==NULL ) return false;
    XParseVar pv(sv, sp, ep);
    XFuncSrc* fsrc=NULL;
    XFuncNode* fnCurInit=NULL;
    int sa=0, ea=0;
    char c=pv.ch();
    while( pv.valid() ) {
        c=pv.ch();
        if( c==0 || c=='>' ) break;
        if( c==';' ) {
            c=pv.incr().ch();
            continue;
        }
        if( c=='/' ) {
            c=pv.ch(1);
            if( c=='/') {
                pv.findEnd("\n");
            } else if( c=='*' ) {
                pv.match("/*", "*/", FIND_SKIP);
            }
            continue;
        }
        LPCC funcNm=NULL, param=NULL;
        bool addMode=false, bEvt=false;
        sa=pv.start;
        c=pv.moveNext().ch();
        if( c=='#' || c=='-' ) {
            c=pv.incr().moveNext().ch();
        }
        ea=pv.start;
        funcNm=pv.Trim(sa,ea);
        if( slen(funcNm)==0 ) {
            qDebug("#9##class function error (name: %s)\n", funcNm);
            return false;
        }
        if( c=='(' && pv.match("(",")") ) {
            param=pv.v();
            c=pv.ch();
        }
        if( c!='{' ) break;
        if( pv.match("{","}",FIND_SKIP)==false ) {
            qDebug("#9##class function match error (name: %s)\n", funcNm);
            return false;
        }
        sv=target->gv(funcNm);
        if( SVCHK('f',1) ) {
            fsrc = (XFuncSrc*)SVO;
        } else if( SVCHK('f',0) ) {
            XFuncNode* fnCur=(XFuncNode*)SVO;
            XFunc* fc=fnCur->xfunc;
            fsrc=fc ? fc->getFuncSrc(): NULL;
        } else {
            fsrc=gfsrcs.getFuncSrc();
            addMode=true;
        }
        if( fsrc ) {
            if( addMode==false  ) {
                fsrc->reuse();
                fsrc->xparam.reuse();
                fsrc->xflag=0;
                if( fsrc->xfunc ) {
                    gfuncs.deleteFunc(fsrc->xfunc);
                    fsrc->xfunc=NULL;
                }
            }
            if( slen(param) ) fsrc->xparam.set(param);
            fsrc->readBuffer(pv.GetVar(), pv.prev, pv.cur);
            if( fsrc->makeFunc() && fsrc->xfunc ) {
                // qDebug("#0## object function make ok (name: %s)", funcNm);
            } else {
                qDebug("#9## class function make error (name: %s)\n", funcNm);
            }
            if( StartWith(funcNm,"on") && slen(funcNm)>2 && isUpperCase(funcNm[2]) ) {
                bEvt=true;
            }
            qDebug("#0# function : %s (%s: %s)\n", funcNm, (bEvt?"event":"func"), (addMode?"add":"modify") );
            if( addMode ) {
                if(bEvt) {
                    XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)target);
                    fnCur->setNodeFlag(FLAG_PERSIST);
                    if(ccmp(funcNm,"onInit")) {
                        fnCurInit=fnCur;
                    }
                } else {
                    target->GetVar(funcNm)->setVar('f',1,(LPVOID)fsrc );
                }
            }
        }
    }
    XFuncNode* fnInit=NULL;
    XFuncNode* fnParent=NULL;
    TreeNode* page=findPageNode(target);
    sv=target->gv("onInit");
    if(SVCHK('f',0)) {
        fnInit=(XFuncNode*)SVO;
    }
    if( page && target!=page ) {
        sv=page->gv("onInit");
        if( SVCHK('f',0) ) {
            fnParent=(XFuncNode*)SVO;
        }
    }
    if( fnCurInit ) {
        qDebug("#0#onInit call start \n" );
        if( fnInit==NULL ) {
            fnInit=fnCurInit;
            target->GetVar("onInit")->setVar('f',0,(LPVOID)fnInit);
            fnCurInit->call();
        } else {
            fnCurInit->call();
            for(WBoxNode* bn=fnCurInit->First(); bn; bn=fnCurInit->Next()) {
                 LPCC vnm=fnCurInit->getCurCode();
                 if(slen(vnm)==0 || vnm[0]=='@' ) continue;
                 sv=fnInit->gv(vnm);
                 if(sv==NULL) {
                     qDebug("#0#onInit var name %s  (status:%s) \n", vnm, sv?"skip":"set");
                     fnInit->GetVar(vnm)->set(bn->value);
                 } else {
                     qDebug("#0#onInit var name %s already setting  !!!\n", vnm);
                 }
            }
            fnCurInit->clearNodeFlag(FLAG_PERSIST);
            funcNodeDelete(fnCurInit);
        }
    }
    if(fnInit) {
        if(fnParent ) {
            fnInit->xparent = fnParent;
        }
        for(WBoxNode* bn=target->First(); bn; bn=target->Next()) {
            sv=&(bn->value);
            if(SVCHK('f',0)) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                if(fnCur!=fnInit) {
                    if(fnCur->xparent && fnCur->xparent!=fnInit) {
                        qDebug("#0# member function reset parent name:%s\n", target->getCurCode());
                    }
                    fnCur->xparent=fnInit;
                }
            }
        }
    }
    return true;
}

void regNodeFuncs() {
    if( isHashKey("node") )
        return;
    PHashKey hash = getHashKey("node");
    U16 uid = 1;
    hash->add("addNode", uid);		uid++;
    hash->add("get", uid);			uid++;
    hash->add("set", uid);			uid++;
    hash->add("pushNode", uid);		uid++;
    hash->add("parentNode", uid);	uid++;
    hash->add("index", uid);		uid++;
    hash->add("checkFlag", uid);	uid++;
    hash->add("reuse", uid);		uid++;
    hash->add("find", uid);			uid++;
    hash->add("filter", uid);		uid++; // 10
    hash->add("addArray", uid);     uid++;
    hash->add("localArray", uid);   uid++;
    hash->add("filter", uid);   uid++;
    hash->add("newNode", uid);   uid++;
    hash->add("newArray", uid);   uid++;
    hash->add("delay", uid);   uid++;
    hash->add("class", uid);   uid++;
    hash->add("inlineCall", uid);   uid++;
    hash->add("copyNode", uid);     uid++;
    uid=20;
    hash->add("child", uid);		uid++;
    hash->add("size", uid);			uid++;
    hash->add("typeof", uid);		uid++;
    hash->add("flag", uid);			uid++;
    hash->add("type", uid);			uid++;
    hash->add("childCount", uid);	uid++; // 25
    hash->add("objects", uid);		uid++;
    uid=30;
    hash->add("removeAll", uid);	uid++;
    hash->add("remove", uid);		uid++;
    hash->add("setNull", uid);		uid++;
    hash->add("incrNum", uid);		uid++;
    hash->add("reset", uid);		uid++;
    uid=90;
    hash->add("parseXml", uid);		uid++;
    hash->add("delete", uid);		uid++;
    hash->add("isVar", uid);        uid++;	// ;
    uid++;  // hash->add("makeXml", uid);
    hash->add("parseJson", uid);	uid++;
    hash->add("sizeFormat", uid);	uid++;
    hash->add("dateFormat", uid);	uid++;
    hash->add("impl", uid);     	uid++;
    hash->add("filter", uid);       uid++;
    hash->add("map", uid);          uid++;
    hash->add("func", uid);			uid++;
    uid=901;
    hash->add("appendText", uid);	uid++;	// hash->add("address", uid);
    hash->add("makeJson", uid);		uid++;
    hash->add("merge", uid);		uid++;
    hash->add("keys", uid);			uid++;
    hash->add("push", uid);			uid++;
    hash->add("toggle", uid);		uid++;
    hash->add("sort", uid);			uid++;
    hash->add("initNode", uid);		uid++;
    hash->add("insertNode", uid);	uid++;
    hash->add("cmp", uid);          uid++;	// 910
	uid=990;
	hash->add("put", uid);			uid++;
	hash->add("with", uid);			uid++;
    hash->add("inject", uid);		uid++;
	hash->add("timeVal", uid);		uid++;
    hash->add("var", uid);			uid++;
	hash->add("memberVar", uid);	uid++;
    hash->add("isset", uid);		uid++;
    hash->add("ref", uid);			uid++;
    hash->add("member", uid);		uid++;
    hash->add("memberFunction", uid);		uid++;
}
inline TreeNode* findNodeTag(TreeNode* root, LPCC tag) {
    for(int n=0,sz=root->childCount(); n<sz; n++) {
        TreeNode* cur=root->child(n);
        if( cur->cmp("tag", tag) ) {
            return cur;
        }
        if( cur->childCount()>0 ) {
            cur=findNodeTag(cur, tag);
            if( cur) return cur;
        }
    }
    return NULL;
}

int parseVarFunc( XParseVar* pv, TreeNode* node, LPCC code, StrVar* rst, XFuncNode* fn, XFunc* fc) {
    if( pv==NULL || fc==NULL ) return 0;
    // if( pv->findPos("(")!=-1 ) return false;
    XFunc* fcParam=NULL;
    if( fc && fc->getParamSize()>2 ) {
        fcParam=fc->getParam(2);
    }
    if( pv->ch()=='@') {
        pv->incr();
    }
    StrVar* var=node->GetVar(code);
    LPCC cmd=cmd=pv->NextWord();
    if( ccmp(cmd,"incr") ) {
        if( isNumberVar(var) ) {
            int num=toInteger(var), n=1;
            if( fcParam ) {
                execParamFunc(fcParam, fn, rst->reuse());
                if( isNumberVar(rst) ) {
                    n=toInteger(rst);
                }
            }
            var->setVar('0',0).addInt(num+n);
            rst->setVar('0',0).addInt(num);
        }
        return 1;
    } else if( ccmp(cmd,"append") ) {
        if( var && fcParam ) {
            execParamFunc(fcParam, fn, rst->reuse());
            var->add(toString(rst));
        }
        return 1;
    } else if( ccmp(cmd,"ref") ) {
        if( checkFuncObject(var,'9',0) ) {
            var->reuse();
        } else if( var->charAt(0)=='\b') {
            rst->reuse();
            return 9;
        }
        int sp=0, ep=var->length();
        rst->setVar('s',0, (LPVOID)var).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
        return 9;
    }
    return 0;
}

bool callNodeFunc(TreeNode* tn, LPCC fnm, PARR arrs, XFuncNode* fn, StrVar* rst, StrVar* var, XFunc* fc) {
    if( tn==NULL || fc==NULL )
        return false;
    if( rst==NULL ) rst = getStrVarTemp();
    if( rst->length()) {
        rst->reuse();
    }
    if( fnm ) {
        int pos=IndexOf(fnm,'.');
        if( pos>0 ) {
            StrVar* sv=NULL;
            bool echk=false;
            for( int n=0;n<8;n++ ) {
                LPCC cd=gBuf.add(fnm,pos);
                sv=getVarValue(tn, cd);
                if( sv==NULL ) {
                    qDebug(">> node call error (%s not define)\n", fc ? fc->getValue(): fnm, cd );
                    return true;
                }
                if( SVCHK('n',0) ) {
                    tn=(TreeNode*)SVO;
                } else {
                    echk=true;
                }
                fnm+=pos+1;
                pos=IndexOf(fnm,'.');
                if( pos<=0 ) break;
            }
            if( echk && objectFunction(fc, fnm, sv, arrs, fn, rst->reuse()) ) {
                // qDebug("... node object check function name:%s\n", fnm);
                return true;
            }
        }

        if( fc->xfuncRef==0 ) {
            if( execMemberFunc(tn->gv(fnm),arrs,rst,fn,tn,fnm) ) {
                releaseLocalArray(arrs);
                return true;
            }
        }
    }
    bool call=false;
    U16 nodeFlag=0x2000, widgetFlag=0x4000|0x8000;
    U16 flag = fc->xfflag;
    /*
    if( flag&0x1000 ) {
        return false;
    } else
    */
    if( flag&nodeFlag ) {
        call=callExecNodeFunc(tn, fc,arrs,fn,var,rst);
        return call;
    } else if( flag&widgetFlag ) {
#ifdef WIDGET_USE
        QWidget* widget=gwidget(tn);
        if( widget ) {
            if( flag&0x8000 ) {
                call=callWidgetAll(fnm, arrs, tn, fn, rst, widget, fc);
            } else if( flag&0x4000 ) {
                call=callWidgetFunc(tn, fc, widget, arrs, fn, rst);
            }
        }
        if(!call ) {
            fc->xfflag=0;
            fc->xfuncRef=0;
            qDebug("#9#[%s] object function call error (name:%s)\n", getBaseFuncName(fn), fnm);
        }
#endif
        return true;
    }
#ifdef WIDGET_USE
    QWidget* widget=NULL;
#endif
    switch(tn->xstat) {
#ifdef WIDGET_USE
    case LTYPE_GRID:
    case LTYPE_ROW:
    case LTYPE_HBOX:
    case LTYPE_VBOX: {
        call=callLayoutFunc(fnm, arrs, tn, fn, rst);
    } break;
    case FNSTATE_PDF:			call=callPdfFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_CANVASITEM:    call=callCanvasItemFunc(fnm, fn, fc, tn->gv("@c"), arrs, rst); break;
	case FNSTATE_DRAW:			call=callDrawFunc(fnm, arrs, tn, fn, rst, fc); break;
#endif
    case FNSTATE_DB:			call=callDbFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_CRON:			call=callCronFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_WORKER:		call=callWorkerFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_PROCESS:		call=callProcessFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_WEBSERVER:		call=callWasFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_WEBCLIENT:		call=callWebFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_FILE:			call=callFileFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_SOCKET:        call=callConnectNodeFunc(fc, arrs, tn, fn, rst); break;
    // case FNSTATE_TIMELINE:        call=callTimelineFunc(fnm, arrs, tn, fn, rst, fc); break;
    case FNSTATE_MESSAGE_SERVER:	call=callMessageServerFunc(fnm, arrs, tn, fn, rst); break;
    case FNSTATE_MESSAGE_CLIENT:	call=callMessageClientFunc(tn->gv("@c"), fc, fnm, arrs, tn, fn, rst); break;
    case FNSTATE_UDP:               call=callUdpFunc(tn->gv("@c"), fc, fnm, arrs, tn, fn, rst); break;
    case FNSTATE_MQTT:              call=callMqFunc(fnm, arrs, tn, fn, rst, fc); break;
#ifdef EXCEL_USE
    case FNSTATE_EXCEL:         call=callExcelFunc(fnm, arrs, tn, fn, rst, fc); break;
#endif
#ifdef QT5_USE
	case FNSTATE_MODBUS:        call=callModbusFunc(fnm, arrs, tn, fn, rst, fc); break;
#endif
    case FNSTATE_ZIP:           call=callZipFunc(fnm, arrs, tn, fn, rst, fc); break;
    default:
#ifdef WIDGET_USE
        widget=gwidget(tn);
#endif
        break;
    }
    if( call ) {
        return true;
    }
#ifdef WIDGET_USE
    if( widget ) {
        /*
        if( tn->xstat==WTYPE_PAGE && callPageFunc(fnm, arrs, tn, fn, rst, NULL, fc) ) {
            fc->xfflag|=0x4000;
            return true;
        } else
        */
        if( callWidgetAll(fnm, arrs, tn, fn, rst, widget, fc) ) {
            fc->xfflag|=0x8000;
            return true;
        } else {
            regWidgetFuncs();
            fc->xfuncRef=getHashKeyVal("widget", fnm);
            if( fc->xfuncRef && callWidgetFunc(tn, fc, widget, arrs, fn, rst) ) {
                fc->xfflag|=0x4000;
                return true;
            }
        }
    }
#endif
    if( fc->xfuncRef==0 ) {
        regNodeFuncs();
        fc->xfuncRef = getHashKeyVal("node", fnm );
        if( fc->xfuncRef ) {
            fc->xfflag|=nodeFlag;
        }
    }
    if( fc->xfuncRef ) {
        call=callExecNodeFunc(tn, fc,arrs,fn,var,rst);
    } else {
        LPCC funcName=fc->getFuncName();
        qDebug("#0#node function error funcName:%s\n", funcName);
        // fc->xfflag|=0x1000;
    }
    return call;
}

inline bool nodeTimeout(TreeNode* node, XFuncSrc* fsrc, int interval, XFuncNode* pfn, PARR arr, int start) {
    if( node==NULL || fsrc==NULL ) {
        return false;
    }
    QEventLoop loop;
    QTimer timer;
    if( interval<=0 ) interval=100;
    timer.setInterval(interval);
    timer.setSingleShot(true);
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    cfVar("@delayLoop")->setVar('e',11, (LPVOID)&loop);
    loop.exec();
    XFunc* pfc=fsrc->xfunc;
    XFuncNode* fn=gfns.getFuncNode(pfc, pfn);
#ifdef INLINENODE_USE
    if( pfc && pfc->xflag & FUNC_INLINE ) {
        fn->GetVar("@inlineNode")->setVar('n',0,(LPVOID)node);
    }
#endif
    if( fn ) {
        if( arr && arr->size()>start ) setFuncSrcParam(fn,arr,fsrc,start,pfn);
        fn->GetVar("@this")->setVar('n',0,(LPVOID)node);
        fn->call();
        funcNodeDelete(fn);
    }
    return timer.isActive();

}
inline TreeNode* objectLoadCheck(LPCC subCode) {
    TreeNode* cur=getTreeNode("page",subCode, false);
    if( cur) return cur;
    cur=getTreeNode("class",subCode, false);
    if( cur) return cur;
    cur=getTreeNode("dialog",subCode, false);
    if( cur) return cur;
    return getTreeNode("main",subCode, false);
}

int nodeInlineSource(TreeNode* node, XFuncNode* fn, StrVar* var, int sp, int ep, StrVar* rst, LPCC curId, LPCC curMode, bool event, bool checkClass, bool checkExtend) {
    TreeNode* temp=getTreeNodeTemp();
    XParseVar pv(var, sp, ep);
    int ret=0;
    char c=pv.ch();
    TreeNode* funcInfo=cfNode("@funcInfo");
    StrVar funcType;
    bool bStatic=false;
    if( ccmp(curMode,"static") ) {
        curMode=NULL;
        bStatic=true;
    }
    bool bMain=false, bAll=false;
    if( ccmp(curId,"@all")) {
        bAll=true;
    } else if( slen(curId)==0 && checkClass ) {
        bMain=true;
    }
    XListArr* arrWidgets=NULL;
    for(int idx=0; idx<1024; idx++  ) {
        if( pv.ch()=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) break;
            c=pv.ch();
        }
        if( c!='<' ) {
            if( idx==0 ) {
                if( bAll ) {
                    setModifyClassFunc(node, var, sp, ep, fn, rst, false, "func");
                } else {
                    parseArrayVar(pv, node, fn, rst);
                }
                return 1;
            }
            break;
        }
        // <!DOCTYPE
        if( pv.ch(1)=='!' ) {
            node->GetVar("html")->set(var, pv.start, ep);
            break;
        }
        int sp=pv.start, start=sp;
        LPCC tag=pv.incr().NextWord();
        pv.setPos(sp);
        if( slen(tag)==0 ) break;
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            if( temp->cmp("mode","break") ) break;
            temp->reuse();
            sp=parseProp(temp, var, pv.prev,pv.cur, PROP_SOURCE );
            if( sp==-1 ) {
                return 0;
            }
            LPCC tagId=temp->get("id");
            LPCC tagMode=temp->get("mode");

            if( ccmp(tagMode,"skip") ) {
                continue;
            }
            if( bAll || bMain ) {
            } else {
                if( !ccmp(curId,tagId) ) {
                    continue;
                }
                if( slen(curMode) && slen(tagMode) && !ccmp(curMode,tagMode) ) {
                    continue;
                }
            }
            if( ccmp(tag,"func") ) {
                if( bMain && slen(tagId) ) {
                    continue;
                } else if( checkClass && event ) {
                    continue;
                }
                LPCC ref=temp->get("ref");
                funcInfo->set("ref", ref);
                setFuncSource(var, sp, pv.cur, "common");
            } else if( ccmp(tag,"conf") ) {
                if( slen(tagId) ) {
                    TreeNode* cf=NULL;
                    StrVar* sv=confVar(NULL);
                    if( SVCHK('n',0) ) {
                        cf=(TreeNode*)SVO;
                        cf->GetVar(tagId)->set(var, sp, pv.cur);
                    }
                }
            } else if( ccmp(tag,"call") ) {
                TreeNode* callNode=cfNode("@callVars", true);
                StrVar* src=callNode->GetVar("@src");
                StrVar baseCode;
                baseCode.set(funcInfo->get("baseCode"));
                src->set(var, sp, pv.cur);
                callScriptFunc(src, 0, src->length() );
                // qDebug("-------------------- prev base code: %s --------------------\n",funcInfo->get("baseCode"));
                funcInfo->set("baseCode", baseCode.get());
                // qDebug("-------------------- cur base code: %s --------------------\n",baseCode.get());
            } else if( ccmp(tag,"define") ) {
                setBaroDefine(var, sp, pv.cur, node);
            } else if( ccmp(tag,"pages") || ccmp(tag,"widgets") || ccmp(tag,"page")  || ccmp(tag,"dialog") || ccmp(tag,"main") ) {
                if( bMain && slen(tagId)) {
                    continue;
                }
                if(arrWidgets==NULL) arrWidgets=new XListArr();
                baroSourceApply(var, start, pv.start, NULL, arrWidgets );
            } else  {
                TreeNode* object=NULL;
                bool bClass=checkClass;
                StrVar* sv=NULL;
                LPCC baseCode=funcInfo->get("baseCode"), fileCode=NULL;
                if( slen(tagId)==0) tagId="main";
                if( slen(baseCode) ) {
                    int pos=IndexOf(baseCode,':');
                    if(pos>0) {
                        fileCode=gBuf.add(baseCode,pos);
                    }
                }
                // qDebug(">>> %s %s %s (%s, %s), %s, %s", tag, tagId, tagMode, curId, curMode, baseCode, fileCode);
                LPCC subCode=FMT("%s:%s", fileCode, tagId);
                if(slen(fileCode) && arrWidgets ) {
                    TreeNode* sub=NULL;
                    for(int n=0, sz=arrWidgets->size(); n<sz; n++) {
                        sv=arrWidgets->get(n);
                        if( SVCHK('n',0)) {
                            sub=(TreeNode*)SVO;
                            if( sub->cmp("@baseCode", subCode)) {
                                bClass=false;
                                object=sub;
                                break;
                            }
                        }
                    }
                }
                if( bMain ) {
                   if( object==NULL) {
                       if( objectLoadCheck(subCode) ) {
                           qDebug("... %s already loaded ...", subCode);
                           continue;
                       }
                       bClass=true;
                       object=getTreeNode("class",subCode, true );
                       if( object->isset("tag")) object->set("tag", "class");
                   }
               } else if(object==NULL) {
                    object=node;
               }

               if( ccmp(tag,"inline") || ccmp(tag,"event") ) {
                    funcType.set(bStatic?"static":bClass?"class":"func");
                    if( checkExtend ) {
                        funcType.add(".").add(curId);
                    }
                    if( setModifyClassFunc(object, var, sp, pv.cur, fn, rst, false, funcType.get()) ) {
                        ret++;
                    }
               }  else if( ccmp(tag,"widgetAdd") ) {
                   int cnt=temp->incrNum(FMT("@%sCount",tag));
                   StrVar* sv=object->GetVar(FMT("@%s",tag));
                   if(cnt==0) sv->reuse();
                   sv->add(var,start, pv.start);
               } else {
                   XParseVar pvCur(var, sp, pv.cur );
                   int cnt=temp->incrNum(FMT("@%sCount",tag));
                   StrVar* sv=NULL;
                   if( pvCur.ch()=='<' ) {
                       sv=object->GetVar(tag); if(cnt==0) sv->reuse();
                       sv->add(var, sp, pv.cur);
                       continue;
                   }
                   XParseVar sub;
                   LPCC code=NULL;
                   pvCur.findEnd("#>",FIND_SKIP );
                   sub.SetVar(var, sp, pvCur.cur);
                   if( sub.ch() ) {
                       sv=object->GetVar(tag); if(cnt==0) sv->reuse();
                       sv->add(var, sp, pvCur.cur);
                   }
                   while( pvCur.valid() ) {
                       pvCur.findEnd("\n");
                       sub.SetVar(var, pvCur.prev, pvCur.cur);
                       code=sub.findEnd("[").v();
                       pvCur.findEnd("#>",FIND_SKIP );
                       if( slen(code)==0 ) continue;
                       sv=object->GetVar(FMT("%s#%s",tag,code));
                       if(cnt==0) sv->reuse();
                       sv->add(var, pvCur.prev, pvCur.cur);
                   }
                   ret++;
               }
            }
        } else {
            qDebug("#9#inline Source tag %s not match", tag);
            return 0;
        }
    }
    if( arrWidgets ) {
        for( int n=arrWidgets->size()-1; n>=0; n-- ) {
            StrVar* sv=arrWidgets->get(n);
            if( SVCHK('n',0) ) {
                TreeNode* cur=(TreeNode*)SVO;
                // qDebug("#0#------------------ init function call id:%s ---------------------------\n", cur->get("id"));
                callInitFunc(cur);
            }
        }
        SAFE_DELETE(arrWidgets);
    }
    return ret;
}
bool callExecNodeFunc(TreeNode* node, XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* refVar, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;

    if( node==NULL )
        return false;
    switch(fc->xfuncRef) {
    case 1: {	// addNode
        if( cnt==0 ) {
            TreeNode* cur=node->addNode();
            if( cur ) {
                int depth=node->getInt("@depth");
                cur->GetVar("@depth")->setVar('0',0).addInt(depth+1);
                rst->setVar('n',0,(LPVOID)cur );
            }
            return true;
        }
        bool overwrite=false;
        StrVar* sv=arrs->get(0);
        TreeNode* cur=NULL;
        TreeNode* src=NULL;
        if( SVCHK('n',0) ) {
            // addNode(node);
            src=(TreeNode*)SVO;
            sv=arrs->get(1);
            if( SVCHK('3',1) || SVCHK('3',0) ) {
                cur=node->addNode();
                if( SVCHK('3',1)) overwrite=true;
            } else {
                if( node->find(src)==-1 ) {
                    node->xchilds.add(src);
                }
                return true;
            }
        } else {
            // addNode(code) addNode(code, true) or addNode(code, node, true);
            LPCC code=AS(0);
            if( slen(code) ) {
                sv=arrs->get(1);
                if(SVCHK('3',1)|| SVCHK('3',0)) {
                    cur=node->addNode(code, SVCHK('3',1));
                    sv=arrs->get(2);
                } else {
                    cur=node->addNode(code);
                    if( SVCHK('n',0) ) {
                       src=(TreeNode*)SVO;
                       sv=arrs->get(2);
                       if(SVCHK('3',1)) overwrite=true;
                    }
                }
            }
        }
        if( cur && src) {
            cur->clone(src, overwrite);
        }
        if( cur ) {
            sv=node->gv("@depth");
            if( SVCHK('0',0) ) {
                cur->setInt("@depth", toInteger(sv)+1);
            }
            rst->setVar('n',0,(LPVOID)cur );
        }
    } break;
    case 2: { // get
        if( cnt==0 ) return false;
        StrVar* sv = arrs->get(0);
        if( isNumberVar(sv) ) {
            int idx=toInteger(sv);
            int last=node->getInt("@lastIndex");
            rst->reuse();
            if(idx<last) {
                int num=0, cnt=node->childCount();
                for(int n=0; n<cnt ; n++ ) {
                    TreeNode* cur=node->xchilds.getvalue(n);
                    if(!cur->isNodeFlag(FLAG_VARCHECK)) {
                        if(idx==num++ ) {
                            rst->setVar('n',0,(LPVOID)cur);
                            break;
                        }
                    }
                }
            } else {
                qDebug("node get function index error(%d >= %d)",idx, last);
            }
            return true;
        }
        LPCC cd=AS(0);
        sv=node->gv(cd);
        rst->reuse();
#ifdef WIDGET_USE
        if( sv==NULL || sv->length()==0 ) {
            if( isPageNode(node) ) {
                TreeNode* cur=findWidgetId(node, cd);
                if( cur==NULL ) {
                    cur=findNodeTag(node,cd);
                }
                if( cur ) {
                    rst->setVar('n',0,(LPVOID)cur);
                } else {
                    sv=node->gv("onInit");
                    if( SVCHK('f',0) ) {
                        XFuncNode* fnCur=(XFuncNode*)SVO;
                        sv=fnCur->gv(cd);
                    }
                }
            }
        }
#endif
        // (jwas)
        if( sv ) {
            rst->add(sv);
        } else {
            sv=node->gv("onInit");
            if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                sv=fnCur->gv(cd);
                rst->add(sv);
            }
        }
        // rst->reuse()->add( getVarValue(node, var) ); // getSubVarValue(node, var)
    } break;
    case 3: { // set
        // ### version 1.0.4
        if(cnt==0 ) {
            return true;
        }       
        LPCC code=AS(0);
        rst->reuse();
        if( slen(code)) {
            int sp=0,ep=0;
            StrVar* sv=getStrVarPointer(arrs->get(1),&sp,&ep);
            StrVar* var=node->GetVar(code)->reuse();
            if( sp<ep ) {
                var->add(sv, sp, ep);
                rst->add(sv, sp, ep);
            }
        }
    } break;
    case 4: { // pushNode
        if( cnt==0 )
            return true;
        StrVar* sv=arrs->get(0);
        if(SVCHK('n',0) ) {
           TreeNode* cur=(TreeNode*)SVO;
           node->addChild(cur);
        }
    } break;
    case 5: { // parentNode
        if( cnt==0) {
            TreeNode* p = node->parent();
            if( p )
                rst->setVar('n',0,(LPVOID)p);
            else
                rst->setVar('3',0);
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* parent=(TreeNode*)SVO;
            node->xparent=parent;
            sv=arrs->get(1);
            if( SVCHK('3',1)) {
                sv=parent->gv("onInit");
                if(SVCHK('f',0)) {
                    XFuncNode* fnParent=(XFuncNode*)SVO;
                    sv=node->gv("onInit");
                    if( SVCHK('f',0) ) {
                        XFuncNode* fnCur=(XFuncNode*)SVO;
                        fnCur->xparent=fnParent;
                    } else {
                        qDebug("#0#node funcNode not define\n");
                    }
                } else {
                    qDebug("#0#parent funcNode not define\n");
                }
            }
            rst->setVar('n',0,(LPVOID)parent);
        }
    } break;
    case 6: { // index
        if( cnt==0 )
            rst->setVar('0',0).addInt(node->row());
    } break;
    case 7: { // checkFlag
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(node->xflag);
            return true;
        }
        U32 flag = (U32)toInteger(arrs->get(0));
        if( cnt==1 ) {
            if( flag ) {
                rst->setVar('3',node->isNodeFlag((U32)flag) ? 1: 0);
            } else {
                rst->setVar('3',0);
            }
        } else if( cnt==2 ) {
            bool chk=isVarTrue(arrs->get(1));
            if( chk )
                node->setNodeFlag(flag);
            else
                node->clearNodeFlag(flag);
        } else if( cnt==3 ) {
            // (ex) state( 12, true, true);
            node->xflag=flag;
        }
    } break;
    case 8: { // reuse
        if(cnt==0) {
            node->reuseAll(false);
        } else {
            StrVar* sv=arrs->get(0);
            if( SVCHK('3',1) ) {
                node->reuseAll(true);
            } else {
                node->reuseAll(false);
            }
        }
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 9: { // find
        if( cnt==0 ) {
            rst->setVar('3',0);
            return false;
        }
        StrVar* sv=arrs->get(0);
        rst->reuse();
        if( SVCHK('n',0) ) {
            TreeNode* cur=(TreeNode*)SVO;
            int pos=node->find(cur);
            bool bchk= isTrueCheck(fc);
            if(pos!=-1) {
                if( bchk ) {
                    rst->setVar('3',1);
                } else {
                    rst->setVar('0',0).addInt(pos);
                }
            } else {
                if( bchk ) {
                    rst->setVar('3',0);
                } else {
                    rst->setVar('0',0).addInt(pos);
                }
            }
            return true;
        }
        StrVar* obj=NULL;
        TreeNode* cur=NULL;
        LPCC type=NULL, val=NULL;
        if(cnt==1) {
            type="id", val=AS(0);
        } else {
            char ch=0;
            type=AS(0);
            sv=arrs->get(1);
            ch=sv->charAt(0);
            if(ch!='\b' || SVCHK('s',0) ) {
                val=toString(sv);
            } else if(ch=='\b') {
                obj=sv;
            }
        }
        if( slen(type)==0 ) {
            return true;
        }
        if(obj) {
            char c=obj->charAt(1);
            if( isNumberVar(obj)) {
                for(int n=0,cnt=node->childCount();n<cnt;n++) {
                    sv=node->child(n)->gv(type);
                    if(sv==NULL) {
                        continue;
                    }
                    if(c=='4') {
                        if(toDouble(obj)==toDouble(sv)) {
                            cur=node->child(n);
                            break;
                        }
                    } else if(c=='1') {
                        if(toUL64(obj)==toUL64(sv)) {
                            cur=node->child(n);
                            break;
                        }
                    } else if(toInteger(obj)==toInteger(sv)) {
                         cur=node->child(n);
                         break;
                    }
                }
            } else {
                int sz=sizeof(double);
                U16 stat=obj->getU16(2);
                for(int n=0,cnt=node->childCount();n<cnt;n++) {
                    sv=node->child(n)->gv(type);
                    if(sv==NULL) {
                        continue;
                    }
                    if(c=='i' ) {
                        if(stat==4||stat==5) {
                            if(SVCHK('i',4)||SVCHK('i',5)) {
                                QRectF r0=getQRectF(obj), r1=getQRectF(sv);
                                if(r0==r1 ) {
                                    cur=node->child(n);
                                    break;
                                }
                            }
                        } else if( stat==20 ) {
                            double x=obj->getDouble(4), y=obj->getDouble(4+sz);
                            if(SVCHK('i',2)||SVCHK('i',20)) {
                                if(SVCHK('i',2)) {
                                    double x1=(double)sv->getInt(4), y1=(double)sv->getInt(8);
                                    if(x==x1 && y==y1 ) {
                                        cur=node->child(n);
                                        break;
                                    }
                                } else {
                                    double x1=sv->getDouble(4), y1=sv->getDouble(4+sz);
                                    if(x==x1 && y==y1 ) {
                                        cur=node->child(n);
                                        break;
                                    }
                                }
                            }
                        } if( stat==2 ) {
                            int x=obj->getInt(4), y=obj->getInt(8);
                            if(SVCHK('i',2)||SVCHK('i',20)) {
                                if(SVCHK('i',2)) {
                                    int x1=sv->getInt(4), y1=sv->getInt(8);
                                    if(x==x1 && y==y1 ) {
                                        cur=node->child(n);
                                        break;
                                    }
                                } else {
                                    int x1=(int)sv->getDouble(4), y1=(int)sv->getDouble(4+sz);
                                    if(x==x1 && y==y1 ) {
                                        cur=node->child(n);
                                        break;
                                    }
                                }
                            }
                        }
                    } else if(c==sv->charAt(0) && stat==sv->getU16(2) ) {
                        LPVOID aa=obj->getObject(FUNC_HEADER_POS), ab=SVO;
                        if(aa==ab) {
                            cur=node->child(n);
                            break;
                        }
                    }
                }
            }
        } else {
            for(int n=0,cnt=node->childCount();n<cnt;n++) {
                LPCC str=toString(node->child(n)->gv(type));
                if(ccmp(val,str)) {
                    cur=node->child(n);
                    break;
                }
            }
        }
        if(cur) {
            rst->setVar('n',0,(LPVOID)cur);
        }

    } break;
    case 10: { // filter
        if(cnt==0) {
           return true;
        }
        LPCC find=AS(0), ty=NULL;
        if(cnt==1) {
            ty="regexp";
        } else {
            ty=AS(1);
        }
        XListArr* a=node->addArray("@keys",true);
        if(ccmp(ty,"regexp")) {
            QRegExp regexp(KR(find));
            for( WBoxNode* bn = node->First(); bn; bn=node->Next() ) {
                LPCC code = node->getCurCode();
                int pos=KR(code).indexOf(regexp);
                if(pos!=-1) {
                    a->add()->add(node->GetVar(code));
                }
            }
        } else {
            for( WBoxNode* bn = node->First(); bn; bn=node->Next() ) {
                LPCC code = node->getCurCode();
                if(ccmp(ty,"start")) {
                    if(StartWith(code,ty)) {
                        a->add()->add(node->GetVar(code));
                    }
                } else if(ccmp(ty,"match")) {
                    rst->set(code);
                    if( rst->findPos(find)!=-1) {
                        a->add()->add(node->GetVar(code));
                    }
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 11: {	// addArray
        LPCC code= arrs? AS(0): NULL;
        XListArr* a=NULL;
        if( slen(code) ) {
            StrVar* sv=arrs->get(1);
            bool reuse=SVCHK('3',1);
            a=node->addArray(code,reuse);
        } else {
            a=node->addArray(NULL);
            // a=getListArrTemp();
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 12: {	// localArray
        if( cnt==0) {
            int size=node->xarrs.size();
            rst->setVar('0',0).addInt(size);
        } else {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                int idx=toInteger(sv);
                rst->setVar('a',0,node->xarrs.getvalue(idx));
            } else if(SVCHK('3',1)) {
                XListArr* a=getListArrTemp();
                int size=node->xarrs.size();
                for(int n=0;n<size;n++) {
                    a->add()->setVar('a',0,(LPVOID)node->xarrs.getvalue(n));
                }
                rst->setVar('a',0,(LPVOID)a);
            } else {
                LPCC ty=toString(sv);
                if(ccmp(ty,"reuse")) {
                    node->xarrs.reuse();
                } else if(ccmp(ty,"reset")) {
                    int size=node->xarrs.size();
                    for(int n=0;n<size;n++) {
                        XListArr* a=node->xarrs.getvalue(n);
                        for(int x=0;x<a->size();x++) {
                            a->get(n)->close();
                        }
                        a->reuse();
                    }
                }
            }
        }
    } break;
    case 13: {	// filter
        if( cnt==0) return true;
        StrVar* sv=NULL;
        XListArr* a=node->addArray("@filterArray", true);
        LPCC ty=AS(0);
        for( WBoxNode* bn=node->First(); bn; bn=node->Next() ) {
            LPCC name=node->getCurCode();
            sv=node->gv(name);
            if( ccmp(ty,"node") || ccmp(ty,"widget") ) {
                if(SVCHK('n',0)) {
                    if(ccmp(ty,"widget")) {
                        if( gwidget((TreeNode*)SVO)!=NULL ) a->add()->setVar('x',21).add(name).add('\f').add(sv);
                    } else {
                        a->add()->setVar('x',21).add(name).add('\f').add(sv);
                    }
                }
            } else if(ccmp(ty,"array")) {
                if(SVCHK('a',0)) {
                    a->add()->setVar('x',21).add(name).add('\f').add(sv);
                }
            } else if(ccmp(ty,"num") || ccmp(ty,"number") ) {
                if(isNumberVar(sv)) {
                    a->add()->setVar('x',21).add(name).add('\f').add(sv);
                }
            } else if(ccmp(ty,"bool")) {
                if(SVCHK('3',1)|| SVCHK('3',0) ) {
                    a->add()->setVar('x',21).add(name).add('\f').add(sv);
                }
            } else if(ccmp(ty,"null")) {
                if(SVCHK('9',0) ) {
                    a->add()->setVar('x',21).add(name).add('\f').add(sv);
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 14: {	// newNode
        rst->setVar('n',0,(LPVOID)node->addNode(NULL));
    } break;
    case 15: {	// newArray
        rst->setVar('a',0,(LPVOID)node->addArray(NULL));
    } break;
    case 16: {	// delay
        StrVar* sv=NULL;
        if(cnt==0 ) {
            sv=cfVar("@delayLoop");
            if( sv && sv->length() ) {
                rst->add(sv);
            }
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int interval=toInteger(sv);
            sv=arrs->get(1);
            if( SVCHK('f',1)) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                nodeTimeout(node, fsrc, interval, fn, arrs, 2);
            } else {
                qDebug("#9#[node.delay] callback function not define (interval:%d) xxx", interval);
            }
        }
    } break;
    case 17: {	// class
        if( cnt==0) {
            return true;
        }
        TreeNode* base=NULL;
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            base=(TreeNode*)SVO;
        }

        if( base ) {
            XFuncNode* fnInit=NULL;
            XFuncSrc* fsrcInit=NULL;
            XListArr* arrMember=NULL;
            sv=arrs->get(1);
            if( sv) {
                int sp=0,ep=0;
                sv=getStrVarPointer(sv,&sp,&ep);
                if( sp<ep) {
                    XParseVar pv(sv,sp,ep);
                    for(int n=0; n<256 && pv.valid(); n++ ) {
                        LPCC code=pv.findEnd(",").v();
                        if( slen(code) ) {
                            if(arrMember==NULL) arrMember=getListArrTemp();
                            arrMember->add()->add(code);
                        }
                    }
                }
            }
            for( WBoxNode* bn=base->First(); bn; bn=base->Next() ) {
                LPCC name=base->getCurCode();
                if( ccmp(name,"tag") || ccmp(name,"id") || ccmp(name,"@this") || ccmp(name,"@source") || ccmp(name,"@baseCode") || ccmp(name,"@c") ) {
                    continue;
                }
                if( arrMember ) {
                    bool ok=false;
                    for(int n=0, sz=arrMember->size(); n<sz; n++ ) {
                        LPCC nameCheck=arrMember->get(n)->get();
                        if(ccmp(name,nameCheck) ) {
                            ok=true;
                            break;
                        }
                    }
                    if(ok==false) continue;
                }
                XFuncSrc* fsrc=NULL;
                sv=&(bn->value);
                if( ccmp(name,"onInit") ) {
                    if(SVCHK('f',0)) {
                        fnInit=(XFuncNode*)SVO;
                    }
                } else {
                    if( SVCHK('f',1) ) {
                        fsrc=(XFuncSrc*)SVO;
                        if(ccmp(name,"init")) {
                            fsrcInit=fsrc;
                        }
                        node->GetVar(name)->setVar('f',1,(LPVOID)fsrc);
                    } else if( SVCHK('f',0) ) {
                        XFuncNode* fnCur=(XFuncNode*)SVO;
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
                        fnCur->setNode(FLAG_PERSIST);
                        node->GetVar(name)->setVar('f',0,(LPVOID)fnCur );
                        node->setNodeFlag(FLAG_SETEVENT);
                    }
                }
            }
            if(fnInit) {
                XFuncNode* fnCur=NULL;
                sv=node->gv("onInit");
                if(SVCHK('f',0)) {
                    fnCur=(XFuncNode*)SVO;
                    fnInit->xparent=fn;
                    fnCur->GetVar("funcNode")->setVar('f',0,(LPVOID)fn);
                    fnInit->GetVar("@this")->setVar('n',0,(LPVOID)node);
                    fnInit->call();
                    fnCur->GetVar("funcNode")->reuse();
                    fnInit->xparent=NULL;
                    for(WBoxNode* bn=fnInit->First(); bn; bn=fnInit->Next() ) {
                        LPCC code=fnInit->getCurCode();
                        if(ccmp(code,"@this") ) continue;
                        sv=&(bn->value);
                        fnCur->GetVar(code)->reuse()->add(sv);
                    }
                } else {
                    XFuncSrc* fsrc=fnInit->getFuncSrc();
                    fnCur= gfns.getFuncNode(fsrc->xfunc, fn);
                    fnCur->GetVar("funcNode")->setVar('f',0,(LPVOID)fn);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
                    fnCur->call();
                    fnCur->GetVar("funcNode")->reuse();
                    fnCur->xparent=NULL;
                    node->GetVar("onInit")->setVar('f',0,(LPVOID)fnCur);
                }
                for(WBoxNode* bn=node->First(); bn; bn=node->Next() ) {
                    sv=&(bn->value);
                    if(SVCHK('f',0) ) {
                        XFuncNode* fnSub=(XFuncNode*)SVO;
                        if(fnSub!=fnCur) {
                            fnCur->xparent=fnInit;
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
            rst->setVar('n',0,(LPVOID)node);
        } else {
            // class function add/modify
            LPCC name=AS(0), curId=AS(1), curMode=AS(2);
            sv=arrs->get(3);
            bool extend=SVCHK('3',1);
            LPCC fileName=getFullFileName(name);
            QFileInfo fi(fileName);
            int num=0;
            if( ccmp(curMode,"class") ) {
                curMode=NULL;
            }

            qDebug("#0#class source info(file:%s, id:%s)\n", fileName, curId);
            if( fi.isFile() ) {
                QFile file(KR(fileName));
                bool ok=true;
                TreeNode* cf=cfNode("@includeNode");
                UL64 modifyDate=(UL64)fi.lastModified().toLocalTime().toTime_t(), prev=cf->getLong(name);
                if( prev && modifyDate==prev) {
                    ok=false;
                }
                if( ok ) {
                    if( file.open(QIODevice::ReadOnly)) {
                        QByteArray ba=file.readAll();
                        TreeNode* funcInfo=cfNode("@funcInfo");
                        LPCC baseCode=funcInfo->get("baseCode");
                        if(slen(baseCode)==0 ) {
                            int pos=IndexOf(name,'/');
                            if(pos>0) {
                                name+=pos+1;
                            }
                            pos=LastIndexOf(name,'.');
                            if(pos>0) {
                                baseCode=gBuf.add(name,pos);
                                funcInfo->set("baseCode", baseCode);
                            }
                        }

                        funcInfo->set("includeFileName", fileName);
                        sv=getStrVar(NULL, NULL);
                        sv->set(ba.constData(), ba.size() );
                        nodeInlineSource(node, fn, sv, 0, sv->length(), rst, curId, curMode, false, true, extend );
                        funcInfo->set("includeFileName", "");
                        file.close();
                    } else {
                        qDebug("#9#class file open error (fileName:%s)", fileName);
                    }

                } else {
                    qDebug("#0#class already setting (fileName:%s)", fileName);
                }
            }
            rst->setVar('0', 0).addInt(num);
        }
    } break;
    case 18: { // inlineCall
        if(cnt==0) {
            return true;
        }
        XFuncSrc* fsrc=NULL;
        StrVar* sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
        } else {
            LPCC fnm=AS(0);
            sv=node->gv(fnm);
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
        }
        if( fsrc ) {
            XFuncNode* fnInit=NULL;
            TreeNode* page=NULL;
#ifdef WIDGET_USE
            page=findPageNode(node);
#endif
            if(page) {
                sv=page->gv("onInit");
                if(SVCHK('f',0)) fnInit=(XFuncNode*)SVO;
            }
            XFunc* func=fsrc->xfunc;
            if(func) {
                XFuncNode* fnCur=gfns.getFuncNode(func, fnInit);
                setFuncSrcParam(fnCur,arrs,fsrc,1,fn);
                qDebug("----- inlineCall -------- param size:%d onInit:%s\n\n", arrs->size(), fnInit?"true":"false" );
                func->xflag|=FUNC_INLINE;
                fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)node);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
                fnCur->call(func, rst);
                funcNodeDelete(fnCur);
            } else {
                rst->setVar('3',0);
            }
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 19: { // copyNode
        if(cnt==0) {
            return true;
        }
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* cur=(TreeNode*)SVO;
            sv=arrs->get(1);
            if(SVCHK('3',1)) {
                node->clone(cur, true);
            } else if(SVCHK('a',0)) {
                XFuncNode* fnInit=NULL;
                sv=node->gv("onInit");
                if(SVCHK('f',0)) {
                    fnInit=(XFuncNode*)SVO;
                }
                XListArr* a=(XListArr*)SVO;
                for(int n=0; n<a->size() ;n++) {
                    LPCC fnm=toString(a->get(n));
                    sv=cur->gv(fnm);
                    if( SVCHK('f',1) ) {
                        XFuncSrc* fsrc=(XFuncSrc*)SVO;
                        if(fsrc->xflag & FLAG_REF ) {
                            sv=node->gv(fnm);
                            if(sv==NULL ) {
                                node->GetVar(fnm)->setVar('f',1,(LPVOID)fsrc);
                            }
                        } else {
                            node->GetVar(fnm)->setVar('f',1,(LPVOID)fsrc);
                        }
                    } else if(SVCHK('f',0) ) {
                        XFuncNode* fnPrev=(XFuncNode*)SVO;
                        XFuncNode* fnCur=setCallbackFunction(fnPrev->getFuncSrc(),fnInit,node);
                        node->GetVar(fnm)->setVar('f',0,(LPVOID)fnCur);
                        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
                        fnCur->setNodeFlag(FLAG_PERSIST);
                    }
                }
            } else if(sv) {
                LPCC ty=toString(sv);
                if(ccmp(ty,"form")) {
                    for( int n=0; n<cur->xchilds.size(); n++ ) {
                        TreeNode* row=cur->xchilds.getvalue(n);
                        TreeNode* rowNode=node->addNode(NULL);
                        //  sub->clone(src, true);
                        for( WBoxNode* bn=row->First(); bn; bn=row->Next() ) {
                            LPCC code=row->getCurCode();
                            if(code[0]=='@') continue;
                            sv=row->gv(code);
                            if(SVCHK('n',0)) {

                            } else {
                                rowNode->GetVar(code)->reuse()->add(sv);
                            }
                        }
                        for( int m=0; m<row->xchilds.size(); m++ ) {
                            TreeNode* cell=row->xchilds.getvalue(m);
                            TreeNode* cellNode=rowNode->addNode(NULL);
                            for( WBoxNode* bn=cell->First(); bn; bn=cell->Next() ) {
                                LPCC code=cell->getCurCode();
                                if(code[0]=='@') continue;
                                sv=cell->gv(code);
                                if(SVCHK('n',0)) {

                                } else {
                                    cellNode->GetVar(code)->reuse()->add(sv);
                                }
                            }
                        }
                    }
                    for( WBoxNode* bn=cur->First(); bn; bn=cur->Next() ) {
                        LPCC code=cur->getCurCode();
                        if(code[0]=='@') continue;
                        sv=cur->gv(code);
                        if(SVCHK('n',0)) {

                        } else {
                            node->GetVar(code)->reuse()->add(sv);
                        }
                    }
                } else if(ccmp(ty,"class")) {
                    XFuncNode* fnInit=NULL;
                    bool evetCheck=false, initCheck=false;
                    sv=node->gv("onInit");
                    if(SVCHK('f',0)) {
                        fnInit=(XFuncNode*)SVO;
                    }
                    for(WBoxNode* bn=cur->First(); bn; bn=cur->Next()) {
                        sv=&(bn->value);
                        if( SVCHK('f',1) ) {
                            XFuncSrc* fsrc=(XFuncSrc*)SVO;
                            LPCC fnm=cur->getCurCode();
                            if(fsrc->xflag & FLAG_REF ) {
                                sv=node->gv(fnm);
                                if(sv==NULL ) {
                                    node->GetVar(fnm)->setVar('f',1,(LPVOID)fsrc);
                                }
                            } else {
                                node->GetVar(fnm)->setVar('f',1,(LPVOID)fsrc);
                            }
                        } else if(SVCHK('f',0) ) {
                            LPCC fnm=cur->getCurCode();
                            if( ccmp(fnm,"onInit")) {
                                initCheck=true;
                            } else {
                                XFuncNode* fnPrev=(XFuncNode*)SVO;
                                XFuncNode* fnCur=setCallbackFunction(fnPrev->getFuncSrc(),fnInit,node);
                                node->GetVar(fnm)->setVar('f',0,(LPVOID)fnCur);
                                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
                                fnCur->setNodeFlag(FLAG_PERSIST);
                            }
                            evetCheck=true;
                        }
                    }
                    if( !node->isNodeFlag(FLAG_SETEVENT)) {
                        node->setNodeFlag(FLAG_SETEVENT);
                    }
                    if( initCheck ) {
                        // qDebug("#0#copyNode class onInit function duplication (id=%s)", node->get("id") );
                    }
                    rst->reuse();
                    if( fnInit ) {
                        if(!fnInit->isNodeFlag(FLAG_INIT) ) {
                            if(fnInit->xfunc ) {
                                fnInit->GetVar("@this")->setVar('n',0,(LPVOID)node);
                                fnInit->call();
                            }
                            fnInit->setNodeFlag(FLAG_INIT);
                        }
                        rst->setVar('f',0,(LPVOID)fnInit);
                    } else {
                        qDebug("onInit function not set (id=%s) !!! ", node->get("id") );
                    }
                }
            } else {
                node->clone(cur, false);
            }
        }
    } break;
    case 20: { // child
        if( node && cnt>0 && isNumberVar(arrs->get(0)) ) {
            int idx = toInteger(arrs->get(0));
            if( idx<0 ) idx = node->childCount()+idx;
            TreeNode* child = node->child(idx);
            if( child ) {
                rst->setVar('n',0,(LPVOID)child);
                return true;
            }
        }
        rst->setVar('3',0);
    } break;
    case 21: {	// size
        rst->setVar('0',0).addInt(node->getInt("@lastIndex"));
    } break;
    case 25: {	// childCount
        rst->setVar('0',0).addInt(node->childCount());
    } break;
    case 22: { // typeof
        if( cnt==0 ) {
            return true;
        }
        LPCC code=AS(0), ty=AS(1);
        if( !getTypeOfResult(ty, node->gv(code), rst, NULL) ) {
            rst->setVar('3',0);
        }
    } break;
    case 23: { // flag
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(node->xflag);
            return true;
        }
        U32 flag = (U32)toInteger(arrs->get(0));
        if( cnt==1 ) {
            if( flag ) {
                rst->setVar('3',node->isNodeFlag((U32)flag) ? 1: 0);
            } else {
                rst->setVar('3',0);
            }
        } else if( cnt==2 ) {
            bool chk=isVarTrue(arrs->get(1));
            if( chk )
                node->setNodeFlag(flag);
            else
                node->clearNodeFlag(flag);
        } else if( cnt==3 ) {
            // (ex) state( 12, true, true);
            node->xflag=flag;
        }
    } break;
    case 24: { // type
        if( cnt==0 ) {
            rst->setVar('0',0).addInt((int)node->xstat);
        } else {
            node->xstat = (U16)toInteger(arrs->get(0));
        }
    } break;
    case 26: { // objects
        if( cnt==0 ) {
            XListArr* a=getListArrTemp();
            for(int n=0,cnt=node->xarrs.size(); n<cnt; n++ ) {
                a->add()->setVar('a',0,(LPVOID)node->xarrs.getvalue(n) );
            }
            rst->setVar('a',0,(LPVOID)a);
        } else {

        }
    } break;

    case 30: {	// removeAll
        StrVar* sv=arrs ? arrs->get(0): NULL;
        node->reset(SVCHK('3',1));
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 31: {	// remove
        bool check=false;
        if( cnt==0 ) {
            node->reuseAll();
        } else {
            StrVar* sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                TreeNode* removeNode=(TreeNode*)SVO;
                if( node->remove(removeNode) ) {
                    rst->setVar('3', 1);
                    if( isVarTrue(arrs->get(1)) ) {
                        removeNode->reset(true);
                        if( removeNode->isNodeFlag(FLAG_NEW) ) {
                            qDebug("remove node ok ~~~");
                            SAFE_DELETE(removeNode);
                        }
                    }
                    check=true;
                }
            } else if( SVCHK('3',1) ) {
                if( node->isNodeFlag(FLAG_NEW) ) {
                    qDebug("remove self node ok ~~~");
                    SAFE_DELETE(node);
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',1);
                }
            }
        }
        rst->setVar('3', check? 1: 0);
    } break;
    case 32: {	// setNull
        for( int n=0,sz=fc->getParamSize();n<sz;n++ ) {
            XFunc* param=fc->getParam(n);
            LPCC code = param->getValue();
            node->GetVar(code)->setVar('9',0);
        }
    } break;
    case 33: {	// incrNum
        if( cnt==0 ) return false;
        LPCC code=AS(0);
        if( slen(code) ) {
            int step=1, num=0;
            StrVar* sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                step=toInteger(sv);
            }
            sv=arrs->get(1);
            if( SVCHK('3',1) ) {
                num=step;
                node->setInt(code, step);
            } else {
                num=node->incrNum(code, step);
            }
            rst->setVar('0',0).addInt(num);
        }
    } break;
    case 34: {	// reset
        StrVar* sv=arrs? arrs->get(0): NULL;
        node->reset(SVCHK('3',1));
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 90: { // parseXml
        StrVar* data = arrs->get(0);
        XParseVar pv(data);
        // TreeNode* chkNode = checkObject(arrs->get(1),'n') ?(TreeNode*)arrs->get(1)->getObject(FUNC_HEADER_POS) : NULL;
        LPCC listtag = AS(1);
        int rtn = 0;
        if( slen(listtag) ) {
            LPCC st=gBuf.fmt("<%s",listtag);
            int sp = pv.findPos(st);
            if( sp!=-1 ) {
                LPCC et = gBuf.fmt("</%s",listtag);
                if( pv.match(st,et,FIND_FUNCSTR) ) {
                    int ep = pv.find(">");
                    XParseVar sub(pv.GetVar(), sp, ep+1);
                    rtn = parseXmlList(sub, node, node,NULL, XML_LIST, 0);
                }
            }
        } else {
            TreeNode* chkNode = checkObject(arrs->get(1),'n') ?(TreeNode*)arrs->get(1)->getObject(FUNC_HEADER_POS) : NULL;
            rtn = parseXmlList(pv, node, chkNode,"root", XML_ROOT, 0);
        }
        rst->setVar('3',(U16)rtn);
    } break;
    case 91: { // delete
        node->setNodeFlag(FLAG_DELETE);
        node->reset(true);
        StrVar* sv=arrs ? arrs->get(0):NULL;
        if( SVCHK('3',1) && node->isNodeFlag(FLAG_NEW) && !node->gv("@this") ) {
            SAFE_DELETE(node);
        }
    } break;
    case 92: { // isVar
        if( cnt==0 ) {
            return true;
        }
        LPCC code=AS(0);
        StrVar* sv=node->gv(code);
        rst->setVar('3',sv?1:0);
    } break;
    case 93: { // makeXml

    } break;
    case 94: { // parseJson
        if( cnt==0 ) {
            return false;
        }
        int sp=0, ep=0;
        StrVar* sv=arrs->get(0);
        StrVar* val = getStrVarPointer(sv,&sp,&ep);
        sv=arrs->get(1);
        if( SVCHK('3',1) ) {
            node->reset(true);
        }
        sv=arrs->get(2);
        if( SVCHK('3',0) ) {
            // not update
            node->setNodeFlag(FLAG_SKIP);
        } else if( node->isNodeFlag(FLAG_SKIP) ) {
            node->clearNodeFlag(FLAG_SKIP);
        }
        sv=arrs->get(3);
        if( SVCHK('3',1) ) {
            // convert
            node->setNodeFlag(FLAG_RESULT);
        } else if( node->isNodeFlag(FLAG_RESULT) ) {
            node->clearNodeFlag(FLAG_RESULT);
        }
        XParseVar pv(val,sp,ep);
        node->setJson(pv, (WBox*)fn);
        rst->setVar('n',0,(LPVOID) node);
    } break;
    case 95: { // sizeFormat
        if( cnt==0 ) {
            return false;
        }
        LPCC code = AS(0);
        StrVar* sv = node->gv(code);
        if( sv==NULL || sv->length()==0 ) {
            rst->reuse();
            return true;
        }
        UL64 size = toUL64(sv);
        const quint64 kb = 1024;
        const quint64 mb = 1024 * kb;
        const quint64 gb = 1024 * mb;
        const quint64 tb = 1024 * gb;
        QString s = "";
        if( size >= tb )
            s = QString( "%1TB" ).arg( qreal(size) / tb, 0, 'f', 3 );
        else if( size >= gb )
            s = QString( "%1GB" ).arg( qreal(size) / gb, 0, 'f', 2 );
        else if( size >= mb )
            s = QString( "%1MB" ).arg( qreal(size) / mb, 0, 'f', 2 );
        else {
            quint64 retnSize = 0;
            if (size > 0) {
                retnSize = size / kb;
                if (size % kb) retnSize++;
            }
            s = QString( "%L2KB" ).arg(retnSize);
        }
        rst->set(Q2A(s));
        XFunc* pfc = fc->parent();
        if( pfc->xftype>FTYPE_FUNCEND ) {
            node->GetVar(code)->set(Q2A(s));
        }
    } break;
    case 96: { // dateFormat
        LPCC code=NULL, fmt="yyyy-MM-dd hh:mm:ss";
        if( cnt>0 )
            code = AS(0);
        if( cnt>1)
            fmt = AS(1);
        if( slen(code) ) {
            StrVar* date = node->GetVar(code);
            QString str = dateTimeString(toString(date),fmt);
            rst->reuse()->add(Q2A(str));
            XFunc* pfc = fc->parent();
            if( pfc->xftype>FTYPE_FUNCEND ) {
                node->GetVar(code)->set(Q2A(str));
            }
        }
    } break;
    case 97: { // impl
        if( cnt==0 ) return true;
        rst->setVar('3',0);
    } break;
    case 98: { // map
        if( cnt==0 ) return true;
        StrVar* sv=arrs->get(0);
        if(SVCHK('f',1)) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XListArr* arr=NULL;
            int idx=1;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            sv=arrs->get(idx++);
            if( !SVCHK('n',0) ) {
                sv=fn?fn->gv("@this"): NULL;
            }
            if(SVCHK('n',0)) {
                TreeNode* thisNode=(TreeNode*)SVO;
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                sv=arrs->get(idx++);
            }
            if(SVCHK('a',0)) {
                arr=(XListArr*)SVO;
            } else {
                arr=getListArrTemp();
            }
            for(int n=0;n<node->childCount();n++) {
                TreeNode* cur=node->child(n);
                arrs->reuse();
                arrs->add()->setVar('n',0,(LPVOID)cur);
                arrs->add()->setVar('0',0).addInt(n);
                setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
                fnCur->call();
                arr->add()->add(fnCur->getResult());
            }
            funcNodeDelete(fnCur);
            rst->setVar('a',0,(LPVOID)arr);
        } else {
            LPCC field=toString(sv);
            XListArr* arr=getListArrTemp();
            for(int n=0;n<node->childCount();n++) {
                TreeNode* cur=node->child(n);
                if(!node->isNodeFlag(FLAG_VARCHECK)) {
                    arr->add()->add(cur->gv(field));
                }
            }
            rst->setVar('a',0,arr);
        }

    } break;
    case 99: { // filter
        if( cnt==0 ) return true;
        StrVar* sv=arrs->get(0);
        if(SVCHK('f',1)) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XListArr* arr=NULL;
            int idx=1;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            sv=arrs->get(idx++);
            if( !SVCHK('n',0) ) {
                sv=fn?fn->gv("@this"): NULL;
            }
            if(SVCHK('n',0)) {
                TreeNode* thisNode=(TreeNode*)SVO;
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                sv=arrs->get(idx++);
            }
            if(SVCHK('a',0)) {
                arr=(XListArr*)SVO;
            } else {
                arr=getListArrTemp();
            }
            for(int n=0;n<node->childCount();n++) {
                TreeNode* cur=node->child(n);
                arrs->reuse();
                arrs->add()->setVar('n',0,(LPVOID)cur);
                arrs->add()->setVar('0',0).addInt(n);
                setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
                fnCur->call();
                sv=fnCur->getResult();
                if(SVCHK('3',1)) {
                    arr->add()->setVar('n',0,(LPVOID)cur);
                }
            }
            funcNodeDelete(fnCur);
            rst->setVar('a',0,(LPVOID)arr);
        }
    } break;
    case 100: { // func
        if(cnt==0) return true;
        int sp=0,ep=0;
        StrVar* sv= getStrVarPointer(arrs->get(0),&sp,&ep);
        if(sp<ep) {
            rst->setVar('3',setClassFunc(node, sv, sp, ep, fn)?1:0);
        }
    } break;
    case 901: { // appendText
        if( cnt==0 ) {
			node->GetVar("@data")->reuse();
			rst->setVar('n',0,(LPVOID)node);
			return true;
		}
		if( cnt==1) {
			// (ex) node.appendText().appendText("xxxx");
			StrVar* sv=node->GetVar("@data");
			char ch=sv->charAt(0);
			if( ch=='\b' ) {
				rst->set(toString(sv));
				sv->reuse()->add(rst);
			}
			int sp=0, ep=0;
			StrVar* var=getStrVarPointer(arrs->get(0),&sp,&ep);
			if(sp<ep) {
				sv->add(var, sp, ep);
			}			
		} else {
			LPCC field=AS(0);
			StrVar* sv=node->GetVar(field);
			char ch=sv->charAt(0);
			if( ch=='\b' ) {
				rst->set(toString(sv));
				sv->reuse()->add(rst);
			}
            for(int n=1; n<cnt; n++ ) {
				int sp=0, ep=0;
				StrVar* var=getStrVarPointer(arrs->get(n),&sp,&ep);
				if(sp<ep) {
					sv->add(var, sp, ep);
				}
			}
		}
    } break;
    case 902: { // makeJson

    } break;
    case 903: { // merge
        if( cnt== 0)
            return false;
        StrVar* sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            U32 flag=isVarTrue(arrs->get(1))? 1: 0;
            mergeNode((TreeNode*)SVO, node, flag );
        }
    } break;
    case 904: { // keys
        XListArr* a = NULL;
        StrVar* sv= arrs ? arrs->get(0): NULL;
        LPCC startVal=NULL;
        bool ball=false;
        if( SVCHK('a',0) ) {
            a=(XListArr*)SVO;
            if( isVarTrue(arrs->get(1)) ) a->reuse();
        } else {
            if( sv ) {
                if( SVCHK('3',1) ) {
                    ball=true;
                } else {
                    startVal=toString(sv);
                }
            }
            a=node->addArray("@keys",true);
        }
        for( WBoxNode* bn = node->First(); bn; bn=node->Next() ) {
            LPCC code = node->getCurCode();
            if( startVal ) {
                if( StartWith(code, startVal) ) {
                    a->add()->set(code);
                }
            } else {
                if( code[0]=='@' && ball==false ) continue;
                a->add()->set(code);
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 905: { // push

    } break;
    case 906: { // toggle
        if( cnt==1 ) {
            LPCC code = AS(0);
            StrVar* sv = node->GetVar(code);
            if( isVarTrue(sv) )
                sv->setVar('3',0);
            else
                sv->setVar('3',1);
            rst->reuse()->add(sv);
        }
    } break;
    case 907: { // sort
        XListArr* a=NULL;
        if( cnt==0 ) {
            a=node->addArray("@sortArray", true);
            for( int n=0,sz=node->childCount();n<sz;n++) {
                a->add()->setVar('n',0,(LPVOID)node->child(n) );
            }
            rst->setVar('a',0,(LPVOID)a);
            return true;
        }
        StrVar* sv=NULL;
        LPCC field=NULL, ord=NULL, subField=NULL;
        if( cnt==1 ) {
            ord=AS(0);
        } else if( cnt==2 ) {
            field=AS(0); ord=AS(1);
        } else {
            field=AS(0), subField=AS(1), ord=AS(2);
            sv=arrs->get(3);
            if( SVCHK('a',0) ) {
                a=(XListArr*)SVO;
            }
        }
        if( a==NULL ) {
           a=node->addArray("@sortArray", true);
        }
        for( int n=0,sz=node->childCount();n<sz;n++) {
            a->add()->setVar('n',0,(LPVOID)node->child(n) );
        }
        bubbleSort(a, ccmp(ord,"asc")?0 : ccmp(ord,"desc")?1 : ccmp(ord,"alphanum")? 2 : ccmp(ord,"alphanum_desc")?3 :0, field, subField );
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 908: { // initNode
        node->reset(true);
        rst->setVar('n',0,(LPVOID)node);
    } break;

    case 909: {	// insertNode
        if( cnt==0 ) {
            return false;
        }
        StrVar* sv = arrs->get(0);
        TreeNode* cur = NULL;
        if( SVCHK('n',0) ) {
            cur = (TreeNode*)SVO;
            if( node->find(cur) ) {
                rst->setVar('3',0);
                return true;
            }
            node->addChild(cur);
            // add
            return true;
        }
        int idx=-1;
        if( isNumberVar(sv) ) {
            idx=toInteger(sv);
        }
        sv = arrs->get(1);
        if( SVCHK('n',0) ) {
            cur=node->addNode();
        }
        if( idx!=-1 && idx<node->childCount() ) {
            if(cur==NULL ) {
                cur = new TreeNode(2, true);
            }
            cur->xparent=node;
            node->xchilds.insert(idx,cur);
        }
        rst->setVar('n',0,(LPVOID)cur);
    } break;
    case 910: {	// cmp
        if( cnt==0 ) {
            return false;
        }
        LPCC code=AS(0);
        StrVar* sv=node->gv(code);
        rst->setVar('3',0);
        if( sv ) {
            StrVar* var=arrs->get(1);
            if( var && ccmp(toString(sv), toString(var)) ) {
                rst->setVar('3',1);
            }
        }
    } break;
	case 990: { // put
		StrVar* sv=NULL;
		TreeNode* pageNode=NULL;
#ifdef WIDGET_USE
		pageNode=findPageNode(node);
#endif
		if(pageNode==NULL) pageNode=node;
		sv=pageNode->gv("onInit");
		if( SVCHK('f',0) ) {
			XFuncNode* fnInit=(XFuncNode*)SVO;
			for(int n=0,sz=fc->getParamSize(); n<sz; n++ ) {
				XFunc* fcParam = fc->getParam(n);
				TreeNode* cur=NULL;
				if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
					XParseVar* pv=&(fcParam->xpv);
					pv->SetPosition();
					while(pv->valid()) {
						if(pv->ch()==0 ) break;
						LPCC id=pv->NextWord(), targetId=NULL;
						if( pv->ch()==':' ) {
							pv->incr();
							targetId=pv->NextWord();
							sv=fn->gv(targetId);
							if(SVCHK('n',0)) {
								cur=(TreeNode*)SVO;
							} else {
#ifdef WIDGET_USE
								cur=findWidgetId(pageNode, targetId);
								if( cur==NULL ) {
									cur=findNodeTag(pageNode,targetId);
									if(cur==NULL) {
										sv=node->gv(targetId);
										if(SVCHK('n',0)) {
											cur=(TreeNode*)SVO;
										}
									}
								}
#endif
							}
							if( cur) {
								// qDebug("put========id:%s ok",id);
								sv=fnInit->gv(id);
								if(sv==NULL || SVCHK('9',0) ) {
									fnInit->GetVar(id)->setVar('n',0,(LPVOID)cur);
								}
							}
						} else {
							break;
						}
					}
				} else if( fcParam->xdotPos==0 && fcParam->xftype==FTYPE_VAR ) {
					LPCC id = fcParam->getValue(), varName=id;
					int pos=IndexOf(id,':');
					if( pos>0 ) {
						varName=gBuf.add(id,pos), id+=pos+1;
						qDebug(".. node put %s: %s\n", varName, id);
					}
					//20202 07 21
					StrVar* var=NULL;
#ifdef WIDGET_USE
                    cur=findWidgetId(pageNode, id);
#endif
                    if( !cur) {
                        sv=fn->gv(id);
                        if(SVCHK('n',0)) {
                            cur=(TreeNode*)SVO;
                        } else if(sv) {
                            var=sv;
                        }
                    }
                    if( cur) {
                        fnInit->GetVar(varName)->setVar('n',0,(LPVOID)cur);
                    } else {
						if(var) {
							fnInit->GetVar(varName)->reuse()->add(var);
						} else {
							fnInit->GetVar(varName)->setVar('9',0);
						}
					}
				} else {
					qDebug("#0#put name:%s not define !!!", fcParam->getValue() );
				}
			}
        } else {
            qDebug("#0#node put error [node onInit function not define]");
        }
		rst->setVar('n',0,(LPVOID)node);
	} break;
    case 991: {	// with
        XFunc* func=fc->getParam(0);
        if( func==NULL ) return true;
        /*
        if( func->xftype==FTYPE_ARRAY && func->xpv.ch()=='@' ) {
            XParseVar* pv=&(func->xpv);
            if( pv->ch(1)=='s' && pv->ch(2)=='[' ) {
                pv->start+=2;
                if( pv->match("[","]",FIND_SKIP) ) {
                    XParseVar sub(pv->GetVar(), pv->prev, pv->cur);
                    setJsonVal(&sub, node, fn);
                }
            }
            pv->SetPosition();
            rst->setVar('n',0,(LPVOID)node);
            return true;
        }
        */
        if( func->xftype==FTYPE_TEXT || func->xftype==FTYPE_STR ) {
            StrVar* sv=cfVar(NULL);
            takeParseVar(sv,func,fn);
            XParseVar sub(sv, 0, sv->length() );
            setJsonVal(&sub, node, fn);
        } else {
            XParseVar* pv=&(func->xpv);
            if( pv->ch('[') || pv->find(":")!=-1) {
                node->setNodeFlag(FLAG_RESULT);
                node->setJson(*pv, (WBox*)fn);
            } else {
                setJsonVal(pv, node, fn);
            }
            pv->SetPosition();
        }
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 992: {	// inject
        StrVar* sv=NULL;
        cnt=fc->getParamSize();
        if( cnt==0 ) {
            WBoxNode* bn=node->First();
            while(bn) {
                LPCC code=node->getCurCode();
                if( code[0]!='@' ) {
                    StrVar* sv=&(bn->value);
                    fn->GetVar(code)->reuse()->add(sv);
                }
                bn=node->Next();
            }
        } else {
            XFuncNode* fnInit=NULL;
#ifdef WIDGET_USE
            if( fnInit==NULL && isPageNode(node) ) {
                sv=node->gv("onInit");
                if( SVCHK('f',0)) fnInit=(XFuncNode*)SVO;
            }
#endif
            for( int n=0;n<cnt;n++) {
                XFunc* fcParam=fc->getParam(n);
                if( fcParam->xftype==FTYPE_ARRAY ) {
                    XParseVar* pv=&(fcParam->xpv);
                    pv->SetPosition();
                    continue;
                }
                if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
                    StrVar* textVar=getStrVarTemp();
                    takeParseVar(textVar,fcParam,fn);
                    XParseVar pv(textVar );
                    nodeInjectVar(&pv, node, fn, fnInit);
                    continue;
                }
                LPCC code = fcParam->getVarName();
                sv = node->gv(code);
                if( sv==NULL && fnInit ) {
                    sv=fnInit->gv(code);
                }
                bool bref=false;
                if(code[0]=='@') {
                    code+=1;
                }
                if( code[0]=='&' ) {
                    bref=true;
                    code+=1;
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
                    if( SVCHK('s',0) ) {
                        StrVar* var=fn->GetVar(code)->reuse();
                        int sp=0,ep=0;
                        sv=getStrVarPointer(sv,&sp,&ep);
                        if( sp<ep ) var->add(var->get(sp), ep-sp );
                    } else {
                        fn->GetVar(code)->reuse()->add(sv);
                    }
                }
            }
        }
        rst->setVar('n',0,(LPVOID)node);
    } break;
	case 993: {	// timeVal
		StrVar* sv=NULL;
		int sz=fc->getParamSize();
		if( sz==0 ) {
			UL64 dist=node->getLong("@pageStartTick");
			if( dist>0 ) {
				dist=GetTickCount()-dist;
			}
			rst->setVar('1',0).addUL64(dist);
			return true;
		}

		XFunc* fcParam=fc->getParam(0);
		LPCC code=NULL;
		if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {	//
			code = fcParam->getValue();
			if( IndexOf(code,'$')!=-1 ) {
				StrVar* textVar=cfVar(NULL);
				takeParseVar(textVar,fcParam,fn);
				code=toString(textVar);
			}
		} else {
			code = fcParam->getValue();
		}
		if( slen(code)==0 ) return true;
		/*
		if( code[0]!='@') {
			sprintf(gVarBuf,"@%s",code);
			code=gVarBuf;
		}
		*/
		rst->reuse();
		if( sz==1 ) {
			qint64 prev=node->getLong(code);
			if( prev>0 ) {
				qint64 dist=GetTickCount()-prev;
				rst->setVar('1',0).addUL64(dist);
			} else {
				rst->setVar('1',0).addUL64(0);
			}
		} else {
			fcParam=fc->getParam(1);
			if( fcParam ) {
				if( fcParam->xftype==FTYPE_BOOL ) {
					LPCC val=fcParam->getValue();
					if( ccmp(val,"true") ) {
						if( cnt==3 ) {
							fcParam=fc->getParam(2);
							if( fcParam->xftype==FTYPE_BOOL ) {
								val=fcParam->getValue();
								if( ccmp(val,"true") ) {
									// ex) timeVal(code, true, true);
									qint64 prev=node->getLong(code);
									if( prev==0 ) {
										node->setLong(code,GetTickCount());
										rst->setVar('1',0).addUL64(0);
									} else {
										rst->setVar('1',0).addUL64(prev);
									}
									return true;
								}
							}
						}
						node->setLong(code,GetTickCount());
					} else {
						qint64 prev=node->getLong(code);
						if( prev>0 ) {
							qint64 dist=GetTickCount()-prev;
							rst->setVar('1',0).addUL64(dist);
						} else {
							rst->setVar('1',0).addUL64(0);
						}
						node->setLong(code,0);
					}
				} else {
					// timeVal("code", 200);
					execParamFunc(fcParam, fn, rst->reuse());
					if( isNumberVar(rst) ) {
						qint64 checkNum=(qint64)toUL64(rst), prev=node->getLong(code);
						rst->setVar('3',0);
						if( prev>0 ) {
							qint64 dist=GetTickCount()-prev;
							if( dist>checkNum ) {
								rst->setVar('3',1);
								node->setLong(code,0);
							}
						} else {
							node->setLong(code,0);
						}
					} else {
						rst->setVar('3',0);
					}
				}
			}
		}
    } break;
    case 994: {	// var
        // ### version 1.0.4 ###
        int sz=fc->getParamSize();
        if( sz==0 ) return false;
        StrVar* sv=NULL;
        XFunc* fcParam=fc->getParam(0);
        LPCC code=NULL;
        if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {	//
            code = fcParam->getValue();
            if( IndexOf(code,'$')!=-1 ) {
                takeParseVar(rst->reuse(),fcParam,fn);
                code=rst->get();
            }
        } else {
            code = fcParam->getValue();
        }
        if( slen(code)==0 ) {
            rst->reuse();
            return true;
        }
        char buf[32];
        if( code[0]!='@') {
            sprintf(buf,"@%s",code);
            code=buf;
        }

        rst->reuse();
        if( sz==1 ) {
            // [ex] node.var(@rect.key );
            // [ex] node.var(rect.key );
            sv= node->gv(code);
            if( sv && !SVCHK('9',0) ) rst->add(sv);
        } else {
            fcParam=fc->getParam(1);
            if( fcParam ) {
                LPCC ty=NULL;
                XFunc* fcCheck=fc->getParam(2);
                if( fcCheck) {
                    if(fcCheck->xftype==FTYPE_BOOL) {
                        LPCC code=fcCheck->getValue();
                        if(ccmp(code,"true")) {
                            ty="append";
                        } else {
                            ty="delete";
                        }
                    } else {
                        ty=fcCheck->getValue();
                    }
                    // qDebug("... node var type:%s ...\n", ty);
                }
                sv=node->GetVar(code); // gvar.set("@").add(code).get()
                execParamFunc(fcParam, fn, rst->reuse());
                if( slen(ty)) {
                    LPCC val=toString(rst);
                    if( ccmp(ty,"append") || ccmp(ty,"find")||ccmp(ty,"delete") ) {
                        XParseVar pv(sv);
                        bool find=false;
                        if(ccmp(ty,"append") || ccmp(ty,"find")) {
                            while(pv.valid()) {
                                LPCC code=pv.findEnd(",").v();
                                if(ccmp(code,val)) {
                                    find=true;
                                    break;
                                }
                            }
                            if(ccmp(ty,"append") && find==false ) {
                                if(sv->length()) sv->add(", ");
                                sv->add(val);
                            }
                        } else if(ccmp(ty,"delete")) {
                            StrVar* str=getStrVarTemp();
                            while(pv.valid()) {
                                LPCC code=pv.findEnd(",").v();
                                if(ccmp(code,val)) {
                                    find=true;
                                    continue;
                                }
                                if(str->length()) str->add(", ");
                                str->add(code);
                            }
                            sv->reuse()->add(str);
                        }
                        rst->setVar('3',find?1:0);
                    } else {
                        sv->set(val);
                    }
                } else {
                    setStrVar(sv, rst);
                }
            }
        }
    } break;
	case 995: {	// memberVar

    } break;
    case 996: {		// isset
        int sz=fc->getParamSize();
        if( sz==0 ) {
            return true;
        }
        StrVar* sv=NULL;
        XFunc* fcParam=NULL;
        LPCC code=NULL;
        bool bset=true;
        for( int n=0; n<sz; n++ ) {
            fcParam=fc->getParam(0);
            if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
				code=fcParam->getValue();
				if( IndexOf(code,'$')!=-1 ) {
					takeParseVar(rst->reuse(),fcParam,fn);
					code=gBuf.add(rst->get());
				}
            } else {
                code=fcParam->getValue();
				code = fcParam->getValue();
				if( fn->gv(code)) {
				   code=toString(fn->gv(code));
				}
            }
            if( slen(code)> 0 ) {
                sv=node->gv(code);
                if( sv==NULL || SVCHK('9',0) || (sv->length()==0) ) {
                    bset=false;
                    break;
                }
            }
        }
        rst->setVar('3', bset?1:0);
    } break;
    case 997: {		// ref
        int sz=fc->getParamSize();
        if( sz==0 ) {
            return true;
        }
        LPCC code = NULL;
        XFunc* fcParam=fc->getParam(0);
        if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
            code = fcParam->getValue();
			if( IndexOf(code,'$')!=-1 ) {
				takeParseVar(rst->reuse(),fcParam,fn);
				code=gBuf.add(rst->get());
			}
        } else {
			code = fcParam->getValue();
			if( fn->gv(code)) {
			   code=toString(fn->gv(code));
			}
        }
        StrVar* sv=node->gv(code);
        StrVar* prev=NULL;
        if( SVCHK('s',0) ) {
            prev=(StrVar*)SVO;
        }
        if( sz==1 ) {
            if( prev ) {
                rst->add(prev);
            } else {
                prev=new StrVar();
                node->setNodeFlag(FLAG_REF);
                if( sv ) prev->add(toString(sv) );
                rst->setVar('s',0,(LPVOID)prev).addInt(0).addInt(prev->length()).addInt(0).addInt(0);
                node->GetVar(code)->reuse()->add(rst);
            }
            return true;
        }
        bool reload=false, append=false;
        int ty=0;
        fcParam= sz>1 ? fc->getParam(2): NULL;
        if( fcParam ) {
            execParamFunc(fcParam, fn, rst->reuse() );
            LPCC type=toString(rst);
            if( ccmp(type,"node") ) {
                if( SVCHK('a',0) ) return true;
                ty=1;
            } else if( ccmp(type,"array") ) {
                if( SVCHK('n',0) ) return true;
                ty=2;
            } else if( ccmp(type,"append") ) {
                append=true;
            } else if( ccmp(type,"reload") ) {
                reload=true;
            }
            if( ty==0 ) {
                if( SVCHK('n',0) || SVCHK('a',0) ) return true;
            }
        }
        int sp=0, ep=0;
        StrVar* var=NULL;
        fcParam= sz>0? fc->getParam(1): NULL;
        if( fcParam ) {
            execParamFunc(fcParam, fn, rst->reuse() );
            sv=rst;
            if( !SVCHK('9',0) ) {
                var=getStrVarPointer(sv, &sp, &ep);
            }
        }
        if( ty==0 ) {
            if( prev==var ) {
                node->GetVar(code)->reuse()->add(sv);
                return true;
            }
            if( prev==NULL ) {
                prev=new StrVar();
                node->setNodeFlag(FLAG_REF);
            }
            if( sp==ep ) {
                prev->reuse();
                rst->setVar('s',0,(LPVOID)prev).addInt(0).addInt(0).addInt(0).addInt(0);
                node->GetVar(code)->reuse()->add(rst);
            } else {
                if( append ) {
                    prev->add(var->get(sp), ep-sp);
                } else {
                    prev->set(var->get(sp), ep-sp);
                }
                rst->setVar('s',0,(LPVOID)prev).addInt(0).addInt(prev->length()).addInt(0).addInt(0);
            }
            node->GetVar(code)->reuse()->add(rst);
        } else if( ty==1 ) {
            TreeNode* cur=NULL;
            sv=node->GetVar(code);
            if( SVCHK('n',0) ) {
                cur=(TreeNode*)SVO;
            } else {
                cur=new TreeNode(2, true);
            }
            if( sp<ep ) {
                cur->setJson(var, sp, ep);
            }
            rst->setVar('n',0,(LPVOID)cur);
        } else if( ty==2 ) {
            XListArr* cur=NULL;
            sv=node->GetVar(code);
            if( SVCHK('a',0) ) {
                cur=(XListArr*)SVO;
            } else {
                cur=new XListArr(true);
            }
            if( sp<ep ) {
                XParseVar pv(var, sp, ep);
                setArrayVal(&pv, cur);
            }
            rst->setVar('a',0,(LPVOID)cur);
        }

    } break;
    case 998: {		// member
        int sz=fc->getParamSize();
        XFuncNode* fnInit=NULL;
        StrVar* sv=NULL;
        if( sz==0 ) {
            sv=node->gv("onInit");
            if( SVCHK('f',0)) {
                fnInit=(XFuncNode*)SVO;
                for(WBoxNode* bn=fnInit->First(); bn; bn=fnInit->Next()) {
                    LPCC vnm=fnInit->getCurCode();
                    if(ccmp(vnm,"@this") || ccmp(vnm,"@c")) continue;
                    rst->add(vnm).add("=");
                    toString(&(bn->value),rst);
                    rst->add("\n");
                }
            }
            return true;
        }
        LPCC code = NULL;
        XFunc* fcParam=fc->getParam(0);
        if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
            code = fcParam->getValue();
			if( IndexOf(code,'$')!=-1 ) {
				takeParseVar(rst->reuse(),fcParam,fn);
				code=gBuf.add(rst->get());
            }
        } else {
            if( fcParam->xftype==FTYPE_VAR && fcParam->xdotPos==0 ) {
                code = fcParam->getValue();
            } else {
                execParamFunc(fcParam, fn, rst->reuse() );
				code=gBuf.add(rst->get());
            }
        }
        sv=node->gv("onInit");
        if( SVCHK('f',0)) {
            fnInit=(XFuncNode*)SVO;
        } else if(sz>1) {
            XFuncSrc* fsrc=gfsrcs.getFuncSrc();
            fnInit=gfns.getFuncNode(fsrc->xfunc);
            fnInit->GetVar("@this")->setVar('n',0,(LPVOID)node);
            fnInit->setNodeFlag(FLAG_PERSIST);
            node->GetVar("onInit")->setVar('f',0,(LPVOID)fnInit);
		}
        if( fnInit ) {
            sv=fnInit->gv(code);
            if( sz==1) {
                /*
                if(sv==NULL) {
                    XFuncNode* fnCur=fn->parent();
                    for(int n=0; fnCur && n<8;n++) {
                        sv=fnCur->gv(code);
                        if(sv) break;
                        fnCur=fnCur->parent();
                    }
                }
                */
				rst->reuse()->add(sv);
            } else {
                execParamFunc(fc->getParam(1), fn, rst->reuse() );
                sv=fnInit->GetVar(code)->reuse();
                sv->add(rst);
                /*
                sv=rst;
                if(SVCHK('f',1)) {
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    execParamFunc(fc->getParam(2), fn, rst->reuse() );
                    sv=node->gv(code);
                    if(checkFuncObject(rst,'3',1) || sv==NULL ) {
                        if(sv==NULL) {
                            sv=node->GetVar(code);
                        }
                        sv->setVar('f',1,(LPVOID)fsrc);
                    }
                } else {
                    sv=fnInit->GetVar(code)->reuse();
                    sv->add(rst);
                }
                */
            }
        }
    } break;
    default: return false;
    }
    /*(na) 20240625
    if( fc->xfflag&FFLAG_PPARAM && rst->length()==0 )
        rst->setVar('n',0,(LPVOID)node);
    */
    return true;
}


bool objectFunction(XFunc* fc, LPCC fnm, StrVar* var, PARR arrs, XFuncNode* fn, StrVar* rst) {
    if( fc==NULL || var==NULL ) return false;
    char ch=var->charAt(0);
    if( ch=='\b' ) {
        ch=var->charAt(1);
    } else {
        ch='\0';
    }
    if( ch=='\0' || ch=='9' ) {
        if( fc->xfuncRef==0 ) {
            regStrFuncs();
            fc->xfuncRef = getHashKeyVal("str", fnm );
        }
        if( fc->xfuncRef ) {
            execStrFunc(fc,arrs,fn,var,rst);
        }
    } else {
        execObjectFunc(fc, arrs, fn, var, rst, NULL, fnm);
    }
    return true;
}
