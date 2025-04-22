#include "func_util.h"
#include "tree_model.h"
#include "../util/user_event.h"
#include <QApplication>
#include <QDesktopWidget>

bool setBaroDefine(StrVar* var, int sp, int ep, TreeNode* node) {
    DbUtil* db=getConfDb();
    XParseVar pv(var, sp, ep);
    char ch=pv.ch();
    qDebug("#0#define start============");

    if( ch==0 || db==NULL ) return false;
    /*
    if( !isHashFlag("def") ) {
        db->select("select type,def,value from global_define");
        while(db->fetchNode()) {
            getHashFlagVal(db->get("type"), db->get("def"), (U32)toUL64(db->GetVar("value")) );
        }
    }
    */
    StrVar* rst=getStrVarTemp();
    LPCC name=NULL, group=NULL, base=node&&node->isset("group") ? node->get("group"): "BARO";
    int pos=0, num=0;

    while( pv.valid() ) {
        ch=pv.ch();
        if( ch==0 ) break;
        if( ch=='#' ) {
            pv.findEnd("\n");
            continue;
        }
        name=pv.findEnd("=").v();
        if( !pv.valid() ) break;
        pos=IndexOf(name,'.');
        if( pos>0 ) {
            group=gBuf.add(name,pos), name+=pos+1;
        } else {
            group=base;
        }
        rst->set(pv.findEnd("\n").v());
        qDebug("#0#define %s=%s\n", group, name);
        getHashFlagVal(group, name, (U32)toUL64(rst) );
        if( node && node->cmp("mode","update") ) {
            if( db->prepare("select count(1) from global_define where type=? and def=?") ) {
                int num=db->bindStr(group).bindStr(name).fetchInt();
                if( num==0 ) {
                    if( db->prepare("insert into global_define(type, def,value) values(?,?,?)")  ) {
                        db->bindStr(group).bindStr(name).bindVar(rst).exec();
                    }
                } else {
                    if( db->prepare("update global_define set value=? where type=? and name=?")  ) {
                        db->bindVar(rst).bindStr(group).bindStr(name).exec();
                    }
                }
            }
        }
        num++;
    }
    return num>0? true: false;
}
void regClassFuncs() {
    if( isHashKey("class") )
        return;
    PHashKey hash = getHashKey("class");
    U16 uid = 1;
    hash->add("rect", uid);			uid++;	// 1
    hash->add("point", uid);		uid++;	// 2
    hash->add("mimeData", uid);		uid++;	// 3
    hash->add("drawObject", uid);	uid++;	// 4
    hash->add("color", uid);		uid++;	// 5
    hash->add("size", uid);			uid++;	// 6
    hash->add("pair", uid);			uid++;	// 7
    hash->add("page", uid);         uid++;	// 8
    hash->add("dialog", uid);		uid++;	// 9
    hash->add("main", uid);         uid++;	// 10
    hash->add("file", uid);			uid++;	// 11
    uid++; // 12 // hash->add("filefind", uid);
    hash->add("web", uid);			uid++;	// 13
    hash->add("worker", uid);		uid++;	// 14
    hash->add("db", uid);			uid++;	// 15
    hash->add("batch", uid);		uid++;	// 16
    hash->add("excel", uid);		uid++;	// 17
    hash->add("process", uid);		uid++;	// 18
    hash->add("ftp", uid);          uid++;	// 19 //
    hash->add("pdf", uid);			uid++;	// 20
    uid++;	// 21 // hash->add("watcher", uid);
    hash->add("was", uid);			uid++;	// 22
    hash->add("zip", uid);			uid++;	// 23
    hash->add("socket", uid);		uid++;	// 24
    hash->add("model", uid);		uid++;	// 25
    uid++;	// 26
    uid++;	// 27
    uid++;	// 28
    uid++;	// 29
    uid++;	//
    hash->add("server", uid);       uid++;	//
    hash->add("client", uid);		uid++;	// 32
    hash->add("modbus", uid);		uid++;	// 33
    hash->add("printer", uid);		uid++;	// 34
    hash->add("udp", uid);          uid++;	// 35
    hash->add("mqtt", uid);         uid++;	// 36
    uid=50;
    hash->add("rc", uid);		uid++;
    hash->add("pt", uid);		uid++;
}

bool execClassFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* rst) {
    // int cnt = arrs ? arrs->size(): 0;
    StrVar* sv= NULL;
    LPCC tag=NULL;
    switch(fc->xfuncRef) {
    case 1: { // rect
        setClassRect(arrs, rst);
    } break;
    case 6:   // size
    case 2: { // point
        setClassPoint(arrs, rst);
    } break;
    case 3: { // mimeData
        QMimeData* mimeData = new QMimeData();
        int cnt = arrs->size();
        StrVar* sv = NULL;
        LPCC ty = NULL;
        if( cnt==1 ) {
            sv = arrs->get(0);
        } else if( cnt==2 ) {
            ty=AS(0);
            sv = arrs->get(1);
        }
        if( sv ) {
            if( slen(ty)==0 ) ty = "application/nodeData";
            QByteArray ba(sv->get(),sv->length());
            mimeData->setData(ty,ba);
        }
        rst->setVar('m',2,(LPVOID)mimeData);
    } break;
    case 4: {	// drawObject
        setClassCanvas(arrs, rst);
    } break;
    case 5: { // color
        setClassColor(arrs, rst);
    } break;
    case 7: { // pair
        if( arrs==NULL ) return false;
        LPCC key=AS(0);
        rst->setVar('x',21);
        rst->add(key).add('\f').add(arrs->get(1));
    } break;
    case 8: tag="page"; break;
    case 9: tag="dialog"; break;
    case 10: tag="main"; break;
    case 11: tag="file";		break;
    case 12: tag="filefind";	break;
    case 13: tag="web";			break;
    case 14: tag="worker";		break;
    case 15: tag="db";          break;
    case 16: tag="batch";		break;
    case 17: tag="excel";		break;
    case 18: tag="process";		break;
    case 19: tag="ftp";			break;
    case 20: tag="pdf";			break;
    case 21: tag="watcher";		break;
    case 22: tag="was";			break;
    case 23: tag="zip";			break;
    case 24: tag="socket";		break;
    case 25: tag="model";		break;
    case 26: tag="audio";		break;
    case 27: tag="google";		break;
    case 28: tag="bluetooth";	break;
    case 29: tag="ftpServer";	break;
    case 30: tag="telnetServer";	break;
    case 31: tag="server";      break;
    case 32: tag="client";      break;
    case 33: tag="modbus";      break;
    case 34: tag="printer";     break;
    case 35: tag="udp";         break;
    case 36: tag="mqtt";         break;
    case 50: { // rc
        setClassRect(arrs, rst, true);
    } break;
    case 51: { // pt
        setClassPoint(arrs, rst, true);
    } break;
    default: return false;
    }
    if( slen(tag) ) {
        if( arrs ) {
            setClassTagObject(tag, AS(0), rst, arrs->get(1));
        } else {
            setClassTagObject(tag, NULL, rst);
        }
    }
    return true;
}

