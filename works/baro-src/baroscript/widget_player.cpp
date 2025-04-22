#include "widget_player.h"
#include "func_util.h"
#include <QApplication>
#include <QDesktopWidget>

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MpWidget
//
//////////////////////////////////////////////////////////////////////////////////////////////////

GMpProcess::GMpProcess(TreeNode* cf, QObject *parent ) :
    QProcess(parent), m_state(GMpWidget::NotStartedState), m_mplayerPath("mplayer"), m_fakeInputconf(NULL), xcf(cf)
{
    resetValues();

#ifdef Q_WS_WIN
    m_mode = GMpWidget::EmbeddedMode;
    m_videoOutput = "direct3d"; // directx,directx:noaccel";
#elif defined(Q_WS_X11)
    m_mode = GMpWidget::EmbeddedMode;
    m_videoOutput = "xv";

#elif defined(Q_WS_MAC)
    m_mode = GMpWidget::PipeMode;
    m_videoOutput = "quartz";
#endif

    m_movieFinishedTimer.setSingleShot(true);
    m_movieFinishedTimer.setInterval(100);

    connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdout()));
    connect(this, SIGNAL(readyReadStandardError()), this, SLOT(readStderr()));
    connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished()));
    connect(&m_movieFinishedTimer, SIGNAL(timeout()), this, SLOT(movieFinished()));
}

GMpProcess::~GMpProcess()
{
    if (m_fakeInputconf != NULL) {
        delete m_fakeInputconf;
    }
}

