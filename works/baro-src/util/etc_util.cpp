#include "widget_util.h"
#ifdef FUNCNODE_USE
#include "../baroscript/func_util.h"
#endif
/*
int gRectsIdx=0;
QRectF*	gRects = new QRectF[16];
*/

bool printError(LPCC errorCode, LPCC defaultFmt, ... ) {
    StrVar* err=cfVar("@error", true);
    LPCC fmt=toString(confVar(err->get()));
    err->add("error.").add(errorCode);
    if( slen(fmt)==0 ) {
        fmt=err->add("=>").add(defaultFmt).get();
    }
    int len=0;
    char buf[2048];
    va_list ap;
    va_start(ap, defaultFmt);
    len=vsnprintf(buf, 2048, fmt, ap);
    va_end(ap);
    _LOG("#[error]: %s (size:%d)\n", buf, len);
    if( len>=2048 ) {
        len=2048;
    }
    err->set(buf, len);
    return false;
}

StrVar* setVarRect(StrVar* sv, const QRect& r ) {
    sv->setVar('i',4).addInt(r.x()).addInt(r.y()).addInt(r.width()).addInt(r.height());
    return sv;
}
StrVar* setVarRectF(StrVar* sv, const QRect& r ) {
    sv->setVar('i',5).addDouble(r.x()).addDouble(r.y()).addDouble(r.width()).addDouble(r.height());
    return sv;
}
StrVar* setVarRectF(StrVar* sv, const QRectF& r ) {
    sv->setVar('i',5).addDouble(r.x()).addDouble(r.y()).addDouble(r.width()).addDouble(r.height());
    return sv;
}

QRect setQRect( StrVar* sv) {
    int sz=sizeof(double);
    return  SVCHK('i',4) ? QRect(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16) ):
            SVCHK('i',5) ? QRect(sv->getDouble(4),sv->getDouble(4+sz),sv->getDouble(4+(2*sz)),sv->getDouble(4+(3*sz)) ):QRect();
}
QRectF getQRectF( StrVar* sv ) {
    /*
    if( gRectsIdx>=16 ) {
        gRectsIdx=0;
    }
    QRectF* rect = &gRects[gRectsIdx++];
    if( rect ) {
        if( SVCHK('i',5) ) {
            int sz=sizeof(double);
            rect->setRect(sv->getDouble(4),sv->getDouble(4+sz),sv->getDouble(4+(2*sz)),sv->getDouble(4+(3*sz)));
        } else if( SVCHK('i',4) ) {
            rect->setRect(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16) );
        }
        return *rect;
    }
    */
    if( SVCHK('i',5) ) {
        int sz=sizeof(double);        
        return QRectF(sv->getDouble(4),sv->getDouble(4+sz),sv->getDouble(4+(2*sz)),sv->getDouble(4+(3*sz)));
    } else if( SVCHK('i',4) ) {
        return QRectF(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16) );
    }
    return QRectF();
}

inline XListArr* setArrayComma(TreeNode* node, LPCC name, StrVar* var, int sp, int ep) {
    if( node==NULL ) return NULL;
    XParseVar sub(var, sp, ep);
    if( sub.ch() ) {
        XListArr* arr=node->addArray(name);
        if( arr ) {
            while( sub.valid() ) {
                arr->add()->set(sub.findEnd(",").v());
                sub.ch();
            }
            for( int n=0,sz=arr->size(); n<sz; n++ ) {
                StrVar* sv=arr->get(n);
                if( isNumberVar(sv) ) {
                    if( sv->find(".")!=-1 ) {
                        sv->setVar('4',0).addDouble(toDouble(sv));
                    } else if(sv->length()>6 ){
                        sv->setVar('1',0).addUL64(toUL64(sv));
                    } else {
                        sv->setVar('0',0).addInt(toInteger(sv));
                    }
                }
            }
        }
        return arr;
    }
    return NULL;
}

