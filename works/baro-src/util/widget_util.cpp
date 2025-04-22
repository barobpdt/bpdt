#include "widget_util.h"
#include "user_event.h"
#include <QMessageBox>
#include "../fobject/FMutex.h"

FMutex	gMutexPixMap;
FMutex	gMutexStrVar;

StrVar gGlobalVar;
WHash<QPixmap*> gPixmaps;
WHash<HashKey> gHashKeys;
WHash<HashFlag> gHashFlags;
TreeNode gStrVarMap(8);
TreeNode gTreeNodeMap;
TreeNode gconfigMap;

#define CREATE_CONF_INFO "CREATE TABLE conf_info (\
    conf_group text, conf_group_dtl text,\
    conf_code text, conf_code_dtl text,\
    grp text,\
    cd text,\
    data text,\
    note text,\
    lang text,\
    tm int64 DEFAULT '0',\
    regdt datetime DEFAULT (datetime('now','localtime'))\
)"

TreeNode* getCf() {
    return uom.getInfo();
}
StrVar* cfVar(LPCC code, bool reuse, LPCC str) {
    if( slen(code)==0 ) {
        return gGlobalVar.reuse();
    }
    StrVar* var=getCf()->GetVar(code);
    if( var==NULL ) {
        var=gGlobalVar.reuse();
    }
    if( reuse ) var->reuse();
    if( str && var->length()==0 ) var->add(str);
    return var;
}

XListArr* cfArray(LPCC code, bool reuse ) {
    if( slen(code)==0 ) code=FMT("@cfArray_%d", getCf()->incrNum("@cfArrayIdx") );
    return getCf()->addArray(code, reuse);
}
TreeNode* cfNode(LPCC code, bool reuse ) {
    if( slen(code)==0 ) code=FMT("@cfNode_%d", getCf()->incrNum("@cfNodeIdx") );
    return getCf()->addNode(code, reuse);
}

bool isHashKey(LPCC type) {
    return gHashKeys.get(type)? true: false;
}
PHashKey getHashKey(LPCC type) {
    PHashKey map=gHashKeys.get(type);
    if( map==NULL ) {
        HashKey hash;
        map=gHashKeys.add(type,hash );
    }
    return map;
}

U16 getHashKeyVal(LPCC type, LPCC name, U16 key) {
    PHashKey map=getHashKey(type);
    U16 ret=0;
    if( map ) {
        if( key>0 ) {
            map->add(name, key);
            ret=key;
        } else {
            ret=map->value(name);
        }
    }
    return ret;
}


bool isHashFlag(LPCC type) {
    return gHashFlags.get(type)? true: false;
}
PHashFlag getHashFlag(LPCC type) {
    PHashFlag map=gHashFlags.get(type);
    if( map==NULL ) {
        HashFlag hash;
        map=gHashFlags.add(type, hash );
    }
    return map;
}

U32 getHashFlagVal(LPCC type, LPCC name, U32 key) {
    PHashFlag map=getHashFlag(type);
    U32 ret=0;
    if( map ) {
        if( key>0 ) {
            map->add(name, key);
            ret=key;
        } else {
            ret=map->value(name);
        }
    }
    return ret;
}

TreeNode* getTreeNode(LPCC code, LPCC name, bool newNode ) {
    if( code==NULL ) {
        return &gTreeNodeMap;
    }
    if( ccmp(code,"vars") && slen(name)==0 ) {
        return &gStrVarMap;
    }
    if( slen(name)==0 ) {
        name="main";
    }
    LPCC nodeId=FMT("%s.%s", code, name);
    TreeNode* node=gTreeNodeMap.getNode(nodeId);
    if( newNode==false ) {
        if( node==NULL ) return NULL;
    }
    if( node==NULL ) {
        qDebug("#0#get object name %s\n", nodeId);
        node=gTreeNodeMap.setNode(nodeId);
        node->setNodeFlag(FLAG_PERSIST);
        node->xparent=NULL;
    }
    return node;
}


