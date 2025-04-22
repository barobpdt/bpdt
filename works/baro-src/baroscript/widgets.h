#ifndef WIDGETS_H
#define WIDGETS_H
#include <QtGui>
#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDialog>
#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QToolBox>
#include <QCalendarWidget>
#include <QDateEdit>
#include <QTimeEdit>
#include <QHeaderView>
#include <QTreeView>
#include <QComboBox>
#include <QItemDelegate>
#include <QListView>
#include <QTableView>
#include <QSplitter>
#include <QSplitterHandle>
#include <QGroupBox>
#include <QStackedWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCompleter>
#include <QGraphicsOpacityEffect>

#include "XFunc.h"
#include "../util/canvas_item.h"
#include "../util/mywidget.h"

#define PAGE_NODE_USE
bool isLayoutNode(TreeNode* node);
bool isPageNode(TreeNode* node);

QWidget* gwidget( TreeNode* cf );
QLayout* getLayout(TreeNode* cf, U16* stat=NULL);
QWidget* getWidget(TreeNode* cf);
void setWidget(TreeNode* cf, QWidget* widget, U16 stat);
void setLayout(TreeNode* cf, QLayout* layout, U16 stat);
bool setScroll(TreeNode* cf);
TreeNode* findChildId( TreeNode* root, LPCC id );
TreeNode* findWidgetId( TreeNode* root, LPCC id, int mode=0 );
TreeNode* findWidgetTag( TreeNode* root, LPCC tag, XListArr* arr=NULL  );
TreeNode* findWidgetNode(TreeNode*cf, U16 stat=0);
XListArr* getWidgetArr(TreeNode* root, XListArr* a );
bool procWidgetEvent(QEvent* evt, TreeNode* cf, QWidget* curWidget, U32 flag=0 );
QLayout* createLayout(TreeNode* cf, QWidget* parent=NULL );
QWidget* createWidget(LPCC tag, TreeNode* cf, QWidget* parent=NULL );
bool createWidgetChild(TreeNode* root, QWidget* parent=NULL, int depth=0);

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Layout
//
//////////////////////////////////////////////////////////////////////////////////////////////////
class VBox : public QVBoxLayout {
    Q_OBJECT
public:
    VBox(TreeNode* cf, QWidget* p=NULL) : QVBoxLayout(p), xcf(cf) {}
    ~VBox() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
};


class HBox : public QHBoxLayout {
    Q_OBJECT
public:
    HBox(TreeNode* cf, QWidget* p=NULL) : QHBoxLayout(p), xcf(cf) {}
    ~HBox() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

};


class GridLayout : public QGridLayout {
    Q_OBJECT
public:
    GridLayout(TreeNode* cf, QWidget* p=NULL) : QGridLayout(p), xcf(cf) {}
    ~GridLayout() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

};

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Widget
//
//////////////////////////////////////////////////////////////////////////////////////////////////
class TreeProxyModel;
class GViewEventFilter: public QObject {
    Q_OBJECT
public:
    GViewEventFilter(QWidget* view, TreeNode* cf): QObject(view), xview(view), xcf(cf) {
    };
    bool eventFilter(QObject* obj, QEvent* evt)  {
        return procWidgetEvent(evt, xcf, qobject_cast<QWidget*>(obj), 1);
    }
private:
    QWidget*		xview;
    TreeNode*		xcf;
};

class GMainPage : public QMainWindow  {
    Q_OBJECT
public:
    GMainPage(TreeNode* cf, QWidget* p=NULL) : QMainWindow(p), xcf(cf) {}
    ~GMainPage() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QMainWindow::event(evt);
    }
};

class GDialog : public QDialog {
    Q_OBJECT
public:
    GDialog(TreeNode* cf, QWidget* p=NULL) : QDialog(p), xcf(cf) {}
    ~GDialog() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QDialog::event(evt);
    }
};

