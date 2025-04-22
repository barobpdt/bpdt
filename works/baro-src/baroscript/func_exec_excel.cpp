#define QGUI_USE
#ifdef QGUI_USE
#include <qpixmap.h>
#endif
#include "func_util.h"

#ifdef EXCEL_USE
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "xlsxcellreference.h"
#include "xlsxcellrange.h"
#include "xlsxworksheet.h"
#include "xlsxworkbook.h"
#include "xlsxconditionalformatting.h"
#include "xlsxchart.h"

QTXLSX_USE_NAMESPACE

StrVar gfmt;
StrVar gExcelVar;
Format gExcelFormat;

/*
inline QPixmap* getIconPixmapType(LPCC type, LPCC id, bool alpha) {
    if( slen(type)==0 || slen(id)==0 ) return NULL;
    LPCC key=FMT("%s:%s",type,id);
    QPixmap* p = gPixmaps.value(key);
    if( p==NULL ) {
        gMutexPixMap.EnterMutex();
        DbUtil* db = getIconDb();
        if( !db->query("select data from icons where type=? and id=?").bindStr(type).bindStr(id).exec().fetchNode() ) {
            return NULL;
        }
        StrVar* data = db->GetVar("data");
        // qDebug("...[getIconPixmapType] %s %d", key, data->length() );
        if( data->length() ) {
            if( alpha ) {
                QImage img;
                if( !img.loadFromData((const uchar*)data->get(),data->length(),"PNG") ) {
                    img.loadFromData((const uchar*)data->get(),data->length(),"JPG");
                }
                QImage alpha = img.alphaChannel();
                QRgb c=img.pixel(1,1);
                int height=img.height(),width=img.width();
                for(int j = 0; j < height; j++) {
                    for(int i = 0; i < width; i++) {
                        if( img.pixel(i,j)==c )
                            alpha.setPixel(i,j,0);
                    }
                }
                img.setAlphaChannel(alpha);
                p = new QPixmap(QPixmap::fromImage(img));
            } else {
                p = new QPixmap();
                if( !p->loadFromData((const uchar*)data->get(),data->length(),"PNG") ) {
                    p->loadFromData((const uchar*)data->get(),data->length(),"JPG");
                }
            }
            if( p ) {
                gPixmaps.add(key,p);
            }
        }
        gMutexPixMap.LeaveMutex();
    }
    // qDebug()<<"getImagePixmap key="<<key<<" w="<<p->width()<<" h="<<p->height();
    return p;
}

inline QPixmap* getPixmap(LPCC c, bool alpha) {
    LPCC code = c;
    LPCC type = NULL;
    int pos = IndexOf(c,'.');
    if( pos > 0 ) {
        type=gBuf.add(c,pos);
        code=c+pos+1;
    }
    if( slen(type)==0 )
        type = "vicon";
    return getIconPixmapType(type,code,alpha);
}
*/

