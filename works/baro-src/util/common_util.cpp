#include "widget_util.h"
#include "draw_util.h"
#include <QApplication>

QIcon gGlobalIcon;
QColor gclr;
StrVar gclrVar;
TreeNode	gFontNode;
TreeNode	gPixmapNode;

inline int str_cmp(LPCC a, LPCC b) {
    while( true ) {
        if( *a==0 || *b==0 || tolower((unsigned char) *a) != tolower((unsigned char) *b))
            return *a-*b;
        a++;
        b++;
    }
    return 0;
}
inline bool args_cmp(LPCC args, LPCC val) {
    int i=0, n=0;
    char buf[64];
    while( ISBLANK(args[i]) ) i++;

    while(args[i]) {
        if(args[i]==',') {
          buf[n]=0;
          if( ccmp(buf, val) ) return true;
          n=0;
        } else if(args[i]=='\'') {
          i+=2;
        } else {
            if( n>60 ) n--;
            if(args[i]=='\"') {
                i++;
                while(args[i]) {
                    if(args[i+1]=='\"' && args[i]!='\\') {
                        buf[n++]=args[i++];
                        i++;
                        break;
                    }
                    buf[n++]=args[i++];
                }
            } else {
              buf[n++]=args[i++];
            }
        }
    }
    if( n>0 ) {
        buf[n]=0;
        if( ccmp(buf, val) ) return true;
    }
    return false;
}

bool cf_eq(TreeNode *cf, LPCC field, LPCC args) {
    return args_cmp(toString(cf->gv(field)), args);
}
LPCC getFullFileName(LPCC fileName) {
    int len=slen(fileName);
    if( len==0 ) return NULL;
    if( len>2 && (fileName[0]=='/'||fileName[1]==':') ) {
        return fileName;
    }
    // if( fileName[0]=='/' ) fileName++;
    return FMT("%s/%s", Q2A(QApplication::applicationDirPath()), fileName);
}

XListArr* convertArray(TreeNode* cf, LPCC field, LPCC sep) {
    StrVar* sv=cf->GetVar(field);
    if( SVCHK('a',0) ) return (XListArr*)SVO;
    if( slen(sep) ) sep=",";
    XParseVar pv(sv);
    XListArr* arr=cf->addArray(field, true);
    while( pv.valid() ) {
        arr->add()->set( pv.findEnd(sep).v() );
    }
    return arr;
}

int firstNonSpace(const QString& text)  {
    int i = 0;
    while (i < text.size()) {
        if( !text.at(i).isSpace())
            return i;
        ++i;
    }
    return i;
}

time_t getTm() {
    time_t tt;
    time( &tt);
    localtime(&tt);
    return tt;
}

int confLoad(LPCC fileName, LPCC lang, bool overwite) {
    int num=0;
    StrVar* rst=cfVar("temp", true);
    if( !isFile(fileName) ) fileName=getSaveFileName(fileName);
    fileReadAll(fileName, rst);
    if( rst->length()==0 ) return num;
    TreeNode* node=getCf()->addNode("temp")->reuseAll(true);
    XParseVar pv(rst);
    node->setJson(pv);
    if( slen(lang) ) cfVar("lang")->set(lang);
    StrVar* sv=NULL;
    for( WBoxNode* bn=node->First(); bn; bn=node->Next() ) {
        LPCC grp=node->getCurCode();
        sv=&(bn->value);
        if( SVCHK('n',0) ) {
            TreeNode* sub=(TreeNode*)SVO;
            for( WBoxNode* bn0=sub->First(); bn0; bn0=sub->Next() ) {
                LPCC cd=sub->getCurCode(), confCode=FMT("%s.%s",grp,cd);
                confVar( confCode, &(bn0->value), overwite);
                num++;
            }
        }
    }
    return num;
}

TreeNode* getFileInfoList(LPCC path, TreeNode* root, LPCC fields, LPCC sort, LPCC kind, LPCC filter) {
    QFileInfoList fi;
    QDir dirs(KR(path));
    U32 sortFlag=0, kindFlag=0;
    StrVar* sv=cfVar("@fileInfo",true);
    XListArr* fieldsArr=getCf()->addArray("@fileInfo", true);
    if( root==NULL ) {
        root=getCf()->addNode("@fileInfo", true);
    }
    if( slen(fields)==0 ) {
        fields="type, name, size";
    }
    sv->set(fields);
    XParseVar pv(sv,0,sv->length() );
    while( pv.valid() ) {
        fieldsArr->add()->set( pv.findEnd(",").v() );
    }
    if( slen(sort) ) {
        sv->set(sort);
        pv.SetVar(sv,0,sv->length() );
        while( pv.valid() ) {
            LPCC type=pv.findEnd(",").v();
            if( ccmp(type,"name") ) sortFlag|=QDir::Name;
            else if( ccmp(type,"time") ) sortFlag|=QDir::Time;
            else if( ccmp(type,"size") ) sortFlag|=QDir::Size;
            else if( ccmp(type,"type") ) sortFlag|=QDir::Type;
            else if( ccmp(type,"desc") ) sortFlag|=QDir::Reversed;
            else if( ccmp(type,"case") ) sortFlag&=~QDir::IgnoreCase;
        }
    } else {
        sortFlag=QDir::IgnoreCase;
    }
    if( slen(kind) ) {
        sv->set(kind);
        pv.SetVar(sv,0,sv->length() );
        while( pv.valid() ) {
            LPCC type=pv.findEnd(",").v();
            if( ccmp(type,"folders") ) kindFlag|=QDir::Dirs;
            else if( ccmp(type,"all") ) kindFlag|=QDir::AllDirs|QDir::Files;
            else if( ccmp(type,"allEntry") ) kindFlag|=QDir::AllEntries;
            else if( ccmp(type,"files") ) kindFlag|=QDir::Files;
            else if( ccmp(type,"drivers") ) kindFlag|=QDir::Drives;
            else if( ccmp(type,"readable") ) kindFlag|=QDir::Readable;
            else if( ccmp(type,"writable") ) kindFlag|=QDir::Writable;
            else if( ccmp(type,"executable") ) kindFlag&=~QDir::Executable;
            else if( ccmp(type,"hidden") ) kindFlag|=QDir::Hidden;
            else if( ccmp(type,"system") ) kindFlag|=~QDir::System;
        }
    } else {
        kindFlag=QDir::NoDotAndDotDot|QDir::AllEntries;
    }
    if( slen(filter) ) {
        QStringList sa;
        sv->set(filter);
        pv.SetVar(sv,0,sv->length() );
        while( pv.valid() ) {
            LPCC str=pv.findEnd(",").v();
            sa.append(KR(str));
        }
        dirs.setNameFilters(sa);
    }
    dirs.setFilter((QDir::Filters)kindFlag);
    dirs.setSorting((QDir::SortFlags)sortFlag);
    int sz= fieldsArr ? fieldsArr->size(): 0;
    for(int n=0; n<fi.size(); n++ ) {
        QFileInfo info=fi.at(n);
        TreeNode* cur=root->addNode();
        for( int x=0; x<sz; n++ ) {
            sv=fieldsArr->get(x);
            if( svcmp(sv,"type") ) cur->set("type", info.isFile()? "file": "folder" );
            else if( svcmp(sv,"name") ) cur->set("name", Q2A(info.fileName()) );
            else if( svcmp(sv,"fullPath") ) cur->set("fullPath", Q2A(info.absolutePath()) );
            else if( svcmp(sv,"relativePath") ) cur->set("fullPath", Q2A(info.canonicalFilePath()) );
            else if( svcmp(sv,"ext") ) cur->set("ext", Q2A(info.suffix()) );
            else if( svcmp(sv,"size") ) cur->setLong("size", info.size() );
            else if( svcmp(sv,"modify") ) cur->setLong("modify", info.lastModified().toLocalTime().toTime_t() );
            else if( svcmp(sv,"created") ) cur->setLong("created", info.created().toLocalTime().toTime_t() );
        }
    }
    return root;
}


