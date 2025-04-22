#include "func_util.h"

#include "../util/db_util.h"
#include "../fobject/FMutex.h"

#define HASH_USER_FUNC 1
#define HASH_SUB_FUNC 2
#define LOCAL_PARAMS_SIZE	64
#define FLAG_LOCALARRAY_SET 0x200

#ifdef WINDOW_USE
#ifndef _NO_FED_NAMESPACE
    using namespace fed;
#endif
#else
USING_NAMESPACE_FED
#endif

XFuncNode* gScriptFuncNode=NULL;
FMutex		gMutexArr;
FMutex		gMutexSubFunc;
FMutex      gMutexCommListArr;
FMutex      gMutexCommStrVar;
FMutex      gMutexCommTreeNode;

XFuncSrc    gfsrc;
StrVar      gCallLog;

TreeNode*	gTreeNodes = new TreeNode[128];
XListArr*	gArrays = new XListArr[512];
XListArr	gStrVars;

int gStrVarsIdx=0;
int gTreeNodeIdx=0;
int gArrayIndex=0;


XListArr*	gParams = new XListArr[LOCAL_PARAMS_SIZE];
int gParamIndex=0;

XFuncSrc*	gFuncSrcs = new XFuncSrc[16];
int gFuncSrcIndex=0;

TreeNode* getTreeNodeTemp() {
	gMutexCommTreeNode.EnterMutex();
    if( gTreeNodeIdx>=128 ) {
        qDebug("xxx getTreeNodeTemp reset (%d) xxx", gTreeNodeIdx);
        gTreeNodeIdx=0;
    }
    TreeNode* node = &gTreeNodes[gTreeNodeIdx++];
    node->reset(true);
	gMutexCommTreeNode.LeaveMutex();
    return node;
}

XListArr* getListArrTemp() {
	gMutexCommListArr.EnterMutex();
    if( gArrayIndex>=512 ) {
        qDebug("xxx getListArrTemp reset (%d) xxx", gArrayIndex);
        gArrayIndex=0;
    }
    XListArr* arr = &gArrays[gArrayIndex++];
    arr->reuse();
	gMutexCommListArr.LeaveMutex();
    return arr;
}
StrVar* getStrVarTemp() {
    gMutexCommStrVar.EnterMutex();
    if( gStrVarsIdx>=1024 ) {
        // LPCC funcNm=getCf()->get("@currentFuncName");
        // qDebug("xxx getStrVarTemp (%s %d) xxx", funcNm, gStrVarsIdx);
        gStrVarsIdx=0;
    }
    if( gStrVars.size()==0 ) {
        for(int n=0; n<1024;n++ ) gStrVars.add();
    }
    StrVar* rst=gStrVars.get(gStrVarsIdx++)->reuse();
    gMutexCommStrVar.LeaveMutex();
    return rst;
}

XListArr* getLocalArray(bool bset ) {
    XListArr* arr=NULL;
    gMutexArr.EnterMutex();
    if( gParamIndex>=LOCAL_PARAMS_SIZE ) {
        // qDebug("#0#getLocalArray reset :  %d \n", gParamIndex );
        gParamIndex=0;
    }
    int start=gParamIndex;
    bool loop=false;
    while(true) {
        if( gParamIndex>=LOCAL_PARAMS_SIZE ) {
            // qDebug("#0#getLocalArray reset start:%d roop : %d \n", start, gParamIndex );
            gParamIndex=0;
            loop=true;
        }
        if( loop && gParamIndex>=start ) {
            qDebug("#0#getLocalArray allocate fail : %d, %d \n", start, gParamIndex );
            arr=&gParams[gParamIndex++];
            break;
        }
        arr=&gParams[gParamIndex++];
        if( (arr->xflag & FLAG_LOCALARRAY_SET)==0 ) {
            break;
        }
    }
    if( arr ) {
        arr->reuse();
        arr->xflag|=FLAG_LOCALARRAY_SET;
    } else {
        qDebug("#9#local function args array error %d, %d \n", start, gParamIndex );
    }
    gMutexArr.LeaveMutex();
    return arr;
}

StrVar* getStrVarPointer(StrVar* sv, int* sp, int* ep ) {
    *sp=0, *ep=0;
    if( sv==NULL ) return NULL;
    if( sv->charAt(0)=='\b') {
        char ch=sv->charAt(1);
        if( ch=='s' ) {
            if( sv->getU16(2)==0 ) {
                StrVar* str = (StrVar*)SVO;
                int s=sv->getInt(OBJ_PROP_START_POS), e=sv->getInt(OBJ_PROP_START_POS+4);
                if( s==0 && e==0 )
                    e=str->length();
                if( s<e ) {
                    *sp=s, *ep=e;
                    return str;
                }
            } else if( sv->getU16(2)==1 ) {
                StrVar* str = (StrVar*)SVO;
                *ep = str->length();
                return str;
            }
        } else {
            /*
            StrVar* str=getStrVarTemp(); // gum.getStrVar(true);
            str->set(toString(sv));
            *ep = str->length();
            return str;
            */
            *ep=sv->length();
        }
    } else if( sv ) {
        *ep = sv->length();
    }
    return sv;
}

//#> func util
SqlDatabase* getModelDatabase(LPCC code, LPCC inode, TreeNode* node ) {
    if( slen(code)==0 ) return NULL;
    if( slen(inode)==0 ) {
        inode=code;
    }
    if( node==NULL ) {
        node=getTreeNode("dbms", inode, false);
    }
    SqlDatabase* db=NULL;
    StrVar* sv=NULL;
    if( node ) {
        sv=node->gv("@db");        
        if( SVCHK('d',1) ) {
            db=(SqlDatabase*)SVO;
        }
    }
    if( db==NULL ) {
        DbUtil* dbCf=getConfDb();
        if( dbCf ) {
            if( dbCf->prepare("select dsn, driver, port from db_info where dsn=?") &&
                dbCf->bindStr(code).bind() && dbCf->fetch() )
            {
                LPCC driver=dbCf->get("driver"), dbms=NULL;
                if( slen(driver)==0 || ccmp(driver,"sqlite") ) {
                    return NULL;
                }
                if( StartWith(driver,"SQL Server") ) {
                    dbms = "mssql";
                } else {
                    dbms = driver;
                }
                if( node==NULL ) {
                    node=getTreeNode("dbms", inode);
                    node->set("tag","dbms");
                    node->setNodeFlag(FLAG_PERSIST);
                }
                node->xstat=FNSTATE_DB;
                node->set("driver",dbms);
                node->set("dsn",code);
                node->set("inode",inode);

                db=new SqlDatabase(node);
                // db->open(slen(inode)?inode:code);
                if( node ) {
                    node->GetVar("@db")->setVar('d',1,(LPVOID)db);
                }
            }
        } else {
            qDebug("#9#data model create error (%s %s)", code, inode);
        }
    }

    return db;
}


//#> delete object

void deleteAllTreeNode(TreeNode* root, U32 flag) {
    if( root==NULL)
        return;
    if( flag ) {
        root->removeAll(true, true);
        if( flag&0x2 ) {
            if( root->isNodeFlag(FLAG_NEW) && !root->isNodeFlag(FLAG_PERSIST) ) {
                SAFE_DELETE(root);
            }
        }
    } else {
        root->removeAll(true);
    }
}
void deleteAllArray(XListArr* a) {
    StrVar* sv=NULL;
    for( int n=0,sz=a->size();n<sz;n++) {
        sv=a->get(n);
        if( SVCHK('n',0) ) {
            TreeNode* tree=(TreeNode*)SVO;
            tree->reset();
        } else if( SVCHK('a',0) ) {
            XListArr* arr=(XListArr*)SVO;
            deleteAllArray(arr);
        }
    }
    a->reuse();
}
void deleteAllFunc(XFunc* fc) {
    if( fc==NULL )
        return;
    U32 ref=fc->xfuncRef;
    for( int n=0,size=fc->xsubs.size();n<size;n++) {
        XFunc* cur = fc->getFunc(n); // >xsubs.getvalue(n);
        if( cur==NULL ) break;
        if( fc==cur )
            break;
        if( ref==9001 ) {
            if( cur->xftype==FTYPE_SET && cur->xdotPos==0 ) {
                XFunc* p = cur->getParam(0);
                if( p && (p->getType()==FTYPE_CALLBACK) && (p->xfuncRef>0) ) {
                    LPCC code=gBuf.fmt("c%d", p->xfuncRef );
                    StrVar* sv=getStrVar("func", code);
                    if( SVCHK('f',1) ) {
                        XFuncSrc* fsrc=(XFuncSrc*)SVO;
                        gfsrcs.deleteFuncSrc(fsrc);
                    }
                }
            }
        }
        deleteAllFunc(cur);
        // gfuncs.deleteFunc(cur);
    }
    fc->xsubs.reuse();
    for( int n=0,size=fc->xparams.size();n<size;n++) {
        XFunc* cur = fc->xparams.getvalue(n);
        if( fc==cur )
            break;
        deleteAllFunc(cur);
        // gfuncs.deleteFunc(cur);
    }
    fc->xparams.reuse();
    fc->xflag=0;
    gfuncs.deleteFunc(fc);
}
void funcNodeDelete(XFuncNode* fnCur ) {
    if( fnCur==NULL ) return;
    XFuncNode* fnParent=fnCur->parent();
    if( fnParent && fnParent->isNodeFlag(FLAG_PERSIST) ) {
        fnCur->xchilds.reuse();
    } else {
		if( !fnCur->isNodeFlag(FLAG_PERSIST) ) {
			StrVar* var=fnCur->gv("@localVarObjects");
			if(var) {
#ifdef QT5_USE
				int dist=12;
#else
				int dist=8;
#endif
                int curPos=0, len=var->length();
				while(curPos<len ) {
					if( var->charAt(curPos)=='\b' ) {
						char ch=var->charAt(curPos+1);
						U16 state=var->getU16(curPos+2);
						if(ch=='n' && state==0 ) {
							TreeNode* node=(TreeNode*)var->getObject(curPos+FUNC_HEADER_POS);
							if(node->isNodeFlag(FLAG_NEW) ) {
								SAFE_DELETE(node);
							}
						} else if(ch=='a' && state==0 ) {
							XListArr* arr=(XListArr*)var->getObject(curPos+FUNC_HEADER_POS);
							if(arr->isNodeFlag(FLAG_NEW) ) {
								SAFE_DELETE(arr);
							}
						}
                        curPos+=dist;
					} else {
						break;
					}
				}
				var->reuse();
			}
		}
	}
    gfns.deleteFuncNode(fnCur);
}

