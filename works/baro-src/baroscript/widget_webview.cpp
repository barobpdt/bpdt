#include "widget_webview.h"

#ifdef WEBVIEW_USE
#include <qwebframe.h>
#include <qwebelement.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkrequest.h>
#include <qsslconfiguration.h>
#include <qsslcertificate.h>

inline bool isOverMap(QPoint pos, QWebPage* webPage ) {
     if( webPage) {
        QWebFrame* webFrame = webPage->frameAt(pos);
        if (webFrame) {
            QString selectorQuery = "#map-canvas"; // Based on https://developers.google.com/maps/tutorials/fundamentals/adding-a-google-map
            QList<QWebElement> list = webFrame->findAllElements(selectorQuery).toList(); // Find all the maps!
            foreach(QWebElement element, list) {
                if( element.geometry().contains(pos)) {
                    return true; // Cursor is over a map
                }
            }
        }
    }
    return false; // No match
}

WebPage::WebPage(QObject *parent) : QWebPage(parent)
{
#ifdef WEBPLUGIN_USE
    setPluginFactory(webPluginFactory());
    NetworkAccessManagerProxy *networkManagerProxy = new NetworkAccessManagerProxy(this);
    networkManagerProxy->setWebPage(this);
    networkManagerProxy->setPrimaryNetworkAccessManager(BrowserApplication::networkAccessManager());
    setNetworkAccessManager(networkManagerProxy);
#endif
    connect(this, SIGNAL(unsupportedContent(QNetworkReply *)),
            this, SLOT(handleUnsupportedContent(QNetworkReply *)));
    connect(this, SIGNAL(frameCreated(QWebFrame *)),
            this, SLOT(addExternalBinding(QWebFrame *)));
    addExternalBinding(mainFrame());
    loadSettings();
}

WebPage::~WebPage() {
    setNetworkAccessManager(0);
}

QList<WebPageLinkedResource> WebPage::linkedResources(const QString &relation)
{
    QList<WebPageLinkedResource> resources;

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    QUrl baseUrl = mainFrame()->baseUrl();

    QWebElementCollection linkElements = mainFrame()->findAllElements(QLatin1String("html > head > link"));

    foreach( const QWebElement &linkElement, linkElements) {
        QString rel = linkElement.attribute(QLatin1String("rel"));
        QString href = linkElement.attribute(QLatin1String("href"));
        QString type = linkElement.attribute(QLatin1String("type"));
        QString title = linkElement.attribute(QLatin1String("title"));

        if (href.isEmpty() || type.isEmpty())
            continue;
        if (!relation.isEmpty() && rel != relation)
            continue;

        WebPageLinkedResource resource;
        resource.rel = rel;
        resource.type = type;
        resource.href = baseUrl.resolved(QUrl::fromEncoded(href.toUtf8()));
        resource.title = title;

        resources.append(resource);
    }
#else
    QString baseUrlString = mainFrame()->evaluateJavaScript(QLatin1String("document.baseURI")).toString();
    QUrl baseUrl = QUrl::fromEncoded(baseUrlString.toUtf8());

    QFile file(QLatin1String(":fetchLinks.js"));
    if (!file.open(QFile::ReadOnly))
        return resources;
    QString script = QString::fromUtf8(file.readAll());

    QVariantList list = mainFrame()->evaluateJavaScript(script).toList();
    foreach (const QVariant &variant, list) {
        QVariantMap map = variant.toMap();
        QString rel = map[QLatin1String("rel")].toString();
        QString type = map[QLatin1String("type")].toString();
        QString href = map[QLatin1String("href")].toString();
        QString title = map[QLatin1String("title")].toString();

        if (href.isEmpty() || type.isEmpty())
            continue;
        if (!relation.isEmpty() && rel != relation)
            continue;

        WebPageLinkedResource resource;
        resource.rel = rel;
        resource.type = type;
        resource.href = baseUrl.resolved(QUrl::fromEncoded(href.toUtf8()));
        resource.title = title;

        resources.append(resource);
    }
#endif
    return resources;
}

void WebPage::addExternalBinding(QWebFrame *frame)
{
#ifdef WEBPLUGIN_USE
    if (!m_javaScriptExternalObject)
        m_javaScriptExternalObject = new JavaScriptExternalObject(this);

    if (frame == 0) { // called from QWebFrame::javaScriptWindowObjectCleared
        frame = qobject_cast<QWebFrame*>(sender());

        if (frame->url().scheme() == QLatin1String("qrc")
            && frame->url().path() == QLatin1String("/startpage.html")) {

            if (!m_javaScriptAroraObject)
                m_javaScriptAroraObject = new JavaScriptAroraObject(this);

            frame->addToJavaScriptWindowObject(QLatin1String("arora"), m_javaScriptAroraObject);
        }
    } else { // called from QWebPage::frameCreated
        connect(frame, SIGNAL(javaScriptWindowObjectCleared()),
                this, SLOT(addExternalBinding()));
    }
    frame->addToJavaScriptWindowObject(QLatin1String("external"), m_javaScriptExternalObject);
#endif
}

bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request,
                                      NavigationType type)
{
    lastRequest = request;
    lastRequestType = type;
    bool accepted = QWebPage::acceptNavigationRequest(frame, request, type);
    if( accepted && frame == mainFrame()) {
        m_requestedUrl = request.url();
        emit aboutToLoadUrl(request.url());
    }
    return accepted;
}

void WebPage::loadSettings() {

}

static bool contentSniff(const QByteArray &data)
{
    if (data.contains("<!doctype")
        || data.contains("<script")
        || data.contains("<html")
        || data.contains("<!--")
        || data.contains("<head")
        || data.contains("<iframe")
        || data.contains("<h1")
        || data.contains("<div")
        || data.contains("<font")
        || data.contains("<table")
        || data.contains("<a")
        || data.contains("<style")
        || data.contains("<title")
        || data.contains("<b")
        || data.contains("<body")
        || data.contains("<br")
        || data.contains("<p"))
        return true;
    return false;
}

