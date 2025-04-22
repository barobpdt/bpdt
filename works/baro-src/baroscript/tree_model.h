#ifndef TREE_MODEL_H
#define TREE_MODEL_H

#include "func_util.h"
#include <QList>
#include <QVariant>
#include <QProxyStyle>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>

#define MODEL_FETCHSTART	0x10
#define MODEL_FETCHEND		0x20

#define MODEL_CAN_FETCH		1
#define MODEL_FETCH			2

// node flag
#define NF_CALL				0x40
#define NF_FINISH			0x80
#define NF_HASCHILD			0x400
#define NF_CALLBACK			0x800
#define NF_USEONE			0x8000

class TreeProxyModel;
class TreeNode;
class XFuncNode;
class TreeModel : public QAbstractItemModel  {
    Q_OBJECT
public:
    TreeModel(TreeNode* cf, QObject *parent = 0);
    ~TreeModel();
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual int		rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int		columnCount(const QModelIndex &parent = QModelIndex()) const;
    TreeNode*		getRootNode() { return xroot; }
    QModelIndex		getIndex(TreeNode* node, int idx=0) const;
    TreeNode*		setRootNode(TreeNode* root=NULL, bool reset=false );
    TreeNode*		setFields(StrVar* var=NULL);
    void			initModel();
    TreeNode*		getFieldsNode();
    TreeNode*		setFieldsNode(TreeNode* fields);
    TreeNode*		config() { return xcf; }

public:
    TreeNode*				xcf;
    TreeNode*				xroot;
    TreeNode*				xfields;
    XFuncNode*				xfnChildData;
    U32						xstate;
};

#define FILTER_NOCHILD	0x400
#define FILTER_VALID	0x100
#define FILTER_SEARCH	0x200


class TreeProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    TreeProxyModel(	TreeNode* winfo, QObject* parent=NULL);
    ~TreeProxyModel();

protected:
    bool	setData(const QModelIndex & index, const QVariant & value, int role);
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
    bool canFetchMore(const QModelIndex &) const;
    void fetchMore(const QModelIndex & midx);

public:
    virtual QVariant	data (const QModelIndex& index, int role=Qt::DisplayRole) const;
    virtual bool		lessThan(const QModelIndex &left, const QModelIndex &right) const;
    virtual bool		filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;
    Qt::ItemFlags		flags(const QModelIndex &index) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;


public:
    TreeNode*			xwinfo;
    TreeModel*			xsrcModel;
    // WHash<StrVar>		xmap;
    // ListObject<TreeNode*> xfieldNodes;
    TreeNode*			xfields;
    TreeNode*			xfetchNode;
    TreeNode*			xrootNode;
    U32					xfilterFlag;
    TreeNode*			xdraw;
    QString				xsearchString;
    QRegExp				xsearchMatcher;
    QRegExp				xwordMatcher;
    int					xlineHeight;

public:
    TreeNode* widgetConfig() { return xwinfo; }
    TreeNode* fieldNode() { return xfields; }
    void	setRootModel(TreeModel* parent);
    int		setFieldMap(StrVar* var, int sp=0, int ep=-1);

    LPCC	getMapCode(int index) const;
    int		getMapIndex(LPCC code ) const;

    TreeNode* getHeaderNode(int c, U32 flag=0 ) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;

    bool isValid() const;
    void setValid(bool b);
    QString searchString() const;
    void setSearchString(const QString &str);

public:
    void setDraw(XFuncNode* fn);
};


//
//
//

class TreeDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    TreeDelegate(TreeProxyModel* proxy, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    bool eventFilter(QObject* editor, QEvent* event);
    void	initDelegate();
public:
    TreeProxyModel*		xproxy;
    XListArr*			xarrs;
};

//
//
//

class myProxyStyle : public QProxyStyle {
public:
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const  {
        if( hint == QStyle::SH_ComboBox_Popup) {
            const GCombo* combo = qobject_cast<const GCombo*>(widget);//convert qwidget in QComboBox
            if( combo==NULL ) {
                return QProxyStyle::styleHint(hint, option, widget, returnData);
            }
            const QObjectList a = combo->children();
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
            if(hasView == true) {
                TreeNode* cf= combo->xcf;
                // ### version 1.0.4
                StrVar* sv= cf ? cf->gv("@textWidth") : NULL;
                int width = combo->width();
                if( SVCHK('0',0) ) {
                    width = qMax(width, sv->getInt(FUNC_HEADER_POS) );
                } else {
                    const QFontMetrics fontMetrics1 = view->fontMetrics();
                    const QFontMetrics fontMetrics2 = combo->fontMetrics();
                    const int iconSize = combo->iconSize().width();
                    const int j = combo->count();
                    for(int i=0; i < j; ++i) {
                        const int textWidth = qMax( fontMetrics1.width(combo->itemText(i) + "WW"),  fontMetrics2.width(combo->itemText(i) + "WW") );
                        if(combo->itemIcon(i).isNull()) {
                            width = qMax(width, textWidth);
                        } else {
                            width = qMax(width, textWidth + iconSize);

                        }
                    }
                }
                view->setFixedWidth(width);
            }
        }
        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};
#endif // TREE_MODEL_H