XFuncSrc* getFuncSrc() {
    if( gFuncSrcIndex>=16 ) {
        gFuncSrcIndex=0;
    }
    XFuncSrc* fsrc=&gFuncSrcs[gFuncSrcIndex++];
    if( fsrc->xfunc ) {
        deleteAllFunc(fsrc->xfunc);
        fsrc->xfunc=NULL;
    }
    fsrc->reuse();
    fsrc->xflag=0;
    return fsrc;
}

XFuncNode* getInitFuncNode(TreeNode* node) {
    StrVar* sv=node->gv("onInit");
    XFuncNode* fn=NULL;
    if( SVCHK('f',0) ) {
        fn=(XFuncNode*)SVO;
    } else {
#ifdef WIDGET_USE
        TreeNode* page=findPageNode(node);
        if( page ) {
            sv=page->gv("onInit");
            if( SVCHK('f',0) ) {
                fn=(XFuncNode*)SVO;
            }
        }
#endif
    }
    return fn;
}
bool reloadFuncSrc(XFuncSrc* fsrc, StrVar* sv, int sp, int ep, LPCC fnm ) {
    if( fsrc==NULL ) {
        return false;
    }
    XFunc* fc=fsrc->xfunc;
    fsrc->reuse();
    fsrc->xparam.reuse();
    fsrc->xflag=0;
    if( fc ) {
        deleteAllFunc(fc);
        fsrc->xfunc=NULL;
    }
    if( fnm ) qDebug("#0### %s reload local function \r\n\r\n", fnm);
    fsrc->readBuffer(sv,sp, ep );
    // fsrc->parseData(sv,pv.prev,pv.cur );
    if( fsrc->makeFunc() && fsrc->xfunc ) {
        return true;
    }
    return false;
}
bool callScriptFunc(StrVar* src, int sp, int ep, XFuncNode* fn) {
    if( src==NULL ) {
        if( gScriptFuncNode ) {
            cfVar("@scriptFuncNode")->setVar('f',0,(LPVOID)gScriptFuncNode);
            return true;
        }
        return false;
    }
    bool ret=false;
    StrVar* sv=NULL;
    TreeNode* thisNode=NULL;
    TreeNode* callNode=cfNode("@callVars");
    LPCC inlineMode=callNode->get("@inlineMode");
    if(slen(inlineMode)==0) {
        inlineMode=cfVar("@inlineMode")->get();
    }
    if( fn==NULL ) {
        sv=cfVar("@currentObject");
        if( !SVCHK('n',0) ) {
             sv=cfVar("@currentPage");
        }
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
            sv=thisNode->gv("onInit");
            if( SVCHK('f',0) ) {
                fn=(XFuncNode*)SVO;
            }
        }
    } else {
        sv=callNode->gv("@thisNode");
        if( SVCHK('n',0) ) {
            thisNode=(TreeNode*)SVO;
        }
    }
    bool appendMode=ccmp(inlineMode,"append");
    XFuncSrc* fsrc = gfsrcs.getFuncSrc();
    XFuncNode* fnCur=NULL;
    if( appendMode ) {
        if(gScriptFuncNode==NULL) {
            gScriptFuncNode=new XFuncNode();
        }
        fnCur=gScriptFuncNode;
        callNode->GetVar("scriptFuncNode")->setVar('f',0,(LPVOID)fnCur);
    }
    if( fsrc ) {
        fsrc->reuse();
        fsrc->readBuffer(src, sp, ep);
        fsrc->makeFunc();
        fsrc->xflag=0;
        XFunc* func=fsrc->xfunc;
        if( func ) {
            if(fnCur==NULL) {
                fnCur=gfns.getFuncNode(func,fn);
            } else {
                if( fnCur==gScriptFuncNode ) gScriptFuncNode->xfunc=func;
            }

            if( thisNode==NULL ) {
                if( fn ) {
                    sv=fn->gv("@page");
                    if( SVCHK('n',0) ) {
                        thisNode=(TreeNode*)SVO;
                        fnCur->GetVar("@page")->setVar('n',0,(LPVOID)thisNode);
                    }
                    sv=fn->gv("@this");
                    if( SVCHK('n',0) ) {
                        thisNode=(TreeNode*)SVO;
                    }
                } else {
                    sv=cfVar("@currentPage");
                    if( SVCHK('n',0) ) {
                        thisNode=(TreeNode*)SVO;
                    }
                }
            }
            if( thisNode ) {
                func->xflag|=FUNC_INLINE;
#ifdef INLINENODE_USE
                fnCur->GetVar("@inlineNode")->setVar('n',0,(LPVOID)thisNode);
#endif
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
            /*
            if( !appendMode ) {
                for( WBoxNode* bn=callNode->First(); bn; bn=callNode->Next() ) {
                    LPCC name=callNode->getCurCode();
                    fnCur->GetVar(name)->set(bn->value);
                }
            }
            */
            getCf()->set("@currentFuncName","runScript");
            if( ccmp(inlineMode,"debug") ) {
                gCallLog.reuse();
                fsrc->xfunc->printFunc(gCallLog,0);
                qDebug("#0#callScriptFunc:%s\n", gCallLog.get());
                callNode->GetVar("logs")->add(&gCallLog);
            }
            fnCur->call();
            if( appendMode ) {
                LPCC varName=NULL;
                XParseVar pv(src, sp, ep);
                char ch=0;
                if(pv.ch()) {
                    LPCC name=pv.NextWord();
                    ch=pv.ch();
                    while(ch=='.') {
                        ch=pv.incr().moveNext().ch();
                    }
                    if(ch=='\0') {
                        varName=name;
                    } else if(ch=='=') {
                        pv.findEnd("\n");
                        if(!pv.ch()) varName=name;
                    }
                }
                if( slen(varName) ) {
                    sv=fnCur->gv(varName);
                    qDebug("#0#>> %s = %s\n", varName, toString(sv));
                } else {
                    StrVar* sv=fnCur->getResult();
                    if( SVCHK('3',1)) {
                        StrVar* rst=gCallLog.reuse();
                        for( WBoxNode* bn=fnCur->First(); bn; bn=fnCur->Next() ) {
                            LPCC code=fnCur->getCurCode();
                            if( ccmp(code,"@this") || ccmp(code,"@page")  || ccmp(code,"_funcName_") ) continue;
                            rst->add("\n\t").add(code).add("=");
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
                        // qDebug("#0#>> functions vars %s", rst->get() );
                    }
                    else if(sv && sv->length() ) {
                        qDebug("#0#>> result %s\n", toString(sv));
                    }
                }
            } else {
                if( ccmp(inlineMode,"result") ) {
                    StrVar* sv=fnCur->getResult();
                    StrVar* rst=callNode->GetVar("@result")->reuse();
                    if( sv ) {
                        rst->add(sv);
                    }
                }
                funcNodeDelete(fnCur);
            }
            ret=true;
        }
        gfsrcs.deleteFuncSrc(fsrc);
    }

    return ret;
}

bool callScript(StrVar* src, XFuncNode* fn) {
    int sp=0, ep=0;
    StrVar* sv=getStrVarPointer(src,&sp,&ep);
    return callScriptFunc(sv, sp, ep, fn);
}

bool callEventFunc(XFuncNode* fn, XFunc* fc, StrVar* rst) {
    XFunc* fcCur=fc->getFunc(0);
    if( fcCur==NULL ) return false;
    if( fcCur->getType()==FTYPE_RETURN && fc->getFuncSize()==1 ) {
        fcCur=fcCur->getParam(0);
        if( fcCur==NULL ) return false;
        U16 type=fcCur->getType();
        StrVar* sv=fn->gv("@this");
        if( SVCHK('n',0) && (type==FTYPE_FUNC_THIS || type==FTYPE_FUNC ) ) {
            TreeNode* thisNode=(TreeNode*)SVO;
            XListArr* arrs=NULL;
            LPCC fnm=fcCur->getFuncName();
            int asize=fcCur->getParamSize();
            if( asize ) {
                arrs=getLocalArray(true);
                for( int n=0; n<asize; n++ ) {
                    XFunc* param = fcCur->getParam(n);
                    LPCC name=param->getValue();
                    arrs->add()->add(fn->gv(name));
                }
            }
            if( type==FTYPE_FUNC_THIS ) {
                if( callModuleFuncCheck(thisNode, fnm, arrs,fn, rst,NULL) ) {
                    return true;
                }
            } else if( fcCur->xdotPos>0 ) {
                LPCC vnm=fcCur->getVarName();
                if( vnm[0]=='@' ) {
                    execCallSubfunc(vnm+1, fnm, fn, arrs, rst, true, true);
                }
            } else {
                execUserFunc(fcCur, arrs, fn, NULL, rst, fnm);
            }
            if( arrs ) releaseLocalArray(arrs);
            return true;
        }
    }
    return false;
}

#ifdef SETIF_USE
bool callIfCheck(XFunc* fc, XFuncNode* fn, StrVar* rst ) {
    TreeNode* thisNode=NULL;
    StrVar* sv=getFuncVar(fn,"@this");
    XFunc* fcParam=fc->getParam(0);
    if( SVCHK('n',0) ) {
        thisNode=(TreeNode*)SVO;
    }
    if( fcParam==NULL || thisNode==NULL ) {
        return false;
    }
    int sp=0,ep=0;
    StrVar var;
    execFunc(fcParam,fn,&var);
    sv=getStrVarPointer(&var, &sp, &ep);
    XParseVar pv(sv, sp, ep );
    parseArrayVar(pv,thisNode,fn,rst,0x200,fcParam);
    // int rtn=0;
    return isVarTrue(rst);
}
#endif

int callControl(XFunc* fc, XFuncNode* fn, StrVar* rst, int* offset) {
    int rtn = 0;
    getControlFuncKind(fc);
    switch(fc->xfkind) {
    case 1:
    case 2:		// ifThis
    case 10:	// not
    case 11: {	// notThis
        bool bcalled = false;
        if( fc->xfkind==1 || fc->xfkind==10 ) {
            rtn = fn->callIf(fc,rst,bcalled);
#ifdef SETIF_USE
            if( rtn>=100 && rtn<2000 ) {
                if( offset ) *offset=rtn-100;
                return 0;
            }
#endif
        }
#ifdef SETIF_USE
        else if( fc->xfkind==2 || fc->xfkind==11 ) {
            bcalled = callIfCheck(fc,fn,rst);
            if( fc->xfkind==11 ) {
                bcalled = !bcalled;
            }
            if( bcalled ) {
                rtn = callSubFuncs(fc,fn,rst);
            }
        }
#endif
        if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN || rtn==FTYPE_CONTINUE ) {
            return rtn;
        }
        XFunc* next = fc->nextFunc();
        while( next ) {
            if( next->xftype==FTYPE_CONTROL ) {
                getControlFuncKind(next);
                if( next->xfkind>4 && next->xfkind<9 ) {
                    if( offset ) *offset++;
                    if( !bcalled  ) {
                        bool chk = false;
                        if( next->xfkind==5 ) {			// else
                            chk = true;
                        } else if( next->xfkind==6 ) {	// else if
                            chk = funcTrueCheck(next,fn,rst);
                        }
#ifdef SETIF_USE
                        else if( next->xfkind==7 ) {	// else ifThis
                            chk = callIfCheck(next,fn,rst);
                        }
#endif
                        if( chk ) {
                            // rtn = callFuncs(next,fn,rst);
                            rtn = callSubFuncs(next,fn,rst);
                            bcalled = true;
                        }
                    }
                } else
                    break;
            } else
                break;
            next = next->nextFunc();
        }
    } break;
    case 4:	// switch
        rtn = fn->callSwitch(fc,rst); break;
    case 13: {
        rtn = fn->callFor(fc,rst);
        if( rtn==FTYPE_BREAK || rtn==FTYPE_CONTINUE )
            rtn = 0;
    } break;
    default: break;
    }
    return rtn;
}