LPCC getUri(LPCC uri, StrVar* rst, int interval, StrVar* err) {
    QUrl url(KR(uri));
    QNetworkAccessManager nam;
    QNetworkReply* reply=nam.get(QNetworkRequest(url));
    if( rst==NULL ) rst=cfVar("@result", true);
    if( err==NULL ) err=cfVar("@error", true);
    if( reply ) {
        QEventLoop loop;
        QTimer timer;
        timer.setInterval(interval);
        timer.setSingleShot(true);
        loop.connect(&timer, SIGNAL(timeout()), SLOT(quit()));
        loop.connect(reply, SIGNAL(finished()), SLOT(quit()) );
        timer.start();
        loop.exec();
        if( timer.isActive() ) {
            timer.stop();
        }
        if( reply->error()==QNetworkReply::NoError ) {
            int length = reply->rawHeader("Content-Length").toInt();
            QByteArray ba=reply->readAll();
            if( ba.size()!=length ) {
                err->fmt("size not match (Content-Length:%d, read:%d)", length, ba.length() );
            }
            rst->add(ba.constData(), ba.size() );
            return rst->get();
        } else {
            QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            getUriError(reply->error(), status.toInt(), err );
        }
    } else {
        err->fmt("URI Request error %s", uri);
    }
    return rst->get();
}