bool setClassRect(PARR arrs, StrVar* rst, bool bDouble) {
    int cnt=arrs==NULL ? 0: arrs->size();
    StrVar* sv=NULL;
    switch( cnt ) {
    case 0: {
        if( bDouble )
            rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(0).addDouble(0);
        else
            rst->setVar('i',4).addInt(0).addInt(0).addInt(0).addInt(0);
    } break;
    case 1: {
        sv=arrs->get(0);
        if( SVCHK('i',4) ) {
            if( bDouble )
                rst->setVar('i',5).addDouble(sv->getInt(4)).addDouble(sv->getInt(8)).addDouble(sv->getInt(12)).addDouble(sv->getInt(16));
            else
                rst->setVar('i',4).addInt(sv->getInt(4)).addInt(sv->getInt(8)).addInt(max(1,sv->getInt(12))).addInt(max(1,sv->getInt(16)) );
        } else if( SVCHK('i',5) ) {
            int sz=sizeof(double);
            double x=sv->getDouble(4), y=sv->getDouble(4+sz),ww=sv->getDouble(4+(2*sz)),hh=sv->getDouble(4+(3*sz));
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(ww).addDouble(hh);
        } else if( SVCHK('i',20) ) {
            int sz=sizeof(double);
            rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(sv->getDouble(4)).addDouble(sv->getDouble(4+sz));
        } else if( SVCHK('i',2) ) {
            rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(sv->getInt(4)).addDouble(sv->getInt(8));
        } else if( SVCHK('n',0) ) {
            TreeNode* tn=(TreeNode*)SVO;
            sv = tn->gv("@img");
            if( SVCHK('i',9) ) {
                QImage* img = (QImage*)SVO;
                rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(img->width()).addDouble(img->height());
            }
        } else if( SVCHK('i',6) ) {
            QPixmap* pix = (QPixmap*)SVO;
            if( pix ) {
                rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(pix->width()).addDouble(pix->height());
            }
        } else if( SVCHK('i',9) ) {
            QImage* pix = (QImage*)SVO;
            if( pix ) {
                rst->setVar('i',5).addDouble(0).addDouble(0).addDouble(pix->width()).addDouble(pix->height());
            }
        }
    } break;
    case 2: {
        sv=arrs->get(0);
        if( SVCHK('i',2) ) {
            int x=sv->getInt(4), y=sv->getInt(8), x1=0, y1=0;
            sv=arrs->get(1);
            if( SVCHK('i',2) ) {
                x1=sv->getInt(4), y1=sv->getInt(8);
            }
            int xs=min(x, x1), ys=min(y,y1), xe=max(x,x1), ye=max(y, y1);
            if( bDouble )
                rst->setVar('i',5).addDouble(xs).addDouble(ys).addDouble(xe-xs).addDouble(ye-ys);
            else
                rst->setVar('i',4).addInt(xs).addInt(ys).addInt(xe-xs).addInt(ye-ys);
        } else if( SVCHK('i',20) ) {
            int sz=sizeof(double);
            double x=sv->getDouble(4), y=sv->getDouble(4+sz), x1=0, y1=0;
            sv=arrs->get(1);
            if( SVCHK('i',2) ) {
                x1=sv->getInt(4), y1=sv->getInt(8);
            } else if( SVCHK('i',20) ) {
                x1=sv->getDouble(4), y1=sv->getDouble(4+sz);
            }
            double xs=min(x, x1), ys=min(y,y1), xe=max(x,x1), ye=max(y, y1);
            rst->setVar('i',5).addDouble(xs).addDouble(ys).addDouble(xe-xs).addDouble(ye-ys);
        } else if( SVCHK('i',4) ) {
            int x=sv->getInt(4), y=sv->getInt(8), w=sv->getInt(12), h=sv->getInt(16);
            double num=toDouble(arrs->get(1));
            rst->setVar('i',5).addDouble(x*num).addDouble(y*num).addDouble(w*num).addDouble(h*num);
        }
    } break;
    case 3: {
        sv=arrs->get(0);
        if( SVCHK('i',2) ) {
            int x= sv->getInt(4), y=sv->getInt(8);
            int ww=toInteger(arrs->get(1)), hh=toInteger(arrs->get(2));
            if( bDouble )
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(ww).addDouble(hh);
            else
                rst->setVar('i',4).addInt(x).addInt(y).addInt(ww).addInt(hh);
        } else if( SVCHK('i',20) ) {            
            int sz=sizeof(double);
            double x=sv->getDouble(4), y=sv->getDouble(4+sz);
            double ww=toDouble(arrs->get(1)), hh=toDouble(arrs->get(2));
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(ww).addDouble(hh);
        } else {
            double x=toDouble(arrs->get(0)), y=toDouble(arrs->get(1));
            double ww=0, hh=0;
            sv=arrs->get(2);
            if( SVCHK('i',2) ) {
                ww= sv->getInt(4), hh=sv->getInt(8);
            } else if( SVCHK('i',20) ) {
                ww= sv->getDouble(4), hh=sv->getDouble(4+sizeof(double));
            } else if( SVCHK('i',6) ) {
                QPixmap* pix = (QPixmap*)SVO;
                if( pix ) {
                    ww=pix->width(), hh=pix->height();
                }
            } else if( SVCHK('i',9) ) {
                QImage* pix = (QImage*)SVO;
                if( pix ) {
                    ww=pix->width(), hh=pix->height();
                }
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(ww).addDouble(hh);
        }
    } break;
    case 4:
    case 5: {
        sv=arrs->get(4);
        if( bDouble || SVCHK('3',1) ) {
            double x=toDouble(arrs->get(0)), y=toDouble(arrs->get(1));
            double w=toDouble(arrs->get(2)), h=toDouble(arrs->get(3));
            double ww=max(1.0,w), hh=max(1.0,h);
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(ww).addDouble(hh);
        } else {
            int x=toInteger(arrs->get(0)), y=toInteger(arrs->get(1));
            int w=toInteger(arrs->get(2)), h=toInteger(arrs->get(3));
            int ww=max(1,w), hh=max(1,h);
            if( cnt>4) {
                int cw=ww/2, ch=hh/2;
                x-=cw, y-=ch;
            }
            rst->setVar('i',4).addInt(x).addInt(y).addInt(ww).addInt(hh);
        }
    } break;
    default: {
        rst->setVar('i',4).addInt(0).addInt(0).addInt(0).addInt(0);
    } break;
    }
    return true;
}

bool setClassPoint(PARR arrs, StrVar* rst, bool bDouble) {
    int cnt=arrs==NULL ? 0 : arrs->size();
    StrVar* sv=NULL;
    if( bDouble ) {
        if( cnt==0 ) {
            rst->setVar('i',20).addDouble(0).addDouble(0);
        } else if( cnt==1 ) {
            sv=arrs->get(0);
            if( SVCHK('i',5) || SVCHK('i',20) ) {
                int sz=sizeof(double);
                rst->setVar('i',20).addDouble(sv->getDouble(4)).addDouble(sv->getDouble(4+sz));
            } else if( SVCHK('i',2) ) {
                rst->setVar('i',20).addDouble(sv->getInt(4)).addDouble(sv->getInt(8));
            } else if( SVCHK('i',4) ) {
                rst->setVar('i',20).addDouble(sv->getInt(4)).addDouble(sv->getInt(8));
            }
        } else if( cnt==2 ) {
            rst->setVar('i',20).addDouble(toDouble(arrs->get(0))).addDouble(toDouble(arrs->get(1)));
        }
    } else {
        if( cnt==0 ) {
            rst->setVar('i',2).addInt(0).addInt(0);
        } else if( cnt==1 ) {
            sv=arrs->get(0);
            if( SVCHK('i',2) ) rst->setVar('i',2).addInt(sv->getInt(4)).addInt(sv->getInt(8));
        } else if( cnt==2 ) {
            rst->setVar('i',2).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1)));
        } else {
            if( checkFuncObject(arrs->get(2),'3',1) ) {
                double x=toDouble(arrs->get(0)), y=toDouble(arrs->get(1));
                rst->setVar('i',20).addDouble(x).addDouble(y);
            } else {
                rst->setVar('i',2).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1)));
            }
        }
    }
    return true;
}
TreeNode* getBaroObject(LPCC tag, LPCC id) {
    TreeNode* node=getTreeNode(tag, id, false);
    if( node==NULL ) return NULL;
    setClassTagObject(tag,id);
    return node;
}
int setClassFuncNode(TreeNode* h, XFuncNode* pfn, char ch, TreeNode* thisNode, U16 stat ) {
    int cnt=0;
    if( h==NULL ) return cnt;
    h->xstat=stat;

    // XListArr* a=NULL;
    if( thisNode && ch=='n' ) {
        StrVar* sv=NULL;
        for( WBoxNode* bn=h->First(); bn; bn=h->Next() ) {
            sv=&(bn->value);
            if( SVCHK('f',0) ) {
                XFuncNode* fn=(XFuncNode*)SVO;
                if( pfn ) fn->xparent=pfn;
                fn->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                cnt++;
            }
        }
    }
    return cnt;
}

