#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QtGui>
#include <QLayout>
#include "listobject.h"

/************************************************************************/
/*  commonn			                                                    */
/************************************************************************/

#define SYNTAX_BLOCK    0x10000
#define NF_TABUSE       0x100
#define NF_LINENUM      0x200
#define NF_HIGHTLIGHT	0x400
/************************************************************************/
/*  widget util                                                       */
/************************************************************************/

class FlickCharmPrivate;
class FlickCharm: public QObject
{
    Q_OBJECT
public:
    FlickCharm(QObject *parent = 0);
    ~FlickCharm();
    void activateOn(QWidget *widget);
    void deactivateFrom(QWidget *widget);
    virtual bool eventFilter(QObject *object, QEvent *event);

protected:
    void timerEvent(QTimerEvent *event);
private:
    FlickCharmPrivate *d;
};

#ifdef WIDGETMAP_USE
#ifdef MPLAYER_USE
class MyMPlayer;
#endif
#ifdef WEBVIEW_USE
class WebView;
#endif
class WidgetMap;
#endif
class WidgetConf : public TreeNode {
public:
    WidgetConf();
    ~WidgetConf();
public:
#ifdef WIDGETMAP_USE
    WidgetMap* xwidgetMap;
#endif
    QWidget* xwidget;
    QLayout* xlayout;
    QObject* xparentObject;
public:
    WidgetConf* addWidgetConf(LPCC code=NULL);
};


#ifdef WIDGETMAP_USE
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Layout
//
//////////////////////////////////////////////////////////////////////////////////////////////////
class MyVBox : public QVBoxLayout {
    Q_OBJECT
public:
    MyVBox(TreeNode* cf, QWidget* p=NULL) : QVBoxLayout(p), xcf(cf) {}
    ~MyVBox() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode* xcf;
};

class MyHBox : public QHBoxLayout {
    Q_OBJECT
public:
    MyHBox(TreeNode* cf, QWidget* p=NULL) : QHBoxLayout(p), xcf(cf) {}
    ~MyHBox() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode* xcf;
};


class MyGridLayout : public QGridLayout {
    Q_OBJECT
public:
    MyGridLayout(TreeNode* cf, QWidget* p=NULL) : QGridLayout(p), xcf(cf) {}
    ~MyGridLayout() {}

public:
    TreeNode* config() { return xcf; }
    TreeNode* xcf;
};

/************************************************************************/
/*  my widget class                                                       */
/************************************************************************/

class MyCanvas;
class MyTreeModel;
class MyTreeProxyModel;
class MyTreeDelegate;
class CanvasItem;
class UserEvent;

class MyTimeLine : public QTimeLine {
    Q_OBJECT
public:
    MyTimeLine(WidgetConf* cf, QWidget* p=NULL);

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    CanvasItem* xitem;
    void setCode(LPCC code) { xcf->set("timelineCode", code); }
    LPCC getCode() { return xcf->get("timelineCode"); }
};


bool procWidgetEvent(MyCanvas* widget, QEvent* evt, WidgetConf* cf );
int setWidgetConf(WidgetConf* node);

typedef bool (*PWIDGET_INIT)( WidgetMap* map );
typedef bool (*PWIDGET_CLICK)( WidgetConf* cf );
typedef bool (*PWIDGET_CHANGE)( WidgetConf* cf );
typedef bool (*PWIDGET_REMOVE)( WidgetConf* cf );
typedef bool (*PWIDGET_POPUP)( WidgetConf* cf );
typedef bool (*PWIDGET_TIMEOUT)( WidgetConf* cf, LPCC code );
typedef bool (*PWIDGET_DRAW)( WidgetConf* cf, QPainter* painter, const QRect& rect);

typedef bool (*PWIDGET_SIZE)( WidgetConf* cf);
typedef bool (*PWIDGET_ACTION)( WidgetConf* cf);
typedef bool (*PWIDGET_LINK_CLICK)( WidgetConf* cf);
typedef bool (*PWIDGET_LINK_HOVER)( WidgetConf* cf);
typedef bool (*PWIDGET_ACTIVATED)( WidgetConf* cf);
typedef bool (*PWIDGET_SPLITTER_MOVE)( WidgetConf* cf, int w, int h);

typedef bool (*PWIDGET_KEYDOWN)( WidgetConf* cf, QKeyEvent* evt );
typedef bool (*PWIDGET_MOUSEDOWN)( WidgetConf* cf, QMouseEvent* evt );
typedef bool (*PWIDGET_MOUSEUP)( WidgetConf* cf, QMouseEvent* evt );
typedef bool (*PWIDGET_DOUBLE_CLICK)( WidgetConf* cf, QMouseEvent* evt );
typedef bool (*PWIDGET_MOUSEMOVE)( WidgetConf* cf, QMouseEvent* evt );
typedef bool (*PWIDGET_DRAGSTART)( WidgetConf* cf, QMouseEvent* evt );