StrVar* getStrVar(LPCC code, LPCC name, char ch, U16 state, LPVOID pobj ) {
    if( code==NULL ) {
        return uom.getStrVar();
    }
	gMutexStrVar.EnterMutex();
    if( slen(name)==0 ) {
        name="main";
    }
	char id[256];
	sprintf(id,"%s.%s", code, name);
	// LPCC id=gBuf.fmt("%s.%s", code, name);
    if( ccmp(code,"fsrc") || ccmp(code,"subfunc") ) {
        getCf()->set("@currentFuncName", name);
    }
    StrVar* var=gStrVarMap.gv(id);
    /*
    PHashStr map=gStrVarMap.get(code);
    if( map==NULL ) map=gStrVarMap.add(code, HashStr() );
    var=map->get(name);
    if( var==NULL ) {
        var=map->add(name, StrVar() );
    }
    */
    if( ch && pobj ) {
        if(var==NULL) {
            if( ccmp(code,"subfunc") ) {
                int pos=IndexOf(name,':');
                if(pos>0) {
                    TreeNode* map=getTreeNode("user", "subfuncMap");
                    LPCC group=gBuf.add(name,pos);
                    StrVar* sv=map->GetVar(group);
                    if(sv->length()) sv->add(',');
                    sv->add(name+pos+1);
                }
            }
            var=gStrVarMap.GetVar(id);
        }
        var->setVar(ch, state, pobj);
    } else if( var==NULL ) {
        var=gGlobalVar.reuse();
    }
	gMutexStrVar.LeaveMutex();
    return var;
}
StrVar* setStrVar(LPCC code, LPCC name, StrVar* sv, bool overwrite ) {
    if( slen(name)==0 ) {
        name="main";
    }
    LPCC id=FMT("%s.%s", code, name);
    StrVar* var=gStrVarMap.GetVar(id);
    /*
    StrVar* var=NULL;
    PHashStr map=gStrVarMap.get(code);
    if( map==NULL ) map=gStrVarMap.add(code, HashStr() );
    var=map->get(name);
    if( var==NULL ) {
        var=map->add(name, StrVar() );
        overwrite=true;
    }
    */
    if( overwrite ) {
        var->reuse()->add(sv);
    }
    return var;
}


DbUtil* sqliteOpen(DbUtil* db, LPCC fileName) {
    if( db->isopen() || slen(fileName)==0 ) {
        return db;
    }
    if( !isFile(fileName) ) {
        fileName=getFullFileName(fileName);
    }
    db->open(fileName);
    if( !db->isopen() ) {
        _LOG("sqliteOpen open fail:%s", fileName);
    }
    return db;
}
DbUtil* sqliteDb(LPCC code, LPCC fileName, TreeNode* node) {
    DbUtil* db=NULL;
    if( node==NULL ) {
        node=getTreeNode("db",code);
    }
    bool newCheck=false;
    StrVar* sv=node->gv("@db");
    if( SVCHK('d',0) ) {
        db=(DbUtil*)SVO;
    } else {
        db=new DbUtil(node);
        newCheck=true;
        if( slen(fileName)==0 ) {
            if( ccmp(code,"config") ) {
                fileName=getFullFileName("data/config.db");
            } else if( ccmp(code,"icons") ) {
                fileName=getFullFileName("data/icons.db");
            } else if( ccmp(code,"funcs") ) {
                fileName=getFullFileName("data/funcs.db");
            }
        }
    }
    if( db && slen(fileName) ) {
        if( db->open(fileName) ) {
            if( ccmp(code,"config") ) {
                if( db->tableCount("conf_info")==0 ) {
                    db->exec(CREATE_CONF_INFO);
                }
            }
        } else {
            qDebug("#9#sqlite open fail (file:%s)", fileName);
        }
    }
    if( node && db ) {
        sv=node->gv("@db");
        if( !SVCHK('d',0) ) {
            if( sv==NULL ) sv=node->GetVar("@db");
            sv->setVar('d',0,(LPVOID)db);
        }
    }
    return db;
}

