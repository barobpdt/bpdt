#include "CronThread.h"
#include "../baroscript/func_util.h"

static char  *MonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL };
static char  *DowNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun", NULL };

CronThread::CronThread( TreeNode* cf ) : xtype(0), xstate(0), xflag(0), xconfig(cf) {
    xroot=NULL;
}


CronThread::~CronThread() {
    if( xroot && xroot->isNodeFlag(FLAG_NEW) ) {
        SAFE_DELETE(xroot);
    }
}

TreeNode* CronThread::root() {
    if( xroot==NULL ) xroot=new TreeNode(2, true);
    return xroot;
}

bool CronThread::exec() {
    xflag|= FLAG_INIT;
    if( !IsRunning() )
        Create();
    return true;
}

void CronThread::push(StrVar* sv) {
    xarr.xflag|=FLAG_RUN;
    xarr.add()->add(sv);
    FThread::Sleep(20);
    xarr.xflag&=~FLAG_RUN;
}

void CronThread::Run() {
    xroot->setNodeFlag(FLAG_START);
    cronRun(xroot);
}

void CronThread::Final() {
    xroot->clearNodeFlag(FLAG_START);
}

//#> util
void cronSetTime(TreeNode* root) {
    if( root==NULL ) return;
    UL64 start = time((time_t *)0);
    root->setLong("startTime", start );
    root->setLong("startClock", start / (unsigned long)SECONDS_PER_MINUTE);
}

void cronSleep(long target) {
    register UL64 seconds_to_wait;
    seconds_to_wait = (UL64)(target*SECONDS_PER_MINUTE - time((time_t*)0)) + 1;
    if (seconds_to_wait > 0 && seconds_to_wait< 65) {
        FThread::Sleep((unsigned int)seconds_to_wait);
    }
}

TreeNode* cronFindJobs(TreeNode* root, long vtime, int doWild, int doNonWild) {
#ifdef CRON_WORKER_USE
    time_t   virtualSecond  = vtime;
    register struct tm      *tm = localtime(&virtualSecond);
    register int            sec, minute, hour, dom, month, dow;
#else
    time_t   virtualSecond  = vtime * SECONDS_PER_MINUTE;
    register struct tm      *tm = localtime(&virtualSecond);
    register int            sec, minute, hour, dom, month, dow;
#endif
    sec		= tm->tm_sec - FIRST_SEC;
    minute	= tm->tm_min - FIRST_MINUTE;
    hour	= tm->tm_hour - FIRST_HOUR;
    dom		= tm->tm_mday - FIRST_DOM;
    month	= tm->tm_mon +1 - FIRST_MONTH; /* 0..11 -> 1..12 */
    dow		= tm->tm_wday - FIRST_DOW;

    /* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
    * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
    * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
    * is why we keep 'e->dow_star' and 'e->dom_star'.  yes, it's bizarre.
    * like many bizarre things, it's the standard.
    */
    StrVar* sv=NULL;
    XListArr arrs;
    for( int n=0,size=root->childCount(); n<size;n++) {
        TreeNode* cur=root->child(n);
        if( !cur->isNodeFlag(FLAG_RUN) ) continue;
        sv=cur->GetVar("@entry");
        if( !SVCHK('t',11) || !cur->isNodeFlag(FLAG_START) )
            continue;
        CronEntry* e = (CronEntry*)SVO;
        if( e==NULL )
            continue;
        if( bit_test(e->sec, sec) && bit_test(e->minute, minute) && bit_test(e->hour, hour) &&
            bit_test(e->dom,dom) && bit_test(e->month, month) )
        {
/*
            ( ((e->flags & DOM_STAR) || (e->flags & DOW_STAR))
            ? (bit_test(e->dow,dow) && bit_test(e->dom,dom))
            : (bit_test(e->dow,dow) || bit_test(e->dom,dom)))

*/
            if ((doNonWild && !(e->flags & (SEC_STAR|MIN_STAR|HR_STAR))) || (doWild && (e->flags & (SEC_STAR|MIN_STAR|HR_STAR))))
            {
                // ex) config field ( jobName, jobExpr, jobCode )
                DWORD tick=0;
                sv=cur->GetVar("@prevTick");
                if( SVCHK('1',0) ) {
                    tick=(DWORD)sv->getUL64(FUNC_HEADER_POS);
                }

                // qDebug()<<"tick dist = "<<GetTickCount()-tick;
                if( GetTickCount()-tick > 850 ) {
                    if( tick==0 ) tick = GetTickCount();
                    cur->GetVar("@startTm")->setVar('1',0).addUL64((UL64)getTm());
                    cur->GetVar("@prevTick")->setVar('1',0).addUL64(GetTickCount());
                    XFuncNode* fn=getEventFuncNode(cur, "@callback");
                    if( fn ) {
                        int index=cur->incrNum("@index");
                        PARR arrs=getLocalParams(fn);
                        arrs->add()->setVar('n',0,(LPVOID)cur);
                        arrs->add()->setVar('0',0).addInt(index);
                        arrs->add()->setVar('1',0).addUL64(GetTickCount()-tick);
                        setFuncNodeParams(fn, arrs);
                        fn->call();
                    }
                    cur->GetVar("@endTm")->setVar('1',0).addUL64((UL64)getTm());
                }
            }
        }
    }
    return NULL;
}

