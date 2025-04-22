#include "mywidget.h"
#include "widget_util.h"
#include "user_event.h"
#include <QApplication>
#include <QAbstractScrollArea>
#include <QScrollBar>

#ifdef WIDGETMAP_USE
#include "editor_util.h"
#include "treemodel.h"
#ifdef MPLAYER_USE
#include "widget_myplayer.h"
#endif
#ifdef WEBVIEW_USE
#include "widget_webview.h"
#endif
#endif

//
// FickCharm (touch scroll )
//
const int fingerAccuracyThreshold = 3;


WidgetConf::WidgetConf() : TreeNode(2) {
    set("@objectType", "WidgetConf");
#ifdef WIDGETMAP_USE
    xwidgetMap=NULL;
#endif
    xwidget=NULL;
    xlayout=NULL;
    xparentObject=NULL;
}

WidgetConf::~WidgetConf() {
}

WidgetConf *WidgetConf::addWidgetConf(LPCC code) {
    WidgetConf* node=NULL;
    StrVar* sv=NULL;
    if( slen(code) ) {
        sv=gv(code);
        if( SVCHK('n',1) ) {
            node=(WidgetConf*)SVO;
        }
    }
#ifdef NODE_OBJECT_USE
    if( node==NULL ) {
        node = new WidgetConf();
        node->xparent=(TreeNode*)this;
        node->setNodeFlag(FLAG_NEW);
        xobjects.add()->setVar('n',1,(LPVOID)node );
        xchilds.add((TreeNode*)node);
        if( slen(code) ) {
            GetVar(code)->setVar('n',1,(LPVOID)node );
        }
    }
#endif
    return node;
}


struct FlickData {
    typedef enum {
        Steady, // Interaction without scrolling
        ManualScroll, // Scrolling manually with the finger on the screen
        AutoScroll, // Scrolling automatically
        AutoScrollAcceleration // Scrolling automatically but a finger is on the screen
    } State;
    State state;
    QWidget *widget;
    QPoint pressPos;
    QPoint lastPos;
    QPoint speed;
    QTime speedTimer;
    QList<QEvent*> ignored;
    QTime accelerationTimer;
    bool lastPosValid:1;
    bool waitingAcceleration:1;

     FlickData()
         : lastPosValid(false)
         , waitingAcceleration(false)
     {}

     void resetSpeed()
     {
         speed = QPoint();
         lastPosValid = false;
     }
     void updateSpeed(const QPoint &newPosition)
     {
         if (lastPosValid) {
             const int timeElapsed = speedTimer.elapsed();
             if (timeElapsed) {
                 const QPoint newPixelDiff = (newPosition - lastPos);
                 const QPoint pixelsPerSecond = newPixelDiff * (1000 / timeElapsed);
                 // fingers are inacurates, we ignore small changes to avoid stopping the autoscroll because
                 // of a small horizontal offset when scrolling vertically
                 const int newSpeedY = (qAbs(pixelsPerSecond.y()) > fingerAccuracyThreshold) ? pixelsPerSecond.y() : 0;
                 const int newSpeedX = (qAbs(pixelsPerSecond.x()) > fingerAccuracyThreshold) ? pixelsPerSecond.x() : 0;
                 if (state == AutoScrollAcceleration) {
                     const int max = 4000; // px by seconds
                     const int oldSpeedY = speed.y();
                     const int oldSpeedX = speed.x();
                     if ((oldSpeedY <= 0 && newSpeedY <= 0) ||  (oldSpeedY >= 0 && newSpeedY >= 0)
                         && (oldSpeedX <= 0 && newSpeedX <= 0) ||  (oldSpeedX >= 0 && newSpeedX >= 0)) {
                         speed.setY(qBound(-max, (oldSpeedY + (newSpeedY / 4)), max));
                         speed.setX(qBound(-max, (oldSpeedX + (newSpeedX / 4)), max));
                     } else {
                         speed = QPoint();
                     }
                 } else {
                     const int max = 2500; // px by seconds
                     // we average the speed to avoid strange effects with the last delta
                     if (!speed.isNull()) {
                         speed.setX(qBound(-max, (speed.x() / 4) + (newSpeedX * 3 / 4), max));
                         speed.setY(qBound(-max, (speed.y() / 4) + (newSpeedY * 3 / 4), max));
                     } else {
                         speed = QPoint(newSpeedX, newSpeedY);
                     }
                 }
             }
         } else {
             lastPosValid = true;
         }
         speedTimer.start();
         lastPos = newPosition;
     }

     // scroll by dx, dy
     // return true if the widget was scrolled
     bool scrollWidget(const int dx, const int dy)
     {
         QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
         if (scrollArea) {
             const int x = scrollArea->horizontalScrollBar()->value();
             const int y = scrollArea->verticalScrollBar()->value();
             scrollArea->horizontalScrollBar()->setValue(x - dx);
             scrollArea->verticalScrollBar()->setValue(y - dy);
             return (scrollArea->horizontalScrollBar()->value() != x
                     || scrollArea->verticalScrollBar()->value() != y);
         }
         /*
         QWebView *webView = qobject_cast<QWebView*>(widget);
         if (webView) {
             QWebFrame *frame = webView->page()->mainFrame();
             const QPoint position = frame->scrollPosition();
             frame->setScrollPosition(position - QPoint(dx, dy));
             return frame->scrollPosition() != position;
         }
         */
         return false;
     }

     bool scrollTo(const QPoint &newPosition)
     {
         const QPoint delta = newPosition - lastPos;
         updateSpeed(newPosition);
         return scrollWidget(delta.x(), delta.y());
     }
};



class FlickCharmPrivate
{
public:
    QHash<QWidget*, FlickData*> flickData;
    QBasicTimer ticker;
    QTime timeCounter;
    void startTicker(QObject *object)
    {
        if (!ticker.isActive())
            ticker.start(15, object);
        timeCounter.start();
    }
};

FlickCharm::FlickCharm(QObject *parent): QObject(parent)
{
    d = new FlickCharmPrivate;
}

FlickCharm::~FlickCharm()
{
    delete d;
}

void FlickCharm::activateOn(QWidget *widget)
{
    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *viewport = scrollArea->viewport();

        viewport->installEventFilter(this);
        scrollArea->installEventFilter(this);

        d->flickData.remove(viewport);
        d->flickData[viewport] = new FlickData;
        d->flickData[viewport]->widget = widget;
        d->flickData[viewport]->state = FlickData::Steady;

        return;
    }
    /*
    QWebView *webView = qobject_cast<QWebView*>(widget);
    if (webView) {
        QWebFrame *frame = webView->page()->mainFrame();
        frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
        frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

        webView->installEventFilter(this);

        d->flickData.remove(webView);
        d->flickData[webView] = new FlickData;
        d->flickData[webView]->widget = webView;
        d->flickData[webView]->state = FlickData::Steady;

        return;
    }
    */
    qWarning() << "FlickCharm only works on QAbstractScrollArea (and derived classes)";
    qWarning() << "or QWebView (and derived classes)";
}

void FlickCharm::deactivateFrom(QWidget *widget)
{
    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(widget);
    if (scrollArea) {
        QWidget *viewport = scrollArea->viewport();

        viewport->removeEventFilter(this);
        scrollArea->removeEventFilter(this);

        delete d->flickData[viewport];
        d->flickData.remove(viewport);

        return;
    }
    /*
    QWebView *webView = qobject_cast<QWebView*>(widget);
    if (webView) {
        webView->removeEventFilter(this);

        delete d->flickData[webView];
        d->flickData.remove(webView);

        return;
    }
    */
}

static QPoint deaccelerate(const QPoint &speed, const int deltatime)
{
    const int deltaSpeed = deltatime;

    int x = speed.x();
    int y = speed.y();
    x = (x == 0) ? x : (x > 0) ? qMax(0, x - deltaSpeed) : qMin(0, x + deltaSpeed);
    y = (y == 0) ? y : (y > 0) ? qMax(0, y - deltaSpeed) : qMin(0, y + deltaSpeed);
    return QPoint(x, y);
}

bool FlickCharm::eventFilter(QObject *object, QEvent *event)
{
    if (!object->isWidgetType())
        return false;

    const QEvent::Type type = event->type();

    switch (type) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
        break;
    case QEvent::MouseButtonDblClick: // skip double click
        return true;
    default:
        return false;
    }

    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if (type == QEvent::MouseMove && mouseEvent->buttons() != Qt::LeftButton)
        return false;

    if (mouseEvent->modifiers() != Qt::NoModifier)
        return false;

    QWidget *viewport = qobject_cast<QWidget*>(object);
    FlickData *data = d->flickData.value(viewport);
    if (!viewport || !data || data->ignored.removeAll(event))
        return false;

    const QPoint mousePos = mouseEvent->pos();
    bool consumed = false;
    switch (data->state) {

    case FlickData::Steady:
        if (type == QEvent::MouseButtonPress) {
            consumed = true;
            data->pressPos = mousePos;
        } else if (type == QEvent::MouseButtonRelease) {
            consumed = true;
            QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,
                                                  data->pressPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier);
            QMouseEvent *event2 = new QMouseEvent(QEvent::MouseButtonRelease,
                                                  data->pressPos, Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier);

            data->ignored << event1;
            data->ignored << event2;
            QApplication::postEvent(object, event1);
            QApplication::postEvent(object, event2);
        } else if (type == QEvent::MouseMove) {
            consumed = true;
            data->scrollTo(mousePos);

            const QPoint delta = mousePos - data->pressPos;
            if (delta.x() > fingerAccuracyThreshold || delta.y() > fingerAccuracyThreshold)
                data->state = FlickData::ManualScroll;
        }
        break;

    case FlickData::ManualScroll:
        if (type == QEvent::MouseMove) {
            consumed = true;
            data->scrollTo(mousePos);
        } else if (type == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::AutoScroll;
            data->lastPosValid = false;
            d->startTicker(this);
        }
        break;

    case FlickData::AutoScroll:
        if (type == QEvent::MouseButtonPress) {
            consumed = true;
            data->state = FlickData::AutoScrollAcceleration;
            data->waitingAcceleration = true;
            data->accelerationTimer.start();
            data->updateSpeed(mousePos);
            data->pressPos = mousePos;
        } else if (type == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::Steady;
            data->resetSpeed();
        }
        break;

    case FlickData::AutoScrollAcceleration:
        if (type == QEvent::MouseMove) {
            consumed = true;
            data->updateSpeed(mousePos);
            data->accelerationTimer.start();
            if (data->speed.isNull())
                data->state = FlickData::ManualScroll;
        } else if (type == QEvent::MouseButtonRelease) {
            consumed = true;
            data->state = FlickData::AutoScroll;
            data->waitingAcceleration = false;
            data->lastPosValid = false;
        }
        break;
    default:
        break;
    }
    data->lastPos = mousePos;
    return true;
}

void FlickCharm::timerEvent(QTimerEvent *event)
{
    int count = 0;
    QHashIterator<QWidget*, FlickData*> item(d->flickData);
    while (item.hasNext()) {
        item.next();
        FlickData *data = item.value();
        if (data->state == FlickData::AutoScrollAcceleration
            && data->waitingAcceleration
            && data->accelerationTimer.elapsed() > 40) {
            data->state = FlickData::ManualScroll;
            data->resetSpeed();
        }
        if (data->state == FlickData::AutoScroll || data->state == FlickData::AutoScrollAcceleration) {
            const int timeElapsed = d->timeCounter.elapsed();
            const QPoint delta = (data->speed) * timeElapsed / 1000;
            bool hasScrolled = data->scrollWidget(delta.x(), delta.y());

            if (data->speed.isNull() || !hasScrolled)
                data->state = FlickData::Steady;
            else
                count++;
            data->speed = deaccelerate(data->speed, timeElapsed);
        }
    }

    if (!count)
        d->ticker.stop();
    else
        d->timeCounter.start();

    QObject::timerEvent(event);
}

#ifdef WIDGETMAP_USE
bool procWidgetEvent(MyCanvas* curWidget, QEvent* evt, WidgetConf* cf ) {
    if( curWidget==NULL || cf==NULL ) {
        return false;
    }
    switch(evt->type()) {
    case QEvent::KeyPress: {
        if( curWidget->onKeyDown ) {
            return curWidget->onKeyDown(cf, static_cast<QKeyEvent*>(evt) );
        }
    } break;
    case QEvent::ActivationChange: {
        if( curWidget->onActivationChange ) {
            return curWidget->onActivationChange(cf );
        }
    } break;
    case QEvent::FocusIn: {
        if( curWidget->onFocusIn) {
            return curWidget->onFocusIn(cf  );
        }
    } break;
    case QEvent::FocusOut: {
        if( curWidget->onFocusOut) {
            return curWidget->onFocusOut(cf );
        }
    } break;
    case QEvent::Move: {
        if( curWidget->onMove) {
            return curWidget->onMove(cf, static_cast<QMoveEvent*>(evt) );
        }
    } break;
    case QEvent::Wheel: {
        if( curWidget->onWheel) {
            return curWidget->onWheel(cf, static_cast<QWheelEvent*>(evt) );
        }
    } break;
    case QEvent::DragEnter: {
        if( curWidget->onDragEnter) {
            return curWidget->onDragEnter(cf, static_cast<QDragEnterEvent*>(evt) );
        }
    } break;
    case QEvent::DragLeave: {
        if( curWidget->onDragLeave) {
            return curWidget->onDragLeave(cf, static_cast<QDragLeaveEvent*>(evt) );
        }
    } break;
    case QEvent::DragMove: {
         if( curWidget->onDragMove) {
            return curWidget->onDragMove(cf, static_cast<QDragMoveEvent*>(evt) );
        }
    } break;
    case QEvent::Drop: {
         if( curWidget->onDrop) {
            return curWidget->onDrop(cf, static_cast<QDropEvent*>(evt) );
        }
    } break;
    case QEvent::ContextMenu: {
        if( curWidget->onContextMenu) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return curWidget->onContextMenu(cf, e->pos().x(), e->pos().y() );
        }
    } break;
    case QEvent::MouseButtonDblClick: {
        if( curWidget->onDoubleClick) {
            return curWidget->onDoubleClick(cf, static_cast<QMouseEvent*>(evt) );
        }
    } break;
    case QEvent::MouseMove:{
        if( curWidget->onDragStart ) {
            QMouseEvent* e = static_cast<QMouseEvent*>(evt);
            if( (e->buttons() == Qt::LeftButton) ) {
                StrVar* sv=cf->GetVar("@mousePos");
                if( SVCHK('i',2) ) {
                    int x=sv->getInt(4), y=sv->getInt(8);
                    QPoint pt = e->pos() - QPoint(x,y);
                    if( pt.manhattanLength()>QApplication::startDragDistance() ) {
                        curWidget->onDragStart(cf, e);
                    }
                }
            }
        }
        CanvasItem* items=curWidget->canvasItems();
        if( items ) {
            QMouseEvent* e = static_cast<QMouseEvent*>(evt);
            items->mouseMove(e->pos().x(), e->pos().y() );
        } else if( curWidget->onMouseMove ) {
            return curWidget->onMouseMove(cf, static_cast<QMouseEvent*>(evt) );
        }

    } break;
    case QEvent::MouseButtonPress: {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        cf->GetVar("@mousePos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
        if( curWidget->onMouseDown ) {
            return curWidget->onMouseDown(cf, static_cast<QMouseEvent*>(evt) );
        }
    } break;
    case QEvent::MouseButtonRelease: {
        QMouseEvent* e = static_cast<QMouseEvent*>(evt);
        StrVar* sv=cf->gv("@mousePos");
        if( SVCHK('i',2) ) {
            int x=sv->getInt(4)-8, y=sv->getInt(8)-8;
            QRect rc(x,y,16,16);
            sv->reuse();
            if( rc.contains(e->pos()) ) {
                CanvasItem* items=curWidget->canvasItems();
                if( items ) {
                    return items->mouseClick(e->pos().x(), e->pos().y());
                } else if( curWidget->onClick ) {
                    return curWidget->onClick(cf );
                }
            }
        }
        if( curWidget->onMouseUp ) {
            return curWidget->onMouseUp(cf, e );
        }

    } break;
    case QEvent::Timer:{
        if( curWidget->onTimer ) {
            return curWidget->onTimer(cf, static_cast<QTimerEvent*>(evt) );
        }
    } break;
    case QEvent::Resize:{
        if( curWidget->onSize) {
            return curWidget->onSize(cf);
        } else {
            curWidget->canvasItems();
        }
    } break;
    case QEvent::Close:{
        if( curWidget->onClose) {
            return curWidget->onClose(cf);
        }
    } break;
    case QEvent::Paint: {
        /*
        CanvasItem* items=curWidget->canvasItems();
        if( items ) {
            QPaintEvent* e=static_cast<QPaintEvent*>(evt);
            QPainter paint;
            paint.begin(curWidget);
            items->draw(&paint, e->rect() );
            paint.end();
        } else
        */
        if( curWidget->onDraw ) {
            QPaintEvent* e=static_cast<QPaintEvent*>(evt);
            QPainter paint;
            paint.begin(curWidget);
            curWidget->onDraw(cf, &paint, e->rect() );
            paint.end();
        }
    } break;
    case QEvent::TouchBegin:{
        if( curWidget->onTouch) {
            return curWidget->onTouch(cf, static_cast<QTouchEvent*>(evt) );
        }
    } break;
    case QEvent::TouchUpdate:{
        if( curWidget->onTouchUpdate) {
            return curWidget->onTouchUpdate(cf, static_cast<QTouchEvent*>(evt) );
        }
    } break;
    case QEvent::TouchEnd:{
        if( curWidget->onTouchEnd) {
            return curWidget->onTouchEnd(cf, static_cast<QTouchEvent*>(evt) );
        }
    } break;
    case USER_EVENT: {
        if( curWidget->onUserEvent ) {
            return curWidget->onUserEvent(cf, static_cast<UserEvent*>(evt) );
        }
    } break;
    default: {
    }break;
    }
    return false;
}