void WebPage::handleUnsupportedContent(QNetworkReply *reply) {
    if (!reply)
        return;

    QUrl replyUrl = reply->url();
    switch (reply->error()) {
    case QNetworkReply::NoError:
        if( reply->header(QNetworkRequest::ContentTypeHeader).isValid() ) {
            return;
        }
        break;
    case QNetworkReply::ProtocolUnknownError: {
        break;
    }
    default:
        break;
    }

    // Find the frame that has the unsupported content
    if (replyUrl.isEmpty() || replyUrl != m_requestedUrl)
        return;

    QWebFrame *notFoundFrame = mainFrame();
    if( !notFoundFrame)
        return;

    if( reply->header(QNetworkRequest::ContentTypeHeader).toString().isEmpty()) {
        // do evil
        QByteArray data = reply->readAll();
        if( contentSniff(data)) {
            notFoundFrame->setHtml(QLatin1String(data), replyUrl);
            return;
        }
    }
#ifdef WEBPLUGIN_USE
    QFile notFoundErrorFile(QLatin1String(":/notfound.html"));
    if (!notFoundErrorFile.open(QIODevice::ReadOnly))
        return;
    QString title = tr("Error loading page: %1").arg(QString::fromUtf8(replyUrl.toEncoded()));
    QString html = QLatin1String(notFoundErrorFile.readAll());
    QPixmap pixmap = qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view()).pixmap(QSize(32, 32));
    QBuffer imageBuffer;
    imageBuffer.open(QBuffer::ReadWrite);
    if (pixmap.save(&imageBuffer, "PNG")) {
        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
                     QLatin1String(imageBuffer.buffer().toBase64()));
    }
    html = html.arg(title,
                    reply->errorString(),
                    tr("When connecting to: %1.").arg(QString::fromUtf8(replyUrl.toEncoded())),
                    tr("Check the address for errors such as <b>ww</b>.arora-browser.org instead of <b>www</b>.arora-browser.org"),
                    tr("If the address is correct, try checking the network connection."),
                    tr("If your computer or network is protected by a firewall or proxy, make sure that the browser is permitted to access the network."));
    notFoundFrame->setHtml(html, replyUrl);
    // Don't put error pages to the history.
    BrowserApplication::instance()->historyManager()->removeHistoryEntry(replyUrl, notFoundFrame->title());
#endif
}


/*
QPoint currentPosition = webView->page()->mainFrame()->scrollPosition();
if (currentPosition.y() == webView->page()->mainFrame()->scrollBarMinimum(Qt::Vertical))
   qDebug() << "Head of contents";
if (currentPosition.y() == webView->page()->mainFrame()->scrollBarMaximum(Qt::Vertical))
   qDebug() << "End of contents";
*/



WebView::WebView(TreeNode*cf, QWidget *parent)
    : QWebView(parent), xcf(cf)
    , m_progress(0)
    , m_currentZoom(100)
    , m_page(new WebPage(this))
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    , m_enableAccessKeys(true)
    , m_accessKeysPressed(false)
#endif
{
    setPage(m_page);
    // setMouseTracking(true);

#if QT_VERSION >= 0x040600
    QPalette p;
    if (p.color(QPalette::Window) != Qt::white) {
        QWindowsStyle s;
        p = s.standardPalette();
        setPalette(p);
    }
#endif
    connect(page(), SIGNAL(statusBarMessage(const QString&)), SLOT(setStatusBarText(const QString&)));
    connect(page()->networkAccessManager(),SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )),
        this, SLOT(handleSslErrors(QNetworkReply*, const QList<QSslError> & )));
    connect(this, SIGNAL(loadProgress(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(this, SIGNAL(linkClicked(QUrl)),this,SLOT(slotLinkClicked(QUrl)));

    connect(page(), SIGNAL(aboutToLoadUrl(const QUrl &)), this, SIGNAL(urlChanged(const QUrl &)));
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));
    page()->setForwardUnsupportedContent(true);
    page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    page()->settings()->setAttribute(QWebSettings::LocalStorageEnabled, true);
    page()->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls,true);
    page()->settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
    page()->settings()->setAttribute(QWebSettings::AutoLoadImages,true);
    page()->settings()->setMaximumPagesInCache(50);
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    page()->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    // Object that contains path
    // page()->mainFrame()->addToJavaScriptWindowObject("customPdfJsObject",m_loader,QWebFrame::QtOwnership

    // setAcceptDrops(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    // setContextMenuPolicy( Qt::PreventContextMenu );

    /* SSL	*/
    QSslConfiguration sslconf = QSslConfiguration::defaultConfiguration();
    QList<QSslCertificate> cert_list = sslconf.caCertificates();
    QList<QSslCertificate> cert_new = QSslCertificate::fromData("CaCertificates");
    cert_list += cert_new;

    sslconf.setCaCertificates(cert_list);
    sslconf.setProtocol(QSsl::AnyProtocol);
    QSslConfiguration::setDefaultConfiguration(sslconf);



    // the zoom values (in percent) are chosen to be like in Mozilla Firefox 3
    m_zoomLevels << 30 << 50 << 67 << 80 << 90;
    m_zoomLevels << 100;
    m_zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    connect(m_page, SIGNAL(loadStarted()), this, SLOT(hideAccessKeys()));
    connect(m_page, SIGNAL(scrollRequested(int, int, const QRect &)), this, SLOT(hideAccessKeys()));
#endif
    // loadSettings();
}

TreeNode* WebView::config() { return xcf; }
void WebView::call(LPCC str, StrVar* rst) {
    QVariant val = page()->mainFrame()->evaluateJavaScript(KR(str)); // "ReturnValueToQT()"
    rst->set(Q2A(val.toString()));
}

StrVar* WebView::callback(LPCC type, StrVar* var) {
    XFuncNode* fn=getEventFuncNode(config(), "@callback");
    StrVar* rst=NULL;
    if( fn ) {
        PARR arrs=getLocalParams(fn);
        arrs->add()->set(type);
        arrs->add()->add(var);
        setFuncNodeParams(fn, arrs);
        fn->call();
        rst=fn->getResult();
    }
    return rst;
}