long cronRun( TreeNode* root ) {
    long diff = 0;
    int wack = 0;
    UL64 startTime=0, startClock=0, run=0, prev=0;

#define SET_CRONTIME(root) cronSetTime((root)); startTime = (root)->getLong("startTime"); startClock = (root)->getLong("startClock");
    SET_CRONTIME( root );
    while ( true ) {
        if( !root->isNodeFlag(FLAG_START) ) {
            FThread::Sleep(500);
            continue;
        }
        if( root->isNodeFlag(FLAG_CALL) ) {
            XFuncNode* fnCur=NULL;
            root->clearNodeFlag(FLAG_CALL);
            StrVar* sv=root->gv("@callFunction");
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnTarget=NULL;
                sv=root->gv("@callTarget");
                if( SVCHK('f',0) ) {
                    fnTarget=(XFuncNode*)SVO;
                }
                fnCur=gfns.getFuncNode(fsrc->xfunc, fnTarget);
                fnCur->call();
                funcNodeDelete(fnCur);
            } else if( SVCHK('f',0) ) {
                fnCur=(XFuncNode*)SVO;
                fnCur->call();
            }
            FThread::Sleep(50);
            continue;
        } else if( root->isNodeFlag(FLAG_INIT) ) {
            StrVar* sv=root->gv("@checkFunction");
            if( SVCHK('f',0) ) {
                XFuncNode* fnCur=(XFuncNode*)SVO;
                fnCur->call();
            }
        }

        do {
            cronSleep(run + 1);
            UL64 start = time((time_t *)0);
            startTime	= start;
            startClock	= start / (unsigned long)SECONDS_PER_MINUTE;
        } while ( startClock == run );
        run = startClock;
        diff = run - prev;

        /* shortcut for the most common case */
        if (diff == 1) {
            prev = run;
            cronFindJobs( root, prev, TRUE, TRUE);
        } else {
            wack = -1;
            if (diff > -(3*MINUTE_COUNT))
                wack = 0;
            if (diff > 0)
                wack = 1;
            if (diff > 5)
                wack = 2;
            if (diff > (3*MINUTE_COUNT))
                wack = 3;

            switch (wack) {
              case 1: // diff is a small positive number
                  do {
                      prev++;
                      if( !cronFindJobs( root, prev, TRUE, TRUE) ) FThread::Sleep(50);
                  } while ( prev< run);
                  break;

              case 2:	 // diff is a medium-sized positive number,
                  if( !cronFindJobs( root, run, TRUE, FALSE) ) {
                    do {
                          prev++;
                          if( !cronFindJobs( root, prev, FALSE, TRUE) ) FThread::Sleep(50);
                          SET_CRONTIME(root );
                      } while ( prev< run && startClock == run);
                  }
                  break;
              case 0:	// diff is a small or medium-sized
                  cronFindJobs( root, run, TRUE, FALSE);
                  break;
              default:	// other: time has changed a *lot*,
                  prev = run;
                  cronFindJobs( root, run, TRUE, TRUE) ;
            }
        }
    }
    return 1;
}