class GCanvas : public QLabel {
    Q_OBJECT
public:
    GCanvas(TreeNode* cf, QWidget* p=NULL);
    ~GCanvas();
public:
    bool startTimeLine(LPCC code, int frameEnd, int duration, LPCC mode, bool bBack=false);
    bool stopTimeLine();
    bool isTimeLine() { return xcurrentTimeline? true: false; }
    MyTimeLine* setTimeline(LPCC code, StrVar* rst);
    MyTimeLine* nextTimeline(StrVar* rst, bool rewind=false);
    MyTimeLine* prevTimeline(StrVar* rst, bool rewind=false);
    CanvasItem* canvasItems(WidgetConf* root=NULL);
    bool setLayout(StrVar* sv=NULL, int sp=0, int ep=-1);
    bool setTouchMode(bool on=true);
    bool draw(QPainter* painter, const QRectF& rect);
    bool drawDefault(QPainter* painter, const QRectF& rect );
    WidgetConf* widgetConf() { return &xwidgetConf; }
    TreeNode* drawObject() { return &xdrawObject; }
    CanvasItem* currentItem() { return xpopupItem? xpopupItem: xdivItem? xdivItem: xpageItem; }
    bool invalidate(bool recalc=false, bool all=false);
    bool showPopup(LPCC id, QRectF rect);
    bool showDiv(LPCC id, QRectF rect);
    bool showWidget(LPCC id, QRectF rect);
    void hidePopup();
    void hideDiv();
    bool hideWidget(LPCC id=NULL);
    TreeNode* widgets();
    bool addWidgets(TreeNode* root, StrVar* var, int sp, int ep );
public:
    TreeNode* config() { return xcf; }
    TreeNode*       xcf;
    WidgetConf      xwidgetConf;
    CanvasItem*     xcanvasItems;
    CanvasItem*     xpageItem;
    CanvasItem*     xpopupItem;
    CanvasItem*     xdivItem;
    ListObject<TreeNode*>	xshowWidgets;
    MyTimeLine*     xcurrentTimeline;
    TreeNode        xdrawObject;
    XListArr        xtimelines;

public slots:
    void slotTimeLineFinish();
    void updateCanvas(int frame);

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QLabel::event(evt);
    }
};

class GPage : public QWidget {
    Q_OBJECT
public:
    GPage(TreeNode* cf, QWidget* p=NULL) : QWidget(p), xcf(cf) {}
    ~GPage() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QWidget::event(evt);
    }
};

class GWidget : public QWidget {
    Q_OBJECT
public:
	GWidget(TreeNode* cf, QWidget* p=NULL);
    ~GWidget() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
	QGraphicsOpacityEffect fade_effect;
	QPropertyAnimation animation;
public:
	bool openOverlay(int duration);
	void showOverlay(const bool& show);
	void newParent();

protected:
	bool eventFilter(QObject* obj, QEvent* ev) override;
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QWidget::event(evt);
    }
};

// QOpenGLWidget

class GContext : public QOpenGLWidget, protected QOpenGLFunctions_1_1 {
    Q_OBJECT
public:
    GContext(TreeNode* cf, QWidget* p=NULL);
    ~GContext() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
    QGraphicsOpacityEffect fade_effect;
    QPropertyAnimation animation;
    QTimer _qTimer;
    GLuint _idTex;
    int _contextWidth;
    int _contextHeight;
    int _interval;
public:
    bool openOverlay(int duration);
    void showOverlay(const bool& show);
    void newParent();
    void startTimer(int delay);
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QOpenGLWidget::event(evt);
    }
private slots:
    void timeout();
};



class GLabel : public QLabel {
    Q_OBJECT
public:
    GLabel(TreeNode* cf, QWidget* p=NULL) : QLabel(p), xcf(cf) {}
    ~GLabel() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QLabel::event(evt);
    }
};

class GProgress : public QProgressBar {
    Q_OBJECT
public:
    GProgress(TreeNode* cf, QWidget* p=NULL);
    ~GProgress() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotValueChanged( int value );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QProgressBar::event(evt);
    }
};


class ComboDelegate : public QItemDelegate {
public:
    TreeNode*		xcf;
    XFuncNode*		xfnDraw;
    TreeNode*		xdraw;
    TreeProxyModel*	xproxy;
    bool			xuse;
    int lineHeight;
    ComboDelegate(TreeNode* cf, TreeProxyModel* proxy=NULL);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};
class GCombo : public QComboBox {
    Q_OBJECT
public:
    GCombo(TreeNode* cf, QWidget* p=NULL);
    ~GCombo() {
        qDebug("combo close ~~");
        xcf->reuse();
    }
    void showPopup();

public slots:
    void	slotActivated ( int index );
    void	slotCurrentIndexChanged ( int index );

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QComboBox::event(evt);
    }
    virtual bool eventFilter(QObject* object, QEvent* event);
};


