#include "func_util.h"
#include "../webserver/WorkerThread.h"

bool callWorkerFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->GetVar("@c");
    WorkerThread* w = NULL;
    if( SVCHK('t',1) ) {
        w=(WorkerThread*)SVO;
    } else {
        w=new WorkerThread(tn);
        sv->setVar('t',1,(LPVOID)w);
    }
    U32 ref = fc->xfuncRef;
    if( ref==0 ) {
        ref = ccmp(fnm,"start") ? 1:// pus
            ccmp(fnm,"stop") ? 2:
            ccmp(fnm,"is") ? 3:
            ccmp(fnm,"close") ? 4:
            ccmp(fnm,"resume") ? 11:
            ccmp(fnm,"suspend") ? 12:
            ccmp(fnm,"pop") ? 13:
            ccmp(fnm,"push") ? 14:
            ccmp(fnm,"event") ? 15:
            ccmp(fnm,"list") ? 16:
            ccmp(fnm,"remain") ? 17:
            ccmp(fnm,"lock") ? 18:
            ccmp(fnm,"unlock") ? 19:
            ccmp(fnm,"current") ? 20:
            ccmp(fnm,"callback") ? 21:
            ccmp(fnm,"sleep") ? 22:
            0;
        fc->xfuncRef = ref;
    }
    int cnt=arrs? arrs->size(): 0;
    switch( ref ) {
    case 1: { // start
        // worker.start( class, func, async, delay )
        // worker.start( func, async, delay )
        w->xflag|=FLAG_START;
        if( cnt==0 ) {
            if( (w->xflag&(FLAG_INIT | FLAG_START))==0 ) {
                w->exec();
            }
            return true;
        }
        int idx=0;
        TreeNode* thisNode=NULL;
        sv=arrs->get(idx++);
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(idx++);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur =setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
            sv = arrs->get(idx++);
        }
        if( SVCHK('3',1) ) {
            w->xtype = WORKER_ASYNC;
        } else {
            w->xtype = WORKER_MODULE;
        }
        sv=arrs->get(idx++);
        if( isNumberVar(sv) ) {
            int delay=toInteger(sv);
            tn->setInt("@delayTime", delay);
        }
        w->exec();
        qDebug("worker start %d %s", w->xtype, w->xfnWorker?"callback":"callback not define");
        tn->setNodeFlag(FLAG_START | FLAG_PERSIST );
        rst->setVar('3', w->xfnWorker? 1: 0 );
    } break;
    case 2: { // stop
        sv=arrs?arrs->get(0):NULL;
        tn->clearNodeFlag(FLAG_START);
        w->xflag&=~FLAG_START;
        if( isVarTrue(sv) ) {
            w->xarr.reuse();
            w->xwait.reuse();
        }
    } break;
    case 3: { // is
         LPCC ty=arrs ? AS(0): "run";
         if( ccmp(ty,"run") ) {
             rst->setVar('3', w->IsRunning()? 1: 0 );
         } else if( ccmp(ty, "suspend") ) {
            rst->setVar('3', w->IsSuspended() ? 1: 0 );
         } else if( ccmp(ty, "start") ) {
            rst->setVar('3', w->xflag&FLAG_START ? 1: 0 );
         } else if( ccmp(ty, "call") ) {
            rst->setVar('3', w->xflag&FLAG_CALL ? 1: 0 );
         } else if( ccmp(ty, "doing") ) {
            rst->setVar('3', w->xflag&FLAG_RUN? 1: 0 );
         }
    } break;
    case 4: { // close
        FTRY {
            w->Stop();
            w->xflag=0;
        } FCATCH(FException, ex) {
            qDebug("close exception %s", ex.Get());
        }
    } break;
    case 11: { // resume
        if( cnt==0 ) return false;
        FTRY {
            w->Resume();
        } FCATCH(FException, ex) {
            qDebug("close exception %s", ex.Get());
        }
    } break;
    case 12: { // suspend
        FTRY {
            w->Suspend();
        } FCATCH(FException, ex) {
            qDebug("close exception %s", ex.Get());
        }
    } break;
    case 13: { // pop
        rst->reuse();
    } break;
    case 14: { // push
        TreeNode* node=NULL;
        if( arrs  ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                node=(TreeNode*)SVO;
                qDebug("push node size: %d, wait: %d\n", w->xarr.size(), w->xwait.size() );
            } else {
                LPCC code=toString(sv);
                node=tn->addNode(code);
            }
        } else {
            node=tn->addNode();
        }
        if( node ) {
            w->pushNode(node);
            rst->setVar('n',0,(LPVOID)node);
        }
    } break;
    case 15: { // event
    } break;
    case 16: { // list
        rst->setVar('a',0, (LPVOID)&(w->xarr) );
    } break;
    case 17: { // remain
    } break;
    case 18: { // lock
    } break;
    case 19: { // unlock
    } break;
    case 21: { // callback
        if( cnt==0 ) {
            if( w->xfnWorker ) {
                rst->setVar('f',0,(LPVOID)w->xfnWorker);
            }
            return false;
        }
        sv=arrs->get(0);
        TreeNode* thisNode=NULL;
        if( SVCHK('n',0)) {
            thisNode=(TreeNode*)SVO;
            sv = arrs->get(1);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
            if(thisNode) {
                fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
            }
            w->xfnWorker = fnCur;
        }
    } break;
    case 22: { // sleep
        ULONG tm=100;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            tm=(ULONG)toUL64(arrs->get(0));
        }
        w->Sleep(tm);
    } break;
    default:
        return false;
    }
    return true;
}
