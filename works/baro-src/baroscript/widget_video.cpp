#include "func_util.h"
#include "widget_video.h"
#include <QApplication>
#include <QDesktopWidget>

GVideo::GVideo(TreeNode* cf, QWidget *parent ) : QLabel(parent), xcf(cf) {
    setMouseTracking(TRUE);
    setFocusPolicy( Qt::NoFocus );
    setAutoFillBackground(TRUE);
    xcf->xstat=WTYPE_VIDEO;
    xcanvasConf.xstat=WTYPE_CANVAS;
    xcanvasConf.GetVar("video")->setVar('n',0,(LPVOID)xcf);
    xcanvas=new GCanvas(&xcanvasConf, this );
    xcanvas->setStyleSheet("QLabel{background: transparent;}");  // background-color: rgba(0,0,0,0)
    xcanvas->setAttribute(Qt::WA_TranslucentBackground, true);
    // xcanvas->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    xcanvasConf.GetVar("@this")->setVar('w',0,(LPVOID)xcanvas);
    xcanvasConf.setNodeFlag(FLAG_SETEVENT|FLAG_DRAW);
    xcanvas->setStyleSheet("QLabel{background: transparent;}");  // background-color: rgba(0,0,0,0)
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::transparent);
    xcanvas->setPalette(p);
    /*
    U32 flags = xcanvas->windowFlags();
    flags&=~Qt::WindowTitleHint;
    flags|=Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint;
    */
    xcanvas->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    xcanvas->show();
    /*
    xcanvas->setAttribute( Qt::WA_OpaquePaintEvent, false );
    xcanvas->setAttribute(Qt::WA_NoSystemBackground, true);
    xcanvas->setAttribute(Qt::WA_TranslucentBackground, true);
    xcanvas->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    xcanvas->setAttribute( Qt::WA_OpaquePaintEvent, false );
    xcanvas->setAttribute(Qt::WA_PaintOnScreen);
    ## for linux
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    */

    // setMinimumSize(320,240); // setMinimumSize( QSize(0,0) );
    xcursor = QPoint(0,0);
    xcursorLast = QPoint(0,0);

    QTimer *timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(checkMousePos()) );
    timer->start(2000);

    m_seekTimer.setInterval(50);
    m_seekTimer.setSingleShot(true);
    connect(&m_seekTimer, SIGNAL(timeout()), this, SLOT(delayedSeek()));

    m_process = new GMpProcess(cf, this);
    connect(m_process, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    connect(m_process, SIGNAL(streamPositionChanged(double)), this, SLOT(onStreamPositionChanged(double)));
    connect(m_process, SIGNAL(error(const QString &)), this, SLOT(onError(const QString &)));
    connect(m_process, SIGNAL(readStandardOutput(const QString &)), this, SLOT(onReadOut(const QString &)));
    connect(m_process, SIGNAL(readStandardError(const QString &)), this, SLOT(onError(const QString &)));
}

GVideo::~GVideo() {
    if( m_process->processState() == QProcess::Running) {
        m_process->quit();
    }
    delete m_process;
}

void GVideo::checkMousePos() {
    if ( xcursor == xcursorLast ) {
        if (cursor().shape() != Qt::BlankCursor) {
            //qDebug(" hiding mouse cursor");
            setCursor(QCursor(Qt::BlankCursor));
        }
    } else {
        xcursorLast = xcursor;
    }
}

void GVideo::mouseMoveEvent( QMouseEvent * e ) {
    xcursor = e->pos();
    if (cursor().shape() != Qt::ArrowCursor) {
        setCursor(QCursor(Qt::ArrowCursor));
    }
}
void GVideo::onReadOut(const QString &line) {
    XFuncNode* fn = getEventFuncNode(config(), "onReadOut");
    if( fn ) {
        PARR arr=getLocalParams(fn);
        arr->add()->set(Q2A(line));
        setFuncNodeParams(fn,arr);
        fn->call();
    }
}
void GVideo::onError(const QString &err) {
    XFuncNode* fn = getEventFuncNode(config(), "onError");
    if( fn ) {
        PARR arr=getLocalParams(fn);
        arr->add()->set(Q2A(err));
        setFuncNodeParams(fn,arr);
        fn->call();
    }
}