class GHeaderView : public QHeaderView {
    Q_OBJECT
public:
    GHeaderView(TreeNode* cf, Qt::Orientation orientation, QWidget * parent = 0);
public:
    int xidx;
    TreeNode* xcf;
protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    QSize sizeHint() const;
public:
    void clickDetailHeader(int index);

private slots:
    void sectionClickedSlot(int index);
};


class GTree : public QTreeView {
    Q_OBJECT
public:
    GTree(TreeNode* cf, QWidget* p=NULL);
    ~GTree() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
    void	slotDoubleClicked(const QModelIndex & index);
    void	slotClicked ( const QModelIndex & index );
    void	slotCollapsed(const QModelIndex & index);
    void	columnResized(int logicalIndex, int oldSize, int newSize);
    void	slotClickHeader(int index );


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QTreeView::event(evt);
    }
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    virtual void dropEvent(QDropEvent *event);
    virtual void startDrag(Qt::DropActions act);
};

class GListVIew : public QListView {
    Q_OBJECT
public:
    GListVIew(TreeNode* cf, QWidget* p=NULL);
    ~GListVIew() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


public slots:
    void	slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
    void	slotDoubleClicked(const QModelIndex & index);
    void	slotClicked ( const QModelIndex & index );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QListView::event(evt);
    }
};

class GTable : public QTableView {
    Q_OBJECT
public:
    GTable(TreeNode* cf, QWidget* p=NULL);
    ~GTable() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


public slots:
    void	slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous );
    void	slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
    void	slotDoubleClicked(const QModelIndex & index);
    void	slotClicked ( const QModelIndex & index );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QTableView::event(evt);
    }
};


class GTab : public QTabWidget {
    Q_OBJECT
public:
    GTab(TreeNode* cf, QWidget* p=NULL);
    ~GTab() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


public slots:
    void	slotCurrentChanged ( int index );
    void	slotTabCloseRequested ( int index );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QTabWidget::event(evt);
    }
};

class GSplitter : public QSplitter {
    Q_OBJECT
public:
    GSplitter(TreeNode* cf, QWidget* p=NULL);
    ~GSplitter() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotSplitterMoved ( int pos, int index );
    QSplitterHandle* createHandle();
protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QSplitter::event(evt);
    }
};
class GSplitterHandle : public QSplitterHandle {
    Q_OBJECT
public:
    GSplitterHandle(TreeNode* cf, Qt::Orientation ori, QSplitter* p=NULL);
    ~GSplitterHandle() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
protected:
    void paintEvent(QPaintEvent *event);
};

class GGroup : public QGroupBox {
    Q_OBJECT
public:
    GGroup(TreeNode* cf, QWidget* p=NULL) : QGroupBox(p), xcf(cf) {}
    ~GGroup() { }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QGroupBox::event(evt);
    }
};

class GStacked : public QStackedWidget {
    Q_OBJECT
public:
    GStacked(TreeNode* cf, QWidget* p=NULL);
    ~GStacked() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotCurrentChanged( int index );
    void	slotWidgetRemoved( int index );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QStackedWidget::event(evt);
    }
};


class GToolButton : public QToolButton {
    Q_OBJECT
public:
    GToolButton(TreeNode* cf, QWidget * p=NULL);
    ~GToolButton() { }

public:

    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotClicked ( bool checked = false );
    void	slotToggled ( bool checked );

};


class GButton : public QPushButton {
    Q_OBJECT
public:
    GButton(TreeNode* cf, QWidget * p=NULL);
    ~GButton() {
        xcf->reuse();
    }

public:

    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotClicked();

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QPushButton::event(evt);
    }
};


class GRadio : public QRadioButton {
    Q_OBJECT
public:
    GRadio(TreeNode* cf, LPCC txt, QWidget* p=NULL);
    ~GRadio() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


public slots:
    void	slotClicked ( bool checked );
    void	slotToggled ( bool checked );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QRadioButton::event(evt);
    }
};

class GCheck : public QCheckBox {
    Q_OBJECT
public:
    GCheck(TreeNode* cf, QWidget* p=NULL);
    ~GCheck() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotClicked ( bool checked );
    void	slotToggled ( bool checked );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QCheckBox::event(evt);
    }
};

class GSpin : public QSpinBox {
    Q_OBJECT
public:
    GSpin(TreeNode* cf, QWidget* p=NULL);
    ~GSpin() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


public slots:
    void	slotValueChanged ( const QString & text );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QSpinBox::event(evt);
    }
};