MyTimeLine::MyTimeLine(WidgetConf *cf, QWidget *p) : QTimeLine(1000,p), xcf(cf) {
    xitem=NULL;
}

MyCanvas::MyCanvas(WidgetConf *cf, QWidget *p) : QWidget(p), xcf(cf) {
    onActivationChange=NULL;
    onKeyDown=NULL;
    onMouseDown=NULL;
    onMouseUp=NULL;
    onMouseMove=NULL;
    onClick=NULL;
    onDoubleClick=NULL;
    onMove=NULL;
    onWheel=NULL;
    onDragStart=NULL;
    onDragEnter=NULL;
    onDragLeave=NULL;
    onDragMove=NULL;
    onDrop=NULL;
    onContextMenu=NULL;
    onFocusIn=NULL;
    onFocusOut=NULL;

    onTimer=NULL;
    onSize=NULL;
    onClose=NULL;
    onDraw=NULL;

    onTouch=NULL;
    onTouchUpdate=NULL;
    onTouchEnd=NULL;
    onWheel=NULL;
    onUserEvent=NULL;
    onTimeOut=NULL;

    xtimeline=NULL;
    xitems=NULL;
    if( xcf ) {
        xcf->xwidget=this;
#ifdef MYCANVAS_USE
        if( isVarTrue(xcf->gv("scroll")) ) {
            setCanvasScroll(xcf);
        }
#endif
        if( xcf->isTrue("overlay") ) {
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            setAttribute(Qt::WA_NoSystemBackground);
            setAttribute(Qt::WA_TranslucentBackground);
            setAttribute(Qt::WA_PaintOnScreen);
            setAttribute(Qt::WA_TransparentForMouseEvents);
        }
    }
}

MyCanvas::~MyCanvas() { }

bool MyCanvas::startTimeLine(LPCC code, int frameEnd, int duration, LPCC mode, bool bBack) {
    if( xtimeline==NULL ) {
        xtimeline=new MyTimeLine(xcf, this);
    }
    if( xtimeline->state()== QTimeLine::Running  ) {
        // return false;
        stopTimeLine();
    }
    connect( xtimeline, SIGNAL(frameChanged(int)), this, SLOT(updateCanvas(int)) );
    connect( xtimeline, SIGNAL(finished()), this, SLOT(slotTimeLineFinish()) );
    xtimeline->xitem=NULL; // getCanvasItem(xcf, code);
    xtimeline->setCode(code);
    xtimeline->setFrameRange(0,frameEnd);
    xtimeline->setDuration(duration );
    if( bBack ) {
        xtimeline->setDirection(QTimeLine::Backward);
    } else {
        xtimeline->setDirection(QTimeLine::Forward);
    }
    if( ccmp(mode,"in") ) {
        xtimeline->setCurveShape(QTimeLine::EaseInCurve);
    } else if( ccmp(mode,"out") ) {
        xtimeline->setCurveShape(QTimeLine::EaseOutCurve);
    } else if( ccmp(mode,"inout") ) {
        xtimeline->setCurveShape(QTimeLine::EaseInOutCurve);
    } else if( ccmp(mode,"linear") ) {
        xtimeline->setCurveShape(QTimeLine::LinearCurve);
    } else if( ccmp(mode,"sine") ) {
        xtimeline->setCurveShape(QTimeLine::SineCurve);
    } else if( ccmp(mode,"cosine") ) {
        xtimeline->setCurveShape(QTimeLine::CosineCurve);
    } else {
        xtimeline->setCurveShape(QTimeLine::LinearCurve);
    }
    if( xitems ) {
        xitems->updateStart();
    }
    xtimeline->start();
    return xtimeline->state()== QTimeLine::Running;
}

bool MyCanvas::stopTimeLine() {
    if( xtimeline && xtimeline->state()==QTimeLine::Running  ) {
        xtimeline->stop();
        xtimeline->xitem=NULL;
        return true;
    }
    return false;
}

CanvasItem *MyCanvas::canvasItems( bool resize )
{
#ifdef MYCANVAS_USE
    if( xitems==NULL ) {
        xitems=setCanvasItems(xcf);
        xitems->divInfoApply();
    } else if( resize ) {
        setCanvasItemRect(xitems);
        xitems->divInfoApply();
    }
#endif
    return xitems;
}

bool MyCanvas::setTimeout(LPCC timerCode, int interval)
{
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(interval);
    timer.setSingleShot(true);
    loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
    timer.start();
    loop.exec();
    return onTimeOut ? onTimeOut(xcf, timerCode): false;
}

void MyCanvas::updateCanvas(int frame ) {
    if( xitems ) {
        xitems->update(xtimeline);
    }
    update();
}

void MyCanvas::slotTimeOut() {
    if( onTimeOut ) onTimeOut(xcf, NULL);
}

void MyCanvas::slotTimeLineFinish()
{
    if( xitems ) {
        xitems->updateEnd();
    }
}

bool MyCanvas::event(QEvent *evt) {
    if( procWidgetEvent(this, evt, config()) ) return true;
    return QWidget::event(evt);
}


MyHeaderView::MyHeaderView(WidgetConf* cf, Qt::Orientation orientation, QWidget * parent) : QHeaderView(orientation,parent), xcf(cf) {
    xidx = 0;
    onDrawHeader=NULL;
    onSizeHeader=NULL;
    onClickHeader=NULL;
}
QSize MyHeaderView::sizeHint() const {
    // Get the base implementation size.
    QSize baseSize = QHeaderView::sizeHint();
    if( onSizeHeader ) {
        QWidget* widget=static_cast<QWidget*>(parent());
        if( widget ) {
            onSizeHeader(xcf, &baseSize );
        }
    } else {
        StrVar* sv=xcf->gv("@headerHeight");
        if( isNumberVar(sv) ) {
            baseSize.setHeight( toInteger(sv) );
        } else {
            baseSize.setHeight( 25 );
        }
    }
    return baseSize;
}
void MyHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const {
    QString text = model()->headerData( logicalIndex, orientation(), Qt::DisplayRole).toString();
    if( onDrawHeader ) {
        QWidget* widget=static_cast<QWidget*>(parent());
        if( widget ) {
            onDrawHeader(xcf, painter, rect, Q2A(text), logicalIndex, sortIndicatorOrder() );
        }
    } else {
        painter->fillRect(rect, Qt::white);
        if( text.isEmpty() ) return;

        text = painter->fontMetrics().elidedText(text, Qt::ElideRight, rect.width());
        int textWidth = painter->fontMetrics().width(text);

        painter->save();
        if( logicalIndex == xidx ) {
            painter->setPen(QColor(62, 32, 32));
            painter->drawText(rect, Qt::AlignCenter, text);
            QPixmap* pm=NULL;
            if(Qt::DescendingOrder == sortIndicatorOrder() ) {
                pm = getPixmap("list_icon.down");
                if( pm )
                painter->drawPixmap(
                    ((rect.right() - (rect.width()/2) ) + (textWidth / 2)) + 5, (rect.bottom() - rect.top() - 4 )/2 ,
                    6,4,*pm
                );
            } else if(Qt::AscendingOrder == sortIndicatorOrder()) {
                pm = getPixmap("list_icon.up");
                if( pm )
                painter->drawPixmap(
                    ((rect.right() - (rect.width()/2) ) + (textWidth / 2)) + 5, (rect.bottom() - rect.top() - 4 )/2 ,
                    6, 4, *pm
                );
            }
        } else {
            painter->setPen(QColor(38, 38, 38));
            painter->drawText(rect, Qt::AlignCenter, text);
        }
        painter->restore();
        painter->save();
        painter->setPen(QColor(126, 126, 126));
        // 오른쪽 구분선
        painter->drawLine(rect.right(), (rect.bottom()-rect.top())/2, rect.right(), rect.bottom());
        // 아래 구분선
        painter->drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
        painter->restore();
    }
}

void MyHeaderView::sectionClickedSlot(int index) {
    if( onClickHeader ) {
        QWidget* widget=static_cast<QWidget*>(parent());
        if( widget ) onClickHeader(xcf, index, 0);
    }
}

MyTree::MyTree(WidgetConf* cf, QWidget* p) : QTreeView(p), xcf(cf) {
    xbaseModel=NULL;
    xproxyModel=NULL;
    onContext=NULL;
    onChange=NULL;
    onChangeColumn=NULL;
    onChangeSelection=NULL;
    onClick=NULL;
    onDoubleClick=NULL;
    onCollapsed=NULL;
    // header
    onSizeHeader=NULL;
    onClickHeader=NULL;
    onDrawHeader=NULL;
    // model
    onChildData=NULL;
    onDropData=NULL;
    onMimeData=NULL;
    onToolTip=NULL;
    onLessThan=NULL;
    onFilterRow=NULL;
    onFlagsRow=NULL;
    onFetchMore=NULL;
    onDraw=NULL;
    onEditRow=NULL;
    onRowSize=NULL;
    onEditFocusIn=NULL;
    onEditFocusOut=NULL;
    onEditKeyDown=NULL;
    onEditMouseDown=NULL;
    onMouseDown=NULL;
    onMouseUp=NULL;
    onMouseMove=NULL;
    onKeyDown=NULL;
    onWheel=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    setMouseTracking(true);
    installEventFilter(this);
    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);
}

MyTree::~MyTree()
{
    if( config()->getBool("@baseCreate") ) {
        SAFE_DELETE(xbaseModel);
    }
    SAFE_DELETE(xproxyModel);
}
void MyTree::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf, NULL); }
bool MyTree::eventFilter(QObject *object, QEvent *evt)
{
   switch( evt->type() ) {
   case QEvent::ContextMenu: {
       if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           xcf->set("@sender", object==viewport()?"view":"this");
           return onContext(xcf, e->pos().x(), e->pos().y()  );
       }
   } break;
   case QEvent::MouseButtonPress: {
       if( onMouseDown ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseDown(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::MouseButtonRelease: {
       if( onMouseUp ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseUp(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::MouseMove: {
       if( onMouseMove ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseMove(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::KeyPress: {
       QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
       if( onKeyDown ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onKeyDown(xcf, keyEvent) ) return true;
       }
   } break;
   }
   return false;
   // return QTreeView::event(evt);
}

void MyTree::initModel(MyTreeModel* baseModel)
{
    bool bset=false;
    if( baseModel ) {
        if( xbaseModel ) {
            if( xbaseModel!=baseModel ) {
                xbaseModel=baseModel;
                bset=true;
            }
        } else {
            xbaseModel=baseModel;
            bset=true;
        }
    } else if(xbaseModel==NULL ) {
        setTreeModelHeader(xcf);
        config()->setBool("@baseCreate", true);
        bset=true;
        xbaseModel=new MyTreeModel(xcf, NULL, this);
    }
    if( bset && xproxyModel ) {
        xproxyModel->m_sourceModel=xbaseModel;
    }
    if( xproxyModel==NULL ) {
        xproxyModel=new MyTreeProxyModel(xbaseModel, this);
        setModel(xproxyModel);
        setItemDelegate(new MyTreeDelegate(xproxyModel, this) );
        if( xcf->gv("headerUse") ) {
            setHeader( new MyHeaderView(xcf,Qt::Horizontal,this) );
        }
        connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)) );
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDoubleClicked(QModelIndex)) );
        connect(this, SIGNAL(collapsed(QModelIndex)), this, SLOT(slotCollapsed(QModelIndex)) );

        connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)) );
        connect(selectionModel(), SIGNAL(currentColumnChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentColumnChanged(QModelIndex,QModelIndex)) );
        connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)) );
        if( isVarTrue(xcf->gv("dragUse")) ) {
            setDragDropMode(QAbstractItemView::InternalMove);
            setSelectionMode(QAbstractItemView::ExtendedSelection);
            setDragEnabled(true);
        }
        if( isVarTrue(xcf->gv("dropUse")) ) {
            setAcceptDrops(true);
            setDropIndicatorShown(true);
        }
    }
    update();
}


void MyTree::setRootNode(TreeNode *rootNode)
{
    if( xbaseModel && rootNode ) {
        xbaseModel->setRootNode(rootNode);
        update();
    }
}

void MyTree::update()
{
    if( xbaseModel && xproxyModel ) {
        xbaseModel->onChildData=onChildData;
        xproxyModel->onDraw=onDraw;
        xproxyModel->onFilterRow=onFilterRow;
        xproxyModel->onLessThan=onLessThan;
        xproxyModel->onToolTip=onToolTip;
        xproxyModel->onRowSize=onRowSize;
        xproxyModel->onFlagsRow=onFlagsRow;
        xproxyModel->onEditRow=onEditRow;
        xproxyModel->onFetchMore=onFetchMore;
        xproxyModel->onDropData=onDropData;
        xproxyModel->onMimeData=onMimeData;
        xproxyModel->onEditFocusIn=onEditFocusIn;
        xproxyModel->onEditFocusOut=onEditFocusOut;
        xproxyModel->onEditKeyDown=onEditKeyDown;
        xproxyModel->onEditMouseDown=onEditMouseDown;
        xproxyModel->invalidate();
    }
}

void	MyTree::slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
    if( xproxyModel && onChange ) {
        QModelIndex srcIndex = xproxyModel->mapToSource(current);
        TreeNode* cur= srcIndex.isValid() ? (TreeNode*)srcIndex.internalPointer(): NULL;
        onChange( xcf, cur );
    }
}
void	MyTree::slotCurrentColumnChanged ( const QModelIndex & current, const QModelIndex & previous ) {
    if( xproxyModel && onChangeColumn ) {
        QModelIndex srcIndex = xproxyModel->mapToSource(current);
        TreeNode* cur= srcIndex.isValid() ? (TreeNode*)srcIndex.internalPointer(): NULL;
        onChangeColumn( xcf, cur );
    }
}
/*
void	MyTree::slotCurrentRowChanged ( const QModelIndex & current, const QModelIndex & previous ) {
    if( xproxyModel && onChangeRow) {
        QModelIndex srcIndex = xproxyModel->mapToSource(current);
        TreeNode* cur= srcIndex.isValid() ? (TreeNode*)srcIndex.internalPointer(): NULL;
        onChangeColumn( xcf, cur );
    }
}
*/
void	MyTree::slotDoubleClicked(const QModelIndex & index) {
    if( xproxyModel && onDoubleClick) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onDoubleClick( xcf, cur );
    }
}
void	MyTree::slotClicked ( const QModelIndex & index ) {
    if( xproxyModel && onClick ) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onClick( xcf, cur );
    }
}
void MyTree::slotCollapsed(const QModelIndex & index) {
    if( xproxyModel && onCollapsed) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onCollapsed( xcf, cur );
    }
}