bool setClassTagObject(LPCC tag, LPCC id, StrVar* rst, StrVar* svObj) {
    if( tag==NULL ) return false;
    if( slen(id)==0 ) id="main";
    TreeNode* cf=getTreeNode(tag, id);
    if( cf==NULL ) {
        qDebug("#9#create class error (tag:%s, id:%s)", tag, id);
        return false;
    }
    if( cf->xstat && cf->cmp("tag",tag) && cf->isNodeFlag(FLAG_PERSIST) ) {
        if( rst ) rst->setVar('n',0,(LPVOID) cf);
        return true;
    }
    cf->set("tag", tag);
    cf->set("id", id);
    cf->setNodeFlag(FLAG_PERSIST);
    if( ccmp(tag,"file") ) {
        cf->xstat=FNSTATE_FILE;
    } else if( ccmp(tag,"page") ) {
        cf->xstat=WTYPE_PAGE;
    } else if( ccmp(tag,"dialog") ) {
        cf->xstat=WTYPE_DIALOG;
    } else if( ccmp(tag,"main") ) {
        cf->xstat=WTYPE_MAIN;
    } else if( ccmp(tag,"db") ) {
        LPCC inode=toString(svObj);
        SqlDatabase* db=getModelDatabase(id, inode, cf);
        if( db==NULL ) {
            cf->set("dsn", id);
            cf->set("dbms", "sqlite");
            sqliteDb(id, NULL, cf);
        }
        cf->xstat=FNSTATE_DB;
    } else if( ccmp(tag,"worker") ) {
        int idx=getCf()->incrNum("@workerIndex");
        cf->xstat=FNSTATE_WORKER;
        cf->setInt("@index", idx+1);
    } else if( ccmp(tag,"process") ) {
        cf->xstat=FNSTATE_PROCESS;
    } else if( ccmp(tag,"was") ) {
        cf->xstat=FNSTATE_WEBSERVER;
    } else if( ccmp(tag,"web") ) {
        cf->xstat=FNSTATE_WEBCLIENT;
    } else if( ccmp(tag,"ftp") ) {
        cf->xstat=FNSTATE_FTPCLIENT;
    } else if( ccmp(tag,"server") ) {
        cf->xstat=FNSTATE_MESSAGE_SERVER;
    } else if( ccmp(tag,"client") ) {
        cf->xstat=FNSTATE_MESSAGE_CLIENT;
    } else if( ccmp(tag,"socket") ) {
        cf->xstat=FNSTATE_SOCKET;
    } else if( ccmp(tag,"udp") ) {
        cf->xstat=FNSTATE_UDP;
    } else if( ccmp(tag,"mqtt") ) {
        cf->xstat=FNSTATE_MQTT;
    } else if( ccmp(tag,"batch") ) {
        cf->xstat=FNSTATE_CRON;
    } else if( ccmp(tag,"model") ) {
        cf->xstat=FNSTATE_MODEL;
        StrVar* sv=cf->GetVar("@model");
        TreeModel* model;
        if( SVCHK('m',0) ) {
            model=(TreeModel*)SVO;
        } else {
            model=new TreeModel(cf);
            model->setRootNode(cf);
            sv->setVar('m',0,(LPVOID)model);
        }
        /*
        TreeProxyModel* proxy= NULL;
        sv=node->GetVar("@proxy");
        if( SVCHK('m',1) ) {
            proxy = (TreeProxyModel*)SVO;
        } else {
            proxy = new TreeProxyModel(node);
            proxy->setRootModel(model);
            node->GetVar("@proxy")->setVar('m',1,(LPVOID)proxy);
        }
        */

    } else if( ccmp(tag,"excel") ) {
        cf->xstat=FNSTATE_EXCEL;
        cf->setInt("row",1);
        cf->setInt("column",1);
    } else if( ccmp(tag,"pdf") || ccmp(tag,"printer") ) {
        cf->xstat=FNSTATE_PDF;
    } else if( ccmp(tag,"zip") ) {
        cf->xstat=FNSTATE_ZIP;
    } else if( ccmp(tag,"modbus") ) {
        cf->xstat=FNSTATE_MODBUS;
        if( svObj ) {
            cf->set("@serverId", toString(svObj) );
        }
    } else {
        cf->xstat=0;
    }
    U16 stat=cf->xstat;
    qDebug("------ setClassTagObject %s (%d) ---------", tag, (int)cf->xstat);
    if( stat==WTYPE_PAGE || stat==WTYPE_DIALOG || stat==WTYPE_MAIN ) {
        QWidget* widget=NULL;
        if( cf->childCount() ) {
            // ex) Baro.page('id', 'filename', 'parentId');
            QWidget* parentWidget=NULL;
            StrVar* sv=svObj;
            if( SVCHK('n',0) ) {
                TreeNode* cur=(TreeNode*)SVO;
                if( cur ) {
                    parentWidget=gwidget(cur);
                    if( parentWidget ) {
                        cf->GetVar("@parent")->setVar('n',0,(LPVOID)cur);
                    }
                }
            } else {
                LPCC parentId=toString(sv);
                if( slen(parentId)==0 ) {
                    parentId=cf->get("parentWidget");
                }
                if( slen(parentId) ) {
                    LPCC group=NULL, code=NULL;
                    int pos=IndexOf(parentId,'.');
                    if( pos>0 ) {
                        group=gBuf.add(parentId,pos), code=parentId+pos+1;
                    } else {
                        group="page", code=parentId;
                    }
                    if( slen(group) && slen(code) ) {
                        TreeNode* cur=getTreeNode(group, code);
                        if( cur ) {
                            parentWidget=gwidget(cur);
                            if( parentWidget ) {
                                cf->GetVar("@parent")->setVar('n',0,(LPVOID)cur);
                            }
                        }
                    }
                }
            }
            widget=gwidget(cf);
            if( widget=NULL ) widget=createWidget(tag, cf, parentWidget);
        }        
    } else {
        if( !cf->isNodeFlag(FLAG_INIT) ) {
            callInitFunc(cf);
        }
    }
    if( rst ) rst->setVar('n',0,(LPVOID)cf);
    return true;
}

