#ifndef CutyCapt_H
#define CutyCapt_H

#include <QtWebKit>
#include <QByteArray>

#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets>
#endif

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include "func_util.h"

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    NetworkAccessManager(QObject *parent = 0);

    virtual QNetworkReply* createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );

private:
    QList<QString> sslTrustedHostList;
    qint64 requestFinishedCount;
    qint64 requestFinishedFromCacheCount;
    qint64 requestFinishedPipelinedCount;
    qint64 requestFinishedSecureCount;
    qint64 requestFinishedDownloadBufferCount;

public slots:
    void loadSettings();
    void requestFinished(QNetworkReply *reply);

private slots:
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif
};

class CutyCapt;
class CutyPage : public QWebPage {
  Q_OBJECT

public:
  void setAttribute(QWebSettings::WebAttribute option, const QString& value);
  void setUserAgent(const QString& userAgent);
  void setAlertString(const QString& alertString);
  void setPrintAlerts(bool printAlerts);
  void setCutyCapt(CutyCapt* cutyCapt);
  QString getAlertString();

protected:
  QString chooseFile(QWebFrame *frame, const QString& suggestedFile);
  void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);
  bool javaScriptPrompt(QWebFrame* frame, const QString& msg, const QString& defaultValue, QString* result);
  void javaScriptAlert(QWebFrame* frame, const QString& msg);
  bool javaScriptConfirm(QWebFrame* frame, const QString& msg);
  QString userAgentForUrl(const QUrl& url) const;
  QString mUserAgent;
  QString mAlertString;
  bool mPrintAlerts;
  CutyCapt* mCutyCapt;
};

class XFuncNode;
class TreeNode;
class CutyCapt : public QObject {
  Q_OBJECT

public:

  // TODO: This should really be elsewhere and be named differently
  enum OutputFormat { SvgFormat, PdfFormat, PsFormat, InnerTextFormat, HtmlFormat,
    RenderTreeFormat, PngFormat, JpegFormat, MngFormat, TiffFormat, GifFormat,
    BmpFormat, PpmFormat, XbmFormat, XpmFormat, JpgFormat, OtherFormat };

  CutyCapt(CutyPage* page,
           const QString& output,
           int delay,
           OutputFormat format,
           const QString& scriptProp,
           const QString& scriptCode,
           bool insecure,
           bool smooth);

  QByteArray getByteArray() { return mByteArray; }
  XFuncNode* getCallback() { return mCallback; }
  void setCallback(XFuncNode* fn);
  TreeNode* config();
  void setConfig(TreeNode* cf);
  void setFormat(const char* fmt);
private slots:
  void DocumentComplete(bool ok);
  void InitialLayoutCompleted();
  void JavaScriptWindowObjectCleared();
  void Timeout();
  void Delayed();
  void handleSslErrors(QNetworkReply* reply, QList<QSslError> errors);

private:
  void TryDelayedRender();
  void saveSnapshot();
  bool mSawInitialLayout;
  bool mSawDocumentComplete;

protected:
  QString      mOutput;
  int          mDelay;
  CutyPage*    mPage;
  OutputFormat mFormat;
  QObject*     mScriptObj;
  QString      mScriptProp;
  QString      mScriptCode;
  QByteArray   mByteArray;
  XFuncNode*   mCallback;
  TreeNode*	   mConfig;
  bool         mInsecure;
  bool         mSmooth;
  bool		   mFinished;
};


#endif