void WebView::viewSource()
{
    /*
    QNetworkAccessManager* accessManager = page()->networkAccessManager();
    QNetworkRequest request(url());
    QNetworkReply* reply = accessManager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(slotSourceDownloaded()));
    */
    StrVar* src=NULL;
    StrVar* sv=config()->GetVar("@refSource");
    if( SVCHK('s',0) ) {
        src=(StrVar*)SVO;
        src->reuse();
    } else {
        src=new StrVar();
        sv->setVar('s',0,(LPVOID)src).addInt(0).addInt(0).addInt(0).addInt(0);
    }
    StrVar* data=getResult();
    src->set( Q2A(page()->mainFrame()->toHtml()) );
    data->setVar('s',0,(LPVOID)src).addInt(0).addInt(src->length()).addInt(0).addInt(0);
    callback("viewSource", data);
}

void WebView::slotSourceDownloaded()
{
    /*
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(const_cast<QObject*>(sender()));
    QTextEdit* textEdit = new QTextEdit(NULL);
    textEdit->setAttribute(Qt::WA_DeleteOnClose);
    textEdit->show();
    textEdit->setPlainText(reply->readAll());
    reply->deleteLater();
    */
}


void WebView::loadSettings()
{
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    QSettings settings;
    settings.beginGroup(QLatin1String("WebView"));
    m_enableAccessKeys = settings.value(QLatin1String("enableAccessKeys"), m_enableAccessKeys).toBool();

    if (!m_enableAccessKeys)
        hideAccessKeys();
#endif
    m_page->loadSettings();
}

void WebView::handleSslErrors(QNetworkReply* reply, const QList<QSslError> &errors) {
    qDebug() << "handleSslErrors: ";
    StrVar* data=getResult();
    foreach(QSslError e, errors ) {
        qDebug() << "ssl error: " << e;
        if( data->length() ) data->add("\r\n");
        data->add( Q2A(e.errorString()) );
    }
    callback("sslError", data);
    reply->ignoreSslErrors();
}

void WebView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    if (!r.linkUrl().isEmpty()) {
        QAction *newWindowAction = menu->addAction(tr("Open in New &Window"), this, SLOT(openActionUrlInNewWindow()));
        newWindowAction->setData(r.linkUrl());
        QAction *newTabAction = menu->addAction(tr("Open in New &Tab"), this, SLOT(openActionUrlInNewTab()));
        newTabAction->setData(r.linkUrl());
        menu->addSeparator();
        menu->addAction(tr("Save Lin&k"), this, SLOT(downloadLinkToDisk()));
        menu->addAction(tr("&Bookmark This Link"), this, SLOT(bookmarkLink()))->setData(r.linkUrl());
        menu->addSeparator();
        if (!page()->selectedText().isEmpty())
            menu->addAction(pageAction(QWebPage::Copy));
        menu->addAction(tr("&Copy Link Location"), this, SLOT(copyLinkToClipboard()));
    }

    if (!r.imageUrl().isEmpty()) {
        if (!menu->isEmpty())
            menu->addSeparator();
        QAction *newWindowAction = menu->addAction(tr("Open Image in New &Window"), this, SLOT(openActionUrlInNewWindow()));
        newWindowAction->setData(r.imageUrl());
        QAction *newTabAction = menu->addAction(tr("Open Image in New &Tab"), this, SLOT(openActionUrlInNewTab()));
        newTabAction->setData(r.imageUrl());
        menu->addSeparator();
        menu->addAction(tr("&Save Image"), this, SLOT(downloadImageToDisk()));
        menu->addAction(tr("&Copy Image"), this, SLOT(copyImageToClipboard()));
        menu->addAction(tr("C&opy Image Location"), this, SLOT(copyImageLocationToClipboard()))->setData(r.imageUrl().toString());
        menu->addSeparator();
        menu->addAction(tr("Block Image"), this, SLOT(blockImage()))->setData(r.imageUrl().toString());
    }

    if (!page()->selectedText().isEmpty()) {
        if (menu->isEmpty()) {
            menu->addAction(pageAction(QWebPage::Copy));
        } else {
            menu->addSeparator();
        }
        QMenu *searchMenu = menu->addMenu(tr("Search with..."));

        /*
        QList<QString> list = ToolbarSearch::openSearchManager()->allEnginesNames();
        for (int i = 0; i < list.count(); ++i) {
            QString name = list.at(i);
            OpenSearchEngine *engine = ToolbarSearch::openSearchManager()->engine(name);
            QAction *action = new OpenSearchEngineAction(engine, searchMenu);
            searchMenu->addAction(action);
            action->setData(name);
        }
        */
        connect(searchMenu, SIGNAL(triggered(QAction *)), this, SLOT(searchRequested(QAction *)));
    }

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    QWebElement element = r.element();
    if (!element.isNull()
        && element.tagName().toLower() == QLatin1String("input")
        && element.attribute(QLatin1String("type"), QLatin1String("text")) == QLatin1String("text")) {
        if (menu->isEmpty()) {
            menu->addAction(pageAction(QWebPage::Copy));
        } else {
            menu->addSeparator();
        }

        QVariant variant;
        variant.setValue(element);
        menu->addAction(tr("Add to the toolbar search"), this, SLOT(addSearchEngine()))->setData(variant);
    }
#endif

    if (menu->isEmpty()) {
        delete menu;
        menu = page()->createStandardContextMenu();
    } else {
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
            menu->addAction(pageAction(QWebPage::InspectElement));
    }

    if ( !menu->isEmpty() ) {
        menu->exec(mapToGlobal(event->pos()));
        delete menu;
        return;
    }
    QWebView::contextMenuEvent(event);
}

void WebView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        m_currentZoom = m_currentZoom + numSteps * 10;
        applyZoom();
        event->accept();
        return;
    }
    QWebView::wheelEvent(event);
}