void getUriError(QNetworkReply::NetworkError code, int status, StrVar *err)
{
    switch(code ) {
    case QNetworkReply::ConnectionRefusedError :			err->add("ConnectionRefusedError"); break;
    case QNetworkReply::RemoteHostClosedError :				err->add("RemoteHostClosedError"); break;
    case QNetworkReply::HostNotFoundError :					err->add("HostNotFoundError"); break;
    case QNetworkReply::TimeoutError :						err->add("TimeoutError"); break;
    case QNetworkReply::OperationCanceledError :			err->add("OperationCanceledError"); break;
    case QNetworkReply::SslHandshakeFailedError :			err->add("SslHandshakeFailedError"); break;
    case QNetworkReply::TemporaryNetworkFailureError :		err->add("TemporaryNetworkFailureError"); break;
    case QNetworkReply::UnknownNetworkError :				err->add("UnknownNetworkError"); break;
    case QNetworkReply::ProxyConnectionRefusedError :		err->add("ProxyConnectionRefusedError"); break;
    case QNetworkReply::ProxyConnectionClosedError :		err->add("ProxyConnectionClosedError"); break;
    case QNetworkReply::ProxyNotFoundError :				err->add("ProxyNotFoundError"); break;
    case QNetworkReply::ProxyTimeoutError :					err->add("ProxyTimeoutError"); break;
    case QNetworkReply::ProxyAuthenticationRequiredError :	err->add("ProxyAuthenticationRequiredError"); break;
    case QNetworkReply::UnknownProxyError :					err->add("UnknownProxyError"); break;
    case QNetworkReply::ContentAccessDenied :				err->add("ContentAccessDenied"); break;
    case QNetworkReply::ContentOperationNotPermittedError : err->add("ContentOperationNotPermittedError"); break;
    case QNetworkReply::ContentNotFoundError :				err->add(""); break;
    case QNetworkReply::AuthenticationRequiredError :		err->add("AuthenticationRequiredError"); break;
    case QNetworkReply::ContentReSendError :				err->add("ContentReSendError"); break;
    case QNetworkReply::UnknownContentError :				err->add("UnknownContentError"); break;
    case QNetworkReply::ProtocolUnknownError :				err->add("ProtocolUnknownError"); break;
    case QNetworkReply::ProtocolInvalidOperationError :		err->add("ProtocolInvalidOperationError"); break;
    case QNetworkReply::ProtocolFailure :					err->add("ProtocolFailure"); break;
    default: err->add("unknown error"); break;
    }
    err->format(64," ERROR (CODE: %d STATUS: %d)", code, status);
}
void setFontInfo(QFont* font) {
    if( font ) {
        gFontNode.GetVar("size")->setVar('0',0).addInt(font->pointSize());
        gFontNode.GetVar("weight")->set(font->bold()? "bold" : "normal");
        gFontNode.GetVar("name")->set( Q2A(font->family()) );
        // gFontNode.GetVar("italic")->setVar('3', font->italic()? 1: 0);
        // gFontNode.GetVar("spacing")->setVar('4',0).addDouble(font->letterSpacing());
        // gFontNode.GetVar("fixed")->setVar('3', font->fixedPitch()? 1: 0);
    }
}
TreeNode* fontInfoNode(QFont* font, TreeNode* node, LPCC fontInfo) {
    if( slen(fontInfo) ) {
        StrVar* var=cfVar("@fontInfo",true,fontInfo);
        // node->clearNodeFlag(FLAG_SKIP);
        node->reuse();
        node->setJson(var,0,var->length() );
    }
    StrVar* sv=node->gv("size");
    if( isNumberVar(sv) ) {
        font->setPointSize( toInteger(sv) );
    } else if(sv) {
        int pos=sv->findPos("px");
        if( pos>0 ) {
            LPCC num=SCPY(sv->get(),pos);
            if( isnum(num) ) {
                font->setPixelSize(atoi(num));
            }
        }
    }
    /*
    sv=node->gv("align");
    if( sv ) {
        cfVar("@fontAlign",true)->add(sv);
    }
    sv=node->gv("shadow");
    if( sv ) {
        cfVar("@fontShadow",true)->add(sv);
    }
    */
    sv=node->gv("name");
    if( sv==NULL ) sv=cfVar("@fontName");
    if( sv && sv->length() ) {
        LPCC name=toString(sv);
        font->setStyleStrategy(QFont::PreferAntialias);
        font->setFamily(KR(name));
    }
    sv=node->gv("weight");
    if( sv ) {
        if( isNumberVar(sv) ) {
            font->setWeight(toInteger(sv));
        } else {
            LPCC weight=toString(sv);
            font->setBold( ccmp(weight,"bold")? true: false );
        }
    }
    if( node->isNum("spacing") ) {
        font->setLetterSpacing(QFont::AbsoluteSpacing, node->getDouble("spacing") );
    }
    if( node->isNum("margin") ) {
        font->setWordSpacing( node->getDouble("margin") );
    }
    sv=node->gv("italic");
    if( SVCHK('3',1) ) {
        font->setItalic(isVarTrue(sv));
        sv->reuse();
    }
    sv=node->gv("strike");
    if( SVCHK('3',1) ) {
        font->setStrikeOut(isVarTrue(sv));
        sv->reuse();
    }
    sv=node->gv("fixed");
    if( SVCHK('3',1) ) {
        font->setFixedPitch(isVarTrue(sv));
        sv->reuse();
    }
    sv=node->gv("underline");
    if( SVCHK('3',1) ) {
        font->setUnderline(isVarTrue(sv));
        sv->reuse();
    }
    sv=node->gv("overline");
    if( SVCHK('3',1) ) {
        font->setOverline(isVarTrue(sv));
        sv->reuse();
    }
    sv=node->gv("cap");
    if( sv && sv->length() ) {
        LPCC cap=toString(sv);
        font->setCapitalization(
            ccmp(cap,"upper")? QFont::AllUppercase:
            ccmp(cap,"lower")?QFont::AllLowercase:
            ccmp(cap,"small")?QFont::SmallCaps: QFont::Capitalize );
        sv->reuse();
    }

    return node;
}
TreeNode* getFontInfo(QFont* font, LPCC fontInfo, QPainter* painter) {
    TreeNode* node=&gFontNode;
    if( font==NULL  ) {
        return node;
    }
    if( slen(fontInfo) ) {
        StrVar* var=cfVar("@fontInfo",true,fontInfo);
        StrVar* sv=node->gv("align"); if(sv) sv->reuse();
        sv=node->gv("color"); if(sv) sv->reuse();
        sv=node->gv("cap"); if(sv) sv->reuse();
        // node->gv("shadow"); if(sv) sv->reuse();
        node->clearNodeFlag(FLAG_SKIP);
        node->setJson(var,0,var->length() );
    }
    fontInfoNode(font, node);
    if( painter ) {
        StrVar* sv=node->gv("color");
        if( sv ) {
            QPen pen(getGlobalColor(sv), 1, Qt::SolidLine);
            painter->setPen(pen);
            cfVar("@fontColor")->reuse()->add(sv);
        }
    }
    return node;
}

void hsv2rgb(QColor& clr, double hh, double hs, double hv, int alpha) {
    double      ha, p, q, t, ff;
    long        i;
    double rr=0, rg=0, rb=0;

    if(hs <= 0.0) {       // < is bogus, just shuts up warnings
        rr = hv;
        rg = hv;
        rb = hv;
    } else {
        ha = hh;
        if(ha >= 360.0) ha = 0.0;
        ha /= 60.0;
        i = (long)ha;
        ff = ha - i;
        p = hv * (1.0 - hs);
        q = hv * (1.0 - (hs * ff));
        t = hv * (1.0 - (hs * (1.0 - ff)));

        switch(i) {
        case 0:
            rr = hv;
            rg = t;
            rb = p;
            break;
        case 1:
            rr = q;
            rg = hv;
            rb = p;
            break;
        case 2:
            rr = p;
            rg = hv;
            rb = t;
            break;

        case 3:
            rr = p;
            rg = q;
            rb = hv;
            break;
        case 4:
            rr = t;
            rg = p;
            rb = hv;
            break;
        case 5:
        default:
            rr = hv;
            rg = p;
            rb = q;
            break;
        }
    }
    clr.setAlpha(255);
    clr.setRed((int)(rr*255));
    clr.setGreen((int)(rg*255));
    clr.setBlue((int)(rb*255));
    if( alpha>=0 ) clr.setAlpha(alpha);
}