typedef bool (*PWIDGET_FOCUSIN)( WidgetConf* cf );
typedef bool (*PWIDGET_FOCUSOUT)( WidgetConf* cf );
typedef bool (*PWIDGET_ACTIVATION_CHANGE)( WidgetConf* cf);
typedef bool (*PWIDGET_MOVE)( WidgetConf* cf, QMoveEvent* evt);
typedef bool (*PWIDGET_WHEEL)( WidgetConf* cf, QWheelEvent* evt);
typedef bool (*PWIDGET_DRAGENTER)( WidgetConf* cf, QDragEnterEvent* evt);
typedef bool (*PWIDGET_DRAGLEAVE)( WidgetConf* cf, QDragLeaveEvent* evt);
typedef bool (*PWIDGET_DRAGMOVE)( WidgetConf* cf, QDragMoveEvent* evt);
typedef bool (*PWIDGET_DROP)( WidgetConf* cf, QDropEvent* evt);
typedef bool (*PWIDGET_CONTEXTMENU)( WidgetConf* cf, int x, int y);
typedef bool (*PWIDGET_TIMER)( WidgetConf* cf, QTimerEvent* evt);
typedef bool (*PWIDGET_CLOSE)( WidgetConf* cf);
typedef bool (*PWIDGET_TOUCH)( WidgetConf* cf, QTouchEvent* evt);
typedef bool (*PWIDGET_TOUCHUPDATE)( WidgetConf* cf, QTouchEvent* evt);
typedef bool (*PWIDGET_TOUCHEND)( WidgetConf* cf, QTouchEvent* evt);
typedef bool (*PWIDGET_USER_EVENT)( WidgetConf* cf, UserEvent* evt);

typedef bool (*PTREE_CHANGE)( WidgetConf* cf, TreeNode* cur );
typedef bool (*PTREE_CHANGE_COLUMN)( WidgetConf* cf, TreeNode* cur );
typedef bool (*PTREE_CHANGE_SELECTION)( WidgetConf* cf, XListArr* arr );
typedef bool (*PTREE_CLICK)( WidgetConf* cf, TreeNode* cur );
typedef bool (*PTREE_DOUBLE_CLICK)( WidgetConf* cf, TreeNode* cur );
typedef bool (*PTREE_COLLAPSED)( WidgetConf* cf, TreeNode* cur );
typedef bool (*PTREE_CLICK_HEADER)( WidgetConf* cf, int logicalIndex, int ty );
typedef bool (*PTREE_SIZE_HEADER)( WidgetConf* cf, QSize* size );
typedef bool (*PTREE_DRAW_HEADER)( WidgetConf* cf, QPainter* painter, const QRect &rc, LPCC text, int logicalIndex, Qt::SortOrder sort);

typedef int (*PTREE_CHILD_DATA)( WidgetConf* cf, TreeNode* parent );
typedef bool (*PTREE_DROP_DATA)( WidgetConf* cf, TreeNode* parent, const QMimeData* mimeData, Qt::DropAction action, int row, int column );
typedef QMimeData* (*PTREE_MIME_DATA)( WidgetConf* cf, QList<TreeNode*> list );
typedef QVariant (*PTREE_TOOL_TIP)( WidgetConf* cf, TreeNode* node );
typedef bool (*PTREE_LESS_THAN)( WidgetConf* cf, TreeNode* left, TreeNode* right);
typedef bool (*PTREE_FILTER_ROW)( WidgetConf* cf, TreeNode* node);
typedef Qt::ItemFlags (*PTREE_FLAGS_ROW)( TreeNode* cf, TreeNode* node);
typedef bool (*PTREE_FETCH_MORE)( WidgetConf* cf, TreeNode* root);
typedef bool (*PTREE_DRAW_ROW)(WidgetConf* cf, QPainter *painter, TreeNode* node, const QStyleOptionViewItem &option, LPCC field);
typedef QWidget* (*PTREE_EDIT_ROW)(WidgetConf* cf, LPCC type, TreeNode* node, QWidget* editor, const QStyleOptionViewItem &option, LPCC field);
typedef QSize (*PTREE_ROW_SIZE)(WidgetConf* cf, TreeNode* node, const QStyleOptionViewItem &option, LPCC field);
typedef bool (*PTREE_FOCUS_IN)( WidgetConf* cf, QObject* editor, TreeNode* node);
typedef bool (*PTREE_FOCUS_OUT)( WidgetConf* cf, QObject* editor, TreeNode* node);
typedef bool (*PTREE_EVENT_KEYDOWN)( WidgetConf* cf, QObject* editor, TreeNode* node, QKeyEvent* evt);
typedef bool (*PTREE_EVENT_MOUSEDOWN)( WidgetConf* cf, QObject* editor, TreeNode* node, QMouseEvent* evt);