void WebView::resizeEvent(QResizeEvent *event)
{
    int offset = event->size().height() - event->oldSize().height();
    int currentValue = page()->mainFrame()->scrollBarValue(Qt::Vertical);
    setUpdatesEnabled(false);
    page()->mainFrame()->setScrollBarValue(Qt::Vertical, currentValue - offset);
    setUpdatesEnabled(true);
    QWebView::resizeEvent(event);
}

void WebView::downloadLinkToDisk()
{
    pageAction(QWebPage::DownloadLinkToDisk)->trigger();
}

void WebView::copyLinkToClipboard()
{
    pageAction(QWebPage::CopyLinkToClipboard)->trigger();
}

void WebView::openActionUrlInNewTab() {
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QWebPage *page = webPage();
        QNetworkRequest request(action->data().toUrl());
        request.setRawHeader("Referer", url().toEncoded());
        page->mainFrame()->load(request);
    }
}

void WebView::openActionUrlInNewWindow()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QWebPage *page = webPage();
        QNetworkRequest request(action->data().toUrl());
        request.setRawHeader("Referer", url().toEncoded());
        page->mainFrame()->load(request);
    }
}

void WebView::openImageInNewWindow()
{
    pageAction(QWebPage::OpenImageInNewWindow)->trigger();
}

void WebView::downloadImageToDisk()
{
    pageAction(QWebPage::DownloadImageToDisk)->trigger();
}

void WebView::copyImageToClipboard()
{
    pageAction(QWebPage::CopyImageToClipboard)->trigger();
}

void WebView::copyImageLocationToClipboard()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        // QApplication::clipboard()->setText(action->data().toString());
    }
}

void WebView::blockImage()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QString imageUrl = action->data().toString();
        StrVar* data=getResult();
        data->add(Q2A(imageUrl) );
        callback("blockImage", data);
    }
}

void WebView::bookmarkLink()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QString url=action->data().toUrl().toEncoded();
        StrVar* data=getResult();
        data->add(Q2A(url) );
        callback("bookmark", data);
    }
}

void WebView::searchRequested(QAction *action) {
    QString searchText = page()->selectedText();
    if (searchText.isEmpty())
        return;
    StrVar* data=getResult();
    data->add(Q2A(searchText) );
    callback("searchText", data);
}

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
void WebView::addSearchEngine()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action)
        return;

    QVariant variant = action->data();
    if (!variant.canConvert<QWebElement>())
        return;

    QWebElement element = qvariant_cast<QWebElement>(variant);
    QString elementName = element.attribute(QLatin1String("name"));
    QWebElement formElement = element;
    while (formElement.tagName().toLower() != QLatin1String("form"))
        formElement = formElement.parent();

    if (formElement.isNull() || formElement.attribute(QLatin1String("action")).isEmpty())
        return;

    QString method = formElement.attribute(QLatin1String("method"), QLatin1String("get")).toLower();
    if (method != QLatin1String("get")) {
        QMessageBox::warning(this, tr("Method not supported"),
                             tr("%1 method is not supported.").arg(method.toUpper()));
        return;
    }

    QUrl searchUrl(page()->mainFrame()->baseUrl().resolved(QUrl(formElement.attribute(QLatin1String("action")))));
    QMap<QString, QString> searchEngines;
    QWebElementCollection inputFields = formElement.findAll(QLatin1String("input"));
    foreach (QWebElement inputField, inputFields) {
        QString type = inputField.attribute(QLatin1String("type"), QLatin1String("text"));
        QString name = inputField.attribute(QLatin1String("name"));
        QString value = inputField.evaluateJavaScript(QLatin1String("this.value")).toString();

        if (type == QLatin1String("submit")) {
            searchEngines.insert(value, name);
        } else if (type == QLatin1String("text")) {
            if (inputField == element)
                value = QLatin1String("{searchTerms}");

            searchUrl.addQueryItem(name, value);
        } else if (type == QLatin1String("checkbox") || type == QLatin1String("radio")) {
            if (inputField.evaluateJavaScript(QLatin1String("this.checked")).toBool()) {
                searchUrl.addQueryItem(name, value);
            }
        } else if (type == QLatin1String("hidden")) {
            searchUrl.addQueryItem(name, value);
        }
    }

    QWebElementCollection selectFields = formElement.findAll(QLatin1String("select"));
    foreach (QWebElement selectField, selectFields) {
        QString name = selectField.attribute(QLatin1String("name"));
        int selectedIndex = selectField.evaluateJavaScript(QLatin1String("this.selectedIndex")).toInt();
        if (selectedIndex == -1)
            continue;

        QWebElementCollection options = selectField.findAll(QLatin1String("option"));
        QString value = options.at(selectedIndex).toPlainText();
        searchUrl.addQueryItem(name, value);
    }

    bool ok = true;
    if (searchEngines.count() > 1) {
        QString searchEngine = QInputDialog::getItem(this, tr("Search engine"),
                                                    tr("Choose the desired search engine"), searchEngines.keys(),
                                                    0, false, &ok);
        if (!ok)
            return;
        if (!searchEngines[searchEngine].isEmpty())
            searchUrl.addQueryItem(searchEngines[searchEngine], searchEngine);
    }

    QString engineName;
    QWebElementCollection labels = formElement.findAll(QString(QLatin1String("label[for=\"%1\"]")).arg(elementName));
    if (labels.count() > 0)
        engineName = labels.at(0).toPlainText();
    qDebug("#0# name=%s\n", Q2A(engineName) );

    /*
    engineName = QInputDialog::getText(this, tr("Engine name"), tr("Type in a name for the engine"),
                                       QLineEdit::Normal, engineName, &ok);
    if (!ok)
        return;
    OpenSearchEngine *engine = new OpenSearchEngine();
    engine->setName(engineName);
    engine->setDescription(engineName);
    engine->setSearchUrlTemplate(searchUrl.toString());
    engine->setImage(icon().pixmap(16, 16).toImage());
    ToolbarSearch::openSearchManager()->addEngine(engine);
    */
}
#endif

