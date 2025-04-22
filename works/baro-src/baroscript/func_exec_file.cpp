#include "func_util.h"
#include "../fobject/FMutex.h"


void regFileFuncs() {
    if( isHashKey("file") )
        return;
    PHashKey hash = getHashKey("file");
    U16 uid = 1;
    hash->add("isFile", uid);		uid++;	// 1
    hash->add("isFolder", uid);		uid++;	// 2
    hash->add("isExecutable", uid);	uid++;	// 3
    hash->add("fileHash", uid);		uid++;	// 4
    hash->add("inode", uid);			uid++;	// 5
    hash->add("readAll", uid);		uid++;	// 6
    hash->add("writeAll", uid);		uid++;
    hash->add("open", uid);			uid++;
    hash->add("read", uid);			uid++;
    hash->add("write", uid);			uid++; // 10
    hash->add("append", uid);		uid++;
    hash->add("size", uid);			uid++;
    hash->add("path", uid);			uid++;
    hash->add("close", uid);			uid++;
    hash->add("pos", uid);			uid++;
    hash->add("seek", uid);			uid++;
    hash->add("flush", uid);		uid++;
    hash->add("mkdir", uid);			uid++;
    hash->add("rmDir", uid);		uid=21;
    hash->add("modifyDate", uid);	uid++;
    hash->add("createDate", uid);	uid++;
    hash->add("rename", uid);		uid++;
    hash->add("copy", uid);			uid++;
    hash->add("delete", uid);		uid++;
    hash->add("list", uid);			uid++;
    hash->add("timeStamp", uid);		uid++;
    uid=172;
    hash->add("move", uid);			uid++;
    hash->add("readBase64", uid);	uid++;
    hash->add("writeDecode", uid);	uid++;
}