StrVar* GetColorValue(LPCC s, StrVar* rst ) {
    if( s==NULL || rst==NULL ) return rst;
    if(*s=='#')
        s++;

    int len = slen(s);
    if( len<3 ) {
        return rst;
    }
    if( len==3 || len==4 ) {
        char buf[4];
        buf[2] = 0;
        int r, g, b, a;
        r = g = b = a = 0;
        buf[0] = s[0];
        buf[1] = buf[0]=='0'?'0':'f';
        sscanf(toLower(buf),"%x",&r);
        buf[0] = s[1];
        buf[1] = buf[0]=='0'?'0':'f';
        sscanf(toLower(buf),"%x",&g);
        buf[0] = s[2];
        buf[1] = buf[0]=='0'?'0':'f';
        sscanf(toLower(buf),"%x",&b);
        rst->setVar('i',3).addInt(r).addInt(g).addInt(b);
        if( len==4 ) {
            buf[0] = s[3];
            buf[1] = buf[0]=='0'?'0':'f';
            sscanf(toLower(buf),"%x",&a);
            rst->addInt(a);
        }
    } else if( len==6|| len==8 ) {
        LPCC clr=NULL;
        char alpha[4]={0,};
        if( len>6 ) {
            clr=gBuf.add(s,6);
            alpha[0]=s[6];
            alpha[1]=s[7];
            alpha[2]=0;

        } else {
            clr=s;
        }
        U32 dec = (U32)strtol(clr, NULL, 16);
        int r = ((dec >> 16) & 0xFF);
        int g = ((dec >> 8) & 0xFF);
        int b = ((dec) & 0xFF);
        // qDebug("getQColor ####################### RGB(%d,%d,%d) \n", r, g, b);
        rst->setVar('i',3).addInt(r).addInt(g).addInt(b);
        if( len>6 ) {
            int a=(U32)strtol(alpha, NULL, 16);
            rst->addInt(a);
        }
    }
    return rst;
}

QColor getQColor(StrVar* sv ) {
    if( sv==NULL )
        return gclr;
    if( !SVCHK('i',3) ) {
        GetColorValue(sv->get(), sv);
    }

    if( SVCHK('i',3) ) {
        gclr.setRed(sv->getInt(4));
        gclr.setGreen(sv->getInt(8));
        gclr.setBlue(sv->getInt(12));
        if( sv->length()>16 ) {
            gclr.setAlpha(sv->getInt(16));
        } else if( gclr.alpha() < 0xFF ) {
            gclr.setAlpha(0xFF);
        }
    }
    return gclr;
}

QColor getGlobalColor(StrVar* sv, LPCC def) {
    if( sv==NULL ) {
        if( def ) {
            gclrVar.set(def);
            return getGlobalColor(&gclrVar);
        }
        return gclr;
    }
    if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
        return getQColor(sv);
    }
    // version 1.0.3
    XParseVar pv(sv);
    LPCC c=pv.NextWord();
    if( pv.ch()=='(' && pv.match("(",")") ) {
        // ex) hsv(50,100,50) or rgb() or rgba()
        XParseVar sub(sv, pv.prev, pv.cur);
        int r,g,b,a=-1;
        LPCC val=NULL;
        sub.findEnd(","); val=sub.v(); if(isnum(val) ) r=atoi(val);
        sub.findEnd(","); val=sub.v(); if(isnum(val) ) g=atoi(val);
        sub.findEnd(","); val=sub.v(); if(isnum(val) ) b=atoi(val);
        sub.findEnd(","); val=sub.v(); if(isnum(val) ) a=atoi(val);

        if( ccmp(c,"hsv") ) {
            hsv2rgb(gclr,(double)r, (double)g/100.0, (double)b/100.0,a);
        } else {
            gclr.setRed(r), gclr.setGreen(g), gclr.setBlue(b);
        }
        if( a>=0 ) {
            gclr.setAlpha(a);
        }
    } else {
        Qt::GlobalColor gc= ccmp(c, "black")? Qt::black :
        ccmp(c, "white")? Qt::white :
        ccmp(c, "darkGray")? Qt::darkGray :
        ccmp(c, "gray")? Qt::gray :
        ccmp(c, "lightGray")? Qt::lightGray :
        ccmp(c, "red")? Qt::red :
        ccmp(c, "green")? Qt::green :
        ccmp(c, "blue")? Qt::blue :
        ccmp(c, "cyan")? Qt::cyan :
        ccmp(c, "magenta")? Qt::magenta :
        ccmp(c, "yellow")? Qt::yellow :
        ccmp(c, "darkRed")? Qt::darkRed :
        ccmp(c, "darkGreen")? Qt::darkGreen :
        ccmp(c, "darkBlue")? Qt::darkBlue :
        ccmp(c, "darkCyan")? Qt::darkCyan :
        ccmp(c, "darkMagenta")? Qt::darkMagenta :
        ccmp(c, "darkYellow")? Qt::darkYellow : Qt::darkGray;
        gclr = gc;
    }
    if( sv && ccmp(def,"update") ) {
        sv->setVar('i',3).addInt(gclr.red()).addInt(gclr.green()).addInt(gclr.blue()).addInt(gclr.alpha());
    }
    return gclr;
}
QColor darkColor(const QColor& clr, int scale) {
    int ar=clr.red(), ag=clr.green(), ab=clr.blue();
    gclr.setRed(MulDiv(255-ar,scale,255)+ar);
    gclr.setGreen(MulDiv(255-ag,scale,255)+ag);
    gclr.setBlue(MulDiv(255-ab,scale,255)+ab);
    return gclr;
}
QColor lightColor(const QColor& clr, int scale) {
    int ar=clr.red(), ag=clr.green(), ab=clr.blue();
    gclr.setRed(MulDiv(ar,(255-scale),255));
    gclr.setGreen(MulDiv(ag,(255-scale),255));
    gclr.setBlue(MulDiv(ab,(255-scale),255));
    return gclr;
}
QColor difColor(const QColor& clr, int scale) {
    int ar=clr.red(), ag=clr.green(), ab=clr.blue();
    gclr.setRed(qAbs(ar-scale));
    gclr.setGreen(qAbs(ag-scale));
    gclr.setBlue(qAbs(ab-scale));
    return gclr;
}

