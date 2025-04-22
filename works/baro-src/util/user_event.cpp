#include "user_event.h"
#include "../baroscript/tree_model.h"
#include <QApplication>
#include <QCryptographicHash>

struct	tm	gLogTm ;
FILE *		gLogFp = NULL;
StrVar gWatcherFile;

void UserEvent::setEventInfo(TreeNode *cf, int ty, StrVar *data){
    xcf=cf;
    xtype=ty;
    xdata.reuse();
    if( data ) {
        xdata.add(data);
    }
}


UserObjectManager::UserObjectManager() : xflag(0)
#ifdef WINDOW_USE
    , xconsole(NULL)
#endif
{
    xsidx=0;
    for( int n=0;n<MAX_TEMP_STR_NUM;n++ ) {
        xstrs.add();
    }
#ifdef TIMEOUT_SRC_USE
    LPCC fileName=getFullFileName("data/timeout.src");
    QFileInfo fi(fileName);
    if( fi.isFile() ) {
        qint64 fsize=fi.size();
        xinfos.setLong("size.timeout", fsize);
    }
#endif
    DbUtil* db=getConfDb();
    if( db ) {
        db->select("select type,def,value from global_define");
        while(db->fetchNode()) {
            getHashFlagVal(db->get("type"), db->get("def"), (U32)toUL64(db->GetVar("value")) );
        }
    }
}
UserObjectManager::~UserObjectManager() {

}
bool UserObjectManager::createConsole() {
#ifdef WINDOW_USE
    if( xconsole==NULL ) {
        xconsole = new CConsole();
        xconsole->Create("debug console",true);
    }
#endif
    return false;
}
void UserObjectManager::startTimer() {
    if( !xtimer.isActive() ) {
        StrVar* sv=getInfo()->gv("@timerDelay");
        int delay = isNumberVar(sv) ? toInteger(sv): 1500;
        connect(&xtimer, SIGNAL(timeout()), this, SLOT(onTimeout()) );
        xtimer.start(delay);
        qDebug(".........start timer.......... %d ", delay);
    }
}
void UserObjectManager::stopTimer() {
    if( xtimer.isActive() ) {
        xtimer.stop();
    }
}
StrVar*	UserObjectManager::getStrVar(bool move) {
    if( xsidx>=MAX_TEMP_STR_NUM )
        xsidx=0;
    StrVar* sv = xstrs.get(xsidx);
    if( sv && move ) {
        sv->reuse();
        xsidx++;
    }
    return sv;
}

