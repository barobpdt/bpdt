#include "func_util.h"
#include "tree_model.h"
#include "../util/user_event.h"
#include <QScrollBar>
#include <QHeaderView>

inline void regTreeFuncs() {
    if( isHashKey("tree") )
        return;
    PHashKey hash = getHashKey("tree");
    U16 uid = 1;
    hash->add("selectNodes", uid);	uid++;	// 1
    hash->add("selectMode", uid);	uid++;	// 2
    hash->add("selectClear", uid);	uid++;	// 3
    hash->add("selectAll", uid);		uid++;	// 4
    uid=11;
    hash->add("firstNode", uid);		uid++;	// 11
    hash->add("lastNode", uid);		uid++;
    hash->add("nextNode", uid);		uid++;
    hash->add("prevNode", uid);		uid++;
    hash->add("nodeRect", uid);		uid++;
    uid = 21;
    hash->add("field", uid);			uid++;
    hash->add("current", uid);		uid++;
    hash->add("rootNode", uid);		uid++;
    hash->add("reload", uid);		uid++;
    hash->add("edit", uid);			uid++;
    hash->add("changeRoot", uid);	uid++;
    hash->add("map", uid);			uid++;
    hash->add("at", uid);			uid++;
    uid = 31;
    hash->add("check", uid);			uid++;	//
    hash->add("expand", uid);		uid++;	//
    hash->add("indent", uid);		uid++;	//
    hash->add("header", uid);		uid++;	//
    hash->add("update", uid);		uid++;	//
    hash->add("sort", uid);			uid++;	//
    hash->add("invalid", uid);		uid++;	//
    uid = 41;
    hash->add("resize", uid);		uid++;	//
    hash->add("model", uid);			uid++;	//
    hash->add("fields", uid);		uid++;	//
    hash->add("scroll", uid);		uid++;	//
    hash->add("rowCount", uid);		uid++;	//
    hash->add("columnCount", uid);        uid++;	//
    hash->add("lineHeight", uid);	uid++;	//
    hash->add("headerHeight", uid);	uid++;	//
    hash->add("headerWidth", uid);	uid++;	//
    hash->add("isStyle", uid);		uid++;	//
    hash->add("viewSize", uid);		uid++;	//
    hash->add("scrollValue", uid);	uid++;	//
    uid=173;
    hash->add("is", uid);			uid++;	//
}

inline QModelIndex treeFindIndex(TreeModel* m, QModelIndex& parent, TreeNode* findNode, GTree* w=NULL, TreeProxyModel* proxy=NULL, bool expand=false ) {
    if( proxy ) {
        TreeNode* node=static_cast<TreeNode*>(parent.internalPointer());
        if( node==findNode ) {
            if( w && expand ) {
                QModelIndex index = proxy->mapFromSource(parent);
                if( index.isValid() ) {
                    w->setExpanded(index, expand);
                }
            }
            return parent;
        }
    }
    for( int n=0, cnt=m->rowCount(parent);n<cnt;n++ ) {
        QModelIndex midx=m->index(n,0,parent);
        TreeNode* node=static_cast<TreeNode*>(midx.internalPointer());
        if( node==findNode ) {
            return midx;
        }
        if( m->rowCount(midx) ) {
            QModelIndex find=treeFindIndex(m, midx, findNode, w, w?proxy:NULL, expand );
            if( find.isValid() ) {
                if( w && expand ) {
                    QModelIndex index = proxy->mapFromSource(midx);
                    if( index.isValid() ) {
                        w->setExpanded(index, expand);
                    }
                }
                return find;
            }
        }
    }
    return QModelIndex();
}

