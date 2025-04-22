#include "tree_model.h"


TreeModel::TreeModel(TreeNode* cf, QObject*parent ) : QAbstractItemModel(parent), xcf(cf), xfields(NULL), xroot(NULL), xfnChildData(NULL), xstate(0) {
    // xheader = gtrees.getTreeNode();
}

TreeModel::~TreeModel() {
    if(xroot) xroot->reset(true);
}

void TreeModel::initModel() {    
}

TreeNode* TreeModel::getFieldsNode() {
    return xfields;
}
TreeNode* TreeModel::setFieldsNode(TreeNode* fields ) {
    if( fields==NULL ) return xfields;
    xfields=fields;
    return xfields;
}
TreeNode* TreeModel::setRootNode(TreeNode* root, bool reset ) {
    if( root==NULL ) {
        if( xroot==NULL ) {
            root=new TreeNode(2, true);
        } else {
            root=xroot;
        }
    }
    if( xroot && xroot!=root ) {
        if( reset && xroot->isNodeFlag(FLAG_NEW) ) {
            SAFE_DELETE(xroot);
        }
    }
    root->setNodeFlag(FLAG_ROOT);
    xroot=root;
    return xroot;
}

TreeNode* TreeModel::setFields(StrVar* sv) {
    if( SVCHK('n',0) ) {
        xfields = (TreeNode*)SVO;
    }
    return xfields;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if( !index.isValid() ) {
        return QVariant();
    }
    if( role== Qt::DisplayRole ) {
        TreeNode* node=static_cast<TreeNode*>(index.internalPointer());
        if( node==NULL )
            return QVariant();
        TreeNode* header = xfields && index.column()<xfields->childCount() ? xfields->child(index.column()) : NULL;
        if( header ) {
            StrVar* sv=node->gv(header->get("field"));
            if( sv ) return KR(toString(sv));
        }
        // return gum.modelDisplayRole(this, static_cast<TreeNode*>(mapToSource(index).internalPointer()), getMapCode(index.column()) );
    }

    return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
    return 0;
}

QModelIndex TreeModel::index(int row, int column,const QModelIndex &parent) const {
    if( !hasIndex(row, column, parent)) {
        return QModelIndex();
    }
    TreeNode *p = parent.isValid()? static_cast<TreeNode*>(parent.internalPointer()): xroot;
    TreeNode *c = p->child(row);
    return c ? createIndex(row, column, c): QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
    if( !index.isValid())
        return QModelIndex();
    TreeNode *c = static_cast<TreeNode*>(index.internalPointer());
    TreeNode *p = c?c->parent(): NULL;
    return p==NULL || p==xroot ? QModelIndex() : createIndex(p->row(), 0, p);
}

int TreeModel::rowCount(const QModelIndex &parent) const {
    if( parent.column() > 0 ) {
        qDebug("rowCount skip ==========%d",  parent.column() );
        return 0;
    }
    int cnt=0;
    TreeNode* cur=parent.isValid() ? static_cast<TreeNode*>(parent.internalPointer()) : xroot;
    if( cur ) {
        cnt = cur->childCount();
        if( cnt || cur->isBool("@load") )
            return cnt;
        XFuncNode* fnCur=getEventFuncNode(xcf, "onChildData");
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            arrs->add()->setVar('n',0,(LPVOID)cur);
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
            cur->setBool("@load", true);
            return cur->childCount();
        }
    }
    return cnt; // gum.modelChildData(cur,xcf, xroot );
}

int TreeModel::columnCount(const QModelIndex &parent) const {
    int cnt=0;
    if( xcf ) {
        cnt=xcf->getInt("@columCount");
        if( cnt>0 ) return cnt;
    }
    if( xfields ) {
        cnt=xfields->childCount();
    }
    return cnt;
}

QModelIndex TreeModel::getIndex(TreeNode* node, int idx) const {
    return node && node->parent() ? createIndex(node->row(), idx, node): QModelIndex();
}

/************************************************************************/
/* TreeProxyModel                                                       */
/************************************************************************/
inline LPCC findJsonCode(TreeNode* root, TreeNode* cur) {
    WBoxNode* bn = root->First();
    StrVar* sv=NULL;
    while( bn ) {
        sv=&(bn->value);
        if( SVCHK('n',0) && cur==(TreeNode*)SVO ) {
            return root->getCurCode();
        }
        bn = root->Next();
    }
    return NULL;
}