QColor midColor(const QColor& clr1, const QColor& clr2) {
    int ar=clr1.red(), ag=clr1.green(), ab=clr1.blue();
    int br=clr2.red(), bg=clr2.green(), bb=clr2.blue();
    gclr.setRed(MulDiv(7,ar,10) + MulDiv(3,br,10));
    gclr.setGreen(MulDiv(7,ag,10) + MulDiv(3,bg,10));
    gclr.setBlue(MulDiv(7,ab,10) + MulDiv(3,bb,10));
    return gclr;
}
QColor mixColor(const QColor& clr1, const QColor& clr2) {
    int ar=clr1.red(), ag=clr1.green(), ab=clr1.blue();
    int br=clr2.red(), bg=clr2.green(), bb=clr2.blue();
    gclr.setRed(MulDiv(86,ar,100) + MulDiv(14,ar,100));
    gclr.setGreen(MulDiv(86,ag,100) + MulDiv(14,bg,100));
    gclr.setBlue(MulDiv(86,ab,100) + MulDiv(14,bb,100));
    return gclr;
}
QColor invertColor(const QColor& clr) {
    int ar=clr.red(), ag=clr.green(), ab=clr.blue();
    gclr.setRed(255 - ar);
    gclr.setGreen(255 - ag);
    gclr.setBlue(255 - ab);
    return gclr;
}
LPCC getImageFileName(LPCC fileName) {
    int len=slen(fileName);
    if( len==0 ) return NULL;
    if( len>2 && fileName[1]==':' ) {
        return fileName;
    }
    LPCC imagePath=NULL;
    if( fileName[0]=='/' ) {
        fileName++;
        imagePath=Q2A(QApplication::applicationDirPath());
    } else {
        int pos=IndexOf(fileName,'/');
        StrVar* var=NULL;
        if(pos>0) {
            LPCC name=gBuf.add(fileName,pos);
            fileName+=pos+1;
            var=confVar(FMT("path.images#%s",name));
        } else {
            var=confVar("path.images");
        }
        if (var && var->length() ) {
            imagePath=toString(var);
        }
        if(slen(imagePath)==0 ) imagePath=Q2A(QApplication::applicationDirPath());
    }
    // qDebug("image path:%s, name:%s", imagePath, fileName);
    return FMT("%s/%s", getFullFileName(imagePath), fileName);
}

QPixmap* getPixmap(LPCC c, bool alpha) {
    int pos = IndexOf(c,':');
    if( pos > 1 ) {
        return getIconPixmapType(gBuf.add(c,pos),c+pos+1,alpha);
    }
    pos=LastIndexOf(c,'.');
    LPCC ext=NULL;
    if( pos>0 ) {
        ext=toUpper(c+pos+1);
    }
    return getPixmapFile(c, ext);
}

QPixmap* getPixmapFile(LPCC fileName, LPCC ext, bool thumb) {
    StrVar* sv=gPixmapNode.gv(fileName);
    if( SVCHK('i',6) ) {
        return (QPixmap*)SVO;
    }
    LPCC fnm=getImageFileName(fileName);
    if( !isFile(fnm) ) {
        qDebug("#9#getPixmapFile file not found: %s", fnm);
        return NULL;
    }
    if( ext==NULL ) {
        int pos=LastIndexOf(fileName,'.');
        if( pos>0 ) {
            ext=toUpper(fileName+pos+1);
        }
    }
    if( slen(ext)==0 || ccmp(ext,"JPG") ) ext="JPEG";
    QImage img;
    if( thumb && fetchThumbnail(fnm, img) ) {

    } else {
       img.load(KR(fnm), ext);
    }
    if( img.isNull() ) {
        _LOG("#0## image load fail filename: %s\n",fnm);
    } else {
        QByteArray ba;
        QBuffer buffer( &ba );
        buffer.open( QIODevice::WriteOnly );
        img.save( &buffer, ext);
        int sz = ba.size();
        if( sz>0 ) {
            QPixmap* p = new QPixmap();
            p->loadFromData((const uchar *)ba.constData(), sz, ext );
            gPixmapNode.GetVar(fileName)->setVar('i',6,(LPVOID)p);
            return p;
        }
    }
    return NULL;
}


const QIcon& getQIcon( LPCC code, LPCC mode, LPCC stat) {
    QPixmap* img=getPixmap(code, true);
    QIcon::Mode m=QIcon::Normal;
    QIcon::State s=QIcon::Off;
    if( mode ) {
        if( ccmp(mode,"disable") )
            m= QIcon::Disabled;
        else if( ccmp(mode,"select") )
            m= QIcon::Selected;
    }
    if( stat ) {
        if( ccmp(stat,"on") ) s=QIcon::On;
    }
    if( img ) {
        gGlobalIcon.addPixmap(*img, m, s);
    }
    return gGlobalIcon;
}

bool isFile(LPCC fileName, bool makeFolder) {
    QFileInfo fi(KR(fileName));
    bool ret=fi.isFile();
    if( ret==false && makeFolder ) {
        ret=makeFileFolder(fileName);
    }
    return ret;
}
bool isFolder(LPCC path) {
    QFileInfo fi(KR(path));
    return fi.isDir();
}

bool makeFileFolder(LPCC fileName, bool all, StrVar* rst) {
    if( isFile(fileName) ) {
        return true;
    }
    bool ret=false;
    int pos=LastIndexOf(fileName,'/');
    _LOG("makeFileFolder %s %d", fileName, pos);
    if( pos>0 ) {
        LPCC path=SCPY(fileName,pos);
        /*
        if( path[0]!='/' ) {
            path=FMT("%s/%s", Q2A(QApplication::applicationDirPath()), path);
        }
        */
        if( isFolder(path) ) {
            ret=true;
        } else {
            QDir dir = QDir::root();
            if( all ) {
                ret=dir.mkpath(KR(path));
            } else {
                ret=dir.mkdir(KR(path));
            }
        }
        if( rst && ret ) {
            rst->set(path);
        }
    }
    return ret;
}
LPCC getSaveFileName(LPCC fileName) {
    StrVar* var=cfVar("temp", true);
    if( makeFileFolder(fileName,true,var) && var->length() ) {
        int pos=LastIndexOf(fileName,'/');
        if( pos>0 ) {
            var->add('/').add(fileName+pos+1);
            fileName=var->get();
            _LOG("xxxx path=%s xxxx", fileName);
        }
    }
    return fileName;
}