void WebView::setProgress(int progress)
{
    m_progress = progress;
    StrVar* data=getResult();
    data->setVar('0',0).addInt(progress);
    callback("progress", data);
}

int WebView::levelForZoom(int zoom)
{
    int i;
    i = m_zoomLevels.indexOf(zoom);
    if (i >= 0)
        return i;

    for (i = 0 ; i < m_zoomLevels.count(); ++i)
        if (zoom <= m_zoomLevels[i])
            break;

    if (i == m_zoomLevels.count())
        return i - 1;
    if (i == 0)
        return i;

    if (zoom - m_zoomLevels[i-1] > m_zoomLevels[i] - zoom)
        return i;
    else
        return i-1;
}

void WebView::slotLinkClicked(QUrl url) {
    StrVar* sv=config()->GetVar("@linkClickUse");
    if( SVCHK('3',0) ) {
        qDebug()<<"slotLinkClicked =========== "<<url<<"\n";
    } else {
        QString pageUrl = url.toString();
        StrVar* data=getResult();
        data->add(Q2A(pageUrl));
        callback("linkClick", data);
        load(url);
    }
}

void WebView::applyZoom() {
    StrVar* data=getResult();
    data->setVar('0',0).addInt(m_currentZoom);
    callback("zoom", data);
    setZoomFactor(qreal(m_currentZoom) / 100.0);
}

void WebView::zoomIn() {
    int i = levelForZoom(m_currentZoom);
    if (i < m_zoomLevels.count() - 1)
        m_currentZoom = m_zoomLevels[i + 1];
    applyZoom();
}

void WebView::zoomOut() {
    int i = levelForZoom(m_currentZoom);
    if (i > 0)
        m_currentZoom = m_zoomLevels[i - 1];
    applyZoom();
}

void WebView::resetZoom() {
    m_currentZoom = 100;
    applyZoom();
}

void WebView::setZoom(int zoom) {
    int i=levelForZoom(zoom);
    if( i>=0 && i<m_zoomLevels.count() ) {
        m_currentZoom = m_zoomLevels[i];
        applyZoom();
    }
}

void WebView::loadFinished()
{
    if (100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
    StrVar* data=getResult();
    data->add( Q2A(url().toString()) );
    if( xlistFind.size() ) {
        xlistFind.clear();
    }
    callback("finish", data);
}

void WebView::loadUrl(const QUrl &url, const QString &title) {
    m_initialUrl = url;
    if (!title.isEmpty())
        emit titleChanged(tr("Loading..."));
    else
        emit titleChanged(title);
    xlistFind.clear();
    m_progress = 0;
    callback("start");
    load(url);
}

void WebView::loadPage(const QUrl &url, QNetworkDiskCache* cache) {
    /*
    if( url.scheme() == QLatin1String("javascript") ) {
        QString scriptSource = QUrl::fromPercentEncoding(url.toString(Q_FLAGS(QUrl::TolerantMode|QUrl::RemoveScheme)).toAscii());
        QVariant result = page()->mainFrame()->evaluateJavaScript(scriptSource);
        return;
    }
    */
    page()->networkAccessManager()->setCache(cache);
    m_initialUrl = url;
    xlistFind.clear();
    m_progress = 0;
    callback("start");
    load(url);
}

QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}

QUrl WebView::url() const {
    QUrl url = QWebView::url();
    if (!url.isEmpty())
        return url;
    return m_initialUrl;
}

void WebView::mousePressEvent(QMouseEvent *event)
{
    // BrowserApplication::instance()->setEventMouseButtons(event->buttons());
    // BrowserApplication::instance()->setEventKeyboardModifiers(event->modifiers());

    switch (event->button()) {
    case Qt::XButton1:
        pageAction(WebPage::Back)->trigger();
        break;
    case Qt::XButton2:
        pageAction(WebPage::Forward)->trigger();
        break;
    default:
        config()->GetVar("@mpos")->setVar('i',2).addInt(event->pos().x()).addInt(event->pos().y());
        QWebView::mousePressEvent(event);
        break;
    }
}

void WebView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void WebView::dragMoveEvent(QDragMoveEvent *event)
{
    event->ignore();
    if (event->source() != this) {
        if (!event->mimeData()->urls().isEmpty()) {
            event->acceptProposedAction();
        } else {
            QUrl url(event->mimeData()->text());
            if (url.isValid())
                event->acceptProposedAction();
        }
    }
    if (!event->isAccepted()) {
        QWebView::dragMoveEvent(event);
    }
}

void WebView::dropEvent(QDropEvent *event)
{
    QWebView::dropEvent(event);
    if (!event->isAccepted()
        && event->source() != this
        && event->possibleActions() & Qt::CopyAction) {

        QUrl url;
        if (!event->mimeData()->urls().isEmpty())
            url = event->mimeData()->urls().first();
        if (!url.isValid())
            url = event->mimeData()->text();
        if (url.isValid()) {
            loadUrl(url);
            event->acceptProposedAction();
        }
    }
}
void WebView::hitTest(QPoint pt, StrVar* rst) {
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(pt);
    QString link = r.linkUrl().toString();
    rst->reuse();
    if( !link.isNull() ) {
        if( rst ) {
            rst->set(Q2A(link));
        } else {
            StrVar* data=getResult();
            data->add(Q2A(link));
            callback("hitTest", data);
        }
    }
}
void WebView::sendMouseDownEvent(QPoint pt, StrVar* rst) {
    QMouseEvent pevent(QEvent::MouseButtonPress, pt, Qt::LeftButton, 0, 0);
    qApp->sendEvent(this, &pevent);
    QMouseEvent revent(QEvent::MouseButtonRelease, pt, Qt::LeftButton, 0, 0);
    qApp->sendEvent(this, &revent);
    if( rst ) rst->setVar('3',1);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    const bool isAccepted = event->isAccepted();
    m_page->event(event);
    if( event->button() == Qt::LeftButton ) {
        StrVar* sv = config()->GetVar("@mpos");
        if( SVCHK('i',2) ) {
            QRect rc(sv->getInt(4),sv->getInt(8),8,8);
            if( rc.contains(event->pos()) ) {
                sv=config()->gv("@linkClickUse");
                if( SVCHK('3',0) ) {
                    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
                    QString link = r.linkUrl().toString();
                    if( !link.isNull() ) {
                        StrVar* data=getResult();
                        data->add(Q2A(link));
                        callback("linkClick", data);
                    }
                    event->setAccepted(false);
                    return;
                }
            }
        }
    }
    /*
    if (!event->isAccepted()
        && (BrowserApplication::instance()->eventMouseButtons() & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            BrowserApplication::instance()->setEventMouseButtons(Qt::NoButton);
            loadUrl(url);
        }
    }
    */
    event->setAccepted(isAccepted);
}