inline QVariant getFieldVariant(StrVar* var) {
    if( var==NULL ) return QVariant();
    return  var ? KR(var->get()): QVariant();
}

TreeProxyModel::TreeProxyModel( TreeNode* winfo, QObject* parent ) : QSortFilterProxyModel(parent),
    xwinfo(winfo), xsrcModel(NULL), xfetchNode(NULL), xfilterFlag(0), xlineHeight(28), xfields(NULL), xrootNode(NULL) {
    setDynamicSortFilter(true);
    xdraw=new TreeNode(2, true);
    xdraw->xstat=FNSTATE_DRAW;
}

TreeProxyModel::~TreeProxyModel() {
    if( xdraw && xdraw->isNodeFlag(FLAG_NEW) ) {
        SAFE_DELETE(xdraw);
    }
}

bool TreeProxyModel::isValid() const {
    return true;
}

void TreeProxyModel::setValid(bool b) {
    invalidateFilter();
}

QString TreeProxyModel::searchString() const
{
    return xsearchString;
}

void TreeProxyModel::setSearchString(const QString &str) {
    xfilterFlag|=FILTER_SEARCH;
    if( str == xsearchString )
        return;

    xsearchString = str;
    xsearchMatcher.setPattern(str);
    xwordMatcher.setPattern(QLatin1String("\\b") + QRegExp::escape(str));
    invalidateFilter();
}

void TreeProxyModel::setRootModel(TreeModel* srcModel) {
    if( srcModel==NULL )
        return;
    xsrcModel = srcModel;
    setSourceModel(srcModel);
    // if( xfields==NULL ) setFieldMap(NULL);
}

int TreeProxyModel::setFieldMap( StrVar* sv, int sp, int ep ) {
    if( xsrcModel==NULL || xwinfo==NULL ) {
        return 0;
    }
    bool addMode=false;
    if( SVCHK('a',0)) {
        XListArr* arr=(XListArr*)SVO;
        int size=arr->size();
        if( size>0) {
            TreeNode* cur=NULL;
            xfields = xwinfo->addNode("@fieldNode");
            if( xfields->childCount()!=size) {
                xfields->removeAll(true);
                addMode=true;
            }
            for(int n=0; n<size; n++) {
                if(addMode) {
                    cur=xfields->addNode();
                } else {
                    cur=xfields->child(n);
                }
                if(cur==NULL) break;
                sv=arr->get(n);
                if(SVCHK('n',0)) {
                    TreeNode* src=(TreeNode*)SVO;
                    cur->clone(src,true);
                } else {
                    LPCC name=toString(sv);
                    cur->set("field", name);
                    cur->set("text", name);
                }
            }
        }
    } else if( SVCHK('n',0)) {
        TreeNode* node=(TreeNode*)SVO;
        TreeNode* cur=NULL;
        xfields = xwinfo->addNode("@fieldNode");
        if( xfields->childCount()!=node->getInt("@lastIndex")) {
            xfields->removeAll(true);
            addMode=true;
        }
        for(int n=0;n<node->childCount();n++) {
            TreeNode* sub=node->child(n);
            if(sub->isNodeFlag(FLAG_VARCHECK)) continue;
            if(addMode) {
                cur=xfields->addNode();
            } else {
                cur=xfields->child(n);
            }
            if(cur==NULL) break;
            cur=xfields->addNode();
            cur->clone(sub,true);
        }
    } else {
        xfields = xwinfo->addNode("@fieldNode"); //xsrcModel->getFieldsNode();
        XListArr* arrs=NULL;
        XParseVar pv;
        pv.SetVar(sv, sp, ep);
        if( pv.ch()=='[' ) {
            xfields->setJson(sv,sp,ep);
        } else {
            // field: name#width#align; || field1, field2, ..., field9
            pv.SetPosition();
            LPCC sep=pv.find(";")==-1? ",": ";";
            arrs=getListArrTemp();
            while(pv.valid() ) {
                pv.findEnd(sep);
                arrs->add()->set(pv.v());
            }
        }
        if( arrs ) {
            int size=arrs->size();
            if( xfields->childCount()!=size ) {
                xfields->removeAll(true);
                addMode=true;
            }
            for( int n=0,sz=size; n<sz; n++ ) {
                pv.SetVar(arrs->get(n));
                LPCC code=pv.findEnd(":").v();
                TreeNode* cur=NULL;
                if( slen(code) ) {
                    if(addMode) {
                        cur=xfields->addNode();
                    } else {
                        cur=xfields->child(n);
                    }
                }
                if(cur==NULL){
                    break;
                }
                cur->set("field", code);
                if( pv.valid() ) {
                    // ex) id: ID#100#center
                    for(int x=1; x<5 && cur && pv.valid(); x++ ) {
                        LPCC val=pv.findEnd("#").v();
                        if( x==1 ) cur->set("text",val);
                        else if( x==2 ) cur->set("width",val);
                        else if( x==3 ) cur->set("align",val);
                        else if( x==4 ) cur->set("type",val);
                    }
                }
                sv=cur->GetVar("text");
                if( sv->length()==0 ) sv->set(code);
            }
        }
    }
    TreeNode* cf=xsrcModel->xcf;
    int cnt=xfields->childCount();
    if( cf ) {
        xsrcModel->setFieldsNode(xfields);
        int prev=cf->getInt("@columCount");
        if( prev<cnt ) cf->setInt("@columCount", cnt);
    }
    return cnt;
}

