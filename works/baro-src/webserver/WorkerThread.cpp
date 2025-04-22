#include "WorkerThread.h"
#include "HttpNode.h"

FMutex		gMutexWorker;

/************************************************************************/
/* WorkerThread                                                            */
/************************************************************************/

WorkerThread::WorkerThread( TreeNode* cf ) : xtype(0), xstate(0), xflag(0), xpushTick(0), xconfig(cf), xio(NULL), xfnWorker(NULL) {
	httpServer=NULL;
	config()->setInt("index",0);
}


WorkerThread::~WorkerThread() {

}


bool WorkerThread::exec() {
    StrVar* sv=xconfig->gv("@callback");
    if(SVCHK('f',0) ) {
        xfnWorker=(XFuncNode*)SVO;
    }
    xflag|= FLAG_INIT | FLAG_START;
    xconfig->setNodeFlag(FLAG_START);
    if( !IsRunning() )
		Create();
	return true;
}

void WorkerThread::push(StrVar* sv) {
	if( !IsRunning() )
		exec();
	xlock.EnterMutex();
	xpushTick=GetTickCount();
	xarr.add()->add(sv);
	// config()->incr("index");
	xlock.LeaveMutex();
}

void WorkerThread::pushObject(char ty, U16 stat, LPVOID obj) {
	if( !IsRunning() )
		exec();
	xlock.EnterMutex();
	xpushTick=GetTickCount();
	xarr.add()->setVar(ty,stat,obj);
	xlock.LeaveMutex();
}

void WorkerThread::pushNode(TreeNode* node) {
	if( xtype==WORKER_MODULE || xtype==WORKER_ASYNC ) {
		if( !IsRunning() )
			exec();
	}
	xlock.EnterMutex();
	xpushTick=GetTickCount();
	xarr.add()->setVar('n',0,(LPVOID)node);
	xlock.LeaveMutex();
}

void WorkerThread::Run() {
	xconfig->setNodeFlag(FLAG_START);
	while( true ) {
		if( (xflag&FLAG_START)==0 ) {
            FThread::Sleep(100);
			continue;
		}
		if( httpServer ) {
			httpServer->clientCloseCheck();
			if( xfnWorker ) {
				int delay=config()->getInt("@delayTime",200);
				getLocalParams(xfnWorker);
				xfnWorker->call();
				if( delay<50 ) delay=50;
				FThread::Sleep(delay);
			} else {
				FThread::Sleep(100);
			}
			continue;
		}

		if( xtype==WORKER_MODULE ) {
			if( xfnWorker ) {
				StrVar* sv=NULL;
				TreeNode* cur = NULL;
				xlock.EnterMutex();
				sv=xarr.get(0);
				if( SVCHK('n',0) ) {
					cur=(TreeNode*)SVO;
				}
				xarr.remove(0);
				xlock.LeaveMutex();
				if( cur ) {
					qDebug("xxxx current=%s", cur->log());
					xflag|=FLAG_CALL;
					XListArr* arrs=getLocalParams(xfnWorker);
					arrs->add()->setVar('n',0, (LPVOID)cur);
					setFuncNodeParams(xfnWorker, arrs);
					xfnWorker->call();
					if( xarr.size()==0 ) {
						xconfig->removeAll(true);
					}
					xflag&=~FLAG_CALL;
				 }
			}
		} else if( xtype==WORKER_ASYNC ) {
			if( xfnWorker ) {
				int delay=config()->getInt("@delayTime",20);
				xlock.EnterMutex();
				xflag|=FLAG_CALL;
				getLocalParams(xfnWorker);
				xfnWorker->call();
				xflag&=~FLAG_CALL;
				xlock.LeaveMutex();
				if( delay<10 ) delay=20;
				FThread::Sleep(delay);
			}
        } else {
			XFuncNode* fn=NULL;
			ConnectNode* cn=NULL;
			StrVar* sv=NULL;
			xflag|=FLAG_RUN;
			xlock.EnterMutex();
			sv=xarr.get(0);
			if( sv ) {
				if( SVCHK('f',0) ) {
					fn=(XFuncNode*)SVO;
                } else if(SVCHK('v',1) ) {
					cn=(ConnectNode*)SVO;
				}
				xarr.remove(0);
			}
			xlock.LeaveMutex();
			xflag|=FLAG_CALL;
            if( fn ) {
                fn->call();
                sv=fn->gv("req");
                if( !SVCHK('v',1) ) {
                    sv=&(fn->xrst);
                }
                if( SVCHK('v',1) ) {
                    cn=(ConnectNode*)SVO;
                    cn->setTick();
                    cn->setNodeFlag(FLAG_END);
                    if(cn->isNodeFlag(FLAG_SINGLE)) {
                        cn->clearNodeFlag(FLAG_SETEVENT);
                    } else {
                        // if(cn->isNodeFlag(FLAG_CALL)) qDebug("may be call web proxy !!!");
                        cn->clearNodeFlag(FLAG_USE);
						if( cn->valid() && cn->cmp("Connection","keep-alive")  ) {
							initConnectNode(cn, true);
                        } else {
                            cn->closeSocket();
						}
                    }
                } else {
                    qDebug("xxxxxxx web call connect node error\n");
                }
				fn->clearNodeFlag(FLAG_CALL | FLAG_RUN );
			} else if( cn ) {
                bool ok=false;
                int state=cn->getInt("@state");
                if( state!=0 && httpSend(cn) ) {
                    ok=true;
                } else {
                    qDebug("#0#web page state==%d", state);
                }
                if(ok) {
                    cn->clearNodeFlag(FLAG_USE);
                    if( cn->valid() && cn->cmp("Connection","keep-alive")  ) {
                        initConnectNode(cn, true);
                    } else {
                        cn->closeSocket();
                    }
                } else {
                    qDebug("#0#web page send error!!! (URL:%s, FLAGS:0X%X)", cn->get("url"), cn->xflag );
                    cn->closeSocket();
                }
			}
			xflag&=~(FLAG_CALL|FLAG_RUN);
		}
		FThread::Sleep(50);
	}
    qDebug("WorkerThread loop exit");
}