void WebView::setStatusBarText(const QString &string)
{
    if( !string.isNull() ) {
        StrVar* data=getResult();
        data->add(Q2A(string));
        callback("statusText", data);
    }
    m_statusBarText = string;
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    qDebug("download request\n");
    // BrowserApplication::downloadManager()->download(request);
}

void WebView::keyPressEvent(QKeyEvent *event)
{
#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
    if( m_enableAccessKeys) {
        m_accessKeysPressed = (event->modifiers() == Qt::ControlModifier
            && event->key() == Qt::Key_Control);
        if (!m_accessKeysPressed) {
            if( checkForAccessKey(event)) {
                hideAccessKeys();
                event->accept();
                return;
            }
            hideAccessKeys();
        } else {
            QTimer::singleShot(300, this, SLOT(accessKeyShortcut()));
        }
    }
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
#endif

#if QT_VERSION < 0x040600
    switch (event->key()) {
    case Qt::Key_Back:
        pageAction(WebPage::Back)->trigger();
        event->accept();
        break;
    case Qt::Key_Forward:
        pageAction(WebPage::Forward)->trigger();
        event->accept();
        break;
    case Qt::Key_Stop:
        pageAction(WebPage::Stop)->trigger();
        event->accept();
        break;
    case Qt::Key_Refresh:
        pageAction(WebPage::Reload)->trigger();
        event->accept();
        break;
    default:
        QWebView::keyPressEvent(event);
        return;
    }
#endif
    QWebView::keyPressEvent(event);
}

#if QT_VERSION >= 0x040600 || defined(WEBKIT_TRUNK)
void WebView::accessKeyShortcut()
{
    if (!hasFocus()
        || !m_accessKeysPressed
        || !m_enableAccessKeys)
        return;
    if (m_accessKeyLabels.isEmpty()) {
        showAccessKeys();
    } else {
        hideAccessKeys();
    }
    m_accessKeysPressed = false;
}

void WebView::keyReleaseEvent(QKeyEvent *event)
{
    if (m_enableAccessKeys)
        m_accessKeysPressed = event->key() == Qt::Key_Control;
    QWebView::keyReleaseEvent(event);
}

void WebView::focusOutEvent(QFocusEvent *event)
{
    if (m_accessKeysPressed) {
        hideAccessKeys();
        m_accessKeysPressed = false;
    }
    QWebView::focusOutEvent(event);
}

bool WebView::checkForAccessKey(QKeyEvent *event)
{
    if (m_accessKeyLabels.isEmpty())
        return false;

    QString text = event->text();
    if (text.isEmpty())
        return false;
    QChar key = text.at(0).toUpper();
    bool handled = false;
    if (m_accessKeyNodes.contains(key)) {
        QWebElement element = m_accessKeyNodes[key];
        QPoint p = element.geometry().center();
        QWebFrame *frame = element.webFrame();
        Q_ASSERT(frame);
        do {
            p -= frame->scrollPosition();
            frame = frame->parentFrame();
        } while (frame && frame != page()->mainFrame());

        QMouseEvent pevent(QEvent::MouseButtonPress, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(this, &pevent);
        QMouseEvent revent(QEvent::MouseButtonRelease, p, Qt::LeftButton, 0, 0);
        qApp->sendEvent(this, &revent);
        handled = true;
    }
    return handled;
}

void WebView::hideAccessKeys()
{
    if (!m_accessKeyLabels.isEmpty()) {
        for (int i = 0; i < m_accessKeyLabels.count(); ++i) {
            QLabel *label = m_accessKeyLabels[i];
            label->hide();
            label->deleteLater();
        }
        m_accessKeyLabels.clear();
        m_accessKeyNodes.clear();
        update();
    }
}

void WebView::tagInfo(LPCC tag, LPCC attr, TreeNode* root, bool isView ) {
    QRect viewport = QRect(m_page->mainFrame()->scrollPosition(), m_page->viewportSize());
    // Priority first goes to elements with accesskey attributes
    StrVar* sv=NULL;
    QString tagName=KR(tag);
    QList<QWebElement> result = page()->mainFrame()->findAllElements(tagName).toList();
    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c)
        unusedKeys << QLatin1Char(c);
    for (char c = '0'; c <= '9'; ++c)
        unusedKeys << QLatin1Char(c);

    QList<QWebElement> alreadyLabeled;
    foreach (const QWebElement &element, result) {
        const QRect geometry = element.geometry();
        if( geometry.size().isEmpty() ) {
            continue;
        }
        if( isView && !viewport.contains(geometry.topLeft()) ) {
            continue;
        }
        QString value = ccmp(attr,"text") ? element.toPlainText(): element.attribute(KR(attr));
        if( slen(attr) && value.isEmpty() ) {
            continue;
        }
        int idx=xlistFind.indexOf(element);
        if( idx==-1 ) {
            idx=xlistFind.size();
            xlistFind.append(element);
        }
        TreeNode* cur=root->addNode();
        cur->GetVar("key")->fmt("%d", idx);
        cur->GetVar("geometry")->setVar('i',4).addInt(geometry.x()).addInt(geometry.y()).addInt(geometry.width()).addInt(geometry.height());
        if( slen(attr) ) {
            cur->GetVar(attr)->set(Q2A(value));
        }
        if( isView ) {
            QString accessKeyAttribute = element.attribute(QLatin1String("accesskey")).toUpper();
            if (accessKeyAttribute.isEmpty())
                continue;
            QChar accessKey;
            for (int i = 0; i < accessKeyAttribute.count(); i+=2) {
                const QChar &possibleAccessKey = accessKeyAttribute[i];
                if (unusedKeys.contains(possibleAccessKey)) {
                    accessKey = possibleAccessKey;
                    break;
                }
            }
            if (accessKey.isNull())
                continue;
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
            if( cur ) {
                QString text=element.toPlainText();
                QString key=QString(QLatin1String("<b>%1</b>")).arg(accessKey);
                cur->set("elementText", Q2A(text));
                cur->set("accessKey", Q2A(key));
            }
            alreadyLabeled.append(element);
        }
    }
    if( isView ) {
        QWebElementCollection result = page()->mainFrame()->findAllElements(tagName);
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if( unusedKeys.isEmpty()
                || alreadyLabeled.contains(element)
                || geometry.size().isEmpty()
                || !viewport.contains(geometry.topLeft())) {
                continue;
            }
            QChar accessKey;
            QString text = element.toPlainText().toUpper();
            for (int i = 0; i < text.count(); ++i) {
                const QChar &c = text.at(i);
                if (unusedKeys.contains(c)) {
                    accessKey = c;
                    break;
                }
            }
            if (accessKey.isNull())
                accessKey = unusedKeys.takeFirst();
            unusedKeys.removeOne(accessKey);
            int idx=xlistFind.indexOf(element);
            if( idx!=-1 ) {
                char key[16];
                sprintf(key, "%d", idx);
                TreeNode* cur=NULL;
                for( int n=0;n<root->childCount();n++ ) {
                    TreeNode* node=root->child(n);
                    if( node && ccmp(key,node->get("key")) ) {
                        cur=node;
                        break;
                    }
                }
                if( cur ) {
                    QString key=QString(QLatin1String("<b>%1</b>")).arg(accessKey);
                    cur->set("elementText", Q2A(text));
                    cur->set("accessKey", Q2A(key));
                }
            }
            makeAccessKeyLabel(accessKey, element);
        }
    }
}