void MyTree::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList list=selected.indexes();
    int size=list.size();
    if( onChangeSelection && xproxyModel && size>1 ) {
        XListArr* arr=xcf->addArray("@selections")->reuse();
        for( int n=0; n<size; n++) {
            QModelIndex curIndex = xproxyModel->mapToSource(list.at(n));
            TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
            if( cur ) {
                arr->add()->setVar('n',0,(LPVOID)cur);
            }
        }
        onChangeSelection(xcf, arr);
    }
}

/*
void	MyTree::slotSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected ) {
}
*/

MyListView::MyListView(WidgetConf *cf, QWidget *p) : QListView(p), xcf(cf) {
    xbaseModel =NULL;
    xproxyModel=NULL;
    onContext=NULL;
    onChange=NULL;
    onClick=NULL;
    onDoubleClick=NULL;
    onChangeSelection=NULL;
    // model
    onChildData=NULL;
    onDropData=NULL;
    onMimeData=NULL;
    onToolTip=NULL;
    onLessThan=NULL;
    onFilterRow=NULL;
    onFlagsRow=NULL;
    onFetchMore=NULL;
    onDraw=NULL;
    onEditRow=NULL;
    onRowSize=NULL;
    onEditFocusIn=NULL;
    onEditFocusOut=NULL;
    onEditKeyDown=NULL;
    onEditMouseDown=NULL;
    onMouseDown=NULL;
    onMouseUp=NULL;
    onMouseMove=NULL;
    onKeyDown=NULL;
    onWheel=NULL;
    onTimeOut=NULL;

    installEventFilter(this);
    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);
    if( xcf ) xcf->xwidget=this;
}

MyListView::~MyListView()
{
    if( config()->getBool("@baseCreate") ) {
        SAFE_DELETE(xbaseModel);
    }
    SAFE_DELETE(xproxyModel);
}
void MyListView::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf, NULL); }
void MyListView::slotCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if( xproxyModel && onChange ) {
        QModelIndex srcIndex = xproxyModel->mapToSource(current);
        TreeNode* cur= srcIndex.isValid() ? (TreeNode*)srcIndex.internalPointer(): NULL;
        onChange( xcf, cur );
    }

}

void MyListView::slotDoubleClicked(const QModelIndex &index)
{
    if( xproxyModel && onDoubleClick) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onDoubleClick( xcf, cur );
    }
}

void MyListView::slotClicked(const QModelIndex &index)
{
    if( xproxyModel && onClick ) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onClick( xcf, cur );
    }
}

void MyListView::slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList list=selected.indexes();
    int size=list.size();
    if( onChangeSelection && xproxyModel && size>1 ) {
        XListArr* arr=xcf->addArray("@selections")->reuse();
        for( int n=0; n<size; n++) {
            QModelIndex curIndex = xproxyModel->mapToSource(list.at(n));
            TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
            if( cur ) {
                arr->add()->setVar('n',0,(LPVOID)cur);
            }
        }
        onChangeSelection(xcf, arr);
    }
}

void MyListView::initModel(MyTreeModel* baseModel)
{
    bool bset=false;
    if( baseModel ) {
        if( xbaseModel ) {
            if( xbaseModel!=baseModel ) {
                xbaseModel=baseModel;
                bset=true;
            }
        } else {
            xbaseModel=baseModel;
            bset=true;
        }
    } else if(xbaseModel==NULL ) {
        setTreeModelHeader(xcf);
        config()->setBool("@baseCreate", true);
        bset=true;
        xbaseModel=new MyTreeModel(xcf, NULL, this);
    }
    if( bset && xproxyModel ) {
        xproxyModel->m_sourceModel=xbaseModel;
    }
    if( xproxyModel==NULL ) {
        xproxyModel=new MyTreeProxyModel(xbaseModel, this);
        setModel(xproxyModel);
        setItemDelegate(new MyTreeDelegate(xproxyModel, this) );
        connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)) );
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDoubleClicked(QModelIndex)) );
        connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentChanged(QModelIndex,QModelIndex)) );
        connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged(QItemSelection,QItemSelection)) );
        if( isVarTrue(xcf->gv("dragUse")) ) {
            setDragDropMode(QAbstractItemView::InternalMove);
            setSelectionMode(QAbstractItemView::ExtendedSelection);
            setDragEnabled(true);
        }
        if( isVarTrue(xcf->gv("dropUse")) ) {
            setAcceptDrops(true);
            setDropIndicatorShown(true);
        }
    }
    update();
}

void MyListView::setRootNode(TreeNode* rootNode) {
    if( xbaseModel && rootNode ) {
        xbaseModel->setRootNode(rootNode);
        update();
    }
}
void MyListView::update() {
    if( xbaseModel && xproxyModel ) {
        xbaseModel->onChildData=onChildData;
        xproxyModel->onDraw=onDraw;
        xproxyModel->onFilterRow=onFilterRow;
        xproxyModel->onLessThan=onLessThan;
        xproxyModel->onToolTip=onToolTip;
        xproxyModel->onRowSize=onRowSize;
        xproxyModel->onFlagsRow=onFlagsRow;
        xproxyModel->onEditRow=onEditRow;
        xproxyModel->onFetchMore=onFetchMore;
        xproxyModel->onDropData=onDropData;
        xproxyModel->onMimeData=onMimeData;
        xproxyModel->onEditFocusIn=onEditFocusIn;
        xproxyModel->onEditFocusOut=onEditFocusOut;
        xproxyModel->onEditKeyDown=onEditKeyDown;
        xproxyModel->onEditMouseDown=onEditMouseDown;
        xproxyModel->invalidate();
    }
}

bool MyListView::eventFilter(QObject *object, QEvent *evt)
{
   switch( evt->type() ) {
   case QEvent::ContextMenu: {
       if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onContext(xcf, e->pos().x(), e->pos().y()) ) return true;
       }
   } break;
   case QEvent::MouseButtonPress: {
       if( onMouseDown ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseDown(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::MouseButtonRelease: {
       if( onMouseUp ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseUp(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::MouseMove: {
       if( onMouseMove ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onMouseMove(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
       }
   } break;
   case QEvent::KeyPress: {
       QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
       if( onKeyDown ) {
           xcf->set("@sender", object==viewport()?"view":"this");
           if( onKeyDown(xcf, keyEvent) ) return true;
       }
   } break;
   }
   // return QListView::event(evt);
   return false;
}


/************************************************************************/
/*   MyHighlighter                                                     */
/************************************************************************/


MyHighlighter::MyHighlighter( QTextDocument *parent, WidgetConf* cf) : QSyntaxHighlighter(parent), xcf(cf), xstringHighlightData(NULL) {
}
MyHighlighter::~MyHighlighter() {
    for( int n=0,size=xfmts.size();n<size;n++) {
        MyHighlightData* h = xfmts.getvalue(n);
        SAFE_DELETE(h);
    }
    for( int n=0,size=xcommentFmts.size();n<size;n++) {
        MyHighlightData* h = xcommentFmts.getvalue(n);
        SAFE_DELETE(h);
    }
    for( int n=0,size=xblockFmts.size();n<size;n++) {
        MyHighlightData* h = xblockFmts.getvalue(n);
        SAFE_DELETE(h);
    }
    SAFE_DELETE(xstringHighlightData);
}

void MyHighlighter::asHtml(QString& html) {
    // Create a new document from all the selected text document.
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    QTextDocument* tempDocument(new QTextDocument);
    Q_ASSERT(tempDocument);
    QTextCursor tempCursor(tempDocument);

    tempCursor.insertFragment(cursor.selection());
    tempCursor.select(QTextCursor::Document);
    // Set the default foreground for the inserted characters.
    QTextCharFormat textfmt = tempCursor.charFormat();
    textfmt.setForeground(Qt::gray);
    tempCursor.setCharFormat(textfmt);

    // Apply the additional formats set by the syntax highlighter
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    end = end.next();
    const int selectionStart = cursor.selectionStart();
    const int endOfDocument = tempDocument->characterCount() - 1;
    for(QTextBlock current = start; current.isValid() ; current = current.next()) {
        const QTextLayout* layout(current.layout());

        foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
            const int start = current.position() + range.start - selectionStart;
            const int end = start + range.length;
            if(end <= 0 || start >= endOfDocument)
                continue;
            tempCursor.setPosition(qMax(start, 0));
            tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
            tempCursor.setCharFormat(range.format);
        }
    }

    // Reset the user states since they are not interesting
    for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
        block.setUserState(-1);

    // Make sure the text appears pre-formatted, and set the background we want.
    tempCursor.select(QTextCursor::Document);
    QTextBlockFormat blockFormat = tempCursor.blockFormat();
    blockFormat.setNonBreakableLines(true);
    blockFormat.setBackground(Qt::black);
    tempCursor.setBlockFormat(blockFormat);

    // Finally retreive the syntax higlighted and formatted html.
    html = tempCursor.selection().toHtml();
    delete tempDocument;
}

MyHighlightData *MyHighlighter::find(U32 type)
{
    int n=0;
    MyHighlightData* hd=NULL;
    for( n=0;n<xcommentFmts.size();n++ ) {
        hd=xcommentFmts.getvalue(n);
        if( hd->xtype==type ) {
            return hd;
        }
    }
    for( n=0;n<xfmts.size();n++ ) {
        hd=xfmts.getvalue(n);
        if( hd->xtype==type ) {
            return hd;
        }
    }
    for( n=0;n<xblockFmts.size();n++ ) {
        hd=xblockFmts.getvalue(n);
        if( hd->xtype==type ) {
            return hd;
        }
    }
    return NULL;
}

/*
inline int getHighlightStringPos( const QString& text, char ch ) {
    int ep=text.indexOf(ch, 0);
    if( ep==-1 ) return ep;
    bool bEnd=false;
    while( ep!=-1 ) {
        if( ep>0 && text.at(ep-1)==QChar('\\') ) {
            ep = text.indexOf(ch, ep+1);
            continue;
        }
        ep = text.indexOf(ch, ep+1);
        if( ep==-1 ) break;
        if( bEnd )
            bEnd=false;
        else
            bEnd=true;
    }
    return bEnd? -1 : ep;
}
*/

void MyHighlighter::highlightBlock( const QString &text ) {
    /*
    static char prevChar='\0';
    static long prevCharTick=0;
    */
    int index=0,length=0, commentStart=-1, commentEnd=-1, commentPos=-1, stringEnd=0;
    int statePrev = previousBlockState(), stateCur=0;
    // ver 1.0.3

    if( statePrev==52 && xstringHighlightData ) {
        // long dist=GetTickCount() - prevCharTick;
        int sp=0, ep=0;
        char ch=xstringHighlightData->xtype&0x10000 ? '\'': '\"';
        ep=text.lastIndexOf( ch );
        /*
        if( dist<10 ) {
            ep=text.lastIndexOf( prevChar);
        } else {
            ep=getHighlightStringPos(text, '\'');
            if( ep==-1 ) ep=getHighlightStringPos(text, '\"');
        }
        */
        if( ep!=-1 ) {
            index = ep+1;
            stringEnd=index;
            setFormat(sp, index-sp, xstringHighlightData->charFormat());
        } else {
            ep=text.length();
            setCurrentBlockState(52);
            setFormat(sp, ep-sp, xstringHighlightData->charFormat());
            return;
        }
    }


    // 이전 상태가 문자열이 아니라면( ok==false 면 이전상태 문자열)
    for( int n=0,size=xfmts.size(); statePrev<50 && n<size; n++) {
        MyHighlightData* h = xfmts.getvalue(n);
        int sp=stringEnd;
        U16 state = h->getState();
        if( state==10 ) {	// line comment
            index = h->exp.indexIn(text, sp);
            if( index >= 0 ) {
                commentStart=index;
                length = h->exp.matchedLength();
                setFormat(index, length, h->charFormat());
            }
        } else if( state==71 || state==72 ) {
            index = h->exp.indexIn(text, sp);
            while( index >= 0 ) {
                length=h->exp.matchedLength();
                int ep = h->expEnd.indexIn(text,index+length);
                if( ep>index ) {
                    if( state==71 ) {
                        ep+=h->expEnd.matchedLength();
                        setFormat(index, ep-index, h->charFormat());
                        sp=ep;
                    } else {
                        setFormat(index, length, h->charFormat());
                        setFormat(ep, h->expEnd.matchedLength(), h->charFormat());
                        sp=ep+h->expEnd.matchedLength();
                    }
                    index = h->exp.indexIn(text, sp);
                } else {
                    break;
                }
            }
        } else {
            index = h->exp.indexIn(text,sp);
            length = 0;
            while(index >= 0 ) {
                if( state==13 ) {			// SYNTAX.tag
                    index = h->exp.pos(1);
                    length = h->exp.cap(0).length();
                } else {
                    length = h->exp.matchedLength();
                    if( length==0 )
                        break;
                    if( state==11 )
                        length-=1;// SYNTAX.func
                }
                if( index==-1 || length==0 ) {
                    break;
                }
                setFormat(index, length, h->charFormat());
                index = text.indexOf(h->exp, index+length); // expr.indexIn(text, index + length);
            }
        }
    }
    bool ok=true;
    for( int n=0,size=xblockFmts.size();n<size;n++) {
        MyHighlightData* h = xblockFmts.getvalue(n);
        U16 state = h->getState();
        int startIndex=-1;
        length=0;
        if( statePrev==state ) {
            startIndex=0;
        } else {
            if( statePrev>100 ) continue;
            startIndex = text.indexOf(h->exp, stringEnd);
            if( startIndex>=0 ) {
                length=h->exp.matchedLength();
                if( commentStart!=-1 ) {
                    if( commentStart < startIndex ) {
                        startIndex=-1;
                    } else {
                        commentPos=commentStart;
                        commentStart=startIndex;
                    }
                } else {
                    commentStart=startIndex;
                }
            }
        }

        while( startIndex >= 0) {
            int endIndex = text.indexOf(h->expEnd, startIndex+length);
            int commentLength;
            if( endIndex == -1) {
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + h->expEnd.matchedLength();
            }
            if( commentLength==0 ) {
                startIndex = text.indexOf(h->exp, startIndex + 1);
            } else {
                setFormat(startIndex, commentLength, h->charFormat());
                startIndex = text.indexOf(h->exp, startIndex + commentLength);
            }
            if( endIndex != -1) {
                if( commentPos!=-1 ) commentStart=commentPos;
                commentEnd=endIndex;
                // statePrev=0;
                setCurrentBlockState(0);
                ok=true;
                break;
            } else {
                setCurrentBlockState(state);
                ok=false;
            }
        }
        if( statePrev==state ) {
            return;
        }
    }
    for( int n=0,size=xcommentFmts.size();ok && n<size ;n++) {
        MyHighlightData* h = xcommentFmts.getvalue(n);
        U16 state = h->getState();
        index = h->exp.indexIn(text, 0);
        if( index==-1 ) continue;
        if( state==62 ) {
            if( index<2 ) {
                length = h->exp.matchedLength();
                if( length>0 ) setFormat(index, length, h->charFormat());
                setCurrentBlockState(0);
                return;
            }
        } else {
            length = h->exp.matchedLength();
            if( length>0 ) setFormat(index, length, h->charFormat());
            commentStart=index;
        }
    }

    if( xstringHighlightData  ) {
        U16 state = xstringHighlightData->getState();
        if( state==52  || state==51 ) {
            char ch=0;
            int sp=0, ep=0;
            index=stringEnd;
            for( int n=0; n<256; n++ ) {
                sp = text.indexOf('\"', index );
                if( sp!=-1 ) {
                    ep = text.indexOf('\'', index);
                    ch = '\"';
                    if( ep!=-1 && sp>ep ) {
                        ch = '\'';
                        sp = ep;
                    }
                } else {
                    sp = text.indexOf('\'', index);
                    ch= '\'';
                }
                if( sp==-1 ) break;

                if( commentStart!=-1 ) {
                    if( sp>commentStart ) {
                        if( commentEnd!=-1 ) {
                            if( sp<commentEnd ) {
                                break;
                            }
                        } else {
                            break;
                        }
                    } else if( ok==false ) {
                        ok=true;
                    }
                }
                ep = text.indexOf(ch,sp+1);
                ok = ep!=-1 ? true: false;

                while( ch=='\"' && ep>0 && ok ) {
                    if( text.at(ep-1)!=QChar('\\') )
                        break;
                    if( ep>1 && text.at(ep-2)==QChar('\\') )
                        break;
                    ep = text.indexOf(ch, ep+1);
                    if( ep==-1 ) {
                        ok = false;
                    }
                }
                // if( cp==-1 ) cp = sp;
                if( ok ) {
                    index = ep+1;
                    setFormat(sp, index-sp, xstringHighlightData->charFormat());
                } else {
                    ep = text.size();
                    if( state==52 ) { // mode=='eps'
                        /*
                        prevChar = ch;
                        prevCharTick=GetTickCount();
                        */
                        xstringHighlightData->xtype&=~0xFF0000;
                        if( ch=='\'' ) {
                            xstringHighlightData->xtype|=0x10000;
                        } else {
                            xstringHighlightData->xtype|=0x20000;
                        }

                        setCurrentBlockState(state);
                    }
                    setFormat(sp, ep-sp, xstringHighlightData->charFormat());
                    break;
                }
            }
            if( state!=52 && ok==false ) ok = true;
        }
    }

    if( ok ) {
        setCurrentBlockState(0);
    }
}



/************************************************************************/
/*   MyTextLineNumberArea                                                     */
/************************************************************************/
MyTextLineNumberArea::MyTextLineNumberArea(MyTextEdit *editor) : QWidget(editor), xflag(0)
{
    xedit = editor;
}
QSize MyTextLineNumberArea::sizeHint() const
{
    return QSize(xedit->lineNumberAreaWidth(), 0);
}
void MyTextLineNumberArea::paintEvent(QPaintEvent *event)
{
    xedit->lineNumberAreaPaintEvent(event);
}
void MyTextLineNumberArea::mouseMoveEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
}

void MyTextLineNumberArea::mousePressEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
}

void MyTextLineNumberArea::mouseReleaseEvent( QMouseEvent* evt ) {
    xedit->extraAreaMouseEvent(evt);
    //QWidget::mouseReleaseEvent(evt);
}

MyTextEdit::MyTextEdit(WidgetConf* cf, QWidget* p) : QTextEdit(p), xcf(cf), xscrollLock(false) {
    setCursorWidth(2);
    setLineWrapMode(QTextEdit::NoWrap);
    onContext=NULL;
    onDrawEditor=NULL;
    onDrawLineNum=NULL;
    onChangeText=NULL;
    onChangeModify=NULL;
    onChangeCursor=NULL;
    onChangeFormat=NULL;
    onEnableCheck=NULL;
    onClickLineNum=NULL;
    onMimeData=NULL;
    onKeyDown=NULL;
    onMouseDown=NULL;
    onMouseUp=NULL;
    onMouseMove=NULL;
    onDoubleClick=NULL;
    onWheel=NULL;
    onTimeOut=NULL;
    // setMouseTracking(true);
    // setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setLayoutDirection(Qt::LeftToRight);
    setTabStopWidth(16);
    setTabChangesFocus(true);

    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);
    installEventFilter(this);

    connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollValueChange(int)));
    connect(this, SIGNAL(undoAvailable( bool)), this, SLOT(slotUndoAvailable( bool)) );
    connect(this, SIGNAL(copyAvailable( bool)), this, SLOT(slotCopyAvailable( bool)) );
    connect(this, SIGNAL(redoAvailable( bool)), this, SLOT(slotRedoAvailable( bool)) );
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(textCurrentCharFormatChanged(QTextCharFormat)) );
    // connect(this, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect(this, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(textCurrentCharFormatChanged(QTextCharFormat)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(textCursorPositionChanged()));
    // connect(document(), SIGNAL(updateRequest(const QRect &, int)), this, SLOT(updateLineNumberArea(const QRect &, int)));
    connect(document(), SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(textContentsChange(int,int,int)), Qt::DirectConnection);
    connect(document(), SIGNAL(modificationChanged(bool)), this, SLOT(textModificationChanged(bool)));


    xcf->setNodeFlag(NF_LINENUM);
    xlinenum = new MyTextLineNumberArea(this);
    xsyntax = new MyHighlighter(document(),xcf);
    updateLineNumberAreaWidth(0);
    if( xcf ) xcf->xwidget=this;
}