// Starts the MPlayer process in idle mode
void GMpProcess::start(QWidget *widget, const QStringList &args) {
    if (m_mode == GMpWidget::PipeMode) {
        m_mode = GMpWidget::EmbeddedMode;
    }

    // Figure out the mplayer version in order to check if
    // "-input nodefault-bindings" is available
#ifdef WINDOW_USE
    QStringList myargs;
    myargs += "-slave";
    if( args.size()==0 ) {
        if( !m_videoOutput.isEmpty()) {
            myargs += "-vo";
            myargs += m_videoOutput;
        } else {
            myargs += "-vo";
            myargs += "direct3d";
        }
        myargs += "-slave";
        myargs += "-idle";
        myargs += "-vf";
        myargs += "scale";
        myargs += "-zoom";
        myargs += "-xy";
        myargs += "600";
        myargs += "-noquiet";
        myargs += "-identify";
        myargs += "-nodouble";
        myargs += "-nomouseinput";
        myargs += "-nokeepaspect";
        myargs += "-monitorpixelaspect";
        myargs += "1";

        bool useFakeInputconf = true;
        QString version = mplayerVersion();
        if (version.contains("SVN")) { // Check revision
            QRegExp re("SVN-r([0-9]*)");
            if (re.indexIn(version) > -1) {
                int revision = re.cap(1).toInt();
                if (revision >= 28878) {
                    useFakeInputconf = false;
                }
            }
        }

        if( !useFakeInputconf) {
            myargs += "-input";
            myargs += "nodefault-bindings:conf=/dev/null";
        } else {
            // Ugly hack for older versions of mplayer (used in kmplayer and other)
            if( m_fakeInputconf == NULL) {
                m_fakeInputconf = new QTemporaryFile();
                if (m_fakeInputconf->open()) {
                    writeFakeInputconf(m_fakeInputconf);
                } else {
                    delete m_fakeInputconf;
                    m_fakeInputconf = NULL;
                }
            }
            if( m_fakeInputconf != NULL) {
                myargs += "-input";
                myargs += QString("conf=%1").arg(m_fakeInputconf->fileName());
            }
        }
    } else if( args.size()==1 ) {
        if( !m_videoOutput.isEmpty()) {
            myargs += "-vo";
            myargs += m_videoOutput;
        }
    }
    // Ugly hack for older versions of mplayer (used in kmplayer and other)

    myargs += "-wid";
    myargs += QString::number((long)widget->winId());
    if( args.size()>0 ) {
        myargs += args;
    }
    if( xcf ) {
        QString str=myargs.join(" ");
        xcf->set("@args", Q2A(str) );
    }
    //
    qDebug() <<"mplayer path:"<<m_mplayerPath<<" args:"<< myargs<<"\n";

    QProcess::setProcessChannelMode(QProcess::MergedChannels);
    QProcess::start(m_mplayerPath, myargs);
    changeState(GMpWidget::IdleState);
#else
    QStringList myargs;
    if( args.size()==2 ) {
        QString cmd=args[0];
        if( cmd=="bash" ) {
            QString prog=args[1];
            myargs+="-c";
            prog += " -wid ";
            prog += QString::number((long)widget->winId());
            prog += " -";
            myargs+= prog;
            if( xcf ) {
                QString str=myargs.join(" ");
                xcf->set("@args", Q2A(str) );
            }
            QProcess::setProcessChannelMode(QProcess::MergedChannels);
            QProcess::start("bash", myargs);
            changeState(GMpWidget::IdleState);
            // waitForFinished();
            return;
        }
    }
    myargs += "-slave";
    if( args.size()==0 ) {
        myargs += "-vo";
        myargs += "x11";
        myargs += "-vf";
        myargs += "scale";
        myargs += "-zoom";
        myargs += "-xy";
        myargs += "600";
        myargs += "-nomouseinput";
        myargs += "-nokeepaspect";
        myargs += "-nodouble";
        myargs += "-noquiet";

        /*
         * -vf scale -zoom -xy 600
         *
        myargs += "-idle";
        myargs += "-identify";
        myargs += "-nokeepaspect";
        myargs += "-nodouble";
        myargs += "-noquiet";
        myargs += "-monitorpixelaspect";
        myargs += "1";
        myargs += "-cache";
        myargs += "64";
        myargs += "-cache-min";
        myargs += "20";
        myargs += "-stop-xscreensaver";
        */

        bool useFakeInputconf = true;
        QString version = mplayerVersion();
        if (version.contains("SVN")) { // Check revision
            QRegExp re("SVN-r([0-9]*)");
            if (re.indexIn(version) > -1) {
                int revision = re.cap(1).toInt();
                if (revision >= 28878) {
                    useFakeInputconf = false;
                }
            }
        }

        if( !m_videoOutput.isEmpty()) {
        }
    }
    myargs += "-wid";
    myargs += QString::number((long)widget->winId());
    myargs += args;
    if( xcf ) {
        QString str=myargs.join(" ");
        xcf->set("@args", Q2A(str) );
    }
    //
#ifdef QMP_DEBUG_OUTPUT
    qDebug() <<"path:"<<m_mplayerPath<<" args:"<< myargs;
#endif
    QProcess::setProcessChannelMode(QProcess::MergedChannels);
    QProcess::start(m_mplayerPath, myargs);
    if( QProcess::waitForStarted(100) ) {
        return;
    }
    //retrieve basic information
    writeCommand("get_video_resolution" );
    writeCommand ("get_time_length" );

    changeState(GMpWidget::IdleState);
#endif

}

QString GMpProcess::mplayerVersion() {
    QProcess p;
    p.start(m_mplayerPath, QStringList("-version"));
    if (!p.waitForStarted()) {
        return QString();
    }
    if (!p.waitForFinished()) {
        return QString();
    }

    QString output = QString(p.readAll());
    QRegExp re("MPlayer ([^ ]*)");
    if (re.indexIn(output) > -1) {
        return re.cap(1);
    }
    return output;
}

QProcess::ProcessState GMpProcess::processState() const {
    return QProcess::state();
}

void GMpProcess::writeCommand(const QString &command) {
#ifdef QMP_DEBUG_OUTPUT
    qDebug("in: \"%s\"", qPrintable(command));
#endif
    QString line=command+"\n";
    QProcess::write(Q2A(line));
}

void GMpProcess::quit() {
    writeCommand("quit");
    QProcess::waitForFinished(100);
    if (QProcess::state() == QProcess::Running) {
        QProcess::kill();
    }
    QProcess::waitForFinished(-1);
}

