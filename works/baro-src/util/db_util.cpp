#include "db_util.h"


/************************************************************************/
/* DbUtil                                                               */
/************************************************************************/


DbUtil::DbUtil(TreeNode* cf) : xcf(cf), xstmt(NULL), xflag(0) {
    xdb=NULL;
    if( xcf==NULL ) {
        xcf=new TreeNode(2,true);
    }
}
DbUtil::~DbUtil() {
    if( xcf->isNodeFlag(FLAG_NEW) ) {
        SAFE_DELETE(xcf);
    }
}

bool DbUtil::err(int stat, LPCC msg) {
    LPCC rtn = NULL;
    switch (stat) {
        case SQLITE_OK          : rtn = "SQLITE_OK";			break;
        case SQLITE_ERROR       : rtn = "SQLITE_ERROR";			break;
        case SQLITE_INTERNAL    : rtn = "SQLITE_INTERNAL";		break;
        case SQLITE_PERM        : rtn = "SQLITE_PERM";			break;
        case SQLITE_ABORT       : rtn = "SQLITE_ABORT";			break;
        case SQLITE_BUSY        : rtn = "SQLITE_BUSY";			break;
        case SQLITE_LOCKED      : rtn = "SQLITE_LOCKED";		break;
        case SQLITE_NOMEM       : rtn = "SQLITE_NOMEM";			break;
        case SQLITE_READONLY    : rtn = "SQLITE_READONLY";		break;
        case SQLITE_INTERRUPT   : rtn = "SQLITE_INTERRUPT";		break;
        case SQLITE_IOERR       : rtn = "SQLITE_IOERR";			break;
        case SQLITE_CORRUPT     : rtn = "SQLITE_CORRUPT";		break;
        case SQLITE_NOTFOUND    : rtn = "SQLITE_NOTFOUND";		break;
        case SQLITE_FULL        : rtn = "SQLITE_FULL";			break;
        case SQLITE_CANTOPEN    : rtn = "SQLITE_CANTOPEN";		break;
        case SQLITE_PROTOCOL    : rtn = "SQLITE_PROTOCOL";		break;
        case SQLITE_EMPTY       : rtn = "SQLITE_EMPTY";			break;
        case SQLITE_SCHEMA      : rtn = "SQLITE_SCHEMA";		break;
        case SQLITE_TOOBIG      : rtn = "SQLITE_TOOBIG";		break;
        case SQLITE_CONSTRAINT  : rtn = "SQLITE_CONSTRAINT";	break;
        case SQLITE_MISMATCH    : rtn = "SQLITE_MISMATCH";		break;
        case SQLITE_MISUSE      : rtn = "SQLITE_MISUSE";		break;
        case SQLITE_NOLFS       : rtn = "SQLITE_NOLFS";			break;
        case SQLITE_AUTH        : rtn = "SQLITE_AUTH";			break;
        case SQLITE_FORMAT      : rtn = "SQLITE_FORMAT";		break;
        case SQLITE_RANGE       : rtn = "SQLITE_RANGE";			break;
        case SQLITE_ROW         : rtn = "SQLITE_ROW";			break;
        case SQLITE_DONE        : rtn = "SQLITE_DONE";			break;
        default: rtn = "UNKNOWN_ERROR";
    }
    xbinds.reuse();
    if( msg ) {
        qDebug("[sqlite error] %s => %s\n",rtn,msg);
        GetVar("@error")->set("SQLITE error : ").add(msg).add(" RESULT CODE: ").add(rtn);
    } else  {
        qDebug("[sqlite error] %s",rtn);
        GetVar("@error")->set("SQLITE error : ").add(rtn);
    }
    return false;
}

bool DbUtil::closeStmt() {
    clearNodeFlag(FLAG_FETCHED);
    clearNodeFlag(FLAG_BINDED);
    GetVar("@error")->reuse();
    xbinds.reuse();
    if( xstmt ) {
        int ret = sqlite3_finalize(xstmt);
        if( ret==SQLITE_OK ) {
            xstmt = NULL;
            return true;
        } else {
            xstmt = NULL;
            err(ret,"closeStmt");
        }
    }
    return false;
}

bool DbUtil::close() {
    closeStmt();
    if( xdb ) {
        int ret = sqlite3_close(xdb);
        if( ret==SQLITE_OK ) {
            xdb  = NULL;
            return true;
        } else {
            err(ret,"close");
        }
    }
    return false;
}