class GInput : public QLineEdit {
    Q_OBJECT
public:
    GInput(TreeNode* cf, QWidget* p=NULL);
    ~GInput() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void	slotCursorPositionChanged ( int prev, int cur );
    void	slotEditingFinished ();
    void	slotReturnPressed ();
    void	slotSelectionChanged ();
    void	slotTextChanged ( const QString & text );
    void	slotTextEdited ( const QString & text );

protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QLineEdit::event(evt);
    }
};

/************************************************************************/
/*   LineNumberArea                                                     */
/************************************************************************/
class GTextEdit;
class TextLineNumberArea : public QWidget
{
public:
    TextLineNumberArea(GTextEdit *editor);
    QSize sizeHint() const;
    U32			xflag;

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent* evt );
    void mousePressEvent(QMouseEvent* evt );
    void mouseReleaseEvent(QMouseEvent* evt );

private:
    GTextEdit*	xedit;
};

class XHighlightData {
public:
    XHighlightData(QTextCharFormat tcf, U32 type=0) : fmt(tcf), xtype(type) {
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

class XHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    XHighlighter(QTextDocument *parent=0, TreeNode* cf=0);
    ~XHighlighter();
protected:
    void highlightBlock(const QString &text);

public:
#ifdef TEXTMATCH_USE
    struct MatchWords {
        MatchWords() : scnt(0), ecnt(0), npos(0), epos(0) {}
        StrVar st, et;
        int sctn, ecnt;
        int npos;, epos;
    };
    ListObject<MatchWords>			xmatchs;
    ListObject<SelectedText*>		xsels;
#endif
    XParseVar						xpv;
    StrVar							xvar;
    TreeNode*						xcf;

    XHighlightData*					xstringHighlightData;
    ListObject<XHighlightData*>		xcommentFmts;
    ListObject<XHighlightData*>		xfmts;
    ListObject<XHighlightData*>		xblockFmts;

public:
#ifdef TEXTMATCH_USE
    void addMatch(LPCC st, LPCC et);
#endif
    void asHtml(QString& html);
};

class XFuncNode;
class GTextEdit : public QTextEdit {
    Q_OBJECT
public:
    GTextEdit(TreeNode* cf, QWidget* p=NULL);
    ~GTextEdit() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
    bool			xscrollLock;


    QRectF				mBlockRect;
    QPointF				mOffsetPt;
    QTextBlock			mStartBlock;
    QTextBlock			mEndBlock;
    QTextBlock::iterator	mBlockIterator;
    QTextFragment		mNextFragment;
    QTextCharFormat		mCharFormat;
    TextLineNumberArea*	xlinenum;
    XHighlighter*		xsyntax;

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

public:
    QRectF& getBlockRect(QTextBlock& block);
    QPointF& getContentOffset();
    QTextBlock* getFirstBlock(int ep=-1);
    QTextBlock* getStartBlock(int sp, int ep=-1);
    QTextBlock* getLineBlock(int startLine, int endLine=-1);
    QTextBlock* nextBlock(QTextBlock* block);
    QTextFragment* getFragment(int pos=-1);
    QTextFragment* nextFragment();


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
    void slotCopyAvailable ( bool yes );
    void slotRedoAvailable ( bool available );
    void updateLineNumberArea(const QRect &rect, int dy);
    void updateLineNumberAreaWidth(int newBlockCount);
    void scrollValueChange(int dy);

protected:
    virtual bool event(QEvent* evt);
};

class GDateEdit : public QDateEdit {
    Q_OBJECT
    Q_PROPERTY(bool nullable READ isNullable WRITE setNullable)

public:
    GDateEdit(TreeNode* cf, QWidget* p=NULL);
    ~GDateEdit() {
        xcf->reuse();
    }

    QDateTime dateTime() const;
    QDate date() const;
    QTime time() const;

