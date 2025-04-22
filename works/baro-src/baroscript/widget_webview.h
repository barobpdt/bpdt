#ifndef WIDGET_WEBVIEW_H
#define WIDGET_WEBVIEW_H
#include "widgets.h"
#include "func_util.h"

#ifdef WEBVIEW_USE
#include <qwebview.h>
#include <qwebpage.h>
#include <qwebframe.h>
#include <qnetworkdiskcache.h>

class WebPageLinkedResource
{
public:
    QString rel;
    QString type;
    QUrl href;
    QString title;
};

class WebViewStyle : public QWindowsStyle {
public:
    int mScrollWidth;
    WebViewStyle(int width) {
        mScrollWidth=width;
    }
    int pixelMetric ( PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const {
        if( metric == QStyle::PM_ScrollBarExtent && widget == 0)
            return mScrollWidth;
        return QWindowsStyle::pixelMetric(metric, option, widget);
    }
};
class WebPage : public QWebPage {
    Q_OBJECT
signals:
    void aboutToLoadUrl(const QUrl &url);
public:
    WebPage(QObject *parent = 0);
    ~WebPage();
    void loadSettings();
     QList<WebPageLinkedResource> linkedResources(const QString &relation = QString());
protected:
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request,
                                 NavigationType type);

protected slots:
    void handleUnsupportedContent(QNetworkReply *reply);
    void addExternalBinding(QWebFrame *frame = 0);

protected:
    QUrl m_requestedUrl;

private:
    QNetworkRequest lastRequest;
    QWebPage::NavigationType lastRequestType;

};

class WebView : public QWebView {
    Q_OBJECT

public:
    WebView(TreeNode* cf, QWidget *parent = 0);
    WebPage* webPage() const { return m_page; }

#if !(QT_VERSION >= 0x040600)
    static QUrl guessUrlFromString(const QString &url);
#endif
    void loadSettings();

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    void keyReleaseEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *event);
#endif

    void loadUrl(const QUrl &url, const QString &title = QString());
    QUrl url() const;

    QString lastStatusBarText() const;
    inline int progress() const { return m_progress; }
    void viewSource();
    void slotSourceDownloaded();
    void loadPage(const QUrl &url, QNetworkDiskCache* cache);
public:
    TreeNode* xcf;
    XListArr xparam;
    StrVar xrst;
    QList<QWebElement> xlistFind;
    QList<QWebElement> xlistInfo;

    TreeNode* config();
    void call(const char* str, StrVar* rst);
    StrVar* getResult() { return xrst.reuse(); }
    StrVar* callback(LPCC type, StrVar* var=NULL);
    void hitTest(QPoint pt, StrVar* rst=NULL);
    void sendMouseDownEvent(QPoint pt, StrVar* rst);
    void showAccessKeys();
    void hideAccessKeys();
    void tagInfo(LPCC tag, LPCC attr, TreeNode* root, bool isView=false );
    void findParentTag(int index, LPCC tag, LPCC attr, StrVar* rst );
    void findAll(int index, LPCC query, LPCC attr, XListArr* arr );
    void findChild(int index, LPCC query, LPCC attr, XListArr* arr );
    bool getElementInfo(int index, TreeNode* node);
    bool setElementAttribute(int index, LPCC name, StrVar* data);
    bool setElementValue(int index, LPCC data, StrVar* rst);
    bool renderElement(int index, LPCC type, LPCC output=NULL, StrVar* sv=NULL);
    void setZoom(int zoom);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void applyZoom();
    void slotLinkClicked(QUrl);
    void handleSslErrors(QNetworkReply* reply, const QList<QSslError> &errors);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);
    void resizeEvent(QResizeEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    int levelForZoom(int zoom);

private slots:
    void setProgress(int progress);
    void loadFinished();
    void setStatusBarText(const QString &string);
    void downloadRequested(const QNetworkRequest &request);
    void openActionUrlInNewTab();
    void openActionUrlInNewWindow();
    void downloadLinkToDisk();
    void copyLinkToClipboard();
    void openImageInNewWindow();
    void downloadImageToDisk();
    void copyImageToClipboard();
    void copyImageLocationToClipboard();
    void blockImage();
    void bookmarkLink();
    void searchRequested(QAction *action);
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    void addSearchEngine();
    void accessKeyShortcut();
#endif

private:
    QString m_statusBarText;
    QUrl m_initialUrl;
    int m_progress;
    int m_currentZoom;
    QList<int> m_zoomLevels;
    WebPage *m_page;

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    bool m_enableAccessKeys;
    bool checkForAccessKey(QKeyEvent *event);

    void makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element);
    QList<QLabel*> m_accessKeyLabels;
    QHash<QChar, QWebElement> m_accessKeyNodes;
    bool m_accessKeysPressed;
#endif
};
#endif

#endif // WIDGET_WEBVIEW_H
