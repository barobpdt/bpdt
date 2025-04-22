#include "func_util.h"
#include "../webserver/ConnectNode.h"
#include "../webserver/HttpNode.h"

bool callWasFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->GetVar("@c");
    HttpServerThread* w=NULL;
    if( SVCHK('v',0) ) {
        w=(HttpServerThread*)SVO;
    } else {
        w=new HttpServerThread(tn);
        sv->setVar('v',0,(LPVOID)w);
    }
    int cnt=arrs? arrs->size() : 0;
    U32 ref=fc? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"start")? 1 :
            ccmp(fnm,"startServer")? 2 :
            ccmp(fnm,"databasePool")? 3 :
            ccmp(fnm,"rootNode")? 4 :
            ccmp(fnm,"clientArray")? 5 :
            ccmp(fnm,"stop")? 6 :
            ccmp(fnm,"close")? 7 :
            ccmp(fnm,"rootPath")? 8 :
            ccmp(fnm,"loadPage")? 9 :
            ccmp(fnm,"session")? 10 :
            ccmp(fnm,"errorInfo")? 11 :
            ccmp(fnm,"config")? 12 :
            ccmp(fnm,"statusInfo")? 13 :
            ccmp(fnm,"funcUpdate")? 14 :
            ccmp(fnm,"pageNode")? 15 :
            ccmp(fnm,"urlMap")? 16 :
            ccmp(fnm,"loadWebPage")? 17 :
            ccmp(fnm,"isRun")? 18 :
            ccmp(fnm,"selectTimeout")? 19 :
            ccmp(fnm,"pageMap")? 20 :
            ccmp(fnm,"shutdown")? 21 :
            ccmp(fnm,"db")? 22 :
            ccmp(fnm,"configFlag")? 23 :
            ccmp(fnm,"uri")? 24 :
            ccmp(fnm,"mapArray")? 25 :
            ccmp(fnm,"is")? 26 :
            ccmp(fnm,"uriMap")? 27 :
            ccmp(fnm,"proxyReq")? 28 :
            ccmp(fnm,"proxyRes")? 29 :
            ccmp(fnm,"execPage")? 30 :
            0;
        if( fc ) fc->xfuncRef=ref;
    }
    switch( ref ) {
    case 1: { // start
        int cnt=arrs ? arrs->size() : 0;
        LPCC rootPath=NULL;
        LPCC port=toString(confVar("was.port"));
        if( cnt>0 ) {
            port=AS(0);
        }
        if( cnt>1 ) {
            LPCC path=getFullFileName(AS(1));
            QFileInfo info(KR(path));
            if( info.isDir() ) {
                rootPath=path;
            }
        }
        int portNum=_intVal(port, 80);
        if( slen(rootPath)==0 ) {
            rootPath=toString(confVar("was.path"));
            if( slen(rootPath)==0 ) rootPath="data/web";
        }
        QFileInfo info(KR(rootPath));
        if( !info.isDir() ) {
            qDebug("#9#web root path error(path:%s, port:%d)", rootPath, portNum);
            return true;
        }
        ConnectNode* root=w->xroot;
        if(root) {
            root->set("serverType","was");
            tn->setInt("port", portNum);
            tn->set("rootPath",rootPath);
            sv=arrs->get(2);
            if(isNumberVar(sv) ) {
                root->GetVar("@timeout")->reuse()->add(sv);
            }
            w->startHttpServer();
            rst->setVar('3', tn->isNodeFlag(FLAG_RUN)? 1: 0 );
        }
    } break;
    case 2: { // startServer
        if(cnt==0) return true;
        // port, (class), callback, (serverType), timeout
        ConnectNode* root=w->xroot;
        int port=8093;
        sv=arrs->get(0);
        if(isNumberVar(sv)) {
            port=toInteger(sv);
        }
        int idx=1;
        sv=arrs->get(idx++);
        TreeNode* thisNode=NULL;
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(idx++);
        }
        if(SVCHK('f',1) && root ) {
            StrVar* var=root->GetVar("@clientCallback")->reuse();
            var->add(sv);
            if(thisNode) {
                sv=thisNode->gv("@useClass");
                if(SVCHK('3',1)) {
                    root->GetVar("@classModule")->setVar('n',0,(LPVOID)thisNode);
                }
            }
        }
        if(root ) {
            LPCC serverType="proxy";
            sv=arrs->get(idx++);
            if(isNumberVar(sv) ) {
                root->GetVar("@timeout")->reuse()->add(sv);
            } else {
                serverType=toString(sv);
                sv=arrs->get(idx++);
                if(isNumberVar(sv) ) {
                    root->GetVar("@timeout")->reuse()->add(sv);
                }
            }
            if(slen(serverType) ) root->set("serverType", serverType);
            tn->setInt("port", port);
            w->startHttpServer();
        }
        rst->setVar('3', tn->isNodeFlag(FLAG_RUN)? 1: 0 );
    } break;
    case 3: { // databasePool
        if( cnt==0 ) {
            sv=tn->gv("@databasePool");
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                for( int n=0; n<a->size(); n++ ) {
                    sv=a->get(n);
                    if( SVCHK('n',0) ) {
                        TreeNode* model=(TreeNode*)SVO;
                        model->clearNodeFlag(FLAG_USE);
                    }
                }
                rst->setVar('a',0,(LPVOID)a);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        LPCC dsn=AS(0);
        // LPCC dbms=AS(1);
        int num=isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): 2;
        if( slen(dsn) ) {
            XListArr* a=tn->addArray("@databasePool");
            int idx = a->size() + 1;
            for( int n=0;n<num;n++) {
                LPCC inode=gBuf.fmt("%s_%d", dsn, idx++ );
                TreeNode* model=getTreeNode("dbms", inode);
                getModelDatabase(dsn, inode, model);
                a->add()->setVar('n',0,(LPVOID)model);
                setClassTagObject("db",inode);
            }
            rst->setVar('a',0,(LPVOID)a );
        } else {
            qDebug("#9#database make pool error (dsn: %s)\n", dsn );
            rst->setVar('3',0 );
        }
    } break;
    case 4: { // rootNode
        if( w->xroot ) {
            rst->setVar('v',1,(LPVOID)w->xroot);
        }
    } break;
    case 5: { // clientArray
        bool checkNode=arrs && isVarTrue(arrs->get(0));
        ConnectNode* root=w->xroot;
        XListArr* a=tn->addArray("@clientArray",true);
        if( root ) {
            for( int n=0, sz=root->xchilds.size(); n<sz; n++ ) {
                ConnectNode* c=root->child(n);
                if(checkNode) {
                    a->add()->setVar('n',0,(LPVOID)c->config() );
                } else {
                    a->add()->setVar('v',1,(LPVOID)c);
                }
            }
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 6: { // stop
        tn->clearNodeFlag(FLAG_RUN);

    } break;
    case 7: { // close
        ConnectNode* root=w->xroot;
        tn->clearNodeFlag(FLAG_RUN);
        if( root ) {
            for( int n=0, sz=root->childCount(); n<sz; n++ ) {
                ConnectNode* conn=root->child(n);
                conn->closeSocket();
            }
            root->xchilds.reuse();
            root->closeSocket();
        }
    } break;
    case 8: { // rootPath

    } break;
    case 9: { // loadPage

    } break;
    case 10: { // session

    } break;
    case 11: { // errorInfo

    } break;
    case 12: { // config

    } break;
    case 13: { // statusInfo

    } break;
    case 14: {	// funcUpdate

    } break;
    case 15: {	// pageNode
        if( cnt==0 || w->xroot==NULL ) return false;
        TreeNode* map=NULL;
        TreeNode* child=NULL;
        sv=w->xroot->config()->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            map=(TreeNode*)SVO;
            child=map->child(0);
        }
        TreeNode* pageNode=NULL;
        if( map && child ) {
            LPCC uri=AS(0);
            sv=child->gv(uri);
            if( SVCHK('n',0) ) {
                pageNode=(TreeNode*)SVO;
            } else {
                int pos=LastIndexOf(uri,'/');
                if( pos>0 ) {
                    rst->set(uri,pos);
                    sv=child->gv(rst->get());
                    if( SVCHK('n',0) ) {
                        pageNode=(TreeNode*)SVO;
                    }
                }
            }
        }
        if( pageNode ) rst->setVar('n',0,(LPVOID)pageNode);
    } break;
    case 16: {	// urlMap
        sv=tn->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            TreeNode* map=(TreeNode*)SVO;
            rst->setVar('n',0,(LPVOID)map);
        }
    } break;
    case 17: { // loadWebPage
        if( cnt==0 || w->xroot==NULL ) return true;
        sv=tn->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            TreeNode* map=(TreeNode*)SVO;
            LPCC uri=AS(0);
            LPCC target = uri[0]=='/' ? uri+1: uri;
            int pos=IndexOf(target,'/');
            if( pos>0 ) {
                LPCC pageCode=target+pos+1;
                LPCC pageGroup=gBuf.add(target,pos);
                sv=arrs->get(1);
                if( sv ) {
                    // ex) loadWebPage('demo/page',src);
                    XListArr* a=getListArrTemp(); // getLocalArray();
                    a->add()->set(pageGroup);
                    a->add()->set(pageCode);
                    a->add()->add(sv);
                    loadWebPage(a, fn, rst, map);
                } else {
                    rst->setVar('3',0);
                }
            } else {
                // pageGroup, pageCode, pageSrc, currentPageId
                loadWebPage(arrs, fn, rst, map);
            }
        }

    } break;
    case 18: { // isRun
        rst->setVar('3', tn->isNodeFlag(FLAG_RUN)? 1: 0 );
    } break;
    case 19: { // selectTimeout
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(w->xtm.tv_sec);
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                w->xtm.tv_sec=toInteger(arrs->get(0));
            }
        }
    } break;
    case 20: { // pageMap
        if( cnt==0 || w->xroot==NULL ) return false;
        // exam) pageMap("/login", "/pages/common/login");
        TreeNode* map=NULL;
        TreeNode* child=NULL;
        LPCC mapUri=AS(0), uri=AS(1);
        sv=tn->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            map=(TreeNode*)SVO;
            child=map->child(0);
        }
        if( map && child ) {
            TreeNode* pageNode=NULL;
            sv=child->gv(uri);
            if( SVCHK('n',0) ) {
                pageNode=(TreeNode*)SVO;
            } else {
                int pos=LastIndexOf(uri,'/');
                if( pos>0 ) {
                    rst->set(uri,pos);
                    sv=child->gv(rst->get());
                    if( SVCHK('n',0) ) {
                        pageNode=(TreeNode*)SVO;
                    }
                }
            }
            XFuncSrc* fsrc=NULL;
            sv=map->gv(uri);
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
            if( pageNode && fsrc && slen(mapUri) ) {
                map->GetVar(mapUri)->setVar('f',1,(LPVOID)fsrc);
                child->GetVar(mapUri)->setVar('n',0,(LPVOID)pageNode);
                rst->setVar('3',1);
            } else {
                qDebug("#9#webpage mapping fail (map:%s, target:%s)\n", mapUri, uri);
                rst->setVar('3',0);
            }
        }
    } break;
    case 21: { // shutdown
        ConnectNode* root=w->xroot;
        tn->clearNodeFlag(FLAG_RUN);
        if( root ) {
            for( int n=0, sz=root->childCount(); n<sz; n++ ) {
                ConnectNode* conn=root->child(n);
                conn->closeSocket();
            }
            root->xchilds.reuse();
            root->closeSocket();
        }
        tn->GetVar("@c")->reuse();
        SAFE_DELETE(w);
    } break;
    case 22: { // db
    } break;
    case 23: { // configFlag

    } break;
    case 24: { // uri

    } break;
    case 25: { // mapArray
        // exam) mapArray("/login");
        XListArr* a=NULL;
        sv=tn->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            TreeNode* map=NULL;
            if( cnt==0 ) {
                if( a==NULL ) a=getListArrTemp();
                for( WBoxNode* bn=map->First(); bn ; bn=map->Next() ) {
                    LPCC code=map->getCurCode();
                    a->add()->add(code);
                }
            } else {
                LPCC uri=AS(0);
                sv=map->gv(uri);
                if( SVCHK('f',1) ) {
                    XFuncSrc* fsrc=(XFuncSrc*)SVO;
                    for( WBoxNode* bn=map->First(); bn ; bn=map->Next() ) {
                        sv=&(bn->value);
                        if( SVCHK('f',1) ) {
                            XFuncSrc* fsrcCur=(XFuncSrc*)SVO;
                            if( fsrc==fsrcCur ) {
                                LPCC code=map->getCurCode();
                                if( a==NULL ) a=getListArrTemp();
                                a->add()->add(code);
                            }
                        }
                    }
                } else {
                    qDebug("#9#web error (uri:%s not find)\n", uri);
                }
            }
        }
        if( a ) {
            rst->setVar('a',0,(LPVOID)a);
        }
    } break;
    case 26: { // is
        LPCC ty= arrs? AS(0): "run";
        rst->setVar('3',0);
        if( ccmp(ty,"run") ) {
            if( tn->isNodeFlag(FLAG_RUN) ) {
                rst->setVar('3',1);
            }
        }
    } break;
    case 27: { // uriMap
        sv=tn->GetVar("@uriMap");
        TreeNode* uriMap=NULL;
        if( SVCHK('n',0) ) {
            uriMap=(TreeNode*)SVO;
        } else {
            uriMap=new TreeNode(8);
            sv->setVar('n',0,(LPVOID)uriMap);
        }
        rst->setVar('n',0,(LPVOID)uriMap);
    } break;
    case 28: { // proxyReq
        if( cnt==0 ) {
            return true;
        }
        sv=tn->GetVar("@uriMap");
        if( SVCHK('n',0) ) {
            TreeNode* map=(TreeNode*)SVO;
            LPCC pageCode=AS(1);
            if( slen(pageCode) ) pageCode="/baro/common/proxyReq";
            sv = loadWebPageFunc(pageCode, map, NULL);
            if( SVCHK('f',1)  ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                TreeNode* client=NULL;
                XFuncNode* fnCur=w->getFuncNode(NULL, fsrc, NULL, 0);
                int sp=0, ep=0;
                sv=arrs->get(0);
                if( SVCHK('n',0) ) {
                    client=(TreeNode*)SVO;
                    sv=client->gv("@content");
                    qDebug("#0#was.proxyReq content (size:%d)\n", sv?sv->length(): 0 );
                    if( sv ) {
                        ep=sv->length();
                        fnCur->GetVar("proxyData")->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                    } else {
                        client=NULL;
                    }
                }
                if( client==NULL ) {
                    sv=getStrVarPointer(arrs->get(0), &sp, &ep);
                    if( sp<ep ) {
                        fnCur->GetVar("proxyData")->set(sv->get(sp), ep-sp);
                    } else {
                        qDebug("#0#userConnectMessage proxyReq data error (sp:%d, ep:%d)\n", sp, ep);
                    }
                }
                WorkerThread* worker=w->getWorker();
                if( fnCur && worker  ) {
                    fnCur->setNodeFlag(FLAG_RUN | FLAG_WEB_CALL);
                    fnCur->xfunc = fsrc->xfunc;
                    fnCur->xtick = GetTickCount();
                    fnCur->set("method", "PROXY_REQ");
                    fnCur->GetVar("was")->setVar('v',0,(LPVOID)w);
                    worker->pushObject('f',0,(LPVOID)fnCur);
                    rst->setVar('3',1);
                }
            } else {
                qDebug("#0#userConnectMessage proxyReq pageFunc find error \n" );
            }
        } else {
            qDebug("#0#userConnectMessage proxyReq url mapping error \n");
        }
    } break;
    case 30: {		// execPage

    } break;
    default: return false;
    }
    return true;
}