MyTextEdit::~MyTextEdit() { }
void MyTextEdit::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf, NULL); }
void MyTextEdit::textCurrentCharFormatChanged(const QTextCharFormat &format) {
    if( onChangeFormat ) {
        onChangeFormat( xcf, &format);
    }
}
void MyTextEdit::textCursorPositionChanged() {
    StrVar* sv = config()->gv("@tabStopPostions");
    if( SVCHK('a',0) ) {
        XListArr* arr = (XListArr*)SVO;
        QTextCursor c = textCursor();
        int size = arr->size();
        int sp = config()->getInt("@tabStart");
        if( size>0 ) {
            int ep=0;
            int pos = c.position();
            sv = arr->get(size-1);
            if( SVCHK('0',0) ) {
                ep = sv->getInt(FUNC_HEADER_POS);
            } else arr->reuse();
            if( sp<ep ) {
                if( pos<sp || pos>ep ) {
                    arr->reuse();
                }
            } else arr->reuse();
        }
    }
    bool reset=true;
    int findsPos=config()->getInt("@findsPos");
    if( findsPos==textCursor().position() ) {
        reset=false;
    } else {
        UL64 tick=config()->getLong("@findsTick");
        if( tick ) {
            reset= (GetTickCount()-tick) > 1500 ? true: false;
        }
    }
    if( reset ) {
        config()->getArray("@finds", true);
    }

    if( onChangeCursor ) {
        QTextCursor cursor = textCursor();
        onChangeCursor( xcf, &cursor);
    }
    viewport()->update();
    updateLineNumberArea(contentsRect(), 0);
}


void MyTextEdit::textClipboardDataChanged() {
    const QMimeData *md = QApplication::clipboard()->mimeData();
    if( md->hasText() ) {
        qDebug("clipboardDataChanged => text");
    }
}

void MyTextEdit::textModificationChanged(bool modify) {
    if( onChangeModify ) {
        onChangeModify( xcf, modify );
    }
}
void MyTextEdit::textContentsChange(int pos,int removePos,int addPos) {
    StrVar* sv = config()->gv("@tabStopPostions");
    if( SVCHK('a',0) ) {
        XListArr* arr=(XListArr*)SVO;
        int mp=0;
        for( int n=0,sz=arr->size();n<sz;n++ ) {
            sv=arr->get(n);
            if( !SVCHK('0',0) ) continue;
            mp=toInteger(sv);
            if( pos<=mp ) {
                if( removePos>0 ) {
                    mp-=removePos;
                } else if( addPos>0 ) {
                    mp+=addPos;
                }
                if( mp<0 ) sv->reuse();
                else sv->setVar('0',0).addInt(mp);
            }
            if( pos>mp ) {
                arr->reuse();
                break;
            }
        }
    }

    if( onChangeText ) {
        onChangeText( xcf, pos, addPos, removePos);
    }
}

void MyTextEdit::paintEvent(QPaintEvent *event) {

    // begin QPlainTextEdit::paintEvent() copy
    QPainter painter(viewport());
    XListArr* a = NULL;
    XListArr* ta = NULL;
    StrVar* sv = config()->gv("@finds");
    if( SVCHK('a',0) ) {
        a = (XListArr*)SVO;
        if( a->size()==0 ) {
            a=NULL;
        }
    }
    sv=config()->gv("@tapPositions");
    if( SVCHK('a',0) ) {
        ta=(XListArr*)SVO;
        if( ta->size()<3 ) {
            ta=NULL;
        }
    }
    if( onDrawEditor ) {
        onDrawEditor( xcf, &painter, event->rect() );
    }
    // onEditorDraw
    if( a==NULL && ta==NULL ) {
        QTextEdit::paintEvent(event);
        return;
    }
    QColor color(200,200,80,150);
    QColor colorTab(220,220,220,150);
    QTextBlock block= cursorForPosition(QPoint(0, 1)).block();
    if( !block.isValid() ) {
        QTextEdit::paintEvent(event);
        return;
    }
    QRect rect = event->rect();
    QRect rc=getBlockRect(block).toRect();
    if( rc.y()<0 ) {
        block=block.next();
    }
    rc=getBlockRect(block).toRect();
    int top=rc.y(), bottom=top+rc.height();
    int prevY=-1, checkNum=0;
    QTextCursor tc = textCursor();
    while( block.isValid() && top <= rect.bottom()) {
        int blockNumber=block.blockNumber();
        if( prevY==rc.y() ) {
            block = block.next();
            rc=getBlockRect(block).toRect();
            if( prevY!=rc.y() ) {
                top=rc.y();
                bottom=top+rc.height();
            }
            continue;
        }
        if( block.isVisible() && bottom >= rect.top()) {
            if( a ) editor_drawMark(painter, a, block, tc, bottom, top, color);
            if( ta ) editor_drawMark(painter, ta, block, tc, bottom, top, colorTab );
        }
        prevY=rc.y();
        block = block.next();
        rc=getBlockRect(block).toRect();
        top = bottom;
        bottom = top + rc.height();
    }
    QTextEdit::paintEvent(event);
}


void MyTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) {
    if( !xcf->isNodeState(NF_LINENUM) ) return;
    // QTextDocument* doc=this->document();
    QPainter painter(xlinenum);
    QRect rect = event->rect();
    painter.fillRect(rect, QColor(220,220,220,220) ); // Qt::lightGray);
    painter.setPen(Qt::darkGray);
    painter.setFont(font());

    QTextBlock block= cursorForPosition(QPoint(0, 1)).block();
    if( block.isValid() ) {
        QRect rc=getBlockRect(block).toRect();
        if( rc.y()<0 ) {
            block=block.next();
        }
        rc=getBlockRect(block).toRect();
        int top=rc.y(), bottom=top+rc.height();
        int prevY=-1;
        while( block.isValid() && top <= rect.bottom()) {
            int blockNumber=block.blockNumber();
            if( prevY==rc.y() ) {
                block = block.next();
                rc=getBlockRect(block).toRect();
                if( prevY!=rc.y() ) {
                    top=rc.y();
                    bottom=top+rc.height();
                }
                continue;
            }
            if( block.isVisible() && bottom >= rect.top()) {
                if( onDrawLineNum ) {
                    int hei=bottom-top;
                    onDrawLineNum( xcf, &painter, blockNumber + 1, QRect(0, top, xlinenum->width(), hei) );
                } else {
                    QString number = QString::number(blockNumber + 1);
                    painter.drawText(0, top+2, xlinenum->width()-2, bottom-top, Qt::AlignRight, number);
                }
            }
            prevY=rc.y();
            block = block.next();
            rc=getBlockRect(block).toRect();
            top = bottom;
            bottom = top + rc.height();
        }
    }
}

void MyTextEdit::zoomIn(int range) {
    QFont f = font();
    const int newSize = f.pointSize() + range;
    if (newSize <= 0)
        return;
    f.setPointSize(newSize);
    setFont(f);
}

void MyTextEdit::zoomOut(int range) {
   zoomIn(-range);
}

QRectF& MyTextEdit::getBlockRect(QTextBlock& block) {
    QTextDocument* doc=this->document();
    QWidget* viewPort=this->viewport();
    mBlockRect = doc->documentLayout()->blockBoundingRect(block).translated(
            viewPort->geometry().x(), viewPort->geometry().y() - (this->verticalScrollBar()->sliderPosition()) );
    return mBlockRect;
}

QTextBlock* MyTextEdit::getLineBlock(int sp, int ep) {
    mStartBlock=document()->findBlockByNumber(sp);
    if( ep<sp ) {
        mEndBlock=document()->lastBlock();
    } else {
        mEndBlock=document()->findBlockByNumber(ep);
    }
    return &mStartBlock;
}
QTextBlock* MyTextEdit::getFirstBlock(int ep) {
    QTextDocument* doc=this->document();
    int blockNumber = editor_firstVisibleBlockPos(this);
    mStartBlock = doc->findBlockByNumber(blockNumber);
    if( ep==-1 ) {
        QRect geo=this->viewport()->geometry();
        QTextBlock block=mStartBlock;
        QTextBlock prev_block = (blockNumber > 0) ? doc->findBlockByNumber(blockNumber-1) : block;
        int translate_y = (blockNumber > 0) ? -this->verticalScrollBar()->sliderPosition() : 0;
        int top = geo.top();
        int additional_margin;
        if (blockNumber == 0)
            additional_margin = (int)doc->documentMargin() -1 - this->verticalScrollBar()->sliderPosition();
        else
            additional_margin = (int)doc->documentLayout()->blockBoundingRect(prev_block).translated(0, translate_y).intersect(geo).height();
        top += additional_margin;
        int bottom = top + (int) doc->documentLayout()->blockBoundingRect(block).height();
        // Draw the numbers (displaying the current line number in green)
        while( block.isValid() && top <=this->rect().bottom()) {
            block = block.next();
            top = bottom;
            bottom = top + (int) doc->documentLayout()->blockBoundingRect(block).height();
        }
        mEndBlock=block;
    } else {
        mEndBlock=document()->findBlock(ep);
    }
    return &mStartBlock;
}
QTextBlock* MyTextEdit::getStartBlock(int sp, int ep) {
    mStartBlock=document()->findBlock(sp);
    if( ep==sp ) {
        mEndBlock=mStartBlock;
    } else if( ep<sp ) {
        mEndBlock=document()->lastBlock();
    } else {
        mEndBlock=document()->findBlock(ep);
    }
    return &mStartBlock;
}
QTextBlock* MyTextEdit::nextBlock(QTextBlock* block) {
    if( block==NULL ) return NULL;
    if( mStartBlock==mEndBlock ) return NULL;
    mStartBlock=block->next();
    return mStartBlock.isValid() ? &mStartBlock: NULL;
}

void MyTextEdit::setScrollPosition(int ypos, int xpos) {
    this->verticalScrollBar()->setSliderPosition(ypos);
    if( xpos!=-1 ) {
        this->horizontalScrollBar()->setSliderPosition(xpos);
    }

}
void MyTextEdit::setFrameMargin(int left, int top, int right, int bottom) {
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QTextFrame* root = this->textCursor().currentFrame();
    if( root ) {
        QTextFrameFormat format =  root->frameFormat();
        if( left!=-1 ) format.setLeftMargin(left);
        if( top!=-1 ) format.setTopMargin(top);
        if( right!=-1 ) format.setRightMargin(right);
        if( bottom!=-1 ) format.setBottomMargin(bottom);
        root->setFrameFormat(format);
    }
}

void MyTextEdit::blockScrollTop(QTextBlock block) {
    QTextDocument* doc=this->document();
    QRectF rc = doc->documentLayout()->blockBoundingRect(block);
    this->setScrollPosition((int)rc.y(), (int)rc.x());
}

void MyTextEdit::scrollContentsBy(int dx, int dy) {
    if( xscrollLock  ) {
        return;
    }
    QTextEdit::scrollContentsBy(dx, dy);
}


void	MyTextEdit::slotTextChanged () {
    updateLineNumberArea(contentsRect(), 0);
}

void	MyTextEdit::slotUndoAvailable ( bool available ) {
    if( onEnableCheck ) onEnableCheck( xcf, "undo", available );
}
void	MyTextEdit::slotCopyAvailable ( bool available ) {
    if( onEnableCheck ) onEnableCheck( xcf, "copy", available );
}

void	MyTextEdit::slotRedoAvailable ( bool available ) {
    if( onEnableCheck ) onEnableCheck( xcf, "redo", available );
}


void MyTextEdit::scrollValueChange(int dy) {
    updateLineNumberArea(contentsRect(), dy);
}

void MyTextEdit::updateLineNumberAreaWidth( int newBlockCount ) {
    if( xcf->isNodeState(NF_LINENUM) ) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    } else {
        setViewportMargins(0, 0, 0, 0);
    }
}