    bool isNullable() const;
    void setNullable(bool enable);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QDateEdit::event(evt);
    }
protected:
   void showEvent(QShowEvent *event);
   void resizeEvent(QResizeEvent *event);
   void paintEvent(QPaintEvent *event);
   void keyPressEvent(QKeyEvent *event);
   void mousePressEvent(QMouseEvent *event);
   bool focusNextPrevChild(bool next);
   QValidator::State validate(QString &input, int &pos) const;

public Q_SLOTS:
   void setDateTime(const QDateTime &dateTime);
   void setDate(const QDate &date);
   void setTime(const QTime &time);

private slots:
    void clearButtonClicked();

private:
    Q_DISABLE_COPY(GDateEdit)
    class Private;
    friend class Private;
    Private* d;


};
class GTimeEdit : public QTimeEdit {
    Q_OBJECT
public:
    GTimeEdit(TreeNode* cf, QWidget* p=NULL) : QTimeEdit(p), xcf(cf) {}
    ~GTimeEdit() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QTimeEdit::event(evt);
    }
};

class GCalendar : public QCalendarWidget {
    Q_OBJECT
public:
    GCalendar(TreeNode* cf, QWidget* p=NULL) : QCalendarWidget(p), xcf(cf) {}
    ~GCalendar() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QCalendarWidget::event(evt);
    }
};

/************************************************************************/
/*  toolbar, toolbox, dockwidget (21,22,23)                            */
/************************************************************************/
class GToolBar : public QToolBar {
    Q_OBJECT
public:
    GToolBar(TreeNode* cf, QWidget * parent=0) : QToolBar(parent), xcf(cf) {}
    ~GToolBar() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QToolBar::event(evt);
    }
};

class GToolBox : public QToolBox {
    Q_OBJECT
public:
    GToolBox(TreeNode* cf, QWidget * parent=0) : QToolBox(parent), xcf(cf) {}
    ~GToolBox() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;


protected:
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QToolBox::event(evt);
    }
};

/************************************************************************/
/*  ACTION MENU TRAY Completer (11,12,13,14)                            */
/************************************************************************/
class GAction : public QAction  {
    Q_OBJECT
public:
    GAction(TreeNode* cf, QObject* p=NULL);
    ~GAction() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

public slots:
    void slotActionClick();

};



class GMenu : public QMenu  {
    Q_OBJECT
public:
    GMenu(TreeNode* cf, QWidget* p=NULL) : QMenu(p), xcf(cf) {}
    ~GMenu() {
        xcf->reuse();
    }

public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;

protected:
    virtual void mouseReleaseEvent(QMouseEvent *e);
};

class GTray : public QSystemTrayIcon {
    Q_OBJECT
public:
    GTray(TreeNode* cf, QObject* p=NULL);
    ~GTray();
public:
    TreeNode* config() { return xcf; }
    TreeNode*		xcf;
    XListArr		xparam;


public slots:
    void trayMessageClicked();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
};

class GCompleter : public QCompleter {
    Q_OBJECT
public:
    GCompleter(TreeNode* cf, QObject* p=NULL) : QCompleter(p), xcf(cf) {
    }
    ~GCompleter() {
    }
public:
    TreeNode* config() { return xcf; }
    TreeNode*	xcf;
};


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Widget UTIL
//
//////////////////////////////////////////////////////////////////////////////////////////////////

class DirFileInfo {
public:
    DirFileInfo(QString path) : dir(path), infoIndex(0) {}
    QDir dir;
    QFileInfoList list;
    QFileInfo info;
    int infoIndex;
public:
    int setDir(U32 sortFlag, U32 fliterFlag);
    void nameFilters(StrVar* sv);
    QFileInfo* next();
};

class GInputCompletion : public QObject {
    Q_OBJECT
public:
    GInputCompletion(QLineEdit* parent, TreeNode* cf, GTree* tree=NULL);
    ~GInputCompletion();
    void setPopup(GTree* tree);
    GTree* getPopup() { return popup; }
public:
    TreeNode*	xcf;
    StrVar		xrst;
    XListArr	xparam;
public slots:
    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
    void showPopup();
protected:
    virtual bool eventFilter(QObject* object, QEvent* event);
private:
    QLineEdit *editor;
    GTree *popup;
    QTimer *timer;
};
QWidget* createPageVar(TreeNode* tn, StrVar* var, int sp=0, int ep=0);
GPage* createPage(LPCC pageId, LPCC fileName, StrVar* rst=NULL);
bool addMenuActions(GMenu* menu, XListArr* a, LPCC code, TreeNode* thisNode=NULL, XListArr* menuActions=NULL);
void setEditorCharFormat(XHighlighter* syntax, LPCC code, TreeNode* cf, TreeNode* config, QWidget* widget );
#endif // WIDGETS_H