void GMpProcess::pause()	{
    writeCommand("pause");
	if (QProcess::state() == GMpWidget::PlayingState ) {
		changeState(GMpWidget::PausedState);
	} else if(QProcess::state() == GMpWidget::PlayingState ) {
		changeState(GMpWidget::PlayingState);
	}
}

void GMpProcess::stop()		{
    writeCommand("stop");
    changeState(GMpWidget::StoppedState);
}

// slots:

void GMpProcess::readStdout() {
    QStringList lines = QString::fromLocal8Bit(readAllStandardOutput()).split("\n", QString::SkipEmptyParts);
    for (int i = 0; i < lines.count(); i++) {
        parseLine(lines[i].remove("\r") );
        emit readStandardOutput(lines[i]);
    }
}

void GMpProcess::readStderr() {
    QStringList lines = QString::fromLocal8Bit(readAllStandardError()).split("\n", QString::SkipEmptyParts);
    for (int i = 0; i < lines.count(); i++) {
        parseLine(lines[i].remove("\r"));
        emit readStandardError(lines[i]);
    }
}

void GMpProcess::finished() {
    // Called if the *process* has finished
    changeState(GMpWidget::NotStartedState);
}

void GMpProcess::movieFinished() {
    if (m_state == GMpWidget::PlayingState) {
        changeState(GMpWidget::IdleState);
    }
}

// private:
void GMpProcess::parseLine(const QString &line) {
    bool bok=true;
    if (line.startsWith("Playing ")) {
        changeState(GMpWidget::LoadingState);
    } else if (line.startsWith("Cache fill:")) {
        changeState(GMpWidget::BufferingState);
    } else if (line.startsWith("Starting playback...")) {
        m_mediaInfo.ok = true; // No more info here
        changeState(GMpWidget::PlayingState);
    } else if (line.startsWith("File not found: ")) {
        changeState(GMpWidget::ErrorState);
    } else if (line.endsWith("ID_PAUSED")) {
        changeState(GMpWidget::PausedState);
    } else if (line.startsWith("ID_")) {
        parseMediaInfo(line);
    } else if (line.startsWith("No stream found")) {
        changeState(GMpWidget::ErrorState, line);
    } else if (line.startsWith("A:") || line.startsWith("V:")) {
        if ( m_state != GMpWidget::PlayingState && m_state!=GMpWidget::StoppedState ) {
            changeState(GMpWidget::PlayingState);
        }
        bok=false;
        parsePosition(line);
    } else if (line.startsWith("Exiting...")) {
        changeState(GMpWidget::NotStartedState);
    }
    if( bok )  qDebug("out: \"%s\"", qPrintable(line));
    if( xcf ) {
        XFuncNode* fn = getEventFuncNode(xcf, "onMessage");
        if( fn ) {
			fn->GetVar("line")->set(Q2A(line));
            fn->call();
        }
    }
}