int fileReadAll(LPCC path, StrVar* rst) {
    if( rst==NULL ) return 0;
    LPCC fileName=getFullFileName(path);
    // _LOG("fileReadAll path: %s", fileName);
    if( isFile(fileName) ) {
        QString filePath(KR(fileName));
        QFile file(filePath);
        if( file.open(QIODevice::ReadOnly) ) {
            QByteArray ba=file.readAll();
            rst->set(ba.constData(), ba.size() );
            file.close();
            return rst->length();
        } else {
            qDebug("#9#read all file open error (filename:%s)\n", fileName);
        }
    } else {
        qDebug("#9#read all file not found (filename:%s)\n", fileName);
    }
    return 0;
}
int fileSaveAll(LPCC path, StrVar* rst) {
    if( rst==NULL ) return 0;
    LPCC fileName=getFullFileName(path);
    makeFileFolder(fileName, true);
    QFile file(KR(fileName));
    if( file.open(QIODevice::WriteOnly) ) {
        file.write(rst->get(), rst->length());
        file.close();
    }
    return 0;
}





int stringNameCmp(LPCC left, LPCC right) {
    return left && right ? str_cmp(left,right) : 0;
}

int stringNameSort(LPCC left, LPCC right) {
    if( left==NULL || right==NULL )
        return 0;
    LPCC a=left,b=right;
    LPCC pa=a, pb=b;
    while( *a && *b ) {
        pa=a, pb=b;
        while ( *a && IS_NOT_DIGIT(*a) ) a++;
        while ( *b && IS_NOT_DIGIT(*b) ) b++;
        int size = (a-pa);
        bool go = size==(b-pb) && memcmp(pa,pb,size)==0 ? true :false;
        if( *a==0 || *b==0 )
            break;
        if( !go ) return str_cmp(pa,pb);
        if( *a=='0' || *b=='0' ) {
            while(1) {
                if( IS_NOT_DIGIT(*a) && IS_NOT_DIGIT(*b)) {
                    break;
                } else if( IS_NOT_DIGIT(*a)) {
                    return -1;
                } else if( IS_NOT_DIGIT(*b)) {
                    return 1;
                } else if( *a < *b) {
                    return -1;
                } else if( *a > *b) {
                    return 1;
                }
                ++a; ++b;
            }
        } else {
            int weight = 0;
            while (1) {
                if( IS_NOT_DIGIT(*a) && IS_NOT_DIGIT(*b)) {
                    if( weight!= 0) {
                        return weight;
                    }
                    break;
                } else if( IS_NOT_DIGIT(*a)) {
                    return -1;
                } else if( IS_NOT_DIGIT(*b)) {
                    return + 1;
                } else if( (*a < *b) && (weight == 0)) {
                    weight = -1;
                } else if( (*a > *b) && (weight == 0)) {
                    weight = + 1;
                }
                ++a; ++b;
            }
        }
    }
    return str_cmp(pa,pb);
}



void bubbleSort(XListArr *list, int dir, LPCC field, LPCC subField)
{
    if( list==NULL )
        return;
    // ver 1.0.2
    int ArySize = list->size();
    StrVar* sv=NULL;
    StrVar temp;
    int i, j;
    for( i=ArySize-1; i>0; i--) {
        for(j=0; j<i; j++)  {
            StrVar *a = list->get(j);
            StrVar *b = list->get(j+1);
            StrVar* aa=NULL, *bb=NULL;
            LPCC findField=field;
            sv=a;
            if( SVCHK('n',0) ) {
                sv = b;
                if( SVCHK('n',0) ) {
                    aa=((TreeNode*)a->getObject(FUNC_HEADER_POS))->gv(field);
                    bb=((TreeNode*)b->getObject(FUNC_HEADER_POS))->gv(field);
                    findField=subField;
                }
            } else {
                aa=a;
                bb=b;
            }
            if(aa==NULL || bb==NULL ) {
                continue;
            }
            if( subField && checkFuncObject(aa,'n',0) && checkFuncObject(bb,'n',0) ) {
                aa=((TreeNode*)aa->getObject(FUNC_HEADER_POS))->gv(subField);
                bb=((TreeNode*)bb->getObject(FUNC_HEADER_POS))->gv(subField);
                findField=NULL;
            }
            bool checkString=false, checkSwap=false;
            char ch=aa->charAt(0);
            if( ch=='\b' ) {
                ch=aa->charAt(1);
                sv = bb;
                if( ch=='x' && SVCHK('x',21) ) {
                    // ### version 1.0.4
                    int ap=aa->find("\f", FUNC_HEADER_POS), bp=bb->find("\f", FUNC_HEADER_POS);
                    if( ap>0 && bp>0 ) {
                        LPCC s1=aa->get(FUNC_HEADER_POS,ap), s2=bb->get(FUNC_HEADER_POS,bp);
                        if( s1 && s2 ) {
                            if( (dir == 0) ? strcmp(s1,s2)>0 :
                                (dir == 1) ? strcmp(s1,s2)<0 :
                                (dir == 2) ? stringNameSort(s1,s2)>0 :
                                (dir == 3) ? stringNameSort(s1,s2)<0 : false )
                            {
                                checkSwap = true;
                            }
                        }
                    }
                } else if( ch=='i' ) {
                    U16 stat=aa->getU16(2);
                    if(!checkFuncObject(bb,ch,stat)) continue;
                    if(slen(findField)==0 ) findField="x";
                    if( stat==5 || stat==20 ) {
                        int sz=sizeof(double);
                        double av= ccmp(findField,"x") ? aa->getDouble(4):
                             ccmp(findField,"y") ? aa->getDouble(4+sz):
                             ccmp(findField,"w") && stat==5 ? aa->getDouble(4+(sz*2)):
                             ccmp(findField,"h") && stat==5 ? aa->getDouble(4+(sz*3)): 0;
                        double bv= ccmp(findField,"x") ? bb->getDouble(4):
                             ccmp(findField,"y") ? bb->getDouble(4+sz):
                             ccmp(findField,"w") && stat==5 ? bb->getDouble(4+(sz*2)):
                             ccmp(findField,"h") && stat==5 ? bb->getDouble(4+(sz*3)): 0;
                        if( dir==0 ? av>bv : av<bv ) {
                            checkSwap = true;
                        }
                    } else if(stat==2 || stat==4) {
                        int av= ccmp(findField,"x") ? aa->getInt(4):
                             ccmp(findField,"y") ? aa->getInt(8):0;
                        int bv= ccmp(findField,"x") ? bb->getInt(4):
                             ccmp(findField,"y") ? bb->getInt(8):0;
                        if( dir==0 ? av>bv : av<bv ) {
                            checkSwap = true;
                        }
                    }
                } else if( ch=='0' ) {
                    if( dir==0 ? aa->getInt(4)<toInteger(bb) : aa->getInt(4)>toInteger(bb) ) {
                        checkSwap = true;
                    }
                } else if( ch=='1' ) {
                    if( dir==0 ? aa->getUL64(4)<toUL64(bb) : aa->getUL64(4)>toUL64(bb) ) {
                        checkSwap = true;
                    }
                } else if( ch=='4' ) {
                    if( dir==0 ? aa->getDouble(4)<toDouble(bb) : aa->getDouble(4)>toDouble(bb) ) {
                        checkSwap = true;
                    }
                }
            } else {
                checkString=true;
            }

            if( checkString ) {
                LPCC s1 = toString(aa), s2 = toString(bb);
                if( s1 && s2 ) {
                    if( (dir == 0) ? strcmp(s1,s2)>0 :
                        (dir == 1) ? strcmp(s1,s2)<0 :
                        (dir == 2) ? stringNameSort(s1,s2)>0 :
                        (dir == 3) ? stringNameSort(s1,s2)<0 : false )
                    {
                        checkSwap = true;
                    }
                }
            }
            if( checkSwap ) {
                temp.reuse()->add(a);
                a->reuse()->add(b);
                b->reuse()->add(&temp);
            }
        }
    }
}