class MyCanvas : public QWidget {
    Q_OBJECT
public:
    MyCanvas(WidgetConf* cf, QWidget* p=NULL);
    ~MyCanvas();
public:
    bool startTimeLine(LPCC code, int frameEnd, int duration, LPCC mode, bool bBack=false);
    bool stopTimeLine();
    CanvasItem* canvasItems(bool resize=false);
    bool setTimeout(LPCC timerCode, int interval=500);
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    CanvasItem* xitems;
    MyTimeLine*  xtimeline;


    PWIDGET_ACTIVATION_CHANGE onActivationChange;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_MOUSEUP onMouseUp;
    PWIDGET_MOUSEMOVE onMouseMove;
    PWIDGET_CLICK onClick;
    PWIDGET_DOUBLE_CLICK onDoubleClick;

    PWIDGET_MOVE onMove;
    PWIDGET_WHEEL onWheel;
    PWIDGET_DRAGSTART onDragStart;
    PWIDGET_DRAGENTER onDragEnter;
    PWIDGET_DRAGLEAVE onDragLeave;
    PWIDGET_DRAGMOVE onDragMove;
    PWIDGET_DROP onDrop;
    PWIDGET_CONTEXTMENU onContextMenu;
    PWIDGET_TIMER onTimer;
    PWIDGET_SIZE onSize;
    PWIDGET_CLOSE onClose;
    PWIDGET_DRAW onDraw;
    PWIDGET_FOCUSIN onFocusIn;
    PWIDGET_FOCUSOUT onFocusOut;

    PWIDGET_TOUCH onTouch;
    PWIDGET_TOUCHUPDATE onTouchUpdate;
    PWIDGET_TOUCHEND onTouchEnd;
    PWIDGET_USER_EVENT onUserEvent;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotTimeOut();
    void slotTimeLineFinish();
    void updateCanvas(int frame);
protected:
    virtual bool event(QEvent* evt);
};

class MyLabel : public QLabel {
    Q_OBJECT
public:
    MyLabel(WidgetConf* cf, QWidget* p=NULL);
    ~MyLabel();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_LINK_CLICK  onLinkClick;
    PWIDGET_LINK_HOVER  onLinkHover;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotLinkActivated(QString link);
    void slotLinkHovered(QString link);
    void slotTimeOut();
protected:
    virtual bool event(QEvent* evt);
};


class MyProgressBar : public QProgressBar {
    Q_OBJECT
public:
    MyProgressBar(WidgetConf* cf, QWidget* p=NULL);
    ~MyProgressBar();
public slots:
    void slotChange( int val);
    void slotTimeOut();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_TIMEOUT onTimeOut;
};

class MyComboBox : public QComboBox {
    Q_OBJECT
public:
    MyComboBox(WidgetConf* cf, QWidget* p=NULL);
    ~MyComboBox();

public:
    WidgetConf* config() { return xcf; }
    void showPopup();
    void initModel(MyTreeModel* baseModel=NULL);
    void update();

public:
    WidgetConf* xcf;
    MyTreeModel*      xbaseModel;
    MyTreeProxyModel* xproxyModel;
    MyTreeDelegate*   xtreeDelegate;

    PWIDGET_CHANGE onChange;
    PWIDGET_ACTIVATED onAcitvated;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_WHEEL onWheel;
    PWIDGET_POPUP   onPopup;

    PTREE_CLICK         onClick;
    PTREE_DOUBLE_CLICK  onDoubleClick;
    PTREE_CHILD_DATA    onChildData;
    PTREE_TOOL_TIP      onToolTip;
    PTREE_LESS_THAN     onLessThan;
    PTREE_FILTER_ROW    onFilterRow;
    PTREE_ROW_SIZE      onRowSize;
    PTREE_DRAW_ROW      onDraw;
    PTREE_CHANGE        onChangeRow;
    PTREE_CLICK         onClickRow;
    PWIDGET_TIMEOUT     onTimeOut;

public slots:
    void slotActivated(int idx);
    void slotIndexChange(int idx);
    void slotDoubleClicked(const QModelIndex & index);
    void slotClicked ( const QModelIndex & index );
    void slotTimeOut();

protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);
};