void WorkerThread::Final() {
	xconfig->clearNodeFlag(FLAG_START);
}

WorkerGroup::WorkerGroup( int size, U32 flag) : xworkerIdx(0), xworkerSize(size), xworkerList(), xflag(flag) {
	initWorker(size, flag);
}
WorkerGroup::~WorkerGroup() {
	for( int n=0,sz=xworkerList.size(); n<sz; n++ ) {
		WorkerThread* worker=xworkerList.getvalue(n);
		TreeNode* cf = worker->config();
		SAFE_DELETE(cf);
		SAFE_DELETE(worker);
	}
}
void WorkerGroup::initWorker(int size, U32 flag) {
	if( size>0 ) {
		xworkerSize = size;
		xworkerList.Create(xworkerSize*8);
		if( flag>0 ) xflag = flag;
		for( int n=0;n<xworkerSize;n++ ) {
			TreeNode* cf=new TreeNode();
			WorkerThread* worker= new WorkerThread(cf);
			worker->xflag|= xflag; // FLAG_WEB_CALL;
			cf->set("tag","worker");
			cf->xstat=FNSTATE_WORKER;
			xworkerList.add(worker);
		}
	}
	// if( size!= xworkerSize ) 다시 생성
}

WorkerThread* WorkerGroup::getWorker() {
#ifdef WORKER_PENDING_USE
	WorkerThread* worker=xworkerList.getvalue(xworkerIdx++);
	if(xworkerSize<=xworkerIdx) {
		xworkerIdx=0;
	}
	if( (worker->xflag & FLAG_START)==0 ) {
		worker->exec();
	}
	return worker;
#endif
	WorkerThread* worker=NULL;
	WorkerThread* cur=NULL;
	int idx=xworkerIdx;
	gMutexWorker.EnterMutex();
	for( ; idx<xworkerSize; idx++ ) {
		cur=xworkerList.getvalue(idx);
		/*
		if( w->xflag& FLAG_RUN )
			continue;
		*/
		if( cur->xarr.size() == 0 ) {
			worker=cur;
			break;
		}
	}
	if( idx==xworkerSize && xworkerIdx<=xworkerSize ) {
		idx=0;
		for( ;idx<xworkerIdx;idx++ ) {
			cur=xworkerList.getvalue(idx);
			if( cur->xarr.size() == 0 ) {
				// qDebug("worker workerSize:%d, Index:%d\n", xworkerSize, idx );
				worker=cur;
				break;
			}
		}
	}
	if( worker ) {
		xworkerIdx = idx+1;
	} else {
		if( xworkerSize<32 ) {
			TreeNode* cf=new TreeNode();
			worker= new WorkerThread(cf);
			cf->set("tag","worker");
			cf->xstat=FNSTATE_WORKER;
			xworkerList.add(worker);
			xworkerIdx = xworkerSize;
			xworkerSize++;
			qDebug("#0#worker recheck workerSize:%d, startIndex:%d\n", xworkerSize, xworkerIdx );
		} else {
			idx++;
			if( idx>=xworkerSize ) idx=0;
			worker=xworkerList.getvalue(idx);
			xworkerIdx = idx+1;
		}
	}
	gMutexWorker.LeaveMutex();
	// qDebug("#0# getWorker size: %d index: %d\n",xworkerSize, xworkerIdx);
	if( (worker->xflag & FLAG_START)==0 ) {
		worker->exec();
	}
	return worker;
}
WorkerThread* WorkerGroup::getWorker(int n) {
	if( n>=0 && n<xworkerSize ) {
		return xworkerList.getvalue(n);
	}
	return NULL;
}

void WorkerGroup::reset() {
}
void WorkerGroup::idle() {

}
void WorkerGroup::stop() {
}