inline void hsv2rgb(QColor& clr, double hh, double hs, double hv, int alpha) {
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

inline StrVar* GetColorValue(LPCC s, StrVar* rst ) {
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

inline void regExcelFuncs() {
    if( isHashKey("excel") )
        return;
    PHashKey hash = getHashKey("excel");
    U16 uid = 1;
    hash->add("open", uid);			uid++;	//
    hash->add("save", uid);			uid++;	//
    hash->add("range", uid);		uid++;	//
    hash->add("write", uid);		uid++;	//
    hash->add("read", uid);			uid++;	//
    hash->add("merge", uid);		uid++;	//
    hash->add("style", uid);		uid++;	//
    hash->add("format", uid);		uid++;	//
    hash->add("close", uid);		uid++;	//

    uid = 21;
    hash->add("width", uid);		uid++;
    hash->add("height", uid);		uid++;
    hash->add("refString", uid);	uid++;
    hash->add("def", uid);			uid++;
    hash->add("pos", uid);			uid++;

    uid = 31;
    hash->add("image", uid);        uid++;
    hash->add("sheet", uid);		uid++;
    hash->add("rule", uid);			uid++;
    hash->add("is", uid);			uid++;
    uid = 41;
    hash->add("chart", uid);		uid++;
    hash->add("groupRows", uid);	uid++;
    hash->add("groupColumns", uid);	uid++;
}

inline int intPow(int x, int p)
{
  if (p == 0) return 1;
  if (p == 1) return x;

  int tmp = intPow(x, p/2);
  if (p%2 == 0) return tmp * tmp;
  else return x * tmp * tmp;
}

inline QString col_to_name(int col_num)
{
    static QMap<int, QString> col_cache;

    if (!col_cache.contains(col_num)) {
        QString col_str;
        int remainder;
        while (col_num) {
            remainder = col_num % 26;
            if (remainder == 0)
                remainder = 26;
            col_str.prepend(QChar('A'+remainder-1));
            col_num = (col_num - 1) / 26;
        }
        col_cache.insert(col_num, col_str);
    }

    return col_cache[col_num];
}

inline int col_from_name(const QString &col_str)
{
    int col = 0;
    int expn = 0;
    for (int i=col_str.size()-1; i>-1; --i) {
        col += (col_str[i].unicode() - 'A' + 1) * intPow(26, expn);
        expn++;
    }

    return col;
}
#ifdef QT5_USE
#endif

inline Format::BorderStyle getFormatLineStyle(LPCC s) {
    return ccmp(s,"thin") ? Format::BorderThin :
        ccmp(s,"dash") ? Format::BorderDashed :
        ccmp(s,"dot") ? Format::BorderDotted :
        ccmp(s,"double") ? Format::BorderDouble :
        ccmp(s,"hair") ? Format::BorderHair:
        Format::BorderThin;
}

inline Format* formatExcel(TreeNode* tn, StrVar* var, LPCC styleId=NULL, Format* fmt=NULL ) {
    if( fmt==NULL ) {
        if( slen(styleId) ) {
            StrVar* sv=tn->GetVar(styleId);
            if( SVCHK('e',11) ) {
                fmt=(Format*)SVO;
            } else {
                fmt=new Format();
                sv->setVar('e',11,(LPVOID)fmt);
            }
        } else {
            fmt=&gExcelFormat;
        }
    }
    if( var ) {
        int sp=0, ep=0;
        var=getStrVarPointer(var, &sp, &ep);
        XParseVar pv(var, sp, ep);
        while( pv.valid() ) {
            if( pv.ch()==0 ) break;
            LPCC type=pv.NextWord();
            if( pv.ch()!=':') break;
            pv.incr();
            pv.findEnd(";");
            LPCC style=pv.v();
            int len=slen(type);
            if( ccmpl(type,len,"fontsize") && isnum(style) )
                fmt->setFontSize(atoi(style));
            else if( ccmpl(type,len,"align") ) fmt->setHorizontalAlignment( ccmp(style,"center") ? Format::AlignHCenter :
                ccmp(style,"right") ? Format::AlignRight :
                ccmp(style,"left") ? Format::AlignLeft : Format::AlignLeft);
            else if( ccmpl(type,len,"valign") ) fmt->setVerticalAlignment( ccmp(style,"center") ? Format::AlignVCenter :
                ccmp(style,"top") ? Format::AlignTop :
                ccmp(style,"bottom") ? Format::AlignBottom : Format::AlignVCenter);
            else if( ccmpl(type,len,"fontname") ) fmt->setFontName(KR(style));
            else if( ccmpl(type,len,"fontcolor") || ccmpl(type,len,"color") ) fmt->setFontColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"italic") ) fmt->setFontItalic( ccmp(style,"false")?false: true );
            else if( ccmpl(type,len,"strike") ) fmt->setFontStrikeOut( ccmp(style,"false")?false: true );
            else if( ccmpl(type,len,"bold") ) fmt->setFontBold( ccmp(style,"false")?false: true );
            else if( ccmpl(type,len,"fill") ) fmt->setPatternBackgroundColor( getGlobalColor(&gfmt.set(style)) );
            else if( ccmpl(type,len,"patterncolor") ) fmt->setPatternForegroundColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"pattern") ) fmt->setFillPattern(
                ccmp(style,"Gray") ? Format::PatternLightGray :
                ccmp(style,"MediumGray") ? Format::PatternMediumGray :
                ccmp(style,"DarkGray") ? Format::PatternDarkGray :
                ccmp(style,"GrayDot") ? Format::PatternGray0625 :
                ccmp(style,"Grid") ? Format::PatternLightGrid :
                ccmp(style,"Horizontal") ? Format::PatternLightHorizontal :
                ccmp(style,"DarkHorizontal") ? Format::PatternDarkHorizontal :
                ccmp(style,"Vertical") ? Format::PatternLightVertical :
                ccmp(style,"DarkVertical") ? Format::PatternDarkVertical :
                ccmp(style,"Trellis") ? Format::PatternLightTrellis :
                ccmp(style,"LightUp") ? Format::PatternLightUp : ccmp(style,"LightDown") ? Format::PatternLightDown :
                ccmp(style,"Up") ? Format::PatternDarkUp : ccmp(style,"Down") ? Format::PatternDarkDown  : Format::PatternNone );
            else if( ccmpl(type,len,"bordercolor") ) fmt->setBorderColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"borderstyle") ) fmt->setBorderStyle( getFormatLineStyle(style) ); // thin, dash, dot, duble, hair
            else if( ccmpl(type,len,"bordertopcolor") ) fmt->setTopBorderColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"borderleftcolor") ) fmt->setLeftBorderColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"borderrightcolor") ) fmt->setRightBorderColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"borderbottomcolor") ) fmt->setBottomBorderColor(getGlobalColor(&gfmt.set(style)));
            else if( ccmpl(type,len,"bordertop") ) fmt->setTopBorderStyle(getFormatLineStyle(style));
            else if( ccmpl(type,len,"borderleft") ) fmt->setLeftBorderStyle(getFormatLineStyle(style));
            else if( ccmpl(type,len,"borderright") ) fmt->setRightBorderStyle(getFormatLineStyle(style));
            else if( ccmpl(type,len,"borderbottom") ) fmt->setBottomBorderStyle(getFormatLineStyle(style));
            else if( ccmpl(type,len,"rotation") && isnum(style) ) fmt->setRotation(atoi(style));
            else if( ccmpl(type,len,"numberformat") ) fmt->setNumberFormat(KR(style));	// #.###, #.00, [Red][<=100];[Green][>100]
            else if( ccmpl(type,len,"numberformatnum") && isnum(style) ) fmt->setNumberFormatIndex(atoi(style));
        }
    }
    return fmt;
}