bool keyEventCheck(int key, U32 modifyer, LPCC keyMap) {
    int len=slen(keyMap);
    U32 flag=0;
    _LOG(">> key:%x, modifyer:%llx (key:%s)\n", key, modifyer, keyMap);
    for( int n=0;n<len;n++) {
        switch( keyMap[n] ) {
        case '!': {
            flag|=1;
            if( (modifyer&Qt::ShiftModifier)==0 ) return false;
        } break;
        case '#': {
            flag|=2;
            if( (modifyer&Qt::ControlModifier)==0 ) return false;
        } break;
        case '@': {
            flag|=4;
            if( (modifyer&Qt::AltModifier)==0 ) return false;
        } break;
        case '`': {
            if( flag==1 && key!=Qt::Key_AsciiTilde ) return false;
            else if( key!=Qt::Key_QuoteLeft ) return false;
        } break;
        case '1': {
            if( flag==1 && key!=21 ) return false;
            else if( key!=Qt::Key_1 ) return false;
        } break;
        case '2': {
            if( flag==1 && key!=40 ) return false;
            else if( key!=Qt::Key_2 ) return false;
        } break;
        case '3': {
            if( flag==1 && key!=23 ) return false;
            else if( key!=Qt::Key_3 ) return false;
        } break;
        case '4': {
            if( flag==1 && key!=24 ) return false;
            else if( key!=Qt::Key_4 ) return false;
        } break;
        case '5': {
            if( flag==1 && key!=25 ) return false;
            else if( key!=Qt::Key_5 ) return false;
        } break;
        case '6': {
            if( flag==1 && key!=52 ) return false;
            else if( key!=Qt::Key_6 ) return false;
        } break;
        case '7': {
            if( flag==1 && key!=26 ) return false;
            else if( key!=Qt::Key_7 ) return false;
        } break;
        case '8': {
            if( flag==1 && key!=29 ) return false;
            else if( key!=Qt::Key_8 ) return false;
        } break;
        case '9': {
            if( flag==1 && key!=28 ) return false;
            else if( key!=Qt::Key_9 ) return false;
        } break;
        case '0': {
            if( flag==1 && key!=29 ) return false;
            else if( key!=Qt::Key_0 ) return false;
        } break;
        case '-': {
            if( flag==1 && key!=0x5f ) return false;
            else if( key!=0x2d ) return false;
        } break;
        case '=': {
            if( flag==1 && key!=0x2b ) return false;
            else if( key!=0x3d ) return false;
        } break;
        case '[': {
            if( flag==1 && key!=0x7b ) return false;
            else if( key!=0x5b ) return false;
        } break;
        case ']': {
            if( flag==1 && key!=0x7d ) return false;
            else if( key!=0x5d ) return false;
        } break;
        case '\\': {
            if( flag==1 && key!=0x7c ) return false;
            else if( key!=0x5c ) return false;
        } break;
        case ';': {
            if( flag==1 && key!=0x3b ) return false;
            else if( key!=0x3a ) return false;
        } break;
        case '\'': {
            if( flag==1 && key!=0x22 ) return false;
            else if( key!=0x27 ) return false;
        } break;
        case ',': {
            if( flag==1 && key!=0x3c ) return false;
            else if( key!=0x2c ) return false;
        } break;
        case '.': {
            if( flag==1 && key!=0x3e ) return false;
            else if( key!=0x2e ) return false;
        } break;
        case '/': {
            if( flag==1 && key!=0x3f ) return false;
            else if( key!=0x2f ) return false;
        } break;

        case 'F': {
            switch( keyMap[n++] ) {
            case '1': {
                if( key!=Qt::Key_F1 ) return false;
            } break;
            case '2': {
                if( key!=Qt::Key_F2 ) return false;
            } break;
            case '3': {
                if( key!=Qt::Key_F3 ) return false;
            } break;
            case '4': {
                if( key!=Qt::Key_F4 ) return false;
            } break;
            case '5': {
                if( key!=Qt::Key_F5 ) return false;
            } break;
            case '6': {
                if( key!=Qt::Key_F6 ) return false;
            } break;
            case '7': {
                if( key!=Qt::Key_F7 ) return false;
            } break;
            case '8': {
                if( key!=Qt::Key_F8 ) return false;
            } break;
            case '9': {
                if( key!=Qt::Key_F9 ) return false;
            } break;
            case '0': {
                if( key!=Qt::Key_F10 ) return false;
            } break;
            case '_': {
                if( key!=Qt::Key_F11 ) return false;
            } break;
            case '+': {
                if( key!=Qt::Key_F12 ) return false;
            } break;
            }
        } break;
        case '<': {
            if( (key!=Qt::Key_PageUp)==0 ) return false;
        } break;
        case '>': {
            if( (modifyer&Qt::Key_PageDown)==0 ) return false;
        } break;
        case '{': {
            if( (key!=Qt::Key_Home)==0 ) return false;
        } break;
        case '}': {
            if( (key!=Qt::Key_End)==0 ) return false;
        } break;
        case '(': { // (:left, ):right, ^:up, _:down
            if( (key!=Qt::Key_Left)==0 ) return false;
        } break;
        case ')': {
            if( (key!=Qt::Key_Right)==0 ) return false;
        } break;
        case '^': {
            if( (key!=Qt::Key_Up)==0 ) return false;
        } break;
        case '_': {
            if( (key!=Qt::Key_Down)==0 ) return false;
        } break;
        case 'N': {
            if( (key!=Qt::Key_NumLock)==0 ) return false;
        } break;
        case 'M': {
            if( (key!=Qt::Key_Meta)==0 ) return false;
        } break;
        case 'P': {
            if( (key!=Qt::Key_Pause)==0 ) return false;
        } break;
        case 'I': {
            if( (key!=Qt::Key_Insert)==0 ) return false;
        } break;
        case 'D': {
            if( key!=Qt::Key_Delete ) return false;
        } break;
        case 'B': {
            if( key!=Qt::Key_Backspace ) return false;
        } break;
        case 'C': {
            if( key!=Qt::Key_CapsLock ) return false;
        } break;
        case 'R': {
            if( key!=Qt::Key_Return || key!=Qt::Key_Enter  ) return false;
        } break;
        case 'T': {
            if( flag==1 && key!=Qt::Key_Backtab ) return false;
            else if( key!=Qt::Key_Tab ) return false;
        } break;
        case 'E': {
            if( key!=Qt::Key_Escape ) return false;
        } break;
        case 'a': {
            if( key!=Qt::Key_A ) return false;
        } break;
        case 'b': {
            if( key!=Qt::Key_B ) return false;
        } break;
        case 'c': {
            if( key!=Qt::Key_C ) return false;
        } break;
        case 'd': {
            if( key!=Qt::Key_D ) return false;
        } break;
        case 'e': {
            if( key!=Qt::Key_E ) return false;
        } break;
        case 'f': {
           if( key!=Qt::Key_F ) return false;
        } break;
        case 'g': {
           if( key!=Qt::Key_G ) return false;
        } break;
        case 'h': {
           if( key!=Qt::Key_H ) return false;
        } break;
        case 'i': {
           if( key!=Qt::Key_I ) return false;
        } break;
        case 'j': {
           if( key!=Qt::Key_J ) return false;
        } break;
        case 'k': {
           if( key!=Qt::Key_K ) return false;
        } break;
        case 'l': {
            if( key!=Qt::Key_L ) return false;
        } break;
        case 'm': {
            if( key!=Qt::Key_M ) return false;
        } break;
        case 'n': {
            if( key!=Qt::Key_N ) return false;
        } break;
        case 'o': {
            if( key!=Qt::Key_O ) return false;
        } break;
        case 'p': {
            if( key!=Qt::Key_P ) return false;
        } break;
        case 'q': {
            if( key!=Qt::Key_Q ) return false;
        } break;
        case 'r': {
            if( key!=Qt::Key_R ) return false;
        } break;
        case 's': {
            if( key!=Qt::Key_S ) return false;
        } break;
        case 't': {
            if( key!=Qt::Key_T ) return false;
        } break;
        case 'u': {
            if( key!=Qt::Key_U ) return false;
        } break;
        case 'v': {
            if( key!=Qt::Key_V ) return false;
        } break;
        case 'w': {
            if( key!=Qt::Key_W ) return false;
        } break;
        case 'x': {
            if( key!=Qt::Key_X ) return false;
        } break;
        case 'y': {
            if( key!=Qt::Key_Y ) return false;
        } break;
        case 'z': {
            if( key!=Qt::Key_Z ) return false;
        } break;
        default: {
            _LOG("[not match] key:%d, modifyer:%lld (key:%s)\n", key, modifyer, keyMap);
            return false;
        } break;
        }
    }
    _LOG("[ok] key:%d, modifyer:%lld (key:%s)\n", key, modifyer, keyMap);
    return true;
}

double randomNum(double fMin, double fMax) {
    double f = (double)rand() / RAND_MAX;
    double val=fMin + f * (fMax - fMin);
    return val;
}
QColor randomColor() {
    hsv2rgb(gclr, randomNum(0,360), 1.0f, 0.5f);
    // qDebug("random color: %d, %d, %d",gclr.red(), gclr.green(), gclr.blue() );
    return gclr;
}

QPixmap* randomPixmap() {
    DbUtil* db=getIconDb();
    int num=(int)randomNum(0,1000);
    if( db->prepare("select type||'.'||id as iconId from icons where type='vicon' limit ?,1") ) {
        db->bindInt(num);
        if( db->fetchNode() ) {
            TreeNode* cf=db->xcf;
            return getPixmap(cf->get("iconId"));
        }
    }
    return getPixmap("vicon:add_default");
}