bool TreeProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if( action == Qt::IgnoreAction) {
        return true;
    }
    XFuncNode* fn= getEventFuncNode(xwinfo, "onDropData");
    if( fn ) {
        XListArr* arrs=getLocalParams(fn);
        arrs->add()->setVar('m',2,(LPVOID)data);
        setFuncNodeParams(fn, arrs);
        fn->call();
        return isVarTrue(fn->getResult());
    }
    return false;
    // return QSortFilterProxyModel::dropMimeData(data, action, row, column, parent);
}


QMimeData *TreeProxyModel::mimeData(const QModelIndexList &indexes) const {
    XFuncNode* fn= getEventFuncNode(xwinfo, "onDragStart");
    if( fn ) {
        XListArr* nodes=xwinfo->addArray("@mineData", true);
        foreach( QModelIndex index, indexes ) {
            if( index.isValid() ) {
                TreeNode* node=static_cast<TreeNode*>(mapToSource(index).internalPointer());
                if( node ) {
                    nodes->add()->setVar('n',0,(LPVOID)node);
                }
            }
        }
        XListArr* arrs=getLocalParams(fn);
        arrs->add()->setVar('a',0,(LPVOID)nodes);
        setFuncNodeParams(fn, arrs);
        fn->call();
        if( isVarTrue(fn->getResult()) ) {
            QMimeData* data = new QMimeData();
            LPCC mineType="application/nodeData";
            QByteArray ba;
            StrVar* var=arrs->get(0);
            ba.append(var->get(), var->length());
            data->setData(KR(mineType),ba);
            return data;
        }
    }
    return QSortFilterProxyModel::mimeData(indexes);
}

QStringList TreeProxyModel::mimeTypes() const {
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}

Qt::DropActions TreeProxyModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}



bool TreeProxyModel::setData(const QModelIndex & index, const QVariant & value, int role)  {
    if( !index.isValid() )
        return false;
    return sourceModel()->setData(mapToSource(index), role);
}

TreeNode* TreeProxyModel::getHeaderNode(int c, U32 flag ) const {
    return xfields && c<xfields->childCount() ? xfields->child(c) : xsrcModel->xroot; // getFieldNodes()->getvalue(c);
}

LPCC TreeProxyModel::getMapCode( int index ) const {
    return getHeaderNode(index)->GetVar("field")->get();
}

int TreeProxyModel::getMapIndex(LPCC code ) const {
    for( int n=0,cnt=xfields->childCount();n<cnt;n++) {
        if( ccmp(getMapCode(n),code) ) return n;
    }
    return -1;
}