class MyTabWidget : public QTabWidget {
    Q_OBJECT
public:
    MyTabWidget(WidgetConf* cf, QWidget* p=NULL);
    ~MyTabWidget();
public slots:
    void slotTabChange(int index);
    void slotTimeOut();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_TIMEOUT onTimeOut;
};

class MySplitter : public QSplitter {
    Q_OBJECT
public:
    MySplitter(WidgetConf* cf, QWidget* p=NULL);
    ~MySplitter();
public slots:
    void slotSplitterMove(int w, int h);
    void slotTimeOut();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_SPLITTER_MOVE onSplitterMoved;
    PWIDGET_TIMEOUT onTimeOut;
};

class MyGroupBox : public QGroupBox {
    Q_OBJECT
public:
    MyGroupBox(WidgetConf* cf, QWidget* p=NULL);
    ~MyGroupBox();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CONTEXTMENU onContext;
protected:
    virtual bool event(QEvent* evt);
};

class MyStackedWidget : public QStackedWidget {
    Q_OBJECT
public:
    MyStackedWidget(WidgetConf* cf, QWidget* p=NULL);
    ~MyStackedWidget();
public slots:
    void slotDivChange(int index);
    void slotPageRemove(int index);
    void slotTimeOut();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_REMOVE onRemove;
    PWIDGET_TIMEOUT onTimeOut;
};

class MyPushButton : public QPushButton {
    Q_OBJECT
public:
    MyPushButton(WidgetConf* cf, QWidget* p=NULL);
    ~MyPushButton();

public slots:
    void slotClicked();
    void slotTimeOut();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CLICK onClick;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_TIMEOUT onTimeOut;

protected:
    virtual bool event(QEvent* evt);
};

class MyToolButton : public QToolButton {
    Q_OBJECT
public:
    MyToolButton(WidgetConf* cf, QWidget* p=NULL);
    ~MyToolButton();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_CLICK onClick;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotClicked(bool push);
    void slotTimeOut();

protected:
    virtual bool event(QEvent* evt);
};

class MyRadioButton : public QRadioButton {
    Q_OBJECT
public:
    MyRadioButton(WidgetConf* cf, QWidget* p=NULL);
    ~MyRadioButton();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CLICK onClick;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotClicked(bool check);
    void slotTimeOut();

};

class MyCheckBox : public QCheckBox {
    Q_OBJECT
public:
    MyCheckBox(WidgetConf* cf, QWidget* p=NULL);
    ~MyCheckBox();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CLICK onClick;
    PWIDGET_CHANGE onChange;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotClicked(bool check);
    void slotStateChange(int state);
    void slotTimeOut();

};

class MySpinBox : public QSpinBox {
    Q_OBJECT
public:
    MySpinBox(WidgetConf* cf, QWidget* p=NULL);
    ~MySpinBox();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_TIMEOUT onTimeOut;
public slots:
    void slotValueChange(int value );
    void slotTimeOut();

};

class MyLineEdit : public QLineEdit {
    Q_OBJECT
public:
    MyLineEdit(WidgetConf* cf, QWidget* p=NULL);
    ~MyLineEdit();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_TIMEOUT onTimeOut;
protected:
    virtual bool event(QEvent* evt);
public slots:
    void slotTextChange();
    void slotTimeOut();
};

class MyGraphicsView : public QGraphicsView {
    Q_OBJECT
public:
    MyGraphicsView(WidgetConf* cf, QWidget* p=NULL);
    ~MyGraphicsView();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;

};

class MyPlainTextEdit : public QPlainTextEdit {
    Q_OBJECT
public:
    MyPlainTextEdit(WidgetConf* cf, QWidget* p=NULL);
    ~MyPlainTextEdit();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;

};

class MyDateEdit : public QDateEdit {
    Q_OBJECT
public:
    MyDateEdit(WidgetConf* cf, QWidget* p=NULL);
    ~MyDateEdit();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_CHANGE onChange;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotDateChage();
    void slotTimeOut();

protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);
};

class MyTimeEdit : public QTimeEdit {
    Q_OBJECT
public:
    MyTimeEdit(WidgetConf* cf, QWidget* p=NULL);
    ~MyTimeEdit();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_CHANGE onChange;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotTimeChange();
    void slotTimeOut();

protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);
};

class MyCalendarWidget : public QCalendarWidget {
    Q_OBJECT
public:
    MyCalendarWidget(WidgetConf* cf, QWidget* p=NULL);
    ~MyCalendarWidget();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CLICK onClick;
    PWIDGET_CHANGE onChange;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotChange();
    void slotPageChange(int year, int month);
    void slotClicked();
    void slotTimeOut();

protected:
    virtual bool event(QEvent* evt);
};