int callSubFuncs(XFunc* fc, XFuncNode* fn, StrVar* rst) {
    int rtn = 0;
    bool chk = false;
    bool bcallback=fc->xfuncRef==9001? true: false;
    TreeNode* targetNode=NULL;
    for( int n=0,cnt=fc->xsubs.size();n<cnt;n++) {
        XFunc* cur = fc->xsubs.getvalue(n);
        if( cur==NULL )
            break;
        if( bcallback ) {
            if( cur->xftype==FTYPE_SET ) {
                XFunc* p = cur->getParam(0);
                if( p && p->getType()==FTYPE_CALLBACK ) {
                    continue;
                }
            }
        }
        if( cur->xfflag&0x80 ) {
            continue;
        }
        rst->reuse();

        if( cur->xftype==FTYPE_CONTROL ) {
            int offset = 0;
            rtn = callControl(cur, fn, rst, &offset);
            if( offset>0 ){
                n+= offset;
            }
        } else {
            if( cur->xfkind==11 ) {
                if( cur->xftype==FTYPE_ARRAY ) {
                    if( targetNode ) {
                        cur->xpv.SetPosition();
                        parseArrayVar( cur->xpv, targetNode, fn, rst, 0, cur );
                    }
                } else {
                    rtn = execFunc(cur,fn,rst);
                    if( checkFuncObject(rst, 'n', 0) ) {
                        targetNode=(TreeNode*)rst->getObject(FUNC_HEADER_POS);
                    }
                }
            } else {
                rtn = execFunc(cur,fn,rst);
                if( targetNode ) {
                    targetNode=NULL;
                }
            }
        }
        if( rtn==FTYPE_BREAK || rtn==FTYPE_RETURN || rtn==FTYPE_CONTINUE ) {
            chk = true;
            break;
        }
    }
    return chk ? rtn: 0;
}


bool callFuncSrc(XFuncSrc* fsrc, XFuncNode* fn, bool remove, TreeNode* thisNode, TreeNode* meNode, LPCC eventNodeName, PARR arrs, StrVar* rst ) {
    if( fsrc==NULL || fsrc->xfunc==NULL ) {
        releaseLocalArray(arrs);
        return false;
    }
    LPCC funcNm=NULL;
    if( rst && rst->length() ) {
        funcNm=rst->get();
    }
    if( slen(funcNm)==0 ) {
        funcNm=eventNodeName ? eventNodeName : "&callFuncSrc";
    }

    StrVar* sv=NULL;
    XFuncNode* fnParent=fn;
    if( thisNode && eventNodeName==NULL ) {
        sv=thisNode->gv("onInit");
        if( SVCHK('f',0) ) {
            fnParent=(XFuncNode*)SVO;
        }
    }
    /*
    if( ((fsrc->xflag & 0x1800)==0) && funcNm[1]=='@' ) {
        int pos=IndexOf(funcNm,'.');
        if( pos>0 ) {
            LPCC subNm=funcNm+pos+1;
            if( arrs==NULL ) {
                arrs=getLocalArray(true);
                arrs->add()->set(subNm);
            } else {
                sv=arrs->insert(0);
                if( sv ) sv->set(subNm);
            }
        }
    }
    */
    XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc,fnParent);
    setFuncSrcParam(fnCur,arrs,fsrc,0,fn);
    if( thisNode ) {
        fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
        if( eventNodeName ) {
            fnCur->GetVar(eventNodeName)->setVar('n',0,(LPVOID)thisNode);
        }
    }
    if( meNode ) {
        fnCur->GetVar("@page")->setVar('n',0,(LPVOID)meNode);
    }
    /*
    if( funcNm[0]=='&' ) {
        getCf()->set("@currentFuncName",funcNm+1);
    }
    */
    if( rst ) {
        rst->reuse();
    }
    fnCur->call(NULL, rst );
    releaseLocalArray(arrs);
    /* (jwas)
    if( rst && sv && sv->length() ) {
        if( rst!=sv ) {
            rst->reuse()->add(sv);
        }
    }
    */
    funcNodeDelete(fnCur);
    return true;
}