bool DbUtil::open(LPCC filenm) {
    close();
    // int ret = sqlite3_open_v2(filenm, &xdb,SQLITE_OPEN_NOMUTEX |SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE,NULL);
    int ret = sqlite3_open( filenm, &xdb );
    if( ret==SQLITE_OK ) {
        // exec("PRAGMA count_changes = 1");
        exec("PRAGMA journal_mode = MEMORY");
        exec("PRAGMA synchronous = OFF");
        // exec("PRAGMA read_uncommitted=TRUE");
        return true;
    } else {
        xdb  = NULL;
        err(ret, FMT("sqlite open error (file:%s)", filenm) );
    }
    return false;
}

void DbUtil::begin() { closeStmt(); exec("BEGIN"); }

void DbUtil::commit() { exec("COMMIT"); }


DbUtil& DbUtil::query(LPCC sql ) {
    prepare(sql);
    return *this;
}

DbUtil& DbUtil::select(LPCC fmt,... ) {
    StrVar* q = GetVar("@query")->reuse();
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsprintf( buf, fmt, ap);
    va_end(ap);
    prepare(buf);
    return *this;
}


LPCC DbUtil::fetchValue(int idx) {
    if( xstmt==NULL )
        return NULL;
    setNodeFlag(FLAG_FETCHED);
    xvar.reuse();
    int ret = sqlite3_step(xstmt);
    if( ret==SQLITE_ROW ) {
        int ccnt = sqlite3_column_count(xstmt);
        if( ccnt>0 && idx<ccnt ) {
            xvar.add((LPCC)sqlite3_column_text(xstmt,idx));
        }
    }  else err(ret,"fetch value");
    return xvar.get();
}

int DbUtil::fetchInt(int idx) {
    if( xstmt==NULL )
        return 0;
    setNodeFlag(FLAG_FETCHED);
    if( !isNodeFlag(FLAG_BINDED) ) {
        exec();
    }
    int ret = sqlite3_step(xstmt);
    if( ret==SQLITE_ROW ) {
        int ccnt = sqlite3_column_count(xstmt);
        if( ccnt>0 && idx<ccnt ) {
            return sqlite3_column_int(xstmt,idx);
        }
    }  else err(ret,"fetch value");
    return 0;
}

UL64 DbUtil::fetchUL64(int idx) {
    if( xstmt==NULL )
        return 0;
    if( !isNodeFlag(FLAG_BINDED) ) {
        exec();
    }
    setNodeFlag(FLAG_FETCHED);
    int ret = sqlite3_step(xstmt);
    if( ret==SQLITE_ROW ) {
        int ccnt = sqlite3_column_count(xstmt);
        if( ccnt>0 && idx<ccnt ) {
            return sqlite3_column_int64(xstmt,idx);
        }
    }  else err(ret,"fetch value");
    return 0;
}

int DbUtil::tableCount(LPCC tbl) {
    LPCC sql = tbl?  xvar.fmt(
        "select count(1) cnt from sqlite_master where type='table' and name='%s'",tbl):
        "select count(1) cnt from sqlite_master where type='table'";
    return isopen() ? select(sql).fetchInt(): 0;
}


LPCC DbUtil::field(int idx) {
    setNodeFlag(FLAG_FETCHED);
    return xstmt && idx<columnCount() ? sqlite3_column_name(xstmt,idx): "";
}

DbUtil& DbUtil::bindStr(LPCC value, int size) {
    StrVar* next = xbinds.add(StrVar());
    next->set(value, size);
    return *this;
}
DbUtil& DbUtil::bindVar(StrVar* sv) {
    StrVar* next = xbinds.add(StrVar());
    if( sv ) {
        if( SVCHK('i',7) ) {
            next->set("\f#o").add((LPCC)sv->get(FUNC_HEADER_POS+4),sv->getInt(FUNC_HEADER_POS) );
        } else if( SVCHK('9',0) ) {
            next->set("\f#x");
        } else {
            next->add(toString(sv));
        }
    }
    return *this;
}

DbUtil& DbUtil::bindBuffer(LPVOID value, int size) {
    StrVar* next = xbinds.add(StrVar());
    next->set("\f#o").add((LPCC)value, size);
    return *this;
}
DbUtil& DbUtil::bindLong( UL64 dot) {
    StrVar* next = xbinds.add(StrVar());
    next->set("\f#d").add((LPCC)&dot, sizeof(UL64));
    return *this;
}
DbUtil& DbUtil::bindInt( int num) {
    StrVar* next = xbinds.add(StrVar());
    next->set("\f#n").add((LPCC)&num, sizeof(int));
    return *this;
}

