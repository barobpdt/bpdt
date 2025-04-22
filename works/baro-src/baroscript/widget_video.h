#ifndef WIDGET_VIDEO_H
#define WIDGET_VIDEO_H
#include "widget_player.h"

class GVideo : public QLabel {
Q_OBJECT
public:
    GVideo(TreeNode* cf, QWidget *parent=0);
    virtual ~GVideo();
    TreeNode* config() { return xcf; }
    QProcess* process() const { return m_process; }
    double tell() const { return m_process->m_streamPosition; }
    GMpWidget::State state() const { return m_process->m_state; }
    GMpWidget::Mode mode() const { return m_process->m_mode; }
    GMpWidget::MediaInfo mediaInfo() const { return m_process->m_mediaInfo; }
    void setVideoOutput(const QString &output) { m_process->m_videoOutput = output; }
    QString videoOutput() const  { return m_process->m_videoOutput; }
    void setMPlayerPath(const QString &path)  { m_process->m_mplayerPath = path; }
    QString mplayerPath() const { return m_process->m_mplayerPath; }
    QString mplayerVersion() { return m_process->mplayerVersion(); }
    void endVideo() {
        if( m_process->processState() == QProcess::Running) {
            m_process->quit();
        }
    }

public:
    TreeNode* xcf;
    TreeNode xcanvasConf;
    GCanvas* xcanvas;
public:
    void start(const QStringList &args = QStringList());
    void openViedo(const QString &url);
    void play();
    void pause();
    void stop();
    void setVolume(int volume);
    void seek(double offset, int whence=2);
    void toggleFullScreen();
    void writeCommand(const QString &command);
    void delayCommand(LPCC cmd);
    GCanvas* canvas() { return xcanvas; }

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual QSize sizeHint() const;
    virtual bool event(QEvent* evt) {
        if( procWidgetEvent(evt, config(), this) ) return true;
        return QLabel::event(evt);
    }

private slots:
    void onVolumeChanged(int volume);
    void onStateChanged(int state);
    void onStreamPositionChanged(double position);
    void onReadOut(const QString &line);
    void onError(const QString &err);
    void delayedSeek();

protected:
    virtual void mouseMoveEvent( QMouseEvent * e );
    virtual void paintEvent ( QPaintEvent * e );

protected slots:
    virtual void checkMousePos();

private:
    GMpProcess *m_process;
    QRect m_geometry;
    QTimer m_seekTimer;
    QString m_seekCommand;
    QPoint xcursor, xcursorLast;
    XListArr xparams;
};

#endif // WIDGET_VIDEO_H