bool setClassCanvas(PARR arrs, StrVar* rst) {
    if( arrs==NULL ) {
        TreeNode* d=new TreeNode(2, true);
        d->set("tag", "drawObject");
        d->xstat=FNSTATE_DRAW;
        d->xtype=0;
        setVarRectF(d->GetVar("@rect"), QRectF(0,0,0,0));
        rst->setVar('n',0,(LPVOID)d);
        return true;
    }
    QImage* img = NULL;
    StrVar* sv=NULL;
    bool bfill=true;
    int cnt=arrs->size(), w=0, h=0;
    if( cnt==1 ) {
        sv = arrs->get(0);
        if( SVCHK('i',2) ) {
            w=sv->getInt(4), h=sv->getInt(8);
            img = new QImage(w,h, QImage::Format_ARGB32);
        } else if( SVCHK('i',4) ) {
            w=sv->getInt(12), h=sv->getInt(16);
            img = new QImage(w,h, QImage::Format_ARGB32);
		} else if( SVCHK('i',5) || SVCHK('i',20) ) {
			int sz=sizeof(double);
			if( SVCHK('i',5) ) {
				w=(int)sv->getDouble(4+(2*sz)), h=(int)sv->getDouble(4+(3*sz));
			} else {
				w=(int)sv->getDouble(4), h=(int)sv->getDouble(4+sz);
			}
			img = new QImage(w,h, QImage::Format_ARGB32);
		} else if( SVCHK('i',6) ) {
            QPixmap* p=(QPixmap*)SVO;
            if( p && !p->isNull() ) {
                img = new QImage(p->toImage());
                w=img->width(), h=img->height();
                bfill=false;
            }
        } else if( SVCHK('i',7) ) {
            img = new QImage();
            img->loadFromData((const uchar *)sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
            w=img->width(), h=img->height();
            bfill=false;
        } else if( SVCHK('i',9) ) {
            QImage* prev = (QImage*)SVO;
            if( prev && !prev->isNull() ) {
                img = new QImage(*prev);
                w=img->width(), h=img->height();
                bfill=false;
            }
            w=img->width(), h=img->height();
        } else if( SVCHK('n',0) ) {
            TreeNode* tn=(TreeNode*)SVO;
            sv = tn->gv("@img");
            if( SVCHK('i',9) ) {
                QImage* prev = (QImage*)SVO;
                if( prev && !prev->isNull() ) {
                    img = new QImage(*prev);
                    w=img->width(), h=img->height();
                    bfill=false;
                }
            }
        } else {
            LPCC filenm=AS(0), ext=NULL;
            int pos=LastIndexOf( filenm,'.');
            if( pos>0 ) {
                ext=filenm+pos+1;
            }
            if( slen(ext)==0 )  ext="png";
            img = new QImage();
            img->load(KR(filenm), ext);
            if( !img->isNull() ) {
                w=img->width(), h=img->height();
            }
            bfill=false;
        }
    } else if( cnt==2 ) {
        if( isNumberVar(arrs->get(0)) ) {
            w=toInteger(arrs->get(0)), h=toInteger(arrs->get(1));
            if( w>0 && h>0 ) {
                img = new QImage(w,h, QImage::Format_ARGB32);
            }
        }
    }
    if( img ) {
        if( bfill ) {
            img->fill(Qt::transparent);
        }
        // qDebug("#### Draw image create (%d, %d) #########\n\n",w, h);
        QPainter* painter = new QPainter(img);
        TreeNode* d=new TreeNode(2, true);
        d->set("tag", "drawObject");
        d->xstat=FNSTATE_DRAW;
        d->xtype=0;
        setVarRectF(d->GetVar("@rect"), QRectF(0,0,w,h));
        d->GetVar("@img")->setVar('i',9,(LPVOID)img);
        d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        rst->setVar('n',0,(LPVOID)d);
    } else {
        rst->reuse();
    }
    return true;
}
/*
inline void getHsv2rgb(double hh, double hs, double hv, int* r, int* g, int* b) {
    if(hs <= 0.0) {       // < is bogus, just shuts up warnings
        *r = hv;
        *g = hv;
        *b = hv;
    } else {
        long        i;
        double      ha, p, q, t, ff;
        ha = hh;
        if(ha >= 360.0) ha = 0.0;
        ha /= 60.0;
        i = (long)ha;
        ff = ha - i;
        p = hv * (1.0 - hs);
        q = hv * (1.0 - (hs * ff));
        t = hv * (1.0 - (hs * (1.0 - ff)));

        switch(i) {
        case 0:
            *r = hv;
            *g = t;
            *b = p;
            break;
        case 1:
            *r = q;
            *g = hv;
            *b = p;
            break;
        case 2:
            *r = p;
            *g = hv;
            *b = t;
            break;

        case 3:
            *r = p;
            *g = q;
            *b = hv;
            break;
        case 4:
            *r = t;
            *g = p;
            *b = hv;
            break;
        case 5:
        default:
            *r = hv;
            *g = p;
            *b = q;
            break;
        }
    }
}

inline float getHue2rgb(float p, float q, float t) {
  if (t < 0)
    t += 1;
  if (t > 1)
    t -= 1;
  if (t < 1./6)
    return p + (q - p) * 6 * t;
  if (t < 1./2)
    return q;
  if (t < 2./3)
    return p + (q - p) * (2./3 - t) * 6;
  return p;
}

inline void getHsl2rgb(int h, int s, int l, int* r, int* g, int* b) {

  if(0 == s) {
    *r = *g = *b = l; // achromatic
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    *r = getHue2rgb(p, q, h + 1./3) * 255;
    *g = getHue2rgb(p, q, h) * 255;
    *b = getHue2rgb(p, q, h - 1./3) * 255;
  }
}
*/

bool setClassColor(PARR arrs, StrVar* rst) {
    int cnt= arrs==NULL ? 0 : arrs->size();
    StrVar* sv=arrs->get(0);
    if( !isNumberVar(sv) &&  cnt>2 ) {
        LPCC type=AS(0);
        QColor clr;
        int r=toInteger(arrs->get(1)), g=toInteger(arrs->get(2)), b=toInteger(arrs->get(3)), a=255;
        if(isNumberVar(arrs->get(4))) {
            a=toInteger(arrs->get(4));
        }
        if( ccmp(type,"hsl") || ccmp(type,"hsla") ) {
            clr.setHsl(r,g,b,a);
            clr.toRgb().getRgb(&r,&g,&b,&a);
        } else if(ccmp(type,"hsv") || ccmp(type,"hsva") ) {
            clr.setHsv(r,g,b,a);
            clr.toRgb().getRgb(&r,&g,&b,&a);
        }
        // qDebug("color %s %d,%d,%d : %d", type, r,g,b, a );
        rst->setVar('i',3).addInt(r).addInt(g).addInt(b).addInt(a);
        return true;
    }
    if( cnt==0 ) {
        rst->setVar('i',3).addInt(0).addInt(0).addInt(0).addInt(255);
    } else if( cnt==1 ) {
        QColor clr=getGlobalColor(sv);
        if( SVCHK('i',3)) {
            rst->add(sv);
        } else {
            rst->setVar('i',3).addInt(clr.red()).addInt(clr.green()).addInt(clr.blue()).addInt(clr.alpha());
        }
    } else if( cnt==3 ) {
        rst->setVar('i',3).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1))).addInt(toInteger(arrs->get(2)));
    } else if( cnt==4 ) {
        rst->setVar('i',3).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1))).addInt(toInteger(arrs->get(2))).addInt(toInteger(arrs->get(3)));
    }
    return true;
}

inline StrVar* applySouceRemove(StrVar* var, int sp, int ep, StrVar* rst) {
    XParseVar pv(var, sp, ep);
    while( pv.valid() ) {
        pv.findEnd("<#", FIND_SKIP );
        if( pv.cur>pv.prev ) rst->add(var,pv.prev,pv.cur);
        if( !pv.valid() ) break;
        pv.findEnd("\n");
        sp=pv.start;
        pv.findEnd("#>");
        rst->add(var, sp, pv.cur);
    }
    return rst;
}
inline WidgetConf* findTagIdWidgetConf(WidgetConf* root, LPCC tag, LPCC id) {
    if( root==NULL) return NULL;
    for( int n=0,sz=root->childCount(); n<sz; n++ ) {
        WidgetConf* cur=getWidgetConf(root->child(n));
        if( cur ) {
            if( cur->cmp("tag",tag) ) {
                if( slen(id)==0 ) return cur;
                if( cur->cmp("id",id) ) return cur;
            }
        }
    }
    return NULL;
}

