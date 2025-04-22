#pragma comment (lib, "zlib.lib")
#include "../ZIP/zip.h"
#include "../ZIP/unzip.h"

#include "func_util.h"

inline void regZipFuncs() {
    if( isHashKey("zip") )
        return;
    PHashKey hash = getHashKey("zip");
    U16 uid = 1;
    hash->add("addFolder", uid);		uid++;	//
    hash->add("addFile", uid);		uid++;	//
    hash->add("password", uid);		uid++;	//
    hash->add("zip", uid);			uid++;	//
    hash->add("close", uid);			uid++;	//
    uid = 21;
    hash->add("open", uid);			uid++;	//
    hash->add("unzipList", uid);		uid++;	//
    hash->add("fileList", uid);		uid++;	//
    hash->add("unzip", uid);			uid++;	//
    hash->add("unzipFile", uid);		uid++;	//
    hash->add("unzipPassword", uid);	uid++;
}

inline Zip* getZipPointer(StrVar* sv) {
    Zip* zip=NULL;
    if( SVCHK('z',11) ) {
        zip = (Zip*)SVO;
    } else {
        zip = new Zip();
        sv->setVar('z',11,(LPVOID)zip);
    }
    return zip;
}
inline UnZip* getUnZipPointer(StrVar* sv) {
    UnZip* zip=NULL;
    if( SVCHK('z',12) ) {
        zip = (UnZip*)SVO;
    } else {
        zip = new UnZip();
        sv->setVar('z',12,(LPVOID)zip);
    }
    return zip;
}
bool callZipFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=NULL; // tn->gv("@c");
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        regZipFuncs();
        ref = getHashKeyVal("zip", fnm? fnm : fc->getFuncName() );
        if( fc ) fc->xfuncRef=ref;
    }
    /*
    z = instance('test.zip');
    z.zip('C:/APP/p1/data/b.zip', true);
    z.addFile('C:/APP/p1/data/info/취약점 파싱.h','info');
    z.close();
    z.open('C:/APP/p1/data/b.zip');
    while( c, z.unzipList() ) print("c->$c");
    */

    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {		// addFolder
        if( arrs==NULL ) return true;
        int cnt = arrs->size();
        Zip* zip = getZipPointer( tn->GetVar("@z") );
        LPCC path = AS(0);
        QString pathStr=KR(path);
        if( cnt==1 ) {
            zip->addDirectory(pathStr);
        } else {
            LPCC str=AS(1), type=NULL, base=NULL;
            U32 flag=0;
            if( isnum(str) ) {
                flag=atoi(str);
            } else if( ccmp(str,"[Absolute]") || ccmp(str,"[SkipRoot]") || ccmp(str,"[OnlyFile]") ) {
                type=str;
            } else {
                base=str;
            }
            if( cnt>2 ) {
                str=AS(2);
                if( isnum(str) ) {
                    flag=atoi(str);
                } else if( type==NULL ) {
                    type=str;
                    if( isNumberVar(arrs->get(3)) ) flag=(U32)toInteger(arrs->get(3));
                }
            }
            Zip::CompressionLevel level=Zip::AutoFull;
            if( flag ) {
                level=(Zip::CompressionLevel)flag;
            }
            if( type ) {
                if( ccmp(type,"[Absolute]") ) {
                    zip->addDirectory(pathStr, base? KR(base): pathStr, Zip::AbsolutePaths | Zip::IgnoreRoot | Zip::SkipBadFiles, level );
                } else if( ccmp(type,"[OnlyFile]") ) {
                    zip->addDirectory(pathStr, base? KR(base): pathStr ,Zip::IgnoreRoot | Zip::IgnorePaths | Zip::SkipBadFiles, level );
                } else if( ccmp(type,"[SkipRoot]") ) {
                    zip->addDirectory(pathStr, base? KR(base): pathStr, Zip::IgnoreRoot | Zip::SkipBadFiles, level );
                } else {
                    zip->addDirectory(pathStr, base? KR(base): pathStr, Zip::IgnoreRoot | Zip::IgnorePaths | Zip::SkipBadFiles, level);
                }
            } else {
                if( base ) {
                    zip->addDirectory(pathStr, KR(base), level);
                } else {
                    zip->addDirectory(pathStr,level);
                }
            }
        }

        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 2: {		// addFile
        if( arrs==NULL ) return true;
        int cnt = arrs->size();
        Zip* zip = getZipPointer( tn->GetVar("@z") );
        LPCC path=AS(0);
        QString pathStr=KR(path);
        if( cnt==1 ) {
            zip->addFile(pathStr);
        } else {
            LPCC str=AS(1), type=NULL, base=NULL;
            U32 flag=0;
            if( isnum(str) ) {
                flag=atoi(str);
            } else if( ccmp(str,"[Absolute]") || ccmp(str,"[SkipRoot]") || ccmp(str,"[OnlyFile]") ) {
                type=str;
            } else {
                base=str;
            }

            if( cnt>2 ) {
                str=AS(2);
                if( isnum(str) ) {
                    flag=atoi(str);
                } else if( type==NULL ) {
                    type=str;
                    if( isNumberVar(arrs->get(3)) ) flag=(U32)toInteger(arrs->get(3));
                }
            }
            Zip::CompressionLevel level=Zip::AutoFull;
            if( flag ) {
                level=(Zip::CompressionLevel)flag;
            }
            if( type ) {
                if( ccmp(type,"[Absolute]") ) {
                    zip->addFile(pathStr, base? KR(base): pathStr, Zip::AbsolutePaths | Zip::SkipBadFiles, level );
                } else if( ccmp(type,"[OnlyFile]") ) {
                    zip->addFile(pathStr, base? KR(base): pathStr ,Zip::IgnoreRoot | Zip::IgnorePaths | Zip::SkipBadFiles, level );
                } else if( ccmp(type,"[SkipRoot]") ) {
                    zip->addFile(pathStr, base? KR(base): pathStr, Zip::IgnoreRoot | Zip::SkipBadFiles, level );
                } else {
                    zip->addFile(pathStr, base? KR(base): pathStr, Zip::IgnoreRoot | Zip::IgnorePaths | Zip::SkipBadFiles, level);
                }
            } else {
                if( base ) {
                    zip->addFile(pathStr, KR(base), level);
                } else {
                    zip->addFile(pathStr,level);
                }
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 3: {		// password
        Zip* zip = getZipPointer( tn->GetVar("@z") );
        if( arrs==NULL ) {
            rst->set(Q2A(zip->password()));
            return true;
        }
        LPCC password = AS(0);
        zip->setPassword(KR(password));
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 4: {		// zip
        // param : path, overwrite
        if( arrs==NULL ) {
            StrVar* sv=tn->gv("@pathZip");
            rst->setVar('3', sv && sv->length() ? 1: 0 );
            return true;
        }
        Zip* zip = getZipPointer( tn->GetVar("@z") );
        StrVar* sv = tn->GetVar("@pathZip");
        LPCC prev = toString(sv);
        if( prev ) {
            zip->closeArchive();
        }
        LPCC path = AS(0);
        sv->set(path);
        if( slen(path) ) {
            zip->createArchive(KR(path),isVarTrue(arrs->get(1)) );
        }
    } break;
    case 5: {		// close
        Zip* zip = getZipPointer( tn->GetVar("@z") );
        if( zip ) {
            tn->GetVar("@pathZip")->reuse();
            zip->closeArchive();
        } else {
            UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
            if( unzip ) {
                tn->GetVar("@pathUnZip")->reuse();
                unzip->closeArchive();
            }
        }
    } break;
    case 21: {		// open
        if( arrs==NULL ) {
            StrVar* sv=tn->gv("@pathUnZip");
            rst->setVar('3', sv && sv->length() ? 1: 0 );
            return true;
        }
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        LPCC path = AS(0);
        tn->GetVar("@pathUnZip")->set(path);
        if( slen(path) ) {
            unzip->openArchive(KR(path));
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 22: {		// unzipList
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        QList<UnZip::ZipEntry> list= unzip->entryList();
        sv = arrs? arrs->get(0):NULL;
        TreeNode* info=NULL;
        if( arrs ) {
            sv = arrs->get(0);
            if( SVCHK('n',0) ) {
                info = (TreeNode*)SVO;
            }
        }
        if( info==NULL ) {
            info = getTreeNodeTemp();
        }
        for(int n=0,sz=list.size();n<sz;n++ ) {
            TreeNode* nd=info->addNode();
            nd->GetVar("type")->set(list.at(n).type==UnZip::File?"file":"folder");
            nd->GetVar("fileName")->set(Q2A(list.at(n).filename));
            nd->GetVar("modifyDate")->setVar('1',0).addUL64(list.at(n).lastModified.toTime_t());
            nd->GetVar("compressedSize")->setVar('1',0).addUL64(list.at(n).compressedSize);
            nd->GetVar("size")->setVar('1',0).addUL64(list.at(n).uncompressedSize);
        }
        rst->setVar('n',0,(LPVOID)info);
    } break;
    case 23: {		// fileList
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        QStringList list = unzip->fileList();
        sv = arrs? arrs->get(0):NULL;
        XListArr* a=NULL;
        if( arrs ) {
            sv = arrs->get(0);
            if( SVCHK('a',0) ) {
                a= (XListArr*)SVO;
            }
        }
        if( a==NULL ) {
            a = getListArrTemp();
        }
        for(int n=0,sz=list.size();n<sz;n++ ) {
            a->add()->set(Q2A(list.at(n)));
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 24: {		// unzip
        if( arrs==NULL ) return true;
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        LPCC path = AS(0);
        LPCC ty=AS(1);
        unzip->extractAll(KR(path),ccmp(ty,"[OnlyFile]")?UnZip::SkipPaths : UnZip::ExtractPaths);
    } break;
    case 25: {		// unzipFile
        if( arrs==NULL ) return true;
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        sv = arrs->get(0);
        LPCC path = AS(1);
        LPCC ty = AS(2);
        if( SVCHK('a',0) ) {
            XListArr* a = (XListArr*)SVO;
            QStringList list;
            for( int n=0,sz=a->size();n<sz;n++ ) {
                LPCC file = a->get(n)->get();
                list.append(KR(file));
            }
            if( slen(path) ) {
                unzip->extractFiles(list,KR(path),ccmp(ty,"[OnlyFile]")?UnZip::SkipPaths : UnZip::ExtractPaths);
            }
        } else if( slen(path) ) {
            LPCC file = AS(0);
            unzip->extractFile(KR(file),KR(path),ccmp(ty,"[OnlyFile]")?UnZip::SkipPaths : UnZip::ExtractPaths);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 26: {		// unzipPassword
        if( arrs==NULL ) return true;
        UnZip* unzip = getUnZipPointer( tn->GetVar("@z") );
        LPCC pass = AS(0);
        unzip->setPassword(KR(pass));
    } break;
    default: return false;
    }
    return true;
}