void GMpProcess::parseMediaInfo(const QString &line) {
    QStringList info = line.split("=");
    if (info.count() < 2) {
        return;
    }
    QString value=info[1];
    if (info[0] == "ID_VIDEO_FORMAT") {
        m_mediaInfo.videoFormat = value;
        xcf->set("@VideoFormat", Q2A(value) );
    } else if (info[0] == "ID_VIDEO_BITRATE") {
        m_mediaInfo.videoBitrate = value.toInt();
        xcf->set("@VideoBitRate", Q2A(value) );
    } else if (info[0] == "ID_VIDEO_WIDTH") {
        m_mediaInfo.size.setWidth(value.toInt());
        xcf->set("@VideoWidth", Q2A(value) );
    } else if (info[0] == "ID_VIDEO_HEIGHT") {
        m_mediaInfo.size.setHeight(value.toInt());
        xcf->set("@VideoHeight", Q2A(value) );
    } else if (info[0] == "ID_VIDEO_FPS") {
        m_mediaInfo.framesPerSecond = value.toDouble();
        xcf->set("@VideoFPS", Q2A(value) );
    } else if (info[0] == "ID_AUDIO_FORMAT") {
        m_mediaInfo.audioFormat = value;
        xcf->set("@AudioFormat", Q2A(value) );
    } else if (info[0] == "ID_AUDIO_BITRATE") {
        m_mediaInfo.audioBitrate = value.toInt();
        xcf->set("@AudioBitRate", Q2A(value) );
    } else if (info[0] == "ID_AUDIO_RATE") {
        m_mediaInfo.sampleRate = value.toInt();
        xcf->set("@AudioRate", Q2A(value) );
    } else if (info[0] == "ID_AUDIO_NCH") {
        m_mediaInfo.numChannels = value.toInt();
        xcf->set("@AudioNCH", Q2A(value) );

    } else if (info[0] == "ID_LENGTH") {
        m_mediaInfo.length = value.toDouble();
        xcf->set("@VideoLength", Q2A(value) );
    } else if (info[0] == "ID_SEEKABLE") {
        m_mediaInfo.seekable = (bool)value.toInt();
        xcf->set("@SeekAble", Q2A(value) );

    } else if (info[0].startsWith("ID_CLIP_INFO_NAME")) {
        m_currentTag = value;
        xcf->set("@VideoName", Q2A(value) );
    } else if (info[0].startsWith("ID_CLIP_INFO_VALUE") && !m_currentTag.isEmpty()) {
        m_mediaInfo.tags.insert(m_currentTag, value);
        xcf->set("@VideoValue", Q2A(value) );
    }
}

void GMpProcess::parsePosition(const QString &line) {
    static QRegExp rx("[ :]");
    QStringList info = line.split(rx, QString::SkipEmptyParts);

    double oldpos = m_streamPosition;
    for (int i = 0; i < info.count(); i++) {
        if ( (info[i] == "V" || info[i] == "A") && info.count() > i) {
            m_streamPosition = info[i+1].toDouble();

            // If the movie is near its end, start a timer that will check whether
            // the movie has really finished.
            if (qAbs(m_streamPosition - m_mediaInfo.length) < 1) {
                m_movieFinishedTimer.start();
            }
        }
    }

    if (oldpos != m_streamPosition) {
        emit streamPositionChanged(m_streamPosition);
    }
}

void GMpProcess::changeState(GMpWidget::State state, const QString &comment) {
    if (m_state == state) {
        return;
    }

    if (m_state == GMpWidget::PlayingState) {
        m_movieFinishedTimer.stop();
    }

    m_state = state;
    emit stateChanged(m_state);

    switch (m_state) {
        case GMpWidget::NotStartedState:
            resetValues();
            break;

        case GMpWidget::ErrorState:
            emit error(comment);
            resetValues();
            break;

        default: break;
    }
}

void GMpProcess::resetValues() {
    m_mediaInfo = GMpWidget::MediaInfo();
    m_streamPosition = -1;
}