class MyToolBar : public QToolBar {
    Q_OBJECT
public:
    MyToolBar(WidgetConf* cf, QWidget* p=NULL);
    ~MyToolBar();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotChange();
    void slotTimeOut();

protected:
    virtual bool event(QEvent* evt);
};

class MyToolBox : public QToolBox {
    Q_OBJECT
public:
    MyToolBox(WidgetConf* cf, QWidget* p=NULL);
    ~MyToolBox();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotChange();
    void slotTimeOut();

protected:
    virtual bool event(QEvent* evt);
};


class MyDockWidget : public QDockWidget {
    Q_OBJECT
public:
    MyDockWidget(WidgetConf* cf, QWidget* p=NULL);
    ~MyDockWidget();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    PWIDGET_CHANGE onChange;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotChange();
    void slotTimeOut();

protected:
    virtual bool event(QEvent* evt);
};

class MyAction : public QAction  {
    Q_OBJECT
public:
    MyAction(WidgetConf* cf, QObject* p=NULL);
    ~MyAction();
public:
    WidgetConf* config() { return xcf; }
    WidgetConf*		xcf;
    PWIDGET_ACTION onAction;
    PWIDGET_CHANGE onChange;

public slots:
    void slotActionClick();
    void slotChange();
};

class MyMenu : public QMenu  {
    Q_OBJECT
public:
    MyMenu(WidgetConf* cf, QWidget* p=NULL);
    ~MyMenu();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf*		xcf;

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);
};

class MySystemTrayIcon : public QSystemTrayIcon  {
    Q_OBJECT
public:
    MySystemTrayIcon(WidgetConf* cf, QWidget* p=NULL);
    ~MySystemTrayIcon();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf*		xcf;

    PWIDGET_CLICK onClick;
    PWIDGET_ACTIVATED onActivated;
    PWIDGET_TIMEOUT onTimeOut;

public slots:
    void slotActivated(QSystemTrayIcon::ActivationReason reason);
    void slotClicked();
    void slotTimeOut();
};



class MyHeaderView : public QHeaderView {
    Q_OBJECT
public:
    MyHeaderView(WidgetConf* cf, Qt::Orientation orientation, QWidget * parent = 0);
public:
    int xidx;
    WidgetConf* xcf;
    PTREE_DRAW_HEADER onDrawHeader;
    PTREE_SIZE_HEADER onSizeHeader;
    PTREE_CLICK_HEADER  onClickHeader;
protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    QSize sizeHint() const;

private slots:
    void sectionClickedSlot(int index);
};

class MyTree : public QTreeView {
    Q_OBJECT
public:
    MyTree(WidgetConf* cf, QWidget* p=NULL);
    ~MyTree();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    MyTreeModel*      xbaseModel;
    MyTreeProxyModel* xproxyModel;
    MyTreeDelegate*   xtreeDelegate;
    MyHeaderView*   xheaderView;

    PTREE_CHANGE            onChange;
    PTREE_CHANGE_COLUMN     onChangeColumn;
    PTREE_CHANGE_SELECTION  onChangeSelection;
    PTREE_CLICK             onClick;
    PTREE_DOUBLE_CLICK      onDoubleClick;
    PTREE_COLLAPSED         onCollapsed;
    // header callback
    PTREE_CLICK_HEADER      onClickHeader;
    PTREE_SIZE_HEADER       onSizeHeader;
    PTREE_DRAW_HEADER       onDrawHeader;
    // model callback
    PTREE_CHILD_DATA       onChildData;
    PTREE_DROP_DATA    onDropData;
    PTREE_MIME_DATA    onMimeData;
    PTREE_TOOL_TIP     onToolTip;
    PTREE_LESS_THAN    onLessThan;
    PTREE_FILTER_ROW   onFilterRow;
    PTREE_FLAGS_ROW    onFlagsRow;
    PTREE_FETCH_MORE   onFetchMore;
    PTREE_DRAW_ROW     onDraw;
    PTREE_EDIT_ROW     onEditRow;
    PTREE_ROW_SIZE     onRowSize;
    PTREE_FOCUS_IN     onEditFocusIn;
    PTREE_FOCUS_OUT    onEditFocusOut;
    PTREE_EVENT_KEYDOWN onEditKeyDown;
    PTREE_EVENT_MOUSEDOWN  onEditMouseDown;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_MOUSEUP onMouseUp;
    PWIDGET_MOUSEMOVE onMouseMove;
    PWIDGET_WHEEL onWheel;
    PWIDGET_TIMEOUT onTimeOut;
protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);