QVariant TreeProxyModel::data(const QModelIndex& index, int role) const {
    if( !index.isValid() )
        return QVariant();
    switch( role ) {
        case Qt::DisplayRole : {
            TreeNode* node=static_cast<TreeNode*>(mapToSource(index).internalPointer());
            if( node==NULL )
                return QVariant();
            StrVar* sv=node->gv(getMapCode(index.column()));
            return KR(toString(sv));
            // return gum.modelDisplayRole(this, static_cast<TreeNode*>(mapToSource(index).internalPointer()), getMapCode(index.column()) );
        } break;
        case Qt::TextAlignmentRole : {
            TreeNode * node = getHeaderNode(index.column());
            if( node==NULL )
                return (int)(Qt::AlignLeft|Qt::AlignVCenter);
            return getAlignFlag(node->get("align") );
        } break;
        case Qt::EditRole : {
            // return gum.modelEditRole(this, static_cast<TreeNode*>(mapToSource(index).internalPointer()), getMapCode(index.column()) );
            TreeNode* node=static_cast<TreeNode*>(mapToSource(index).internalPointer());
            if( node==NULL )
                return QVariant();
            StrVar* sv=node->gv(getMapCode(index.column()));
            return KR(toString(sv));
            // return KR(node->get(getMapCode(index.column())));
        } break;
        case Qt::ForegroundRole : break;
        case Qt::DecorationRole : break;
        case Qt::UserRole : break;
        case Qt::ToolTipRole : {
            XFuncNode* fnCur=getEventFuncNode(xwinfo, "onTip");
            if( fnCur ) {
                TreeNode* node=static_cast<TreeNode*>(mapToSource(index).internalPointer());
                if( node==NULL )
                    return QVariant();
                XListArr* arrs=getLocalParams(fnCur);
                arrs->add()->setVar('0',0).addInt(index.column());
                arrs->add()->setVar('n',0,(LPVOID)node);
                setFuncNodeParams(fnCur, arrs);
                fnCur->call();
                StrVar* sv=fnCur->getResult();
                return KR(toString(sv));
            }
        } break;
        default:
            break;
    }
    return sourceModel()->data(mapToSource(index), role);
}

int TreeProxyModel::columnCount( const QModelIndex &parent) const {
    int cnt = xfields ? xfields->childCount(): 0; // && xfields->listSize() ? xfields->listSize(): getMap()->size();
    return cnt;
}

QVariant TreeProxyModel::headerData( int section, Qt::Orientation orientation, int role ) const {
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
        TreeNode* hn = getHeaderNode(section);
        return hn? KR(hn->get("text")): QVariant();
    }
    return QVariant();
}

bool TreeProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
    TreeNode* leftNode = (TreeNode*)(left.internalPointer());
    TreeNode* rightNode = (TreeNode*)(right.internalPointer());
    if( xsrcModel==NULL )
        return QSortFilterProxyModel::lessThan(left,right);
    XFuncNode* fnCur=getEventFuncNode(xwinfo, "onSort");
    if( fnCur && leftNode && rightNode ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->setVar('0',0).addInt( left.column());
        arrs->add()->setVar('n',0,(LPVOID)leftNode );
        arrs->add()->setVar('n',0,(LPVOID)rightNode );
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
        StrVar* sv = fnCur->getResult();
        if( checkObject(sv,'3') ) {
            if( SVCHK('3',1) ) return true;
            return false;
        }
        // return isVarTrue(xfnSort->getResult());
    }
    // int rtn = gum.modelLessThan(xwinfo, leftNode, rightNode, getMapCode(left.column()) );
    return QSortFilterProxyModel::lessThan(left,right);
}

inline TreeNode* findChildNode(TreeNode* root, TreeNode* findNode) {
    for( int n=0, cnt=root->childCount(); n<cnt; n++ ) {
        TreeNode* cur=root->child(n);
        if( cur==findNode ) return cur;
        if( cur->childCount() ) {
            cur=findChildNode(cur, findNode);
            if( cur ) return cur;
        }
    }
    return NULL;
}
inline int nodeMaxDepth(TreeNode* node, int maxNum=0) {
    int d=0;
    for(int n=0; node && n<64; n++ ) {
        if( maxNum && maxNum<d )
            return d;
        node=node->parent();
        d++;
    }
    return d;
}

