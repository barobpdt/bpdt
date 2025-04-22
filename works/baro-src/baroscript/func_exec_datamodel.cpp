#include "func_util.h"
#include "tree_model.h"



void regDbFuncs() {
    if( isHashKey("datamodel") )
        return;
    PHashKey hash = getHashKey("datamodel");
    U16 uid = 1;
    hash->add("rootNode", uid);		uid++;	// 1
    hash->add("eq", uid);			uid++;	// 2
    hash->add("fetch", uid);			uid++;	// 3
    hash->add("map", uid);			uid++;	// 4
    hash->add("fetchAll", uid);		uid++;	// 5
    hash->add("update", uid);		uid++;	// 6
    hash->add("error", uid);			uid++;
    hash->add("charset", uid);		uid++;
    hash->add("exec", uid);			uid++;
    hash->add("lastInsertId", uid);	uid++;
    hash->add("bind", uid);			uid++;
    hash->add("fields", uid);		uid++;
    hash->add("drivers", uid);		uid++;
    hash->add("saveAs", uid);		uid++;
    hash->add("clear", uid);	uid++;
    uid=21;
    hash->add("fetchByte", uid);		uid++;
    hash->add("open", uid);			uid++;
    hash->add("count", uid);			uid++;
    uid++;
    hash->add("close", uid);			uid++;
    hash->add("fetchResult", uid);	uid++;
    hash->add("proxyServer", uid);	uid++;
    uid=811;
    hash->add("value", uid);
}

bool callDbFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    // TreeNode* cur=NULL, *sns=NULL;
    if( fc==NULL ) return false;
    U32 ref = fc->xfuncRef;
    if( fnm && ref==0 ) {
        regDbFuncs();
        ref = getHashKeyVal("datamodel",fnm);
        fc->xfuncRef = ref;
    }
    if( ref==0 )
        return false;

    StrVar* sv=NULL;

    switch( ref ) {
    case 1: {		//rootNode

    } break;
    case 2: {		//eq
        if( arrs==NULL )
            return false;

    } break;
    case 3: {		// fetch
        if( arrs==NULL ) return false;
        modelDbExecute(tn,arrs,rst,0x10,NULL,fn);
    } break;
    case 4: {		// map
    } break;
    case 5: {		// fetchAll
        if( arrs==NULL ) return false;
        modelDbExecute(tn,arrs,rst,0x110,NULL,fn);

    } break;
    case 6:	{		// update

    } break;
    case 7:	{		// error
        modelDbExecute(tn,arrs,rst,-1,NULL,fn);
    } break;
    case 8: {		// charset
        modelDbExecute(tn,arrs,rst,-3,NULL,fn);
    } break;
    case 9: {       // exec
        modelDbExecute(tn,arrs,rst,0,NULL,fn );
    } break;
    case 10: {		// lastInsertId
        modelDbExecute(tn,arrs,rst,1,NULL,fn );

    } break;
    case 11: {		// bind
        if( arrs==NULL ) return false;
        sv=arrs->get(0);
        if( SVCHK('d',1) ) {
            SqlDatabase* db=(SqlDatabase*)SVO;
            sv=arrs->get(1);
            if( db->xquery && isNumberVar(sv) ) {
                int idx=toInteger(sv);
                LPCC ty=AS(3);
                sv=arrs->get(2);
                if( ccmp(ty,"utf8") ) {
                    db->xquery->bindValue(idx,QVariant(KR(toString(sv))));
                } else if( ccmp(ty,"number") ) {
                    db->xquery->bindValue(idx,QVariant(toInteger(sv)));
                } else if( ccmp(ty,"long") ) {
                    db->xquery->bindValue(idx,QVariant(toUL64(sv)));
                } else {
                    db->xquery->bindValue(idx,QVariant(toString(sv)));
                }
            }
        }

    } break;
    case 12: {		// fields

    } break;
    case 13: {		// drivers
        QStringList list=QSqlDatabase::drivers();
        rst->reuse();
        for( int n=0;n<list.size();n++) {
            if( n ) rst->add(',');
            rst->add(Q2A(list.at(n)));
        }
    } break;

    case 14: {		// saveAs
        int cnt = arrs?arrs->size(): 0;

    } break;
    case 15: {		// clear
        sv = tn->GetVar("@db");
        if( SVCHK('d',0) ) {
            DbUtil* db=(DbUtil*)SVO;
            db->closeStmt();
        } else if( SVCHK('d',1) ) {
            SqlDatabase* db= (SqlDatabase*)SVO;
            if( db ) {
                db->closeStatement();
            }
        }
    } break;
    case 21: {		// fetchByte
        if( arrs==NULL ) return false;
    } break;
    case 22: {		// open
        if( arrs==NULL ) {
            modelDbExecute(tn,NULL,rst,-8,NULL,fn );
        } else {
            LPCC dbms=tn->get("dbms");
            LPCC dsn=tn->get("dsn");
            if( slen(dsn)==0 ) dsn=tn->get("id");
            sv = tn->GetVar("@db");
            if( ccmp(dbms,"sqlite") ) {
                LPCC path = getFullFileName(AS(0));
                DbUtil* db=sqliteDb(dsn, path, tn);
                if( db->isopen() ) {
                    rst->setVar('3',1);
                }
            } else if( SVCHK('d',1) ) {
                SqlDatabase* db= (SqlDatabase*)SVO;
                if( !db->isOpen() ) {
                    LPCC inode = tn->get("inode");
                    if( db->open(slen(inode)? inode: tn->get("dsn")) ) {
                        rst->setVar('3',1);
                    }
                }
            } else {
                modelDbExecute(tn,NULL,rst,0,NULL,fn );
            }
            qDebug("##### callDbFunc %s open (%d) start ", tn->get("tag"), (int)tn->xstat);
            rst->setVar('n',0,(LPVOID)tn);
        }
    } break;
    case 23: {		// count
        if( arrs==NULL ) return false;
        modelDbExecute(tn,arrs,rst,2,NULL,fn);
        if( isNumberVar(rst) ) {
            int num=toInteger(rst);
            rst->setVar('0',0).addInt(num);
        }
    } break;
    case 811: {		// value
        if( arrs==NULL ) return false;
        rst->reuse();
        modelDbExecute(tn,arrs,rst,0x20,NULL,fn);
    } break;
    case 25: {		// close
        modelDbExecute(tn,NULL,rst,-9,NULL,fn );
    } break;
    case 26: {		// fetchResult
        if( arrs==NULL ) return false;
        sv=arrs->get(2);
        TreeNode* cur = SVCHK('n',0) ? (TreeNode*)SVO: NULL;
        modelDbExecute(tn,arrs,rst,0x140,cur,fn);
    } break;
    case 27: { // proxyServer

    } break;
    default:
        return false;
    }
    return true;
}