public slots:
    void	slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotDoubleClicked(const QModelIndex & index);
    void	slotClicked ( const QModelIndex & index );
    void	slotCollapsed(const QModelIndex & index);
    void	slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    // void	slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous );
    // void	columnResized(int logicalIndex, int oldSize, int newSize);
    void    slotTimeOut();

public:
   void initModel(MyTreeModel* baseModel=NULL);
   void setRootNode(TreeNode* rootNode);
   void update();
};


class MyListView : public QListView {
    Q_OBJECT
public:
    MyListView(WidgetConf* cf, QWidget* p=NULL);
    ~MyListView();

public slots:
    void	slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotDoubleClicked(const QModelIndex & index);
    void	slotClicked ( const QModelIndex & index );
    void	slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    void    slotTimeOut();

public:
    void initModel(MyTreeModel* baseModel=NULL);
    void setRootNode(TreeNode* rootNode);
    void update();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf* xcf;
    MyTreeModel*      xbaseModel;
    MyTreeProxyModel* xproxyModel;
    MyTreeDelegate*   xtreeDelegate;

    PTREE_CHANGE            onChange;
    PTREE_CLICK             onClick;
    PTREE_DOUBLE_CLICK      onDoubleClick;
    PTREE_CHANGE_SELECTION  onChangeSelection;
    // model callback
    PTREE_CHILD_DATA   onChildData;
    PTREE_DROP_DATA    onDropData;
    PTREE_MIME_DATA    onMimeData;
    PTREE_TOOL_TIP     onToolTip;
    PTREE_LESS_THAN    onLessThan;
    PTREE_FILTER_ROW   onFilterRow;
    PTREE_FLAGS_ROW    onFlagsRow;
    PTREE_FETCH_MORE   onFetchMore;
    PTREE_DRAW_ROW     onDraw;
    PTREE_EDIT_ROW     onEditRow;
    PTREE_ROW_SIZE     onRowSize;
    PTREE_FOCUS_IN     onEditFocusIn;
    PTREE_FOCUS_OUT    onEditFocusOut;
    PTREE_EVENT_KEYDOWN onEditKeyDown;
    PTREE_EVENT_MOUSEDOWN  onEditMouseDown;
    PWIDGET_CONTEXTMENU     onContext;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_MOUSEUP onMouseUp;
    PWIDGET_MOUSEMOVE onMouseMove;
    PWIDGET_WHEEL onWheel;
    PWIDGET_TIMEOUT onTimeOut;
protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);
};

/************************************************************************/
/*   LineNumberArea                                                     */
/************************************************************************/
class MyTextEdit;
class MyTextLineNumberArea : public QWidget
{
public:
    MyTextLineNumberArea(MyTextEdit *editor);
    QSize sizeHint() const;
    U32	xflag;

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent* evt );
    void mousePressEvent(QMouseEvent* evt );
    void mouseReleaseEvent(QMouseEvent* evt );

private:
    MyTextEdit*	xedit;
};

class MyHighlightData {
public:
    MyHighlightData(QTextCharFormat tcf, U32 type=0) : fmt(tcf), xtype(type) {
    }
    QRegExp exp;
    QRegExp expEnd;
    QTextCharFormat fmt;
    U32 xtype;
    void setPattern(LPCC pattern) {
        exp.setPattern(KR(pattern));
    }
    U32 getState() { return xtype&0xFFFF; }

    void setBlockPattern(LPCC sp, LPCC ep) {
        exp.setPattern(KR(sp));
        expEnd.setPattern(KR(ep));
    }

    bool isBlockPattern() {
        return xtype&SYNTAX_BLOCK ? true : false;
    }

    QTextCharFormat& charFormat() {
        return fmt;
    }
};

class MyHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    MyHighlighter(QTextDocument *parent=0, WidgetConf* cf=0);
    ~MyHighlighter();
protected:
    void highlightBlock(const QString &text);

public:

    XParseVar						xpv;
    StrVar							xvar;
    WidgetConf*						xcf;

    MyHighlightData*					xstringHighlightData;
    ListObject<MyHighlightData*>		xcommentFmts;
    ListObject<MyHighlightData*>		xfmts;
    ListObject<MyHighlightData*>		xblockFmts;

public:
    void asHtml(QString& html);
    MyHighlightData* find(U32 type);
};