inline bool baroSourceCanvas(TreeNode* canvasNode, StrVar* var, int sp, int ep, TreeNode* tagNode ) {
    QWidget* widget=gwidget(canvasNode);
    GCanvas* canvas=qobject_cast<GCanvas*>(widget);
    if( canvas==NULL ) {
        qDebug("baroSourceCanvas error (%s)", canvasNode ? canvasNode->log(): "node not define" );
        return false;
    }
    TreeNode* target=getTreeNodeTemp();
    WidgetConf* root=canvas->widgetConf();
    XParseVar pv(var, sp, ep);
    LPCC mode=NULL;
    bool ret=true;
    while( pv.valid() ) {
        char ch=pv.ch();
        if( ch==0 ) break;
        if( ch==';' ) {
            ch=pv.incr().ch();
            if( ch==0 ) break;
        }
        if( ch=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) {
                ret=false;
                break;
            }
            ch=pv.ch();
        }
        sp=pv.start;
        if( ch=='<' ) {
            LPCC tag=pv.incr().NextWord();
            if( slen(tag)==0 ) {
                break;
            }
            pv.setPos(sp);
            if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
                target->set("id","");
                target->set("mode","");
                parseProp(target, var, pv.prev,pv.cur, PROP_GETID );
                mode=target->get("mode");
                if( ccmp(mode,"skip") ) continue;
                TreeNode* node=NULL;
                if( ccmp(tag,"widgets") ) {
                    TreeNode* cf=canvas->config();
                    node=cf?cf->addNode("@widgets"):NULL;
                } else {
                    WidgetConf* wcf=findTagIdWidgetConf(root, tag, target->get("id") );
                    if( wcf==NULL ) {
                        qDebug("new canvas Item (tag:%s, id:s)", tag, target->get("id"));
                        wcf=root->addWidgetConf();
                        wcf->set("tag",tag);
                    }
                    node=static_cast<TreeNode*>(wcf);
                }
                if( node ) {
                    sp=parseProp(node, var, pv.prev,pv.cur );
                    if( sp==-1 ) {
                        qDebug("canvas layout source not match (%s, %s)", tag, node->log());
                        break;
                    }
                    if( ccmp(tag,"widgets") ) {
                        canvas->addWidgets(node, var, sp, pv.cur);
                    } else {
                        node->GetVar("@canvasNode")->setVar('n',0,(LPVOID)canvas->config() );
                        setModifyClassFunc(node, var, sp, pv.cur, NULL, NULL, false, "event");
                    }
                }
            } else {
                ret=false;
                qDebug("#9#canvas item match error (tag:%s)", tag);
                break;
            }
        } else {
            ch=pv.moveNext().ch();
            if( ch=='(' ) {
                if( !pv.match("(",")",FIND_SKIP) ) break;
            }
            ch=pv.ch();
            if( ch=='{' ) {
                if( !pv.match("{","}",FIND_SKIP) ) break;
                setModifyClassFunc(canvas->config(), var, sp, pv.start, NULL, NULL, false, "event");
            }
        }
        if( ccmp(mode,"break") ) break;
    }
    if( tagNode && !tagNode->cmp("update","false") ) {
        canvas->invalidate(true, true);
    }
    return true;
}

bool baroSourceResult(StrVar* var, int sp, int ep, StrVar* rst) {
    if( rst==NULL ) {
        return false;
    }
    bool ret=false;
    XParseVar pv(var, sp, ep);
    rst->reuse();
    while( pv.valid() ) {
        pv.findEnd("<#",FIND_SKIP);
        if( !pv.valid() ) break;
        pv.findEnd("\n");
        sp=pv.start;
        pv.findEnd("#>");
        rst->add(var, sp, pv.cur);
        ret=true;
    }
    return ret;
}

bool baroSourceUpdate(StrVar* var, int sp, int ep, StrVar* rst) {
    if( rst==NULL ) rst=getStrVarTemp();
    TreeNode* node=NULL;
    XParseVar pv(var, sp, ep);
    StrVar* callVar=uom.getStrVar();
    char c=pv.ch();
    LPCC tag=NULL;
    bool ret=true;
    for(int idx=0; idx<1024; idx++  ) {
        c=pv.ch();
        if( pv.ch()=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) break;
            c=pv.ch();
        }
        if( c!='<' ) {
            if( idx==0 ) {
                callScriptFunc(var, sp, ep );
                return true;
            }
            break;
        }
        sp=pv.start;
        tag=pv.incr().NextWord();
        if(slen(tag)==0 ) {
            break;
        }
        qDebug("#0#--baroSourceUpdate %s %d,%d\n", tag, sp,ep);
        pv.setPos(sp);
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            if( ccmp(tag,"temp") ) {
                continue;
            }
            if( node==NULL ) node=getTreeNodeTemp();
            LPCC mode=NULL;
            node->set("id","");
            node->set("mode","");
            parseProp(node, var, pv.prev,pv.cur, PROP_GETID );
            if( ccmp(tag,"call") ) {
                TreeNode* callNode=cfNode("@callVars", true);
                sp=parseProp(callNode, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate call parse error: %s %s\n", tag, node->log() );
                    return false;
                }
                mode=callNode->get("mode");
                if( ccmp(mode, "skip") ) {
                    callNode->set("mode", "");
                    continue;
                }
                callVar->add(var, sp, pv.cur);

            } else if( ccmp(tag,"pages") || ccmp(tag,"widgets") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate widget parse error: %s %s\n", tag, node->log() );
                    return false;
                }
                TreeNode* root=NULL;
                StrVar* sv=NULL;
                LPCC target=node->get("id");
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                int pos=IndexOf(target,'.');
                if( pos>0 ) {
                    LPCC group=gBuf.add(target,pos), code=target+pos+1;
                    root=getTreeNode(group, code, false);
                    /*
                    if( root ) {
                        sv=cfVar("@currentObject");
                        if( SVCHK('n',0) ) {
                            prevObject=(TreeNode*)SVO;
                        }
                        if( sv ) sv->setVar('n',0,(LPVOID)root);
                    }
                    */
                }
                baroSourceApply(var, sp, pv.cur );
            } else if( ccmp(tag,"conf") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate conf parse error: %s\n", node->log() );
                    return false;
                }
                LPCC id=node->get("id");
                if( slen(id) ) {
                    TreeNode* cf=NULL;
                    StrVar* sv=confVar(NULL);
                    if( SVCHK('n',0) ) {
                        cf=(TreeNode*)SVO;
                    }
                    cf->GetVar(id)->set(var, sp, pv.cur);
                }
            } else if( ccmp(tag,"define") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate define parse error: %s %s\n", tag, node->log() );
                    return false;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;

                setBaroDefine(var, sp, pv.cur, node);
            } else if( ccmp(tag,"func") || ccmp(tag,"subfunc") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate func parse error: %s %s\n", tag, node->log() );
                    return false;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;

                LPCC funcLine=node->get("type");
                if( slen(funcLine) ) funcLine="common";                
                setFuncSource(var, sp, pv.cur, funcLine);
            } else if( ccmp(tag,"config") ) {

            } else {
                LPCC id=node->get("id");
                TreeNode* root=slen(id)? getTreeNode(tag,id, false): NULL;
                /*
                if( root==NULL ) {
                    StrVar* sv=cfVar("@currentObject");
                    if( SVCHK('n',0)) {
                        root=(TreeNode*)SVO;
                    }
                }
                */
                if( root ) {
                    sp=parseProp(root, var, pv.prev,pv.cur);
                    if( sp==-1 ) {
                        qDebug("#9#baroSourceUpdate error pageNode: %s\n", root->log() );
                        return false;
                    }
                    mode=root->get("mode");
                    if( ccmp(mode, "skip") ) continue;

                    LPCC type=root->get("type");
                    if( slen(type)==0 ) type="common";
                    setMemberSource(root, NULL, var, sp, pv.cur, type );
                }
            }
            if( ccmp(mode, "break") ) break;
        } else {
            qDebug("#9#baroSourceUpdate func not match: %s %s\n", tag, node->log() );
            break;
        }
    }
    if( callVar && ret ) {
        TreeNode* callNode=cfNode("@callVars");
        cfVar("@inlineMode")->reuse();
        callScriptFunc(callVar, 0, callVar->length() );
        callNode->reuse();
    }
    return ret;
}