bool TreeProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
    if( !idx.isValid() )
        return false;
    /*
    if( xfilterFlag&FILTER_VALID )
        return true;
    */
    TreeNode* node = static_cast<TreeNode*>(idx.internalPointer());
    if( node==NULL ) return false;
    if( xfilterFlag&FILTER_NOCHILD && xrootNode ) {
        if( xrootNode==node ) return true;
        int maxDepth=nodeMaxDepth(xrootNode);
        if( xrootNode!=node->xparent ) {		// xrootNode!=node &&
            if( nodeMaxDepth(node,maxDepth)> maxDepth )
                return false;
            if( findChildNode(node, xrootNode) )
                return true;
            return false;
        }
    }
    XFuncNode* fnCur=getEventFuncNode(xwinfo, "onFilter");
    if( fnCur ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->setVar('n',0,(LPVOID)node );
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
        return isVarTrue(fnCur->getResult());
    }
    return true; // gum.modelFilter( xwinfo, static_cast<TreeNode*>(idx.internalPointer()), sourceRow );
}

Qt::ItemFlags TreeProxyModel::flags(const QModelIndex &index) const {
    if( !index.isValid() )
        return 0;
    TreeNode* node=static_cast<TreeNode*>(index.internalPointer() );
    XFuncNode* fnCur=getEventFuncNode(xwinfo, "onFlags");
    if( fnCur && node ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->setVar('3', index.column() );
        arrs->add()->setVar('n',0,(LPVOID)node );
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
        U32 flag=toInteger(fnCur->getResult());
        return (Qt::ItemFlags)flag;
    }
    return Qt::ItemIsEnabled |
            Qt::ItemIsSelectable |
            Qt::ItemIsEditable |
            Qt::ItemIsDropEnabled |
            Qt::ItemIsDragEnabled;
    // return gum.modelFlags( xwinfo, static_cast<TreeNode*>(mapToSource(index).internalPointer()), getMapCode(index.column()) );
}


bool TreeProxyModel::canFetchMore( const QModelIndex& index) const {    
    if( xwinfo->cmp("tag","tree") ) {   // !index.isValid() ||
        return false;
    }
    TreeNode* root = xsrcModel->xroot;
    if( root==NULL || root->childCount()==0 ) {
        // qDebug("setModel canFetchMore (%s %d)", xwinfo->get("tag"), index.row() );
        return false;
    }
    StrVar* sv=NULL;
    XFuncNode* fnCur=getEventFuncNode(xwinfo, "onFetchMore");
    if( fnCur ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->setVar('9',0);
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
        sv=fnCur->getResult();
        return SVCHK('3',1);
    } else {
        TreeNode* root = xsrcModel->xroot;
        if( root ) {
            sv=root->gv("fetchNum");
            if( isNumberVar(sv) ) {
                int fetchNum=toInteger(sv);
                if( root->childCount()==fetchNum )
                    return true;
            }
        }
    }
    return false;
}

void TreeProxyModel::fetchMore( const QModelIndex& index ) {
    TreeNode* root = xsrcModel->xroot;
    XFuncNode* fnCur=getEventFuncNode(xwinfo, "onFetchMore");
    qDebug("setModel fetchMore (%d)", index.row() );
    if( root && fnCur ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->setVar('n',0, root);
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();        
    }
}

void TreeProxyModel::setDraw( XFuncNode* fn ) {

}

//
// TreeDelegate
//

TreeDelegate::TreeDelegate(TreeProxyModel* proxy, QObject *parent) : QStyledItemDelegate(parent), xproxy(proxy), xarrs(NULL) {
    initDelegate();
}

void TreeDelegate::initDelegate() {
    if( !xproxy )
        return;
}

void TreeDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    if( !index.isValid() )
        return;
    TreeNode* cf=xproxy->xwinfo;
    XFuncNode* fnCur=getEventFuncNode(cf, "onDraw");
    if( fnCur ) {
        TreeNode* node=static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
        TreeNode* d=xproxy->xdraw;
        if( d && node ) {
            // cf->setNodeFlag(FLAG_DRAW);
            // onDraw(drawObject, rect, node, index, over, state)
            XListArr* arrs=getLocalParams(fnCur);
            setVarRectF(d->GetVar("@rect"), option.rect );
            d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
            arrs->add()->setVar('n',0,(LPVOID)d );
            arrs->add()->setVar('n',0,(LPVOID)node);
            arrs->add()->setVar('0',0).addInt(index.column());
            arrs->add()->setVar('0',0).addInt(option.state);
            arrs->add()->setVar('3',option.state & QStyle::State_MouseOver?1:0);
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
            // cf->clearNodeFlag(FLAG_DRAW);
            return;
        }

    }
    QStyledItemDelegate::paint(painter,option,index);
}