inline void writeExcelFormat(TreeNode* tn, Document* w, int x, int y, StrVar* var, LPCC ty=NULL, LPCC val=NULL, Format* fmt=NULL) {
    int len=0;
    bool dot = false, bnum=isnum(val, &len, &dot);
    if( var || fmt ) {
        Cell* cell=w->cellAt(x,y);
        Format format;
        if( fmt ) {
            format=*fmt;
        } else if( cell ) {
            format=cell->format();
        }
        if( var ) {
            formatExcel(tn, var, NULL, &format);
        }
        if( val ) {
            if( dot )
                w->write(x, y, atof(val), format );
            else if( bnum )
                w->write(x, y, atoi(val), format );
            else
                w->write(x, y, KR(val), format );
        } else {
            if( slen(ty)==0 ) {
                w->write(x,y,QVariant(), format );
            } else if( ccmp(ty,"trim") ) {
                QString s = w->read(x,y).toString();
                w->write(x,y,s.trimmed(), format );
            } else {
                w->write(x,y,w->read(x,y), format );
            }
        }
    } else if( val ) {
        if( dot )
            w->write(x, y, atof(val)  );
        else if( bnum )
            w->write(x, y, atoi(val) );
        else
            w->write(x, y, KR(val) );
    }
}

inline void writeExcel(TreeNode* tn, Document* w, CellReference& cr, StrVar* sv, StrVar* var=NULL, Format* fmt=NULL ) {
    LPCC val = toString(sv);
    writeExcelFormat( tn, w, cr.row(), cr.column(), var, NULL, val, fmt);
}