void GVideo::paintEvent( QPaintEvent * evt ) {
    Q_UNUSED(evt);
    StrVar* sv=xcanvasConf.gv("onDraw");
    if( sv==NULL ) {
        XFuncNode* fnDraw = getEventFuncNode(config(), "onDrawCanvas");
        if( fnDraw ) {
            xcanvasConf.GetVar("onDraw")->setVar('f',0,(LPVOID)fnDraw);
        }
    }
    XFuncNode* fn = getEventFuncNode(config(), "onDraw");
    QPainter p(this);
    if( fn ) {
        TreeNode* d=NULL;
        sv=fn->GetVar("@draw");
        if( SVCHK('n',0) ) {
            d=(TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            d->xstat=FNSTATE_DRAW;
            d->xtype=0;
            sv->setVar('n',0,(LPVOID)d);
        }
        setVarRectF(d->GetVar("@rect"), evt->rect() );
        d->GetVar("@g")->setVar('g',0,(LPVOID)&p);
        PARR arr=getLocalParams(fn);
        arr->add()->setVar('n',0,(LPVOID)d);
        setFuncNodeParams(fn,arr);
        fn->call();
    } else {
        if( xcanvas ) {
            if( xcanvas->isHidden() ) xcanvas->show();
            xcanvas->update();
        }
        p.setCompositionMode(QPainter::CompositionMode_Source);
        GMpWidget::State stat=state();
        if( stat==GMpWidget::IdleState || stat==GMpWidget::StoppedState ) {
            p.fillRect(rect(), Qt::black);
        }
    }
    p.end();
}
QSize GVideo::sizeHint() const {
    if( m_process->m_mediaInfo.ok && !m_process->m_mediaInfo.size.isNull()) {
        return m_process->m_mediaInfo.size;
    }
    return QWidget::sizeHint();
}

void GVideo::start(const QStringList &args) {
    if( m_process->processState() == QProcess::Running) {
        m_process->quit();
    }
    m_process->start(qobject_cast<QWidget*>(this), args);
}

void GVideo::openViedo(const QString &url) {
    if(m_process->state() == QProcess::NotRunning ) {
        qDebug()<< "#9##MPlayer process not started yet URL: "<<url<<"\n";
        return;
    }
    writeCommand("pausing_keep_force pt_step 1");
    writeCommand("get_property pause");
    writeCommand(QString("loadfile '%1'").arg(url));
}

void GVideo::play() {
    if( state() == GMpWidget::PausedState ) {
        m_process->pause();
    }
}

void GVideo::pause() {
    if (state() == GMpWidget::PlayingState) {
        m_process->pause();
    }
}
void GVideo::stop() {
    m_process->stop();
}

void GVideo::toggleFullScreen() {
    if( !isFullScreen() ) {
        int num = QApplication::desktop()->screenNumber(QCursor::pos());
        setGeometry(QApplication::desktop()->screenGeometry(num));
        m_geometry = geometry();
        setWindowFlags((windowFlags() | Qt::Window));
#ifdef Q_WS_X11
        show();
        raise();
        setWindowState(windowState() | Qt::WindowFullScreen);
#else
        setWindowState(windowState() | Qt::WindowFullScreen);
        show();
#endif
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        setGeometry(m_geometry);
        show();
    }
}

void GVideo::writeCommand(const QString &command) {
    qDebug("video write command==%s", Q2A(command) );

    if( command=="quit" ) {
        if( m_process->processState() == QProcess::Running ) {
            m_process->quit();
        }
    } else if( command=="process" ) {
        if( m_process->processState() == QProcess::Running) {
            xcf->GetVar("@state")->set("running");
        } else {
            xcf->GetVar("@state")->reuse();
        }
    } else if( command=="state" ) {
        if( state() == GMpWidget::PausedState ) {
            xcf->GetVar("@state")->set("pause");
        } else if( state() == GMpWidget::PlayingState ) {
            xcf->GetVar("@state")->set("play");
        } else if( state() == GMpWidget::StoppedState ) {
            xcf->GetVar("@state")->set("stop");
            update();
        } else if( state() == GMpWidget::NotStartedState ) {
            xcf->GetVar("@state")->set("notStart");
        } else if( state() == GMpWidget::IdleState ) {
            xcf->GetVar("@state")->set("idle");
        } else if( state() == GMpWidget::LoadingState ) {
            xcf->GetVar("@state")->set("loading");
            update();
        } else if( state() == GMpWidget::BufferingState ) {
            xcf->GetVar("@state")->set("buffering");
            update();
        } else if( state() == GMpWidget::ErrorState ) {
            xcf->GetVar("@state")->set("error");
        } else {
            xcf->GetVar("@state")->reuse();
        }
    } else {
        m_process->writeCommand(command);
    }
}

void GVideo::resizeEvent(QResizeEvent *event) {
    // Q_UNUSED(event);
    QSize mediaSize = m_process->m_mediaInfo.size;
    if( xcf->isTrue("@resizeCheck") && !mediaSize.isNull() ) {
        QSize widgetSize = size();
        double factor = qMin(double(widgetSize.width()) / mediaSize.width(), double(widgetSize.height()) / mediaSize.height());
        QRect rc(0, 0, int(factor * mediaSize.width() + 0.5), int(factor * mediaSize.height()));
        rc.moveTopLeft(rect().center() - rc.center());
        setGeometry(rc);
    }
	/*
    if( xcanvas ) {
        QRect rc=rect();
        QPoint pt=mapToGlobal(pos());
        xcanvas->setGeometry(rc);
        // xcanvas->move(pt.x(),pt.y());
        // xcanvas->raise();
        qDebug()<< "video resize event: "<< rc<< ", point:"<<pt;
    }
	*/
}
void GVideo::seek(double offset, int whence) {
    m_seekTimer.stop();
    m_seekCommand = QString("seek %1 %2").arg(offset).arg(whence);
    m_seekTimer.start();
}
void GVideo::delayCommand(LPCC cmd) {
    if( slen(cmd)==0 ) return;
    m_seekTimer.stop();
    m_seekCommand = KR(cmd);
    m_seekTimer.start();
}
void GVideo::delayedSeek() {
    if( !m_seekCommand.isEmpty() ) {
        writeCommand(m_seekCommand);
        m_seekCommand = QString();
    }
}

void GVideo::setVolume(int volume) {
    writeCommand(QString("volume %1 1").arg(volume));
    onVolumeChanged(volume);
}

void GVideo::onStateChanged(int state) {
    update();
    XFuncNode* fn = getEventFuncNode(xcf, "onStateChange");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        StrVar* sv=arrs->add();
        if( state==GMpWidget::StoppedState ) sv->set("stop");
        else if( state==GMpWidget::IdleState ) sv->set("idle");
        else if( state==GMpWidget::LoadingState ) sv->set("loading");
        else if( state==GMpWidget::BufferingState ) sv->set("buffering");
        else if( state==GMpWidget::PausedState ) sv->set("paused");
        else if( state==GMpWidget::PlayingState ) sv->set("play");
        else if( state==GMpWidget::StoppedState ) sv->set("stop");
        else if( state==GMpWidget::NotStartedState ) sv->set("notStarted");
        else if( state==GMpWidget::ErrorState ) sv->set("error");
        else sv->set("etc");
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void GVideo::onStreamPositionChanged(double position) {
    XFuncNode* fn = getEventFuncNode(config(), "onPositionChange");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('4',0).addDouble(position);
        setFuncNodeParams(fn, arrs);
        fn->call();
        xcf->GetVar("@seekPosition")->setVar('4',0).addDouble(position);
    }
}

void GVideo::onVolumeChanged(int volume) {
    XFuncNode* fn = getEventFuncNode(config(), "onVolumChange");
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->setVar('0',0).addInt(volume);
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