DbUtil* getIconDb() {
    return sqliteDb("icons");
}
DbUtil* getConfDb() {
    return sqliteDb("config");
}
DbUtil* getFuncsDb() {
    return sqliteDb("funcs");
}
inline StrVar* setConfVarValue(StrVar* value, StrVar* sv) {
    if(sv==NULL ) return NULL;
    int sp=0,ep=0;
    StrVar* var=getStrVarPointer(value,&sp,&ep);
    sv->reuse();
    if(sp<ep) sv->add(var,sp,ep);
    return sv;
}
StrVar* confVar(LPCC confCode, StrVar* value, bool overwrite) {
    StrVar* var=NULL;
    if( confCode==NULL ) {
        var=getStrVarTemp();
        var->setVar('n',0,(LPVOID)&gconfigMap);
        return var;
    }
    if( slen(confCode)==0 ) {
        return var;
    }
    LPCC group=NULL, code=NULL;
    int pos=IndexOf(confCode,'.');
    if( pos==-1 && confCode[0]=='#' ) {
         var=getStrVarTemp();
         group=confCode+1;
         if( ccmp(group,"keys") ) {
             for(PHashKey keys=gHashKeys.First(); keys; keys=gHashKeys.Next() ) {
                 LPCC code=gHashKeys.code();
                 if( slen(code) ) {
                     if( var->length()) var->add(',');
                     var->add(code);
                 }
             }
         } else if( ccmp(group,"flags") ) {
             for(PHashFlag keys=gHashFlags.First(); keys; keys=gHashFlags.Next() ) {
                 LPCC code=gHashFlags.code();
                 if( slen(code) ) {
                     if( var->length()) var->add(',');
                     var->add(code);
                 }
             }
         } else if( ccmp(group,"objectMap") ) {
             for(WBoxNode* bn=gTreeNodeMap.First(); bn; bn=gTreeNodeMap.Next() ) {
                 LPCC code=gTreeNodeMap.getCurCode();
                 if( slen(code) ) {
                     if( var->length()) var->add(',');
                     var->add(code);
                 }
             }
         } else if( ccmp(group,"varMap") ) {
             for(WBoxNode* bn=gStrVarMap.First(); bn; bn=gStrVarMap.Next() ) {
                 LPCC code=gStrVarMap.getCurCode();
                 if( slen(code) ) {
                     if( var->length()) var->add(',');
                     var->add(code);
                 }
             }
         } else if( ccmp(group,"confMap") ) {
             for(WBoxNode* bn=gconfigMap.First(); bn; bn=gconfigMap.Next() ) {
                 LPCC code=gconfigMap.getCurCode();
                 if( slen(code) ) {
                     if( var->length()) var->add(',');
                     var->add(code);
                 }
             }
         }
         return var;
    }

    if( pos>0 ) {
        group=gBuf.add(confCode,pos), code=confCode+pos+1;
    } else {
        group=confCode, code="cf";
        confCode=FMT("%s.%s", group, code);
    }
    /*
    PHashStr hash = gconfigMap.get(group);
    if( hash==NULL ) {
        hash = gconfigMap.add(group, HashStr() );
    }
    StrVar* sv=hash->get(code);
    */

    StrVar* sv=gconfigMap.gv(confCode);
    if(SVCHK('9',0) ) {
        sv=NULL;
    }
    if( sv  && overwrite==false ) {
        return value ? setConfVarValue(value, sv): sv;
    }

    DbUtil* db = getConfDb();
    LPCC lang=uom.getInfo()->get("lang");
    if( slen(lang)==0 ) {
        lang="ko";
        uom.getInfo()->set("lang", lang);
    }

    if( !db->isopen() ) {
        qDebug("#9#config database open error");
        return NULL;
    }
    bool insert=false; //, notuse=false;
    var=gconfigMap.GetVar(confCode);
    if( db->prepare("select data, note from conf_info where grp=? and cd=? and lang=?") &&
        db->bindStr(group[0]=='#' ? group+1:group).bindStr(code).bindStr(lang).exec().fetchNode() )
    {
        StrVar* data=group[0]=='#' ? db->GetVar("note"):db->GetVar("data");
        if( overwrite ) {
            setConfVarValue(value, var);
            if( value && slen(toString(value))==0 ) {
                if( data->length()>0 ) {
                    overwrite=false;
                    // notuse=true;
                }
            }
        } else {
            setConfVarValue(data, var );
        }
    } else {
        overwrite=false;
        setConfVarValue(value, var);
        if( value ) {
            insert=true;
        }
    }
    UL64 tm=(UL64)getTm();
    if( insert ) {
        if( group[0]=='#') {
            if( db->prepare("insert into conf_info(grp, cd, note, lang, tm ) values (?, ?, ?, ?, ?)") ) {
                db->bindStr(group+1).bindStr(code).bindVar(var).bindStr(lang).bindLong(tm).exec();
            }
        } else {
            if( db->prepare("insert into conf_info(grp, cd, data, lang, tm ) values (?, ?, ?, ?, ?)") ) {
                db->bindStr(group).bindStr(code).bindVar(var).bindStr(lang).bindLong(tm).exec();
            }
        }
    } else if( overwrite ) {
        if( group[0]=='#') {
            if( db->prepare("update conf_info set note=?, tm=? where grp=? and cd=? and lang=?") ) {
                db->bindVar(var).bindLong(tm).bindStr(group+1).bindStr(code).bindStr(lang).exec();
            }
        } else {
            if( db->prepare("update conf_info set data=?, tm=? where grp=? and cd=? and lang=?") ) {
                db->bindVar(var).bindLong(tm).bindStr(group).bindStr(code).bindStr(lang).exec();
            }
        }
    }
    int ep=var->length();
    if( ep>0 ) {
        int type=1;
        int n=0;
        char ch=var->charAt(n);
        char firstCh=ch;
        if( ch=='+' || ch=='-' ) n++;
        for( ; n<ep; n++) {
            ch = var->charAt(n);
            if( IS_DIGIT(ch) ) continue;
            if( ch=='.') {
                type=4;
                if( type==4 ) type=0;
            } else {
                type=0;
                break;
            }
        }
        if( type==1 && n>8 ) {
            type=2;
        }
        if( type==1 ) {
            int num=toInteger(var);
            var->setVar('0',0).addInt(num);
        } else if( type==2 ) {
            UL64 num=toUL64(var);
            var->setVar('1',0).addUL64(num);
        } else if( type==4 ) {
            double num=toDouble(var);
            var->setVar('4',0).addDouble(num);
        } else {
            if( firstCh=='t' && ccmp(var->get(),"true") ) {
                var->setVar('3',1);
            } else if( firstCh=='f' && ccmp(var->get(),"false") ) {
                var->setVar('3',0);
            } else if( firstCh=='n' && ccmp(var->get(),"null") ) {
                var->setVar('9',0);
            }
        }
    }
    return var;
}