void GMpProcess::writeFakeInputconf(QIODevice *device) {
    // Query list of supported keys
    QProcess p;
    p.start(m_mplayerPath, QStringList("-input") += "keylist");
    if (!p.waitForStarted()) {
        return;
    }
    if (!p.waitForFinished()) {
        return;
    }
    QStringList keys = QString(p.readAll()).split("\n", QString::SkipEmptyParts);

    // Write dummy command for each key
    QTextStream out(device);
    for (int i = 0; i < keys.count(); i++) {
        keys[i].remove("\r");
        out << keys[i] << " " << "ignored" << endl;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  MpWidget
//
//////////////////////////////////////////////////////////////////////////////////////////////////


GVideoLayer::GVideoLayer(GMpWidget *parent, Qt::WindowFlags flag ) : QWidget(parent, flag ), xplayer(parent) {
    setMouseTracking(TRUE);
    setAutoFillBackground(TRUE);
    setFocusPolicy( Qt::NoFocus );
    setMinimumSize(320,240); // setMinimumSize( QSize(0,0) );
    xcursor = QPoint(0,0);
    xcursorLast = QPoint(0,0);

    QTimer *timer = new QTimer(this);
    connect( timer, SIGNAL(timeout()), this, SLOT(checkMousePos()) );
    timer->start(2000);

    // Change attributes
    setAttribute(Qt::WA_NoSystemBackground);
    // setAttribute(Qt::WA_StaticContents);
    // setAttribute( Qt::WA_OpaquePaintEvent );
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_PaintUnclipped);
    // setAttribute(Qt::WA_PaintOutsidePaintEvent);
}

GVideoLayer::~GVideoLayer()
{
}

void GVideoLayer::paintEvent( QPaintEvent * evt ) {
    //qDebug("Screen::paintEvent");
    Q_UNUSED(evt);
    TreeNode* cf=xplayer->config();
    XFuncNode* fn = getEventFuncNode(cf, "onDraw");
    if( fn ) {
        QPainter p(this);
        TreeNode* d=NULL;
        StrVar* sv=fn->GetVar("@draw");
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
        fn->call();
    } else {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);

        GMpWidget::State stat=  xplayer ? xplayer->state() : GMpWidget::NotStartedState;
        if( stat==GMpWidget::IdleState || stat==GMpWidget::StoppedState ) {
            p.fillRect(rect(), Qt::black);
        }
        p.end();
    }
    //painter.fillRect( e->rect(), QColor(255,0,0) );
}

void GVideoLayer::play() {
    qDebug("MplayerLayer::playingStarted");
    repaint();
}

void GVideoLayer::stop() {
    qDebug("MplayerLayer::playingStopped");
    repaint();
}

void GVideoLayer::checkMousePos() {
    //qDebug("Screen::checkMousePos");

    if ( xcursor == xcursorLast ) {
        //qDebug(" same pos");
        if (cursor().shape() != Qt::BlankCursor) {
            //qDebug(" hiding mouse cursor");
            setCursor(QCursor(Qt::BlankCursor));
        }
    } else {
        xcursorLast = xcursor;
    }
}

void GVideoLayer::mouseMoveEvent( QMouseEvent * e ) {
    //qDebug("Screen::mouseMoveEvent");
    //qDebug(" pos: x: %d y: %d", e->pos().x(), e->pos().y() );
    TreeNode* cf=xplayer->config();
    XFuncNode* fn = getEventFuncNode(cf, "onMouseMove");
    if( fn ) {
        XFunc* fc=fn->xfunc;
        fn->GetVar("@pos")->setVar('i',2).addInt(e->pos().x()).addInt(e->pos().y());
        fn->GetVar("@mode")->setVar('1',0).addUL64((UL64)e->modifiers());
        fn->call();
        if( ccmp(toString(&(fn->xrst)),"ignore") ) {
            return;
        }
    }
    xcursor = e->pos();
    if (cursor().shape() != Qt::ArrowCursor) {
        //qDebug(" showing mouse cursor" );
        setCursor(QCursor(Qt::ArrowCursor));
    }
}



GMpWidget::MediaInfo::MediaInfo()
    : videoBitrate(0), framesPerSecond(0), sampleRate(0), numChannels(0),
      ok(false), length(0), seekable(false)
{

}

GMpWidget::GMpWidget(TreeNode* cf, QWidget *parent) : QWidget(parent), xcf(cf) {
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAutoFillBackground(true);
    m_widget = new GVideoLayer(this);

    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);

    m_seekTimer.setInterval(50);
    m_seekTimer.setSingleShot(true);
    connect(&m_seekTimer, SIGNAL(timeout()), this, SLOT(delayedSeek()));

    m_process = new GMpProcess(cf, this);
    connect(m_process, SIGNAL(stateChanged(int)), this, SLOT(mpStateChanged(int)));
    connect(m_process, SIGNAL(streamPositionChanged(double)), this, SLOT(mpStreamPositionChanged(double)));
    connect(m_process, SIGNAL(error(const QString &)), this, SIGNAL(error(const QString &)));
    connect(m_process, SIGNAL(readStandardOutput(const QString &)), this, SIGNAL(readStandardOutput(const QString &)));
    connect(m_process, SIGNAL(readStandardError(const QString &)), this, SIGNAL(readStandardError(const QString &)));
}