void MyTextEdit::updateLineNumberArea( const QRect &rect, int dy ) {
    /*
    if( dy )
        xlinenum->scroll(0, dy);
    else
        xlinenum->update(0, rect.y(), xlinenum->width(), rect.height());

    if( rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
    */
    this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->sliderPosition());
    // QRect rect =  this->contentsRect();
    xlinenum->update(0, rect.y(), xlinenum->width(), rect.height());
    updateLineNumberAreaWidth(0);
    /*
    dy = this->verticalScrollBar()->sliderPosition();
    if (dy > -1) {
        xlinenum->scroll(0, dy);
    }

    // Addjust slider to alway see the number of the currently being edited line...
    int firstId = editor_firstVisibleBlockPos(this);
    if (firstId == 0 || this->textCursor().block().blockNumber() == firstId-1)
        this->verticalScrollBar()->setSliderPosition(dy-this->document()->documentMargin());
    */
}

int MyTextEdit::lineNumberAreaWidth() {

    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
    while( max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().width(QLatin1Char('9')) * digits;
    return space;
}

void MyTextEdit::resizeEvent( QResizeEvent* evt ) {
    QTextEdit::resizeEvent(evt);
    if( xcf->isNodeState(NF_LINENUM) ) {
        QRect cr = contentsRect();
        xlinenum->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
    }
}

void MyTextEdit::extraAreaMouseEvent(QMouseEvent *evt) {

    // Set whether the mouse cursor is a hand or a normal arrow
    if( evt->type() == QEvent::MouseMove) {
        int markWidth = xlinenum->width();
        bool hand = (evt->pos().x() <= markWidth);
        if( hand != (xlinenum->cursor().shape() == Qt::PointingHandCursor)) {
            xlinenum->setCursor(hand ? Qt::PointingHandCursor : Qt::ArrowCursor);
        }
    }
    StrVar* sv=config()->GetVar("@selectNumber");
    int selectNumber = SVCHK('0',0)? sv->getInt(4): -1;
    if( evt->type() == QEvent::MouseButtonPress || evt->type() == QEvent::MouseButtonDblClick) {
        bool bok=false;

        QTextCursor cursor = cursorForPosition(QPoint(0, evt->pos().y()));
        QTextCursor selection = cursor;
        if( onClickLineNum ) {
            QTextBlock block=selection.block();
            int bsp=block.position(), bep=bsp+block.length();
            bok=onClickLineNum( xcf, evt, selectNumber, bsp, bep);
        }
        if( bok) {
            selection.setVisualNavigation(true);
            config()->GetVar("@selectNumber")->setVar('0',0).addInt( selection.blockNumber() );
            selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            setTextCursor(selection);
        }
    }  else if( selectNumber >= 0 ) {
        if( evt->type() == QEvent::MouseMove) {
            QTextCursor cursor = cursorForPosition(QPoint(0, evt->pos().y()));
            QTextCursor selection = cursor;
            selection.setVisualNavigation(true);
            QTextBlock anchorBlock = document()->findBlockByNumber(selectNumber);
            selection.setPosition(anchorBlock.position());
            if( cursor.blockNumber() < selectNumber) {
                selection.movePosition(QTextCursor::EndOfBlock);
                selection.movePosition(QTextCursor::Right);
            }
            selection.setPosition(cursor.block().position(), QTextCursor::KeepAnchor);
            if(  cursor.blockNumber() >= selectNumber) {
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                selection.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
            }
            setTextCursor(selection);
        } else {
            config()->GetVar("@selectNumber")->reuse();
        }

    }
}


bool MyTextEdit::eventFilter(QObject* object, QEvent* evt) {
    switch( evt->type() )  {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onContext(xcf, e->pos().x(), e->pos().y()) ) return true;
        }
    } break;
    case QEvent::MouseButtonPress: {
        if( onMouseDown ) {
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onMouseDown(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
        }
    } break;
    case QEvent::MouseButtonRelease: {
        if( onMouseUp ) {
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onMouseUp(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
        }
    } break;
    case QEvent::MouseMove: {
        if( onMouseMove ) {
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onMouseMove(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
        }
    } break;
    case QEvent::MouseButtonDblClick: {
        if( onDoubleClick ) {
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onDoubleClick(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
        }
    } break;
    case QEvent::KeyPress: {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(evt);
        if( onKeyDown ) {
            xcf->set("@sender", object==viewport()?"view":"this");
            if( onKeyDown(xcf, keyEvent) ) return true;
        }
        QTextCursor c = textCursor();
        if( keyEvent->key() == Qt::Key_Tab) {
            if( c.hasSelection()) {
                editor_tabIndent(document(), c, true);
                setTextCursor(c);
                return true;
            }
        } else if( keyEvent->key() == Qt::Key_Backtab
            || (keyEvent->key() == Qt::Key_Tab && keyEvent->modifiers() & Qt::ShiftModifier))
        {
            QTextCursor c = textCursor();
            if( c.hasSelection()) {
                editor_tabIndent(document(), c, false);
                setTextCursor(c);
                return true;
            }
        } else if( keyEvent->key() == Qt::Key_Return ) {
            QTextCursor c = textCursor();
            QString text = c.block().text();
            int indent = firstNonSpace(text);
            if( indent ) {
                c.beginEditBlock();
                c.insertBlock();
                c.insertText(text.mid(0,indent));
                c.endEditBlock();
            } else {
                c.insertBlock();
            }
            setTextCursor(c);
            return true;
        }
    } break;
    default: break;
    }
    return false;
}

void MyTextEdit::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    print(printer);
#endif
}

bool MyTextEdit::canInsertFromMimeData(const QMimeData* source) const {
    return source->hasImage() || source->hasUrls() ||
        QTextEdit::canInsertFromMimeData(source);
}

void MyTextEdit::insertFromMimeData(const QMimeData* source) {
    /*
    if (source->hasImage()) {
        static int i = 1;
        QUrl url(QString("dropped_image_%1").arg(i++));
        dropImage(url, qvariant_cast<QImage>(source->imageData()));
    } else if (source->hasUrls()) {
        foreach (QUrl url, source->urls()) {
            QFileInfo info(url.toLocalFile());
            if (QImageReader::supportedImageFormats().contains(info.suffix().toLower().toLatin1()))
                dropImage(url, QImage(info.filePath()));
            else
                dropTextFile(url);
        }
    }
    */
    if( onMimeData && onMimeData( xcf, source) ) {

    } else {
        QTextEdit::insertFromMimeData(source);
    }
}

void MyTextEdit::dropImage(const QUrl& url, const QImage& image) {
    if (!image.isNull()) {
        document()->addResource(QTextDocument::ImageResource, url, image);
        textCursor().insertImage(url.toString());
    }
}

void MyTextEdit::dropTextFile(const QUrl& url) {
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        textCursor().insertText(file.readAll());
    }
}

void MyTextEdit::setCharFormat(LPCC code, TreeNode* cf) {
    editor_charFormats(xsyntax, code, cf, config() );
}




MyMenu::MyMenu(WidgetConf *cf, QWidget *p) : QMenu(p), xcf(cf) {
    if( xcf ) xcf->xwidget=NULL;
}

MyMenu::~MyMenu() { }

void MyMenu::mouseReleaseEvent(QMouseEvent *e)
{
    QAction *action = activeAction();
    if( action && action->isEnabled()) {
        action->setEnabled(false);
        QMenu::mouseReleaseEvent(e);
        action->setEnabled(true);
        action->trigger();
        this->close();
    }
    else
        QMenu::mouseReleaseEvent(e);
}

MyPushButton::MyPushButton(WidgetConf *cf, QWidget *p) : QPushButton(p), xcf(cf) {
    onContext=NULL;
    onClick=NULL;
    onTimeOut=NULL;
    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()) );
    if( xcf ) xcf->xwidget=this;
}

MyPushButton::~MyPushButton() { }

void MyPushButton::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf, NULL); }
void MyPushButton::slotClicked()
{
    xcf->set("clicked", "button");
    if( onClick ) {
        onClick(xcf);
    }
}

bool MyPushButton::event(QEvent *evt) {
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return onContext(xcf, e->pos().x(), e->pos().y()  );
        }
    } break;
    }
    return QPushButton::event(evt);
}


/*========================================================================
*    WidgetMap
========================================================================*/

WidgetMap::WidgetMap(QWidget* parent) {
    xcf.set("tag","widgetConf");
    xrootConf=NULL;
    parentWidget=parent;
}
WidgetMap::~WidgetMap() {
    StrVar* sv=NULL;
    xmap.removeAll(true);
    for( int n=0; n<xcf.xobjects.size(); n++ ) {
        sv=xcf.xobjects.get(n);
        if( SVCHK('n',1) ) {
            WidgetConf* conf=(WidgetConf*)SVO;
            SAFE_DELETE(conf);
        }
    }
    xcf.removeAll(true);
}

bool WidgetMap::isLayoutNode(TreeNode* node) {
    if( node==NULL ) return false;
    LPCC tag=node->get("tag");
    return ccmp(tag,"page") || ccmp(tag,"vbox") || ccmp(tag,"hbox") || ccmp(tag,"grid") ? true: false;
}

WidgetConf* WidgetMap::findLayoutNode(WidgetConf* node) {
    // if( isLayoutNode(node) ) return node;
    TreeNode* p=node->parent();
    while( p ) {
        if( isLayoutNode(p) ) {
            return p&&p->cmp("@objectType","WidgetConf") ? static_cast<WidgetConf*>(p): NULL;
        }
        p=p->parent();
    }
    return NULL;
}

WidgetConf* WidgetMap::findParentNode(WidgetConf* node) {
    TreeNode* p=node->parent();
    if( p && p->cmp("tag","row") ) p=p->parent();
    return p&&p->cmp("@objectType","WidgetConf") ? static_cast<WidgetConf*>(p): NULL;
}

WidgetConf *WidgetMap::findId(LPCC id)
{
    StrVar* sv=xcf.gv(id);
    if( SVCHK('n',1) ) {
        return (WidgetConf*)SVO;
    }
    return xrootConf;
}

WidgetConf *WidgetMap::findTag(LPCC tag, int index, WidgetConf *parent)
{
    if( parent==NULL ) {
        if( xrootConf==NULL ) return NULL;
        parent=xrootConf;
    }

    for(int n=0;n<parent->childCount();n++ ) {
        WidgetConf* cur=getWidgetConf( parent->child(n) );
        if( cur ) {
            if( cur->cmp("tag",tag) ) {
                if( index==0 ) return cur;
                if( cur->getInt("index")==index ) {
                    return cur;
                }
            }
            if( cur->childCount() ) {
                cur=findTag(tag, index, cur);
                if( cur ) return cur;
            }
        }
    }
    return NULL;
}

bool WidgetMap::setWidgetScroll(WidgetConf *cf) {
    QWidget* w=cf->xwidget;
    StrVar* sv=cf->GetVar("@scroll");
    if( w && !SVCHK('W',20) ) {
        QScrollArea* scroll=new QScrollArea();
        scroll->setWidget(w);
        scroll->setWidgetResizable(false);
        sv->setVar('W',20,(LPVOID)scroll);
        return true;
    }
    return false;
}

int WidgetMap::setClickEvent(PWIDGET_CLICK fpClick) {
    int num=0;
    for( WBoxNode* bn=xmap.First(); bn; bn=xmap.Next() ) {
        LPCC id=xmap.getCurCode();
        StrVar* sv=xcf.gv(id);
        if( SVCHK('n',1) ) {
            WidgetConf* wcf=(WidgetConf*)SVO;
            bool ok=true;
            switch(wcf->xtype ) {
            case 10: myCanvas(wcf)->onClick=fpClick; break;
            case 25: myButton(wcf)->onClick=fpClick; break;
            case 26: myToolButton(wcf)->onClick=fpClick; break;
            case 27: myRadio(wcf)->onClick=fpClick; break;
            case 28: myCheck(wcf)->onClick=fpClick; break;
            case 37: myCalendar(wcf)->onClick=fpClick; break;
            case 102: myTray(wcf)->onClick=fpClick; break;
            default: ok=false; break;
            }
            if( ok ) num++;
        }
    }
    return num;
}

int WidgetMap::setChangeEvent(PWIDGET_CHANGE fpChange){
    int num=0;
    for( WBoxNode* bn=xmap.First(); bn; bn=xmap.Next() ) {
        LPCC id=xmap.getCurCode();
        StrVar* sv=xcf.gv(id);
        if( SVCHK('n',1) ) {
            WidgetConf* wcf=(WidgetConf*)SVO;
            switch(wcf->xtype ) {
            case 22: myProgress(wcf)->onChange=fpChange; break;
            case 23: myCombo(wcf)->onChange=fpChange; break;
            case 90: myTab(wcf)->onChange=fpChange; break;
            case 93: myDiv(wcf)->onChange=fpChange; break;
            case 28: myCheck(wcf)->onChange=fpChange; break;
            case 29: mySpin(wcf)->onChange=fpChange; break;
            case 32: myInput(wcf)->onChange=fpChange; break;
            case 35: myDate(wcf)->onChange=fpChange; break;
            case 36: myTime(wcf)->onChange=fpChange; break;
            case 37: myCalendar(wcf)->onChange=fpChange; break;
            case 38: myToolbar(wcf)->onChange=fpChange; break;
            case 39: myToolbox(wcf)->onChange=fpChange; break;
            case 100: myAction(wcf)->onChange=fpChange; break;
            default: return false;
            }
            num++;
        }
    }
    return num;
}

int WidgetMap::setLinkEvent(PWIDGET_LINK_CLICK fpClick, PWIDGET_LINK_HOVER fpHover)
{
    int num=0;
    for( WBoxNode* bn=xmap.First(); bn; bn=xmap.Next() ) {
        LPCC id=xmap.getCurCode();
        StrVar* sv=xcf.gv(id);
        if( SVCHK('n',1) ) {
            WidgetConf* cf=(WidgetConf*)SVO;
            if( cf->xtype==21 && cf->xwidget ) {
                MyLabel* w=qobject_cast<MyLabel*>(cf->xwidget);
                w->onLinkClick=fpClick;
                w->onLinkHover=fpHover;
            }
        }
    }
    return num;
}