inline TreeModel* setTreeModel(TreeNode* cf, bool reset ) {
    // model: {id:xx, fields:[]}
    TreeModel* model=NULL;
    StrVar* sv=cf->GetVar("@model");
    if( SVCHK('m',0) ) {
        model=(TreeModel*)SVO;
    } else {
        model=new TreeModel(cf);
        sv->setVar('m',0,(LPVOID)model);
    }
    if( model->xroot==NULL ) {
        model->setRootNode();
    }
    return model;
}
#ifdef QT5_USE
inline bool resizeTree(GTree* w, TreeProxyModel* proxy) {
    TreeNode* fields=proxy->xfields;
    if( fields==NULL )
        return false;
    int size=fields->childCount();
    for( int n=0;n<size; n++ ) {
        TreeNode* node=fields->child(n);
        if( node==NULL ) break;
        StrVar* sv= node->gv("width");
        if( sv==NULL ) {
            // w->header()->setSectionResizeMode(n, QHeaderView::Stretch);
            continue;
        }
        int pos= sv->findPos("px");
        if( pos>0 ) {
            LPCC val=sv->get(0,pos);
            int wid=isnum(val) ? atoi(val): 2;
            w->header()->setSectionResizeMode(n, QHeaderView::Fixed);
            w->header()->resizeSection(n, wid );
        } else if( isNumberVar(sv) ) {
            QHeaderView::ResizeMode mode=w->header()->sectionResizeMode(n);
            if( mode != QHeaderView::Interactive  ) {
                w->header()->setSectionResizeMode(n, QHeaderView::Interactive);
            }
            w->header()->resizeSection(n, toInteger(sv) );
        } else {
            LPCC ws=toString(sv);
            if( ccmp(ws,"flex") || ccmp(ws,"stretch") ) {
                 w->header()->setSectionResizeMode(n, QHeaderView::Stretch);
            } else if( ccmp(ws,"fixed") ) {
                 w->header()->setSectionResizeMode(n, QHeaderView::Fixed);
            } else if( ccmp(ws,"resize") ) {
                w->header()->setSectionResizeMode(n, QHeaderView::ResizeToContents);
            } else if( ccmp(ws,"interactive") ) {
                 w->header()->setSectionResizeMode(n, QHeaderView::Interactive);
            }
        }
    }
    /*
    if( size ) {
        proxy->invalidate();
    }
    */
    return true;
}
#else
inline bool resizeTree(GTree* w, TreeProxyModel* proxy) {
    if( proxy->xfields==NULL )
        return false;
    int size=proxy->columnCount();
    for( int n=0;n<size; n++ ) {
        TreeNode* node=proxy->getHeaderNode(n);
        if( node==NULL ) break;
        StrVar* sv= node->gv("width");
        if( sv==NULL ) {
            w->header()->setResizeMode(n, QHeaderView::Custom);
            continue;
        }
        if( isNumberVar(sv) ) {
            QHeaderView::ResizeMode mode=w->header()->resizeMode(n);
            if( mode != QHeaderView::Interactive  ) {
                w->header()->setResizeMode(n, QHeaderView::Interactive);
            }
            w->header()->resizeSection(n, toInteger(sv) );
        } else {
            LPCC ws=toString(sv);
            if( ccmp(ws,"flex") ) {
                 w->header()->setResizeMode(n, QHeaderView::Stretch);
            } else if( ccmp(ws,"resize") ) {
                 w->header()->setResizeMode(n, QHeaderView::ResizeToContents);
            } else if( ccmp(ws,"interactive") ) {
                 w->header()->setResizeMode(n, QHeaderView::Interactive);
            }
        }
    }
    return true;
}
#endif
bool callTreeFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    if( widget==NULL )
        widget=gwidget(tn);
    GTree* w=qobject_cast<GTree*>(widget);
    if( w==NULL )
        return false;
    StrVar* sv=NULL;

    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        regTreeFuncs();
        ref=getHashKeyVal("tree", fnm);
        if( fc ) fc->xfuncRef = ref;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: {		// selectNodes
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) && w->selectionModel() ) {
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;
            XListArr* a = getListArrTemp();
            QList<QModelIndex> list = w->selectionModel()->selectedRows();
            int size = list.size();
            if( size==0 ) {
                QModelIndex index = w->selectionModel()->currentIndex();
                a->add()->setVar('n',0,proxy->mapToSource(index).internalPointer());
            } else {
                for( int n=0; n<size; n++) {
                    a->add()->setVar('n',0,proxy->mapToSource(list[n]).internalPointer());
                }
            }
            list.clear();
            rst->setVar('a',0,(LPVOID)a);
        }
    } break;
    case 2: {		// selectMode
        int num = cnt>0 ? toInteger(arrs->get(0)): 0;
        w->setSelectionMode(num? (QAbstractItemView::SelectionMode)num: QAbstractItemView::SingleSelection);
        num = cnt>1 ? toInteger(arrs->get(1)): 0;
        w->setSelectionBehavior( num? (QAbstractItemView::SelectionBehavior)num: QAbstractItemView::SelectRows);
    } break;
    case 3: {		// selectClear
        QItemSelectionModel *selection = w->selectionModel();
        if( selection ) {
            //  seleccion->clear();
            selection->clearSelection();
        }
        w->clearSelection();
    } break;
    case 4: {		// selectAll
        w->selectAll();
    } break;
    case 11:
    case 12:	{		// firstNode, lastNode
        if( cnt==0 ) return false;
        sv=tn->gv("@model");
        if( !SVCHK('m',0) )
            return true;
        TreeModel* m=(TreeModel*)SVO;
        TreeNode* cur = NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            cur=(TreeNode*)SVO;
        }
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;
            QModelIndex idx=m->index(0,0);
            if( cur ) {
                QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, idx, cur, NULL, proxy) : m->getIndex(cur);
                if( src.isValid() ) {
                    QModelIndex parent = proxy->mapFromSource(src);
                    idx = proxy->index( 0, parent.column(), parent);
                }
            } else {
                idx = proxy->index(0,0);
            }
            if( idx.isValid() ) {
                if( ref==12 ) {
                    int rows = proxy->rowCount(idx.parent());
                    idx = proxy->index(rows-1,0);
                }
                TreeNode* node = (TreeNode*)proxy->mapToSource(idx).internalPointer();
                if( node ) rst->setVar('n',0,(LPVOID)node);
            }
        }
    } break;
    case 13:
    case 14: {		// nextNode, prevNode
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( !SVCHK('n',0) )
            return false;
        TreeNode* cur = (TreeNode*)SVO;
        sv=tn->gv("@model");
        if( SVCHK('m',0) ) {
            TreeModel* m=(TreeModel*)SVO;
            QModelIndex src = m->getIndex(cur);
            sv=tn->gv("@proxy");
            if( src.isValid() && SVCHK('m',1) ) {
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                QModelIndex idx = proxy->mapFromSource(src);
                if( idx.isValid() ) {
                    QModelIndex next = proxy->index(
                        ref==13 ? idx.row()+1:
                        ref==14 ? idx.row()-1: 0, idx.column(), idx.parent());
                    TreeNode* node = next.isValid() ? (TreeNode*)proxy->mapToSource(next).internalPointer(): NULL;
                    if( node ) {
                        rst->setVar('n',0,(LPVOID)node);
                        LPCC code = cur->get("@code");
                        if( slen(code) ) {
                            node->set("@code",code);
                        }
                    } else
                        rst->setVar('3',0);
                }
            }
        }
    } break;
    case 15: {		// nodeRect
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( !SVCHK('n',0) ) return false;
        TreeNode* cur = (TreeNode*)SVO;
        sv=tn->gv("@model");
        if( SVCHK('m',0) ) {
            TreeModel* m=(TreeModel*)SVO;
            TreeProxyModel* proxy=NULL;
            sv=tn->gv("@proxy");
            if( SVCHK('m',1) ) {
                proxy=(TreeProxyModel*)SVO;
            }
            QModelIndex rootIndex=m->index(0,0);
            QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, cur, NULL, proxy): m->getIndex(cur);
            if( src.isValid() && proxy ) {
                int c=0;
                if( isNumberVar(arrs->get(1)) ) {
                    c = toInteger(arrs->get(1));
                } else if( cnt==2 ) {
                    c=proxy->getMapIndex(toString(arrs->get(1)));
                } else {
                    LPCC code = cur->get("@code");
                    if( slen(code) ) {
                        c=proxy->getMapIndex(code);
                    }
                }
                QModelIndex idx = proxy->mapFromSource(src.sibling(src.row(),c));
                if( idx.isValid() ) {
                    QRect r = w->visualRect(idx);
                    rst->setVar('i',5).addDouble(r.x()).addDouble(r.y()).addDouble(r.width()).addDouble(r.height());
                }
            }
        }
    } break;
    case 21: {		// field
        if( cnt==0 )
            return false;
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int idx=toInteger(sv);
            sv=tn->gv("@proxy");								if( !SVCHK('m',1) ) return false;
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return false;
            sv= NULL;
            if( idx<proxy->xfields->childCount() ) {
                TreeNode* cur = proxy->xfields->child(idx);
                if( cur ) {
                    sv = cur->gv("field");
                    if( sv==NULL ) sv = cur->gv("text");
                }
            }
            rst->reuse()->add(sv);
        } else {
            sv=tn->gv("@proxy");								if( SVCHK('m',1)==false ) return false;
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return false;
            LPCC code = AS(0);
            rst->setVar('3',(U16)proxy->getMapIndex(code));
        }
    } break;
    case 22: {		// current
        rst->reuse();
        if( cnt > 0 ) {
            sv = arrs->get(0);
            if( !SVCHK('n',0) ) {
                return true;
            }
            TreeNode* node=(TreeNode*)SVO;
            sv=tn->gv("@model");
            if( SVCHK('m',0) ) {
                TreeModel* m=(TreeModel*)SVO;
                TreeProxyModel* proxy=NULL;
                sv=tn->gv("@proxy");
                if( SVCHK('m',1) ) {
                    proxy=(TreeProxyModel*)SVO;
                }
                QModelIndex rootIndex=m->index(0,0);
                QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, node, NULL, proxy): m->getIndex(node);
                if( src.isValid() && proxy ) {
                    QModelIndex index = proxy->mapFromSource(src);
                    if( index.isValid() ) {
                        w->scrollTo(index);
                        w->setCurrentIndex(index);
                    }
                }
            }
        } else {
            QItemSelectionModel* im=w->selectionModel();
            if( im==NULL ) {
                rst->setVar('3',0);
                return true;
            }
            QModelIndex index = im->currentIndex();
            sv=tn->gv("@proxy");
            if( index.isValid() && SVCHK('m',1) ) {
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                rst->setVar('n',0,(LPVOID)static_cast<TreeNode*>(proxy->mapToSource(index).internalPointer()));
            }
        }
    } break;
    case 23: {		// rootNode
        // ### version 1.0.4
        TreeNode* root=NULL;
        TreeProxyModel* proxy=NULL;
        if( arrs ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                root=(TreeNode*)SVO;
            }
        }
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            proxy=(TreeProxyModel*)SVO;
        }
        sv=tn->gv("@model");
        if( SVCHK('m',0) ) {
            TreeModel* m=(TreeModel*)SVO;
            if( root ) {
                m->setRootNode(root);
                if( proxy ) proxy->invalidate();
            }
            if( m->getRootNode() ) {
                rst->setVar('n',0,(LPVOID)m->getRootNode() );
            }
        }
    } break;
    case 24: {		// reload
        sv=tn->gv("@model");
        if( SVCHK('m',0) ) {
            TreeModel* m=(TreeModel*)SVO;
            sv=tn->gv("@proxy");
            if( SVCHK('m',1) ) {
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                if( cnt==0 ) {
                    TreeNode* root = m->xroot;
                    root->reset();
                    root->clearNodeFlag(NF_CALLBACK);
                } else {
                    TreeNode* cur = NULL;
                    QItemSelectionModel* im=w->selectionModel();
                    if( im==NULL ) {
                        rst->reuse();
                        return false;
                    }
                    QModelIndex index = im->currentIndex();
                    if( index.isValid() ) {
                        cur = static_cast<TreeNode*>(proxy->mapToSource(index).internalPointer());
                    }
                    if( cur ) {
                        cur->reset();
                        cur->clearNodeFlag(NF_CALLBACK);
                    }
                }
                if( proxy ) proxy->invalidate();
            }
        }
    } break;
    case 25: {		// edit
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* node = (TreeNode*)arrs->get(0)->getObject(FUNC_HEADER_POS);
            sv=tn->gv("@model");
            if( SVCHK('m',0) ) {
                TreeModel* m=(TreeModel*)SVO;
                TreeProxyModel* proxy=NULL;
                sv=tn->gv("@proxy");
                if( SVCHK('m',1) ) {
                    proxy=(TreeProxyModel*)SVO;
                }
                QModelIndex rootIndex=m->index(0,0);
                QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, node, NULL, proxy) :m->getIndex(node);
                if( src.isValid() && proxy ) {
                    int c = 0;
                    // version 1.0.3
                    sv=arrs->get(1);
                    if( isNumberVar(sv) ) {
                        c=toInteger(sv);
                    } else {
                        LPCC code=toString(sv);
                        c=proxy->getMapIndex(code);
                    }
                    QModelIndex index = proxy->mapFromSource(src.sibling(src.row(),c));
                    if( index.isValid() ) {
                        proxy->xfilterFlag |= 0x100;
                        w->edit(index);
                        /*XX
                        if( cnt==1 ) {
                            w->edit(index);
                        } else {
                            int idx = isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): proxy->getMapIndex(toString(arrs->get(0)));
                            if( idx!=-1 ) w->edit(index.sibling(index.row(),idx));
                        }
                        */
                    }
                }
            }
        }
    } break;
    case 26: {		// changeRoot         
        sv=tn->gv("@model");
        if( SVCHK('m',0) ) {
            TreeModel* m=(TreeModel*)SVO;
            TreeProxyModel* proxy=NULL;
            sv=tn->gv("@proxy");
            if( SVCHK('m',1) ) {
                proxy=(TreeProxyModel*)SVO;
                // m->xroot=cur;
                if( proxy->xfilterFlag&FILTER_NOCHILD ) {
                    proxy->xfilterFlag&=~FILTER_NOCHILD;
                }
            }
            if( proxy==NULL ) {
                qDebug("#9#[chagenRoot] proxy error");
                return true;
            }
            TreeNode* cur=NULL;
            QModelIndex rootIndex=m->index(0,0);
            if( arrs ) {
                sv=arrs->get(0);
                if( SVCHK('n',0) ) {
                    cur=(TreeNode*)SVO;
                }
            } else {
                cur=m->getRootNode();
            }
            if( cur ) {
                QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, cur, NULL, proxy): m->getIndex(cur);
                if( src.isValid() ) {
                    // proxy->setRootModel(m);
                    QModelIndex index = proxy->mapFromSource(src);
                    if( index.isValid() ) {
                        proxy->xrootNode = cur;
                        if( isVarTrue(arrs->get(1)) ) {
                            proxy->xfilterFlag|=FILTER_NOCHILD;
                        } else {
                            proxy->xfilterFlag&=~FILTER_NOCHILD;
                        }
                        w->setRootIndex(index); // (index);
                        proxy->invalidate();
                    } else {
                        qDebug("#9#[chagenRoot] index not valid");
                    }
                } else {
                    if( arrs==NULL ) {
                        QModelIndex index = proxy->mapFromSource(rootIndex);
                        proxy->xrootNode = m->getRootNode();
                        if( index.isValid() ) {
                            w->setRootIndex(index);
                        } else {
                            qDebug("#9#[chagenRoot] root index not valid");
                        }
                    } else {
                        qDebug("#9#[chagenRoot] source index not valid");
                    }
                }
            }
        }
    } break;
    case 27: {		// map
        if( cnt==0 )
            return false;
        LPCC ty= cnt==1 ? "field": AS(0);
        rst->reuse();
        if( ccmp(ty,"field") ) {
            sv=cnt==1 ? arrs->get(0): arrs->get(1);
            if( isNumberVar(sv) ) {
                int idx=toInteger(sv);
                sv=tn->gv("@proxy");								if( !SVCHK('m',1) ) return true;
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return true;
                sv= idx<proxy->xfields->childCount() ? proxy->xfields->child(idx)->gv("field") : NULL;
                rst->add(sv);
            }
        }
    } break;
    case 28: {		// at
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        rst->setVar('3',0);
        int h=w->header()->height(), x=0, y=0;
        if( SVCHK('i',2) ) {
            x=sv->getInt(4), y=sv->getInt(8);
        } else if(SVCHK('i',20) ) {
            x=(int)sv->getDouble(4), y=(int)sv->getDouble(4+sizeof(double));
        }
        QModelIndex index = w->indexAt(QPoint(x,y-h));
        sv=tn->gv("@proxy");
        if( index.isValid() && SVCHK('m',1) ) {
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;
            TreeNode* node = static_cast<TreeNode*>(proxy->mapToSource(index).internalPointer());
            LPCC code = proxy->getMapCode(index.column());
            if( slen(code) && node ) {
                node->GetVar("@code")->set(code);
            }
            rst->setVar('n',0,(LPVOID)node);
        }
    } break;
    case 32: {		// expand
        TreeNode* node = NULL;
        if( cnt ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                node = (TreeNode*)SVO;
            }
        }
        int idx=2;
        bool chk = true;
        rst->reuse();
        if( node==NULL ) {
            QItemSelectionModel* im=w->selectionModel();
            if( im==NULL ) return true;
            QModelIndex index = im->currentIndex();
            sv=tn->gv("@proxy");
            if( index.isValid() && SVCHK('m',1) ) {
                TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                if( cnt==0 ) {
                    rst->setVar('3',w->isExpanded(index)?1:0);
                    return true;
                } else {
                    node = static_cast<TreeNode*>(proxy->mapToSource(index).internalPointer());
                    chk = isVarTrue(arrs->get(0));
                    idx = 1;
                }
            }
        } else if( cnt>1) {
            chk = isVarTrue(arrs->get(1));
        }
        sv=tn->gv("@model");
        if( SVCHK('m',0) && node ) {
            TreeModel* m=(TreeModel*)SVO;
            TreeProxyModel* proxy=NULL;
            sv=tn->gv("@proxy");
            if( SVCHK('m',1) ) {
                proxy=(TreeProxyModel*)SVO;
            }
            QModelIndex rootIndex=m->index(0,0);
            QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1) ? treeFindIndex(m, rootIndex, node, w, proxy, chk): m->getIndex(node);
            if( src.isValid() && proxy ) {
                QModelIndex index = proxy->mapFromSource(src);
                if( index.isValid() ) {
                    w->setExpanded(index, chk);
                    sv = arrs->get(idx);
                    if( sv ) {
                        w->scrollTo(index);
                        w->setCurrentIndex(index);
                    }
                }
            }
        }
    } break;
    case 33: {		// indent
        int num = cnt==1 ? toInteger(arrs->get(0)): 0;
        w->setIndentation(num);
        w->setAnimated(false);
        w->setAttribute(Qt::WA_StyledBackground);
        w->setFocusPolicy(Qt::StrongFocus);
    } break;
    case 34: {		// header
    } break;
    case 35: {		// update
        TreeProxyModel* proxy=NULL;
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            bool sort=w->isSortingEnabled();
            proxy=(TreeProxyModel*)SVO;
            if(sort) w->setSortingEnabled(false);
            if( proxy ) {
                if( arrs ) {
                    bool clear=false, resize=false;
                    if( checkFuncObject(arrs->get(0),'3',1) ) clear=true;
                    if( checkFuncObject(arrs->get(1),'3',1) ) resize=true;

                    if( clear) {
                        QItemSelectionModel *sel = w->selectionModel();
                        if( sel ) sel->clear();
                        w->clearSelection();
                    }
                    if( resize) {
                        resizeTree(w, proxy);
                    }
                }
                proxy->invalidate();
            }
            if(sort) w->setSortingEnabled(true);
        }
    } break;
    case 36: {		// sort
        // ### version 1.0.4
        if( cnt==0 ) {
            w->setSortingEnabled(true);
            if( w->header()->sortIndicatorOrder()==Qt::AscendingOrder ) {
                rst->set("asc");
            } else {
                rst->set("desc");
            }
            // w->sortByColumn(0, Qt::AscendingOrder);
        } else {
            int idx = -1;
            if( isNumberVar(arrs->get(0)) ) {
                idx = toInteger(arrs->get(0));
            } else {
                sv=tn->gv("@proxy");
                if( SVCHK('m',1) ) {
                    TreeProxyModel* proxy=(TreeProxyModel*)SVO;
                    LPCC field=AS(0);
                    idx = proxy->getMapIndex(field);
                }
            }
            if( idx!=-1 ) {
                LPCC sort=AS(1);
                w->setSortingEnabled(true);
                if( slen(sort)==0 ) {
                    sort="asc";
                }
                w->sortByColumn(idx, ccmp(sort,"asc") ? Qt::AscendingOrder:  Qt::DescendingOrder );
            }
        }
    } break;
    case 41: {		// resize
        rst->reuse();
        sv=tn->gv("@proxy");								if( SVCHK('m',1)==false ) return true;
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return true;
        if( arrs ) {
            int sp=0,ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp, &ep);
            proxy->setFieldMap(sv, sp, ep);
        }
        resizeTree(w, proxy);
        rst->setVar('3',1);
    } break;
    case 42: {		// model
        if( cnt==0 ) {
            sv=tn->gv("@model");
            if( SVCHK('m',0) ) {
                TreeModel* tm = (TreeModel*)SVO;
                TreeNode* root=tm->setRootNode();
                if( root ) {
                    rst->setVar('n', 0, (LPVOID)root);
                }
            }
            return true;
        }

        /*
        TreeModel* tm=NULL;
        if( SVCHK('m',0) ) {
            tm = (TreeModel*)SVO;
        } else if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            sv=node->gv("@model");
            if( SVCHK('m',0) ) {
                tm=(TreeModel*)SVO;
                if( !getEventFuncNode(tm->xcf, "onChildData") ) {
                    tm->xcf=tn;
                }
            } else {
                tm=new TreeModel(tn);
                tm->setRootNode(node);
                node->GetVar("@model")->setVar('m',0,(LPVOID)tm);
            }
        } else if(sv) {
            rst->add(toString(sv));
        }
        if( tm ) {
            sv = arrs->get(1);
            if( sv ) {
                rst->add(toString(sv));
            } else {
                TreeNode* root=tm->getRootNode();
                sv=root->gv("@fields");
                if( SVCHK('a',0) ) {
                    XListArr* a=(XListArr*)SVO;
                    for( int n=0,sz=a->size(); n<sz; n++ ) {
                        if( n>0 ) rst->add(',');
                        rst->add( toString(a->get(n)) );
                    }
                }
            }
            tn->GetVar("@model")->setVar('m',0,(LPVOID)tm);
        }
        */

        int sp=0, ep=0;
        sv= getStrVarPointer(arrs->get(0), &sp, &ep);
        if(sp<ep) {
            StrVar* fields=tn->GetVar("@fields")->reuse();
            fields->add(sv,sp,ep);
        }
        setModel(w, tn);
        /*
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            TreeProxyModel* p=(TreeProxyModel*)SVO;
            resizeTree(w, p);
        }
        */
    } break;
    case 43: {		// fields
        sv=tn->gv("@proxy");
        rst->reuse();
        if( !SVCHK('m',1) ) {
            return true;
        }
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;
        if( arrs ) {
            int sp=0,ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp, &ep);
            proxy->setFieldMap(sv, sp, ep);
            if( proxy->xfields && proxy->xfields->childCount() >0 ) {
                // proxy->invalidate();
                // resizeTree(w, proxy);
            }
        }
        if( proxy->xfields ) {
            rst->setVar('n',0,(LPVOID)proxy->xfields);
        }
    } break;
    case 44: 		// scroll
    case 45: {		// rowCount
        sv=tn->gv("@model");
        rst->reuse();
        if( !SVCHK('m',0) ) {
            return true;
        }
        TreeModel* m=(TreeModel*)SVO;
        TreeNode* cur = NULL;
        sv = arrs ? arrs->get(0): NULL;
        if( SVCHK('n',0) ) {
            cur = (TreeNode*)SVO;
        }
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;
            if( ref==44 ) {
                if( cur ) {
                    int col =  isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): 0;
                    QModelIndex src = m->getIndex(cur, col);
                    if( src.isValid() ) {
                        QModelIndex index = proxy->mapFromSource(src);
                        if( index.isValid() ) w->scrollTo(index);
                    }
                }
            } else {
                if( cur ) {
                    /*
                    int col =  isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): 0;
                    QModelIndex src = m->getIndex(cur, col);
                    */
                    QModelIndex rootIndex=m->index(0,0);
                    QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1) ? treeFindIndex(m, rootIndex , cur, w, proxy): m->getIndex(cur);
                    if( src.isValid() ) {
                        QModelIndex index = proxy->mapFromSource(src);
                        if( index.isValid() ) {
                            rst->setVar('0',0).addInt( proxy->rowCount(index) );
                        } else {
                            rst->setVar('0',0).addInt(0);
                        }
                    }
                } else {
                    QModelIndex idx = proxy->index(0,0);
                    if( idx.isValid() ) {
                        rst->setVar('0',0).addInt( proxy->rowCount(idx.parent()) );
                    } else {
                        rst->setVar('0',0).addInt(0);
                    }
                }
            }
        }
    } break;
    case 46: {		// columnCount
        sv=tn->gv("@proxy");
        rst->reuse();
        if( !SVCHK('m',1) ) {
            return true;
        }
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;
        TreeNode* fields=proxy->xfields;
        rst->setVar('0',0).addInt(fields?fields->childCount():0);
    } break;
    case 47: {		// lineHeight
        sv=tn->gv("@proxy");
        if( SVCHK('m',1) ) {
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;
            if( arrs==NULL ) {
                rst->setVar('0',0).addInt(proxy->xlineHeight);
            } else if( isNumberVar(arrs->get(0)) ) {
                proxy->xlineHeight = toInteger(arrs->get(0));
            }
        }
    } break;
    case 48: {		// headerHeight
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(w->header()->height());
        } else {
            sv=arrs->get(0);
            if( isNumberVar(sv)) {
                TreeProxyModel* proxy=NULL;
                tn->setInt("@headerHeight", toInteger(sv));
                sv=tn->gv("@proxy");
                if( SVCHK('m',1) ) {
                    proxy=(TreeProxyModel*)SVO;
                    if( proxy ) {
                        proxy->invalidate();
                    }
                }
                w->header()->update();
            }
        }
    } break;
    case 49: {		// headerWidth
        if( cnt==0L ) return false;
        rst->reuse();
        sv=tn->gv("@proxy");								if( SVCHK('m',1)==false ) return true;
        TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return true;
        sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            for( int n=0, sz=a->size(); n<sz; n++ ) {
                sv=a->get(n);
                if( isNumberVar(sv) ) {
                    w->header()->resizeSection(n, toInteger(sv) );
                }
            }
        } else {
            int idx=-1;
            if( isNumberVar(sv) ) {
                idx=toInteger(sv);
            } else {
                LPCC field = AS(0);
                if( slen(field) ) idx = proxy->getMapIndex(field);
            }
            if( idx>=0 ) {
                if(sv) {
                    sv=arrs->get(1);
                    if( isNumberVar(sv) ) {
                        w->header()->resizeSection(idx, toInteger(sv) );
                        sv=arrs->get(2);
                    }
                    if(sv) {
                        LPCC ws=toString(sv);
                        if( ccmp(ws,"flex") || ccmp(ws,"stretch") ) {
                             w->header()->setSectionResizeMode(idx, QHeaderView::Stretch);
                        } else if( ccmp(ws,"fixed") ) {
                             w->header()->setSectionResizeMode(idx, QHeaderView::Fixed);
                        } else if( ccmp(ws,"resize") ) {
                            w->header()->setSectionResizeMode(idx, QHeaderView::ResizeToContents);
                        } else if( ccmp(ws,"interactive") ) {
                             w->header()->setSectionResizeMode(idx, QHeaderView::Interactive);
                        }
                    }
                } else {
                    rst->setVar('0',0).addInt(w->header()->sectionSize(idx));
                }
            }
        }
    } break;
    case 50: {		// isStyle
    } break;
    case 51: {		// viewSize
        QSize sz=w->viewport()->rect().size();
        rst->setVar('i',2).addInt(sz.width()).addInt( sz.height() );
    } break;
    case 52: {		// scrollValue
        if( cnt==0 ) return true;
        LPCC type=AS(0), exec=AS(1);
        if( ccmp(type,"vertical") || ccmp(type,"v") ) {
            if( ccmp(exec,"show") ) {
                w->verticalScrollBar()->show();
            } else if( ccmp(exec, "hide")) {
                w->verticalScrollBar()->hide();
            } else if( ccmp(exec, "value")) {
                sv=arrs->get(2);
                if( isNumberVar(sv) ) {
                    w->verticalScrollBar()->setValue(toInteger(sv));
                } else if( sv ) {
                    if( ccmp(AS(2),"end") ) {
                        int max=w->verticalScrollBar()->maximum();
                        w->verticalScrollBar()->setSliderPosition(max);
                    }
                } else {
                    rst->setVar('0',0).addInt(w->verticalScrollBar()->value());
                }
            } else {
                rst->setVar('3', w->verticalScrollBar()->isVisible() ? 1: 0);
            }
        } else {
            if( ccmp(exec,"show") ) {
                w->horizontalScrollBar()->show();
            } else if( ccmp(exec, "hide")) {
                w->horizontalScrollBar()->hide();
            } else if( ccmp(exec, "value")) {
                sv=arrs->get(2);
                if( isNumberVar(sv) ) {
                    w->horizontalScrollBar()->setValue(toInteger(sv));
                } else if( sv ) {
                    if( ccmp(AS(2),"end") ) {
                        int max=w->horizontalScrollBar()->maximum();
                        w->horizontalScrollBar()->setSliderPosition(max);
                    }
                } else {
                    rst->setVar('0',0).addInt(w->horizontalScrollBar()->value());
                }
            } else {
                rst->setVar('3', w->horizontalScrollBar()->isVisible() ? 1: 0);
            }
        }
    } break;
    case 31:		// check
    case 173:		// is
    {
        if( cnt==0 ) {
            rst->set("expand, child, sortEnable, headerHide, doubleClickExpand, expandEnable, drag, drop, rootDecorated\n\
                     editTrigger, contextMenu, stretchLast, selection, resizeWidth, treeMode, vScrollVisible, hScrollVisible ");
            return false;
        }
        LPCC code=toString(arrs->get(0));
        if( isWidgetCheck(code, w, rst) ) return true;
        sv=arrs->get(1);
        if( ccmp(code,"expand") ) {
            if( SVCHK('n',0) ) {
                TreeNode* node = (TreeNode*)SVO;
                sv=tn->gv("@model");
                if( SVCHK('m',0) && node ) {
                    TreeModel* m=(TreeModel*)SVO;
                    TreeProxyModel* proxy=NULL;
                    sv=tn->gv("@proxy");
                    if( SVCHK('m',1) ) {
                        proxy=(TreeProxyModel*)SVO;
                    }
                    QModelIndex rootIndex=m->index(0,0);
                    QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, node, NULL, proxy): m->getIndex(node);
                    if( src.isValid() && proxy ) {
                        QModelIndex index = proxy->mapFromSource(src);
                        if( index.isValid() && w->isExpanded(index) ) {
                            rst->setVar('3',1);
                        }
                    }
                }
            }
        } else if( ccmp(code,"child") ) {
            if( SVCHK('n',0) ) {
                TreeNode* node = (TreeNode*)SVO;
                sv=tn->gv("@model");
                if( SVCHK('m',0) && node ) {
                    TreeModel* m=(TreeModel*)SVO;
                    TreeProxyModel* proxy=NULL;
                    sv=tn->gv("@proxy");
                    if( SVCHK('m',1) ) {
                        proxy=(TreeProxyModel*)SVO;
                    }
                    QModelIndex rootIndex=m->index(0,0);
                    QModelIndex src = checkFuncObject(tn->gv("@treeMode"),'3',1)? treeFindIndex(m, rootIndex, node, NULL, proxy): m->getIndex(node);
                    if( src.isValid() && proxy ) {
                        QModelIndex index = proxy->mapFromSource(src);
                        if( proxy->hasChildren(index) ) {
                            rst->setVar('3',1);
                        }
                    } else {
                        qDebug("#0#tree child not valid tag:%s", node->get("tag"));
                    }
                }
            }
        } else if( ccmp(code,"sortEnable") ) {
            if( sv ) {
                w->setSortingEnabled(isVarTrue(sv));
            } else {
                rst->setVar('3', w->isSortingEnabled() ? 1: 0 );
            }
        } else if( ccmp(code,"vScrollVisible") ) {
            QScrollBar* sb=w->verticalScrollBar();
            if( sv ) {
                if( isVarTrue(sv) ) {
                    sb->show();
                } else {
                    sb->hide();
                }
            } else {
                rst->setVar('3', sb->isVisible() ? 1: 0);
            }
        } else if( ccmp(code,"hScrollVisible") ) {
            QScrollBar* sb=w->horizontalScrollBar();
            if( sv ) {
                if( isVarTrue(sv) ) {
                    sb->show();
                } else {
                    sb->hide();
                }
            } else {
                rst->setVar('3', sb->isVisible()  ? 1: 0);
            }
        } else if( ccmp(code,"headerHide") ) {
            if( sv ) {
                w->setHeaderHidden(isVarTrue(sv));
            } else {
                rst->setVar('3', w->isHeaderHidden()? 1: 0);
            }
        } else if( ccmp(code,"filter") ) {
            sv=tn->gv("@proxy");								if( SVCHK('m',1)==false ) return false;
            TreeProxyModel* proxy=(TreeProxyModel*)SVO;			if( proxy->xfields==NULL ) return false;
            bool chk = isVarTrue(arrs->get(1));
            proxy->setValid(!chk);
            // proxy->invalidateFilter();
        } else if( ccmp(code,"doubleClickExpand") ) {
            if( sv )
                w->setExpandsOnDoubleClick(isVarTrue(sv));
        } else if( ccmp(code,"expandEnable") ) {
            if( sv )
                w->setItemsExpandable(isVarTrue(sv));
            else
                rst->setVar('3', w->itemsExpandable()? 1: 0);
        } else if( ccmp(code,"drag") ) {
            w->setDragEnabled(true);
            w->setDropIndicatorShown(true);
        } else if( ccmp(code,"drop") ) {
            w->setDragDropMode(QAbstractItemView::DragOnly);
            w->setDropIndicatorShown(true);
            w->setAcceptDrops(true);
        } else if( ccmp(code,"rootDecorated") ) {
            if( sv )
                w->setRootIsDecorated(isVarTrue(sv));
            else
                rst->setVar('3', w->rootIsDecorated()?1: 0);
        } else if( ccmp(code,"editTrigger") ) {
            if( sv ) {
                int num = toInteger(sv);
                w->setEditTriggers( num ? (QAbstractItemView::EditTrigger)num : QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked  );
            } else {
                int num=(int)w->editTriggers();
                rst->setVar('0',0).addInt(num);
            }
        } else if( ccmp(code,"contextMenu") ) {
            if( sv ) {
                LPCC ty = toString(sv);
                if( ccmp(ty,"custom") ) {
                    w->setContextMenuPolicy(Qt::CustomContextMenu);
                } else if( ccmp(ty,"default") ) {
                    w->setContextMenuPolicy(Qt::DefaultContextMenu);
                } else if( ccmp(ty,"action") ) {
                    w->setContextMenuPolicy(Qt::ActionsContextMenu);
                } else {
                    w->setContextMenuPolicy(Qt::NoContextMenu);
                }
            } else {
                w->setContextMenuPolicy(Qt::CustomContextMenu);
            }
        } else if( ccmp(code,"stretchLast") ) {
            if( sv )
                w->header()->setStretchLastSection(isVarTrue(sv));
            else
                rst->setVar('3', w->header()->stretchLastSection()? 1: 0);
        } else if( ccmp(code,"selection") ) {
            if( sv ) {
                LPCC mode=toString(sv);
                w->setSelectionMode(ccmp(mode,"single")? QAbstractItemView::SingleSelection: QAbstractItemView::ExtendedSelection );
            } else {
                rst->setVar('0',0).addInt((int)w->selectionMode());
            }
        } else if( ccmp(code,"resizeWidth") ) {
            if( isVarTrue(sv) ) {
                QObject::connect(w->header(), SIGNAL(sectionResized(int logicalIndex, int oldSize, int newSize)), w, SLOT(columnResized(int logicalIndex, int oldSize, int newSize)));
            } else {
                QObject::disconnect(w->header(), SIGNAL(sectionResized(int logicalIndex, int oldSize, int newSize)), w, SLOT(columnResized(int logicalIndex, int oldSize, int newSize)));
            }
        } else if( ccmp(code,"treeMode") ) {
            if( isVarTrue(sv) ) {
                w->setAttribute(Qt::WA_StyledBackground);
                w->setIndentation(20);
                w->setAnimated(false);
                w->setExpandsOnDoubleClick(false);
                w->setFocusPolicy(Qt::StrongFocus);
                w->setExpandsOnDoubleClick(true);
                w->setAcceptDrops(true);
                w->setHeaderHidden(true);
                w->setSelectionMode(QAbstractItemView::SingleSelection);
                w->setSelectionBehavior(QAbstractItemView::SelectRows);
                w->setEditTriggers(QAbstractItemView::NoEditTriggers);
                tn->GetVar("@treeMode")->setVar('3',1);
            } else {
                w->setIndentation(0);
                w->setExpandsOnDoubleClick(false);
                w->header()->setStretchLastSection(false);
                w->setEditTriggers(QAbstractItemView::NoEditTriggers);
                tn->GetVar("@treeMode")->setVar('3',0);
            }
        } else {
            return false;
        }
    } break;
    default: return false;
    }
    return true;
}

