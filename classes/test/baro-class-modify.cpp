if( ccmp(fnm,"callFuncSrc") ) {
    sv=fnCur->gv("@eventFuncList");
    // qDebug("@@ callFuncSrc start");
    if( SVCHK('a',0) ) {
        XListArr* list=(XListArr*)SVO;
        XFuncSrc* fsrcCur = NULL;
        TreeNode* prevThisNode = NULL;
        TreeNode* curThisNode = NULL;
        int size=list->size();
        sv=fnCur->gv("@this");
        if(SVCHK('n',0)) {
            prevThisNode = (TreeNode*)SVO;
        }
        sv=arrs!=NULL ? arrs->get(0):NULL;
        if(SVCHK('f',1) ) {
            fsrcCur=(XFuncSrc*)SVO;
            sv=arrs->get(1);
        }
        if(SVCHK('n',0)) {
            curThisNode = (TreeNode*)SVO;
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)curThisNode);
        }
        // qDebug("@@ callFuncSrc size:%d",size);
        for(int n=0; n<size; n++) {
            sv=list->get(n);
            if(SVCHK('f',1)) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                if(fsrcCur && fsrcCur!=fsrc ) {
                    continue;
                }
                XListArr* params=NULL;
                sv=fnCur->gv("@params");
                if(SVCHK('a',0)) {
                    params=(XListArr*)SVO;
                    XParseVar pv(&(fsrc->xparam));
                    for(int p=0; pv.valid() && p<256; p++ ) {
                        LPCC code = pv.findEnd(",").v();
                        if( slen(code)==0 ) break;
                        fnCur->GetVar(code)->reuse()->add(params->get(p));
                    }
                }
                if( fnCur->isNodeFlag(FLAG_CALL) ) {
                    // qDebug("==> funcNode callFuncSrc already called");
                    fnCur->clearNodeFlag(FLAG_CALL);
                }

                fnCur->call(fsrc->xfunc, rst->reuse());
                if( checkFuncObject(rst,'3',1) ||
                    ccmp("ignore",rst->get()) ) {
                    break;
                }
            } else {
                qDebug("@@ callFuncSrc n==%d not function", n);
            }
        }
        if( prevThisNode && curThisNode ) {
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)prevThisNode);
        }
    } else {
        qDebug("callFuncSrc error [function list not defined]");
    }
} 