StrVar* cronLineAdd(TreeNode* cur ) {
    CronEntry* e = NULL;
    StrVar* sv=cur->GetVar("@entry");
    if( SVCHK('t',11) ) {
        e=(CronEntry*)SVO;
    } else {
        e=new CronEntry();
        sv->setVar('t',11,(LPVOID)e);
    }
    cur->setNodeFlag( FLAG_START | FLAG_RUN );
    sv=cur->GetVar("expr");
    XParseVar pv(sv);
    char ch = pv.NextChar();
    while( pv.valid() ) {
        if( ch=='@' ) {
            pv.start++;
            LPCC cmd = pv.NextWord();
            if ( ccmp("reboot", cmd)) {
                e->flags |= WHEN_REBOOT;
            } else if ( ccmp("yearly", cmd) ||  ccmp("annually", cmd) ){
                bit_set(e->minute, 0);
                bit_set(e->hour, 0);
                bit_set(e->dom, 0);
                bit_set(e->month, 0);
                bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
                e->flags |= DOW_STAR;
            } else if ( ccmp("monthly", cmd) ) {
                bit_set(e->minute, 0);
                bit_set(e->hour, 0);
                bit_set(e->dom, 0);
                bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
                bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
                e->flags |= DOW_STAR;
            } else if ( ccmp("weekly", cmd) ) {
                bit_set(e->minute, 0);
                bit_set(e->hour, 0);
                bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
                e->flags |= DOM_STAR;
                bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
                bit_nset(e->dow, 0,0);
            } else if ( ccmp("daily", cmd) ||  ccmp("midnight", cmd) ) {
                bit_set(e->minute, 0);
                bit_set(e->hour, 0);
                bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
                bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
                bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
            } else if ( ccmp("hourly", cmd) ) {
                bit_set(e->minute, 0);
                bit_nset(e->hour, 0, (LAST_HOUR-FIRST_HOUR+1));
                bit_nset(e->dom, 0, (LAST_DOM-FIRST_DOM+1));
                bit_nset(e->month, 0, (LAST_MONTH-FIRST_MONTH+1));
                bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
                e->flags |= HR_STAR;
            } else {
                cur->GetVar("error")->set("알수 없는 command");
                break;
            }
        } else {
            bit_nset(e->dow, 0, (LAST_DOW-FIRST_DOW+1));
            ch=pv.ch();
            for( int n=0;n<5;n++) {
                int sp=0,ep=0,flag=SEC_STAR;
                PU8 pu=NULL;
                switch(n) {
                case 0: pu=e->sec;		sp=FIRST_SEC;		ep=LAST_SEC;	break;
                case 1: pu=e->minute;	sp=FIRST_MINUTE;	ep=LAST_MINUTE;		flag=MIN_STAR;	break;
                case 2: pu=e->hour;		sp=FIRST_HOUR;		ep=LAST_HOUR;		flag=HR_STAR;		break;
                case 3: pu=e->dom;		sp=FIRST_DOM;		ep=LAST_DOM;		flag=DOM_STAR;		break;
                case 4: pu=e->month;	sp=FIRST_MONTH;		ep=LAST_MONTH;		break;
                }
                // bit_nset(pu, 0, (ep-sp+1));
                bit_nclear(pu, 0, (ep-sp+1));
                if( ch==0 || ch=='*' ) {
                    e->flags |= flag;
                    // ### version 1.0.4 (x<=ep)
                    for(int x=sp;x<=ep;x++ )
                        bit_set(pu, (x-sp));
                    if( ch=='*' )
                        ch=pv.incr().ch();
                } else {
                    int a1=0,a2=0,a3=1;
                    bool ok=false;
                    LPCC next=pv.NextWord();
                    if( isnum(next) ) {
                        a1=atoi(next);
                        ok=true;
                    } else if( n==3 ) {

                    } else {

                    }
                    ch=pv.ch();
                    if( ch=='-' ) {
                        next=pv.incr().NextWord();
                        if( isnum(next) ) a2=atoi(next);
                        for(int x=a1;x<=a2;x++ ) {
                            bit_set(pu, (x-sp));
                        }
                    } else if( ch==',') {
                        bit_set(pu, (a1-sp));
                        while(ch==',') {
                            next=pv.incr().NextWord();
                            if( isnum(next) ) {
                                a2=atoi(next);
                                bit_set(pu, (a2-sp));
                            } else
                                break;
                        }
                        ok=false;
                    }

                    if( ok ) {
                        if( ch=='/') {
                            next=pv.incr().NextWord();
                            if( isnum(next) ) {
                                a2=ep;
                                a3=atoi(next);
                                if( a3<=0 ) a3=a2;
                            }
                        }
                        if( a1>a2 )
                            a2=a1;
                        if( a2>ep )
                            a2=ep;
                        if( a2<sp )
                            a2=sp;
                        for(int x=a1;x<=a2;x+=a3 )
                            bit_set(pu, (x-sp));
                    }
                    ch=pv.ch();
                }
            }
            break;
        }

    }
    return cur->gv("error");
}