void WebView::findAll(int index, LPCC query, LPCC attr, XListArr* arr ) {
    if( index==-1 ) xlistFind.clear();
    QWebElement parent = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : page()->mainFrame()->documentElement();
    QWebElementCollection collection = parent.findAll( KR(query) );
    foreach( QWebElement el, collection ) {
        int idx=xlistFind.indexOf(el);
        if( idx==-1 ) {
            idx=xlistFind.size();
            xlistFind.append(el);
        }
        QString value = ccmp(attr,"text") ? el.toPlainText(): el.attribute(KR(attr));
        if( slen(attr) && value.isEmpty() ) {
            continue;
        }
        if( value.isEmpty() ) {
            value=el.tagName();
        }
        StrVar* rst=arr->add();
        rst->setVar('x',21);
        rst->format(8,"%d",idx);
        rst->add("\f").add(Q2A(value));
    }
}

void WebView::findChild(int index, LPCC query, LPCC attr, XListArr* arr ) {
    QWebElement parent = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : page()->mainFrame()->documentElement();

    for( QWebElement el=parent.firstChild(); el.isNull() ; el=el.nextSibling() ) {
        int idx=xlistFind.indexOf(el);
        if( idx==-1 ) {
            idx=xlistFind.size();
            xlistFind.append(el);
        }
        QString value = ccmp(attr,"text") ? el.toPlainText(): el.attribute(KR(attr));
        if( slen(attr) && value.isEmpty() ) {
            continue;
        }
        if( value.isEmpty() ) {
            value=el.tagName();
        }
        StrVar* rst=arr->add();
        rst->setVar('x',21);
        rst->format(8,"%d",idx);
        rst->add("\f").add(Q2A(value));
    }
}

void WebView::findParentTag(int index, LPCC tag, LPCC attr, StrVar* rst ) {
    QWebElement el = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : page()->mainFrame()->documentElement();
    QWebElement parent = el;
    if( slen(tag)==0 ) {
        parent = parent.parent();
    } else {
        QString tagLower=KR(tag).toLower();
        for( int n=0; (n < 256) && ( parent.tagName().toLower() != tagLower ) ; n++ ) {
            if( parent.isNull() )
                return;
            parent = parent.parent();
        }
    }
    QString value = ccmp(attr,"text") ? parent.toPlainText(): parent.attribute(KR(attr));
    if( slen(attr) && value.isEmpty() ) {
        return;
    }
    int idx=xlistFind.indexOf(parent);
    if( idx==-1) {
        idx=xlistFind.size();
        xlistFind.append(parent);
    }
    rst->setVar('x',21);
    rst->format(8,"%d",idx);
    rst->add("\f");
    if( value.isEmpty() ) {
        rst->add(tag);
    } else {
        rst->add(Q2A(value));
    }
}

bool WebView::getElementInfo(int index, TreeNode* node) {
    QWebElement el = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : QWebElement();
    if( el.isNull() || node==NULL ) return false;

    QRect geometry=el.geometry();
    node->set("tag", Q2A(el.tagName()));
    node->set("text",Q2A(el.toPlainText()));
    node->GetVar("geometry")->setVar('i',4).addInt(geometry.x()).addInt(geometry.y()).addInt(geometry.width()).addInt(geometry.height());
    foreach( QString attr, el.attributeNames() ) {
        node->set( Q2A(attr), Q2A(el.attribute(attr)) );
    }
    return true;
}

bool WebView::setElementAttribute(int index, LPCC name, StrVar* data) {
    QWebElement el = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : QWebElement();
    if( el.isNull() || slen(name)==0 ) {
        return false;
    }
    LPCC val=toString(data);
    el.setAttribute(KR(name), KR(val));
    return true;
}