bool callExcelFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->gv("@c");
    Document* w = NULL;
    if( SVCHK('p',2) ) {
        w=(Document*)SVO;
    }
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        regExcelFuncs();
        ref=getHashKeyVal("excel", fnm );
        if( fc ) fc->xfuncRef = ref;
    }
    int cnt=arrs? arrs->size(): 0;
    if( ref==0 )
        return false;

    switch( ref ) {
    case 1: {		// open
        if( arrs==NULL ) {
            rst->setVar('3',w?1:0);
            if(w==NULL ) {
                StrVar* sv=tn->GetVar("@c");
                w=new Document();
                sv->setVar('p',2,(LPVOID)w);
            }
            return true;
        }
        LPCC filenm=AS(0);        
        if( slen(filenm) ) {
            LPCC fullName=getFullFileName(filenm);
            StrVar* sv=tn->GetVar("@c");
            if( w ) {
                SAFE_DELETE(w);
                if(sv) sv->reuse();
            }
            tn->set("@fullName", fullName);
            if( QFile::exists(KR(fullName))) {                
                w=new Document(KR(fullName));
            } else {
                w=new Document();
            }
            sv->setVar('p',2,(LPVOID)w);
            rst->setVar('3',1);
        }        
    } break;
    case 2: {		// save
        if( arrs==NULL ) {
            return true;
        }
        if(w==NULL) return true;
        LPCC filenm=AS(0);
        LPCC fullName=getFullFileName(filenm);
        rst->setVar('3', w->saveAs(KR(fullName)) ? 1: 0);
        if(isVarTrue(arrs->get(1)) ) {
            SAFE_DELETE(w);
            tn->GetVar("@c")->reuse();
        }
    } break;
    case 3: {		// range
        if(w==NULL) return true;
        int cnt = arrs? arrs->size():0;
        if( cnt==0 ) return false;
        StrVar* sv=NULL;
        if( isNumberVar(arrs->get(0)) ) {
            // range(1,1, 10,1);
            int r=toInteger(arrs->get(0)), c=toInteger(arrs->get(1));
            CellReference refStart(r, c);
            rst->set(Q2A(refStart.toString()) );
            if( isNumberVar(arrs->get(2)) ) {
                int r1=toInteger(arrs->get(2)), c1=toInteger(arrs->get(3));
                CellReference refEnd(r1, c1);
                rst->add(":");
                rst->add(Q2A(refEnd.toString()) );
            }
        } else {
            // range("1,2") || range('1,2:10,2')
            sv=arrs->get(0);
            rst->set( excelRange(sv, rst) );
        }
    } break;
    case 4: {		// write
        if(w==NULL) return true;
        if( arrs==NULL ) {
            return true;
        }
        int cnt= arrs->size();
        Format* fmt=NULL;
        StrVar* var=NULL;
        StrVar* txt=NULL;
        if( isNumberVar(arrs->get(0)) ) {
            int r=toInteger(arrs->get(0)), c=toInteger(arrs->get(1));
            sv=arrs->get(2);
            if( SVCHK('e',11) ) {
                // ex) write( x,y, fmt, text) || write(x,y, fmt, style, text)
                fmt=(Format*)SVO;
                if( cnt==4 ) {
                    txt=arrs->get(3);
                } else if( cnt==5 ) {
                    var=arrs->get(3);
                    txt=arrs->get(4);
                }
            } else {
                // ex) write( x,y, text) || write(x,y, style, text)
                if( cnt==3 ) {
                    txt=arrs->get(2);
                } else if( cnt==4 ) {
                    var=arrs->get(2);
                    txt=arrs->get(3);
                }
            }
            CellReference ref(r,c);
            writeExcel(tn, w, ref, txt, var, fmt);
        } else {
            LPCC range=excelRange(arrs->get(0), rst);
            sv=arrs->get(1);
            if( SVCHK('e',11) ) {
                // ex) write( range, fmt, text) || write(range, fmt, style, text)
                fmt=(Format*)SVO;
                if( cnt==3 ) {
                    txt=arrs->get(2);
                } else if( cnt==5 ) {
                    var=arrs->get(2);
                    txt=arrs->get(3);
                }
            } else {
                // ex) write( range, text) || write(range, style, text)
                if( cnt==2 ) {
                    txt=arrs->get(1);
                } else if( cnt==4 ) {
                    var=arrs->get(1);
                    txt=arrs->get(2);
                }
            }
            CellReference ref(range);
            writeExcel(tn, w, ref, txt, var, fmt);
        }
    } break;
    case 5: {		// read
        if(w==NULL) return true;
        int cnt=arrs? arrs->size():0;
        if( cnt==1 ) {
            LPCC range=excelRange(arrs->get(0)); // rst->get();
            rst->set( Q2A(w->read(range).toString()) );
        } else if( isNumberVar(arrs->get(0)) && cnt==2 ) {
            int r=toInteger(arrs->get(0)), c=toInteger(arrs->get(1));
            rst->set( Q2A(w->read(r, c).toString()) );
        }
    } break;
    case 6: {		// merge
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        int cnt=arrs->size();
        if( cnt==1 ) {
            LPCC range=excelRange(arrs->get(0), rst);
            w->mergeCells(CellRange(range));
        } else {
            LPCC range=excelRange(arrs->get(0), rst);
            int p = IndexOf(range,':');
            LPCC a=NULL, b=NULL;
            if( p>0 ) {
                a=gBuf.add(range,p), b=range+p+1;
            }
            Format* fmt=NULL;
            StrVar* var=NULL;
            LPCC txt=NULL;
            sv=arrs->get(1);
            if( SVCHK('e',11) ) {
                // ex) merge(range, fmt) || merge( range, fmt, text) || merge(range, fmt, style, text)
                fmt=(Format*)SVO;
                if( cnt==3 ) {
                    txt=toString(arrs->get(2));
                } else if( cnt==4 ) {
                    var=arrs->get(2);
                    txt=toString(arrs->get(3));
                }
            } else {
                // ex) merge( range, style, text) || merge(range, text);
                if( cnt==3 ) {
                    var=sv;
                    txt=toString(arrs->get(2));
                } else {
                    txt=toString(sv);
                }
            }
            if( b ) {
                int r,c;
                w->mergeCells(CellRange(range) );
                excelCellReference(a,r,c);
                if(fmt) {
                    int rm,cm;
                    excelCellReference(b,rm,cm);
                    for( int x=r; x<=rm; x++ ) {
                        for( int y=c; y<=cm; y++ ) {
                            writeExcelFormat(tn, w, x, y, var, NULL, NULL, fmt);
                        }
                    }
                }
                if( slen(txt) ) {
                    writeExcelFormat(tn, w, r, c, var, NULL, txt );
                }
            }
        }
    } break;
    case 7: {		// style
        if( arrs==NULL ) return false;
        LPCC id=AS(0);
        Format* fmt=NULL;
        if( slen(id) ) {
            StrVar* var=arrs->get(1);
            if( var && var->length() ) {
                fmt=formatExcel(tn, var, id );
            } else {
                var=tn->gv(id);
                if( SVCHK('e',11) ) {
                    fmt=(Format*)SVO;
                }
            }
        }
        if( fmt ) rst->setVar('e',11,(LPVOID)fmt);
    } break;
    case 8: {		// format
        if(w==NULL) return true;
        if( arrs==NULL ) {
            return false;
        }
        int cnt=arrs->size();
        LPCC range=excelRange(arrs->get(0));
        LPCC a=NULL, b=NULL;

        int p=IndexOf(range,':');
        if( p>0 ) {
            a=gBuf.add(range,p), b=range+p+1;
        } else {
            a=range;
        }
        sv=arrs->get(1);
        if( sv ) {
            Format* fmt=NULL;
            StrVar* var=NULL;
            LPCC ty=NULL;
            if( SVCHK('e',11) ) {
                // ex) format(range, fmt, type) || format( range, fmt, type, var)
                fmt=(Format*)SVO;
                ty=AS(2);
                if( cnt==4 ) {
                    var=arrs->get(3);
                }
            } else {
                // ex) format(range, var, type)
                var=sv;
                if( cnt==3 ) {
                    ty=AS(2);
                }
            }
            if( b ) {
                int r,c,rm,cm;
                excelCellReference(a,r,c);
                excelCellReference(b,rm,cm);
                for( int x=r; x<=rm; x++ ) {
                    for( int y=c; y<=cm; y++ ) {
                        writeExcelFormat(tn, w, x, y, var, ty, NULL, fmt);
                    }
                }
            } else if( a ) {
                int r,c;
                excelCellReference(a,r,c);
                writeExcelFormat(tn, w, r, c, var, ty, NULL, fmt);
            }
        }
    } break;
    case 9: {		// close
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        sv=tn->GetVar("@c");
        if( w ) {
            delete w;
        }
        w=new Document();
        sv->setVar('p',2,(LPVOID)w);
    } break;
    case 21: {		// width
        if(w==NULL) return true;
        int cnt=arrs? arrs->size():0;
        int col=1;
        StrVar* var=NULL;
        if( cnt==1 ) {
            // ex) width("10,50,30");
            var=arrs->get(0);
        } else if( cnt==2 ) {
            // ex) width(1, 50);
            sv =arrs->get(0);
            col= isNumberVar(sv) ? toInteger(sv): col_from_name(KR(toString(sv)));
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                int wid=toInteger(sv);
                w->setColumnWidth(col, wid/8 );
            } else {
                var=sv;
            }
        }
        if( var ) {
            int sp=0, ep=0;
            sv=getStrVarPointer(var, &sp, &ep);
            XParseVar pv( sv, sp, ep);
            for( ; pv.valid(); col++ ) {
                pv.findEnd(",");
                LPCC wid=pv.v();
                if( isnum(wid) ) {
                    w->setColumnWidth(col, atoi(wid)/8);
                }
            }
        }
    } break;
    case 22: {		// height
        if(w==NULL) return true;
        int cnt=arrs? arrs->size():0;
        if( cnt==2 ) {
            // ex) height(row, height)
            int row=toInteger(arrs->get(0));
            w->write(row, 1, KR(" ") );
            w->setRowHeight(row, toInteger(arrs->get(1)));
        }
    } break;
    case 23: {		// refString
        if( cnt==0 ) {
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            int r=toInteger(arrs->get(0)), c=toInteger(arrs->get(1));
            CellReference ref(r, c);
            rst->set(Q2A(ref.toString()) );
        }
    } break;
    case 24: {		// def
        if( cnt==0 ) {
            return true;
        }
        w->defineName(AS(0),AS(1));
    } break;
    case 25: {		// pos
        if( cnt==0 ) {
            return true;
        }
        int row=0, col=0;
        excelCellReference(AS(0),row,col);
        rst->setVar('i',2).addInt(row).addInt(col);
    } break;
    case 31: {		// image
        if(w==NULL) return true;
        if( arrs==NULL )
            return false;
#ifdef QGUI_USE
        int cnt = arrs->size(), row=0, col=0;
        QPixmap* pixmap = NULL;
        bool resize=false;
        if( isNumberVar(arrs->get(0)) && cnt>2 ) {
            // ex) image(1,1, "vicon.add', resize)
            row=toInteger(arrs->get(0)), col= toInteger(arrs->get(1));
            if( row>0 ) row-=1;
            if( col>0 ) col-=1;
            pixmap = getStrVarPixmap(arrs->get(2));
            if( cnt==4 ) resize=isVarTrue(arrs->get(3));
        } else {
            // ex) image("A1", "vicon.add', true)
            LPCC range=excelRange(arrs->get(0));
            excelCellReference(range, row, col);
            sv=arrs->get(1);
            pixmap = getStrVarPixmap(arrs->get(1));
            if( cnt==3 ) resize=isVarTrue(arrs->get(3));
        }
        if( pixmap ) {
            w->insertImage(row, col, pixmap->toImage());
            if( resize ) {
                w->setRowHeight(row, pixmap->height()/8 );
            }
        }
#endif
    } break;
    case 32: {		// sheet
        if(w==NULL) return true;
        if( arrs==NULL ) {
            QStringList list=w->sheetNames();
            XListArr* a=getListArrTemp();
            for( int n=0;n<list.size();n++) {
                a->add()->add(Q2A(list.at(n)));
            }
            rst->setVar('a',0,(LPVOID)a);
        } else {
            LPCC name=AS(0);
            bool bchk=w->selectSheet(KR(name));
            if( isVarTrue(arrs->get(1)) ) {
                rst->setVar('3',bchk?1:0);
            } else if( bchk==false ) {
                w->addSheet(KR(name) );
            }
        }
    } break;
    case 33: {		// rule
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        LPCC ty=AS(0);
        LPCC range=excelRange(arrs->get(1), rst);
        int p=IndexOf(range,':');
        if( p<=0 ) return false;
        Format* fmt=NULL;
        ConditionalFormatting cf;
        cf.addRange(KR(range));
        sv=arrs->get(2);
        if( SVCHK('e',11) )
            fmt=formatExcel(tn,NULL,NULL,(Format*)SVO);
        else
            fmt=formatExcel(tn,sv);
        LPCC val=AS(3);
        if( ccmp(ty,"lt") || ccmp(ty,"gt") || ccmp(ty,"le") || ccmp(ty,"ge") ) {
            // ex) rule('gt', 'A1:A10', [fmt|var], 10);
            cf.addHighlightCellsRule(
                ccmp(ty,"lt") ? ConditionalFormatting::Highlight_LessThan :
                ccmp(ty,"le") ? ConditionalFormatting::Highlight_LessThanOrEqual :
                ccmp(ty,"gt") ? ConditionalFormatting::Highlight_GreaterThan :
                ccmp(ty,"ge") ? ConditionalFormatting::Highlight_GreaterThanOrEqual : ConditionalFormatting::Highlight_LessThan,
                val, *fmt );
        } else if( ccmp(ty,"between") || ccmp(ty,"notBetween")  ) {
            // ex) rule('between', 'A1:A10', [fmt|var], 10, 100);
            LPCC start=val, end=AS(4);
            cf.addHighlightCellsRule(
                ccmp(ty,"between") ? ConditionalFormatting::Highlight_Between :ConditionalFormatting::Highlight_NotBetween,
                start, end, *fmt );
        } else if( ccmp(ty,"startWith") || ccmp(ty,"endWith") ) {
            cf.addHighlightCellsRule(
                ccmp(ty,"startWith") ? ConditionalFormatting::Highlight_BeginsWith :ConditionalFormatting::Highlight_EndsWith,
                val, *fmt );
        } else if( ccmp(ty,"contain") || ccmp(ty,"notContain") ) {
            cf.addHighlightCellsRule(
                ccmp(ty,"contain") ? ConditionalFormatting::Highlight_ContainsText :ConditionalFormatting::Highlight_NotContainsText,
                val, *fmt );
        } else if( ccmp(ty,"bar") ) {
            StrVar* var=getStrVarTemp();
            var->add(val);
            cf.addDataBarRule(getQColor(var));
        }
        w->addConditionalFormatting(cf);
    } break;
    case 34: {		// is
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        LPCC ty=AS(0);
        if( ccmp(ty,"gridUse") ) {
            w->currentWorksheet()->setGridLinesVisible( isVarTrue(arrs->get(1)) );
        } else if( ccmp(ty,"htmlUse") ) {
            w->workbook()->setHtmlToRichStringEnabled( isVarTrue(arrs->get(1)) );
        } else {
            return false;
        }
    } break;
    case 41: {		// chart
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        if( !isNumberVar(arrs->get(0)) ) {
            return true;
        }
        int r=toInteger(arrs->get(0)), c=toInteger(arrs->get(1));
        int wid=300, hei=300;
        int idx=2;
        sv=arrs->get(idx);
        // ex) chart(width, height, 'pie', 'A1:b:2')
        if( isNumberVar(sv) ) {
            wid=toInteger(sv), hei=toInteger(arrs->get(3));
            idx=4;
        } else if( SVCHK('i',2) ) {
            wid=sv->getInt(4), hei=sv->getInt(8);
            idx=3;
        }
        sv=arrs->get(idx++);
        LPCC type=toString(sv);
        Chart* chart = w->insertChart(r, c, QSize(wid,hei));
        if( ccmp(type,"pie") ) chart->setChartType(Chart::CT_Pie);
        else if( ccmp(type,"pie3D") ) chart->setChartType(Chart::CT_Pie3D );
        else if( ccmp(type,"ofpie") ) chart->setChartType(Chart::CT_OfPie );
        else if( ccmp(type,"bar") ) chart->setChartType(Chart::CT_Bar);
        else if( ccmp(type,"bar3D") ) chart->setChartType(Chart::CT_Bar3D );
        else if( ccmp(type,"line") ) chart->setChartType(Chart::CT_Line );
        else if( ccmp(type,"line3D") ) chart->setChartType(Chart::CT_Line3D );
        else if( ccmp(type,"area") ) chart->setChartType(Chart::CT_Pie3D );
        else if( ccmp(type,"area3D") ) chart->setChartType(Chart::CT_Pie3D );
        else if( ccmp(type,"surface") ) chart->setChartType(Chart::CT_Surface );
        else if( ccmp(type,"surface3D") ) chart->setChartType(Chart::CT_Surface3D );
        else if( ccmp(type,"scatter") ) chart->setChartType(Chart::CT_Scatter );
        else if( ccmp(type,"doughnut") ) chart->setChartType(Chart::CT_Doughnut );
        else if( ccmp(type,"bouble") ) chart->setChartType(Chart::CT_Bubble );
        else if( ccmp(type,"stock") ) chart->setChartType(Chart::CT_Stock );
        else if( ccmp(type,"radar") ) chart->setChartType(Chart::CT_Radar );
        sv=arrs->get(idx++);
        if( sv ) {
            LPCC range=excelRange(sv, rst);
            chart->addSeries(CellRange(range));
        }
    } break;
    case 42: {		// groupRows
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        if( !isNumberVar(arrs->get(0)) ) {
            return true;
        }
        int rowStart=toInteger(arrs->get(0)), rowEnd=toInteger(arrs->get(1));
        sv=arrs->get(2);
        if( sv && !isVarTrue(sv) ) {
            w->groupRows(rowStart, rowEnd, false);
        } else {
            w->groupRows(rowStart, rowEnd);
        }
    } break;
    case 43: {		// groupColumns
        if(w==NULL) return true;
        if( arrs==NULL ) return false;
        if( !isNumberVar(arrs->get(0)) ) {
            return true;
        }
        int colStart=toInteger(arrs->get(0)), colEnd=toInteger(arrs->get(1));
        sv=arrs->get(2);
        if( sv && !isVarTrue(sv) ) {
            w->groupColumns(colStart, colEnd, false);
        } else {
            w->groupColumns(colStart, colEnd);
        }
    } break;
    default: return false;
    }
    return true;
}