int parseProp( TreeNode* node, StrVar* var, int sp, int ep, U32 flags, StrVar* rst, XFuncNode* fnParent) {
    XParseVar pv(var,sp,ep);
    char ch = pv.ch();
    if( ch=='>' ) {
        pv.incr();
        return pv.start;
    }
    LPCC name=NULL;
    while( pv.valid() ) {
        ch=pv.ch();
        if( !ch ) break;
        ch = pv.ch();
        if( ch=='/' ) {
            if(pv.ch(1)=='*' ) {
                qDebug("... prop comment ---------------\n");
                pv.match("/*", "*/");
                continue;
            }
            ch=pv.incr().ch();
            if( ch!='>' ) return -1;
            // single tag
            if( node ) node->setNodeFlag(FLAG_SINGLE);
            ch=pv.incr().ch();
            break;
        } else if( ch=='>' || ch=='<' ) {
            if( ch=='>' ) {
                ch=pv.incr().ch();
            }
            break;
        }
        if( ch==',' ) {
            ch=pv.incr().ch();
        }
        sp=pv.start;
        if( ch=='@' ) {
            pv.incr();
        }
        ch=pv.moveNext().ch();
        while( ch=='-' || ch=='.' || ch=='@' || ch=='#' ) {
            ch=pv.incr().moveNext().ch();
        }
        if( ch=='(') {
            bool ok=false;
            int begin=sp;
            name=pv.Trim(sp, pv.start);            
            if( pv.MatchWordPos("(",")",FIND_SKIP) ) {
                ch=pv.ch();
                sp = pv.prev, ep = pv.cur;
                if( ch=='{' ) {
                    if( pv.MatchWordPos("{","}",FIND_SKIP) && slen(name) ) {
                        if( flags&(PROP_GETID|PROP_SOURCE) ) continue;
                        if( name[0]=='@' ) {
                            setFuncSource(var, begin, pv.start, "common");
                        } else {
                            setMemberSource(node, fnParent, var, begin, pv.start, "func");
                        }
                        ok=true;
                    }
                }
            }
            if( ok==false ) {
                _LOG("#9#page parseProp error(name:%s)", name);
                return -1;
            }
        } else {
            if( ch!='=' )
                break;
            name=pv.Trim(sp, pv.start);
            ch=pv.incr().ch();
            sp=pv.start;

            if( ch=='\'' || ch=='"' ) {
                if( pv.MatchStringPos(ch) ) {                    
                    if( flags&PROP_GETID && !ccmp(name,"id") ) {
                        continue;
                    }
                    if( node ) {
                        node->GetVar(name)->set(var, pv.prev, pv.cur);
                    }
                    if( flags&PROP_GETID ) return pv.start;
                }
            } else if( ch=='{' || ch=='[' ) {
                if( pv.ch(1)=='{' ) {
                    if( pv.match("{{","}}",FIND_SKIP) ) {
                        if( flags&(PROP_GETID|PROP_SOURCE) ) continue;
                        if( node ) node->GetVar(name)->set(var, pv.prev, pv.cur);
                    }
                } else {
                    if( pv.match("[","]",FIND_SKIP) ) {
                        if( flags&(PROP_GETID|PROP_SOURCE) ) continue;
                        if( node ) {
                            XParseVar sub(var,pv.prev,pv.cur);
                            XListArr* arr = node->addArray(name);
                            node->setArrayValue(arr,sub,NULL);
                        }
                    } else if( pv.match("{","}",FIND_SKIP) ) {
                        if( flags&(PROP_GETID|PROP_SOURCE) ) continue;
                        if( node ) {
                            TreeNode* cur=node->addNode(name);
                            XParseVar pvCur(var,pv.prev,pv.cur);
                            cur->setJson(pvCur );
                            // node->GetVar(name)->setVar('n',0,(LPVOID)cur);
                        }
                    }
                }
            } else {
                int ep = pv.find(">");
                if( ep==-1 )
                    ep=pv.wep;
                if( flags&(PROP_GETID|PROP_SOURCE) ) {
                    if( ccmp(name,"id") ) {
                        if( flags&PROP_SOURCE  ) continue;
                    } else {
                        if( flags&PROP_GETID  ) continue;
                    }
                }
                if( pv.findPos(" \n\t",ep,FIND_CHARS)!=-1 ) {
                    if( node ) node->GetVar(name)->set(pv.Trim(pv.prev, pv.cur));
                } else {                    
                    if( node ) node->GetVar(name)->set(pv.Trim(sp, ep));
                    pv.setPos(ep);
                }
                if( flags&PROP_GETID ) return pv.start;
            }
        }
    }
    return pv.start;
}