void pageMapUri(LPCC pageGroup, LPCC pageCode, bool map, TreeNode* root, TreeNode* sub ) {
    if( root==NULL || sub==NULL ) {
        return;
    }
    TreeNode* child=root->child(0);
    StrVar* sv=NULL;
    LPCC pageId=gBuf.fmt("/%s/%s",pageGroup, pageCode);
    if( map ) {
        if( child==NULL ) child=root->addNode();
        // base page add
        sv=child->gv(pageId);
        if( !SVCHK('n',0) ) {
            child->GetVar(pageId)->setVar('n',0,(LPVOID)sub);
        }
    }
    for( WBoxNode* bn=sub->First(); bn ; bn=sub->Next() ) {
        sv=&(bn->value);
        if( !SVCHK('f',1) ) {
            continue;
        }
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        LPCC curCode=sub->getCurCode();
        LPCC uri=curCode;
        if( slen(curCode)==0 ) {
            qDebug("#0# not define uri [pageId=%s]\n", pageId );
            continue;
        }
        if( fsrc->xflag & FLAG_RESULT ) {
            continue;
        }
        fsrc->xflag|=FLAG_RESULT;
        if( uri[0]=='/' ) {
            uri++;
        }
        LPCC url=gBuf.fmt("/%s/%s/%s",pageGroup, pageCode, uri);
        if( map ) {
            if( ccmp(uri,"main") ) {
                if( slen(pageId) ) {
                    root->GetVar(pageId)->setVar('f',1,(LPVOID)fsrc);
                }
            }
            root->GetVar(url)->reuse()->setVar('f',1,(LPVOID)fsrc);
        } else {
            TreeNode* cur=root->addNode();
            cur->set("pageGroup", pageGroup);
            cur->set("pageGroup", pageCode);
            cur->set("url", url);
            cur->set("uri", curCode);
        }
    }
}