LPCC DbUtil::getFiledValue(int idx, StrVar* rst, int* size) {
    if( rst==NULL ) return NULL;
    if( size ) *size = 0;
    if( xdb && xstmt && idx<sqlite3_column_count(xstmt) ) {
        if( SQLITE_BLOB==sqlite3_column_type(xstmt,idx) ) {
            if( size ) *size = sqlite3_column_bytes(xstmt,idx);
            rst->set( (LPCC)sqlite3_column_blob(xstmt,idx), *size);
        } else if( SQLITE_INTEGER==sqlite3_column_type(xstmt,idx) ) {
            UL64 val = 0;
            sqlite3_bind_int64(xstmt,idx,val);
            rst->setVar('1',0).addUL64(val);
        } else {
            LPCC text=(LPCC)sqlite3_column_text(xstmt,idx);
            if( size ) *size=slen(text);
            rst->set(text);
        }
    }
    return rst->get();
}


bool DbUtil::prepare(LPCC sql) {
    if( slen(sql)==0 ) return false;
    LPCC e=NULL;
    closeStmt();
    int ret = sqlite3_prepare(xdb,sql,-1,&xstmt,&e);
    if( ret!=SQLITE_OK ) {
        err(ret, sqlite3_errmsg(xdb) );
        _LOG("DbUtil::prepare query:%s\nerror:%s", sql, e?e:"prepare error");
    }
    return ( ret==SQLITE_OK ) ? true : false;
}

bool DbUtil::bind(LPCC sql) {
    if( xstmt==NULL ) {
        return false;
    }
    int bcnt = 0;
    int bsize = xbinds.size();
    if( bsize>0 ) {
        if( slen(sql) ) {
            if( !prepare(sql) ) {
                xbinds.reuse();
                return false;
            }
        }
        int ret = 0;
        for( int n=0; n<bsize; n++ ) {
            StrVar* var = xbinds.get(n);
            if( var==NULL )
                break;
            LPCC buf = var->get();
            int len = var->length();
            if( buf==NULL ) {
                ret = sqlite3_bind_text(xstmt, n+1, "", 0, SQLITE_STATIC);
                // qDebug()<< "[BIND]"<<n <<"======== NULL";
                continue;
            }
            if( buf[0]=='\f' && buf[1]=='#' ) {
                char c = buf[2];
                buf+=3; len-=3;
                switch(c) {
                    case 'd': {
                        if( len==sizeof(UL64) ) {
                            UL64 dot = 0;
                            memcpy(&dot,buf,len);
                            // qDebug()<< "[BIND]"<<n <<"========"<<dot;
                            ret = sqlite3_bind_int64(xstmt, n+1, dot);
                        } else
                            ret = sqlite3_bind_text(xstmt, n+1, buf, len, SQLITE_STATIC);
                    } break;
                    case 'n': {
                        int num = 0;
                        if( len==sizeof(int) ) {
                            memcpy(&num,buf,len);
                            ret = sqlite3_bind_int(xstmt, n+1, num);
                        } else
                            ret = sqlite3_bind_text(xstmt, n+1, buf, len, SQLITE_STATIC);
                        // qDebug()<< "[BIND]"<<n <<"========"<<num;
                    } break;
                    case 'o':
                        qDebug()<< "[BIND]"<<n <<" BLOB size ="<<len;
                        ret = sqlite3_bind_blob(xstmt, n+1, buf, len, SQLITE_STATIC); break;
                    case 'x':
                        qDebug()<< "[BIND]"<<n <<" null\n";
                        ret = sqlite3_bind_null(xstmt, n+1); break;
                    default:
                        ret = sqlite3_bind_text(xstmt, n+1, buf, len, SQLITE_STATIC); break;
                }
            } else {
                ret = sqlite3_bind_text(xstmt, n+1, buf, len, SQLITE_STATIC);
                // qDebug()<< "[BIND]"<<n <<"========"<<KR(buf);
            }
            if( var && ret==SQLITE_OK )
                bcnt++;
            else
                return err(ret,xvar.fmt("bind err:%s",var?var->get():"") );
        }
    }
    xbinds.reuse();
    if( sql )
        return true;
    int stat=0, busyCnt = 0;
    while ( true ) {
        stat = sqlite3_step( xstmt );
        if ( stat == SQLITE_BUSY ) {
            if ( busyCnt++ > 20 ) {
                qDebug() << "Busy-counter has reached maximum. Aborting this sql statement!\n";
                break;
            }
            Sleep( 500 ); // Sleep 100 msec
            qDebug() << "sqlite3_step: BUSY counter: " << busyCnt << endl;
        } else
            break;
    }
    bool rtn = ( stat==SQLITE_DONE || stat==SQLITE_ROW ) ? true : false;
    if( !rtn ) {
        err(stat,"sqlite prepare execute error : ");
    }
    setNodeFlag(FLAG_BINDED);
    sqlite3_reset(xstmt);
    return rtn;
}