//#> func util
void setFuncSrcParam(XFuncNode* fnCur, PARR arrs, XFuncSrc* fsrc, int start, XFuncNode* fn) {
    if( fnCur==NULL ) return;
    if( fsrc==NULL && fnCur->xfunc ) {
        fsrc=fnCur->xfunc->getFuncSrc();
    }
    if( fsrc==NULL ) return;
    StrVar* sv = NULL;
    XParseVar pv(&(fsrc->xparam));
    int sz=0;
    if( arrs ) {
        sz=arrs->size();
        /*
        sv=fnCur->GetVar("@params");
        if(SVCHK('a',0)) {
            LPCC name=getBaseFuncName(fn?fn:fnCur);
            qDebug("param already setting %s\n", name);
        }
        */
        if( !fnCur->isNodeFlag(FLAG_PERSIST) && !arrs->isNodeFlag(FLAG_LOCALARRAY_SET) ) {
            arrs->setNodeFlag(FLAG_LOCALARRAY_SET);
            LPCC name=getBaseFuncName(fn?fn:fnCur);
            qDebug("param local array setting %s\n", name);
        }
        fnCur->GetVar("@params")->setVar('a',0,(LPVOID)arrs);
    } else {
        sv=fnCur->gv("@params");
        if( sv ) sv->reuse();
    }
    char ch=pv.ch();
    if( ch==0 || ch=='#' ) {
        return;
    }
    for(int n=start; pv.valid() && n<256; n++  ) {
        LPCC code = pv.findEnd(",").v();
        if( slen(code)==0 ) break;
        ch=code[0];
        /*
        if( ch=='@' ) {
            XFuncNode* fnParent=fnCur->parent();
            if( fnParent==NULL ) fnParent=fn;
            if( fnParent ) {
                bool ok=true;
                code++;
                sv = n<sz ? arrs->get(n): NULL;
                if( SVCHK('s',0) ) {
                    void* ref=SVO;
                    for(WBoxNode* bn=fnParent->First(); bn; bn=fnParent->Next() ) {
                        StrVar* var=&(bn->value);
                        if( checkFuncObject(var,'s',0) && ref==var->getObject(FUNC_HEADER_POS) ) {
                            int sp=sv->getInt(OBJ_PROP_START_POS), ep=sv->getInt(OBJ_PROP_START_POS+4);
                            if(sp==var->getInt(OBJ_PROP_START_POS) && ep==var->getInt(OBJ_PROP_START_POS+4) ) {
                                fnCur->GetVar(code)->setVar('s',1,(LPVOID)var);
                                break;
                            }
                        }
                    }
                    ok=false;
                }
                if( ok ) {
                    fnParent->GetVar(code)->reuse();
                }
            } else {
                qDebug("#0#parent variable ref error function not define (name:%s)\n", code);
            }
        } else
        */
        if( ch=='&' ) {
            code++;
            if( n<sz ) {
                sv = arrs->get(n);
                if( sv->charAt(0)=='\b' ) {
                    fnCur->GetVar(code)->reuse()->add(sv);
                } else if( sv ) {
                    int sp=0, ep=sv->length();
                    fnCur->GetVar(code)->setVar('s',0, (LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                } else {
                    fnCur->GetVar(code)->reuse();
                }
            } else {
                fnCur->GetVar(code)->reuse();
            }
        } else {
            if( n<sz )
                fnCur->GetVar(code)->reuse()->add(arrs->get(n));
            else
                fnCur->GetVar(code)->reuse();
        }
    }
}

bool callObjectFunc(LPCC funcName, StrVar* cur, XFuncNode* fn, StrVar* rst, XFunc* func) {
    if( funcName==NULL ) funcName=func->getFuncName();
    if( (func->xfflag & FFLAG_SAVE) ) {
        bool bObject= cur->charAt(0)=='\b' && !checkFuncObject(cur,'s',0);
        if( bObject && func->xfuncRef<800 ) {
            qDebug("callObjectFunc [%s, %d] may be type change \n", funcName, func->xfuncRef);
            func->xfflag&=~FFLAG_SAVE;
            func->xfuncRef=0;
        }
    }
    if( func->xfuncRef==0  ) {
        if( checkFuncObject(cur,'n',0) ) {
            regNodeFuncs();
            U16 ref = getHashKeyVal("node", funcName );
			if( ref>980 ) {
                func->xfuncRef=ref;
            }
        } else if( checkFuncObject(cur,'a',0) ) {
            if( ccmp(funcName,"with") ) {
                func->xfuncRef=991;
            } else if( ccmp(funcName,"inject") ) {
                func->xfuncRef=992;
            }
        } else if( checkFuncObject(cur,'x',21) ) {
            if( ccmp(funcName,"inject") ) {
                func->xfuncRef=992;
            }
        } else if( checkObject(cur,'i') ) {
            if( ccmp(funcName,"inject") ) {
                func->xfuncRef=992;
            }
        }
    }
    bool rtn=false;
    XListArr* arrs=( (func->xfuncRef==891) || (func->xfuncRef> 980) ) ? NULL : getParamArray(func,fn,rst);
    if( execObjectFunc(func,arrs,fn,cur,rst,NULL, funcName) ) {
        rtn=true;
    }
    releaseLocalArray(arrs);
    return rtn;
}



bool callModuleFuncCheck(TreeNode* tn, LPCC fnm, XListArr* arrs, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->gv(fnm);
    if( SVCHK('f',1) || SVCHK('f',0) ) {
        if( execMemberFunc(sv,arrs,rst,fn,tn,fnm) ) {
            if( fc ) fc->xflag|=FFLAG_SAVE;
            releaseLocalArray(arrs);
            return true;
        }
    }
    releaseLocalArray(arrs);
    return false;
}

bool callAutomataFunc(XFunc* fc, XListArr* arrs, XFuncNode* fn, StrVar* rst, StrVar* var) {
    if( fc->xfuncRef==0 ) {
        LPCC fnm=fc->getFuncName();
        fc->xfuncRef =
            ccmp(fnm,"keyState") ? 1 : ccmp(fnm,"keyPush") ? 2 :
            ccmp(fnm,"toString") ? 3 :
            ccmp(fnm,"value") ? 4 : ccmp(fnm,"getSize") ? 5 :
            ccmp(fnm,"mode") ? 6 :
            ccmp(fnm,"clear") ? 7 :
            ccmp(fnm,"ingString") ? 8 : 0;
    }
    StrVar* sv=var;
    CKoreanAutomata* automata=(CKoreanAutomata*)SVO;
    switch( fc->xfuncRef ) {
    case 1: {	// keyState
        /* 	HS_START, HS_CHOSUNG, HS_JOONGSUNG, HS_DJOONGSUNG, HS_JONGSUNG, HS_DJONGSUNG, HS_END1, HS_END2 */
        int stat=automata->State();
        rst->setVar('0',0).addInt(stat);
    } break;
    case 2: {	// keyPush
        if( arrs==NULL ) return false;
        LPCC str=AS(0);
        if( slen(str)>0 ) {
            char key=str[0];
            automata->KeyPush((int)key);
        }
    } break;
    case 3: {	// toString
        if( arrs==NULL ) return false;
        sv=arrs->get(0);
        if( !SVCHK('n',0) ) {
            return false;
        }
        TreeNode* node=(TreeNode*)SVO;
        LPCC done=AS(1);		if( slen(done)==0 ) done="doneString";
        LPCC doing=AS(2);		if( slen(doing)==0 ) doing="doingString";
        LPCC str=toString( node->GetVar("ingString") );
        int n=0, sz=slen(str);

        automata->ClearAll();
        for( ; n<sz; n++ ) {
            automata->KeyPush((int)str[n]);
        }
        int stat = automata->State();
        LPCC txt = automata->GetStringBuffer();
        unsigned short code = automata->GetConvertedUnionCode();
        WCHAR buff = (WCHAR)(code & 0xFFFF);

        // int size = automata->GetStringSize();
        StrVar* var=cfVar(NULL);
        if( slen(txt) ) {
            n=0;
            if( stat==1 ) {
                n=sz-1;
            } else if( stat==2 ) {
                n=sz-2;
            } else if( stat==3 || stat==4 ) {
                n=sz-3;
            } else if( stat==5 ) {
                n=sz-4;
            }
            if( n>0 ) {
                rst->reuse();
                for( ; n<sz; n++ ) {
                    rst->add(str[n]);
                }
                node->GetVar("ingString")->reuse()->add(rst);
            }
            var->add(txt);
            C2UTF(var->get(),var->length(),rst->reuse(),1);
            node->GetVar(done)->add(rst);
        }
        if( stat ) {
            var->reuse()->add((LPCC)(&buff), sizeof(buff));
            sv=node->GetVar(doing);
            C2UTF(var->get(),var->length(),sv,1);
        }
    } break;
    case 4: {	// value
        LPCC str= gBuf.add( automata->GetStringBuffer() );
        C2UTF( (char*)str,slen(str),rst,1);
    } break;
    case 5: {	// getSize
        int size=automata->GetStringSize();
        rst->setVar('0',0).addInt(size);
    } break;
    case 6: {	// mode
        if( arrs ) {
            LPCC ty=AS(0);
            KEYINPUTMODE mode=ccmp(ty,"eng") ? ENG_INPUT: KOR_INPUT;
            automata->ChangeKeyInputMode(mode);
        } else {
            int mode=(int)automata->GetNowMode();
            rst->set( mode==0 ? "eng" : "kor");
        }
    } break;
    case 7: {	// clear
        automata->ClearAll();
    } break;

    default: return false;
    }
    return true;
}

void setUseVars(XFunc* func, XFuncNode* fn, StrVar* rst) {
    int cnt=func? func->getParamSize(): 0;
    if(cnt==0 ) return;
    StrVar* sv=NULL;
    LPCC var=NULL;
    TreeNode* thisNode=NULL;
    XFuncSrc* fsrc=func->getFuncSrc();
    XFunc* pfc=fsrc?fsrc->xfunc: NULL;
    U16 fflag=0;
    if( pfc ) {
        fflag=pfc->xflag;
#ifdef INLINENODE_USE
        if( fflag&FUNC_INLINE ) {
            sv=fn->gv("@inlineNode");
        }
#endif
    }
    if( sv==NULL ) {
        sv=fn->gv("@this");
    }
    if( SVCHK('n',0) ) {
        thisNode=(TreeNode*)SVO;
    }

    bool bref=false;
    for( int n=0;n<cnt;n++ ) {
        XFunc* param = func->getParam(n);
        if( param->xdotPos>0 ) {
            XFuncNode* fnInit=NULL;
            TreeNode* obj=NULL;
            LPCC objName=param->getValue(), var=NULL;
            int pos=IndexOf(objName,'.');
            sv=thisNode? thisNode->gv("onInit"): NULL;
            if( SVCHK('f',0) ) {
                fnInit=(XFuncNode*)SVO;
            }
            for( int xx=0; xx<10 ; xx++) {
                sv=NULL;
                var=gBuf.add(objName,pos);
                objName+=pos+1;
                if( xx==0 ) {
                    if( ccmp(var,"this") ) {
                        obj=thisNode;
                    } else {
                        sv=getFuncVar(fn->parent(),var);
                        if( sv==NULL) sv=fnInit ? fnInit->gv(var): NULL;
                    }
                } else if( obj) {
                  sv=obj->gv(var);
                  if( sv==NULL ) {
                      sv=obj->gv("onInit");
                      if( SVCHK('f',0) ) {
                          fnInit=(XFuncNode*)SVO;
                          sv=fnInit->gv(var);
                      }
                  }
                } else {
                    qDebug("#0#use function object not define var:%s (value:%s)\n", var, func->getValue() );
                    break;
                }
                if( SVCHK('n',0) ) {
                    obj=(TreeNode*)SVO;
                }
                pos=IndexOf(objName,'.');
                if( pos>0 ) {

                } else {
                    if(obj ) {
                        var=objName;
                        sv=obj->gv(var);
                        if( sv==NULL ) {
                            sv=obj->gv("onInit");
                            if( SVCHK('f',0) ) {
                                fnInit=(XFuncNode*)SVO;
                                sv=fnInit->gv(var);
                            }
                        }
                    }
                    break;
                }
            }
            if( sv) {
                fn->GetVar(var)->reuse()->add(sv);
            }
        } else {
            XFuncNode* fnInit=NULL;
            sv=thisNode? thisNode->gv("onInit"): NULL;
            if( SVCHK('f',0) ) {
                fnInit=(XFuncNode*)SVO;
            }
            var=param->getValue();
            if( var[0]=='&' ) {
                bref=true;
                var++;
            }
            LPCC code=var;
            int pos=IndexOf(var,':');
            if( pos>0 ) {
                code=gBuf.add(var,pos);
                var+=pos+1;
            }
            if(fn && fn->gv(code) ) {
                continue;
            }
            sv=NULL;
            if( fflag&FFLAG_EQ ) {
                /* a.inline() */
                sv=thisNode->gv(code);
                if(sv==NULL) {
                    sv=getFuncVar(fn->parent(),var);
                    thisNode->GetVar(code)->add(sv);
                }
            } else {
                XFuncNode* fnParent=fn->parent();
                if(fnParent && fnParent==fnInit ) {
                    sv=fn->gv("@funcNode");
                    if(SVCHK('f',0)) {
                        XFuncNode* fnCur=(XFuncNode*)SVO;
                        sv=fnCur->gv(var);
                    } else {
                        sv=NULL;
                    }
                }
                if( sv==NULL ) {
                    sv=getFuncVar(fnParent,var);
                    if( sv==NULL && thisNode ) {
                        sv=thisNode->gv(var);
                        if(sv==NULL && fnInit) sv=fnInit->gv(var);
                    }
                }

                if( bref && sv ) {
                    int len=sv->length();
                    fn->GetVar(code)->setVar('s',0,(LPVOID)sv).addInt(0).addInt(len).addInt(0).addInt(len);
                } else {                    
                    fn->GetVar(code)->reuse()->add(sv);
                }
            }
        }
    }
}
bool setMemberSource(TreeNode* node, XFuncNode* fn, StrVar* src, int sp, int ep, LPCC funcLine) {
    StrVar* rst=getStrVarTemp();
    if( slen(funcLine)==0 ) funcLine="inline";
    if( setModifyClassFunc(node, src, sp, ep, fn, rst, false, funcLine) ) {
        // node->setNodeFlag(FLAG_PERSIST);
        return true;
    }
    return false;
}

int setFuncSource(StrVar* var, int sp, int ep, LPCC funcLine, bool bset, bool bevent) {
    LPCC funcNm=NULL, param=NULL;
    bool sfunc=false;
    TreeNode* funcInfo=cfNode("@funcInfo");
    // StrVar* prefix=funcInfo->gv("prefix");
	bool refCheck=false, refName=false;
    LPCC ref=funcInfo->get("ref");
    XParseVar pv(var, sp, ep);
    LPCC includeFileName=funcInfo->get("includeFileName");
    TreeNode* funcMap=slen(includeFileName)? getTreeNode("@inc","userFunc"): NULL;
	if(slen(ref)==0) {
		ref=funcInfo->get("refName");
		if(slen(ref)>0) refName=true;
	}
    char c=pv.ch();
    if( c=='/' ) {
        c=pv.ch(1);
        if( c=='/') {
            pv.findEnd("\n");
        } else if( c=='*' ) {
            pv.match("/*", "*/", FIND_SKIP);
        }
        c=pv.ch();
    }
    int sa=pv.start, ea=sa;
    if( c=='@' ) {
        pv.incr();
        sa++;
        sfunc=true;
        refCheck=true;
    }
    pv.moveNext();
    ea=pv.start;
    c=pv.ch();
    if( c=='-' || c=='#' ) {
        while( c=='-' || c=='#' ) {
            c=pv.incr().moveNext().ch();
        }
        ea=pv.start;
    }
	LPCC funcGroup=refCheck || refName ? ref: NULL; //, funcName=NULL;
    if( pv.ch()=='.' ) {
        pv.incr();
        int fa=pv.start,fb=0;
        c=pv.moveNext().ch();
        if( c=='-' || c=='#' ) {
            while( c=='-' || c=='#' ) {
                c=pv.incr().moveNext().ch();
            }
        }
        fb=pv.start;
        if( fa<fb ) {
            funcGroup=pv.Trim(sa, ea);
            funcNm=pv.Trim(fa,fb);
        }
    } else if(sa<ea) {
        funcNm=pv.Trim(sa, ea);
    }
    if( slen(funcNm)==0 ) {        
        return 0;
    }
    // qDebug("funcGroup => %s", funcGroup);
    if( slen(funcGroup) ) {
        sfunc=true;
        // if( ccmp(funcGroup,ref) ) funcName=funcNm;
        if(IndexOf(funcGroup,':')>0 ) {
            funcNm=FMT("%s#%s", funcGroup, funcNm  );
        } else {
            funcNm=FMT("%s:%s", funcGroup, funcNm  );
        }
    }
    if( pv.ch()=='(' && pv.match("(",")",FIND_SKIP) ) {
        param=pv.v();
    }
    XFuncSrc* fsrc=NULL;
    qDebug("#0# function add %s \n", funcNm);
    if( funcMap && slen(includeFileName) ) {
        funcMap->set(funcNm, includeFileName);
    }
    // qDebug(">> setFuncSourcr funcNm=>%s [%c]\n", funcNm, pv.ch());
    if( pv.ch()=='{' ) {
        StrVar* sv=NULL;
        while( pv.valid() ) {
            if( pv.ch()!='{' ) break;
            if( pv.match("{","}",FIND_SKIP)==false ) {
                qDebug("#9#function match error ... %s (%s) ", funcNm, param);
                break;
            }
            bool reset=false, binline=false;
            if( sfunc ) {
                sv = getStrVar("subfunc", funcNm);  // getSubFuncSrc(funcNm,"");
            } else {
                sv = getStrVar("fsrc", funcNm );
            }
            if( SVCHK('f',1) ) {
                fsrc = (XFuncSrc*)SVO;
                if( ccmp( funcLine,"append") ) {
                    reset=false;
                } else {
                    reset=true;
                }
            } else {
                fsrc=gfsrcs.getFuncSrc();
            }
            if( fsrc==NULL ) {
                break;
            }
            if( bset || reset ) {
                fsrc->reuse();
                fsrc->xparam.reuse();
                fsrc->xflag=0;
                if( fsrc->xfunc ) {
                    if( fsrc->xfunc->xflag & FUNC_INLINE ) {
                        binline=true;
                    }
                    deleteAllFunc(fsrc->xfunc);
                    fsrc->xfunc=NULL;
                }
            }
            if( slen(param) ) {
                fsrc->xparam.set(param);
            }
            /*
            if( prefix && prefix->length() ) {
                qDebug("## setFuncSource %s (prefix:%d)",funcNm, prefix->length() );
                fsrc->readBuffer(prefix, 0, prefix->length() );
            }
            */
            fsrc->readBuffer(pv.GetVar(), pv.prev, pv.cur);
            if( fsrc->makeFunc() && fsrc->xfunc ) {
                if( binline ) {
                    fsrc->xfunc->xflag |= FUNC_INLINE;
                } else if( funcLine ) {
                    if( sfunc ) {
                        if( ccmp(funcLine,"common") ) {
                            fsrc->xflag|=0x800 | 0x400;
                        } else if( ccmp(funcLine,"sub") ) {
                            fsrc->xflag|=0x800;
                        } else if( ccmp(funcLine,"inline") || ccmp(funcLine,"append") ) {
                            if( fsrc->xfunc ) {
                                fsrc->xfunc->xflag|=FUNC_INLINE;
                            }
                            fsrc->xflag|=0x800;
                        }                        
                    } else if( ccmp(funcLine,"inline") && fsrc->xfunc ) {
                        qDebug("#0#inline func add (name:%s)\n", funcNm);
                        fsrc->xflag|=FUNC_INLINE;
                    }
                }

                if( reset==false && bset==false ) {
                    if( sfunc ) {
                        if( bevent ) fsrc->xflag|=0x800;
                        getStrVar("subfunc", funcNm, 'f',1,(LPVOID)fsrc );
                        /*
                        if( funcName ) {
                            sv = getStrVar("fsrc", funcName );
                            if( !SVCHK('f',1) ) {
                                getStrVar("fsrc", funcName, 'f',1,(LPVOID)fsrc );
                            }
                        }
                        */
                    } else {
                        getStrVar("fsrc", funcNm, 'f',1,(LPVOID)fsrc );
                    }
                }
            } else {
                qDebug("#0## %s function make fail\n", funcNm);
            }
            c=pv.ch();
            if( bset || c==0 ) {
                break;
            }
            if( c==';' ) {
                c=pv.incr().ch();
            }
            if( c=='/' ) {
                c=pv.ch(1);
                if( c=='/') {
                    pv.findEnd("\n");
                } else if( c=='*' ) {
                    pv.match("/*", "*/", FIND_SKIP);
                }
                pv.ch();
            }
            if(pv.ch()=='@') {
                pv.incr();
                refCheck=true;
			} else {
				refCheck=false;
			}
            funcNm=NULL, param=NULL;
			funcGroup=refCheck || refName ? ref: NULL; //, funcName=NULL;
            sa=pv.start;
            pv.moveNext();
            ea=pv.start;
            c=pv.ch();
            if( c=='-' || c=='#' ) {
                while( c=='-' || c=='#' ) {
                    pv.incr().moveNext();
                }
                ea=pv.start;
            }
            if( pv.ch()=='.' ) {
                int fa=0,fb=0;
                pv.incr();
                fa=pv.start;
                c=pv.moveNext().ch();
                if( c=='-' || c=='#' ) {
                    while( c=='-' || c=='#' ) {
                       c=pv.incr().moveNext().ch();
                    }
                }
                fb=pv.start;
                if( fa<fb ) {
                    funcGroup=pv.Trim(sa, ea);
                    funcNm=pv.Trim(fa,fb);
                }
            } else {
                funcNm=pv.Trim(sa, ea);
            }
            sfunc=false;
            if( slen(funcGroup) ) {
                sfunc=true;
                // if( ccmp(funcGroup,ref) ) funcName=funcNm;
                if(IndexOf(funcGroup,':')>0 ) {
                    funcNm=FMT("%s#%s", funcGroup, funcNm );
                } else {
                    funcNm=FMT("%s:%s", funcGroup, funcNm );
                }
            }

            if( pv.ch()=='(' && pv.match("(",")",FIND_SKIP) ) {
                param=pv.v();
            }
            if( funcMap ) {
                funcMap->set(funcNm, includeFileName);
            }
            qDebug("#0#function add %s (%s ch:%c)\n", funcNm, param, c);
        }
    } else {
        // qDebug("#9#%s function start error", funcNm);
    }
    if( funcInfo ) {
        // if(prefix ) prefix->reuse();
        if(slen(ref)) funcInfo->GetVar("ref")->reuse();
    }
    return 1;
}

//#> func src
StrVar* getSubFuncSrcSqlite(LPCC funcCode) {
    DbUtil* db = getFuncsDb();
    StrVar* sv= NULL;
    LPCC val=NULL, funcNm=NULL;
    int pos=IndexOf(funcCode,':');
    if( pos>0 ) {
        val=funcCode+pos+1;
        funcNm=gBuf.add(funcCode,pos);
    } else {
        return NULL;
    }
    // gMutexSubFunc.EnterMutex();
    if( db->prepare("select funcname, funcparam, funcdata, funcline from subFunc where funcRef=? and funcName=? ") ) {
        db->bindStr(funcNm).bindStr(val).exec();
        if( db->fetch() ) {
            XFuncSrc* fsrc = gfsrcs.getFuncSrc();
            StrVar* funcData=db->GetVar("funcData");
            fsrc->readBuffer(funcData,0,funcData->length() );
            if( fsrc->makeFunc() && fsrc->xfunc ) {
                LPCC param = db->get("funcParam");
                LPCC funcLine= db->get("funcLine");
                if( ccmp(funcLine,"interface") ) {
                    fsrc->xflag|=0x400;
                } else if( ccmp(funcLine,"common") ) {
                    fsrc->xflag|=0x800 | 0x400;
                } else if( ccmp(funcLine,"sub") ) {
                    fsrc->xflag|=0x800;
                }
                if( slen(param) ) {
                    fsrc->xparam.set(param);
                    fsrc->xflag|=0x1000;
                }
                sv=getStrVar("subfunc", funcCode, 'f',1,(LPVOID)fsrc );
                // gMutexSubFunc.LeaveMutex();
            } else {
                gfsrcs.deleteFuncSrc(fsrc);
                fsrc=NULL;
            }
        }
    }
    // gMutexSubFunc.LeaveMutex();
    return sv;
}

StrVar* getSubFuncSrc(LPCC funcNm, LPCC val, bool sqliteUse ) {
	gMutexSubFunc.EnterMutex();
	StrVar* var=NULL;
	if( slen(val) ) {
		char funcCode[256];
		sprintf(funcCode,"%s:%s",funcNm, val);
		//LPCC fid=gBuf.fmt("%s:%s",funcNm, val);
		var=getStrVar("subfunc", funcCode);
	} else {
		var=getStrVar("subfunc", funcNm);
    }	
	gMutexSubFunc.LeaveMutex();
	return var;
}

StrVar* getCmsFuncSrc(LPCC funcNm ) {
    DbUtil* db = getFuncsDb();
    StrVar* sv= getStrVar("fsrc", funcNm);
    if( SVCHK('f',1) ) {
        return sv;
    }
    if( db->prepare("select funcName, funcParam, funcData, type from cmsFunc where cmsCode='common' and funcName=? and useyn='Y'") ) {
        db->bindStr(funcNm).exec();
        if( db->fetch() ) {
            XFuncSrc* fsrc = gfsrcs.getFuncSrc();
            StrVar* funcData=db->GetVar("funcData");
            fsrc->parseData(funcData,0,funcData->length() );
            if( fsrc->makeFunc() && fsrc->xfunc ) {
                LPCC param = db->get("funcParam");
                fsrc->xflag|=0x800 | 0x400;
                if( slen(param) ) {
                    fsrc->xparam.set(param);
                }
                sv=getStrVar("fsrc", funcNm, 'f', 1, (LPVOID)fsrc );
            } else {
                gfsrcs.deleteFuncSrc(fsrc);
                fsrc=NULL;
            }
        } else {
            qDebug("#0#func load fail : %s\n", funcNm);
        }
    }
    return sv;
}


//#> check func
bool isCheckFunc(XFunc* fc ) {
    return fc && fc->xftype>FTYPE_FUNCSTART && fc->xftype<FTYPE_FUNCEND ? true : false;
}
bool isFuncType(XFunc* func) { return func==NULL? false : (func->xftype>FTYPE_FUNCSTART && func->xftype<FTYPE_FUNCEND); }

bool funcTrueCheck(XFunc* fc, XFuncNode* fn, StrVar* rst ) {
    int cnt=fc?fc->xparams.size():0;
    if( cnt==0 || rst==NULL ) {
        return false;
    }
    bool bchk = false;
    rst->reuse();
    if( cnt==1 ) {
        bchk=isFuncTrue(fc->getParam(0),fn,rst);
        if( fc->xfkind==10 ) bchk=!bchk;
    } else {
        for( int n=0;n<cnt; n++) {
            XFunc* fcParam=fc->getParam(n);
            bchk=isFuncTrue(fcParam,fn,rst );
            if( fc->xfkind==10 ) {
                if(bchk) {
                    bchk=false;
                    break;
                } else {
                    bchk=true;
                }
            } else {
                if(!bchk) break;
            }
        }
    }
    return bchk;
}
bool isFuncTrue(XFunc* fcParam, XFuncNode* fn, StrVar* rst ) {
    if( fcParam==NULL || rst==NULL ) {
        return false;
    }
    bool ret=false;
    int cnt=fcParam->getParamSize();
    XFunc* fcOper=NULL;
    XFunc* fcSub=NULL;
    if( isFuncType(fcParam) && cnt>0 ) {
        if( fcParam->getParam(0)->xftype==FTYPE_OPER ) {
            fcOper=fcParam->getParam(0);
        } else {
            fcSub=fcParam->getParam(cnt-1);
            if( fcSub && fcSub->xfflag&0x10 ) {
                fcOper=fcSub->getParam(0);
            }
        }
    } else if( (fcParam->xfflag&0x800)==0 ) {
        fcOper=fcParam->getParam(0);
    }
    rst->reuse();
    if( fcOper && fcOper->xftype==FTYPE_OPER ) {
        if( fcSub) {
            fcSub->xfflag|=0x800;
            execFunc(fcParam,fn,rst);
            fcSub->xfflag&=~0x800;
            getOperValue(fn, fcSub, fcSub->getParamSize(), rst);
        } else {
            if(fcParam->xftype!=FTYPE_SINGLEVAR) {
                execFunc(fcParam,fn,rst);
            }
            getOperValue(fn, fcParam, fcParam->getParamSize(), rst);
        }
        ret=isVarTrue(rst);
    } else if( fcParam->xftype==FTYPE_VAR ) {
        LPCC code=fcParam->getValue(), param=NULL;
        int pos=IndexOf(code,'.');
        if( pos>0 ) {
            param=code+pos+1;
            code=gBuf.add(code,pos);
        }
        StrVar* sv=getFuncVar(fn, code);
        if( param ) {
            sv=SVCHK('n',0)? getSubVarValue((TreeNode*)SVO, param): NULL;
        }
        if( sv ) {
            if( sv->charAt(0)=='\b' ) {
                switch(sv->charAt(1) ) {
                case '0': {
                    if( sv->getInt(FUNC_HEADER_POS) ) ret=true;
                } break;
                case '1': {
                    if( sv->getUL64(FUNC_HEADER_POS) ) ret=true;
                } break;
                case '2': {
                    if( sv->getU32(FUNC_HEADER_POS) ) ret=true;
                } break;
                case '3': {
                    if( sv->getU16(2) ) ret=true;
                } break;
                case '4': {
                    if( sv->getDouble(FUNC_HEADER_POS) ) ret=true;
                } break;
                case '9': {
                    ret=false;
                } break;
                case 's': {
                    if( sv->getU16(2)==0 ) {
                        int s=sv->getInt(OBJ_PROP_START_POS), e=sv->getInt(OBJ_PROP_START_POS+4);
                        if( s<e ) ret=true;
                    }
                } break;
                default: {
                    ret=true;
                } break;
                }
            } else if(sv->length() ) {
                ret=true;
            }
        }
    } else {
        execFunc(fcParam,fn,rst);
        ret=isVarTrue(rst);
    }
    return ret;
}
bool isVarType(XFunc* func) {
    if( func==NULL ) return false;
    return func->xftype<FTYPE_FUNCSTART || func->xftype>FTYPE_FUNCEND  ? true: false;
}
bool isStringVar( StrVar* sv ) {
    if( sv==NULL || (sv->length()==0) ) return false;
    if( sv->charAt(0)=='\b' ) {
        if( sv->charAt(1)=='s' ) {
            return true;
        }
        return false;
    }
    return true;
}
bool isTrueCheck(XFunc* fc) {
    bool bchk= fc->xfflag==0 || fc->xfflag&(FFLAG_PSET|FFLAG_PARAM);
    if( bchk ) {
        XFunc* pfc=fc->parent();
        if( pfc && pfc->getType()==FTYPE_ARRAY ) {
            pfc = pfc->parent();
        }
        if( pfc ) {
            U32 ftype=pfc->getType();
            if( fc->xfflag&FFLAG_PSET && pfc->xparent && pfc->xparent->getType()==FTYPE_CONTROL) {
                // qDebug("xxxx isTrueCheck ok xxxx");
            } else if( ftype==FTYPE_CONTROL || ftype==FTYPE_FUNCCHECK ) {
                bchk=true;
            } else if( pfc->getParam(0) && pfc->getParam(0)->getType()==FTYPE_OPER) {
                bchk=true;
            } else {
                bchk=false;
            }
        }
    }
    return bchk;
}
LPCC getBaseFuncName(XFuncNode* fn) {
    LPCC fnm=fn->get("_funcName_");
    if(slen(fnm)==0) fnm=getCf()->get("@currentFuncName");
    return fnm;
}
void releaseLocalArray(XListArr* arrs) {
    if( arrs==NULL ) return;
    if( arrs->xflag&FLAG_LOCALARRAY_SET ) {
        arrs->xflag&=~FLAG_LOCALARRAY_SET;
    }
}
unsigned long mixNum(unsigned long a, unsigned long b, unsigned long c) {
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}

void printCallLog(LPCC msg) {

}
void printVar(StrVar* sv, LPCC var, XFuncNode* fn, StrVar* rst) {
    if( SVCHK('n',0) ) {
        TreeNode* node=(TreeNode*)SVO;
        rst->add(var).add("={\n");
        printBox(node, rst, 1);
        rst->add("\n}\n\n");
    } else if( SVCHK('a',0) ) {
        rst->add(var).add("=");
        XListArr* arr = (XListArr*)SVO;
        rst->add("[");
        if( arr ) {
            for( int n=0,size=arr->size();n<size;n++) {
                if( n>0 ) rst->add(", ");
                rst->add(toString(arr->get(n)));
            }
        }
        rst->add("]\n");
    } else {
        rst->add(var).add("=").add(toString(sv));
    }
}

void printBox(WBox* box, StrVar* rst, int depth) {
    if( box==NULL )
        return;
    if( depth>3 ) return;
    WBoxNode* bn = box->First();
    StrVar var;
    char tab[16];
    int x=0;
    if( depth<2 ) {
        for(;x<depth;x++) {
            tab[x]='\t';
        }
    }
    tab[x]='\0';
    int num = 0;
    if( depth==0 ) rst->add("\n");
    StrVar* sv=box->gv("@this");
    XFuncNode* fnInit=NULL;

    int sp=0, ep=0;
    XParseVar pv;
    for(int n=0; n<1024 && bn; n++ ) {
        LPCC code=box->getCurCode();
        if( code[0]=='@' && fnInit ) {
            if( IndexOf(code,'.')>0 ) {
                continue;
            } else {
                LPCC cd=code+1;
                sv=fnInit->gv(cd);
                if( sv ) {
                    continue;
                }
                }
        }
        // ### version 1.0.4
        if( StartWith(code,"@__") || ccmp(code,"@keys") || ccmp(code,"@cms") || ccmp(code,"@this") || ccmp(code,"@page") ||
            ccmp(code,"@timers") || ccmp(code,"@parent") || ccmp(code,"@findId") || ccmp(code,"@proxy") || ccmp(code,"@model") ||
            ccmp(code,"@widget") || ccmp(code,"@page")
        ) {
            bn = box->Next();
            continue;
        }
        if( num!=0 ) {
            if( depth<2) {
                rst->add(tab).add("\r\n");
            } else {
                rst->add(", ");
                if( n>0 && n%5==0 ) rst->add(tab).add("\r\n\t\t");
            }
        }
        num++;
        rst->add(tab).add(code);
        rst->add("=");
        {
            if( bn->value.charAt(0)=='\b' ) {
                sv = NULL;
                switch( bn->value.charAt(1) ) {
                case 's': {
                    sv=getStrVarPointer(&(bn->value), &sp, &ep );
                    pv.SetVar(sv, sp, ep );
                    if( pv.ch() ) {
                        rst->add( pv.findEnd("\n").v() );
                    } else {
                        rst->add(toString(&(bn->value)));
                    }
                } break;
                case 'd':
                case 'n':
                case '&': {
                    StrVar* var = &(bn->value);
                    if( var->getU16(2)==0 ) {
                        WBox* node = (WBox*)var->getObject(FUNC_HEADER_POS);
                        if( depth<3 ) {
                            if( depth==0 ) {
                                rst->add("{\n");
                                printBox(node,rst,depth+1);
                                rst->add("\n};\n");
                            } else {
                                rst->add("{\n\t").add(tab);
                                printBox(node,rst,depth+1);
                                rst->add("\n").add(tab).add("};\n");
                            }
                        } else
                            rst->add("{@node}");
                    }
                } break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case 'i': {
                    rst->add(toString(&(bn->value)));
                } break;
                case 'a': {
                    StrVar* var = &(bn->value);
                    if( var->getU16(2)==0 ) {
                        XListArr* arr = (XListArr*)var->getObject(FUNC_HEADER_POS);
                        rst->add("[");
                        if( arr ) {
                            for( int n=0,size=arr->size();n<size;n++) {
                                if( n>0 ) rst->add(", ");
                                rst->add(toString(arr->get(n)));
                            }
                        }
                        rst->add("]\n");
                    }
                } break;
                default: {
                    rst->add(toString(&(bn->value)));
                    // rst->add(bn->value.charAt(1));
                } break;
                }
            } else {
                sv=getStrVarPointer(&(bn->value), &sp, &ep );
                pv.SetVar(sv, sp, ep );
                if( pv.ch() ) {
                    rst->add( pv.findEnd("\n").v() );
                }
            }
        }
        bn = box->Next();
    }
}
void getPrintInfo(LPCC type, XFuncNode* fn, StrVar* rst, PARR arrs) {
    rst->format(64," ############# %s info ##############\n", type);
    if( ccmp(type,"@stack") ) {
#ifdef PUSH_FUNCNAME
        for( int n=0; n<gfuncNames.size(); n++ ) {
            StrVar* sv=gfuncNames.get(n);
            if( n>0 ) rst->add("\n");
            rst->add(">> ").add(sv);
        }
#endif
    } else if( ccmp(type,"@localParams")  ) {
        int cnt=0;
        for( int n=0; n<LOCAL_PARAMS_SIZE; n++ ) {
            XListArr* a=&gParams[n];
            if( a==arrs ) {
                qDebug("xxxxxxxxxxxxxx %d: print param xxxxxxxxxxxxxxxxx\n", n);
                continue;
            }
            if( a->xflag & FLAG_LOCALARRAY_SET ) {
                a->xflag &=~FLAG_LOCALARRAY_SET;
                if( cnt>0 ) rst->add(", ");
                rst->format(32, "%d:%d ", cnt, n);
                cnt++;
            }
        }
        rst->format(64, "\ntotal:%d, all:%d", cnt, LOCAL_PARAMS_SIZE);

    } else if( ccmp(type,"@func")  ) {
        XFuncNode* pfn=fn;
        int x=0;
        for( WBoxNode* bn=pfn->First(); bn; bn=pfn->Next() ) {
            LPCC code=pfn->getCurCode();
            if( x>0 ) {
                rst->add(", ");
                if( x%5==0 ) rst->add("\n\t");
            }
            rst->add(code).add("=>");
            getObjectType(pfn->gv(code), rst);
            x++;
        }
    } else if( ccmp(type,"@funcs")  ) {
        XFuncNode* pfn=fn;
        for( int n=0; n<8 && pfn ; n++ ) {
            if( n>0 ) rst->add("\n");
            rst->format(16,">> %d \n\t", n);
            int x=0;
            for( WBoxNode* bn=pfn->First(); bn; bn=pfn->Next() ) {
                LPCC code=pfn->getCurCode();
                if( x>0 ) {
                    rst->add(", ");
                    if( x%5==0 ) rst->add("\n\t");
                }
                rst->add(code).add("=>");
                getObjectType(pfn->gv(code), rst);
                x++;
            }
            pfn=pfn->parent();
        }
    } else if( ccmp(type,"@funcTotal") ) {
        // U32 dsize=0;
        // nUnitNum , nUnitCount, nGetPos, nCurrent, nCount, nMaxNum
        // total=(nCurrent*nUnitCount)+nCount
        StrVar var;
        gfns.xnodes.getInfo(&var);
        rst->format(256, "XFuncNode : %s (total: %d)\n", var.get(), gfns.xnodes.total() );
        gfsrcs.xnodes.getInfo(&var);
        rst->format(256, "XFuncSrc : %s (total: %d)\n", var.get(), gfsrcs.xnodes.total() );
        gfuncs.xnodes.getInfo(&var);
        rst->format(256, "XFunc : %s (total: %d)\n", var.get(), gfuncs.xnodes.total() );
    }
    rst->add("\n###############################################\n");
}

TreeNode* mergeNode(TreeNode* info, TreeNode* node, U32 flag) {
    if( info==NULL || node==NULL ) return node;
    if( info==node ) return node;
    StrVar* sv=NULL;
    WBoxNode* bn=info->First();
    while( bn ) {
        LPCC code=info->getCurCode();
        sv=node->gv(code);
        if( flag==1 || sv==NULL ) {
            node->GetVar(code)->add(&(bn->value));
        }
        bn = info->Next();
    }
    return node;
}

inline int uriSlashCount(LPCC uri) {
    int cnt=0, len=slen(uri);
    if( len==0 ) return cnt;
    for( int n=0; n<len; n++ ) {
        if( uri[n]=='/' ) cnt++;
    }
    return cnt;
}
StrVar* loadWebPageFunc(LPCC pageCode, TreeNode* uriMap, StrVar* rst ) {
    if( uriMap==NULL ) {
        return NULL;
    }
    StrVar* funcVar=NULL;
    TreeNode* pageNode=NULL;
    LPCC funcName=NULL;
    TreeNode* child=uriMap->child(0);
    funcVar=uriMap->gv(pageCode);

    if( child ) {
        StrVar* sv=NULL;
        int slashCount=uriSlashCount(pageCode);
        int lpos=LastIndexOf(pageCode,'/');
        qDebug("#0#loadWebPageFunc %s start##\n", pageCode);
        if( slashCount==3 ) {
            if( lpos>0) {
                LPCC pcode=gBuf.add(pageCode,lpos);
                sv=child->gv(pcode);
                if( !SVCHK('n',0) ) {
                    int pos=IndexOf(pageCode, '/', slen(pageCode), 1);
                    if( pos>0 ) {
                        pcode=gBuf.add(pageCode,pos);
                        sv=child->gv(pcode);
                    }
                }
            }
        } else {
            sv=child->gv(pageCode);
            if( !SVCHK('n',0) && slashCount==1 &&slen(pageCode)>1 ) {
                LPCC redirect=gBuf.fmt("%s/pages",pageCode);
                qDebug("#### loadWebPageFunc %s redirect:%s ####\n", pageCode, redirect);
                sv=child->gv(redirect);
            }
        }

        if( SVCHK('n',0) ) {
            pageNode = (TreeNode*)SVO;
            if( slashCount==3 && lpos>0 ) {
                funcName=pageCode+lpos+1;
                sv=pageNode->gv(funcName);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrcMain=NULL;
                XFuncSrc* fsrcCur=(XFuncSrc*)SVO;
                funcVar=sv;
                if( checkFuncObject(funcVar,'f',1) ) {
                    fsrcMain=(XFuncSrc*)funcVar->getObject(FUNC_HEADER_POS);
                }
                if( fsrcCur!=fsrcMain ) {
                    if( fsrcCur ) {
                        child->GetVar(pageCode)->setVar('f',1,(LPVOID)fsrcCur);
                    }
                    qDebug("xxxxxxxxxx loadWebPageFunc pageFunc not match xxxxxxxxxxxxxx\n");
                }
            } else {
                qDebug("#0#loadWebPageFunc %s error\n", funcName );
            }
        }
    }
    if( pageNode ) {
        if( rst ) {
            rst->setVar('n',0,(LPVOID)pageNode);
        }
        if( slen(funcName)==0 ) {
            funcName="main";
        }
        if( funcVar==NULL ) {
            funcVar=pageNode->gv(funcName);
        }

    }
    qDebug("loadWebPageFunc %s %s\n", funcName, funcVar? "ok": "not define");
    return funcVar;
}