FMutex      gMutexFileAppend;
inline bool getReadAllFile(LPCC path, StrVar* rst, UL64 offset) {
    rst->reuse();
    QFile file(KR(path));
    if( !file.open(QFile::ReadOnly)) {
        return false;
    }
    if( offset ) {
        file.seek(offset);
    }
    QByteArray ba = file.readAll();
    rst->reuse()->add(ba.constData(), ba.size());
    file.close();
    return true;
}
bool callFileFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    if( fc->xfuncRef==0  ) {
        regFileFuncs();
        fc->xfuncRef = getHashKeyVal("file",fnm);
    }
    if( fc->xfuncRef==0 )
        return false;
    StrVar* sv=NULL;
    switch(fc->xfuncRef) {
    case 1: {	// isFile
        if( arrs==NULL ) return false;
        QFileInfo fi(KR(AS(0)));
        rst->setVar('3',fi.isFile()?1:0);
    } break;
    case 2: {	// isFolder
        if( arrs==NULL ) return false;
        QFileInfo fi(KR(AS(0)));
        rst->setVar('3',fi.isDir()?1:0);
    } break;
    case 3: {	// isExecutable
        if( arrs==NULL ) return false;
        QFileInfo fi(KR(AS(0)));
        rst->setVar('3',fi.isExecutable()?1:0);
    } break;
    case 4: {	// fileHash
        LPCC path=arrs? AS(0): tn->get("path");
        if( slen(path) )
            rst->set(GetFileHash(KR(path), tn) );
    } break;
    case 5: {	// inode
        LPCC path=arrs? AS(0): tn->get("path");
        if( slen(path) )
            getFileInode(path, rst);
    } break;
    case 6: {	// readAll
        if( arrs==NULL ) return false;
        LPCC path=AS(0);
        UL64 offset = 0;
        if( isNumberVar(arrs->get(1)) ) {
            offset = toUL64(arrs->get(1));
            getReadAllFile(path, rst, offset);
        } else {
            fileReadAll(path, rst);
        }
    } break;
    case 7: {	// writeAll
        if( arrs==NULL ) return false;
        LPCC path=AS(0);
        QFile file(KR(path));
        sv=arrs->get(1);
        if( !file.open(QFile::WriteOnly) && sv ) {
            rst->setVar('3',0);
            qDebug("#9##[%s] file wirteAll error (filePath:%s)\n", getBaseFuncName(fn), path);
            return true;
        }
        if( SVCHK('i',7) ) {
            file.write(sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
        } else {
            int sp=0, ep=0;
            StrVar* data = getStrVarPointer(sv,&sp,&ep);
            if( sp<ep ) file.write(data->get(sp), ep-sp);
        }
        file.close();
    } break;
    case 8: {	// open
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
            if( arrs==NULL ) {
                rst->setVar('3',1);
                return true;
            }
            w->close();
            sv->reuse();
            SAFE_DELETE(w);
        }
        if( arrs==NULL ) {
            rst->setVar('3',0);
            return true;
        }
        LPCC path=AS(0);
        LPCC mode=AS(1);
        w=new QFile(KR(path));
        bool bchk=false;
        if( ccmp(mode,"read") ) {
            bchk=w->open(QFile::ReadOnly);
        } else if( ccmp(mode,"write") ) {
            bchk=w->open(QFile::WriteOnly);
        } else if( ccmp(mode,"append") ) {
            bchk=w->open(QFile::Append);
        } else {
            bchk=w->open(QFile::ReadOnly);
        }
        if( bchk==false ) {
            rst->setVar('3',0);
            SAFE_DELETE(w);
            return true;
        }
        tn->GetVar("@c")->setVar('z',1,(LPVOID)w);
        tn->GetVar("@path")->set(path);
        rst->setVar('3',1);
    } break;
    case 9: {	// read
        QFile* w = NULL;
        if( gMutexFileAppend.IsLocked() ) {
            qDebug("#0#file read lock\n");
            return true;
        }
        gMutexFileAppend.EnterMutex();
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w ) {
            int cnt=arrs?arrs->size():0;
            if( cnt==0 ) {
                QByteArray ba = w->readAll();
                rst->reuse()->add(ba.constData(), ba.size());
            } else if( cnt==1 ) {
                if( isNumberVar(arrs->get(0)) ) {
                    UL64 r=toUL64(arrs->get(0));
                    QByteArray ba = w->read(r);
                    rst->reuse()->add(ba.constData(), ba.size());
                }
            } else if( cnt==2 ) {
                if( isNumberVar(arrs->get(0)) ) {
                    w->seek(toUL64(arrs->get(0)) );
                    QByteArray ba = w->readAll();
                    rst->reuse()->add(ba.constData(), ba.size());
                }
            }
        } else {
            qDebug("#9##[%s] file read error (file not open)\n", getBaseFuncName(fn) );
        }
        gMutexFileAppend.LeaveMutex();
    } break;
    case 10: {	// write
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w==NULL ) {
            qDebug("#9##[%s] file write error (file not open)\n", getBaseFuncName(fn) );
            return true;
        }
        int cnt=arrs?arrs->size():0;
        if( cnt>1 && isNumberVar(arrs->get(1)) ) {
            w->seek(toUL64(arrs->get(1)) );
        }
        int sp=0, ep=0;
        sv=getStrVarPointer( arrs->get(0), &sp, &ep);
        if( sp < ep ) {
            w->write(sv->get(sp), ep-sp );
            tn->GetVar("@wpos")->setVar('1',0).addUL64(w->pos());
        }
    } break;
    case 11: {	// append
        if( arrs==NULL ) return true;
        if( gMutexFileAppend.IsLocked() ) {
            qDebug("#0#file append lock\n");
        }
        gMutexFileAppend.EnterMutex();
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w ) {
            int cnt=arrs->size();
            tn->GetVar("@wpos")->setVar('1',0).addUL64(w->pos());
            if(cnt==1) {
                int sp=0, ep=0;
                sv=getStrVarPointer( arrs->get(0), &sp, &ep);
                if( sp < ep ) {
                    w->write(sv->get(sp), ep-sp );
                }
            } else {
                // ex) fo.append("fileNm", true);
                LPCC fileNm=AS(0);
                if( isFile(fileNm) ) {
                    QString filePath(KR(fileNm));
                    QFile file(filePath);
                    if( file.open(QIODevice::ReadOnly) ) {
                        w->write(file.readAll());
                        file.close();
                    } else {
                        qDebug("#9#append source file open error (filename:%s)\n", fileNm);
                    }
                } else {
                    qDebug("#9#append source file not found (filename:%s)\n", fileNm);
                }
            }
        } else {
            qDebug("#9##[%s] file append error (file not open)\n", getBaseFuncName(fn) );
        }
        gMutexFileAppend.LeaveMutex();
    } break;
    case 12: {	// size
        if( arrs ) {
            LPCC path=AS(0);
            if( slen(path) ) {
                QFile file(KR(path));
                rst->setVar('1',0).addUL64(file.size());
            }
            return true;
        }
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        UL64 fileSize=0;
        if( w ) {
            fileSize=w->size();
        } else {
            qDebug("#9##[%s] file append error (file not open)\n", getBaseFuncName(fn) );
        }
        rst->setVar('1',0).addUL64(fileSize);
    } break;
    case 13: {	// path
        rst->reuse()->add(tn->get("@path"));
    } break;
    case 14: {	// close
        QFile* w = NULL;
        tn->GetVar("@path")->reuse();
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w==NULL ) {
            qDebug("#9##[%s] file close error (file not open)\n", getBaseFuncName(fn) );
            return true;
        }
        w->close();
        sv->reuse();
        SAFE_DELETE(w);
    } break;
    case 15: {	// pos
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w==NULL ) {
            qDebug("#9##[%s] file pos error (file not open)\n", getBaseFuncName(fn) );
            return true;
        }
        if( arrs==NULL ) {
            rst->setVar('1',0).addUL64(w->pos());
        } else {
            LPCC mode=AS(0);
            sv=ccmp(mode,"read")?tn->gv("@rpos"):tn->gv("@wpos");
        }
        if( SVCHK('1',0) ) rst->reuse()->add(sv);
    } break;
    case 16: {	// seek
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w==NULL ) {
            qDebug("#9##[%s] file seek error (file not open)\n", getBaseFuncName(fn) );
            return true;
        }
        if( arrs ) {
            sv=arrs->get(0);
            qint64 offset=w->size();
            if( isNumberVar(sv) ) {
                offset=(qint64)toUL64(sv);
                if( offset<0 )
                    offset = w->size()+offset;
                if( offset<0 )
                    offset = 0;
                if( offset>w->size() )
                    offset = w->size();
            }
            w->seek(offset);
            rst->setVar('1',0).addUL64(w->size()-offset);
        }
    } break;
    case 17: {	// flush
        // int cnt=arrs? arrs->size():0;
        QFile* w = NULL;
        sv=tn->GetVar("@c");
        if( SVCHK('z',1) ) {
            w=(QFile*)SVO;
        }
        if( w==NULL ) {
            qDebug("#9##[%s] file flush error (file not open)\n", getBaseFuncName(fn) );
            return true;
        }
        rst->setVar('3', w->flush() );
    } break;
    case 18: {	// mkdir
        LPCC path=NULL;
        if( arrs ) {
            path=AS(0);
        } else {
            path=tn->get("@path");
        }
        QDir dir = QDir::root();
        if( isVarTrue(arrs->get(1)) ) {
            rst->setVar('3',dir.mkpath(KR(path))?1:0 );
        } else {
            rst->setVar('3',dir.mkdir(KR(path))?1:0 );
        }
    } break;
    case 19: {	// rmDir
        if( arrs==NULL ) return false;
        LPCC path=NULL;
        if( arrs ) {
            path=AS(0);
        } else {
            path=tn->get("@path");
        }
        QDir dir = QDir::root();
        if( isVarTrue(arrs->get(1)) ) {
            rst->setVar('3',dir.rmpath(KR(path))?1:0 );
        } else {
            rst->setVar('3',dir.rmdir(KR(path))?1:0 );
        }
    } break;
    case 21: {	// modifyDate
        LPCC path=NULL;
        if( arrs ) {
            path=AS(0);
        } else {
            path=tn->get("@path");
        }
        QFileInfo fi(KR(path));
        if( fi.isFile() ) {
            rst->setVar('1',0).addUL64(fi.lastModified().toLocalTime().toTime_t());
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 22: {	// createDate
        LPCC path=NULL;
        if( arrs ) {
            path=AS(0);
        } else {
            path=tn->get("@path");
        }
        QFileInfo fi(KR(path));
        if( fi.isFile() ) {
            rst->setVar('1',0).addUL64(fi.created().toLocalTime().toTime_t());
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 23: {	// renanme
        if( arrs==NULL ) return false;
        LPCC path=AS(0), newPath = AS(1);
        QFileInfo fi(KR(newPath));
        if( !fi.isFile() && QFile::rename(KR(path),KR(newPath)) ) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 24: {	// copy
        if( arrs==NULL ) return false;
        LPCC path=AS(0), dest = AS(1);
        QFileInfo fi(KR(path));
        rst->setVar('3',0);
        if( fi.isFile() ) {
            if( QFile::copy(KR(path), KR(dest)) )  rst->setVar('3',1);
        }
    } break;
    case 25: {	// delete
        if( arrs==NULL ) return false;
        LPCC path=AS(0);
        QFileInfo fi(KR(path));
        rst->setVar('3',0);
        if( fi.isFile() ) {
            if( QFile::remove(KR(path)) ) {
                rst->setVar('3',1);
            }
        }
    } break;
    case 26: {	// list
        if( arrs==NULL ) return false;
        TreeNode* root=tn;
        LPCC path=AS(0);
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            root=(TreeNode*)SVO;
            sv=arrs->get(2);
        }
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            getFileInfoListFunc(fn, fsrc, path, root, tn, rst );
        }
    } break;
    case 27: {	// timeStamp
        if( arrs==NULL ) return false;
        LPCC fileName=AS(0);
        if( slen(fileName)==0 ) {
            qDebug("file timeStamp error: file not found\n");
            return true;
        }
        QDateTime cdt, mdt;
        if( getDateTimeVar(cdt, arrs->get(1), rst) ) {
            if( !getDateTimeVar(mdt, arrs->get(2), rst) ) {
                mdt=cdt;
            }
            QString fnm=KR(fileName);
            if( setFileTimeStamp(fnm, cdt, mdt) ) {
                rst->setVar('3',1);
                return true;
            }
        } else {
            qDebug("#9#file timeStamp error: file time not valid (file :%s)\n",fileName);
        }
        rst->setVar('3',0);
    } break;
    case 172: {	// move
        if( arrs==NULL ) return false;
        LPCC path=AS(0), rename = AS(1);
        if( fileRename(path, rename ) ) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 173: {	// readBase64
        if( arrs==NULL ) return false;
        if( gMutexFileAppend.IsLocked() ) {
            qDebug("#0#file read base64 lock\n");
            return true;
        }
        LPCC path=AS(0);
        QFile w(KR(path));
        gMutexFileAppend.EnterMutex();
        if( w.open(QFile::ReadOnly) ) {
            QByteArray ba=w.readAll().toBase64();
            rst->add(ba.constData(), ba.size());
            qDebug("read base64 path:%s [size:%d]", path, ba.size() );
            w.close();
        } else {
            qDebug("#9##[%s] file read base64 error (file not open)\n", getBaseFuncName(fn) );
        }
        gMutexFileAppend.LeaveMutex();
    } break;
    case 174: {	// writeDecode
        if( arrs==NULL ) return false;
        if( gMutexFileAppend.IsLocked() ) {
            qDebug("#0#file writeDecode lock\n");
            return true;
        }
        LPCC path=AS(0);
        int sp=0, ep=0;
        StrVar* var = getStrVarPointer(arrs->get(1),&sp, &ep);
        if(sp>=ep ) {
            qDebug("write decode error no data [path:%s]", path);
            return true;
        }
        QFile w(KR(path));
        gMutexFileAppend.EnterMutex();
        if( w.open(QFile::WriteOnly) ) {
            QByteArray ba=QByteArray::fromBase64(QByteArray(var->get(sp), ep-sp));
            w.write(ba);
            w.close();
        } else {
            qDebug("#9##[%s] file read base64 error (file not open)\n", getBaseFuncName(fn) );
        }
        gMutexFileAppend.LeaveMutex();
    } break;
    default: return false;
    }
    return true;
}

void getFileInfoListFunc(XFuncNode* fn, XFuncSrc* fsrc, LPCC path, TreeNode* root, TreeNode* tn, StrVar* rst) {
    if( rst ) rst->setVar('3',0);
    if( tn==NULL || fsrc==NULL || slen(path)==0 ) return;
    DirFileInfo fi(KR(path));
    U32 sortFlag=0, fliterFlag=0;
    StrVar* sv=tn->gv("@sort");
    if( sv && sv->length() ) {
        XParseVar pv(sv,0,sv->length() );
        while( pv.valid() ) {
            LPCC type=pv.findEnd(",").v();
            if( ccmp(type,"name") ) sortFlag|=QDir::Name;
            else if( ccmp(type,"time") ) sortFlag|=QDir::Time;
            else if( ccmp(type,"size") ) sortFlag|=QDir::Size;
            else if( ccmp(type,"type") ) sortFlag|=QDir::Type;
            else if( ccmp(type,"desc") ) sortFlag|=QDir::Reversed;
            else if( ccmp(type,"case") ) sortFlag&=~QDir::IgnoreCase;
        }
    } else {
        sortFlag=QDir::IgnoreCase;
    }
    sv=tn->gv("@filter");
    if( sv && sv->length() ) {
        XParseVar pv(sv,0,sv->length() );
        while( pv.valid() ) {
            LPCC type=pv.findEnd(",").v();
            if( ccmp(type,"folders") ) fliterFlag|=QDir::Dirs;
            else if( ccmp(type,"all") ) fliterFlag|=QDir::AllDirs|QDir::Files;
            else if( ccmp(type,"allEntry") ) fliterFlag|=QDir::AllEntries;
            else if( ccmp(type,"files") ) fliterFlag|=QDir::Files;
            else if( ccmp(type,"drivers") ) fliterFlag|=QDir::Drives;
            else if( ccmp(type,"readable") ) fliterFlag|=QDir::Readable;
            else if( ccmp(type,"writable") ) fliterFlag|=QDir::Writable;
            else if( ccmp(type,"executable") ) fliterFlag&=~QDir::Executable;
            else if( ccmp(type,"hidden") ) fliterFlag|=QDir::Hidden;
            else if( ccmp(type,"system") ) fliterFlag|=~QDir::System;
        }
    } else {
        fliterFlag=QDir::NoDotAndDotDot|QDir::AllEntries;
    }
    sv=tn->gv("@nameFilter");
    if( sv && sv->length() ) {
        fi.nameFilters(sv);
    }
    fi.setDir(sortFlag, fliterFlag);
    QFileInfo* info=fi.next();
    if( info ) {
        XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
        if(root) {
            fn->GetVar("@this")->setVar('n',0,(LPVOID)root);
        } else {
            sv=fn->gv("@this");
            if( sv ) {
                fnCur->GetVar("@this")->reuse()->add(sv);
            }
        }
        fnCur->GetVar("info")->setVar('f',11,(LPVOID)info);
        fnCur->GetVar("@first")->setVar('3',1);
        fnCur->GetVar("@dir")->setVar('f',12,(LPVOID)&fi);
        fnCur->call(NULL, rst);
        funcNodeDelete(fnCur);
    }
    sv=tn->gv("@sort");         if( sv ) sv->reuse();
    sv=tn->gv("@filter");       if( sv ) sv->reuse();
    sv=tn->gv("@nameFilter");   if( sv ) sv->reuse();
}

bool callFileInfoFunc(XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst) {
    if( !SVCHK('f',11) ) return false;
    QFileInfo* info=(QFileInfo*)SVO;
    if( fc->xfuncRef==0 ) {
        LPCC fnm=fc->getFuncName();
        fc->xfuncRef =
            ccmp(fnm,"fullPath") ? 1 :
            ccmp(fnm,"relativePath") ? 2 :
            ccmp(fnm,"path") ? 3 :
            ccmp(fnm,"baseName") ? 4 :
            ccmp(fnm,"name") ? 5 :
            ccmp(fnm,"is") ? 6 :
            ccmp(fnm,"createDt") ? 7 :
            ccmp(fnm,"modifyDt") ? 8 :
            ccmp(fnm,"readDt") ? 9 :
            ccmp(fnm,"size") ? 10 :
            ccmp(fnm,"ext") ? 11 :
            ccmp(fnm,"suffix") ? 12 :
            ccmp(fnm,"next") ? 13 :
            ccmp(fnm,"valid") ? 90 :
            ccmp(fnm,"with") ? 991 :
            ccmp(fnm,"inject") ? 992 :
            0;
    }
    rst->reuse();
    switch( fc->xfuncRef ) {
    case 1: {	// fullPath
        if( info->isDir() ) {
            rst->set(Q2A(info->absolutePath()) );
        } else {
            rst->set(Q2A(info->absoluteFilePath()) );
        }
    } break;
    case 2: {	// relativePath
        if( info->isDir() ) {
            rst->set(Q2A(info->canonicalPath()) );
        } else {
            rst->set(Q2A(info->canonicalFilePath()) );
        }
    } break;
    case 3: {	// path
        rst->set(Q2A(info->path()) );
    } break;
    case 4: {	// baseName
        rst->set(Q2A(info->baseName()) );
    } break;
    case 5: {	// name
        rst->set(Q2A(info->fileName()) );
    } break;
    case 6: {	// is
        LPCC type = ( arrs==NULL ) ? "file": AS(0);
        bool chk=ccmp(type,"file")? info->isFile() :
            ccmp(type,"folder") ||ccmp(type,"dir") ? info->isDir() :
            ccmp(type,"relative") ? info->isRelative() :
            ccmp(type,"link") ? info->isSymLink() :
            ccmp(type,"hidden") ? info->isHidden() :
            ccmp(type,"readable")? info->isReadable() :
            ccmp(type,"writable")? info->isWritable() :
            false;
        rst->setVar('3', chk?1:0);
    } break;
    case 7: {	// createDt
        U32 tm=info->created().toLocalTime().toTime_t();
        rst->setVar('1',0).addUL64(tm);
    } break;
    case 8: {	// modifyDt
        U32 tm=info->lastModified().toLocalTime().toTime_t();
        rst->setVar('1',0).addUL64(tm);
    } break;
    case 9: {	// readDt
        U32 tm=info->lastRead().toLocalTime().toTime_t();
        rst->setVar('1',0).addUL64(tm);
    } break;
    case 10: {	// size
        rst->setVar('1',0).addUL64((UL64)info->size());
    } break;
    case 11: {	// ext
        rst->set(Q2A(info->suffix()) );
    } break;
    case 12: {	// suffix
        rst->set(Q2A(info->completeSuffix()) );
    } break;
    case 13: {	// next
        sv=getFuncVar(fn, "@dir");
        rst->reuse();
        if( SVCHK('f',12) ) {
            XFunc* pfc=fc->parent();
            DirFileInfo* dfi=(DirFileInfo*)SVO;
            if( pfc && (pfc->getType()==FTYPE_CONTROL||pfc->getType()==FTYPE_FUNCCHECK) ) {
                sv=fn->GetVar("@first");
                if( SVCHK('3',1) ) {
                    sv->setVar('3',0);
                } else {
                    info=dfi->next();
                }
            } else {
                info=dfi->next();
            }
            if( info ) {
                rst->setVar('3',1);
            } else {
                fn->GetVar("info")->reuse();
                rst->setVar('3',0);
            }
        }
    } break;
    case 991:
    case 992: {	// inject, with
        int cnt=fc->getParamSize();
        LPCC fnm=NULL, varNm=NULL;
        for( int n=0;n<cnt;n++) {
            XFunc* fcParam=fc->getParam(n);
            if( fcParam->xftype==FTYPE_TEXT || fcParam->xftype==FTYPE_STR ) {
                StrVar* textVar=getStrVarTemp(); // gum.getStrVar();
                takeParseVar(textVar,fcParam,fn);
                XParseVar pv(textVar );
                while( pv.valid() ) {
                    char ch=pv.ch();
                    if( ch==0 ) break;
                    varNm=pv.NextWord();
                    if( pv.ch()==':' ) {
                        pv.incr();
                        fnm=pv.NextWord();
                    } else {
                        fnm=varNm;
                    }
                    if( ccmp(fnm,"name") ) {
                        fn->GetVar(varNm)->set(Q2A(info->fileName()));
                    } else if( ccmp(fnm,"fullPath") ) {
                        fn->GetVar(varNm)->set(Q2A(info->absoluteFilePath()));
                    } else if( ccmp(fnm,"path") ) {
                        fn->GetVar(varNm)->set(Q2A(info->path()));
                    } else if( ccmp(fnm,"relativePath") ) {
                        fn->GetVar(varNm)->set(Q2A(info->canonicalFilePath()));
                    } else if( ccmp(fnm,"baseName") ) {
                        fn->GetVar(varNm)->set(Q2A(info->baseName()));
                    } else if( ccmp(fnm,"ext") ) {
                        fn->GetVar(varNm)->set(Q2A(info->suffix()));
                    } else if( ccmp(fnm,"type") ) {
                        fn->GetVar(varNm)->set(info->isDir() ? "folder":"file");
                    } else if( ccmp(fnm,"size") ) {
                        fn->GetVar(varNm)->setVar('1',0).addUL64((UL64)info->size());
                    } else if( ccmp(fnm,"modifyDt") ) {
                        fn->GetVar(varNm)->setVar('1',0).addUL64(info->lastModified().toLocalTime().toTime_t());
                    } else if( ccmp(fnm,"createDt") ) {
                        fn->GetVar(varNm)->setVar('1',0).addUL64(info->created().toLocalTime().toTime_t());
                    }
                    if( pv.ch()==',' ) {
                        pv.incr();
                    } else {
                        break;
                    }
                }
                break;
            } else {
                fnm = fcParam->getValue();
                varNm=fnm;
                if( ccmp(fnm,"name") ) {
                    fn->GetVar(varNm)->set(Q2A(info->fileName()));
                } else if( ccmp(fnm,"fullPath") ) {
                    fn->GetVar(varNm)->set(Q2A(info->absoluteFilePath()));
                } else if( ccmp(fnm,"path") ) {
                    fn->GetVar(varNm)->set(Q2A(info->path()));
                } else if( ccmp(fnm,"relativePath") ) {
                    fn->GetVar(varNm)->set(Q2A(info->canonicalFilePath()));
                } else if( ccmp(fnm,"baseName") ) {
                    fn->GetVar(varNm)->set(Q2A(info->baseName()));
                } else if( ccmp(fnm,"ext") ) {
                    fn->GetVar(varNm)->set(Q2A(info->suffix()));
                } else if( ccmp(fnm,"type") ) {
                    fn->GetVar(varNm)->set(info->isDir() ? "folder":"file");
                } else if( ccmp(fnm,"size") ) {
                    fn->GetVar(varNm)->setVar('1',0).addUL64((UL64)info->size());
                } else if( ccmp(fnm,"modifyDt") ) {
                    fn->GetVar(varNm)->setVar('1',0).addUL64(info->lastModified().toLocalTime().toTime_t());
                } else if( ccmp(fnm,"createDt") ) {
                    fn->GetVar(varNm)->setVar('1',0).addUL64(info->created().toLocalTime().toTime_t());
                }
            }
        }

    } break;
    default: break;
    }
    return true;
}

int fileRename( LPCC src, LPCC newName) {
    QString szSrc=KR(src);
    QFile file(szSrc);
    if( !file.exists()) {
        return 0;
    }
    QDir dir;
    QString szNewName=KR(newName);
    if( dir.rename(szSrc, szNewName) ) {
        return 1;
    }
    return 0;
}

bool getDateTimeVar(QDateTime& dt, StrVar* sv, StrVar* rst) {
    if( sv==NULL ) return false;
    time_t tt = 0;
    if( isNumberVar(sv) ) {
        tt = (time_t)toUL64(sv);
    } else {
        LPCC dtm = toString(sv);
        int len=slen(dtm);
        if( len==0 || ccmp(dtm,"now") ) {
            tt = time(&tt);
        } else {
            if( len==8 ) {
                dtm=gBuf.fmt("%s000000", dtm);
            } else if( len==10 ) {
                dtm=gBuf.fmt("%s 00:00:00", dtm);
            }
            if( slen(dtm) ) {
                QDateTime dt = getDateTime(dtm);
                tt = dt.toTime_t();
            }
        }
    }
    if( tt ) {
        rst->reuse();
        rst->ftime("%Y%m%dT%H%M%S",localtime(&tt));
        dt=getDateTime(rst->get());
        return true;
    }
    return false;
}

inline void QDT2ST(QDateTime dt, SYSTEMTIME *st) {
    QDateTime localDT = dt.toLocalTime();
    QDate localDate = localDT.date();
    QTime localTime = localDT.time();
    st->wYear   = localDate.year();
    st->wMonth  = localDate.month();
    st->wDay    = localDate.day();
    st->wHour   = localTime.hour();
    st->wMinute = localTime.minute();
    st->wSecond = localTime.second();
    st->wMilliseconds = localTime.msec();
}

bool setFileTimeStamp(const QString &fname, const QDateTime &cdt, const QDateTime &ldt) {
    SYSTEMTIME st={0};
    FILETIME ft, ftCreation, ftLastwrite;

    QDT2ST(cdt, &st);
    SystemTimeToFileTime(&st, &ft);
    LocalFileTimeToFileTime(&ft,&ftCreation);

    QDT2ST(ldt, &st);
    SystemTimeToFileTime(&st, &ft);
    LocalFileTimeToFileTime(&ft,&ftLastwrite);

    QString filepath = QDir::toNativeSeparators(fname);
    LPCWSTR fileName=(LPCWSTR)filepath.toStdWString().c_str();
    HANDLE hFile = CreateFile(fileName,
        GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
        );

    if(hFile != INVALID_HANDLE_VALUE) {
        bool bRet=false;
        if(SetFileTime(hFile, &ftCreation, &ftLastwrite, &ftLastwrite))
        {
            bRet=true;
        }
        CloseHandle(hFile);
        return bRet;
    }
    return false;
}

//
// FILE HASH
//
inline qint64 GetBlockCnt(const qint64& iPos,const qint64& iBlockSize) {
    if(iPos < iBlockSize)
        return 1;
    qint64 iTotalPos = iPos, iCnt=0;
    qint64 iResidue = iTotalPos % iBlockSize;
    if(iPos == 0)
        iCnt = 0;
    else if((iResidue) == 0)
        iCnt = iTotalPos / iBlockSize;
    else
    {
        iTotalPos = iTotalPos - iResidue;
        iCnt = (iTotalPos / iBlockSize ) + 1;
    }
    return iCnt;
}

inline bool IsLastBlock(const qint64& iPos,const qint64& iFileSize, const qint64& iBlockSize) {
    if( (GetBlockCnt(iFileSize,iBlockSize) - GetBlockCnt(iPos,iBlockSize)) == 0)
        return true;
    else
        return false;
}

inline qint64 GetPosDepth(const qint64& iBlockPos ) {
    if(iBlockPos == 0)
        return 0;
    else if(iBlockPos == 1)
        return 2;
    else if(iBlockPos == 2)
        return 3;
    qint64 iResult=0,iCurrPos = 1, iFrontSize , iDepth=2, iPartialPos=0;
    QList<qint64> ResultList;
    ResultList.clear();
    while(iCurrPos < iBlockPos) {
        iFrontSize = iCurrPos;
        iCurrPos = iCurrPos * 2;
        iDepth++;
        ResultList += iCurrPos;
    }

    iPartialPos = iBlockPos - iFrontSize;

    if( ResultList.contains(iBlockPos)) {
        return iDepth;
    } else {
        return GetPosDepth(iPartialPos);
    }
    return iResult;
}

inline qint64 GetDepth(const qint64& iBlockPos, const qint64& iBlockSize) {
    qint64 iResult,iBlockCnt = GetBlockCnt(iBlockPos,iBlockSize);
    int j = iBlockCnt % 4;
    switch(j) {
    case 1:
        iResult = 0;
        break;
    case 2:
        iResult = 1;
        break;
    case 3:
        iResult = 0;
        break;
    case 0:
        iResult = GetPosDepth(iBlockCnt/4);
        break;
    }
    return iResult;
}

QByteArray GetHash(const QByteArray& bBlockData) {
    QByteArray  bResult;
    QCryptographicHash	FileHash(QCryptographicHash::Sha1);

    if( bBlockData.isEmpty() || bBlockData == "" )
        return bResult;

    FileHash.reset();
    FileHash.addData(bBlockData);

    return FileHash.result();
}

inline QByteArray GetBlockHash(QIODevice *pFile, const qint64& iBlockSize, qint64& iPos) {
    QByteArray  bBlock, bResult;
    pFile->seek(iPos);
    bBlock = pFile->read(iBlockSize);
    iPos = iPos + bBlock.size();
    return GetHash(bBlock);
}


inline QByteArray GetConcatHash(const QByteArray& bFirstBlock, const QByteArray& bSecondBlock) {
    if(bSecondBlock.isEmpty())
        return bFirstBlock;
    QByteArray bConcatResult;
    QCryptographicHash	FileHash(QCryptographicHash::Sha1);
    if( bFirstBlock.isEmpty() || bFirstBlock == "" )
        return bConcatResult;
    FileHash.reset();
    FileHash.addData(bFirstBlock + bSecondBlock);
    bConcatResult = FileHash.result();
    return bConcatResult;
}

inline QByteArray GetSugarSHA1(QIODevice *pFile, const qint64& iBlockSize ) {
    QMap<qint64,QByteArray> HashDepthLastMap;
    QMap<qint64,QByteArray> HashDepthNextMap;
    QByteArray bResult;
    qint64 iFileSize = pFile->size();
    qint64 iPos = 0 , iMaxDepth=1, iDepth = 0;
    double dProcess = 0;

    int i=0;
    while(iPos < iFileSize) {
        HashDepthNextMap[1] = GetBlockHash(pFile,iBlockSize,iPos);

        iDepth = GetDepth(iPos,iBlockSize);

        if(IsLastBlock(iPos,iFileSize,iBlockSize))
        {
            if(iDepth == 0)
            {
                HashDepthLastMap[1] = HashDepthNextMap[1];
                HashDepthNextMap[1].clear();
            }
            iDepth = iMaxDepth+1;
        }

        if(iDepth > 0) {
            for(i=1;i<=iDepth;i++) {
                if(HashDepthLastMap[i+1].isEmpty()) {
                    HashDepthLastMap[i+1] = GetConcatHash(HashDepthLastMap[i],HashDepthNextMap[i]);
                    HashDepthLastMap[i].clear();
                    HashDepthNextMap[i].clear();
                } else {
                    HashDepthNextMap[i+1] = GetConcatHash(HashDepthLastMap[i],HashDepthNextMap[i]);
                    HashDepthLastMap[i].clear();
                    HashDepthNextMap[i].clear();
                }
                if(iMaxDepth < iDepth)
                    iMaxDepth = iDepth;
            }
        } else if(iDepth == 0) {
            HashDepthLastMap[1] = HashDepthNextMap[1];
            HashDepthNextMap[1].clear();
        }
        /*
        if( row && num++%16 == 0 ) {
            row->GetVar("pos")->setVar('1',0).addU64(iPos);
            callbackFuncNode(fn);
            qDebug()<<"[filehash] total:"<<toU64(fn->GetVar("@total"))<<" pos:"<<iPos;
        }
        */
        dProcess = ( (double)iPos/(double)iFileSize ) * 100;
    }
    /*
    if( row ) {
        row->GetVar("pos")->setVar('1',0).addU64(iFileSize);
        callbackFuncNode(fn);
        qDebug()<<"[filehash] total:"<<toU64(fn->GetVar("@total"))<<" pos:"<<iFileSize;
    }
    */
    bResult = HashDepthLastMap[iMaxDepth+1];
    return bResult;
}

inline int GetHashResult(const QString& sSource, QString &szRet ) {
    QFile	File(sSource);
    if(File.open(QIODevice::ReadOnly) == false) {
        qDebug() << " File Open Error!!";
        return 0;
    }
    qint64  iBlockSize = 1024*1024*6;
    QString sHashStr = GetSugarSHA1(&File, iBlockSize ).toHex();

    File.close();
    QCryptographicHash	FinalHash(QCryptographicHash::Sha1);

    QString sFinalStr = sHashStr + QString("%1").arg(File.size());
    FinalHash.addData(sFinalStr.toLatin1());
    szRet = FinalHash.result().toHex().toUpper();
    return 1;
}

LPCC GetFileHash(QString szSrc,  TreeNode* node ) {
    if( !QFile::exists(szSrc) )
        return 0;

    qDebug()<<"File Hash Start => path :"<<szSrc;
    QTime time;
    QString szRet;
    time.start();
    //GetSugarHash(szSrc,szRet);
    GetHashResult(szSrc,szRet );
    node->set("fileHash", Q2A(szRet));
    qDebug()<<"File Hash End => Hash :"<< szRet<<" HashTime:"<<time.elapsed()<<"ms";
    return node->GetVar("fileHash")->get();
}