void loadWebPage(PARR arrs, XFuncNode* fn, StrVar* rst, TreeNode* parentNode ) {
    if( arrs==NULL || arrs->size()==0 ) {
        return;
    }
    LPCC pageGroup=AS(0);
    LPCC pageCode=AS(1);
    if( slen(pageGroup)==0 ) {
        qDebug("#0#loadWebPage fail !!!\n");
        return;
    }
    if( slen(pageCode)==0 ) {
        qDebug("#0#loadWebPage fail [group=%s] !!!\n", pageGroup);
        return;
    }
    int sp=0, ep=0;
    StrVar* sv=getStrVarPointer(arrs->get(2), &sp, &ep);
    XParseVar pv(sv,sp,ep);
    TreeNode* webpagesNode=NULL;
    TreeNode* tn=NULL;

    sv=getStrVar("webpages", pageGroup );
    if( SVCHK('n',0) ) {
        webpagesNode=(TreeNode*)SVO;
    } else {
        webpagesNode=new TreeNode(2, true);
        webpagesNode->set("code", pageGroup);
        getStrVar("webpages",pageGroup, 'n',0,(LPVOID)webpagesNode );
    }
    sv=webpagesNode->gv(pageCode);
    if( SVCHK('n',0) ) {
        tn=(TreeNode*)SVO;
        // deleteAllTreeNode(tn);
        // tn->reuse();
    } else {
        tn = webpagesNode->addNode(); // gtrees.getTreeNode();
        webpagesNode->GetVar(pageCode)->setVar('n',0,(LPVOID)tn);
    }
    tn->setJson(pv,NULL);
    LPCC pageId=AS(3);
    if( slen(pageId)==0 ) {
        if( slen(pageId)==0 ) pageId=pageCode;
    }
    tn->set("@id", pageId);
    tn->set("@pageCode", gBuf.fmt("%s.%s", pageGroup, pageCode) );
    tn->setNodeFlag(FLAG_ROOT);
    LPCC tag=tn->get("tag");
    if( slen(tag)==0 ) {
        tn->set("tag","webpage");
    }
    setClassFuncNode(tn,fn,'n',NULL, FNSTATE_WEBNODE);

    XFuncNode* fnInit=NULL;
    sv=tn->gv("onInit");
    if( SVCHK('f',0) ) {
        fnInit=(XFuncNode*)SVO;
        if( fnInit->isNodeFlag(FLAG_START)==false ) {
            fnInit->setNodeFlag(FLAG_START);
            fnInit->call();
        }
    }
    if( parentNode ) {
        pageMapUri(pageGroup, pageCode, true, parentNode, tn);
    }
    rst->setVar('n',0,(LPVOID)tn);
}