bool modelDbExecute(TreeNode* tn, PARR arrs, StrVar* rst, int mode, TreeNode* root, XFuncNode* fn  ) {
    // mode=> 0: exec, 1: lastInsert, 2:fetch
    // value: 0x20, result: 0x40
    // db setNodeFlag ==> 0x10000: utf8, 0x100:
    //		* default : sqlite=ansi, mssql=utf8
    // TreeNode* cf= NULL;

    StrVar* sv=tn->gv("@db");
    bool fieldUse=false;
    rst->reuse();
    if( checkObject(sv,'d')==false ) {
        LPCC dbms=tn->get("dbms");
        LPCC dsn=tn->get("dsn");
        LPCC inode=tn->get("inode");
        if( slen(inode)==0 ) inode=dsn;
        if( slen(dsn)==0 )
            return false;
        if( ccmp(dbms,"sqlite") ) {
            sqliteDb(dsn, NULL, tn);
        } else {
            getModelDatabase(dsn, inode, tn);
        }
        sv=tn->GetVar("@db");
    }
    if( checkObject(sv,'d')==false ) {
        rst->setVar('3',0);
        return false;
    }
    if( arrs==NULL && (mode==0 || mode>1) ) {
        rst->setVar('3',1);
        return false;
    }

    int childDepth=-1;
    switch( sv->getU16(2) ) {
    case 0: {
        DbUtil* db=(DbUtil*)SVO;
        if( mode==1 ) {
            rst->setVar('1',0).addUL64(sqlite3_last_insert_rowid(db->xdb));
            return true;
        }
        if( mode<0 ) {
            if( db==NULL ) return false;
            if( mode==-1 ) {
                rst->add(db->GetVar("@error"));
            } else if( mode==-2 ) {
                XListArr* arr=getListArrTemp();
                for( int n=0,sz=db->columnCount();n<sz;n++) {
                    arr->add()->add(db->field(n));
                }
                rst->setVar('a',0,(LPVOID)arr);
            } else if( mode==-3 ) {
                db->set("@charset", AS(0) );
            } else if( mode==-8 ) {
                if( db->isopen() ) {
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            } else if( mode==-9 ) {
                db->closeStmt();
                rst->setVar('3', db->close()?1:0);
            }
            return true;
        }

        if( db->xdb==NULL ) {
            LPCC dsn=tn->get("dsn");
            LPCC dbFileName=tn->get("dbFileName");
            // db->close();
            if( slen(dbFileName) ) {
                db->open(dbFileName);
            } else {
                if( ccmp(dsn,"pages") ) {
                    dbFileName="data/pages.db";
                } else if( ccmp(dsn,"config") ) {
                    dbFileName="data/config.db";
                } else if( ccmp(dsn,"icons") ) {
                    dbFileName="data/icons.db";
                }
                sqliteDb(dsn, dbFileName);
            }
            if( db->xdb==NULL ) {
                qDebug("#9# sqlite database open fail !!! (fileName: %s)\n", dbFileName );
            }
        }
        if( arrs==NULL ) {
            return false;
        }
        if( db->isNodeFlag(FLAG_RUN) ) {
            printError("error.modelDbExecute01", "sqlite database is run (fie:%s)", tn->get("dbFileName") );
            return false;
        }
        db->setNodeFlag(FLAG_RUN);
        db->GetVar("@error")->reuse();
        sv=arrs->get(1);
        TreeNode* param = SVCHK('n',0) ? (TreeNode*)SVO: mode&0x110 ? getTreeNodeTemp() : tn;
        if( root==NULL )
            root = param;
        if( SVCHK('n',0) ) {
            sv=root->gv("depth");
            if( SVCHK('0',0) ) {
                childDepth=sv->getInt(FUNC_HEADER_POS)+1;
            }
        }
        if( isVarTrue(arrs->get(2)) ) {
            fieldUse=true;
        }
        XListArr* a=getListArrTemp();
        XListArr* z=getListArrTemp();
        LPCC sql= getQueryString(arrs->get(0),a,param,rst,0,0,z);
        db->GetVar("@query")->set(sql);
        if( getCf()->isNodeFlag(CNF_OUTPUT_QUERY) ) {
            LPCC query=parseDbQuery(db->GetVar("@query"),a,param,db->GetVar("@parse"))->get();
            qDebug("#0#QueryParse [%s]=>%s\n", tn->get("dsn"), query );
        }
        if( db->prepare(sql) ) {
            // int psz=params?params->size() :0;
            for(int n=0,sz=a->size();n<sz;n++ ) {
                sv=a->get(n);
                db->bindVar(sv);
            }
            db->bind();

            rst->reuse();
            if( mode==2 ) {
                rst->setVar('0',0).addInt(db->fetchInt(0));
            } else if( mode&0x20 ) {
                rst->set(db->fetchValue(0));
            } else if( mode&(0x10 | 0x40) ) {
                if( mode&0x100 ) {
                    if( arrs && arrs->size()==1 ) {
                        root->reset();
                    }
                    if( mode&0x10 ) {
                        db->fetchTreeNode(root, false, fieldUse);
                    } else {
                        rst->reuse();
                        for( int n=0; db->fetchResult(rst, n==0?0:1) ; n++ );
                    }
                    if( (mode&0x40)==0 ) rst->setVar('n',0,(LPVOID)root);
                } else {
                    if( db->fetchNode(root) )
                        rst->setVar('n',0,(LPVOID)root);
                    else {
                        rst->setVar('3',0);
                    }
                }
            } else {
                if( db->isset("@error") ) {
                    rst->setVar('e',0).add(db->get("@error"));
                    qDebug("query error %s\n%s\n", db->get("@query"), db->get("@error"));
                } else {
                    int num = sqlite3_changes(db->xdb);
                    tn->GetVar("@affectRows")->setVar('0',0).addInt(num);
                }
            }
        } else {
            qDebug("query error %s\n%s\n", db->get("@query"), db->get("@error"));
            rst->reuse();
        }
        db->clearNodeFlag(FLAG_RUN);
    } break;
    case 1: {
        SqlDatabase* db=(SqlDatabase*)SVO;
        if( mode==1 ) {
            rst->reuse()->add( db->lastInsertId() );
            return true;
        }
        if( mode<0 ) {
            if( mode==-1 ) {
                // exam) db.error();
                StrVar* sv=db->gv("@error");
                if( sv && sv->length()>2 ) {
                    rst->add(sv);
                }
            } else if( mode==-3 ) {
                db->set("@charset", AS(0));
            } else if( mode==-8 ) {
                if( db->isOpen() ) {
                    rst->setVar('3',1);
                } else {
                    rst->setVar('3',0);
                }
            } else if( mode==-9 ) {
                db->closeStatement();
                if( db->xdb ) {
                    db->xdb->close();
                }
            }
            return true;
        }
        sv=db->gv("@error");
        if( sv ) sv->reuse();
        if( !db->isOpen() ) {
            LPCC dsn= tn->get("dsn");
            LPCC inode = tn->get("inode");
            if( !db->open(slen(inode)?inode:dsn) ) {
                return false;
            }
        }
        if( arrs==NULL ) {
            return false;
        }
        sv = arrs->get(1);
        TreeNode* param = SVCHK('n',0) ? (TreeNode*)SVO: mode&0x110 ? getTreeNodeTemp() : tn;
        if( root==NULL )
            root = param;
        if( isVarTrue(arrs->get(2)) ) {
            fieldUse=true;
        }

        XListArr* a=getListArrTemp();
        XListArr* z=getListArrTemp();
        LPCC sql=getQueryString(arrs->get(0),a,param,rst,0,0,z);
        db->GetVar("@query")->set(sql);
        if( getCf()->isNodeFlag(CNF_OUTPUT_QUERY) ) {
            LPCC query=parseDbQuery(db->GetVar("@query"),a,param,db->GetVar("@parse"))->get();
            qDebug("#0#QueryParse[%s:%x]=>%s\n", tn->get("dsn"), mode, query );
        }

        if( mode==0 && param==tn ) {
            db->exec(sql);
            int num = db->affectRows();
            if( num<=0 ) num = 0;
            rst->setVar('0',0).addInt(num);
            tn->GetVar("@affectRows")->setVar('0',0).addInt(num);
        } else if( db->prepare(sql) ) {
            for(int n=0,sz=a->size();n<sz;n++ ) {
                sv=a->get(n);
                db->bind(n, sv);
            }
            if( !db->exec() ) {
                qDebug("#9#Query Execute error [%s]=>\n%s", tn->get("dsn"), sql);
                // db->xquery->exec(KR(query));
            }
            rst->reuse();
            if( mode==2 ) {
                // db.count
                QSqlQuery* q=db->xquery;
                if( db->isOpen() && q->next() && q->record().count()>0 ) {
                    rst->setVar('0',0).addInt(q->value(0).toInt());
                } else {
                    qDebug("#0#Query Execute no data [%s]", tn->get("dsn") );
                }
            } else if( mode&0x20 ) {
                // db.value
                StrVar* var=db->fetchValue(0);
                if( var ) rst->set(var,0,var->length() );
                /*
                QSqlQuery* q=db->xquery;
                rst->reuse();
                if( db->isOpen() && q->next() && q->record().count()>0 ) {
                    rst->add(Q2A(q->value(0).toString()));
                } else {
                    qDebug("#0#Query Execute no data(0x20) [%s]", tn->get("dsn") );
                }
                */
            } else if( mode&(0x10|0x40) ) {
                // db.fetchAll or db.fetch
                if( mode&0x100 ) {
                    if( arrs && arrs->size()==1 ) {
                        root->reset();
                    }
                    if( mode&0x10 ) {
                        db->fetchTreeNode(root);
                    } else if( mode&0x80 ) {
                        // fetchByte
                        QSqlQuery* q=db->xquery;
                        while( true ) {
                            if( db->isOpen() && q->next() ) {
                                TreeNode* cur=root->addNode();
                                for( int n=0,sz=q->record().count();n<sz;n++ ) {
                                    LPCC field=db->fieldName(n)->get();
                                    QByteArray ba = q->value(n).toByteArray();
                                    cur->GetVar(field)->set(ba.constData(),ba.size());
                                }
                            } else {
                                break;
                            }
                        }
                    } else if( mode&0x40) {
                        rst->reuse();
                        db->fetchTreeResult(rst);
                    }
                    if( (mode&0x40)==0 ) rst->setVar('n',0,(LPVOID)root);
                    // if( root->childCount() )
                    // else rst->setVar('3',0);
                } else {
                    if( db->fetchNode(root) )
                        rst->setVar('n',0,(LPVOID)root);
                    else {
                        rst->setVar('3',0);
                    }
                }
            }
        } else {
            rst->reuse();
        }
        LPCC err=db->get("@error");
        if( slen(err) ) {
            qDebug("#9#sql query error:[%s] %s\n", err, db->get("@query"));
        } else {
            int num = db->affectRows();
            if( num<=0 ) num = 0;
            tn->GetVar("@affectRows")->setVar('0',0).addInt(num);
        }
    } break;

    default: break;
    }

    if( root && childDepth!=-1 ) {
        TreeNode* cur=NULL;
        for( int n=0, sz=root->childCount(); n<sz; n++ ) {
            cur=root->child(n);
            if( cur ) cur->GetVar("depth")->setVar('0',0).addInt(childDepth);
        }
    }
    releaseLocalArray(arrs);
    return true;
}

StrVar* parseDbQuery(StrVar* sv, XListArr*a, TreeNode* param, StrVar* rst ) {
    XParseVar pv(sv);
    if( rst ) rst->reuse();
    for(int n=0;n<256 && pv.valid(); n++ ) {
        int sp = pv.start;
        pv.findEnd("?");
        if( pv.valid() ) {
            rst->add(pv.get(sp),pv.cur-sp);
        } else {
            rst->add(pv.get(sp),pv.wep-sp);
            break;
        }
        rst->add('\'').add(toString(a->get(n))).add('\'');
    }
    return rst;
}



LPCC getQueryString(StrVar* sv, XListArr* a, TreeNode* b, StrVar* rst, int sp, int ep, XListArr* z ) {
    if( b==NULL || sv==NULL )
        return sv?sv->get():NULL;
    if( sp==0 && ep==0 ) {
        sv = getStrVarPointer(sv,&sp,&ep);
    }
    XParseVar pv(sv,sp,ep);
    if( sv->findPos("${")!=-1  ) {
        StrVar* var = getStrVarTemp();
        makeTextData(pv,NULL,var,0x20000,b);
        pv.SetVar(var);
    }
    while( pv.valid() ) {
        int sp = pv.start;
        pv.findEnd("#");
        if( pv.valid() ) {
            rst->add(pv.get(sp),pv.cur-sp);
        } else {
            rst->add(pv.get(sp),pv.wep-sp);
            break;
        }
        char ch=pv.ch();
        LPCC next=NULL;
        bool fchk=true;
        if( ch=='{' && pv.MatchWordPos("{","}",FIND_SKIP) ) {
            next=pv.v();
            fchk=false;
        } else if( ch=='[' && pv.MatchWordPos("[","]",FIND_SKIP) ) {
            XParseVar sub(pv.GetVar(),pv.prev,pv.cur);
            // ### version 1.0.4
            bool bchk=false;
            for(int n=0; n<32 && sub.valid(); n++ ) {
                sv=NULL;
                if( n==0 ) {
                    next=sub.NextWord();
                    if( ccmp(next,"not") && sub.ch()=='(' && sub.MatchWordPos("(",")",FIND_SKIP) ) {
                        next=sub.Trim(sub.prev, sub.cur);
                        sv=b->gv(next);
                        if( SVCHK('3',0) ) bchk=true;
                        else if( sv==NULL || sv->length()==0 ) bchk=true;
                    } else {
                        sv=b->gv(next);
                        if( sub.ch()==0 ) {
                            LPCC val=toString(sv);
                            if( slen(val) ) {
                                rst->add( val );
                            }
                            break;
                        }
                        if( sv && sv->length() ) {
                            bchk=SVCHK('3',0) ? false : true;
                        }
                    }
                }
                ch=sub.ch();
                if( ch=='?' || ch==':') {
                    sub.incr();
                    ch=sub.ch();
                    if( (ch=='\'' || ch=='"') && sub.MatchStringPos(ch) ) {
                        if( bchk || n>0 )
                            getQueryString(sub.GetVar(),a,b,rst,sub.prev,sub.cur,z);
                    } else {
                        int pos =  n==0 ? sub.find(":"): 0;
                        if( bchk || n>0 ) {
                            if( pos>0 ) {
                                getQueryString(sub.GetVar(),a,b,rst,sub.start,pos,z);
                                sub.start = pos;
                            } else {
                                getQueryString(sub.GetVar(),a,b,rst,sub.start,sub.wep,z);
                            }
                        } else if( pos>0 ) {
                            sub.start = pos;
                        }
                    }
                } else break;
            }
            continue;
        } else {
#ifdef SQLQUERY_FUNC_USE
            next=pv.NextWord();
#else
            rst->add('#');
            continue;
#endif
        }
#ifdef SQLQUERY_FUNC_USE
        ch=pv.ch();
        if( fchk && ch=='(' && pv.MatchWordPos("(",")",FIND_SKIP) ) {
            if( ccmp(next,"ifset") ) {
                XParseVar sub(pv.GetVar(),pv.prev,pv.cur);
                for(int n=0; n<32 && sub.valid(); n++ ) {
                    sv=NULL;
                    if( n==0 ) {
                        next=sub.NextWord();
                        sv=b->gv(next);
                    }
                    ch=sub.ch();
                    if( ch==',') {
                        sub.incr();
                        ch=sub.ch();
                        if( (ch=='\'' || ch=='"') && sub.MatchStringPos(ch) ) {
                            if( (sv && sv->length()) || n>0 )
                                getQueryString(sub.GetVar(),a,b,rst,sub.prev,sub.cur,z);
                        } else if( ch=='[' && sub.MatchWordPos("[","]") ) {
                            if( (sv && sv->length()) || n>0 ) {
                                getQueryString(sub.GetVar(),a,b,rst,sub.prev,sub.cur,z);
                            }
                        } else {
                            if( (sv && sv->length()) || n>0 ) {
                                getQueryString(sub.GetVar(),a,b,rst,sub.start,sub.wep,z);
                            }
                        }
                    } else break;
                }
            } else {
                qDebug()<<"not support function : "<<KR(next);
            }
        } else
#endif
        {
            if( a )
                a->add()->add(b->gv(next));
            if( z )
                z->add()->add(next);
            rst->add("? ");
        }
    }
    return rst->get();
}


bool callModelFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst) {
    if( !SVCHK('m',0) ) return true;
    TreeModel* model=(TreeModel*)SVO;
    if( model==NULL ) return true;
    U32 fref=fc ? fc->xfuncRef: 0;
    if( fref==0 ) {
        if( slen(fnm)==0 ) fnm=fc->getFuncName();
        fref =
            ccmp(fnm,"rootNode") ? 1 :
            ccmp(fnm,"fields") ? 2 :
            0;
        if( fc ) fc->xfuncRef=fref;
    }
    switch( fref ) {
    case 1: { // rootNode
        if( arrs==NULL ) {
            TreeNode* root=model->getRootNode();
            rst->setVar('n',0,(LPVOID)root);
            return true;
        }
        TreeNode* root=NULL;
        TreeNode* cf=model->config();
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            root=(TreeNode*)SVO;
        } else {
            LPCC code=toString(sv);
            if( cf ) {
                root=cf->addNode(code);
                root->set("id",code);
            }
        }
        if( root ) {
            root->clearNodeFlag(NF_CALLBACK);
            model->setRootNode(root);
            TreeNode* thisNode=NULL;
            sv=arrs->get(1);
            if( SVCHK('n',0)) {
                thisNode=(TreeNode*)SVO;
                sv = arrs->get(2);
            }
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode*fnCur=setCallbackFunction(fsrc, fn, cf, NULL, cf->GetVar("onChildData") );
                if(thisNode) {
                    fnCur->GetVar("sendor")->setVar('n',0,(LPVOID)cf);
                    fnCur->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            }
            rst->setVar('n',0,(LPVOID)root);
        }
    } break;
    case 2: { // fields
        if( arrs==NULL ) {
            TreeNode* fields=model->xfields;
            if( fields ) rst->setVar('n',0,(LPVOID)fields);
            return true;
        }
        sv=arrs->get(0);
        model->setFields(sv);
    } break;
    default:
        return false;
    }
    return true;
}