inline LPCC setComboKeyValue(TreeNode* tn, StrVar* sv, int sp, int ep, LPCC title) {
    XParseVar pv(sv, sp, ep);
    if( pv.ch() ) {
        pv.findEnd(",");
        LPCC code=pv.v();
        if( pv.valid() ) {
            tn->GetVar("@key")->set(code);	pv.findEnd(",");
            code=pv.v();
            tn->GetVar("@val")->set(code);
            if( pv.valid() ) {
                pv.findEnd(",");
                LPCC icon = pv.v();
                if( slen(icon) ) tn->set("@icon", icon);
            }
        } else {
            if( slen(code)==0 ) code="value";
            tn->GetVar("@key")->set(code);
        }
    }

    if( slen(title) ) {
        tn->GetVar("@title")->set(title);
        return title;
    }
    return NULL;
}

inline bool comboNodeUpdate(GCombo* w, TreeNode* tn, U32 flag=0, int comboWidth=0) {
    LPCC title=tn->get("@title");
    if( flag==2 ) {
        w->clear();
        if( slen(title) ) {
            w->addItem(KR(title));
        }
        return true;
    }

    StrVar* sv=tn->gv("@data");
    TreeNode* cur=NULL;
    if( SVCHK('n',0) ) {
        cur=(TreeNode*)SVO;
    }
    tn->setNodeFlag(FLAG_RUN);
    if( cur ) {
        LPCC code=tn->get("@val");
        if( slen(code)==0 ) {
            code=tn->get("@key");
            if( slen(code)==0 ) code="value";
        }
        if( flag==1 ) {
            // combo.update(true);
            XListArr* removeArr=NULL;
            for( int n=0;n<cur->childCount();n++) {
                TreeNode* sub=cur->child(n);
                if( sub->isNodeFlag(FLAG_SKIP) ) continue;
                if( sub->isNodeFlag(FLAG_DELETE) ) {			// delete
                    int idx= slen(title) ? n+1: n;
                    if( removeArr==NULL ) removeArr=getListArrTemp();
                    removeArr->add()->setVar('n',0,(LPVOID)sub);
                    w->removeItem(idx);
                    sub->clearNodeFlag(FLAG_DELETE);
                } else if( sub->isNodeFlag(FLAG_SET) ) {	// add
                    LPCC txt=toString(sub->gv(code));
                    w->addItem(KR(txt));
                    sub->clearNodeFlag(FLAG_SET);
                }
            }
            if( removeArr ) {
                for(int n=0;n<removeArr->size();n++) {
                    sv=removeArr->get(n);
                    if(SVCHK('n',0) ) {
                        TreeNode*sub=(TreeNode*)SVO;
                        cur->remove(sub);
                    }
                }
            }
        } else if(flag==4) {
            const QFontMetrics fm = w->fontMetrics();
            int maxWidth=0;
            if( slen(title) ) {
                if( comboWidth>0 ) maxWidth=fm.width(KR(title))+20;
            }
            for( int n=0;n<cur->childCount();n++) {
                TreeNode* sub=cur->child(n);
                LPCC str=toString(sub->gv(code));
                if( comboWidth && slen(str)) {
                    int wid=fm.width(KR(str))+10;
                    if( maxWidth<wid) maxWidth=wid;
                }
            }
            if( maxWidth>comboWidth ) maxWidth=comboWidth;
            if( maxWidth>0 ) w->setFixedWidth(maxWidth);
        } else {
            // combo.update('field','title');
            const QFontMetrics fm = w->fontMetrics();
            int maxWidth=0;
            w->clear();
            if( slen(title) ) {
                w->addItem(KR(title));
                if( comboWidth>0 ) maxWidth=fm.width(KR(title))+20;
            }

            LPCC icon=tn->get("@icon");            
            for( int n=0;n<cur->childCount();n++) {
                TreeNode* sub=cur->child(n);
                if( sub->isNodeFlag(FLAG_SKIP) ) continue;
                LPCC str=toString(sub->gv(code));
                if( comboWidth>0 ) {
                    int wid=fm.width(KR(str))+10;
                    if(slen(icon)) wid+=15;
                    if( maxWidth<wid) maxWidth=wid;
                }
                if( slen(icon) ) {
                    QPixmap* pix=getStrVarPixmap(tn->gv(icon));
                    if( pix ) {
                        w->addItem(QIcon(*pix),KR(str));
                    } else {
                        w->addItem(QIcon(),KR(str));
                    }
                } else {
                    w->addItem(KR(str));
                }
            }            
            if( maxWidth > 0 ) {
                if( maxWidth>comboWidth ) maxWidth=comboWidth;
                w->setFixedWidth(maxWidth);
            }
        }
    }
    w->update();
    tn->clearNodeFlag(FLAG_RUN);
    return true;
}