bool TreeDelegate::eventFilter(QObject* editor, QEvent* event) {
    TreeNode* cf=xproxy->xwinfo;
    StrVar* sv = cf? cf->gv("@editNode"): NULL;
    if( SVCHK('n',0) ) {
        // qDebug("tree eventFilter %d", event->type() );
        cf = (TreeNode*)SVO;
        switch( event->type() ) {
        case QEvent::FocusIn: {
            XFuncNode* fn= getEventFuncNode(cf, "onEditorFocusIn");
            if( fn ) {
                QFocusEvent *e = static_cast<QFocusEvent*>(event);
                fn->call();
            }
        } break;
        case QEvent::FocusOut: {
            XFuncNode* fn= getEventFuncNode(cf, "onEditorFocusOut");
            if( fn ) {
                QFocusEvent *e = static_cast<QFocusEvent*>(event);
                fn->call();
            }
        } break;
        case QEvent::KeyPress: {
            XFuncNode* fn= getEventFuncNode(cf, "onEditorKeyDown");
            if( fn ) {
                QKeyEvent* e = static_cast<QKeyEvent*>(event);
                XListArr* arrs=getLocalParams(fn);
                arrs->add()->setVar('0',0).addInt(e->key());
                arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
                setFuncNodeParams(fn, arrs);
                fn->call();
                sv = fn->getResult();
                if( SVCHK('3',1) ) {
                    return true;
                }
            }
        } break;
        case QEvent::MouseButtonPress: {
            XFuncNode* fn= getEventFuncNode(cf, "onEditorMouseDown");
            QMouseEvent* e = static_cast<QMouseEvent*>(event);
            if( fn ) {
                XListArr* arrs=getLocalParams(fn);
                arrs->add()->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
                arrs->add()->setVar('1',0).addUL64((UL64)e->modifiers());
                arrs->add()->set(e->button()==Qt::LeftButton?"left":"right");
                setFuncNodeParams(fn, arrs);
                fn->call();
                if( ccmp(toString(fn->getResult()),"ignore") ) {
                    return true;
                }
            } else {
                cf->GetVar("@mpos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
            }
        } break;
        default:break;
        }

    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

QWidget* TreeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // QWidget* w=gum.modelEditor(xproxy, static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer()), option, index.column());
    TreeNode* cf=xproxy->xwinfo;
    qDebug("edit event : createEditor");
    if( cf==NULL ) return QStyledItemDelegate::createEditor(parent,option,index);
    StrVar* editNode = cf->GetVar("@editNode");
    if( editNode ) editNode->reuse();
    XFuncNode* fn= getEventFuncNode(cf, "onEditEvent");
    if( fn ) {
        XFuncSrc* fsrc= fn->xfunc->getFuncSrc();
        TreeNode* node = static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
        if( fsrc && node ) {            
            XListArr* arrs=getLocalParams(fn);
            arrs->add()->set("create");
            arrs->add()->setVar('n',0,(LPVOID)node);
            arrs->add()->setVar('0',0).addInt(index.column());
            arrs->add()->setVar('9',0);
            setFuncNodeParams(fn, arrs);
            fn->call();
            StrVar* sv = fn->getResult();
            if( SVCHK('n',0) ) {
                TreeNode* wnd = (TreeNode*)SVO;
                editNode->setVar('n',0,(LPVOID)wnd);
                QWidget* w = gwidget(wnd);
                if( w ) return w;
            }
        }
    }
    return QStyledItemDelegate::createEditor(parent,option,index);
}

void TreeDelegate::updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
    TreeNode* cf=xproxy->xwinfo;
    XFuncNode* fn= getEventFuncNode(cf, "onEditEvent");
    if( fn ) {
        XFuncSrc* fsrc= fn->xfunc->getFuncSrc();
        TreeNode* node = static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
        if( fsrc && node ) {
            XListArr* arrs=getLocalParams(fn);
            arrs->add()->set("geometry");
            arrs->add()->setVar('n',0,(LPVOID)node);
            arrs->add()->setVar('0',0).addInt(index.column());
            setVarRectF(arrs->add(), option.rect);
            setFuncNodeParams(fn, arrs);
            fn->call();
            StrVar* sv = fn->getResult();
            if( SVCHK('i',4) || SVCHK('i',5) ) {
                QRect rc = setQRect(sv);
                if( rc.isValid() ) {
                    editor->setGeometry(rc);
                    return;
                }
            }
        }
    }

    QLineEdit* edit=qobject_cast<QLineEdit*>(editor);
    if( edit ) {
        edit->setGeometry(option.rect);
        return;
    }

    // editor->setGeometry(option.rect);
    QStyledItemDelegate::updateEditorGeometry(editor,option, index);
}

QSize TreeDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const {
    TreeNode* cf=xproxy->xwinfo;
    XFuncNode* fn= getEventFuncNode(cf, "onRowSize");
    if( fn ) {
        TreeNode* node = static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
        // version 1.0.3
        XListArr* arrs=getLocalParams(fn);
        arrs->add()->setVar('n',0,(LPVOID)node);
        arrs->add()->setVar('0',0).addInt(index.column());
        setVarRectF(arrs->add(), option.rect);
        setFuncNodeParams(fn, arrs);
        fn->call();
        StrVar* sv = fn->getResult();
        if( SVCHK('0',0) ) {
            int w=option.rect.width(), h=sv->getInt(4);
            return QSize(w,h);
        } else if( SVCHK('i',2) ) {
            int w=sv->getInt(4),h=sv->getInt(8);
            return QSize(w,h);
        }
    }
    return QSize(option.rect.width(), xproxy->xlineHeight);
}

void TreeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    // gum.modelDataCommit(xproxy, editor, static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer()), index.column() );

    TreeNode* cf=xproxy->xwinfo;
    if( cf==NULL ) return;

    XFuncNode* fn= getEventFuncNode(cf, "onEditEvent");
    if( fn ) {
        TreeNode* node = static_cast<TreeNode*>(xproxy->mapToSource(index).internalPointer());
        if( node ) {
            // onEditEvent(type, node, index, data, editNode);
            XListArr* arrs=getLocalParams(fn);
            arrs->add()->set("finish");
            arrs->add()->setVar('n',0,(LPVOID)node);
            arrs->add()->setVar('0',0).addInt(index.column());
            StrVar* var=arrs->add();
            QLineEdit* edit=qobject_cast<QLineEdit*>(editor);
            if( edit ) {
                QString text = edit->text();
                var->set(Q2A(text));
            } else {
                cf = getWidgetConfig(editor);
                if( cf ) {
                    var->setVar('n',0,(LPVOID)cf);
                }
            }
            /*
            else {
                bool bchk = true;
                GCombo* combo=qobject_cast<GCombo*>(editor);
                if( combo ) {
                    callComboFunc("value", NULL, combo->config(), NULL, var, editor);
                    bchk = false;
                }
                if( bchk ) {
                    GInput* input=qobject_cast<GInput*>(editor);
                    if( input ) {
                        var->set( Q2A(input->text()) );
                        bchk = false;
                    }
                }
                if( bchk ) {
                    GDateEdit* date=qobject_cast<GDateEdit*>(editor);
                    if( date ) {
                        var->set( Q2A(date->date().toString("yyyy-MM-dd")) );
                        bchk = false;
                    }
                }
                if( bchk ) {
                    GSpin* spin=qobject_cast<GSpin*>(editor);
                    if( spin ) {
                        var->setVar('0',0).addInt(spin->value());
                        bchk = false;
                    }
                }
                if( bchk ) {
                    GCheck* check=qobject_cast<GCheck*>(editor);
                    if( check ) {
                        var->setVar('3', check->isChecked()? 1 : 0);
                        bchk = false;
                    }
                }
                if( bchk ) {
                    cf = getWidgetConfig(editor);
                    if( cf ) {
                        var->setVar('n',0,(LPVOID)cf);
                    }
                }
            }
            StrVar* sv = cf->gv("@editNode");
            if( SVCHK('n',0) ) {
                TreeNode* editNode=(TreeNode*)SVO;
                arrs->add()->setVar('n',0,(LPVOID)editNode);
                sv->reuse();
            }
            */

            setFuncNodeParams(fn, arrs);
            fn->call();
            return;
        }
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}