int WidgetMap::startTimeOut(LPCC id, int interval, PWIDGET_TIMEOUT fpTimeOut)
{
    WidgetConf* wcf=findId(id);
    if( wcf->xwidget==NULL ) return 0;
    bool ok=true;
    switch( wcf->xtype ) {
    case 10: {
       MyCanvas* w=qobject_cast<MyCanvas*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 11: {
       MyTree* w=qobject_cast<MyTree*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 12: {
       MyTextEdit* w=qobject_cast<MyTextEdit*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 21: {
       MyLabel* w=qobject_cast<MyLabel*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 22: {
       MyProgressBar* w=qobject_cast<MyProgressBar*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       w->onTimeOut=fpTimeOut;
    } break;
    case 23: {
       MyComboBox* w=qobject_cast<MyComboBox*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 24: {
       MyListView* w=qobject_cast<MyListView*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 90: {
       MyTabWidget* w=qobject_cast<MyTabWidget*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 93: {
       MyStackedWidget* w=qobject_cast<MyStackedWidget*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 25: {
       MyPushButton* w=qobject_cast<MyPushButton*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 26: {
       MyToolButton* w=qobject_cast<MyToolButton*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       w->onTimeOut=fpTimeOut;
    } break;
    case 27: {
       MyRadioButton* w=qobject_cast<MyRadioButton*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 28: {
       MyCheckBox* w=qobject_cast<MyCheckBox*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 29: {
       MySpinBox* w=qobject_cast<MySpinBox*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 32: {
       MyLineEdit* w=qobject_cast<MyLineEdit*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 35: {
       MyDateEdit* w=qobject_cast<MyDateEdit*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 36: {
       MyTimeEdit* w=qobject_cast<MyTimeEdit*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 37: {
       MyCalendarWidget* w=qobject_cast<MyCalendarWidget*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       w->onTimeOut=fpTimeOut;
    } break;
    case 38: {
       MyToolBar* w=qobject_cast<MyToolBar*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    case 39: {
       MyToolBox* w=qobject_cast<MyToolBox*>(wcf->xwidget);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       w->onTimeOut=fpTimeOut;
    } break;
    case 102: {
       MySystemTrayIcon* w=tray(id);
       w->connect(&xtimer, SIGNAL(timeout()), SLOT(slotTimeOut()) );
       if( fpTimeOut ) w->onTimeOut=fpTimeOut;
    } break;
    default: ok=false; break;
    }
    if( ok ) {
        xtimer.start(interval );
    }
}

int WidgetMap::stopTimeOut()
{
    if( xtimer.isActive() ) {
       xtimer.stop();
       return 1;
    }
    return 0;
}

void WidgetMap::setLayoutProp(WidgetConf *cf, QLayout *layout, LPCC tag) {
    StrVar* sv=cf->gv("margin");
    cf->xlayout=layout;
    if( sv ) {
        if( isNumberVar(sv) ) {
            int num=toInteger(sv);
            layout->setMargin(num);
        } else {
            XParseVar pv(sv);
            int l=0,t=0,r=0,b=0;
            for(int n=0; n<4 && pv.valid(); n++ ) {
                LPCC next=pv.NextWord();
                if( isnum(next) ) {
                    if( n==0 ) l=atoi(next);
                    else if( n==1 ) t=atoi(next);
                    else if( n==2 ) r=atoi(next);
                    else if( n==3 ) b=atoi(next);
                }
                char ch=pv.ch();
                if( isoper(ch) || ch==',' ) pv.incr();
            }
            layout->setContentsMargins(l,t,r,b);
        }

    }
    sv=cf->gv("align");
    if( sv ) {
        layout->setAlignment((Qt::Alignment)getAlignFlag(toString(sv)) );
    }
    sv=cf->gv("spacing");
    if( isNumberVar(sv) ) {
        layout->setSpacing(toInteger(sv));
    }
}


QLayout* WidgetMap::createLayout(WidgetConf *layoutCf, QWidget *parent)
{
    if( layoutCf==NULL ) return NULL;
    LPCC tag=layoutCf->get("tag");
    if( ccmp(tag,"grid") ) {
        MyGridLayout* layout = grid(layoutCf->get("id"), layoutCf);
        setLayoutProp(layoutCf, layout, tag);
        layoutCf->setInt("@r",0);
        layoutCf->setInt("@c",0);
        createChild(layoutCf, parent);
        return layout;
    } else if( ccmp(tag,"hbox") ) {
        MyHBox* box = hbox(layoutCf->get("id"), layoutCf);
        setLayoutProp(layoutCf, box, tag);
        createChild(layoutCf, parent);
        return box;
    } else if( ccmp(tag,"vbox") || ccmp(tag,"page") ) {
        MyVBox* box = vbox(layoutCf->get("id"), layoutCf);
        setLayoutProp(layoutCf, box, tag);
        createChild(layoutCf, parent);
        return box;
    }
    return NULL;
}

QWidget* WidgetMap::createWidget(WidgetConf *wcf, QWidget *parent)
{
    QWidget* widget=NULL;
    LPCC tag=wcf->get("tag");
    wcf->xwidgetMap=this;
    if( ccmp(tag,"action") || ccmp(tag,"menu") || ccmp(tag,"tray") ) {
        if( ccmp(tag,"action") ) {
            initAction(wcf);
        } else if( ccmp(tag,"menu") ) {
            initMenu(wcf);
        } else if( ccmp(tag,"tray") ) {
            initTray(wcf);
        }
        if( wcf->childCount()>0 ) {
            for( int n=0,cnt=wcf->childCount();n<cnt;n++ ) {
                WidgetConf* cf=(WidgetConf*)wcf->child(n);
                createWidget(cf, parent);
            }
        }
    } else if( ccmp(tag,"button") ) {
        widget=qobject_cast<QWidget*>(initButton(wcf) );
    } else if( ccmp(tag,"canvas") ) {
        widget=qobject_cast<QWidget*>(initCanvas(wcf) );
    } else if( ccmp(tag,"tree") ) {
        widget=qobject_cast<QWidget*>(initTree(wcf) );
    } else if( ccmp(tag,"editor") ) {
        widget=qobject_cast<QWidget*>(initEditor(wcf) );
    } else if( ccmp(tag,"label") ) {
        widget=qobject_cast<QWidget*>(initLabel(wcf) );
    } else if( ccmp(tag,"progress") ) {
        widget=qobject_cast<QWidget*>(initProgress(wcf) );
    } else if( ccmp(tag,"combo") ) {
        widget=qobject_cast<QWidget*>(initCombo(wcf) );
    } else if( ccmp(tag,"list") ) {
        widget=qobject_cast<QWidget*>(initList(wcf) );
    } else if( ccmp(tag,"tab") ) {
        widget=qobject_cast<QWidget*>(initTab(wcf) );
    } else if( ccmp(tag,"splitter") ) {
        widget=qobject_cast<QWidget*>(initSplitter(wcf) );
    } else if( ccmp(tag,"group") ) {
        widget=qobject_cast<QWidget*>(initGroup(wcf) );
    } else if( ccmp(tag,"div") ) {
        widget=qobject_cast<QWidget*>(initDiv(wcf) );
    } else if( ccmp(tag,"toolbutton") ) {
        widget=qobject_cast<QWidget*>(initToolButton(wcf) );
    } else if( ccmp(tag,"radio") ) {
        widget=qobject_cast<QWidget*>(initRadio(wcf) );
    } else if( ccmp(tag,"check") ) {
        widget=qobject_cast<QWidget*>(initCheck(wcf) );
    } else if( ccmp(tag,"spin") ) {
        widget=qobject_cast<QWidget*>(initSpin(wcf) );
    } else if( ccmp(tag,"input") ) {
        widget=qobject_cast<QWidget*>(initInput(wcf) );
    } else if( ccmp(tag,"input") ) {
        widget=qobject_cast<QWidget*>(initInput(wcf) );
    } else if( ccmp(tag,"textarea") ) {
        widget=qobject_cast<QWidget*>(initTextArea(wcf) );
    } else if( ccmp(tag,"date") ) {
        widget=qobject_cast<QWidget*>(initDate(wcf) );
    } else if( ccmp(tag,"time") ) {
        widget=qobject_cast<QWidget*>(initTime(wcf) );
    } else if( ccmp(tag,"input") ) {
        widget=qobject_cast<QWidget*>(initInput(wcf) );
    } else if( ccmp(tag,"calendar") ) {
        widget=qobject_cast<QWidget*>(initCalendar(wcf) );
    } else if( ccmp(tag,"toolbar") ) {
        widget=qobject_cast<QWidget*>(initToolbar(wcf) );
    } else if( ccmp(tag,"toolbox") ) {
        widget=qobject_cast<QWidget*>(initToolbox(wcf) );
    } else if( ccmp(tag,"player") ) {
        widget=qobject_cast<QWidget*>(initPlayer(wcf) );
    } else if( ccmp(tag,"webview") ) {
        widget=qobject_cast<QWidget*>(initWebView(wcf) );
    } else {

    }
    if( widget ) {
        wcf->xwidget=widget;
        initWidget(wcf, widget);
    }
    return widget;
}

bool WidgetMap::createChild(WidgetConf *root, QWidget *parent)
{
    for( int n=0,cnt=root->childCount();n<cnt;n++ ) {
        WidgetConf* cf=static_cast<WidgetConf*>(root->child(n));
        LPCC tag=cf->get("tag");
        if( ccmp(tag,"row") ) {
            WidgetConf* grid=static_cast<WidgetConf*>(cf->parent());
            if( grid && grid->cmp("tag","grid") ) {
                grid->incrNum("@r");
                grid->setInt("@c",0);
            }
            createChild(cf,parent);
            continue;
        } else if( ccmp(tag,"xml") ) {
            continue;
        }
        QLayout* layout=NULL;
        QWidget* widget=NULL;
        bool space=ccmp(tag,"space");
        if( space==false ) {
            if( isLayoutNode(cf) ) {
                layout=createLayout(cf, parent);
                cf->xlayout=layout;
            } else {
                widget=createWidget(cf, parent);
                cf->xwidget=widget;
                if( cf->childCount() ) {
                    if( ccmp(tag,"tab") || ccmp(tag,"group") || ccmp(tag,"splitter") || ccmp(tag,"div") ) {
                        createChild(cf,widget);
                    }
                }
            }
        }
        WidgetConf* parentCf = findParentNode(cf);
        if( parentCf==NULL ) {
            continue;
        }

        if( parentCf->xtype<10 ) {
            switch(  parentCf->xtype ) {
            case 3: {
                QGridLayout* grid = qobject_cast<QGridLayout*>(parentCf->xlayout);
                if( grid ) {
                    int r=parentCf->getInt("@r");
                    int c=parentCf->getInt("@c");
                    if( parentCf->isset("vspace") ) {
                        grid->setVerticalSpacing(toInteger(parentCf->gv("vspace")) );
                    }
                    if( parentCf->isset("hspace") ) {
                        grid->setHorizontalSpacing(toInteger(parentCf->gv("hspace")) );
                    }
                    int rs=cf->getInt("rowspan",1);
                    int cs=cf->getInt("colspan",1);
                    int stretch=cf->getInt("stretch"), columnStretch=stretch;
                    if( columnStretch==0 ) {
                        columnStretch=cf->getInt("columnStretch");
                        if( columnStretch ) grid->setColumnStretch(r,columnStretch );
                    }
                    int rowStretch=cf->getInt("rowStretch");
                    if( rowStretch ) {
                        grid->setRowStretch(c,rowStretch);
                    }
                    int minHeight=cf->getInt("minHeight");
                    int minWidth=cf->getInt("minWidth");
                    if( minHeight ) grid->setRowMinimumHeight(c, minHeight);
                    if( minWidth ) grid->setColumnMinimumWidth(c, minWidth);
                    if( layout ) {
                        grid->addLayout( layout, r, c, rs, cs  );
                    } else {
                        grid->addWidget( widget, r, c, rs, cs );
                    }
                    parentCf->incrNum("@c", cs);
                    if( c==0 ) {
                        WidgetConf* row=static_cast<WidgetConf*>(cf->parent());
                        if( row ) {
                            int stretch=row->getInt("stretch");
                            if( stretch>0 ) grid->setRowStretch(r,stretch);
                        }
                    }
                }
            } break;
            case 2:
            case 1: {
                QBoxLayout* box = qobject_cast<QBoxLayout*>(parentCf->xlayout);
                // qDebug("[layout] layout tag: %s %s", parentCf->get("tag"), box?"ok": "fail" );
                if( box ) {
                    int stretch=cf->getInt("stretch");
                    if( space ) {
                        if( stretch==0 ) stretch=1;
                        box->addStretch(stretch);
                    } else if( layout ) {
                        box->addLayout(layout, stretch);
                    } else {
                        box->addWidget(widget, stretch);
                    }
                }
            } break;
            default: {

            } break;
            }
        } else if( widget || layout ) {
            tag=parentCf->get("tag");
            _LOG("xxxxxxxx(tag:%s) id=%s xxxxxxxxxxxx", tag, cf->get("id"));
            if( ccmp(tag,"tab") ) {
                MyTabWidget* tab = qobject_cast<MyTabWidget*>(parentCf->xwidget);
                LPCC title=cf->get("title");
                if( widget ) {
                    tab->addTab(widget,KR(title));
                } else if( layout ) {
                    widget= new QWidget(tab);
                    widget->setLayout(layout);
                    tab->addTab(widget,KR(title));
                }
            } else if( ccmp(tag,"splitter") ) {
                MySplitter* splitter = qobject_cast<MySplitter*>(parentWidget);
                if( widget ) {
                    splitter->addWidget(widget);
                } else if( layout ) {
                    widget= new QWidget(splitter);
                    widget->setLayout(layout);
                    splitter->addWidget(widget);
                }
            } else if( ccmp(tag,"div") ) {
                MyStackedWidget* div = qobject_cast<MyStackedWidget*>(parentWidget);
                if( widget ) {
                    div->addWidget(widget);
                } else if( layout ) {
                    widget= new QWidget(div);
                    widget->setLayout(layout);
                    div->addWidget(widget);
                }
            } else if( ccmp(tag,"group") ) {
                MyGroupBox* group = qobject_cast<MyGroupBox*>(parentWidget);
                if( parentCf->isset("title") ) {
                    LPCC title=parentCf->get("title");
                    group->setTitle(KR(title));
                }
                if( widget ) {
                    QVBoxLayout* vbox=new QVBoxLayout();
                    vbox->addWidget(widget);
                    group->setLayout(vbox);
                } else if( layout ) {
                    group->setLayout(layout);
                }
            }
        }
    }
    return true;
}


bool WidgetMap::initPage(LPCC uri, PWIDGET_INIT fpInit)
{
    StrVar* sv=cfVar("@firstCall");
    if( !checkObject(sv,'3') ) {
        sv->setVar('3',1);
    }
    if( isVarTrue(sv) ) {
        // @firstCall
        qInstallMsgHandler(userPrintOutput);
        sv->setVar('3',0);
    }
    StrVar* err=cfVar("@error", true);
    if( parentWidget==NULL ) {
        err->fmt("page widget is null (uri:%s)", uri);
        return false;
    }
    if( StartWith(uri,"http") ) {
        getUri(uri, xresult.reuse(), 600, err );
    } else {
        fileReadAll(uri, xresult.reuse());
    }
    xcf.setJson(XParseVar(&xresult), (TreeNode*)&xcf );

    if( xcf.isset("title") ) {
        LPCC title=toString(xcf.gv("title"));
        parentWidget->setWindowTitle(KR(title));
    }
    if( xcf.isset("icon") ) {
        parentWidget->setWindowIcon(getQIcon(xcf.get("icon")));
    }
    if( xresult.length()==0 ) {
        err->fmt("init page error (%s result is empty)", uri);
    }
    QLayout* layout=NULL;
    sv=xcf.gv("page");
    if( SVCHK('n',1) ) {
        xrootConf=(WidgetConf*)SVO;
        if( !xrootConf->isset("id") ) {
            xrootConf->set("id", "main");
        }
        xrootConf->set("tag","page");
        layout=createLayout(xrootConf, parentWidget );
        _LOG("initPage info [%s]", xrootConf->log(NULL,true) );
    }

    if( layout && parentWidget ) {
        parentWidget->setLayout(layout);
        if( fpInit ) {
            fpInit(this);
        } else {
            setEventFunc_initPage(this);
        }
        return true;
    }    
    return false;
}



QWidget* WidgetMap::widget(LPCC code, QWidget* w) {
    if( w ) {
        xmap.GetVar(code)->setVar('W',0,(LPVOID)w);
    } else {
        StrVar* sv=xmap.gv(code);
        if( checkObject(sv,'W') ) {
            w=qobject_cast<QWidget*>(SVQ);
        }
    }
    return w;
}
MyVBox* WidgetMap::vbox(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyVBox* layout=NULL;
    if( SVCHK('W',1) ) {
        layout=qobject_cast<MyVBox*>(SVQ);
    } else {
        layout=new MyVBox(wConf(code,"vbox",1,wcf), wcf && wcf->xwidget? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',1,(LPVOID)layout);
    }
    return layout;
}
MyHBox* WidgetMap::hbox(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyHBox* layout=NULL;
    if( SVCHK('W',2) ) {
        layout=qobject_cast<MyHBox*>(SVQ);
    } else {
        layout=new MyHBox(wConf(code,"hbox",2,wcf), wcf && wcf->xwidget? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',2,(LPVOID)layout);
    }
    return layout;
}
MyGridLayout* WidgetMap::grid(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyGridLayout* layout=NULL;
    if( SVCHK('W',3) ) {
        layout=qobject_cast<MyGridLayout*>(SVQ);
    } else {
        layout=new MyGridLayout(wConf(code,"grid",3,wcf), wcf && wcf->xwidget? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',3,(LPVOID)layout);
    }
    return layout;
}
MyCanvas* WidgetMap::canvas(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyCanvas* w=NULL;
    if( SVCHK('W',10) ) {
        w=qobject_cast<MyCanvas*>(SVQ);
    } else {
        w=new MyCanvas(wConf(code,"canvas",10,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',10,(LPVOID)w);
    }
    return w;
}
MyTree* WidgetMap::tree(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyTree* w=NULL;
    if( SVCHK('W',11) ) {
        w=qobject_cast<MyTree*>(SVQ);
    } else {
        w=new MyTree(wConf(code,"tree",11,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',11,(LPVOID)w);
    }
    return w;
}
MyTextEdit* WidgetMap::editor(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyTextEdit* w=NULL;
    if( SVCHK('W',12) ) {
        w=qobject_cast<MyTextEdit*>(SVQ);
    } else {
        w=new MyTextEdit(wConf(code,"editor",12,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',12,(LPVOID)w);
    }
    return w;
}

MyLabel* WidgetMap::label(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyLabel* w=NULL;
    if( SVCHK('W',21) ) {
        w=qobject_cast<MyLabel*>(SVQ);
    } else {
        w=new MyLabel(wConf(code,"label",21,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',21,(LPVOID)w);
    }
    return w;
}
MyProgressBar* WidgetMap::progress(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyProgressBar* w=NULL;
    if( SVCHK('W',22) ) {
        w=qobject_cast<MyProgressBar*>(SVQ);
    } else {
        w=new MyProgressBar(wConf(code,"progress",22,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',22,(LPVOID)w);
    }
    return w;
}
MyComboBox* WidgetMap::combo(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyComboBox* w=NULL;
    if( SVCHK('W',23) ) {
        w=qobject_cast<MyComboBox*>(SVQ);
    } else {
        w=new MyComboBox(wConf(code,"combo",23,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',23,(LPVOID)w);
    }
    return w;
}
MyListView* WidgetMap::list(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyListView* w=NULL;
    if( SVCHK('W',24) ) {
        w=qobject_cast<MyListView*>(SVQ);
    } else {
        w=new MyListView(wConf(code,"list",24,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',24,(LPVOID)w);
    }
    return w;
}
MyTabWidget* WidgetMap::tab(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyTabWidget* w=NULL;
    if( SVCHK('W',90) ) {
        w=qobject_cast<MyTabWidget*>(SVQ);
    } else {
        w=new MyTabWidget(wConf(code,"tab",90,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',90,(LPVOID)w);
    }
    return w;
}
MySplitter* WidgetMap::splitter(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MySplitter* w=NULL;
    if( SVCHK('W',91) ) {
        w=qobject_cast<MySplitter*>(SVQ);
    } else {
        w=new MySplitter(wConf(code,"splitter",91,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',91,(LPVOID)w);
    }
    return w;
}
MyGroupBox* WidgetMap::group(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyGroupBox* w=NULL;
    if( SVCHK('W',92) ) {
        w=qobject_cast<MyGroupBox*>(SVQ);
    } else {
        w=new MyGroupBox(wConf(code,"group",92,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',92,(LPVOID)w);
    }
    return w;
}
MyStackedWidget* WidgetMap::div(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyStackedWidget* w=NULL;
    if( SVCHK('W',93) ) {
        w=qobject_cast<MyStackedWidget*>(SVQ);
    } else {
        w=new MyStackedWidget(wConf(code,"div",93,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',93,(LPVOID)w);
    }
    return w;
}
MyPushButton* WidgetMap::button(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyPushButton* w=NULL;
    if( SVCHK('W',25) ) {
        w=qobject_cast<MyPushButton*>(SVQ);
    } else {
        w=new MyPushButton(wConf(code,"button",25,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',25,(LPVOID)w);
    }
    return w;
}
MyToolButton* WidgetMap::tool(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyToolButton* w=NULL;
    if( SVCHK('W',26) ) {
        w=qobject_cast<MyToolButton*>(SVQ);
    } else {
        w=new MyToolButton(wConf(code,"tool",26,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',26,(LPVOID)w);
    }
    return w;
}

MyRadioButton* WidgetMap::radio(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyRadioButton* w=NULL;
    if( SVCHK('W',27) ) {
        w=qobject_cast<MyRadioButton*>(SVQ);
    } else {
        w=new MyRadioButton(wConf(code,"radio",27,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',27,(LPVOID)w);
    }
    return w;
}
MyCheckBox* WidgetMap::check(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyCheckBox* w=NULL;
    if( SVCHK('W',28) ) {
        w=qobject_cast<MyCheckBox*>(SVQ);
    } else {
        w=new MyCheckBox(wConf(code,"check",28,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',28,(LPVOID)w);
    }
    return w;
}
MySpinBox* WidgetMap::spin(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MySpinBox* w=NULL;
    if( SVCHK('W',29) ) {
        w=qobject_cast<MySpinBox*>(SVQ);
    } else {
        w=new MySpinBox(wConf(code,"spin",29,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',29,(LPVOID)w);
    }
    return w;
}
MyLineEdit* WidgetMap::input(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyLineEdit* w=NULL;
    if( SVCHK('W',32) ) {
        w=qobject_cast<MyLineEdit*>(SVQ);
    } else {
        w=new MyLineEdit(wConf(code,"input",32,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',32,(LPVOID)w);
    }
    return w;
}
MyGraphicsView* WidgetMap::graphics(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyGraphicsView* w=NULL;
    if( SVCHK('W',33) ) {
        w=qobject_cast<MyGraphicsView*>(SVQ);
    } else {
        w=new MyGraphicsView(wConf(code,"canvas",33,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',33,(LPVOID)w);
    }
    return w;
}
MyPlainTextEdit* WidgetMap::textarea(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyPlainTextEdit* w=NULL;
    if( SVCHK('W',34) ) {
        w=qobject_cast<MyPlainTextEdit*>(SVQ);
    } else {
        w=new MyPlainTextEdit(wConf(code,"textarea",34,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',34,(LPVOID)w);
    }
    return w;
}
MyDateEdit* WidgetMap::date(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyDateEdit* w=NULL;
    if( SVCHK('W',35) ) {
        w=qobject_cast<MyDateEdit*>(SVQ);
    } else {
        w=new MyDateEdit(wConf(code,"date",35,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',35,(LPVOID)w);
    }
    return w;
}
MyTimeEdit* WidgetMap::time(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    qDebug("xxxxxxx time xxxxxxxx %s", code );
    MyTimeEdit* w=NULL;
    if( SVCHK('W',36) ) {
        w=qobject_cast<MyTimeEdit*>(SVQ);
    } else {
        w=new MyTimeEdit(wConf(code,"time",36,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',36,(LPVOID)w);
    }
    return w;
}
MyCalendarWidget* WidgetMap::calendar(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyCalendarWidget* w=NULL;
    if( SVCHK('W',37) ) {
        w=qobject_cast<MyCalendarWidget*>(SVQ);
    } else {
        w=new MyCalendarWidget(wConf(code,"calendar",37,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',37,(LPVOID)w);
    }
    return w;
}
MyToolBar* WidgetMap::toolbar(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyToolBar* w=NULL;
    if( SVCHK('W',38) ) {
        w=qobject_cast<MyToolBar*>(SVQ);
    } else {
        w=new MyToolBar(wConf(code,"toolbar",38,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',38,(LPVOID)w);
    }
    return w;
}
MyToolBox* WidgetMap::toolbox(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyToolBox* w=NULL;
    if( SVCHK('W',39) ) {
        w=qobject_cast<MyToolBox*>(SVQ);
    } else {
        w=new MyToolBox(wConf(code,"toolbox",39,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',39,(LPVOID)w);
    }
    return w;
}
MyDockWidget* WidgetMap::dock(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyDockWidget* w=NULL;
    if( SVCHK('W',50) ) {
        w=qobject_cast<MyDockWidget*>(SVQ);
    } else {
        w=new MyDockWidget(wConf(code,"doc",50,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',50,(LPVOID)w);
    }
    return w;
}
MyAction* WidgetMap::action(LPCC code, WidgetConf* wcf) {
    StrVar* sv=xmap.gv(code);
    MyAction* w=NULL;
    if( SVCHK('W',100) ) {
        w=qobject_cast<MyAction*>(SVQ);
    } else {
        w=new MyAction(wConf(code,"action",100,wcf), wcf && wcf->xparentObject? wcf->xparentObject: parentWidget);
        xmap.GetVar(code)->setVar('W',100,(LPVOID)w);
    }
    return w;
}
MyMenu* WidgetMap::menu(LPCC code, WidgetConf* wcf) {
    if( slen(code)==0 ) code="@menu";
    StrVar* sv=xmap.gv(code);
    MyMenu* w=NULL;
    if( SVCHK('W',101) ) {
        w=qobject_cast<MyMenu*>(SVQ);
    } else {
        w=new MyMenu(wConf(code,"menu",101,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',101,(LPVOID)w);
    }
    return w;
}
MySystemTrayIcon* WidgetMap::tray(LPCC code, WidgetConf* wcf) {
    if( slen(code)==0 ) code="@tray";
    StrVar* sv=xmap.gv(code);
    MySystemTrayIcon* w=NULL;
    if( SVCHK('W',102) ) {
        w=qobject_cast<MySystemTrayIcon*>(SVQ);
    } else {
        w=new MySystemTrayIcon(wConf(code,"tray",102,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',102,(LPVOID)w);
    }
    return w;
}

#ifdef MPLAYER_USE
MyMPlayer *WidgetMap::player(LPCC code, WidgetConf *wcf) {
    if( slen(code)==0 ) code="#player";
    int idx=201;
    StrVar* sv=xmap.gv(code);
    MyMPlayer* w=NULL;
    if( SVCHK('W',idx) ) {
        w=qobject_cast<MyMPlayer*>(SVQ);
    } else {
        w=new MyMPlayer(wConf(code,"tray",idx,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',idx,(LPVOID)w);
    }
    return w;
}
#endif

#ifdef WEBVIEW_USE
WebView* WidgetMap::webview(LPCC code, WidgetConf* wcf) {
    if( slen(code)==0 ) code="#player";
    int idx=202;
    StrVar* sv=xmap.gv(code);
    WebView* w=NULL;
    if( SVCHK('W',idx) ) {
        w=qobject_cast<WebView*>(SVQ);
    } else {
        w=new WebView(wConf(code,"tray",idx,wcf), wcf && wcf->xwidget ? wcf->xwidget: parentWidget);
        xmap.GetVar(code)->setVar('W',idx,(LPVOID)w);
    }
    return w;
}
#endif

QProgressDialog* WidgetMap::progressDialog(LPCC label, LPCC title) {
    LPCC code="@progress";
    QProgressDialog* w=NULL;
    StrVar* sv=xmap.gv(code);
    if( SVCHK('W',202) ) {
        w=qobject_cast<QProgressDialog*>(SVQ);
    } else {
        w=new QProgressDialog(parentWidget);
        xmap.GetVar(code)->setVar('W',202,(LPVOID)w);
    }
    if( slen(label) ) {
        w->setLabelText(KR(label));
    }
    if( slen(title) ) {
        w->setWindowTitle(KR(title));
    }
    return w;
}

QColorDialog* WidgetMap::colors() {
    LPCC code="@colors";
    StrVar* sv=xmap.gv(code);
    QColorDialog* w=NULL;
    if( SVCHK('W',202) ) {
        w=qobject_cast<QColorDialog*>(SVQ);
    } else {
        w=new QColorDialog(parentWidget);
        w->setWindowFlags(Qt::Widget);
        w->setOptions(
            QColorDialog::DontUseNativeDialog |
            QColorDialog::NoButtons
        );
        xmap.GetVar(code)->setVar('W',202,(LPVOID)w);
    }
    return w;
}

WidgetConf* WidgetMap::wConf(LPCC code, LPCC tag, U16 kind, WidgetConf* wcf)
{
    WidgetConf* conf=NULL;
    if( wcf ) {
        conf=wcf;
        xcf.GetVar(code)->setVar('n',1,(LPVOID)conf);
    } else {
        StrVar* sv=xcf.gv(code);
        if( SVCHK('n',1) ) {
            conf=(WidgetConf*)SVO;
        } else {
            conf=new WidgetConf();
            xcf.xobjects.add()->setVar('n',1,(LPVOID)conf);
            xcf.GetVar(code)->setVar('n',1,(LPVOID)conf);
        }
        conf->set("id", code);
        conf->set("tag",tag);
    }
    conf->xtype=kind;
    conf->xwidgetMap=this;
    conf->xwidget=NULL;
    return conf;
}

/*
 [open & save file dialog]
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Address Book"), "",
        tr("Address Book (*.abk);;All Files (*)"));
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Address Book"), "",
        tr("Address Book (*.abk);;All Files (*)"));
*/

MyAction::MyAction(WidgetConf *cf, QObject *p) : QAction(p), xcf(cf) {
    onAction=NULL;
    onChange=NULL;
    if( xcf ) xcf->xwidget=NULL;
    connect(this,SIGNAL(triggered()),this, SLOT(slotActionClick()));
    connect(this,SIGNAL(changed()), this, SLOT(slotChange()) );
}

MyAction::~MyAction() { }

void MyAction::slotActionClick()
{
    xcf->set("clicked", "action");
    if( onAction ) {
        onAction(xcf);
    }
}

void MyAction::slotChange()
{
    xcf->set("chaged", "action");
    if( onChange ) {
        onChange(xcf);
    }
}

MyLabel::MyLabel(WidgetConf *cf, QWidget *p) : QLabel(p), xcf(cf) {
    onContext=NULL;
    onLinkClick=NULL;
    onLinkHover=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this,SIGNAL(linkActivated(QString)),this,SLOT(slotLinkActivated(QString)) );
    connect(this,SIGNAL(linkHovered(QString)),this,SLOT(slotLinkHovered(QString)) );
}

MyLabel::~MyLabel() { }
void MyLabel::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL);}
void MyLabel::slotLinkActivated(QString link)
{
    if( onLinkClick ) {
        xcf->set("link", Q2A(link) );
        onLinkClick(xcf);
    }
}

void MyLabel::slotLinkHovered(QString link)
{
    if( onLinkHover ) {
        xcf->set("link", Q2A(link) );
        onLinkHover(xcf);
    }
}

bool MyLabel::event(QEvent *evt) {
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return onContext(xcf, e->pos().x(), e->pos().y()  );
        }
    } break;
    }
    return QLabel::event(evt);
}

MyProgressBar::MyProgressBar(WidgetConf *cf, QWidget *p) : QProgressBar(p), xcf(cf) {
    setRange(0, 100);
    setValue(0);
    onChange=NULL;
    onTimeOut=NULL;
    connect( this, SIGNAL(valueChanged(int)), SLOT(slotChange(int)));
    if( xcf ) xcf->xwidget=this;
}

MyProgressBar::~MyProgressBar() { }
void MyProgressBar::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL);}
void MyProgressBar::slotChange(int val)
{
    xcf->set("chaged", "value");
    if( onChange ) {
        xcf->GetVar("value")->setVar('0',0).addInt(val);
        onChange(xcf);
    }
}

MyComboBox::MyComboBox(WidgetConf *cf, QWidget *p) : QComboBox(p), xcf(cf) {
    onChange=NULL;
    onAcitvated=NULL;
    onMouseDown=NULL;
    onKeyDown=NULL;
    onWheel=NULL;
    onPopup=NULL;
    onTimeOut=NULL;

    onChildData=NULL;
    onToolTip=NULL;
    onLessThan=NULL;
    onFilterRow=NULL;
    onRowSize=NULL;
    onDraw=NULL;
    onChangeRow=NULL;
    onClickRow=NULL;
    onTimeOut=NULL;
    onClick=NULL;
    onDoubleClick=NULL;
    if( xcf ) xcf->xwidget=this;
    installEventFilter(this);
    view()->viewport()->installEventFilter(this);
    // lineEdit()->installEventFilter(this);
    connect( this, SIGNAL(activated(int)), this, SLOT(slotActivated(int)) );
    connect( this, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChange(int)) );
}

MyComboBox::~MyComboBox()
{
    if( config()->getBool("@baseCreate") ) {
        SAFE_DELETE(xbaseModel);
    }
    SAFE_DELETE(xproxyModel);
}
void MyComboBox::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyComboBox::initModel(MyTreeModel *baseModel) {
    bool bset=false;
    if( baseModel ) {
        if( xbaseModel ) {
            if( xbaseModel!=baseModel ) {
                xbaseModel=baseModel;
                bset=true;
            }
        } else {
            xbaseModel=baseModel;
            bset=true;
        }
    } else if(xbaseModel==NULL ) {
        setTreeModelHeader(xcf);
        config()->setBool("@baseCreate", true);
        bset=true;
        xbaseModel=new MyTreeModel(xcf, NULL, this);
    }
    if( bset && xproxyModel ) {
        xproxyModel->m_sourceModel=xbaseModel;
    }
    if( xproxyModel==NULL ) {
        xproxyModel=new MyTreeProxyModel(xbaseModel, this);
        setModel(xproxyModel);
        setItemDelegate(new MyTreeDelegate(xproxyModel, this) );
        connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)) );
        connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDoubleClicked(QModelIndex)) );
    }
    update();
}

void MyComboBox::update()
{
    if( xbaseModel && xproxyModel ) {
        xbaseModel->onChildData=onChildData;
        xproxyModel->onDraw=onDraw;
        xproxyModel->onFilterRow=onFilterRow;
        xproxyModel->onLessThan=onLessThan;
        xproxyModel->onRowSize=onRowSize;
        xproxyModel->onToolTip=onToolTip;
        xproxyModel->invalidate();
    }
}

void MyComboBox::slotDoubleClicked(const QModelIndex & index) {
    if( xproxyModel && onDoubleClick) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onDoubleClick( xcf, cur );
    }
}
void MyComboBox::slotClicked ( const QModelIndex & index ) {
    if( xproxyModel && onClick ) {
        QModelIndex curIndex = xproxyModel->mapToSource(index);
        TreeNode* cur= curIndex.isValid() ? (TreeNode*)curIndex.internalPointer(): NULL;
        onClick( xcf, cur );
    }
}
void MyComboBox::slotActivated(int index)
{
    if( onAcitvated ) {
        xcf->GetVar("index")->setVar('0',0).addInt(index);
        onAcitvated(xcf);
    }
}
void MyComboBox::slotIndexChange(int index)
{
    xcf->set("chaged", "index");
    if( onChange ) {
        xcf->GetVar("index")->setVar('0',0).addInt(index);
        onChange(xcf);
    }
}

bool MyComboBox::eventFilter(QObject *object, QEvent *evt)
{
    switch( evt->type() ) {
    case QEvent::KeyPress: {
        if( onKeyDown ) {
            LPCC ty=object==lineEdit()?"edit": object==view()->viewport()?"popup": "this";
            xcf->set("@sender", ty );
            if( onKeyDown(xcf, static_cast<QKeyEvent*>(evt) ) ) {
                return true;
            }
        }
    } break;
    case QEvent::MouseButtonPress: {
        if( onMouseDown ) {
            LPCC ty=object==lineEdit()?"edit": object==view()->viewport()?"popup": "this";
            xcf->set("@sender", ty );
        }
    } break;
    case QEvent::MouseButtonRelease: {
        LPCC ty=object==lineEdit()?"edit": object==view()->viewport()?"popup": "this";
        if( onMouseDown && xcf->cmp("@sender",ty) ) {
            if( onMouseDown(xcf, static_cast<QMouseEvent*>(evt)) ) {
                return true;
            }
        }
        if( isEditable() && object==lineEdit() ) {
            if( !lineEdit()->hasSelectedText() ) {
                lineEdit()->selectAll();
                return true;
            }
        }
    } break;
    }
    return false;
}

void MyComboBox::showPopup() {
    QComboBox::showPopup();
    if( onPopup ) {
        onPopup(xcf);
    }
}

MyTabWidget::MyTabWidget(WidgetConf *cf, QWidget *p) : QTabWidget(p), xcf(cf) {
    onChange=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(currentChanged(int)), SLOT(slotTabChange(int)) );
}

MyTabWidget::~MyTabWidget() { }

void MyTabWidget::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyTabWidget::slotTabChange(int index)
{
    xcf->set("chaged", "index");
    if( onChange ) {
        xcf->GetVar("index")->setVar('0',0).addInt(index);
        onChange(xcf);
    }
}

MySplitter::MySplitter(WidgetConf *cf, QWidget *p) : QSplitter(p), xcf(cf) {
    onSplitterMoved=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(splitterMoved(int,int)), SLOT(slotSplitterMove(int,int)) );
}

MySplitter::~MySplitter() { }
void MySplitter::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MySplitter::slotSplitterMove(int w, int h)
{
    if( onSplitterMoved ) {
        onSplitterMoved(xcf, w, h);
    }
}

MyStackedWidget::MyStackedWidget(WidgetConf *cf, QWidget *p) : QStackedWidget(p), xcf(cf) {
    onChange=NULL;
    onRemove=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotDivChange(int)) );
    connect(this, SIGNAL(widgetRemoved(int)), this, SLOT(slotPageRemove(int)) );
}

MyStackedWidget::~MyStackedWidget() { }

void MyStackedWidget::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyStackedWidget::slotDivChange(int index)
{
    xcf->set("chaged", "index");
    if( onChange ) {
        xcf->GetVar("index")->setVar('0',0).addInt(index);
        onChange(xcf);
    }
}
void MyStackedWidget::slotPageRemove(int index)
{
    if( onRemove ) {
        xcf->GetVar("index")->setVar('0',0).addInt(index);
        onRemove(xcf);
    }
}
MyToolButton::MyToolButton(WidgetConf *cf, QWidget *p) : QToolButton(p), xcf(cf) {
    onContext=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
}

MyToolButton::~MyToolButton() { }

void MyToolButton::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyToolButton::slotClicked(bool push)
{
    xcf->set("clicked", "button");
    if( onClick ) {
        xcf->GetVar("state")->set(isChecked()?"check": "" );
        onClick(xcf);
    }
}

bool MyToolButton::event(QEvent *evt) {
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return onContext(xcf, e->pos().x(), e->pos().y()  );
        }
    } break;
    }
    return QToolButton::event(evt);
}

MyCheckBox::MyCheckBox(WidgetConf *cf, QWidget *p) : QCheckBox(p), xcf(cf) {
    onClick=NULL;
    onChange=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(slotStateChange(int)) );
}

MyCheckBox::~MyCheckBox() { }
void MyCheckBox::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyCheckBox::slotClicked(bool check)
{
    xcf->set("clicked", "check");
    if( onClick ) {
        xcf->GetVar("state")->set(isChecked()?"check": "" );
        onClick(xcf);
    }
}

void MyCheckBox::slotStateChange(int state)
{
    xcf->set("chaged", "state");
    if( onChange ) {
        onChange(xcf);
    }
}

MyRadioButton::MyRadioButton(WidgetConf *cf, QWidget *p) : QRadioButton(p), xcf(cf) {
    onClick=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(clicked(bool)), this, SLOT(slotClicked(bool)) );
}

MyRadioButton::~MyRadioButton() { }
void MyRadioButton::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyRadioButton::slotClicked(bool check)
{
    xcf->set("clicked", "check");
    if( onClick ) {
        xcf->GetVar("state")->set(isChecked()?"check": "" );
        onClick(xcf);
    }
}

MySpinBox::MySpinBox(WidgetConf *cf, QWidget *p) : QSpinBox(p), xcf(cf) {
    onChange=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(valueChanged(int)), this, SLOT(slotValueChange(int)) );
}

MySpinBox::~MySpinBox() { }
void MySpinBox::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MySpinBox::slotValueChange(int value)
{
    xcf->set("chaged", "value");
    if( onChange ) {
        xcf->GetVar("value")->setVar('0',0).addInt(value);
        onChange(xcf);
    }
}

MyDateEdit::MyDateEdit(WidgetConf *cf, QWidget *p) : QDateEdit(p), xcf(cf) {
    onContext=NULL;
    onChange=NULL;
    onMouseDown=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    installEventFilter(this);
    lineEdit()->installEventFilter(this);
    connect( this, SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(slotDateChage()) );
}

MyDateEdit::~MyDateEdit() { }
void MyDateEdit::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyDateEdit::slotDateChage()
{
    xcf->set("chaged", "date");
    if( onChange ) {
        onChange(xcf);
    }
}


bool MyDateEdit::eventFilter(QObject *object, QEvent *evt)
{
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           xcf->set("@sender", object==lineEdit()?"edit":"this" );
           if( onContext(xcf, e->pos().x(), e->pos().y()) ) return true;
       }
   } break;
   case QEvent::MouseButtonPress: {
       if( onMouseDown ) {
           xcf->set("@sender", object==lineEdit()?"edit":"this" );
       }
   } break;
   case QEvent::MouseButtonRelease: {
       if( onMouseDown ) {
           LPCC ty=object==lineEdit()?"edit":"this";
           if( xcf->cmp("@sender",ty) ) {
              if( onMouseDown(xcf, static_cast<QMouseEvent*>(evt)) ) return true;
           }
       }
   } break;
   }
   return false;
}

MyTimeEdit::MyTimeEdit(WidgetConf *cf, QWidget *p) : QTimeEdit(p), xcf(cf) {
    onContext=NULL;
    onChange=NULL;
    onMouseDown=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(timeChanged(QTime)), this, SLOT(slotTimeChange()) );
}

MyTimeEdit::~MyTimeEdit() { }
void MyTimeEdit::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyTimeEdit::slotTimeChange()
{
    xcf->set("chaged", "time");
    if( onChange ) {
        onChange(xcf);
    }

}

bool MyTimeEdit::eventFilter(QObject *object, QEvent *evt)
{
    switch( evt->type() ) {
   case QEvent::ContextMenu: {
       if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           xcf->set("@sender", object==lineEdit()?"edit":"this" );
           return onContext(xcf, e->pos().x(), e->pos().y()  );
       }
   } break;
   case QEvent::MouseButtonPress: {
       if( onMouseDown ) {
           xcf->set("@sender", object==lineEdit()?"edit":"this" );
       }
   } break;
   case QEvent::MouseButtonRelease: {
       if( onMouseDown ) {
           LPCC ty=object==lineEdit()?"edit":"this";
           if( xcf->cmp("@sender",ty) ) {
              return onMouseDown(xcf, static_cast<QMouseEvent*>(evt) );
           }
       }
   } break;
   }
   return false;
}

MyCalendarWidget::MyCalendarWidget(WidgetConf *cf, QWidget *p) : QCalendarWidget(p), xcf(cf) {
    onContext=NULL;
    onChange=NULL;
    onClick=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(clicked(QDate)), this, SLOT(slotClicked()) );
    connect( this, SIGNAL(currentPageChanged(int,int)), this, SLOT(slotPageChange(int, int)) );
    connect( this, SIGNAL(selectionChanged()), this, SLOT(slotChange()) );
}

MyCalendarWidget::~MyCalendarWidget() { }
void MyCalendarWidget::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyCalendarWidget::slotChange()
{
    xcf->set("chaged", "date");
    if( onChange ) {
        onChange(xcf);
    }
}

void MyCalendarWidget::slotPageChange(int year, int month)
{
    xcf->set("chaged", "page");
    xcf->GetVar("year")->setVar('0',0).addInt(year);
    xcf->GetVar("month")->setVar('0',0).addInt(month);
    if( onChange ) {
        onChange(xcf);
    }
}

void MyCalendarWidget::slotClicked()
{
    xcf->set("clicked", "calendar");
    if( onClick ) {
        onClick(xcf);
    }
}

bool MyCalendarWidget::event(QEvent *evt)
{
   switch( evt->type() ) {
   case QEvent::ContextMenu: {
       if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           return onContext(xcf, e->pos().x(), e->pos().y()  );
       }
   } break;
   }
   return QCalendarWidget::event(evt);
}

MyToolBar::MyToolBar(WidgetConf *cf, QWidget *p) : QToolBar(p), xcf(cf) {
    onChange=NULL;
    onContext=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(movableChanged(bool)), this, SLOT(slotChange()) );
    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(slotChange()) );
}

MyToolBar::~MyToolBar() { }
void MyToolBar::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyToolBar::slotChange()
{
    xcf->set("chaged", "toolbar");
    if( onChange ) {
        onChange(xcf);
    }
}

bool MyToolBar::event(QEvent *evt)
{
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           return onContext(xcf, e->pos().x(), e->pos().y() );
       }
   } break;
   }
   return QToolBar::event(evt);
}

MyToolBox::MyToolBox(WidgetConf *cf, QWidget *p) : QToolBox(p), xcf(cf) {
    onContext=NULL;
    onChange=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(slotChange()) );
}

MyToolBox::~MyToolBox() { }
void MyToolBox::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyToolBox::slotChange()
{
    xcf->set("chaged", "toolbox");
    if( onChange ) {
        onChange(xcf);
    }

}

bool MyToolBox::event(QEvent *evt)
{
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
       if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           return onContext(xcf, e->pos().x(), e->pos().y()  );
       }
   } break;
   }
   return QToolBox::event(evt);
}

MyDockWidget::MyDockWidget(WidgetConf *cf, QWidget *p) : QDockWidget(p), xcf(cf) {
    onContext=NULL;
    onChange=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(slotChange()) );
}

MyDockWidget::~MyDockWidget() {

}
void MyDockWidget::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MyDockWidget::slotChange()
{
    xcf->set("chaged", "dock");
    if( onChange ) {
        onChange(xcf);
    }
}

bool MyDockWidget::event(QEvent *evt)
{
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
           QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
           return onContext(xcf, e->pos().x(), e->pos().y()  );
       }
   } break;
   }
   return QDockWidget::event(evt);
}


MySystemTrayIcon::MySystemTrayIcon(WidgetConf *cf, QWidget *p) : QSystemTrayIcon(p), xcf(cf) {
    onActivated=NULL;
    onClick=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=NULL;
    connect(this,SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
            SLOT(slotActivated(QSystemTrayIcon::ActivationReason)) );
    connect(this,SIGNAL(messageClicked()), this, SLOT(slotClicked()) );
}

MySystemTrayIcon::~MySystemTrayIcon() { }
void MySystemTrayIcon::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
void MySystemTrayIcon::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
    xcf->set("reason",
        reason==QSystemTrayIcon::Unknown ? "unknown" :
        reason==QSystemTrayIcon::Context ? "context" :
        reason==QSystemTrayIcon::DoubleClick ? "doubleclick" : "trigger"
    );
    if( onActivated ) {
        onActivated(xcf);
    }
}

void MySystemTrayIcon::slotClicked()
{
    xcf->set("clicked", "message");
    if( onClick ) {
        onClick(xcf);
    }
}

MyLineEdit::MyLineEdit(WidgetConf *cf, QWidget *p) : QLineEdit(p), xcf(cf) {
    onChange=NULL;
    onContext=NULL;
    onKeyDown=NULL;
    onTimeOut=NULL;
    if( xcf ) xcf->xwidget=this;
    connect( this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChange()) );
}

MyLineEdit::~MyLineEdit() { }
void MyLineEdit::slotTimeOut() { if( onTimeOut ) onTimeOut(xcf,NULL); }
bool MyLineEdit::event(QEvent *evt) {
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return onContext(xcf, e->pos().x(), e->pos().y()  );
        }
    } break;
    case QEvent::KeyPress: {
        if( onKeyDown ) {
            return onKeyDown(xcf, static_cast<QKeyEvent*>(evt) );
        }
    } break;
    }
    return QLineEdit::event(evt);
}

void MyLineEdit::slotTextChange()
{
    xcf->set("chaged", "input");
    if( onChange ) {
        onChange(xcf);
    }
}



MyGroupBox::MyGroupBox(WidgetConf *cf, QWidget *p) : QGroupBox(p), xcf(cf) {
    onContext=NULL;
    if( xcf ) xcf->xwidget=this;
}

MyGroupBox::~MyGroupBox() { }

bool MyGroupBox::event(QEvent *evt) {
    switch( evt->type() ) {
    case QEvent::ContextMenu: {
        if( onContext ) {
            QContextMenuEvent* e=static_cast<QContextMenuEvent*>(evt);
            return onContext(xcf, e->pos().x(), e->pos().y()  );
        }
    } break;
    }
    return QGroupBox::event(evt);
}

MyGraphicsView::MyGraphicsView(WidgetConf *cf, QWidget *p) : QGraphicsView(p), xcf(cf) {
    if( xcf ) xcf->xwidget=this;
}

MyGraphicsView::~MyGraphicsView() { }

MyPlainTextEdit::MyPlainTextEdit(WidgetConf *cf, QWidget *p) : QPlainTextEdit(p), xcf(cf) {
    if( xcf ) xcf->xwidget=this;
}

MyPlainTextEdit::~MyPlainTextEdit() { }

#endif