TreeNode* baroWidgetApply(LPCC tag, LPCC id, StrVar* var, int sp, int ep, LPCC classId ) {
    bool pageCheck=ccmp(tag,"page") || ccmp(tag,"dialog") || ccmp(tag,"main");
    bool widgetCheck=false;
    if( !pageCheck ) {
		widgetCheck=ccmp(tag,"widget") || ccmp(tag,"tree") || ccmp(tag,"grid") || ccmp(tag,"canvas") || ccmp(tag,"input") ||
                ccmp(tag,"editor") || ccmp(tag,"combo") || ccmp(tag,"input") || ccmp(tag,"video") || ccmp(tag,"webview") ||
                ccmp(tag,"button") || ccmp(tag,"check") || ccmp(tag,"radio") || ccmp(tag,"context") || ccmp(tag,"splitter") ||
                ccmp(tag,"calendar") || ccmp(tag,"spin") || ccmp(tag,"date") || ccmp(tag,"time") || ccmp(tag,"label") ||
                ccmp(tag,"group") || ccmp(tag,"tab") || ccmp(tag,"div");
    }
    bool bTest=false;
    LPCC widgetId=NULL;
    if( slen(classId) ) {
        if(ccmp(classId,"sourceApply") ) {            
            if(slen(id) ) {
                widgetId=FMT("test:%s", id );
            } else {
                bTest=true;
                widgetId=FMT("test_%d", getCf()->incrNum("@test_index")+1 );
            }

        } else {
            if(slen(id)==0) id="main";
            widgetId=FMT("%s:%s",classId, id);
        }
    } else if( slen(id)==0 ) {
        widgetId=FMT("%s_%d",tag, getCf()->incrNum(FMT("@%s_index",tag))+1 );
    } else {
        widgetId=id;
    }
    TreeNode* funcInfo=cfNode("@funcInfo");
    LPCC includeFileName=funcInfo->get("includeFileName");
    TreeNode* funcMap=slen(includeFileName)? getTreeNode("@inc","widgets"): NULL;
    if(funcMap) {
        funcMap->set(widgetId,includeFileName);
    }

    qDebug("...baroWidgetApply %s %s ...(%d,%d)",tag, widgetId, sp, ep);
    StrVar* rst=getStrVarTemp();
    TreeNode* root=NULL;
    StrVar* sv=NULL;
    QWidget* parentWidget=NULL;
    bool bClass=false;
    if( pageCheck ) {
        root=getTreeNode(tag,widgetId,false);
        if( root ) {
            if( gwidget(root) ) {
                /*
                sv=cfVar("@currentObject");
                if( sv ) {
                    sv->setVar('n',0,(LPVOID)root);
                }
                */
                return root;
            }
        } else {
            root=getTreeNode(tag,widgetId);
        }
    } else if( widgetCheck ) {
        root=getTreeNode(tag,widgetId,false);
        if(root ) {
            if( gwidget(root) ) {
                return root;
            }
        } else {
            root=getTreeNode(tag,widgetId);
        }
    } else {
        bClass=true;
        root=getTreeNode("class",widgetId);
        if(root && !ccmp(tag,"class") ) root->set("@targetTag", tag);
    }
    if( root==NULL ) {
        qDebug("#9#create object error (tag:%s,id:%s)", tag, widgetId);
        return NULL;
    }
    if( root->childCount()>0 ) {
        qDebug("baro widget apply already child use count %d ", root->childCount());
        root->reset(true);
    }
    sp=parseProp(root, var, sp, ep);    
    if( sp==-1 ) {
        qDebug("#9#baroWidgetApply error tag:%s, node:%s\n", tag, root->log() );
        return NULL;
    }
    LPCC parentCode=root->get("parent");
    if( slen(id)==0 ) root->set("id", id);
    if(slen(parentCode) ) {
        int pos=IndexOf(parentCode,':');
        if(pos!=-1) {
            parentCode=FMT("%s:%s",classId, parentCode);
        }
        parentWidget=gwidget(getTreeNode("page",parentCode,false));
        if( parentWidget==NULL ) {
            parentWidget=gwidget(getTreeNode("dialog",parentCode,false));
            if( parentWidget==NULL ) {
                parentWidget=gwidget(getTreeNode("main",parentCode,false));
            }
        }
    }
    if( root ) {
        root->set("tag", tag);
        root->setNodeFlag(FLAG_INIT);
        if( slen(widgetId)) {
            root->set("@baseCode",widgetId);
            if(!root->isset("id") ) root->set("id", widgetId);
        }
        /*
        sv=funcInfo->gv("@inlineSource");
        if(sv && sv->length() ) {
            nodeInlineSource(root, NULL, sv, 0, sv->length(), rst, "@all", NULL, false, bClass, false );
            qDebug("#0#apply source inline (info:%s, size:%d)\n", root->log(), sv->length() );
            sv->reuse();
        }
        sv=funcInfo->gv("@confSource");
        if(sv && sv->length() ) {
            nodeInlineSource(root, NULL, sv, 0, sv->length(), rst, "@all", NULL, false, bClass, false );
            sv->reuse();
        }
        */
    } else {
        qDebug("#9# baro widget apply root node error");
        return NULL;
    }

    if( pageCheck ) {
        if( parsePageNode(root, var, sp, ep) ) {
            QWidget* widget=createWidget(tag, root, parentWidget);
            if( widget ) {
                if(!root->isset("tag")) root->set("tag", tag);
                if( bTest ) {
                    sv=cfVar("@currentObject");
                    if( sv ) {
                        sv->setVar('n',0,(LPVOID)root);
                    }
                    if( isPageNode(root) ) {
                        QRect rc = QApplication::desktop()->screenGeometry(QCursor::pos());
                        widget->move(rc.center() - widget->rect().center());
                        widget->show();
                    }
                }

            } else {
                qDebug("#9# page create error %s", root->log() );
                return NULL;
            }
        } else {
            qDebug("#9# page create parse error %s", root->log() );
            return NULL;
        }
    } else if( widgetCheck ) {
        if( ccmp(tag,"group") ) {   // || ccmp(tag,"tab") || ccmp(tag,"div")
            if( parsePageNode(root, var, sp, ep) ) {
                QWidget* widget=createWidget(tag, root, parentWidget);
                if( widget ) {
                } else {
                    qDebug("#9# group create error %s", root->log() );
                    return NULL;
                }
            } else {
                qDebug("#9# group parse error %s", root->log() );
                return NULL;
            }
        } else {
            QWidget* widget=createWidget(tag, root, parentWidget);
            if( widget ) {
                setModifyClassFunc(root, var, sp, ep, NULL, rst, false, "func");
            } else {
                qDebug("#9# widget create error %s", root->log() );
                return NULL;
            }
        }
    } else {
        setModifyClassFunc(root, var, sp, ep, NULL, rst, false, bClass?"class":"func");
    }
    return root;
}