void excelCellReference(LPCC str, int& r, int& c) {
    static QRegExp re("^\\$?([A-Z]{1,3})\\$?(\\d+)$");
    int pos = 0;
    r = c = 0;
    if( (pos = re.indexIn(KR(str), pos)) != -1) {
        r = re.cap(2).toInt();
        c = col_from_name(re.cap(1));
    }
}

LPCC excelRange(StrVar* sv, StrVar* rst) {
    if( rst==NULL ) rst = gExcelVar.reuse();
    if( sv->findPos(",")==-1 ) return toString(sv);
    int sp=0, ep=0;
    sv = getStrVarPointer(sv, &sp, &ep);
    XParseVar pv(sv,sp,ep);
    LPCC row=NULL, col=NULL;
    while( pv.valid() ) {
        row=pv.NextWord();
        char ch = pv.ch();
        if( ch==',') {
            pv.incr();
            col=pv.NextWord();
            if( isnum(col) ) {
                int num = atoi(col);
                rst->add(Q2A(col_to_name(num)) ).add(row);
            } else  {
                rst->add(col).add(row);
            }
            if( pv.ch()==':' ) {
                rst->add(':');
                pv.incr();
            }
        } else {
            rst->add(row);
            if( ch==':') {
                rst->add(':');
                pv.incr();
            }
        }
    }
    qDebug("excelRange : %s\n", rst->get());
    return rst->get();
}

