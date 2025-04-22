#include "func_ftp.h"

MyFtp::MyFtp(TreeNode *cf, QObject *parent ) : file(NULL), ftp(NULL), xcf(cf), QObject(parent) {
}

void MyFtp::ftpStateChanged(int state) {
    XFuncNode* fn = getEventFuncNode(xcf, "onStateChange");
    if( fn  ) {
        LPCC text=NULL;
        switch (state) {
        case 0:
            text = "Unconnected";
            break;
        case 1:
            text = "HostLookup";
            break;
        case 2:
            text = "Connecting";
            break;
        case 3:
            text = "Connected";
            break;
        case 4:
            text = "LoggingIn";
            break;
        case 5:
            text = "Closing";
            break;
        default:
            text = "";
            break;
        }
        PARR arrs=getLocalParams(fn);
        arrs->add()->set(text);
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void MyFtp::ftpFinished(int request, bool error) {
    int cmd=(int)ftp->currentCommand();
    switch( cmd ) {
    case QFtp::ConnectToHost: {
        if( error ) qDebug("#0#FTP Failed to connect to host.");
    } break;
    case QFtp::Login: {
        if( error ) qDebug("#0#Failed to login.");
    } break;
    case QFtp::List: {
        if( error ) qDebug("#0#Failed to get file list.\nClosing connection." );
    } break;
    case QFtp::Cd: {
        if( error ) qDebug("#0#Failed to change directory." );
        getFileList();
    } break;
    case QFtp::Put:
    case QFtp::Get: {
        if( error ) qDebug("#0#Failed to get file?" );
        if( file ) {
            file->close();
            file->remove();
            SAFE_DELETE(file)
        }
    } break;
    default: {
      if( error ) qDebug("#0#[FTP Error] error state=%d\n", cmd );
    } break;
    }
    LPCC command=NULL;
    switch( cmd ) {
    case QFtp::SetTransferMode: command="transfer"; break;
    case QFtp::ConnectToHost: command="connect"; break;
    case QFtp::Login:	command="login"; break;
    case QFtp::Close:	command="close"; break;
    case QFtp::List:	command="list"; break;
    case QFtp::Cd:		command="cd"; break;
    case QFtp::Get:		command="get"; break;
    case QFtp::Put:		command="put"; break;
    case QFtp::Remove:	command="remove"; break;
    case QFtp::Mkdir:	command="mkdir"; break;
    case QFtp::Rmdir:	command="rmdir"; break;
    case QFtp::Rename:	command="rename"; break;
    }

    XFuncNode* fn = getEventFuncNode(xcf, "onStateFinish");
    if( fn && ftp  ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set(command);
        arrs->add()->setVar('3', error?1:0);
        if( ccmp(command,"list") && !error ) {
            StrVar* var=arrs->add();
            QByteArray ba = ftp->readAll();
            if( ba.size() ) {
                var->set(ba.constData(),ba.size());
            }
        }
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void MyFtp::ftpListInfo(QUrlInfo info) {
    XFuncNode* fn = getEventFuncNode(xcf, "onFileInfo");
    qDebug("... ftpListInfo start name=%s\n", Q2A(info.name()) );
    if( fn  ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set(info.isFile()?"file":info.isDir()?"folder":"etc");
        arrs->add()->set(Q2A(info.name()) );
        arrs->add()->setVar('1',0).addU16(info.size());
        arrs->add()->setVar('1',0).addUL64(info.lastModified().toLocalTime().toTime_t());
        arrs->add()->setVar('1',0).addUL64(info.lastRead().toLocalTime().toTime_t());
        arrs->add()->setVar('0',0).addInt(info.permissions());
        arrs->add()->set(Q2A(info.owner()) );
        arrs->add()->set(Q2A(info.group()) );
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}
void MyFtp::ftpProgress(qint64 done, qint64 total) {
    XFuncNode* fn = getEventFuncNode(xcf, "onProgress");
    if( fn  ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('1',0).addU16(done);
        arrs->add()->setVar('1',0).addU16(total);
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void MyFtp::getFileList() {
    if(ftp==NULL ) return;
    files.clear();
    qDebug("#0#MyFtp getFileList %d\n", (int)ftp->state() );
    if( ftp->state() == QFtp::LoggedIn ) {
        ftp->list();
    }
}
int MyFtp::connectHost(LPCC host, int port) {
    if( ftp ) {
        qDebug("#0#FTP already connected (%s, %d)\n",host, port);
        return 0;
    }
    ftp = new QFtp(this);
    connect( ftp ,SIGNAL(stateChanged(int)),this,SLOT(ftpStateChanged(int)));
    connect( ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(ftpFinished(int,bool)) );
    connect( ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(ftpProgress(qint64,qint64)) );
    // connect( ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(ftpListInfo(QUrlInfo)) );

    ftp->connectToHost(KR(host), port);
    return 1;
}

int MyFtp::login(LPCC userId, LPCC userPwd ) {
    if(ftp==NULL ) return 0;
    ftp->login(KR(userId), KR(userPwd));
    return 1;
}
void MyFtp::ftpCheckDisconnect() {
    if( ftp ) {
        ftp->abort();
        ftp->deleteLater();
        ftp = NULL;
        // setCursor(Qt::ArrowCursor);
    }
    if( file ) {
        file->close();
        file->remove();
        SAFE_DELETE(file)
    }
}

int MyFtp::cd(LPCC path) {
    if(ftp==NULL) return 0;
    ftp->cd(KR(path));
    return 1;
}

int MyFtp::put(LPCC localFileName, LPCC remoteFileName) {
    if(ftp==NULL) return 0;
    if(slen(localFileName)==0 || slen(remoteFileName)==0 ) return 0;
    if(file) {
        qDebug("#0#FTP Upload Fail %s, %s\n", localFileName, remoteFileName);
        return 0;
    }
    QFileInfo fi(KR(localFileName));
    if( !fi.isFile() ) {
        qDebug("#0#FTP Upload error file not found %s, %s\n", localFileName, remoteFileName);
        return 0;
    }
    file=new QFile(KR(localFileName));
    if( file->open(QIODevice::ReadOnly) ) {
        ftp->put(file,KR(remoteFileName));
    } else {
        qDebug("#0#FTP Upload error file open error %s, %s\n", localFileName, remoteFileName);
        return 0;
    }
    return 1;
}
int MyFtp::get(LPCC remoteFileName, LPCC saveFileName) {
    if(ftp==NULL) return 0;
    if(slen(saveFileName)==0 || slen(remoteFileName)==0 ) return 0;
    if(file) {
        qDebug("#0#FTP Download Fail %s, %s\n", saveFileName, remoteFileName);
        return 0;
    }
    file=new QFile(KR(saveFileName));
    if( file->open(QIODevice::WriteOnly) ) {
        ftp->get(KR(remoteFileName),file);
    } else {
        qDebug("#0#FTP Download error file open error %s, %s\n", saveFileName, remoteFileName);
        return 0;
    }
    return 1;
}
int MyFtp::setMode(LPCC mode) {
    if(ftp==NULL) return 0;
    if(slen(mode)==0) mode="active";
    ftp->setTransferMode(ccmp(mode,"active")?QFtp::Active :QFtp::Passive);
    return 1;
}

bool callFtpFunc(XFunc* fc, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst) {
    if( tn==NULL )
        return false;
    MyFtp* pftp=NULL;
    StrVar* sv=tn->GetVar("@c");
    if( SVCHK('v',11) ) {
        pftp=(MyFtp*)SVO;
    } else {
        pftp = new MyFtp(tn);
        sv->setVar('v',11,(LPVOID)pftp);
    }
    U32 ref = fc->xfuncRef;
    if( ref==0 ) {
        LPCC fnm=fc->getFuncName();
        ref= ccmp(fnm,"connect")?1:
             ccmp(fnm,"close")?2:
             ccmp(fnm,"login")?3:
             ccmp(fnm,"state")?4:
             ccmp(fnm,"is")?5:
             ccmp(fnm,"mode")?6:
             ccmp(fnm,"cd")?10:
             ccmp(fnm,"put")?11:
             ccmp(fnm,"get")?12:
             ccmp(fnm,"list")?13:
             ccmp(fnm,"remove")?14:
             ccmp(fnm,"mkdir")?15:
             ccmp(fnm,"rmdir")?16:
             ccmp(fnm,"rename")?17:
             0;
        fc->xfuncRef=ref;

    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {   // connect
        if( arrs==NULL ) return true;
        LPCC host=AS(0);
        int port=toInteger(arrs->get(1));
        if(port==0) port=21;
        rst->setVar('0',0).addInt( pftp->connectHost(host, port) );
    } break;
    case 2: { // close
        pftp->ftpCheckDisconnect();
    } break;
    case 3: { // login
        if( arrs==NULL ) return true;
        LPCC user=AS(0), pwd=AS(1);
        rst->setVar('0',0).addInt( pftp->login(user, pwd) );
    } break;
    case 4: { // state
        rst->setVar('0',0).addInt( (int)pftp->getState() );
    } break;
    case 5: { // is
        rst->setVar('0',0).addInt( (int)pftp->getCommand() );
    } break;
    case 6: { // mode
        LPCC mode=NULL;
        if( arrs ) mode=AS(0);
        pftp->setMode(mode);
    } break;
    case 10: { // cd
        LPCC path=NULL;
        if( arrs ) path=AS(0);
        rst->setVar('0',0).addInt(pftp->cd(path) );
    } break;
    case 11: { // put
        if( arrs==NULL ) return true;
        LPCC local=AS(0), remote=AS(1);
        rst->setVar('0',0).addInt(pftp->put(local, remote) );
    } break;
    case 12: { // get
        if( arrs==NULL ) return true;
        LPCC remote=AS(0), local=AS(1);
        rst->setVar('0',0).addInt(pftp->get(remote, local) );

    } break;
    case 13: { // list
        pftp->getFileList();
    } break;
    case 14: { // remove
        if( arrs==NULL ) return true;
        LPCC name=AS(0);
        rst->setVar('0',0).addInt(pftp->ftp->remove(KR(name)) );
    } break;
    case 15: { // mkdir
        if( arrs==NULL ) return true;
        LPCC name=AS(0);
        rst->setVar('0',0).addInt(pftp->ftp->mkdir(KR(name)) );

    } break;
    case 16: { // rmdir
        if( arrs==NULL ) return true;
        LPCC name=AS(0);
        rst->setVar('0',0).addInt(pftp->ftp->rmdir(KR(name)) );

    } break;
    case 17: { // rename
        if( arrs==NULL ) return true;
        LPCC oldname=AS(0), newname=AS(1);
        rst->setVar('0',0).addInt(pftp->ftp->rename(KR(oldname), KR(newname)) );

    } break;
    default: return false;
    }
    return true;
}