bool parsePage(WidgetConf* root, StrVar* var, int sp, int ep, WidgetConf* rootNode, int* idx ) {
    char ch=0;
    int pos=0, index=1;
    LPCC tag=NULL;
    WidgetConf* cur=NULL;
    XParseVar pv(var, sp, ep);
    if( idx==NULL ) idx=&index;
    while( pv.valid() ) {
        pv.findEnd("<");
        ch = pv.ch();
        if( ch==0 )
            break;
        if( ch=='/' || ch=='!' ) {
            if( ch=='/') {
                pv.findEnd(">");
            } else {
                pv.findEnd("-->");
            }
            continue;
        }
        sp = pv.cur;
        tag = pv.NextWord();
        if( slen(tag)==0 ) {
            printError("etc_util/parsePage01","tag not define:%s\n", tag);
            break;
        }
        cur = root->addWidgetConf();
        cur->set("tag",	tag);
        cur->setInt("index", (*idx)++ );
        if( rootNode && !cur->isset("id") ) {
            int num=rootNode->incrNum(FMT("@%s_index",tag));
            cur->set("id", FMT("%s_%d",tag, num));
        }
        pos = parseProp((TreeNode*)cur, pv.GetVar(), pv.start, pv.wep );
        if( pos==-1 ) {
            printError("etc_util/parsePage01","prop parse error tag:%s\n",tag);
            break;
        }
        if( ccmp(tag,"vbox") || ccmp(tag,"hbox") || ccmp(tag,"grid") ||
            ccmp(tag,"row") || ccmp(tag,"xml") || ccmp(tag,"canvas") || ccmp(tag,"splitter") || ccmp(tag,"div") || ccmp(tag,"tab") || ccmp(tag,"group") )
        {
            pv.setPos(sp);
            if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
                if( ccmp(tag,"xml") ) {
                    parseXml((TreeNode*)cur, pv.GetVar(), pos, pv.cur);
                } else if( ccmp(tag,"canvas") ) {
                    cur->GetVar("@layout")->set(pv.GetVar(), pos, pv.cur);
                } else {
                    parsePage(cur, pv.GetVar(), pos, pv.cur, rootNode, idx);
                }
            } else {
                printError("etc_util/parsePage02","tag match error (tag:%s)\n",tag);
                break;
            }
        } else {
            _LOG("etc_util/parserPage single tag: %s",tag);
        }
    }
    return true;
}
bool parseXmlUpdate(TreeNode *root, StrVar *var, int sp, int ep, StrVar* rst, XFuncNode* fnParent) {
    if( root==NULL || var==NULL ) return false;
    if( rst==NULL ) rst=getStrVarTemp();
    XParseVar pv(var, sp, ep);
    char ch=0;
    LPCC tag=NULL;
    TreeNode* node=NULL;
    int idx=0, end= ep<=0? var->length(): ep;
    while( pv.valid() ) {
        ch=pv.ch();
        if( ch==0 ) break;
        if( ch=='>' ) {
            ch=pv.incr().ch();
        }
        if( ch!='<' ) {
            printError("etc_util/parseXml01","parse xml start error (char:%c)\n",ch);
            break;
        }
        sp = pv.start;
        ch=pv.incr().ch();
        if( ch=='/' || ch=='!' ) {
            if( ch=='/') {
                pv.findEnd(">");
            } else {
                pv.findEnd("-->");
            }
            continue;
        }
        ch=pv.moveNext().ch();
        while( ch=='-' ) {
            ch=pv.moveNext().ch();
        }
        tag=pv.Trim(sp+1, pv.start );
        if( slen(tag)==0 ) {
            printError("etc_util/parseXml01","parse xml tag:%s\n",tag);
            break;
        }
        if( !pv.valid() ) break;
        node=root->child(idx++);
        if( node && node->cmp("tag",tag) ) {
            ep=parseProp(node, var, pv.prev, end, PROP_SOURCE, rst->reuse(), fnParent );
            if( rst->length() ) {
                setMemberSource(root, fnParent, rst, 0, rst->length(), "event");
            }
            if( ep==-1 ) {
                printError("etc_util/parseXml02","parse prop error (tag:%s)\n",tag);
                return false;
            }
        } else {
            _LOG("#9#update xml node not match (tag:%s)",tag);
            return false;
        }

        _LOG("xxxxx xml update tag:%s xxxxxx, (ch:%c)", tag, ch );
        pv.setPos(sp);
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            parseXmlUpdate(node, var, ep, pv.cur, rst, fnParent);
        } else {
            _LOG("#9#xxxxx update xml error tag:%s xxxxxx, (ch:%c) end:%s\n", tag, pv.ch(), FMT("</%s>",tag) );
            break;
        }
    }
    return true;
}

