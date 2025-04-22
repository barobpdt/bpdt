#ifndef WIDGET_PLAYER_H
#define WIDGET_PLAYER_H
#include "widgets.h"

class GMpProcess;
class GVideoLayer;
class GMpWidget : public QWidget {
Q_OBJECT
public:
    enum State {
        NotStartedState = -1,
        IdleState,
        LoadingState,
        StoppedState,
        PlayingState,
        BufferingState,
        PausedState,
        ErrorState
    };
    struct MediaInfo {
        QString videoFormat;
        int videoBitrate;
        QSize size;
        double framesPerSecond;

        QString audioFormat;
        double audioBitrate;
        int sampleRate;
        int numChannels;

        QHash<QString, QString> tags;

        bool ok;
        double length;
        bool seekable;

        MediaInfo();
    };

    enum Mode {
        EmbeddedMode = 0,
        PipeMode
    };

    enum SeekMode {
        RelativeSeek = 0,
        PercentageSeek,
        AbsoluteSeek
    };

public:
    GMpWidget(TreeNode* cf, QWidget *parent=0);
    virtual ~GMpWidget();

    State state() const;
    MediaInfo mediaInfo() const;
    double tell() const;
    QProcess *process() const;

    Mode mode() const;

    void setVideoOutput(const QString &output);
    QString videoOutput() const;

    void setMPlayerPath(const QString &path);
    QString mplayerPath() const;
    QString mplayerVersion();
    void setSeekSlider(QAbstractSlider *slider);
    void setVolumeSlider(QAbstractSlider *slider);
    void showImage(const QImage &image);
    TreeNode* config() { return xcf; }

public:
    TreeNode* xcf;

public slots:
    void start(const QStringList &args = QStringList());
    void load(const QString &url);
    void play();
    void pause();
    void stop();
    bool seek(int offset, int whence = AbsoluteSeek);
    bool seek(double offset, int whence = AbsoluteSeek);

    void toggleFullScreen();

    void writeCommand(const QString &command);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual QSize sizeHint() const;
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QWidget::event(evt);
    }

private:
    void updateWidgetSize();

private slots:
    void setVolume(int volume);

    void mpStateChanged(int state);
    void mpStreamPositionChanged(double position);
    void mpVolumeChanged(int volume);
    void delayedSeek();

signals:
    void stateChanged(int state);
    void error(const QString &reason);

    void readStandardOutput(const QString &line);
    void readStandardError(const QString &line);
private:
    GMpProcess *m_process;
    GVideoLayer *m_widget;
    QPointer<QAbstractSlider> m_seekSlider;
    QPointer<QAbstractSlider> m_volumeSlider;
    Qt::WindowFlags m_windowFlags;
    QRect m_geometry;

    QTimer m_seekTimer;
    QString m_seekCommand;
};


class GMpProcess : public QProcess {
Q_OBJECT
public:
    GMpProcess(TreeNode* cf, QObject *parent=0);
    ~GMpProcess();
    void start(QWidget *widget, const QStringList &args);
    QString mplayerVersion();
    QProcess::ProcessState processState() const;
    void writeCommand(const QString &command);
    void quit();
    void pause();
    void stop();
    TreeNode* config() { return xcf; }

signals:
    void stateChanged(int state);
    void streamPositionChanged(double position);
    void error(const QString &reason);

    void readStandardOutput(const QString &line);
    void readStandardError(const QString &line);

private slots:
    void readStdout();
    void readStderr();
    void finished();
    void movieFinished();

private:
    void parseLine(const QString &line);
    void parseMediaInfo(const QString &line);
    void parsePosition(const QString &line);
    void changeState(GMpWidget::State state, const QString &comment = QString());
    void resetValues();
    void writeFakeInputconf(QIODevice *device);
public:
    GMpWidget::State m_state;
    GMpWidget::Mode m_mode;
    GMpWidget::MediaInfo m_mediaInfo;
    double m_streamPosition; // This is the video position

    QString m_mplayerPath;
    QString m_videoOutput;
    QString m_pipe;
    QTimer m_movieFinishedTimer;
    QString m_currentTag;
    QTemporaryFile *m_fakeInputconf;

    TreeNode*	xcf;
};

class GVideoLayer : public QWidget {
    Q_OBJECT
public:
    GVideoLayer(GMpWidget *parent, Qt::WindowFlags flag=0 );
    virtual ~GVideoLayer();
public:
    void play();
    void stop();
    GMpWidget* xplayer;

protected:
      virtual void mouseMoveEvent( QMouseEvent * e );
      virtual void paintEvent ( QPaintEvent * e );

protected slots:
      virtual void checkMousePos();
private:
      QPoint xcursor, xcursorLast;
};



#endif // WIDGET_PLAYER_H
