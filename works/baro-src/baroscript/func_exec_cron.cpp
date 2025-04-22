#include "func_util.h"
#include "../webserver/CronThread.h"

bool callCronFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->GetVar("@c");
    CronThread* c = NULL;
    if( SVCHK('t',10) ) {
        c=(CronThread*)SVO;
    } else {
        c=new CronThread(tn);
        sv->setVar('t',10,(LPVOID)c);
    }
    // ### version 1.0.4 ###
    U32 ref = fc->xfuncRef;
    if( ref==0 ) {
        ref = ccmp(fnm,"start") ? 1:
            ccmp(fnm,"addJob") ? 2:
            ccmp(fnm,"expr") ? 3:
            ccmp(fnm,"jobSource") ? 4:
            ccmp(fnm,"jobRun") ? 11:
            ccmp(fnm,"stop") ? 12:
            ccmp(fnm,"remove") ? 13:
            ccmp(fnm,"rootNode") ? 14:
            ccmp(fnm,"is") ? 19:
            ccmp(fnm,"jobStart") ? 20:
            ccmp(fnm,"jobStop") ? 21:
            ccmp(fnm,"jobTarget") ? 22:
            ccmp(fnm,"close") ? 23:
            0;
        fc->xfuncRef = ref;
    }
    switch( ref ) {
    case 1: { // start
        TreeNode* root=c->root();
        if( root ) {
            root->setNodeFlag(FLAG_START);
            if( c->exec() ) rst->setVar('3',1);
        }
    } break;
    case 2: { // addJob
        if( arrs==NULL ) {
            return false;
        }
        TreeNode* root=c->root();
        // param: jobId, jobExpr, func
        LPCC jobId=AS(0), expr=AS(1);
        if( slen(jobId)==0 ) {
            rst->setVar('3',0);
            return false;
        }
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node==NULL ) {
            node= root->addNode();
            node->set("id",jobId);
        }
        XFuncNode* fnCur=NULL;
        TreeNode* thisNode=NULL;
        sv=arrs->get(2);
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(3);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            fnCur=setCallbackFunction(fsrc, fn, node, NULL, node->GetVar("@callback") );
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)node);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
        }
        node->set("expr", expr);
        cronLineAdd(node);
        if( fnCur) {
            node->setNodeFlag(FLAG_START|FLAG_RUN);
            if( !root->isNodeFlag(FLAG_START) ) {
                c->exec();
            }
        }
        rst->setVar('n',0,(LPVOID)node);
    } break;
    case 3: { // expr
        if( arrs==NULL) {
            return false;
        }
        TreeNode* root=c->root();
        LPCC jobId=AS(0);
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node ) {
            LPCC expr=AS(1);
            if( slen(expr) ) {
                node->set("expr",expr);
                cronLineAdd(node);
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 4: { // jobSource
        // exam) jobSource(jobId, script, thisNode)
        if( arrs==NULL ) return false;
        TreeNode* root=c->root();
        LPCC jobId=AS(0);
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node ) {
            sv=arrs->get(1);
            TreeNode* thisNode=NULL;
            if( SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(2);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnCur=setCallbackFunction(fsrc, fn, node, NULL, node->GetVar("@callback") );
                if(thisNode) {
                    fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)node);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
                rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 11: { // jobRun
        // exam) jobRun(jobId)
        if( arrs==NULL) {
            return false;
        }
        TreeNode* root=c->root();
        LPCC jobId=AS(0);
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node ) {
            XFuncNode* fnCur=getEventFuncNode(node, "@callback");
            if( fnCur ) {
                PARR arrs=getLocalParams(fnCur);
                arrs->add()->setVar('n',0,(LPVOID)tn);
                arrs->add()->setVar('n',0,(LPVOID)node);
                setFuncNodeParams(fnCur, arrs);
                bool bstart=root->isNodeFlag(FLAG_START);
                root->clearNodeFlag(FLAG_START);
                node->clearNodeFlag(FLAG_START );
                fnCur->call(NULL,rst);
                node->setNodeFlag(FLAG_START );
                if( bstart ) {
                    root->setNodeFlag(FLAG_START);
                }
            }
        }
    } break;
    case 12: { // stop        
        c->root()->clearNodeFlag(FLAG_START);
    } break;
    case 13: { // remove
        // exam) remove(jobId)
        if( arrs==NULL ) {
            return false;
        }
        TreeNode* root=c->root();
        LPCC jobId=AS(0);
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node ) {
            bool bstart=root->isNodeFlag(FLAG_START);
            root->clearNodeFlag(FLAG_START);
            root->remove(node);
            qDebug("#0##batch job remove ok :%s\n",node->get("id"));
            rst->setVar('3',1);
            if( bstart ) root->setNodeFlag(FLAG_START);
        }
    } break;
    case 14: { // rootNode
        TreeNode* root=c->root();
        rst->setVar('n',0,(LPVOID)root);
    } break;
    case 19: { // is
        TreeNode* root=c->root();
        if( arrs==NULL ) {
            rst->setVar('3', (root->isNodeFlag(FLAG_START) ) ? 1 : 0);
            return true;
        }
        LPCC ty=AS(0);
        if( ccmp(ty,"start") ) {
            sv=arrs->get(1);
            if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                rst->setVar('3', (node->isNodeFlag(FLAG_START) ) ? 1 : 0);
            } else {
                rst->setVar('3', (root->isNodeFlag(FLAG_START) ) ? 1 : 0);
            }
        } else if( ccmp(ty,"stop") ) {
            sv=arrs->get(1);
            if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                rst->setVar('3', (node->isNodeFlag(FLAG_START) ) ? 0 : 1);
            } else {
                rst->setVar('3', (root->isNodeFlag(FLAG_START) ) ? 0 : 1);
            }
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 20:
    case 21:
    case 22: { // jobTarget
        if( arrs==NULL ) return false;
        TreeNode* root=c->root();
        LPCC jobId=AS(0);
        TreeNode* node=NULL;
        for( int n=0,sz=root->childCount();n<sz;n++) {
            TreeNode* cur = root->child(n);
            if( cur->cmp("id",jobId) ) {
                node=cur;
                break;
            }
        }
        if( node ) {
            if( ref==20 ) {
                node->setNodeFlag( FLAG_START| FLAG_RUN );
            } else if( ref==21 ) {
                node->clearNodeFlag( FLAG_START| FLAG_RUN );
            } else {
                XFuncNode* fn=getEventFuncNode(node,"@callback");
                if( fn ) {
                    sv=arrs->get(1);
                    if( SVCHK('n',0) ) {
                        TreeNode* targetNode=(TreeNode*)SVO;
                        fn->GetVar("@this")->setVar('n',0,(LPVOID)targetNode);
                        sv=targetNode->gv("onInit");
                        if( SVCHK('f',0) ) {
                            XFuncNode* fnParent=(XFuncNode*)SVO;
                            fn->xparent=fnParent;
                        }
                    } else {
                        sv=fn->gv("@this");
                        rst->reuse()->add(sv);
                    }
                }
            }
            rst->setVar('n',0,(LPVOID)node);
        }
    } break;
    case 23: { // close
        TreeNode* root=c->root();
        root->clearNodeFlag(FLAG_START);
        c->Final();
    } break;
    default: return false;
    }
    return true;
}