bool parseXml(TreeNode *root, StrVar *var, int sp, int ep, XFuncNode* fnParent) {
    if( root==NULL || var==NULL ) return false;
    XParseVar pv(var, sp, ep);
    char ch=pv.ch();
    if( ch!='<' ) {
        root->GetVar("data")->set(var, sp, ep);
        return true;
    }
    LPCC tag=NULL;
    TreeNode* node=NULL;
    WidgetConf* ref=NULL;
    if( root->cmp("@objectType", "WidgetConf") ) {
        ref=static_cast<WidgetConf*>(root);
    }
    int end=ep<=0? var->length(): ep;
    while( pv.valid() ) {
        ch=pv.ch();
        if( ch==0 ) break;
        if( ch=='>' ) {
            ch=pv.incr().ch();
        }
        if( ch!='<' ) {
            printError("etc_util/parseXml01","parse xml start error (char:%c)\n",ch);
            break;
        }
        sp = pv.start;
        ch=pv.incr().ch();
        if( ch=='/' || ch=='!' ) {
            if( ch=='/') {
                pv.findEnd(">");
            } else {
                pv.findEnd("-->");
            }
            continue;
        }
        ch=pv.moveNext().ch();
        while( ch=='-' ) {
            ch=pv.moveNext().ch();
        }
        tag=pv.Trim(sp+1, pv.start );
        if( slen(tag)==0 ) {
            printError("etc_util/parseXml01","parse xml tag:%s\n",tag);
            break;
        }
        if( !pv.valid() ) break;
        if( ref ) {
            WidgetConf* cur=ref->addWidgetConf();
            cur->xwidget=ref->xwidget;
            cur->xtype=ref->xtype;
            cur->set("tag",	tag);
            node=static_cast<TreeNode*>(cur);
            ep=parseProp(node, var, pv.prev, end, 0, NULL, fnParent );
        } else {
            node=root->addNode();
            node->set("tag",	tag);
            ep=parseProp(node, var, pv.prev, end, 0, NULL, fnParent );
        }
        if( ep==-1 ) {
            printError("etc_util/parseXml02","parse prop error (tag:%s)\n",tag);
            return false;
        }
        _LOG("xxxxx xml prop tag:%s xxxxxx, (ch:%c)", tag, ch );
        if( node->isNodeFlag(FLAG_SINGLE) ) {
            // single tag
            pv.setPos(ep);
            continue;
        }
        pv.setPos(sp);
        if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
            if( ccmp(tag,"widgets") ) {
                parsePageNode(node, var, ep, pv.cur);
            } else {
                parseXml(node, var, ep, pv.cur, fnParent);
            }
        } else {
            _LOG("#9#xxxxx error tag:%s xxxxxx, (ch:%c) end:%s\n", tag, pv.ch(), FMT("</%s>",tag) );
            return false;
        }
    }
    return true;
}

TreeNode* nodeJsonData(XParseVar& pv, TreeNode* root, StrVar* sv) {
    StrVar var;
    TreeNode* node = NULL;
    if( SVCHK('n',0) ) {
        node=(TreeNode*)SVO;
    } else {
        node=root->addNode();
        sv->setVar('n',0,(LPVOID)node);
    }

    char ch=pv.ch();
    if( ch=='[' ) {
        ch=pv.incr().ch();
    }
    if( ch=='{' ) {
        while( pv.valid() ) {
            // ex) <data> {}, {}</data>
            ch=pv.ch();
            if( ch!='{' ) break;
            sv=var.reuse();
            node->setJsonValue(pv,sv,ch);
            ch=pv.ch();
            if( ch==']' || ch=='}' ) break;
            if( ch==',' || ch==';' ) pv.incr();
        }
    } else {
        TreeNode* child=NULL;
        while( pv.valid() ) {
            int sp=pv.start;
            if( !pv.NextMove() ) break;
            ch=pv.ch();
            if( ch==':' ) {
                // ex) <data> code:value, code1: value1</data>
                LPCC code=pv.v();
                child=node->addNode();
                sv=var.reuse();
                ch=pv.incr().ch();
                node->setJsonValue(pv,sv,ch);
                child->set("code", code);
                child->set("value", toString(sv) );
            } else if( ch==',' ) {
                // ex) <data> val0, val1</data>
                LPCC code=pv.v();
                child=node->addNode();
                child->set("code", code);
                child->set("value", code );
            } else {
                // ex) <data> "val0", "val1", {code:c, value:v}</data>
                pv.start=sp;
                sv=var.reuse();
                ch=pv.ch();
                node->setJsonValue(pv,sv,ch);
                if( SVCHK('n',0) || SVCHK('a',0) ) {
                    if(SVCHK('a',0)) {
                        printError("etc_util/nodeJsonData01", "data set error :%s", toString(sv) );
                    }
                } else {
                    LPCC code=toString(sv);
                    child=node->addNode();
                    child->set("code", code);
                    child->set("value", code );
                }
            }
            ch=pv.ch();
            if( ch==',' || ch==';' ) pv.incr();
        }
    }
    return node;
}

