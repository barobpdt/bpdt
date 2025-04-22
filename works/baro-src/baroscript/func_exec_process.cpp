#include "func_util.h"
#include "NodeProcess.h"

bool callProcessFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->GetVar("@c");
    NodeProcess* np = NULL;
    if( SVCHK('p',1) ) {
        np=(NodeProcess*)SVO;
    } else {
        np=new NodeProcess(tn);
        sv->setVar('p',1,(LPVOID)np);
    }
    int cnt=arrs?arrs->size():0;
    U32 ref = fc->xfuncRef;
    if( ref==0 ) {
        ref = ccmp(fnm,"run") ? 1:
            ccmp(fnm,"command") ? 2:
            ccmp(fnm,"write") ? 3:
            ccmp(fnm,"stop") ? 4 :
            ccmp(fnm,"path") ? 5 :
            ccmp(fnm,"exec") ? 6 :
            ccmp(fnm,"close") ? 7 :
            ccmp(fnm,"is") ? 8 :
            ccmp(fnm,"callback") ? 9 :
            ccmp(fnm,"kill") ? 10 :
            0;
        fc->xfuncRef = ref;
    }
    switch( ref ) {
    case 1: { // run
        if( cnt==0 ) {
            rst->setVar('3',np->isRun()?1:0);
        } else {
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            XParseVar pv(sv,sp,ep);
            XListArr params;
            // bool bchk=false;
            for(int n=0; n<32 && pv.valid(); ) {
                char ch=pv.ch();
                // int sp=pv.start;
                if( ch==0 )
                    break;
                if( ch=='\'' || ch=='"' ) {
                    if( pv.MatchStringPos(ch) ) {
                        params.add()->set(pv.v());
                    } else {
                        break;
                    }
                } else {
                    pv.findEnd(" \t\n",FIND_CHARS);
                    params.add()->set(pv.v());
                }
            }
            if( params.size()==0 ) {
                rst->setVar('3',0);
            }
            LPCC prog=NULL;
            QStringList sa;
            for( int n=0,sz=params.size();n<sz;n++ ) {
                sv=params.get(n);
                LPCC val=sv->get();
                if( n==0 ) {
                    prog=val;
                } else {
                    sa.append(KR(val));
                }
            }
            TreeNode* thisNode=NULL;
            U32 flag=0;
            int idx=1;
            sv = arrs->get(idx++);
            if( SVCHK('a',0)) {
                XListArr* a=(XListArr*)SVO;
                for(int n=0;n<a->size();n++) {
                    params.add()->add(a->get(n));
                }
                sv = arrs->get(idx++);
            }
            if( SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(idx++);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnCur = setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
                if(thisNode) {
                    fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
                tn->GetVar("@callback")->setVar('f', 0, fnCur);
            }
            sv=arrs->get(idx++);
            if(isNumberVar(sv)) {
                flag=toInteger(sv);
            } else if( SVCHK('3',1) ) {
                flag|=0x20;
            }
            LPCC workPath=NULL;
            sv=arrs->get(idx++);
            if(sv) {
                workPath=toString(sv );
            }
            if( slen(prog) ) {
                int len=slen(workPath);
                rst->reuse();
                if(len>0 ) {
                    rst->set(workPath);
                    if( workPath[len-1]!='/') {
                        rst->add("/");
                    }
                }
                rst->add(prog);
                LPCC runProgram=rst->get();
                // qDebug("runProgram == %s",runProgram);
                if( np->startProcess(runProgram, sa, flag) ) {
                    rst->setVar('3',1);
                    return true;
                }
            } else {
                qDebug("#9#process run error\n");
            }
            rst->setVar('3',0);
        }
    } break;
    case 2: // command
    case 3: { // write
        if( cnt==1 )  {
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(0), &sp, &ep);
            XParseVar pv(sv, sp, ep);
            char c=pv.ch();
            sv=tn->GetVar("@line");
            while( pv.valid() ) {
                LPCC line=pv.findEnd("\n").v();
                if( slen(line) ) {
                    c=pv.ch();
                    if( ref==2 ) {
                        sv->add(line);
                        if( c ) sv->add("\f");
                    } else {
                        sv->set(line).add("\n");
                        np->writeToStdin( sv->get() );
                    }
                    if( c==0 ) break;
                } else {
                    break;
                }
            }
            if( ref==2 && sv->length() ) {
                sv->add("\n");
                np->writeToStdin( sv->get() );
            }
        }
    } break;
    case 4: { // stop
        if( np && np->isRun() ) {
            if( cnt==1 ) {
                np->stop(AS(0) );
            } else {
                np->killProcess();
            }
        }
    } break;
    case 5: { // path
        // version 1.0.2
        if( cnt==0 ) {
            sv=tn->gv("@workPath");
            if( sv && sv->length() ) {
                rst->reuse()->add(sv);
            } else {
                QString path=np->workingDirectory();
                rst->set(Q2A(path));
            }
        } else {
            LPCC path=AS(0);
            if( path ) {
                np->setWorkingDirectory(KR(path));
                tn->GetVar("@workPath")->set(path);
            }
        }
    } break;
    case 6: { // exec
        if( cnt==0 ) {
            rst->reuse()->add(tn->gv("@execProgram"));
            return true;
        }
        QDir dir("C:\\");
        LPCC prog=AS(0);
        int rtn=QProcess::execute(prog, QStringList() << dir.toNativeSeparators(dir.path()));
        rst->setVar('0',0).addInt(rtn);
        tn->GetVar("@execProgram")->set(prog);
    } break;
    case 7: { // close
        if( cnt==1 ) {
            np->stop(AS(0) );
        }
        sv=tn->gv("@program"); if(sv) sv->reuse();
        sv=tn->gv("@workPath"); if(sv) sv->reuse();
        tn->GetVar("@c")->reuse();
        np->killProcess();
        np->deleteLater();
    } break;
    case 8: { // is
        rst->setVar('3',np->isRun()?1:0);
    } break;
    case 9: { // callback
        if(cnt==0) {
            rst->reuse();
            if( np->xfnProcess ) {
                rst->setVar('f',0,(LPVOID)np->xfnProcess);
            }
        } else {
            TreeNode* thisNode=NULL;
            int idx=0;
            sv = arrs->get(idx++);
            if( SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(idx++);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnCur = setCallbackFunction(fsrc, fn, tn, np->xfnProcess);
                if(thisNode) {
                    fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
                tn->GetVar("@callback")->setVar('f', 0, fnCur);
                np->xfnProcess=fnCur;
            }
        }
    } break;
    case 10: { // kill
        np->kill();
    } break;
    default: return false;
    }
    return true;
}