GMpWidget::~GMpWidget()
{
    if( m_process->processState() == QProcess::Running) {
        m_process->quit();
    }
    delete m_process;
}

GMpWidget::State GMpWidget::state() const {
    return m_process->m_state;
}

GMpWidget::MediaInfo GMpWidget::mediaInfo() const {
    return m_process->m_mediaInfo;
}

double GMpWidget::tell() const {
    return m_process->m_streamPosition;
}

QProcess *GMpWidget::process() const {
    return m_process;
}

GMpWidget::Mode GMpWidget::mode() const {
    return m_process->m_mode;
}

void GMpWidget::setVideoOutput(const QString &output) {
    m_process->m_videoOutput = output;
}

QString GMpWidget::videoOutput() const {
    return m_process->m_videoOutput;
}

void GMpWidget::setMPlayerPath(const QString &path) {
    m_process->m_mplayerPath = path;
}

QString GMpWidget::mplayerPath() const {
    return m_process->m_mplayerPath;
}

QString GMpWidget::mplayerVersion() {
    return m_process->mplayerVersion();
}

void GMpWidget::setSeekSlider(QAbstractSlider *slider) {
    if (m_seekSlider) {
        m_seekSlider->disconnect(this);
        disconnect(m_seekSlider);
    }

    if (m_process->m_mediaInfo.ok) {
        slider->setRange(0, m_process->m_mediaInfo.length);
    }
    if (m_process->m_mediaInfo.ok) {
        slider->setEnabled(m_process->m_mediaInfo.seekable);
    }

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
    m_seekSlider = slider;
}

void GMpWidget::setVolumeSlider(QAbstractSlider *slider) {
    if (m_volumeSlider) {
        m_volumeSlider->disconnect(this);
        disconnect(m_volumeSlider);
    }


    slider->setRange(0, 100);
    slider->setValue(100); // TODO

    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(setVolume(int)));
    m_volumeSlider = slider;
}

void GMpWidget::showImage(const QImage &image) {
    // qobject_cast<GVideoWidget*>(m_widget)->showUserImage(image);
}

QSize GMpWidget::sizeHint() const {
    if (m_process->m_mediaInfo.ok && !m_process->m_mediaInfo.size.isNull()) {
        return m_process->m_mediaInfo.size;
    }
    return QWidget::sizeHint();
}

void GMpWidget::start(const QStringList &args) {
    if (m_process->processState() == QProcess::Running) {
        m_process->quit();
    }
    m_process->start(qobject_cast<QWidget*>(m_widget), args);
}

void GMpWidget::load(const QString &url) {
    if(m_process->state() == QProcess::NotRunning ) {
        qDebug()<< "#9##MPlayer process not started yet URL: "<<url<<"\n";
        return;
    }
    // From the MPlayer slave interface documentation:
    // "Try using something like [the following] to switch to the next file.
    // It avoids audio playback starting to play the old file for a short time
    // before switching to the new one.
    writeCommand("pausing_keep_force pt_step 1");
    writeCommand("get_property pause");

    writeCommand(QString("loadfile '%1'").arg(url));
}

void GMpWidget::play() {
    if (m_process->m_state == PausedState) {
        m_process->pause();
    }
}

void GMpWidget::pause() {
    if (m_process->m_state == PlayingState) {
        m_process->pause();
    }
}
void GMpWidget::stop() {
    m_process->stop();
}

bool GMpWidget::seek(int offset, int whence) {
    return seek(double(offset), whence);
}

bool GMpWidget::seek(double offset, int whence) {
    m_seekTimer.stop(); // Cancel all current seek requests

    switch (whence) {
        case RelativeSeek:
        case PercentageSeek:
        case AbsoluteSeek:
            break;
        default:
            return false;
    }

    // Schedule seek request
    m_seekCommand = QString("seek %1 %2").arg(offset).arg(whence);
    m_seekTimer.start();
    return true;
}