#ifdef OLD_VER_USE
XListArr* baroSourceApply(StrVar* var, int sp, int ep, StrVar* rst, XListArr* arr) {
    if( var==NULL ) return NULL;
    if( ep<=0 ) ep=var->length();
    /*
    if( var->find("<#", sp, ep, FIND_SKIP)!=-1 ) {
        var=applySouceRemove(var, sp, ep, uom.getStrVar() );
        sp=0, ep=var->length();
    }
    */
    StrVar callVar, classId;
    TreeNode* funcInfo=cfNode("@funcInfo");
    TreeNode* node=getTreeNodeTemp();
    XParseVar pv(var, sp, ep);
    LPCC tag=NULL;
    LPCC baseCode=funcInfo->get("baseCode");
    // classId == fileCode
    if( slen(baseCode) ) {
        int pos=IndexOf(baseCode,':');
        if(pos>0) {
            classId.set(baseCode,pos);
        } else {
            classId.set(baseCode);
        }
    }
    XListArr* arrWidgets=arr;
    char c=pv.ch();    
    bool bInc=funcInfo ? funcInfo->getBool("@include"):false;
    for(int idx=0; idx<1024; idx++  ) {
        if( pv.ch()=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) break;
            c=pv.ch();
        }
        if( c!='<' ) break;
        sp=pv.start;
        tag=pv.incr().NextWord();
        pv.setPos(sp);
        if( slen(tag)==0 ) break;
        qDebug("#0#baroSourceApply tag: %s\n", tag  );
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            LPCC mode=NULL;
            if( ccmp(tag,"inline") || ccmp(tag,"event") || ccmp(tag,"text") || ccmp(tag,"sql") ) {
                continue;
            }
            node->reuse();
            node->set("tag", tag);
            if( ccmp(tag,"func") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                LPCC funcLine=node->get("type");
                LPCC tmpl=NULL;
                if( slen(funcLine)==0 ) funcLine="common";
                if( funcInfo ) {
                    LPCC ref=node->get("ref");
                    if(ccmp(ref,"impl") && node->isset("name")) {
                        tmpl=ref;
                    }
                    funcInfo->set("ref", ref);
                }
                if( node->isset("template")) {
                    tmpl=node->get("template");
                }
                if( slen(tmpl)) {
                    if(rst==NULL) {
                        rst=getStrVarTemp();
                    }
                    if(sp<pv.cur) {
                        StrVar* source=node->GetVar("source");
                        if(ccmp(tmpl,"impl") && node->isset("use")) {
                            LPCC use=node->get("use");
                            XParseVar sub(var, sp, pv.cur);
                            sub.findEnd("class(");
                            if(sub.valid()) {
                                source->add(var,sub.prev, sub.cur);
                                sub.findEnd("{");
                                source->add("class() {\n\t").add("use(").add(use).add(");\n");
                                source->add(var,sub.start,sub.wep);
                            } else {
                                source->add("class() {\n\t").add("use(").add(use).add(");\n}\n");
                                source->add(var, sp, pv.cur);
                            }
                            // qDebug("imple use source:%s\n", source->get());
                        } else {
                            source->set(var, sp, pv.cur);
                        }
                        if(ccmp(tmpl,"impl")) {
                            LPCC name=node->get("name");
                            TreeNode* ctrl=getTreeNode("ctrl",name,false);
                            if(ctrl) {
                                XParseVar sub(source);
                                parseArrayVar(sub,ctrl,NULL,rst);
                                qDebug("#0#impl reload function name:%s \n", name);
                                continue;
                            }
                        }
                    }
                    StrVar* src=confVar(FMT("template.%s",tmpl));
                    XParseVar sub(src);

                    makeTextData(sub,NULL,rst->reuse(),0x20000,node);
                    if(rst->length()) {
                        setFuncSource(rst, 0, rst->length(), funcLine);
                    }
                } else {
                    setFuncSource(var, sp, pv.cur, funcLine);
                }
            } else if( ccmp(tag,"conf") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                LPCC id=node->get("id");
                if( slen(id) ) {
                    TreeNode* cf=NULL;
                    StrVar* sv=confVar(NULL);
                    if( SVCHK('n',0) ) {
                        cf=(TreeNode*)SVO;
                        cf->GetVar(id)->set(var, sp, pv.cur);
                        LPCC includeFileName=funcInfo->get("includeFileName");
                        TreeNode* funcMap=slen(includeFileName)? getTreeNode("@inc","confs"): NULL;
                        if(funcMap) {
                            funcMap->set(id,includeFileName);
                        }
                    }
                }
            } else if( ccmp(tag,"define") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate define parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                setBaroDefine(var, sp, pv.cur, node);
            } else if( ccmp(tag,"call") ) {
                TreeNode* callNode=cfNode("@callVars", true);
                sp=parseProp(callNode, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                if( bInc || callNode->cmp("mode", "skip") ) continue;
                callVar.add(var, sp, pv.cur);
            } else if( ccmp(tag,"pages") ||  ccmp(tag,"widgets") ) {
                sp=parseProp(node, var, pv.prev,pv.cur, PROP_SOURCE );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( bInc || ccmp(mode, "skip") ) continue;
                if( arrWidgets==NULL ) {
                    arrWidgets=getListArrTemp();
                }
                LPCC newId=node->get("newId"), base=node->get("base");
                if( slen(newId) ) {
                    funcInfo->GetVar("newId")->set(newId);
                }
                if( slen(base) ) {
                    funcInfo->set("baseCode", base);
                }
                baroSourceApply(var,sp,pv.cur,rst,arrWidgets);
                if( slen(newId) ) {
                    funcInfo->GetVar("newId")->reuse();
                }
                if( slen(base) && !ccmp(baseCode,base)) {
                    funcInfo->set("baseCode", baseCode);
                }
            } else {
                parseProp(node, var, pv.prev, pv.cur, PROP_SOURCE );
                mode=node->get("mode");
                if( bInc || ccmp(mode, "skip") ) continue;
                LPCC id=NULL, newId=funcInfo->get("newId");
                if( slen(newId)==0 ) {
                    id=node->get("id");
                } else {
                    qDebug("new id==%s", newId);
                    id=newId;
                }
                TreeNode* nodeObject=baroWidgetApply(tag, id, var, pv.prev, pv.cur, classId.get());
                if( nodeObject ) {
                    if( arrWidgets==NULL ) {
                        arrWidgets=getListArrTemp();
                    }
                    if( slen(newId) ) {
                        nodeObject->set("id", newId);
                    } else if(nodeObject->cmp("mode","base") ) {
                        nodeObject->GetVar("@source")->set(var, sp, pv.start);
                    }
                    arrWidgets->add()->setVar('n',0,(LPVOID)nodeObject);
                } else {

                }
            }
            //-------------------------------------- else tag
            if( ccmp(mode, "break") ) break;
        } else {
            qDebug("#9#baroSourceApply func not match: %s %s\n", tag, node->log() );
            break;
        }
    }
    if( callVar.length() ) {
        qDebug("baroSourceApply call (size:%d)\n", callVar.length() );
        cfVar("@inlineMode")->get();
        callScriptFunc(&callVar, 0, callVar.length() );
        // uom.startTimer();
    }
    if( arrWidgets && arr==NULL ) {
        for( int n=arrWidgets->size()-1; n>=0; n-- ) {
            StrVar* sv=arrWidgets->get(n);
            if( SVCHK('n',0) ) {
                TreeNode* cur=(TreeNode*)SVO;
                qDebug("initFunc call start (tag:%s)", cur->get("tag"));
                callInitFunc(cur);
                qDebug("initFunc call end");
            }
        }
    }
    return arrWidgets;
}
#endif