void UserObjectManager::printDebug(LPCC msg, int ty) {
    if( msg==NULL )
        return;
    char ch=msg[1];
    if( getInfo()->isNodeFlag(CNF_FUNC_LOG) &&  msg[0]=='#' && ch=='9') {
        StrVar* sv=getInfo()->GetVar("@errorMessage");
        if( sv->length() ) sv->add("\n");
        sv->add(msg+3);
    }
    if( getInfo()->isNodeFlag(CNF_LOGFILE) ) {
        if( msg[0]=='#' && ( ch=='0' || ch=='9') ) {
            time_t t = time(NULL);
            struct tm *tm = localtime(&t);
            if( (gLogFp==NULL) || (tm->tm_mday != gLogTm.tm_mday) ) {
                memcpy(&gLogTm, tm, sizeof(gLogTm));
                LPCC path = getInfo()->get("debugLogPath");
                LPCC fullPath=getInfo()->GetVar("debugLogFile")->fmt("%s/log-%04d%02d%02d.log", path, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
                if( gLogFp ) {
                    fclose(gLogFp);
                }
                gLogFp=fopen(fullPath, "a+");
            }
            if( gLogFp ) {
                msg+=3;
                for( int n=0; ISBLANK(*msg) && n<256; n++ ) {
                    msg++;
                }
                if( ch=='0' ) {
                    fprintf(gLogFp, "%02d:%02d:%02d %s", tm->tm_hour, tm->tm_min, tm->tm_sec, msg ) ;
                } else {
                    fprintf(gLogFp, "%02d:%02d:%02d ##[error]##\n%s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, msg ) ;
                }
                fflush(gLogFp);
            }
        }
    }
    /*
    if( getInfo()->isNodeFlag(CNF_FUNC_LOG) && msg[0]=='#' ) {
        msg+=3;
        cfVar("@errorMessage")->add(msg).add("\n");
    }
    */
#ifdef WINDOW_USE
    if( getInfo()->isNodeFlag(CNF_DEBUG) ) {
        WORD clr=0;
        createConsole();
        if( msg[0]=='#' ) {
            if( msg[1]=='0' ) {
                clr=FOREGROUND_GREEN;
            } else if( msg[1]=='1' ) {
                clr=FOREGROUND_RED;
            } else if( msg[1]=='9' ) {
                clr=FOREGROUND_INTENSITY;
            }
            msg+=3;
        }
        if( xconsole ) {
            xconsole->print(msg, clr);
            xconsole->print("\n");
        }
    } else
#endif
    // if( slen(msg)<2000 )
    {
        fprintf(stderr, ">> %s\n", msg);
    }
}

TreeNode* UserObjectManager::getInfo() {
    return &xinfos;
}
void UserObjectManager::onTimeout() {
    StrVar* sv=xinfos.gv("onTimeout");
    if( SVCHK('f',0) ) {
        XFuncNode* fn=(XFuncNode*)SVO;
        fn->call();
        sv=fn->getResult();
        if( SVCHK('3',1) ) return;
    }
#ifdef TIMEOUT_SRC_USE
    else {
        sv=::getStrVar("fsrc","logPrintCheck");
        if( SVCHK('f',1) ) {
            execMemberFunc(sv, NULL, NULL, NULL, getInfo(), NULL);
        }
    }
    LPCC fileName=getFullFileName("data/timeout.src");
    QFileInfo fi(KR(fileName));
    if( fi.isFile() ) {
        qint64 fsize=fi.size();
        if( xinfos.getLong("size.timeout")!=fsize ) {
            QFile file(KR(fileName));
            if( file.open(QIODevice::ReadOnly) ) {
                qDebug("..... timeout ..... %s (%d)", fileName, (int)fsize);
                QByteArray ba=file.readAll();
                StrVar* src=getStrVar();
                src->set(ba.constData(), ba.size() );
                baroSourceApply(src, 0, src->length(),NULL);
                xinfos.setLong("size.timeout", fsize);
                file.close();
            }
        }
    }
#endif
}

void UserObjectManager::onButtonClick(TreeNode* cf, bool checked) {
    XFuncNode* fn = getEventFuncNode(cf, "onClick");
    if( fn ) {
        fn->call();
    }
}

void UserObjectManager::onButtonTrigger(TreeNode* cf, bool checked) {
    XFuncNode* fn = getEventFuncNode(cf, "onTrigger");
    if( fn ) {
        fn->call();
    }
}

void UserObjectManager::onSpinValueChanged(TreeNode* cf, const QString & text) {
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn ) {
        fn->call();
    }
}


// stacked widget slot
void UserObjectManager::onStackedCurrentChanged(TreeNode* cf, int index ) {
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn==NULL ) return;
    GStacked* div=qobject_cast<GStacked*>(gwidget(cf));
    if( div ) {
        TreeNode* cur=getWidgetConfig(div->widget(index) );
        if( cur ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('0',0).addInt(index);
            arrs->add()->setVar('n',0,(LPVOID)cur);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }

}

void UserObjectManager::onStackedWidgetRemoved(TreeNode* cf, int index ) {
    XFuncNode* fn = getEventFuncNode(cf, "onRemove");
    if( fn==NULL ) return;
    GStacked* div=qobject_cast<GStacked*>(gwidget(cf));
    if( div ) {        
        TreeNode* cur=getWidgetConfig(div->widget(index),0);
        if( cur ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('0',0).addInt(index);
            arrs->add()->setVar('n',0,(LPVOID)cur);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}

void UserObjectManager::onSplitterMoved(TreeNode* cf, int pos, int index) {
}


//
void UserObjectManager::onTabCurrentChanged (TreeNode* cf, int index ) {
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn==NULL ) return;
    GTab* tab=qobject_cast<GTab*>(gwidget(cf));
    if( tab ) {
        TreeNode* cur=getWidgetConfig(tab->widget(index),0);
        if( cur ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('0',0).addInt(index);
            arrs->add()->setVar('n',0,(LPVOID)cur);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }

}
void UserObjectManager::onTabCloseRequested (TreeNode* cf, int index ) {
    XFuncNode* fn = getEventFuncNode(cf, "onRemove");
    if( fn==NULL ) return;
    GTab* tab=qobject_cast<GTab*>(gwidget(cf));
    if( tab ) {
        TreeNode* cur=getWidgetConfig(tab->widget(index),0);
        if( cur ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('0',0).addInt(index);
            arrs->add()->setVar('n',0,(LPVOID)cur);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}


void UserObjectManager::onViewCurrentChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous ) {
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn==NULL ) return;
    StrVar* sv=cf->gv("@model");					if( !SVCHK('m',0) ) return;
    TreeModel* m=(TreeModel*)SVO;
    sv=cf->gv("@proxy");
    if( current.isValid() && SVCHK('m',1) ) {
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;
        QModelIndex src = proxy->mapToSource(current);
        if( src.isValid() ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('n',0,(LPVOID)src.internalPointer());
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}
void UserObjectManager::onViewCurrentColumnChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous ) {
}
void UserObjectManager::onViewCurrentRowChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous ) {
}
void UserObjectManager::onViewSelectionChanged ( TreeNode* cf, const QItemSelection & selected, const QItemSelection & deselected ) {
}
void UserObjectManager::onViewDoubleClicked(TreeNode* cf, const QModelIndex & index) {
    XFuncNode* fn = getEventFuncNode(cf, "onDoubleClicked");
    if( fn==NULL ) return;
    StrVar* sv=cf->gv("@model");
    if( !SVCHK('m',0) ) return;
    TreeModel* m=(TreeModel*)SVO;
    sv=cf->gv("@proxy");
    if( index.isValid() && SVCHK('m',1) ) {
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;
        QModelIndex src = proxy->mapToSource(index);
        if( src.isValid() ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('n',0,(LPVOID)src.internalPointer());
            arrs->add()->setVar('0',0).addInt(src.column());
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}

void UserObjectManager::onViewClicked( TreeNode* cf, const QModelIndex & index ) {
    XFuncNode* fn = getEventFuncNode(cf, "onClicked");
    if( fn==NULL ) return;
    StrVar* sv=cf->gv("@proxy");
    if( index.isValid() && SVCHK('m',1) ) {
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;
        QModelIndex src = proxy->mapToSource(index);
        if( src.isValid() ) {
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('n',0,(LPVOID)src.internalPointer());
            arrs->add()->setVar('0',0).addInt(src.column());
            setFuncNodeParams(fn, arrs);
            fn->call();
            return;
        }
    }
}


void	UserObjectManager::onProgressValueChanged(TreeNode* cf, int value) {
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn==NULL ) return;
    PARR arrs=getLocalParams(fn);
    arrs->add()->setVar('0',0).addInt(value);
    setFuncNodeParams(fn, arrs);
    fn->call();
}
void	UserObjectManager::onComboActivated(TreeNode* cf, int index) {
    XFuncNode* fn = getEventFuncNode(cf, "onHit");
    if( fn==NULL ) return;
    PARR arrs=getLocalParams(fn);
    arrs->add()->setVar('0',0).addInt(index);
    setFuncNodeParams(fn, arrs);
    fn->call();
}
void	UserObjectManager::onComboCurrentChanged(TreeNode* cf, int index) {
    /*
    if( cf->isNodeFlag(FLAG_RUN) ) {
        qDebug("combo change skip :: update run\n");
        return;
    }
    StrVar* sv=cf->gv("@textWidth");
    if( sv ) {
        sv->setVar('0',0).addInt(0);
    }
    */
    XFuncNode* fn = getEventFuncNode(cf, "onChange");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('0',0).addInt(index);
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}


void	UserObjectManager::onInputPositionChanged(TreeNode* cf, int prev, int cur){
}
void	UserObjectManager::onEditingFinished(TreeNode* cf){
}
void	UserObjectManager::onReturnPressed(TreeNode* cf){
    XFuncNode* fn = getEventFuncNode(cf, "onEnter");
    if( fn==NULL ) return;
    fn->call();
}
void	UserObjectManager::onSelectionChanged(TreeNode* cf){
}
void	UserObjectManager::onTextChanged(TreeNode* cf, const QString & text){
    XFuncNode* fn = getEventFuncNode(cf, "onTextChange");
    if( fn) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set( Q2A(text));
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}


void	UserObjectManager::onEditorPositionChanged(TreeNode* cf){
}
void	UserObjectManager::onCurrentSelectionChanged(TreeNode* cf){
}
void	UserObjectManager::onEditorChanged(TreeNode* cf){
}
void UserObjectManager::onChanageClipboard() {
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    XFuncNode* fn = getEventFuncNode(getInfo(), "onChanageClipboard");
	// qDebug("clip board change %s : %s ", fn?"fn ok":"fn error", mimeData?"mimeData data":"!mimeData");
    if(fn && mimeData ) {
        PARR arrs=getLocalParams(fn);
        if( mimeData->hasImage()) {
            QByteArray ba;
            QPixmap img = qvariant_cast<QPixmap>(mimeData->imageData());
            QBuffer buffer( &ba );
            buffer.open( QIODevice::WriteOnly );
            img.save( &buffer, "PNG" ); // writes pixmap into ba in PNG format
            if( img.isNull() ) {
                qDebug()<< "image is null";
            }

            //QByteArray& arr = mimeData->imageData().toByteArray();
            if( ba.length()>0 ) {
                arrs->add()->set("image");
                arrs->add()->add(ba.data(),ba.length());
            }
			// qDebug()<<"clipboardChanged === hasImage size="<< ba.length();
            // setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
        } else if (mimeData->hasHtml()) {
            arrs->add()->set("html");
            arrs->add()->set(Q2A(mimeData->html()));
            arrs->add()->set(Q2A(mimeData->text()));
        } else if (mimeData->hasText()) {
            arrs->add()->set("text");
            arrs->add()->set(Q2A(mimeData->text()));
            // qDebug()<<"clipboardChanged === hasText";
        } else if (mimeData->hasUrls()) {
            arrs->add()->set("urls");
            StrVar* var=arrs->add();
            for(int n=0,size=mimeData->urls().size();n<size;n++) {
                if( var->length() ) var->add("\n");
                var->add(Q2A(mimeData->urls().at(n).toString()));
            }
        } else {
            qDebug()<<"clipboardChanged === non display";
            // setText(tr("Cannot display data"));
        }
		qDebug("clip board change param size: %d ", arrs->size() );
        if( arrs->size() ) {
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}
// action
void UserObjectManager::onActionClick(TreeNode* cf) {
    StrVar* sv=NULL;
    TreeNode* pageNode=NULL;
    sv = cf->gv("@page");
    if( SVCHK('n',0) ) {
        pageNode=(TreeNode*)SVO;
    }
    qDebug("--- action click id:%s (%s) ---", cf->get("id"), pageNode? "ok":"");
    if( pageNode ) {
        XFuncNode* fn = getEventFuncNode(pageNode, "onAction");
        if( fn ) {
            fn->GetVar("@this")->setVar('n',0,(LPVOID)pageNode);
            PARR arrs=getLocalParams(fn);
            arrs->add()->setVar('n',0,(TreeNode*)cf);
            setFuncNodeParams(fn, arrs);
            fn->call();
        }
    }
}


void	UserObjectManager::slotPaintRequested(QPrinter* p) {
    StrVar* sv = getInfo()->GetVar("@pdfCallBack");
    qDebug("#0#pdf print preview paint request start:%s\n", SVCHK('f',0)? "ok": "fail");
    if( SVCHK('f',0) ) {
        XFuncNode* fnCur = (XFuncNode*)SVO;
        QPainter *painter=new QPainter();
        QRect paper = p->pageRect();
        TreeNode* d=NULL;
        TreeNode* tn=NULL;
        sv = fnCur->gv("@this");
        if( SVCHK('n',0) ) {
            tn = (TreeNode*)SVO;
        }
        if( tn==NULL ) return;

        sv = tn->GetVar("@d");
        if( SVCHK('n',0) ) {
            d = (TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            sv->setVar('n',0,(LPVOID)d);
        }
        // painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
        d->xstat=FNSTATE_DRAW;
        d->xtype=0;
        setVarRectF(d->GetVar("@rect"), QRectF(0,0,paper.width(),paper.height()) );
        // d->GetVar("@r")->setVar('i',4).addInt(0).addInt(0).addInt(paper.width()).addInt(paper.height());
        d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        d->GetVar("@printer")->setVar('p',11,(LPVOID)p);
        d->setInt("@pageNum",1);
        tn->GetVar("@c")->setVar('p',11,(LPVOID)p);
        PARR arrs=getLocalParams(fnCur);
        arrs->add()->setVar('n',0,(LPVOID)d);
        setFuncNodeParams(fnCur, arrs);
        if( painter->begin(p) ) {
            fnCur->call();
            painter->end();
        }
    }
}
bool UserObjectManager::event(QEvent* evt) {
    if( evt->type()==USER_EVENT ) {
        UserEvent* e = static_cast<UserEvent*>(evt);
        if( e==NULL ) {
            qDebug("user event error event object cast error");
            return false;
        }
        XFuncNode* fn=e->xcallback;
        if( fn && fn->xfunc ) {
            TreeNode* node=e->xcf;
            PARR arrs=getLocalParams(fn);
            arrs->add()->add(node->get("@eventType"));
            arrs->add()->setVar('n',0,(LPVOID)node);
            setFuncNodeParams(fn, arrs);
            fn->call();
        } else {
            qDebug("#9#postEvent function not define");
        }
        // SAFE_DELETE(e);
        return true;
    }
    return false;
}


/************************************************************************/
/*  RunGuard			                                                */
/************************************************************************/

namespace
{

QString generateKeyHash( const QString& key, const QString& salt )
{
    QByteArray data;

    data.append( key.toUtf8() );
    data.append( salt.toUtf8() );
    data = QCryptographicHash::hash( data, QCryptographicHash::Sha1 ).toHex();

    return data;
}

}


RunGuard::RunGuard( const QString& key )
    : key( key )
    , memLockKey( generateKeyHash( key, "_memLockKey" ) )
    , sharedmemKey( generateKeyHash( key, "_sharedmemKey" ) )
    , sharedMem( sharedmemKey )
    , memLock( memLockKey, 1 )
{
    memLock.acquire();
    {
        QSharedMemory fix( sharedmemKey );    // Fix for *nix: http://habrahabr.ru/post/173281/
        fix.attach();
    }
    memLock.release();
}

RunGuard::~RunGuard()
{
    release();
}

bool RunGuard::isAnotherRunning()
{
    if ( sharedMem.isAttached() )
        return false;

    memLock.acquire();
    const bool isRunning = sharedMem.attach();
    if ( isRunning )
        sharedMem.detach();
    memLock.release();

    return isRunning;
}

bool RunGuard::tryToRun()
{
    if ( isAnotherRunning() )   // Extra check
        return false;

    memLock.acquire();
    const bool result = sharedMem.create( sizeof( quint64 ) );
    memLock.release();
    if ( !result )
    {
        release();
        return false;
    }

    return true;
}

void RunGuard::release()
{
    memLock.acquire();
    if ( sharedMem.isAttached() )
        sharedMem.detach();
    memLock.release();
}


#ifdef WIN32

inline TreeNode* makeFileInfo(LPCC fullPath, int act) {
    /*
    #define FILE_ACTION_ADDED                   0x00000001
    #define FILE_ACTION_REMOVED                 0x00000002
    #define FILE_ACTION_MODIFIED                0x00000003

    struct _stat64 st;
    _stat64(fullPath,  &st );
    int pos = LastIndexOf(fullPath,'\\');
    TreeNode* node = gtrees.getTreeNode();
    node->set("type", st.st_mode & _S_IFDIR ? "folder": "file");
    node->set("wtype", act==1 ? "add": act==2 ? "delete": "update" );
    node->GetVar("name")->set(pos>0? fullPath+pos+1: fullPath);
    node->GetVar("size")->setVar('1',0).addUL64(st.st_size);
    node->GetVar("createDate")->ftime("%Y%m%dT%H%M%SZ", localtime(&st.st_ctime));
    node->GetVar("modifyDate")->ftime("%Y%m%dT%H%M%SZ", localtime(&st.st_mtime));
    node->GetVar("accessDate")->ftime("%Y%m%dT%H%M%SZ", localtime(&st.st_atime));
    getFileInode(fullPath, node->GetVar("inode")->reuse());
    return node;
    */
    return NULL;
}


VOID CALLBACK callbackWatcherRoutine(
    DWORD dwErrorCode,                // completion code
    DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
    LPOVERLAPPED lpOverlapped         // pointer to structure with I/O information
    )
{
    try
    {
        if( dwErrorCode != ERROR_SUCCESS)
            return;
        if( IsBadReadPtr(lpOverlapped,sizeof(Overlapped)) )
            return ;
        WatcherFile* w = ((Overlapped*)lpOverlapped)->w;
        if( dwErrorCode != ERROR_SUCCESS)	return;
        if(IsBadReadPtr(w,sizeof(WatcherFile)))
            return ;
        if( dwErrorCode == ERROR_SUCCESS) {
            PFILE_NOTIFY_INFORMATION pNotify;
            size_t offset =  0;

            do {
                pNotify = (PFILE_NOTIFY_INFORMATION) &w->xbuf[offset];
                offset += pNotify->NextEntryOffset;

                //[temp]
                if( pNotify->Action != FILE_ACTION_RENAMED_OLD_NAME )
                {
                    // qDebug()<<"######   "<<pNotify->FileName <<" size: "<<pNotify->FileNameLength <<" offset: "<<pNotify->NextEntryOffset;
                    w->addChange(pNotify->FileName, pNotify->FileNameLength ,(int)pNotify->Action);
                }

            } while (pNotify->NextEntryOffset != 0);
        }
        w->go();
    } catch(...) {
    }
}
#endif



WatcherFile::WatcherFile(TreeNode* cf) : xcf(cf), xbuf(NULL) {
#ifdef WIN32
    xhandle = INVALID_HANDLE_VALUE;
    xbufLen = sizeof(FILE_NOTIFY_INFORMATION) * 1024 * 128;
#endif
}
WatcherFile::~WatcherFile() {
#ifdef WIN32
    if(xhandle != INVALID_HANDLE_VALUE ) {
        CloseHandle(xhandle);
        xhandle = INVALID_HANDLE_VALUE;
    }
#endif
}
/************************************************************************/
/*  WatcherFile                                                    */
/************************************************************************/
inline LPCC w2c(PWCHAR src, int len, StrVar* buf) {
    // int size = len/sizeof(WCHAR);
    len/=sizeof(WCHAR);
    int size = WideCharToMultiByte( CP_ACP, 0, src, len, NULL, 0, NULL, NULL );
    char* data = buf->alloc(size+1);
    WideCharToMultiByte(CP_ACP, 0, src, len, data, size, NULL, NULL);
    buf->movepos(size);
    return buf->get();
}

bool WatcherFile::addChange(PWCHAR filenm, int filenmLen, int act) {
#ifdef WIN32
    static long watcherPrevTick=0;
    if( filenmLen==0 ) return false;
    WCHAR ch = filenm[0];
    if( ch==L'.' || ch==L'~') {
        return false;
    }
    StrVar* sv = xcf->gv("@callback");
    if( SVCHK('f',0) ) {
        StrVar* var=NULL;
        XFuncNode* fn = (XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('0',0).addInt(act);
        var=arrs->add();
        // sv=config()->gv("target");
        // if(sv) arrs->add()->add(sv);

        if(var) {
            // w2c(filenm, filenmLen, var);
            int len = filenmLen / sizeof(WCHAR);
            filenm[filenmLen] = 0;
            QString fnm=QString::fromStdWString(filenm);
            var->set(Q2A(fnm));
            // qDebug("xxxxxxxxxx filenm xxxxxxxxxx %d %s", len, var->get());
            /*
            if( act==3 ) {
                int len=xWatcherFile.length();
                if( (len == var->length()) &&  strncmp(xWatcherFile.get(), var->get(), len)==0 ) {
                    long dist = GetTickCount()-watcherPrevTick;
                    if( dist < 250 ) {
                        return false;
                    }
                } else {
                    long dist = GetTickCount()-watcherPrevTick;
                    if( dist < 250 ) {
                        LPCC target=config()->get("target");
                        LPCC fnm=var->get();
                        QString path= KR(target)+ "\\" +QString(fnm);
                        QFileInfo fi( path );
                        if( fi.isDir() ) return false;
                    }
                }
            }
            */
            xWatcherFile.reuse()->add(var);
        }
        // watcherPrevTick=GetTickCount();
        fn->call();
    }
#endif
    return false;
}

bool WatcherFile::start(LPCC targetPath)
{
    int len = slen(targetPath);
    if( len==0 )
        return false;

#ifdef WIN32
    char ch = targetPath[len-1];
    StrVar* sv = xcf->GetVar("target");
    sv->set(targetPath, ch=='/' || ch=='\\'?len-1: len);
    sv->replace('/','\\');
    QString path = KR(sv->get());
    // PCWCHAR path = C2W(sv->get());
    // sv->add('\\');
    qDebug("watcher path : %s", sv->get());
    xhandle = CreateFile( path.toStdWString().data(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL);
    if( xhandle == INVALID_HANDLE_VALUE)
        return FALSE;
    if( xbuf==NULL ) {
        xbuf = (BYTE*)malloc(xbufLen);
        memset(xbuf, 0, xbufLen);
        xol = malloc(sizeof(Overlapped));
        ZeroMemory(xol, sizeof(Overlapped));
        ((Overlapped*)xol)->w = this;
    }
    return go();
#else
    return true;
#endif
}


bool WatcherFile::go()
{
 #ifdef WIN32
   DWORD dwNotifyFilter = 0
        | FILE_NOTIFY_CHANGE_FILE_NAME
        | FILE_NOTIFY_CHANGE_DIR_NAME
//        | FILE_NOTIFY_CHANGE_ATTRIBUTES
//        | FILE_NOTIFY_CHANGE_SIZE
        | FILE_NOTIFY_CHANGE_LAST_WRITE
//        | FILE_NOTIFY_CHANGE_LAST_ACCESS
        | FILE_NOTIFY_CHANGE_CREATION;

    DWORD bytesReturned = 0;

    return ReadDirectoryChangesW(
                xhandle,
                (LPVOID)xbuf,
                (DWORD)xbufLen,
                TRUE,
                dwNotifyFilter,
                &bytesReturned,
                (LPOVERLAPPED)xol,
                callbackWatcherRoutine);
#else
    return false;
#endif
}