typedef bool (*PEDITOR_DRAW_LINENUM)( WidgetConf* cf, QPainter* painter, int lineNum, const QRect& rc);
typedef bool (*PEDITOR_DRAW_EDITOR)( WidgetConf* cf, QPainter* painter, const QRect& rc );
typedef bool (*PEDITOR_CHANGE_MODIFY)( WidgetConf* cf, bool modify );
typedef bool (*PEDITOR_CHANGE_CURSOR)( WidgetConf* cf, const QTextCursor* cursor );
typedef bool (*PEDITOR_CHANGE_FORMAT)( WidgetConf* cf, const QTextCharFormat* cursor );

typedef bool (*PEDITOR_CHANGE_TEXT)( WidgetConf* cf, int pos, int add, int del);
typedef bool (*PEDITOR_ENABLE_CHECK)( WidgetConf* cf, LPCC type, bool check);
typedef bool (*PEDITOR_CLICK_LINENUM)( WidgetConf* cf, QMouseEvent *evt, int selectNumber, int sp, int ep);
typedef bool (*PEDITOR_INSERT_MIME_DATA)( WidgetConf* cf, const QMimeData* source);


class MyTextEdit : public QTextEdit {
    Q_OBJECT
public:
    MyTextEdit(WidgetConf* cf, QWidget* p=NULL);
    ~MyTextEdit();

public:
    WidgetConf* config() { return xcf; }
    WidgetConf*		xcf;
    bool			xscrollLock;

    QRectF				mBlockRect;
    QTextBlock			mStartBlock;
    QTextBlock			mEndBlock;
public:
    MyTextLineNumberArea*	xlinenum;
    MyHighlighter*		xsyntax;

    PEDITOR_DRAW_EDITOR     onDrawEditor;
    PEDITOR_DRAW_LINENUM    onDrawLineNum;
    PEDITOR_CHANGE_TEXT     onChangeText;
    PEDITOR_CHANGE_MODIFY   onChangeModify;
    PEDITOR_CHANGE_CURSOR   onChangeCursor;
    PEDITOR_CHANGE_FORMAT   onChangeFormat;

    PEDITOR_ENABLE_CHECK    onEnableCheck;
    PEDITOR_CLICK_LINENUM   onClickLineNum;
    PEDITOR_INSERT_MIME_DATA    onMimeData;
    PWIDGET_CONTEXTMENU onContext;
    PWIDGET_KEYDOWN onKeyDown;
    PWIDGET_MOUSEDOWN onMouseDown;
    PWIDGET_MOUSEUP onMouseUp;
    PWIDGET_MOUSEMOVE onMouseMove;
    PWIDGET_DOUBLE_CLICK onDoubleClick;
    PWIDGET_WHEEL onWheel;
    PWIDGET_TIMEOUT onTimeOut;

public:
    void paintEvent(QPaintEvent *event);
    void resizeEvent( QResizeEvent* evt );
    void scrollContentsBy(int dx, int dy);
    void zoomIn(int range);
    void zoomOut(int range);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void extraAreaMouseEvent(QMouseEvent *evt);
    void setCharFormat(LPCC code, TreeNode* cf);
    void setScrollPosition(int ypos, int xpos=-1);
    void setFrameMargin(int left, int top, int right, int bottom);
    void blockScrollTop(QTextBlock block);
    void slotTimeOut();
public:
    QRectF& getBlockRect(QTextBlock& block);
    QTextBlock* getFirstBlock(int ep=-1);
    QTextBlock* getStartBlock(int sp, int ep=-1);
    QTextBlock* getLineBlock(int startLine, int endLine=-1);
    QTextBlock* nextBlock(QTextBlock* block);


    bool canInsertFromMimeData(const QMimeData* source) const;
    void insertFromMimeData(const QMimeData* source);
    void dropImage(const QUrl& url, const QImage& image);
    void dropTextFile(const QUrl& url);

public slots:
    void textCurrentCharFormatChanged(const QTextCharFormat &format);
    void textCursorPositionChanged();
    void textClipboardDataChanged();
    void textModificationChanged(bool modify);
    void printPreview(QPrinter *);

    void textContentsChange(int,int,int);
    void slotTextChanged ();
    void slotUndoAvailable ( bool available );
    void slotCopyAvailable ( bool available );
    void slotRedoAvailable ( bool available );
    void updateLineNumberArea(const QRect &rect, int dy);
    void updateLineNumberAreaWidth(int newBlockCount);
    void scrollValueChange(int dy);

protected:
    virtual bool eventFilter(QObject* object, QEvent* evt);
};

class WidgetMap : public TreeNode {
public:
    WidgetMap(QWidget* parent);
    ~WidgetMap();