bool callComboFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, QWidget* widget, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    if( widget==NULL )
        widget=gwidget(tn);
    GCombo* w=qobject_cast<GCombo*>(widget);
    if( w==NULL )
        return false;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref =
            ccmp(fnm,"value")		? 811 :
            ccmp(fnm,"addItem")		? 1 :
            ccmp(fnm,"addItems")	? 31 :
            ccmp(fnm,"check")		? 2 :
            ccmp(fnm,"listView")	? 3 :
            ccmp(fnm,"view")		? 4 :
            ccmp(fnm,"count")		? 5 :
            ccmp(fnm,"popupSize")	? 6 :
            ccmp(fnm,"comboStyle")	? 7 :
            ccmp(fnm,"current")		? 8 :
            ccmp(fnm,"remove")		? 10 :
            ccmp(fnm,"delegate")	? 12 :
            ccmp(fnm,"text")		? 13 :
            ccmp(fnm,"rootNode")	? 14 :
            ccmp(fnm,"showPopup")	? 15 :
            ccmp(fnm,"completer")	? 16 :
            ccmp(fnm,"selectText")	? 17 :
            ccmp(fnm,"addText")		? 18 :
            // ccmp(fnm,"fixedWidth")	? 19 :
            ccmp(fnm,"find")		? 20 :
            ccmp(fnm,"model")		? 21 :
            ccmp(fnm,"fields")		? 22 :
            ccmp(fnm,"update")		? 23 :
            ccmp(fnm,"viewWidth")		? 24 :
            ccmp(fnm,"index")		? 25 :
            ccmp(fnm,"textValue")	? 26 :
            ccmp(fnm,"sort")		? 27 :
            ccmp(fnm,"is")	? 173 :
            0;
        if( fc ) fc->xfuncRef = ref;
    }
    StrVar* sv=NULL;
    switch( ref ) {
    case 31:
    case 1: {	// addItem
        if( arrs==NULL ) {
            return false;
        }
        int cnt=arrs->size();
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            // ex) addItem(node, "key, value", "title");
            TreeNode* cur=NULL;
            if( SVCHK('n',0) ) {
                cur=(TreeNode*)SVO;
            }
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(1), &sp, &ep);
            setComboKeyValue(tn, sv, sp, ep, AS(2) );
            if( cur ) {
                tn->GetVar("@data")->setVar('n',0,(LPVOID)cur);
                comboNodeUpdate(w, tn);
            }
        } else {
            LPCC txt = NULL;
            XListArr* arr=tn->addArray("@adds");
            if( arr==NULL ) return false;
            if( ref==31 ) {
                // addItems("a,b,c", title);
                int sp=0,ep=0;
                LPCC title=AS(1);
                if( slen(title) ) {
                    tn->GetVar("@title")->set(title);
                } else {
                    title=tn->get("@title");
                }
                sv = getStrVarPointer(arrs->get(0),&sp,&ep);
                XParseVar pv(sv,sp,ep);
                arr->reuse();
                w->clear();
                if( slen(title) ) {
                    w->addItem(KR(title));
                }
                const QFontMetrics fm = w->fontMetrics();
                int maxWidth=0;
                while( pv.valid() ) {
                    txt=pv.findEnd(",").v();
                    arr->add()->add(txt);
                    QString text=KR(txt);
                    w->addItem(text);
                    int tw = fm.width( text);
                    if( maxWidth<tw ) maxWidth=tw;
                }
                if( isNumberVar(arrs->get(2)) ) {
                    int dw=toInteger(arrs->get(2));
                    if( dw==1 ) dw=320;
                    if( dw>0 ) {
                        maxWidth+=24;
                        if( maxWidth>dw ) maxWidth=dw;
                        w->setFixedWidth(maxWidth);
                    }
                }
            } else {
                if( cnt==1 ) {
                    txt=AS(0);
                    w->addItem(KR(txt));
                    arr->add()->add(txt);
                } if( cnt==2 ) {
                    txt=AS(0);
                    arr->add()->add(txt);
                    QPixmap* pix=getStrVarPixmap(arrs->get(1));
                    if( pix ) {
                        w->addItem(QIcon(*pix),KR(txt));
                    } else {
                        w->addItem(QIcon(),KR(txt));
                    }
                }
            }
            tn->GetVar("@data")->reuse();
        }
    } break;
    case 173:	// is
    case 2: {	// check
        if( arrs==NULL ) {
            rst->set("visible, disable, enable, hidden, active\nfull, max, min, model, topleval\n\
                editable, frame, maxCount");
            return true;
        }
        LPCC code=AS(0);
        if( isWidgetCheck(code, w, rst) ) return true;
        if( ccmp(code,"editable") ) {
            if( cnt==1 ) {
                rst->setVar('3', w->isEditable()? 1: 0 );
            } else {
                bool chk=isVarTrue(arrs->get(1));
                w->setEditable(chk);
                if( chk ) {
                    w->lineEdit()->installEventFilter( w );
                }
            }
        } else if( ccmp(code,"frame") ) {
            w->setFrame(isVarTrue(arrs->get(1)));
            rst->setVar('n',0,(LPVOID)tn);
        } else if( ccmp(code,"popup") ) {
            rst->setVar('3', w->view() && w->view()->isVisible() ? 1 : 0 );
        } else if( ccmp(code,"elideMode") ) {
            LPCC ty = AS(1);
            w->view()->setTextElideMode(
                ccmp(ty,"left")? Qt::ElideLeft:
                ccmp(ty,"right")? Qt::ElideRight :
                ccmp(ty,"middle")? Qt::ElideMiddle :
                Qt::ElideNone);
            rst->setVar('n',0,(LPVOID)tn);
        } else if( ccmp(code,"maxCount") ) {
            if( cnt==1 ) {
                rst->setVar('0',0).addInt( w->maxCount() );
            } else {
                w->setMaxCount(toInteger(arrs->get(1)));
                rst->setVar('n',0,(LPVOID)tn);
            }
        } else {
            return false;
        }
    } break;
    case 3: {	// listView
        if( arrs==NULL ) {
            QWidget* lv = (QWidget*)w->view();
            GListVIew* listview = qobject_cast<GListVIew*>(lv);
            if( listview ) {
                rst->setVar('n',0,(LPVOID)listview->config());
            }
        } else {
            StrVar* sv = arrs->get(0);
            if( SVCHK('n',0) ) {
                TreeNode* cf = (TreeNode*)SVO;
                GListVIew* listview = new GListVIew(cf, w);
                listview->setStyleSheet("QListView::item {			\
                        border-bottom: 5px solid white; margin:3px; }	\
                        QListView::item:selected {						\
                        border-bottom: 5px solid black; margin:3px;		\
                        color: black;									\
                    }\
                ");
                w->setView(listview);
                rst->setVar('n',0,(LPVOID)listview->config());
            }
        }
    } break;
    case 4: {	// view

    } break;
    case 5: {	// count
        rst->setVar('0',0).addInt(w->count());
    } break;
    case 6: {	// popupSize
        w->view();
        // iterate to find QComboBoxPrivateContainer
        if(isNumberVar(arrs->get(0))) {
            QWidget* popup = 0;
            QObjectList objects = w->children();
            for(int i = 0; i < objects.size(); ++i) {
                QObject* obj = objects.at(i);
                if( obj->inherits("QComboBoxPrivateContainer") ) {
                    popup = qobject_cast<QWidget*>(obj);
                    break;
                }
            }

            if(popup) {
                int width=toInteger(arrs->get(0)), height=toInteger(arrs->get(1));
                popup->resize(width, height);
            }
        } else {
            // ex) combo.popupSize('expanding', 200)
            LPCC ty=AS(0);
            QSizePolicy sp = w->view()->sizePolicy();
            if( isNumberVar(arrs->get(1)) ) {
                int size=toInteger(arrs->get(1));
                if(isVarTrue(arrs->get(2))) {
                    w->view()->setMaximumWidth(size);
                } else {
                    w->view()->setMinimumWidth(size);
                }
            }
            if(ccmp(ty,"fixed") ) {
                sp.setHorizontalPolicy(QSizePolicy::Fixed);
            } else if(ccmp(ty,"minExpanding") ) {
                sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
            } else if(ccmp(ty,"preferred") ) {
                sp.setHorizontalPolicy(QSizePolicy::Preferred);
            } else if(ccmp(ty,"min") ) {
                sp.setHorizontalPolicy(QSizePolicy::Minimum);
            } else if(ccmp(ty,"max") ) {
                sp.setHorizontalPolicy(QSizePolicy::Maximum);
            } else if(ccmp(ty,"expanding") ) {
                sp.setHorizontalPolicy(QSizePolicy::Expanding);
            } else if(ccmp(ty,"ignore") ) {
                sp.setHorizontalPolicy(QSizePolicy::Ignored);
            } else {
                sp.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
            }
            w->view()->setSizePolicy(sp);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 7: {	// comboStyle
        LPCC sty=arrs?AS(0):
            "QComboBox { min-height: 40px; min-width: 60px; }"
            "QComboBox QAbstractItemView::item { min-height: 40px; min-width: 60px; }";
        w->setStyleSheet(KR(sty));
    } break;
    case 8: 	// current
    case 811: {	// value
        LPCC title=tn->get("@title");
        TreeNode* root=NULL;
        sv=tn->gv("@data");
        if( SVCHK('n',0) ) {
            root=(TreeNode*)SVO;
        }
        if( arrs==NULL ) {
            int idx=w->currentIndex();
            if( idx!=-1 ) {
                if( slen(title) ) {
                    if( idx==0 ) {
                        rst->reuse();
                        return true;
                    }
                    idx-=1;
                }
                TreeNode* cur=NULL;
                XListArr* arr=NULL;
                sv=tn->gv("@data");
                if( root==NULL ) {
                    sv=tn->gv("@adds");
                    if( SVCHK('a',0) ) {
                        arr=(XListArr*)SVO;
                    }
                }
                if( root ) {
                    cur=root->child(idx);
                } else if( arr ) {
                    sv=arr->get(idx);
                    if(SVCHK('n',0) ) {
                        cur=(TreeNode*)SVO;
                    } else if( ref==811 ) {
                        rst->set(Q2A(w->currentText()));
                    }
                } else if( ref==811 ) {
                    rst->set(Q2A(w->currentText()));
                }
                if( cur ) {
                    if( ref==811 ) {
                        // value
                        LPCC key=tn->get("@key");
                        if( slen(key) )
                            rst->reuse()->add(cur->gv(key));
                        else
                            rst->set(Q2A(w->currentText()));
                    } else {
                        // current
                        rst->setVar('n',0,(LPVOID)cur);
                    }
                }
            } else {
                // idx==-1
                rst->reuse();
            }
        } else {
            LPCC key=tn->get("@key");
            int idx = 0;
            rst->reuse();
            if( ref==8 ) {
                // current
                sv=arrs->get(0);
                if( isNumberVar(sv) ) {
                    int idx=toInteger(sv);
                    if( idx<w->count() ) {
                        w->setCurrentIndex(idx);
                        if( slen(title) ) idx-=1;
                        if( idx>=0 && root ) {
                            TreeNode* sub=root->child(idx);
                            if( sub ) rst->setVar('n',0,(LPVOID)sub);
                        }
                    }
                } else if(root) {
                    TreeNode* targetNode=NULL;
                    LPCC value=NULL;
                    if( SVCHK('n',0) ) {
                        targetNode=(TreeNode*)SVO;
                    } else {
                        value=toString(sv);
                    }
                    if( isVarTrue(arrs->get(1)) ) {
                        comboNodeUpdate(w, tn);
                    }
                    if( slen(title) ) idx++;
                    for( int n=0,sz=root->childCount(); n<sz; n++ ) {
                        TreeNode* sub=root->child(n);
                        if( targetNode ) {
                            if( targetNode==sub ) {
                                w->setCurrentIndex(idx);
                                rst->setVar('n',0,(LPVOID)sub);
                                break;
                            }
                        } else {
                            LPCC code = toString(sub->gv(key));
                            if( ccmp(code,value) ) {
                                w->setCurrentIndex(idx);
                                rst->setVar('n',0,(LPVOID)sub);
                                break;
                            }
                        }
                        idx++;
                    }
                }
                if( rst->length() ) return true;
            }
            LPCC text=AS(0);
            if( slen(title) ) {
                if( slen(text)==0 ) {
                    w->setCurrentIndex(0);
                    return true;
                }
                idx++;
            }
            XListArr* arr=NULL;
            if( root==NULL ) {
                sv=tn->gv("@adds");
                if( SVCHK('a',0) ) {
                    arr=(XListArr*)SVO;
                }
            }
            if( root ) {
                for( int n=0,sz=root->childCount(); n<sz; n++, idx++ ) {
                    TreeNode* sub=root->child(n);
                    LPCC code = toString(sub->gv(key));
                    if( ccmp(code,text) ) {
                        w->setCurrentIndex(idx);
                        break;
                    }
                }
            } else if( arr ) {
                for( int n=0,sz=arr->size(); n<sz; n++, idx++ ) {
                    sv=arr->get(n);
                    if( SVCHK('n',0) ) {
                        TreeNode* sub=(TreeNode*)SVO;
                        LPCC code = toString(sub->gv(key));
                        if( ccmp(code,text) ) {
                            w->setCurrentIndex(idx);
                            break;
                        }
                    } else {
                        LPCC val=toString(sv);
                        if( ccmp(val,text) ) {
                            w->setCurrentIndex(idx);
                            break;
                        }
                    }
                }
            } else {
                if( slen(text)==0 ) {
                    w->setCurrentIndex(0);
                } else {
                    idx=w->findText(text);
                    if( idx!=-1 ) {
                        // sv=tn->gv("@title");
                        // if( sv && sv->length() && idx>1 ) idx--;
                        w->setCurrentIndex(idx);
                    }
                }
            }
            rst->setVar('0',0).addInt(idx);
        }
    } break;
    case 10: {	// remove
        if( arrs==NULL ) {
            comboNodeUpdate(w, tn, 2);
            return true;
        }
        int idx=-1;
        if( isNumberVar(arrs->get(0)) ) {
            idx=toInteger(arrs->get(0));
        } else {
            idx=w->findText(AS(0));
        }
        if( idx>=0 && idx<w->count() ) {
            w->removeItem(idx);
        } else {
            return false;
        }
        if( isVarTrue(arrs->get(1)) ) {
            sv=tn->gv("@data");
            if( sv==NULL ) {
                sv=tn->gv("@adds");
            }
            if( SVCHK('a',0) ) {
                XListArr* a=(XListArr*)SVO;
                a->remove(idx);
            } else if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                TreeNode* sub=node->child(idx);
                if( sub ) sub->isNodeFlag(FLAG_DELETE);
            }
       }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 12: {	// delegate
        if( arrs ) {
            if( isNumberVar(arrs->get(0)) ) {
                int h = toInteger(arrs->get(0) );
                if( h>10 ) {
                    tn->setInt("@lineHeight", h);
                }
            }
        }
        ComboDelegate* d = NULL;
        bool fst=false;
        sv = tn->gv("@d");
        if( SVCHK('w',9) ) {
            d = (ComboDelegate*)SVO;
        } else {
            fst = true;
            d = new ComboDelegate(tn);
            tn->GetVar("@d")->setVar('w',9,(LPVOID)d);
        }
        if( fst ) {
            w->setItemDelegate(d);
        }
        d->xuse = true;
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 13: {	// text
        if( arrs ) {
            if( w->lineEdit() ) {
                sv=arrs->get(0);
                if( SVCHK('3',1) ) {
                    rst->add( Q2A(w->lineEdit()->text()) );
                } else {
                    LPCC text=AS(0);
                    w->lineEdit()->setText(KR(text));
                }
            } else {
                LPCC text=AS(0);
                int idx=w->currentIndex();
                if( idx!=-1 && slen(text) ) w->setItemText(idx, KR(text));
                /*
                int idx=w->findText(AS(0));
                if( idx!=-1 ) w->removeItem(idx);
                */
            }
            rst->setVar('n',0,(LPVOID)tn);
        } else {
            sv=tn->gv("@title");
            if( sv && sv->length() && (w->currentIndex()==0) ) {
                rst->set("");
            } else {
                QString txt=w->currentText();
                rst->set(Q2A(txt));
            }
        }
    } break;
    case 14: {	// rootNode
        TreeNode* root=NULL;
        XListArr* arr=NULL;
        sv=tn->gv("@data");
        if( SVCHK('n',0) ) {
            root=(TreeNode*)SVO;
        } else {
            sv=tn->gv("@adds");
            if( SVCHK('a',0) ) {
                arr=(XListArr*)SVO;
            }
        }
        if( arrs ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                TreeNode* node=(TreeNode*)SVO;
                if( node!=root ) {
                    root=node;
                    tn->GetVar("@data")->setVar('n',0,(LPVOID)node);
                    comboNodeUpdate(w, tn);
                }
            }

        }
        if( root ) {
            rst->setVar('n',0,(LPVOID)root);
        } else if( arr ) {
            rst->setVar('a',0,(LPVOID)arr);
        }
    } break;
    case 15: {	// showPopup
        if( arrs ) {
            if( isVarTrue(arrs->get(0)) ) {
                w->showPopup();
            } else {
                w->hidePopup();
            }
        } else {
            w->showPopup();
        }
    } break;
    case 16: {	// completer
        if( arrs==NULL ) {
            return false;
        }
        LPCC ty = AS(0);
        if( ccmp(ty,"create") ) {
            QCompleter* c = w->completer();
            if( c==NULL ) {
                c = new QCompleter(w);
            }
            StrVar* sv = arrs->get(1);
            bool ok = true;
            LPCC field = AS(2);
            if( SVCHK('n',0) ) {
                TreeNode* cur = (TreeNode*)SVO;
                if( slen(field)==0 ) field = "text";
                QStringList list;
                QStringListModel* model = new QStringListModel(c);
                for( int n=0,sz=cur->childCount(); n<sz; n++ ) {
                    LPCC val = cur->child(n)->get(field);
                    list.append(KR(val));
                }
                model->setStringList(list);
                c->setModel(model);
            } else if( SVCHK('a',0) ) {
                XListArr* a = (XListArr*)SVO;
                QStringList list;
                QStringListModel* model = new QStringListModel(c);
                for( int n=0,sz=a->size(); n<sz; n++ ) {
                    LPCC val=NULL;
                    sv=a->get(n);
                    if( SVCHK('n',0) ) {
                        TreeNode* node=(TreeNode*)SVO;
                        sv=node->gv(field);
                    }
                    val=toString(sv);
                    if( slen(val) ) list.append(KR(val));
                }
                model->setStringList(list);
                c->setModel(model);
            }
            if( ok ) w->setCompleter(c);
        } else if( ccmp(ty,"column") ) {
            QCompleter* c = w->completer();
            if( c==NULL ) return false;
            if( isNumberVar(arrs->get(1)) ) {
                c->setCompletionColumn( toInteger(arrs->get(1)) );
            }
        } else if( ccmp(ty,"mode") ) {
            LPCC kind=AS(1);
            QCompleter* c = w->completer();
            if( c==NULL ) return false;
            if( ccmp(kind,"inline") ) c->setCompletionMode(QCompleter::InlineCompletion);
            else if( ccmp(kind,"popup") ) c->setCompletionMode(QCompleter::PopupCompletion);
            else if( ccmp(kind,"unfilter") ) c->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
            else if( ccmp(kind,"caseSensitive") ) c->setCaseSensitivity(isVarTrue(arrs->get(2))?Qt::CaseSensitive: Qt::CaseInsensitive );
            else if( ccmp(kind,"maxVisibleItems") ) c->setMaxVisibleItems(toInteger(arrs->get(2)) );
            else if( ccmp(kind,"wrapAround") ) c->setWrapAround(isVarTrue(arrs->get(2)) );
            else if( ccmp(kind,"modelColumn") ) c->setCompletionColumn(toInteger(arrs->get(2)) );
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 17: {	// selectText
        if( w->lineEdit() ) {
            if( arrs ) {
                if( cnt==1 ) {
                    sv=arrs->get(0);
                    if( SVCHK('3',1) ) {
                        QString txt=w->currentText();
                        int len=txt.length();
                        w->lineEdit()->setCursorPosition(len);
                        w->lineEdit()->cursorBackward(true,len);
                        // w->lineEdit()->setSelection(0,txt.length());
                        // w->lineEdit()->backspace();
                    } else if( isNumberVar(sv) ) {
                        int sp=toInteger(sv);
                        if( sp==0 ) {
                            w->lineEdit()->setSelection(0,0);
                        } else {
                            QString txt=w->currentText();
                            if( sp<=txt.length() ) {
                                w->lineEdit()->setSelection(sp, txt.length());
                            }
                        }
                    }
                } else {
                    QString txt=w->currentText();
                    int sp=toInteger(arrs->get(0)), ep=toInteger(arrs->get(1));
                    if( sp<=txt.length() ) {
                        if( ep>txt.length() ) ep=txt.length();
                        w->lineEdit()->setSelection(sp, ep);
                    }
                }
            } else {
                w->lineEdit()->selectAll();
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 18: {	// addChild
        if( arrs==NULL ) return false;
        TreeNode* root=NULL;
        XListArr* adds=NULL;
        sv=tn->gv("@data");
        if( SVCHK('n',0) ) {
            root=(TreeNode*)SVO;
        } else {
            sv=tn->gv("@adds");
            if( SVCHK('a',0) ) {
                adds=(XListArr*)SVO;
            }
        }
        if( root ) {
            TreeNode* cur=root->addNode();
            LPCC key=tn->get("@key");
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(0),&sp,&ep);
            XParseVar pv(sv,sp,ep);
            char ch=pv.moveNext().ch();
            pv.SetPosition();
            if( ch==':') {
                cur->setJson(pv);
            } else if( ch=='#' ) {
                LPCC val=tn->get("@val"), k=pv.findEnd("#").v(), v=pv.get();
                cur->set(key, k);
                cur->set(val, v);
            } else {
                cur->set(key, pv.get() );
            }
            cur->setNodeFlag(FLAG_SET);
            comboNodeUpdate(w, tn, 1);
        } else {
            LPCC text=AS(0);
            if( slen(text) ) {
                if( adds ) adds->add()->set(text);
                w->addItem(KR(text));
            }
        }

    } break;

    case 20: {	// find
        if( arrs==NULL ) return false;
        LPCC text=AS(0);
        int idx=w->findText(KR(text));
        rst->setVar('0',0).addInt(idx);
    } break;
    case 21: {	// model

    } break;
    case 22: {	// fields

    } break;
    case 23: {	// update
        TreeNode* cur=NULL;
        TreeNode* data=NULL;
        int comboWidth=0;
        sv=tn->gv("@data");
        if( SVCHK('n',0) ) {
            data=(TreeNode*)SVO;
        }
        if( arrs ) {
            sv=arrs->get(0);
            if( SVCHK('n',0) ) {
                cur=(TreeNode*)SVO;
                tn->GetVar("@data")->setVar('n',0,(LPVOID)cur);
                sv=arrs->get(1);
                if( sv==NULL ) {
                    comboNodeUpdate(w, tn);
                } else if( isNumberVar(sv)) {
                    if(data==cur && data->childCount()==cur->childCount() ) {
                        rst->setVar('3',0);
                        return true;
                    }
                    comboWidth=isNumberVar(sv)? toInteger(sv): 0;
                    comboNodeUpdate(w, tn, 4, comboWidth);
                } else {
                    int sp=0, ep=0;
                    sv=getStrVarPointer(arrs->get(1), &sp, &ep);
                    setComboKeyValue(tn, sv, sp, ep, AS(2) );
                    sv=arrs->get(3);
                    if( isNumberVar(sv) ) comboWidth=toInteger(sv);
                    comboNodeUpdate(w, tn, 0, comboWidth);
                }
                rst->setVar('3',1);
            } else if( SVCHK('3',1) ) {
                sv=arrs->get(1);
                if( isNumberVar(sv) ) comboWidth=toInteger(sv);
                comboNodeUpdate(w, tn, 1, comboWidth);
            }
        } else {
            comboNodeUpdate(w, tn);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 24: {	// viewWidth
        const QObjectList a = w->children();
        const int j = a.count();
        QAbstractItemView *view = 0;
        QString className = "";
        bool hasView = false;
        for(int i = 0; i < j; ++i) {
            const QObjectList b = a.at(i)->children();
            const int y = b.count();
            for(int x = 0; x < y; ++x) {
                className = b.at(x)->metaObject()->className();
                if(className == "QComboBoxListView") {
                    view = (QAbstractItemView *) b.at(x);
                    hasView = true;
                    break;
                }
            }
            if( hasView == true) {
                break;
            }
        }
        if( hasView == true) {
            int wid=0;
            if( arrs==NULL ) {
                wid=view->width();
            } else {
                int cw=w->width();
                wid=isNumberVar(arrs->get(0)) ? toInteger(arrs->get(0)): 0;
                if( wid<=0 ) {
                    wid=cw;
                    tn->GetVar("@textWidth")->setVar('0',0).addInt(wid);
                } else {
                    if( wid>1024 ) wid=980;
                    wid=max(wid,cw);
                }
                view->setFixedWidth( wid );
            }
            rst->setVar('0',0).addInt(wid);
        }
    } break;
    case 25: {	// index
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(w->currentIndex());
        } else {
            sv=arrs->get(0);
            if( isNumberVar(sv)) w->setCurrentIndex(toInteger(sv));
        }
    } break;
    case 26: {	// textValue
        if( arrs==NULL ) {
            QString txt=w->currentText();
            rst->set( Q2A(txt) );
        } else {
            LPCC fieldValue=AS(0);
            LPCC key=tn->get("@key"), val=tn->get("@val"), v=AS(1);
            if( slen(v) ) val=v;
            rst->reuse();
            sv=tn->gv("@data");
            if( SVCHK('n',0) ) {
                TreeNode* data=(TreeNode*)SVO;
                for(int n=0,sz=data->childCount();n<sz;n++ ) {
                    TreeNode* cur=data->child(n);
                    if( cur->cmp(key, fieldValue) ) {
                        sv=cur->gv(val);
                        rst->add(toString(sv));
                        break;
                    }
                }
            }
        }
    } break;
    case 27: {	// sort
    } break;

    default: return false;
    }
    return true;
}

void setModel(QAbstractItemView* view, TreeNode* cf, bool reset  ) {
    TreeModel* model=setTreeModel(cf, reset );
    if( !model ) {
        qDebug("#9#set data model error (id:%s TreeModel not define)", cf->get("id"));
        return;
    }
    TreeProxyModel* proxy= NULL;
    TreeDelegate* delegate=NULL;
    StrVar* sv=cf->GetVar("@proxy");

    if( SVCHK('m',1) ) {
        proxy = (TreeProxyModel*)SVO;
    } else {
        proxy = new TreeProxyModel(cf);
        delegate = new TreeDelegate(proxy,view);
        view->setItemDelegate(delegate);
        cf->GetVar("@proxy")->setVar('m',1,(LPVOID)proxy);
    }
    proxy->setRootModel(model);
    sv = cf->gv("@fields");
    if( sv && sv->length() ) {
        proxy->setFieldMap(sv);
    }

    if( delegate ) {
        switch( cf->xstat ) {
        case WTYPE_COMBO:
        case WTYPE_LIST:
            view->setModel(proxy);
            view->connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), &uom, SLOT(onViewCurrentChanged(QModelIndex,QModelIndex)) );
            view->connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), &uom, SLOT(onViewSelectionChanged(QItemSelection,QItemSelection)) );
            break;
        case WTYPE_TREE:
            GTree* tree = qobject_cast<GTree*>(view);
            // version 1.0.3
            LPCC tag=cf->get("tag");
            sv=cf->gv("headerUse");
            if( ccmp(tag,"grid") || SVCHK('3',1) ) {
                tree->setHeader(new GHeaderView(tree->config(),Qt::Horizontal));
                tree->connect(tree->header(), SIGNAL(sectionClicked(int)), tree, SLOT(slotClickHeader(int)) );
                tree->setHeaderHidden(false);
            }
            tree->setModel(proxy);
            tree->connect(tree, SIGNAL(clicked(QModelIndex)), tree, SLOT(slotClicked(QModelIndex)) );
            tree->connect(view, SIGNAL(doubleClicked(QModelIndex)), view, SLOT(slotDoubleClicked(QModelIndex)) );
            tree->connect(view, SIGNAL(collapsed(QModelIndex)), view, SLOT(slotCollapsed(QModelIndex)) );

            tree->connect(tree->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), tree, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)) );
            tree->connect(tree->selectionModel(), SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), tree, SLOT(slotCurrentColumnChanged(QModelIndex,QModelIndex)) );
            tree->connect(tree->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), tree, SLOT(slotCurrentRowChanged(QModelIndex,QModelIndex)) );
            tree->connect(tree->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), tree, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)) );
            resizeTree(tree, proxy);
            break;
        }
    }



}