XListArr* baroSourceApply(StrVar* var, int sp, int ep, StrVar* rst, XListArr* arr) {
    if( var==NULL ) return NULL;
    if( ep<=0 ) ep=var->length();
    /*
    if( var->find("<#", sp, ep, FIND_SKIP)!=-1 ) {
        var=applySouceRemove(var, sp, ep, uom.getStrVar() );
        sp=0, ep=var->length();
    }
    */
    StrVar callVar, classId;
    TreeNode* funcInfo=cfNode("@funcInfo");
    TreeNode* node=getTreeNodeTemp();
    XParseVar pv(var, sp, ep);
    if(pv.StartWith("<?") ) {
        if( pv.match("<?", "?>", FIND_SKIP) ) {
            setFuncSource(pv.GetVar(), pv.prev, pv.cur, NULL);
        }
        return NULL;
    }
    LPCC tag=NULL;
    LPCC baseCode=funcInfo->get("baseCode");
    // classId == fileCode
    if( slen(baseCode) ) {
        int pos=IndexOf(baseCode,':');
        if(pos>0) {
            classId.set(baseCode,pos);
        } else {
            classId.set(baseCode);
        }
    }
    XListArr* arrWidgets=arr;
    char c=pv.ch();
    bool bInc=funcInfo ? funcInfo->getBool("@include"):false;
    for(int idx=0; idx<1024; idx++  ) {
        if( pv.ch()=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) break;
            c=pv.ch();
        }
        if( c!='<' ) break;
        sp=pv.start;
        tag=pv.incr().NextWord();
        pv.setPos(sp);
        if( slen(tag)==0 ) break;
        qDebug("#0#baroSourceApply tag: %s\n", tag  );
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            LPCC mode=NULL;
            if( ccmp(tag,"inline") || ccmp(tag,"event") || ccmp(tag,"text") || ccmp(tag,"sql") ) {
                continue;
            }
            node->reuse();
            node->set("tag", tag);
            if( ccmp(tag,"func") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                LPCC funcLine=node->get("type");
                if( slen(funcLine)==0 ) funcLine="common";
                setFuncSource(var, sp, pv.cur, funcLine);
            } else if( ccmp(tag,"conf") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                LPCC id=node->get("id");
                if( slen(id) ) {
                    TreeNode* cf=NULL;
                    StrVar* sv=confVar(NULL);
                    if( SVCHK('n',0) ) {
                        cf=(TreeNode*)SVO;
                        cf->GetVar(id)->set(var, sp, pv.cur);
                        LPCC includeFileName=funcInfo->get("includeFileName");
                        TreeNode* funcMap=slen(includeFileName)? getTreeNode("@inc","confs"): NULL;
                        if(funcMap) {
                            funcMap->set(id,includeFileName);
                        }
                    }
                }
            } else if( ccmp(tag,"define") ) {
                sp=parseProp(node, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceUpdate define parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( ccmp(mode, "skip") ) continue;
                setBaroDefine(var, sp, pv.cur, node);
            } else if( ccmp(tag,"call") ) {
                TreeNode* callNode=cfNode("@callVars", true);
                sp=parseProp(callNode, var, pv.prev,pv.cur );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                if( bInc || callNode->cmp("mode", "skip") ) continue;
                callVar.add(var, sp, pv.cur);
            } else if( ccmp(tag,"pages") ||  ccmp(tag,"widgets") ) {
                sp=parseProp(node, var, pv.prev,pv.cur, PROP_SOURCE );
                if( sp==-1 ) {
                    qDebug("#9#baroSourceApply parse error: %s %s\n", tag, node->log() );
                    return arrWidgets;
                }
                mode=node->get("mode");
                if( bInc || ccmp(mode, "skip") ) continue;
                if( arrWidgets==NULL ) {
                    arrWidgets=getListArrTemp();
                }
                LPCC newId=node->get("newId"), base=node->get("base");
                if( slen(newId) ) {
                    funcInfo->GetVar("newId")->set(newId);
                }
                if( slen(base) ) {
                    funcInfo->set("baseCode", base);
                }
                baroSourceApply(var,sp,pv.cur,rst,arrWidgets);
                if( slen(newId) ) {
                    funcInfo->GetVar("newId")->reuse();
                }
                if( slen(base) && !ccmp(baseCode,base)) {
                    funcInfo->set("baseCode", baseCode);
                }
            } else {
                parseProp(node, var, pv.prev, pv.cur, PROP_SOURCE );
                mode=node->get("mode");
                if( bInc || ccmp(mode, "skip") ) continue;
                LPCC id=NULL, newId=funcInfo->get("newId");
                if( slen(newId)==0 ) {
                    id=node->get("id");
                } else {
                    qDebug("new id==%s", newId);
                    id=newId;
                }
                TreeNode* nodeObject=baroWidgetApply(tag, id, var, pv.prev, pv.cur, classId.get());
                if( nodeObject ) {
                    if( slen(newId) ) {
                        nodeObject->set("id", newId);
                    } else if(nodeObject->cmp("mode","base") ) {
                        nodeObject->GetVar("@source")->set(var, sp, pv.start);
                    }
                    arrWidgets->add()->setVar('n',0,(LPVOID)nodeObject);
                } else {

                }
            }
            //-------------------------------------- else tag
            if( ccmp(mode, "break") ) break;
        } else {
            qDebug("#9#baroSourceApply func not match: %s %s\n", tag, node->log() );
            break;
        }
    }
    if( callVar.length() ) {
        qDebug("baroSourceApply call (size:%d)\n", callVar.length() );
        cfVar("@inlineMode")->reuse();
        callScriptFunc(&callVar, 0, callVar.length() );
        // uom.startTimer();
    }
    if( arrWidgets && arr==NULL ) {
        for( int n=arrWidgets->size()-1; n>=0; n-- ) {
            StrVar* sv=arrWidgets->get(n);
            LPCC pageBase = funcInfo->get("pageBase");
            if( SVCHK('n',0) ) {
                TreeNode* cur=(TreeNode*)SVO;
                LPCC tag = cur->get("tag");
                qDebug("============== initFunc call start (tag:%s, pageBase:%s) ====================\n", tag, pageBase);
                if( slen(pageBase)>0 ) {
                    callInitFunc(cur);
                }
                if(ccmp(tag,"page")) {
                    qDebug("============== initFunc call end (tag:%s) ====================\n", tag);
                }
            }
        }
    }
    return arrWidgets;
}

bool includeBaro(LPCC baseFileName) {
    LPCC fileName=getFullFileName(baseFileName);
    qDebug("[includeBaro] fileName:%s", fileName);
    QFileInfo fi(fileName);
    if( fi.isFile() ) {
        QFile file(KR(fileName));
        if( file.open(QIODevice::ReadOnly)) {
            QByteArray ba=file.readAll();
            qDebug("...includeBaro... %s ", fileName  );
            StrVar* sv=uom.getStrVar();
            sv->set(ba.constData(), ba.size() );
            baroSourceApply(sv, 0, sv->length());
            file.close();
            return true;
        } else {
            qDebug("#9#includeBaro file not found %s", fileName);
        }
    }
    return false;
}