void GMpWidget::toggleFullScreen() {
    if( !isFullScreen() ) {
        int num = QApplication::desktop()->screenNumber(QCursor::pos());
        setGeometry(QApplication::desktop()->screenGeometry(num));
        m_windowFlags = windowFlags() & (Qt::Window);
        m_geometry = geometry();
        setWindowFlags((windowFlags() | Qt::Window));
        // From Phonon::VideoWidget
#ifdef Q_WS_X11
        show();
        raise();
        setWindowState(windowState() | Qt::WindowFullScreen);
#else
        setWindowState(windowState() | Qt::WindowFullScreen);
        show();
#endif
    } else {
        setWindowFlags((windowFlags() ^ (Qt::Window)) | m_windowFlags);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        setGeometry(m_geometry);
        show();
    }
}

void GMpWidget::writeCommand(const QString &command) {
    if( command=="quit" ) {
        if( m_process->processState() == QProcess::Running ) {
            m_process->quit();
        }
    } else if( command=="isRunning" ) {
        if( m_process->processState() == QProcess::Running) {
            xcf->GetVar("@state")->set("running");
        } else {
            xcf->GetVar("@state")->reuse();
        }
    } else if( command=="state" ) {
        if( m_process->m_state == PausedState ) {
            xcf->GetVar("@state")->set("pause");
        } else if( m_process->m_state == PlayingState ) {
            xcf->GetVar("@state")->set("play");
        } else if( m_process->m_state == StoppedState ) {
            xcf->GetVar("@state")->set("stop");
            m_widget->update();
        } else if( m_process->m_state == NotStartedState ) {
            xcf->GetVar("@state")->set("notStart");
        } else if( m_process->m_state == IdleState ) {
            xcf->GetVar("@state")->set("idle");
        } else if( m_process->m_state == LoadingState ) {
            xcf->GetVar("@state")->set("loading");
            m_widget->update();
        } else if( m_process->m_state == BufferingState ) {
            xcf->GetVar("@state")->set("buffering");
            m_widget->update();
        } else if( m_process->m_state == ErrorState ) {
            xcf->GetVar("@state")->set("error");
        } else {
            xcf->GetVar("@state")->reuse();
        }
    } else {
        m_process->writeCommand(command);
    }
}

void GMpWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    toggleFullScreen();
    event->accept();
}

void GMpWidget::keyPressEvent(QKeyEvent *event)
{
    XFuncNode* fn = getEventFuncNode(config(), "onKeyDown");
    if( fn ) {
        StrVar* sv=NULL;
        fn->GetVar("@key")->setVar('0',0).addInt(event->key());
        fn->GetVar("@mode")->setVar('1',0).addUL64((UL64)event->modifiers());
        fn->call();
        sv=fn->getResult();
        if( SVCHK('3',1) ) {
            event->setAccepted(false);
            return;
        }
    }
    bool accept = true;
    switch (event->key()) {
        case Qt::Key_P:
        case Qt::Key_Space:
            if (state() == PlayingState) {
                pause();
            } else if (state() == PausedState) {
                play();
            }
            break;

        case Qt::Key_F:
            toggleFullScreen();
            break;

        case Qt::Key_Q:
        case Qt::Key_Escape:
            stop();
            break;

        case Qt::Key_Minus:
            writeCommand("audio_delay -0.1");
            break;
        case Qt::Key_Plus:
            writeCommand("audio_delay 0.1");
            break;

        case Qt::Key_Left:
            seek(-10, RelativeSeek);
            break;
        case Qt::Key_Right:
            seek(10, RelativeSeek);
            break;
        case Qt::Key_Down:
            seek(-60, RelativeSeek);
            break;
        case Qt::Key_Up:
            seek(60, RelativeSeek);
            break;
        case Qt::Key_PageDown:
            seek(-600, RelativeSeek);
            break;
        case Qt::Key_PageUp:
            seek(600, RelativeSeek);
            break;

        case Qt::Key_Asterisk:
            writeCommand("volume 10");
            break;
        case Qt::Key_Slash:
            writeCommand("volume -10");
            break;

        case Qt::Key_X:
            writeCommand("sub_delay 0.1");
            break;
        case Qt::Key_Z:
            writeCommand("sub_delay -0.1");
            break;

        default:
            accept = false;
            break;
    }
    event->setAccepted(accept);
}

void GMpWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    updateWidgetSize();
}

void GMpWidget::updateWidgetSize() {
    bool ok=false;
    bool resizeCheck=isVarTrue(xcf->gv("@resizeCheck"));
    if( resizeCheck && !m_process->m_mediaInfo.size.isNull() ) {
        ok=true;
    }
    if( ok ) {
        QSize mediaSize = m_process->m_mediaInfo.size;
        QSize widgetSize = size();
        double factor = qMin(double(widgetSize.width()) / mediaSize.width(), double(widgetSize.height()) / mediaSize.height());
        QRect rc(0, 0, int(factor * mediaSize.width() + 0.5), int(factor * mediaSize.height()));
        rc.moveTopLeft(rect().center() - rc.center());
        m_widget->setGeometry(rc);
    } else {
        m_widget->setGeometry(QRect(QPoint(0, 0), size()));
    }
}

void GMpWidget::delayedSeek() {
    if (!m_seekCommand.isEmpty()) {
        writeCommand(m_seekCommand);
        m_seekCommand = QString();
    }
}

void GMpWidget::setVolume(int volume) {
    writeCommand(QString("volume %1 1").arg(volume));
}

void GMpWidget::mpStateChanged(int state) {
    if (m_seekSlider != NULL && state == PlayingState && m_process->m_mediaInfo.ok) {
        m_seekSlider->setRange(0, m_process->m_mediaInfo.length);
        m_seekSlider->setEnabled(m_process->m_mediaInfo.seekable);
    }
    if( state==StoppedState || state==IdleState || state==LoadingState || state==BufferingState ) {
        m_widget->update();
    }
    XFuncNode* fn = getEventFuncNode(xcf, "onStateChange");
    if( fn ) {
        StrVar* sv=fn->GetVar("@state");
        if( state==StoppedState ) sv->set("stop");
        else if( state==IdleState ) sv->set("idle");
        else if( state==LoadingState ) sv->set("loading");
        else if( state==BufferingState ) sv->set("buffering");
        else if( state==PausedState ) sv->set("paused");
        else if( state==PlayingState ) sv->set("play");
        else if( state==StoppedState ) sv->set("stop");
        else if( state==NotStartedState ) sv->set("notStarted");
        else if( state==ErrorState ) sv->set("error");
        else sv->set("etc");
        fn->call();
    }
    updateWidgetSize();
    emit stateChanged(state);
}

void GMpWidget::mpStreamPositionChanged(double position) {
    /*
    if (m_seekSlider != NULL && m_seekCommand.isEmpty() && m_seekSlider->value() != qRound(position)) {
        m_seekSlider->disconnect(this);
        m_seekSlider->setValue(qRound(position));
        connect(m_seekSlider, SIGNAL(valueChanged(int)), this, SLOT(seek(int)));
    }
    */
    if( xcf==NULL ) return;
    XFuncNode* fn = getEventFuncNode(config(), "onStateChange");
    xcf->GetVar("@seekPosition")->setVar('4',0).addDouble(position);
    if( fn ) {
        fn->GetVar("@state")->set("position");
        fn->GetVar("@value")->setVar('4',0).addDouble(position);
        fn->call();
    }
}

void GMpWidget::mpVolumeChanged(int volume) {
    XFuncNode* fn = getEventFuncNode(config(), "onStateChange");
    if( fn ) {
        fn->GetVar("@state")->set("volume");
        fn->GetVar("@value")->setVar('0',0).addInt(volume);
        fn->call();
    }
}