bool WebView::setElementValue(int index, LPCC data, StrVar* rst) {
    QWebElement el = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : QWebElement();
    if( el.isNull() || slen(data)==0 ) {
        return false;
    }
    QString value=el.evaluateJavaScript(KR(data) ).toString();
    rst->set(Q2A(value));
    qDebug("#0#setElementValue %s (result:%s)\n", data, rst->get() );
    return true;
}

bool WebView::renderElement(int index, LPCC type, LPCC output, StrVar* sv) {
    QWebElement el = ( index!=-1 && index<xlistFind.size() ) ? xlistFind.at(index) : QWebElement();
    if( el.isNull() ) {
        if( ccmp(type,"pdf") ) {
            QPrinter printer;
            printer.setPageSize(QPrinter::A4);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setPageMargins (15,15,15,15,QPrinter::Millimeter);
            printer.setFullPage( false );
            printer.setOutputFileName(KR(output));
            page()->mainFrame()->print(&printer);
        } else {
            QPainter painter;
            QImage image(page()->viewportSize(), QImage::Format_ARGB32);
            painter.begin(&image);
            if( SVCHK('i',4) ) {
                QRegion range(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16) );
                page()->mainFrame()->render(&painter, range);
            } else {
                page()->mainFrame()->render(&painter);
            }
            if( slen(output) ) {
                image.save(output,"JPG");
            } else {
                QByteArray ba;
                QBuffer buf(&ba);
                buf.open(QIODevice::WriteOnly );
                image.save(&buf,"PNG");
                sv->setVar('i',7).addInt(ba.length()).add( ba.constData(), ba.size() );
            }
            painter.end();
        }
    } else {
        QImage image;
        QPainter painter;
        painter.begin(&image);
        if( SVCHK('i',4) ) {
            int w=sv->getInt(12),h=sv->getInt(16);
            image=QImage(QSize(w,h), QImage::Format_ARGB32);
            el.render(&painter, QRect(sv->getInt(4),sv->getInt(8),w,h ));
        } else {
            image=QImage(page()->viewportSize(), QImage::Format_ARGB32);
            el.render(&painter);
        }
        if( slen(output) ) {
            image.save(output,"JPG");
        } else {
            QByteArray ba;
            QBuffer buf(&ba);
            buf.open(QIODevice::WriteOnly );
            image.save(&buf,"JPG");
            sv->setVar('i',7).addInt(ba.length()).add( ba.constData(), ba.size() );
        }
        painter.end();
    }
    return true;
}

void WebView::showAccessKeys() {
    QStringList supportedElement;
    supportedElement << QLatin1String("input")
                     << QLatin1String("a")
                     << QLatin1String("area")
                     << QLatin1String("button")
                     << QLatin1String("label")
                     << QLatin1String("legend")
                     << QLatin1String("textarea");

    QList<QChar> unusedKeys;
    for (char c = 'A'; c <= 'Z'; ++c)
        unusedKeys << QLatin1Char(c);
    for (char c = '0'; c <= '9'; ++c)
        unusedKeys << QLatin1Char(c);

    QRect viewport = QRect(m_page->mainFrame()->scrollPosition(), m_page->viewportSize());
    // Priority first goes to elements with accesskey attributes
    QList<QWebElement> alreadyLabeled;
    foreach (const QString &elementType, supportedElement) {
        QList<QWebElement> result = page()->mainFrame()->findAllElements(elementType).toList();
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if( geometry.size().isEmpty() || !viewport.contains(geometry.topLeft()) ) {
                continue;
            }
            QString accessKeyAttribute = element.attribute(QLatin1String("accesskey")).toUpper();
            if (accessKeyAttribute.isEmpty())
                continue;
            QChar accessKey;
            for (int i = 0; i < accessKeyAttribute.count(); i+=2) {
                const QChar &possibleAccessKey = accessKeyAttribute[i];
                if (unusedKeys.contains(possibleAccessKey)) {
                    accessKey = possibleAccessKey;
                    break;
                }
            }
            if (accessKey.isNull())
                continue;
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
            alreadyLabeled.append(element);
        }
    }

    // Pick an access key first from the letters in the text and then from the
    // list of unused access keys

    foreach (const QString &elementType, supportedElement) {
        QWebElementCollection result = page()->mainFrame()->findAllElements(elementType);
        foreach (const QWebElement &element, result) {
            const QRect geometry = element.geometry();
            if (unusedKeys.isEmpty()
                || alreadyLabeled.contains(element)
                || geometry.size().isEmpty()
                || !viewport.contains(geometry.topLeft())) {
                continue;
            }
            QChar accessKey;
            QString text = element.toPlainText().toUpper();
            for (int i = 0; i < text.count(); ++i) {
                const QChar &c = text.at(i);
                if (unusedKeys.contains(c)) {
                    accessKey = c;
                    break;
                }
            }
            if (accessKey.isNull())
                accessKey = unusedKeys.takeFirst();
            unusedKeys.removeOne(accessKey);
            makeAccessKeyLabel(accessKey, element);
        }
    }
}

void WebView::makeAccessKeyLabel(const QChar &accessKey, const QWebElement &element)
{
    QLabel *label = new QLabel(this);
    label->setText(QString(QLatin1String("<qt><b>%1</b>")).arg(accessKey));

    QPalette p = QToolTip::palette();
    QColor color(Qt::yellow);
    color = color.lighter(150);
    color.setAlpha(175);
    p.setColor(QPalette::Window, color);
    label->setPalette(p);
    label->setAutoFillBackground(true);
    label->setFrameStyle(QFrame::Box | QFrame::Plain);
    QPoint point = element.geometry().center();
    point -= m_page->mainFrame()->scrollPosition();
    label->move(point);
    label->show();
    point.setX(point.x() - label->width() / 2);
    label->move(point);
    m_accessKeyLabels.append(label);
    m_accessKeyNodes[accessKey] = element;
}

#endif

#endif