bool excelCellRange(XFuncNode* fn, LPCC c, StrVar* rst, int incrc, int incrr, bool move, LPCC refr, LPCC refc ) {
    int len=slen(c);
    if( len==0 ) {
        return false;
    }
    LPCC r=NULL;
    int n=0;
    while( n<len ) {
        if( isalpha(c[n]) )
            n++;
        else
            break;
    }
    if( n==0 ) {
        r = c;
        c="A"; n=1;
    } else {
        r = c+n;
    }
    StrVar* rv=fn->gv("@row");
    if( !isnum(r) ) {
        r=toString(rv);
        if( r==NULL ) r="1";
    }
    gfmt.set(c,n).add(r);
    int row, col;
    excelCellReference(gfmt.get(),row, col);
    if( rv==NULL ) {
        fn->GetVar("@row")->setVar('0',0).addInt(row);
    }
    if( incrc!=-1 ) {
        col+=incrc;
        if( refc )
            fn->GetVar(refc)->set(Q2A(col_to_name(col)));
        if( move )		fn->GetVar("@col")->set(Q2A(col_to_name(col)));
        if( incrc ) col--;
    }
    if( incrr!=-1 ) {
        row+=incrr;
        if( refr )
            fn->GetVar(refr)->setVar('0',0).addInt(row);
        if( move )	{
            if( rv==NULL ) rv=fn->GetVar("@row");
            rv->setVar('0',0).addInt(row);
        }
        if( incrr ) row--;
    }
    rst->add(Q2A(col_to_name(col))).format(8,"%d",row);
    return true;
}

#endif