    TreeNode xmap;
    TreeNode xcf;
    StrVar xresult;
    WidgetConf* xrootConf;
    QWidget* parentWidget;
    QTimer xtimer;

protected:
    void setLayoutProp(WidgetConf* cf, QLayout* layout, LPCC tag);

public:
    bool initPage(LPCC uri, PWIDGET_INIT fpInit=NULL);
    TreeNode* pageMap() { return &xmap; }
    TreeNode* pageConf() { return &xcf; }
    QLayout* createLayout(WidgetConf* layout, QWidget* parent);
    QWidget* createWidget(WidgetConf* wcf, QWidget* parent);
    bool createChild(WidgetConf* root, QWidget* parent);
    bool isLayoutNode(TreeNode* node);
    WidgetConf* findLayoutNode(WidgetConf* node);
    WidgetConf* findParentNode(WidgetConf* node);
    WidgetConf* findId(LPCC id);
    WidgetConf* findTag(LPCC tag, int index=0, WidgetConf* parent=NULL);
    bool setWidgetScroll(WidgetConf* cf );
    int setClickEvent(PWIDGET_CLICK fpClick);
    int setChangeEvent(PWIDGET_CHANGE fpChange);
    int setLinkEvent(PWIDGET_LINK_CLICK fpClick, PWIDGET_LINK_HOVER fpHover=NULL );
    int startTimeOut(LPCC id, int interval=500, PWIDGET_TIMEOUT fpTimeOut=NULL );
    int stopTimeOut();

    QWidget* widget(LPCC code, QWidget* w=NULL);
    MyVBox* vbox(LPCC code, WidgetConf* wcf=NULL);
    MyHBox* hbox(LPCC code, WidgetConf* wcf=NULL);
    MyGridLayout* grid(LPCC code, WidgetConf* wcf=NULL);
    MyCanvas* canvas(LPCC code, WidgetConf* wcf=NULL);
    MyTree* tree(LPCC code, WidgetConf* wcf=NULL);
    MyTextEdit* editor(LPCC code, WidgetConf* wcf=NULL);
    MyLabel* label(LPCC code, WidgetConf* wcf=NULL);
    MyProgressBar* progress(LPCC code, WidgetConf* wcf=NULL);
    MyComboBox* combo(LPCC code, WidgetConf* wcf=NULL);
    MyListView* list(LPCC code, WidgetConf* wcf=NULL);
    MyTabWidget* tab(LPCC code, WidgetConf* wcf=NULL);
    MySplitter* splitter(LPCC code, WidgetConf* wcf=NULL);
    MyGroupBox* group(LPCC code, WidgetConf* wcf=NULL);
    MyStackedWidget* div(LPCC code, WidgetConf* wcf=NULL);
    MyPushButton* button(LPCC code, WidgetConf* wcf=NULL);
    MyToolButton* tool(LPCC code, WidgetConf* wcf=NULL);
    MyRadioButton* radio(LPCC code, WidgetConf* wcf=NULL);
    MyCheckBox* check(LPCC code, WidgetConf* wcf=NULL);
    MySpinBox* spin(LPCC code, WidgetConf* wcf=NULL);
    MyLineEdit* input(LPCC code, WidgetConf* wcf=NULL);
    MyGraphicsView* graphics(LPCC code, WidgetConf* wcf=NULL);
    MyPlainTextEdit* textarea(LPCC code, WidgetConf* wcf=NULL);
    MyDateEdit* date(LPCC code, WidgetConf* wcf=NULL);
    MyTimeEdit* time(LPCC code, WidgetConf* wcf=NULL);
    MyCalendarWidget* calendar(LPCC code, WidgetConf* wcf=NULL);
    MyToolBar* toolbar(LPCC code, WidgetConf* wcf=NULL);
    MyToolBox* toolbox(LPCC code, WidgetConf* wcf=NULL);
    MyDockWidget* dock(LPCC code, WidgetConf* wcf=NULL);
    MyAction* action(LPCC code, WidgetConf* wcf=NULL);
    MyMenu* menu(LPCC code=NULL, WidgetConf* wcf=NULL);
    MySystemTrayIcon* tray(LPCC code=NULL, WidgetConf* wcf=NULL);
#ifdef MPLAYER_USE
    MyMPlayer* player(LPCC code, WidgetConf* wcf=NULL);
#endif
#ifdef WEBVIEW_USE
    WebView* webview(LPCC code, WidgetConf* wcf=NULL);
#endif
    QProgressDialog* progressDialog(LPCC label=NULL, LPCC title=NULL);
    QColorDialog* colors();

protected:
    WidgetConf* wConf(LPCC code, LPCC tag, U16 kind, WidgetConf* wcf=NULL);
};

bool initPage(WidgetMap* ui);
#endif

#endif // MYWIDGET_H