LPCC conf(LPCC confCode, LPCC def, bool overwrite) {
    StrVar* value=getStrVarTemp();
    StrVar* var=NULL;
    value->add(def);
    var=confVar(confCode, value, overwrite);
    return var ? toString(var): NULL;
}

QPixmap* getIconPixmapType(LPCC type, LPCC id, bool alpha) {
    if( slen(type)==0 || slen(id)==0 ) return NULL;
    LPCC key=FMT("%s:%s",type,id);
    QPixmap* p = gPixmaps.value(key);
    if( p==NULL ) {
        gMutexPixMap.EnterMutex();
        DbUtil* db = getIconDb();
        if( !db->query("select data from icons where type=? and id=?").bindStr(type).bindStr(id).exec().fetchNode() ) {
            return NULL;
        }
        StrVar* data = db->GetVar("data");
        // qDebug("...[getIconPixmapType] %s %d", key, data->length() );
        if( data->length() ) {
            if( alpha ) {
                QImage img;
                if( !img.loadFromData((const uchar*)data->get(),data->length(),"PNG") ) {
                    img.loadFromData((const uchar*)data->get(),data->length(),"JPG");
                }
                QImage alpha = img.alphaChannel();
                QRgb c=img.pixel(1,1);
                int height=img.height(),width=img.width();
                for(int j = 0; j < height; j++) {
                    for(int i = 0; i < width; i++) {
                        if( img.pixel(i,j)==c )
                            alpha.setPixel(i,j,0);
                    }
                }
                img.setAlphaChannel(alpha);
                p = new QPixmap(QPixmap::fromImage(img));
            } else {
                p = new QPixmap();
                if( !p->loadFromData((const uchar*)data->get(),data->length(),"PNG") ) {
                    p->loadFromData((const uchar*)data->get(),data->length(),"JPG");
                }
            }
            if( p ) {
                gPixmaps.add(key,p);
            }
        }
        gMutexPixMap.LeaveMutex();
    }
    // qDebug()<<"getImagePixmap key="<<key<<" w="<<p->width()<<" h="<<p->height();
    return p;
}


int getAlignFlag(LPCC align) {
    return
        slen(align)==0 ? (int)(Qt::AlignLeft|Qt::AlignVCenter) :
        ccmp(align,"center")	? (int)(Qt::AlignCenter | Qt::AlignVCenter) :
        ccmp(align,"wrap")		? (int)(Qt::TextWordWrap) :
        ccmp(align,"wrapLeft")	? (int)(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWrapAnywhere) :
        ccmp(align,"wrapCenter")	? (int)(Qt::AlignCenter | Qt::AlignVCenter | Qt::TextWrapAnywhere) :
        ccmp(align,"left")		? (int)(Qt::AlignLeft | Qt::AlignVCenter) :
        ccmp(align,"right")		? (int)(Qt::AlignRight | Qt::AlignVCenter) :
        ccmp(align,"leftTop")	? (int)(Qt::AlignLeft|Qt::AlignTop) :
        ccmp(align,"rightTop")	? (int)(Qt::AlignRight|Qt::AlignTop) :
        ccmp(align,"centerTop")	? (int)(Qt::AlignCenter|Qt::AlignTop) :
        (int)(Qt::AlignLeft );
}

int alert(LPCC message, LPCC title)
{
    QMessageBox msgBox;
    if( slen(title)==0 ) title="alert";
    msgBox.setWindowTitle(KR(title));
    msgBox.setText(KR(message));
    return msgBox.exec();
}

int confirm(LPCC message, LPCC info, LPCC title)
{
    QMessageBox msgBox;
    if( slen(title)==0 ) title="confirm";
    msgBox.setWindowTitle(KR(title));
    msgBox.setText(KR(message));
    if( slen(info) ) msgBox.setInformativeText(KR(info));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    return msgBox.exec();
}

WidgetConf* getWidgetConf(TreeNode *node) {
    if( node && node->cmp("@objectType", "WidgetConf") ) {
        return static_cast<WidgetConf*>(node);
    }
    return NULL;
}

WidgetConf* parentWidgetConf(WidgetConf *wcf) {
    return getWidgetConf(wcf? wcf->parent(): NULL);
}