DbUtil& DbUtil::exec() {
    bind();
    return *this;
}

bool DbUtil::exec(LPCC sql) {
    if( xdb==NULL ) return false;
    char* e=NULL;
    clearNodeFlag(FLAG_BINDED);
    int ret = sqlite3_exec(xdb,sql,NULL,NULL,&e);
    if( ret!=SQLITE_OK )
        err(ret,e);
    return ( ret==SQLITE_OK ) ? true :false;
}
bool DbUtil::fetch() {
    setNodeFlag(FLAG_FETCHED);
    bool ret = ( xdb && xstmt && sqlite3_step(xstmt)==SQLITE_ROW ) ? true : false;
    if( ret ) {
        putData(this);
    }
    return ret;
}
bool DbUtil::fetchNode(TreeNode* node) {
    setNodeFlag(FLAG_FETCHED);
    bool ret = ( xdb && xstmt && sqlite3_step(xstmt)==SQLITE_ROW ) ? true : false;
    if( ret ) {
        putData(node);
    }
    return ret;
}
bool DbUtil::fetchResult(StrVar* rst, U16 flag) {
    setNodeFlag(FLAG_FETCHED);
    bool ret = ( xdb && xstmt && sqlite3_step(xstmt)==SQLITE_ROW) ? true : false;
    if( ret ) {
        if( flag&1 ) rst->add('\b');
        for(int n=0,cnt=columnCount();n<cnt;n++) {
            // LPCC col = field(n);
            if( n ) rst->add('\f');
            switch(sqlite3_column_type(xstmt,n)) {
            case SQLITE_BLOB : {
                int size = sqlite3_column_bytes(xstmt,n);
                rst->add( (LPCC)sqlite3_column_blob(xstmt,n), size);
            } break;
            case SQLITE_NULL: {
                rst->add("");
            } break;
            case SQLITE_INTEGER: {
                UL64 val = sqlite3_column_int64(xstmt,n);
                rst->format(16,"%llu",val);
            } break;
            default: {
                rst->add((LPCC)sqlite3_column_text(xstmt,n));
            } break;
            }
        }
    }
    return ret;
}

TreeNode* DbUtil::fetchTreeNode(TreeNode* root, bool reuse, bool fieldUse) {
    if( root==NULL ) {
        root=xcf;
    }
    setNodeFlag(FLAG_FETCHED);
    TreeNode* cur = NULL;
    if( reuse ) {
        root->removeAll(true);
    }
    if( xdb && xstmt ) {
        if( fieldUse ) {
            XListArr* a=root->addArray("@fields", true);
            for(int n=0,cnt=columnCount();n<cnt;n++) {
                a->add()->add(field(n));
            }
        }
        while( sqlite3_step(xstmt)==SQLITE_ROW ) {
            cur = root->addNode();
            putData(cur);
        }
    }
    return root;
}

bool DbUtil::putData(WBox* node) {
    if( xstmt==NULL ) return false;
    int size = 0;
    if( node==NULL )
        node = this;
    for(int n=0,cnt=columnCount();n<cnt;n++) {
        LPCC col = field(n);
        switch(sqlite3_column_type(xstmt,n)) {
        case SQLITE_BLOB : {
            size = sqlite3_column_bytes(xstmt,n);
            // qDebug("DbUtil::putData field:%s (%d)", col, size);
            node->GetVar(col)->set( (LPCC)sqlite3_column_blob(xstmt,n), size);
        } break;
        case SQLITE_NULL: {
            node->GetVar(col)->reuse();
        } break;
        case SQLITE_INTEGER: {
            UL64 val = sqlite3_column_int64(xstmt,n);
            node->GetVar(col)->setVar('1',0).addUL64(val);
        } break;
        default: {
            node->GetVar(col)->set((LPCC)sqlite3_column_text(xstmt,n));
        } break;
        }
    }
    return true;
}

sqlite3_stmt* DbUtil::stmt(LPCC sql) {
    closeStmt();
    GetVar("@error")->reuse();
    LPCC e=NULL;
    int ret = sqlite3_prepare(xdb,sql,-1,&xstmt,&e);
    if( ret!=SQLITE_OK )
        err(ret,e);
    return ( ret==SQLITE_OK ) ? xstmt : NULL;
}

int DbUtil::columnCount() { return xstmt ? sqlite3_column_count(xstmt): 0; }
