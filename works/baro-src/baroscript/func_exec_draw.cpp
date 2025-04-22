#include "func_util.h"
#include "../util/canvas_item.h"
#include "../webserver/monitoring.h"
#include "../webserver/WorkerThread.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>
#include <QGraphicsColorizeEffect>
#include <QGraphicsDropShadowEffect>
#include <QRect>
#include <QRectF>


inline QImage* BitmapToImage(HBITMAP hBitmap, StrVar* rst ) {
    HDC     hDC;
    int     iBits;
    WORD    wBitCount;
    DWORD   dwPaletteSize=0,dwBmBitsSize,dwDIBSize;
    BITMAP  Bitmap;
    BITMAPFILEHEADER   bmfHdr;
    BITMAPINFOHEADER   bi;
    LPBITMAPINFOHEADER lpbi;
    HPALETTE     hOldPal=NULL;
    hDC = CreateDC(L"DISPLAY",NULL,NULL,NULL);
    iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
    DeleteDC(hDC);
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else if (iBits <= 24)
        wBitCount = 24;
    else
        wBitCount = 24;
    if (wBitCount <= 8)
        dwPaletteSize=(1<<wBitCount)*sizeof(RGBQUAD);

    GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
    bi.biSize            = sizeof(BITMAPINFOHEADER);
    bi.biWidth           = Bitmap.bmWidth;
    bi.biHeight          = Bitmap.bmHeight;
    bi.biPlanes          = 1;
    bi.biBitCount         = wBitCount;
    bi.biCompression      = BI_RGB;
    bi.biSizeImage         = 0;
    bi.biXPelsPerMeter     = 0;
    bi.biYPelsPerMeter     = 0;
    bi.biClrUsed           = 0;
    bi.biClrImportant      = 0;

    dwBmBitsSize = ((Bitmap.bmWidth*wBitCount+31)/32)*4*Bitmap.bmHeight;
    int bufSize=dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER);
    rst->setVar('i',7).addInt(bufSize);
    qDebug("BitmapToImage buf size=%d, pos=%d", bufSize, rst->length());
    rst->alloc(bufSize);
    lpbi = (LPBITMAPINFOHEADER)rst->get(8);
    /*
    HANDLE hDib  = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    */
    *lpbi = bi;
    HANDLE hPal = GetStockObject(DEFAULT_PALETTE);
    if (hPal) {
        hDC = ::GetDC(NULL);
        hOldPal=SelectPalette(hDC,(HPALETTE)hPal,FALSE);
        RealizePalette(hDC);
    }
    GetDIBits(hDC,hBitmap,0,(UINT)Bitmap.bmHeight,(LPSTR)lpbi+sizeof(BITMAPINFOHEADER)+dwPaletteSize, (BITMAPINFO *)lpbi,DIB_RGB_COLORS);
    if (hOldPal) {
        SelectPalette(hDC, hOldPal, TRUE);
        RealizePalette(hDC);
        ::ReleaseDC(NULL, hDC);
    }
    bmfHdr.bfType = 0x4D42;  // "BM"
    dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize;
    bmfHdr.bfSize = dwDIBSize;
    bmfHdr.bfReserved1 = 0;
    bmfHdr.bfReserved2 = 0;
    bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;

    std::vector<uchar>buffer;
    uchar *p=(uchar*)&bmfHdr;
    buffer.insert(buffer.end(),p,p+sizeof(BITMAPFILEHEADER));
    p=(uchar*)lpbi;
    buffer.insert(buffer.end(),p,p+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize);
    /*
    GlobalUnlock(hDib);
    GlobalFree(hDib);
    */
    QImage* img=new QImage();
    img->loadFromData(buffer.data(),buffer.size());
    return img;
}

inline double distancePoints(const QPointF& pt1, const QPointF& pt2) {
	double hd = (pt1.x() - pt2.x()) * (pt1.x() - pt2.x());
	double vd = (pt1.y() - pt2.y()) * (pt1.y() - pt2.y());
	return std::sqrt(hd + vd);
}

inline QPointF getLineStart(const QPointF& pt1, const QPointF& pt2) {
	QPointF pt;
	float rat = 10.0 / distancePoints(pt1, pt2);
	if (rat > 0.5) {
		rat = 0.5;
	}
	pt.setX((1.0 - rat) * pt1.x() + rat * pt2.x());
	pt.setY((1.0 - rat) * pt1.y() + rat * pt2.y());
	return pt;
}

inline QPointF getLineEnd(const QPointF& pt1, const QPointF& pt2) {
	QPointF pt;
	float rat = 10.0 / distancePoints(pt1, pt2);
	if (rat > 0.5) {
		rat = 0.5;
	}
	pt.setX(rat * pt1.x() + (1.0 - rat)*pt2.x());
	pt.setY(rat * pt1.y() + (1.0 - rat)*pt2.y());
	return pt;
}
inline double mapPointValue(double n, double start1, double stop1, double start2, double stop2) {
	return (n - start1) / (stop1 - start1) * (stop2 - start2) + start2;
}

inline bool applyDrawEffect(QPainter* painter, QRect& rc, QPixmap* src, QImage* srcImg, QGraphicsEffect *effect, int extent, LPCC imageMode, StrVar* rcTarget ) {
	static QGraphicsScene gScene;
	static QGraphicsPixmapItem gPixmapItem;
	if( (src || srcImg) && effect ) {
		gPixmapItem.setPixmap(src?*src:QPixmap::fromImage(*srcImg));
		gPixmapItem.setGraphicsEffect(effect);
		if( gScene.items().size()==0 ) {
			gScene.addItem(&gPixmapItem);
		}
		QSize size=src?src->size():srcImg->size();
		int w=src?src->width(): srcImg->width(), h=src?src->height():srcImg->height();
		QImage res(size+QSize(extent*2, extent*2), QImage::Format_ARGB32);
		res.fill(Qt::transparent);
		QPainter ptr(&res);
		gScene.render(&ptr, QRectF(), QRectF( -extent, -extent, w+(extent*2), h+(extent*2) ) );
		if( checkFuncObject(rcTarget,'i',5) ) {
			QRectF rcDest=getQRectF(rcTarget);
			int x=rc.x(), y=rc.y();
			int sx=rcDest.x(), sy=rcDest.y();
			int sw=rcDest.width(), sh=rcDest.height();
			if( imageMode ) {
				QSize imgSize( sw, sh);
				Qt::TransformationMode mode=ccmp(imageMode,"smooth")? Qt::SmoothTransformation: Qt::FastTransformation;
				Qt::AspectRatioMode ration=Qt::KeepAspectRatioByExpanding;
				painter->drawImage(x, y, res.scaled(imgSize,ration,mode), sx, sy, sw, sh);
			} else {
				painter->drawImage(x, y, res, sx, sy, sw, sh);
			}
		} else {
			painter->drawImage(rc, res );
		}
		return true;
	}
	return false;
}
inline QImage applyEffectToImage(QImage* src, QGraphicsEffect *effect, int extent = 0) {
	 if( src==NULL || effect==NULL ) return QImage();
	 QGraphicsScene scene;
	 QGraphicsPixmapItem item;
	 item.setPixmap(QPixmap::fromImage(*src));
	 item.setGraphicsEffect(effect);
	 scene.addItem(&item);
	 QImage res(src->size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
	 res.fill(Qt::transparent);
	 QPainter ptr(&res);
	 scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src->width()+extent*2, src->height()+extent*2 ) );
	 // qDebug()<<"applyEffectToImage"<< res.size();
	 return res;
}
inline QImage applyEffectToPixmap(QPixmap src, QGraphicsEffect *effect, int extent = 0) {
	 if( src.isNull() || effect==NULL ) return QImage();
	 QGraphicsScene scene;
	 QGraphicsPixmapItem item;
	 item.setPixmap(src);
	 item.setGraphicsEffect(effect);
	 scene.addItem(&item);
	 QImage res(src.size()+QSize(extent*2, extent*2), QImage::Format_ARGB32);
	 res.fill(Qt::transparent);
	 QPainter ptr(&res);
	 scene.render(&ptr, QRectF(), QRectF( -extent, -extent, src.width()+extent*2, src.height()+extent*2 ) );
	 return res;
 }

inline bool setPainterMode(QPainter* painter, LPCC type, bool chk) {
	if( ccmp(type,"antialiasing") ) {
		painter->setRenderHints(QPainter::Antialiasing, chk);
	} else if( ccmp(type,"source") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Source);
	} else if( ccmp(type,"sourceIn") ) {
		painter->setCompositionMode(QPainter::CompositionMode_SourceIn);
	} else if( ccmp(type,"sourceOut") ) {
		painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	} else if( ccmp(type,"sourceTop") ) {
		painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
	} else if( ccmp(type,"clear") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Clear);
	} else if( ccmp(type,"dest") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Destination);
	} else if( ccmp(type,"destIn") ) {
		painter->setCompositionMode(QPainter::CompositionMode_DestinationIn);
	} else if( ccmp(type,"destOut") ) {
		painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
	} else if( ccmp(type,"destTop") ) {
		painter->setCompositionMode(QPainter::CompositionMode_DestinationAtop);
	} else if( ccmp(type,"xor") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Xor);
	} else if( ccmp(type,"plus") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Plus);
	} else if( ccmp(type,"multiply") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Multiply);
	} else if( ccmp(type,"screen") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Screen);
	} else if( ccmp(type,"overlay") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Overlay);
	} else if( ccmp(type,"dark") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Darken);
	} else if( ccmp(type,"light") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Lighten);
	} else if( ccmp(type,"hardLight") ) {
		painter->setCompositionMode(QPainter::CompositionMode_HardLight);
	} else if( ccmp(type,"softLight") ) {
		painter->setCompositionMode(QPainter::CompositionMode_SoftLight);
	} else if( ccmp(type,"dodge") ) {
		painter->setCompositionMode(QPainter::CompositionMode_ColorDodge);
	} else if( ccmp(type,"burn") ) {
		painter->setCompositionMode(QPainter::CompositionMode_ColorBurn);
	} else if( ccmp(type,"difference") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Difference);
	} else if( ccmp(type,"exclusion") ) {
		painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	} else if( ccmp(type,"text") ) {
		painter->setRenderHints(QPainter::TextAntialiasing,chk);
	} else if( ccmp(type,"pixmap") ) {
		painter->setRenderHints(QPainter::SmoothPixmapTransform,chk);
	} else if( ccmp(type,"graphic") ) {
		painter->setRenderHints(QPainter::HighQualityAntialiasing,chk);
	} else {
		return false;
		// painter->setRenderHints(QPainter::Antialiasing,chk);
	}
	return true;
}
inline Qt::ImageConversionFlags getConversionFlags(StrVar* sv) {
	if( sv==NULL ) return Qt::AutoColor;
	LPCC ty=toString(sv);
	return ccmp(ty,"color") ? Qt::ColorOnly: ccmp(ty,"mono") ? Qt::MonoOnly:
		   ccmp(ty,"alpha") ? Qt::AlphaDither_Mask:
		   ccmp(ty,"ordered-alpha") ? Qt::OrderedAlphaDither:
		   ccmp(ty,"diffuse-alpha") ? Qt::DiffuseAlphaDither:
		   ccmp(ty,"dither") ? Qt::Dither_Mask:
		   ccmp(ty,"diffuse") ? Qt::DiffuseDither:
		   ccmp(ty,"threshold") ? Qt::ThresholdDither: Qt::AutoColor;
}
inline QBrush getPainterBrush(StrVar* sv) {
	QBrush br;
    if( sv==NULL || sv->length()==0 || SVCHK('9',0) ) {
		br.setColor(Qt::transparent);
	} else if( SVCHK('n',0) ) {
		TreeNode* node=(TreeNode*)SVO;
		if( node->xstat==FNSTATE_DRAW ) {
			sv=node->gv("@img");
			if( SVCHK('i',9) ) {
				QImage* img=(QImage*)SVO;
				br.setTextureImage(*img);
			}
		}
	} else if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
		br.setStyle(Qt::SolidPattern);
		br.setColor(getQColor(sv) );
	} else if( SVCHK('x',21) ) {
		int pos=sv->find("\f", FUNC_HEADER_POS);
		if( pos>0 ) {
			LPCC val=sv->get(FUNC_HEADER_POS);
			br.setStyle(
				 ccmpl(val,0,"no")? Qt::NoBrush :
				 ccmpl(val,0,"solid")?Qt::SolidPattern :
				 ccmpl(val,0,"dense1")?Qt::Dense1Pattern :
				 ccmpl(val,0,"dense2")?Qt::Dense2Pattern  :
				 ccmpl(val,0,"dense3")?Qt::Dense3Pattern  :
				 ccmpl(val,0,"dense4")?Qt::Dense4Pattern  :
				 ccmpl(val,0,"dense5")?Qt::Dense5Pattern  :
				 ccmpl(val,0,"dense6")?Qt::Dense6Pattern  :
				 ccmpl(val,0,"dense7")?Qt::Dense7Pattern  :
				 ccmpl(val,0,"hor")?Qt::HorPattern  :
				 ccmpl(val,0,"ver")?Qt::VerPattern  :
				 ccmpl(val,0,"cross")?Qt::CrossPattern  :
				 ccmpl(val,0,"diag-b")?Qt::BDiagPattern  :
				 ccmpl(val,0,"diag-f")?Qt::FDiagPattern  :
				 ccmpl(val,0,"diag-cross")?Qt::DiagCrossPattern  : Qt::SolidPattern
			);
			sv->set(sv, pos+1, sv->length());
			if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
				br.setColor(getQColor(sv) );
			}
		}
	} else if( SVCHK('9',0) ) {
		br.setColor(Qt::transparent);
	} else if( SVCHK('i',6) ) {
		QPixmap* pixmap=(QPixmap*)SVO;
		if( pixmap) br.setTexture(*pixmap);
	} else if( SVCHK('i',9) ) {
		QImage* img=(QImage*)SVO;
		if( img ) br.setTextureImage(*img);
	} else {
        LPCC name=toString(sv);
        QPixmap* pixmap=getPixmap(name, true);
		if( pixmap) br.setTexture(*pixmap);
	}
	return br;
}

inline bool drawFillPath(QPainter* p, QPainterPath* path, QRectF rc, XListArr* arrs, int idx) {
	int sz=arrs->size();
	int sty=0;
	StrVar* sv=arrs->get(idx);
	bool bclose=false, bfill=false;
	if(SVCHK('3',0) || SVCHK('3',1)) {
		if(SVCHK('3',1)) bclose=true;
		bfill=true;
		idx++;
	} else {
		bool bdraw=false;
		QPen pen=p->pen();
		for( ; idx<sz; idx++ ) {
			sv=arrs->get(idx);
			if(SVCHK('3',1) || SVCHK('3',0) ) {
				if(SVCHK('3',1)) bclose=true;
				bfill=true;
				idx++;
				break;
			}
			if(!bdraw) bdraw=true;
			if( SVCHK('9',0) ) {
				pen.setColor( Qt::transparent );
			} else if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
				pen.setColor( getQColor(sv) );
			} else if( isNumberVar(sv) ) {
				pen.setWidth( toInteger(sv) );
			} else {
				LPCC type=toString(sv);
				if( slen(type) ) {
					if( sty==0 ) {
						pen.setStyle(
							slen(type)==0 ? Qt::SolidLine :
							ccmp(type,"dash")? Qt::DashLine :
							ccmp(type,"dot")? Qt::DotLine :
							ccmp(type,"dashDot")? Qt::DashDotLine : Qt::SolidLine );
					} else if( sty==1 ) {
						pen.setCapStyle( ccmp(type,"round") ? Qt::RoundCap : ccmp(type,"flat") ? Qt::FlatCap : ccmp(type,"square") ? Qt::SquareCap : Qt::MPenCapStyle );
					} else if( sty==2 ) {
						pen.setJoinStyle( ccmp(type,"round") ? Qt::RoundJoin : ccmp(type,"bevel") ? Qt::BevelJoin : ccmp(type,"miter") ? Qt::MiterJoin : Qt::MPenJoinStyle );
					}
					sty++;
				}
			}
		}
		if(bdraw) {
			p->setPen(pen);
			p->drawPath(*path);
		}
	}
	if(bfill) {
		if(bclose) {
			path->closeSubpath();
		}

		int cnt=sz-idx;
		if( cnt>2 ) {
		   QRect rcCur=rc.toRect();
		   LPCC type=AS(idx++);
		   double a=toDouble(arrs->get(idx++)), b=toDouble(arrs->get(idx++));
		   if( ccmp(type,"gradient") || ccmp(type,"grad") ) {
			  double c=toDouble(arrs->get(idx++)), d=toDouble(arrs->get(idx++));
			  fillGradient(p, rcCur, a, b, c, d,  arrs, idx, path);
		   } else if( ccmp(type,"radial") ) {
			  double c=toDouble(arrs->get(idx++)), d=toDouble(arrs->get(idx++)), e=toDouble(arrs->get(idx++));
			  fillRadialGradient(p, rcCur, a, b, c, d, e, arrs, idx, path);
		   } else if( ccmp(type,"conical") ) {
			  fillConicalGradient(p, rcCur, a, b, arrs, idx, path);
		   }
		} else {
			p->fillPath(*path, getPainterBrush(arrs->get(idx)));
			if( path ) {
			} else {
				p->fillRect(rc, getPainterBrush(arrs->get(idx)) );
			}
		}
	}
}
bool callDraw(QPainter* p, StrVar* var, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, LPCC fnm, XFunc* fc) {
	U32 fref=fc? fc->xfuncRef: 0;
	if( fref==0 ) {
		fref=
			ccmp(fnm,"text")? 1 :
			ccmp(fnm,"image")? 2 :
			ccmp(fnm,"font")? 3 :
			ccmp(fnm,"pen")? 4 :
			ccmp(fnm,"opacity")? 5 :
			ccmp(fnm,"arc")? 6 :
			ccmp(fnm,"line")? 7 :
			ccmp(fnm,"path")? 8 :
			ccmp(fnm,"fill")? 9 :
			ccmp(fnm,"translate")? 10 :
			ccmp(fnm,"scale")? 11 :
			ccmp(fnm,"rotate")? 12 :
			ccmp(fnm,"rect")? 13 :
			ccmp(fnm,"circle")? 14 :
			ccmp(fnm,"roundRect")? 15 :
			ccmp(fnm,"curve")? 17 :
			ccmp(fnm,"html")? 18 :
			ccmp(fnm,"bound")? 19 :
			ccmp(fnm,"endPos")? 20 :
			ccmp(fnm,"textSize")? 21 :
			ccmp(fnm,"clip")? 22 :
			ccmp(fnm,"mode")? 23 :
			ccmp(fnm,"save")? 24 :
			ccmp(fnm,"restore")? 25 :
			ccmp(fnm,"pathPosition")? 26 :
			ccmp(fnm,"brush")? 27 :
			ccmp(fnm,"nextPage")? 28 :
			ccmp(fnm,"pageNumber")? 29 :
			ccmp(fnm,"clearImage")? 30 :
			ccmp(fnm,"offset")? 31:
			ccmp(fnm,"drawAngle")? 32:
			ccmp(fnm,"transform")? 33:
			ccmp(fnm,"region")? 34:
			ccmp(fnm,"imageSize")? 35:
			ccmp(fnm,"pie")? 36:
			ccmp(fnm,"rectLine")? 40:
			ccmp(fnm,"destroy")? 41:
			ccmp(fnm,"end")? 42:
			ccmp(fnm,"resizeDraw")? 43:
			ccmp(fnm,"mapValue")? 44:
			ccmp(fnm,"polygon")? 45:
			ccmp(fnm,"arcPath")? 46:
			ccmp(fnm,"polygonPath")? 47:
            ccmp(fnm,"drawPath")? 48:
            ccmp(fnm,"imageData")? 49:
            ccmp(fnm,"convert")? 50:
            ccmp(fnm,"desktop")? 81:
            ccmp(fnm,"encodeAc")? 82:
            ccmp(fnm,"decodeAc")? 83:
            0;
		if( fc ) fc->xfuncRef=fref;
	}
    if(p==NULL && fref<80 ) {
        return false;
    }
	// qDebug("xxxxx callDraw %s (%d) xxxxx", fnm, fref);
	StrVar* sv=NULL;
	int idx=0, cnt=arrs?arrs->size():0;
	bool bRect=false;
	switch( fref ) {
    case 1: { // text
		if( cnt==0 ) return false;
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		LPCC text=NULL;
		sv=arrs->get(idx++);
		if( SVCHK('3',1) ) {
			text=tn->get("@value");
		} else {
			text=toString(sv);
		}
		if( slen(text) ) {
            QString str=KR(text);
            TreeNode* style=tn->getNode("@style");
            int align=getAlignFlag(AS(idx++));
            if( align==0 )  {
                if( style && style->isNodeFlag(FLAG_SET)) {
                    align=getAlignFlag(style->get("align"));
                }
                if( align==0 ) align=Qt::AlignLeft | Qt::AlignVCenter;
            }
            LPCC ty=AS(idx++);
            if( style && style->isNodeFlag(FLAG_SET) && slen(ty)==0 ) {
                ty=style->get("elided");
            }
            if( slen(ty) ) {
				QFontMetrics fm(p->font());
				if( fm.width(str)>rc.width() ) {
                    str=fm.elidedText(str,
                       ccmp(ty,"middle")? Qt::ElideMiddle:
                       ccmp(ty,"left")? Qt::ElideLeft:
                       ccmp(ty,"right")? Qt::ElideRight: Qt::ElideRight, rc.width(), Qt::TextShowMnemonic );
				}
            }
            if( style && style->isNodeFlag(FLAG_SET) ) {
                sv=style->gv("color");
                if(sv && sv->length() ) {
                    QPen pen(getGlobalColor(sv), 1, Qt::SolidLine);
                    p->setPen(pen);
                }
            }
            if(!str.isEmpty()) {
                p->drawText(rc, align, str);
            }
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 2: { // image
		if( cnt==0 ) {
			return false;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		QPixmap* pixmap=NULL;
        QImage* img=NULL;
		// bool offset=tn->isTrue("@offsetUse");
		sv=arrs->get(idx++);
		if( SVCHK('n',0) ) {
			TreeNode* node=(TreeNode*)SVO;
			if( node->xstat==FNSTATE_DRAW ) {
                sv=node->gv("@img");
            } else {
                sv=node->gv("captureBuffer");
                if(SVCHK('s',1)) {
                    sv=(StrVar*)SVO;
                } else {
                    sv=NULL;
                }
            }
        }
        if( SVCHK('i',6) ) {
			pixmap=(QPixmap*)SVO;
        } else if( SVCHK('i',7) ) {
            pixmap->loadFromData((const uchar *)sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
		} else if( SVCHK('i',9) ) {
            img=(QImage*)SVO;
		} else {
			// image(name, false);
            LPCC name=toString(sv);
			sv=arrs->get(idx);
			if( SVCHK('3',1) || SVCHK('3',0) ) idx++;
            pixmap=getPixmap(name, SVCHK('3',1));
		}
		/*
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		else if( bRect ) {
			QRectF rcDest=getQRectF(var);
			if( !rcDest.contains(rc) ) {
				qDebug() << "-- draw skip image:"<< rc << " dest:"<<rcDest;
				return true;
			}
		}
		*/
		// qDebug() << "-- draw image:"<< rc;
        if( img || pixmap  ) {
			sv=arrs->get(idx++);
            if( SVCHK('i',5) || SVCHK('i',4)) {
                QRectF rcDest=getQRectF(sv);
                if( img ) {
                    Qt::ImageConversionFlags flags=getConversionFlags(sv);
                    p->drawImage(rc, *img, rcDest, flags);
                } else if(pixmap ) {
                    p->drawPixmap(rc, *pixmap, rcDest);
                }
                return true;
            }
            if( isNumberVar(sv) ) {
				// image(name, 10,10 [,100,100] );
				LPCC type=NULL;
				double ox=toDouble(sv), oy=toDouble(arrs->get(idx++));
				double ow=img? img->width(): pixmap->width(), oh=img?img->height(): pixmap->height();
				sv=arrs->get(idx++);
				if( isNumberVar(sv) ) {
					double ww=toDouble(sv), hh=toDouble(arrs->get(idx++));
					if( ow>0 && ww<ow ) ow=ww;
					if( oh>0 && hh<oh ) oh=hh;
					type=AS(idx++);
					sv=arrs->get(idx++);
				} else if(sv) {
					type=toString(sv);
					sv=arrs->get(idx++);
				}
				if( ccmp(type,"fixed") ) {
					rc.setWidth(ow);
					rc.setHeight(oh);
				} else if( ccmp(type,"center") ) {
					double x=rc.x(), y=rc.y();
					if( ow>rc.width() || oh>rc.height() ) {
						if( getFullRectPos(rc.width(), rc.height(), ow, oh, rst ) ) {
							if(!bRect ) rc=getQRectF(rst);
							rc.moveTo(rc.x()+x, rc.y()+y);
						}
					} else {
						if( !bRect ) rc=getQRectF(var);
						x+=(rc.width()-ow)/2.0, y+=(rc.height()-oh)/2.0;
						rc=QRect(x,y,ow,oh);
					}
				}
				// qDebug()<< FMT("xxxxxx image rc xxxxxxx (%f,%f,%f,%f)",ox, oy, ow, oh)<< rc;
				if( img ) {
					Qt::ImageConversionFlags flags=getConversionFlags(sv);
					p->drawImage(rc, *img, QRectF(ox, oy, ow, oh), flags);
				} else if(pixmap ) {
					p->drawPixmap(rc, *pixmap, QRectF(ox, oy, ow, oh));
                }
				return true;
			}
			bool resize=false;
			if(sv) {
				LPCC ty=toString(sv);
				// ex) image("img", "fixed");
				if( ccmp(ty,"fixed") ) {
					int w=0, h=0;
					if( img ) {
						w=img->width(), h=img->height();
					} else if(pixmap ) {
						w=pixmap->width(), h=pixmap->height();
                    }
					rc.setWidth(w);
					rc.setHeight(h);
					ty=AS(idx++);
				}

				if( ccmp(ty,"center") || ccmp(ty, "bottom") || ccmp(ty, "right") ) {
					double iw=0, ih=0;
					if( img ) {
						iw=img->width(), ih=img->height();
					} else if(pixmap ) {
						iw=pixmap->width(), ih=pixmap->height();
                    }
					double x=rc.x(), y=rc.y();
					if( iw>rc.width() || ih>rc.height() ) {
						if( ccmp(ty,"center") ) {
							if( getFullRectPos(rc.width(), rc.height(), iw, ih, rst ) ) {
								if(!bRect ) rc=getQRectF(rst);
								rc.moveTo(rc.x()+x, rc.y()+y);
								resize=true;
							}
						} else if( ccmp(ty, "bottom") ) {
							if( ih<rc.height() ) {
								y=rc.bottom()-ih;
							}
							rc=QRect(x,y,iw,ih);
						} else if( ccmp(ty, "right") ) {
							if( iw<rc.width() ) {
								x=rc.right()-iw;
							}
							rc=QRect(x,y,iw,ih);
						}
					} else {
						if( !bRect ) rc=getQRectF(var);
						if( ccmp(ty,"center") ) {
							x+=(rc.width()-iw)/2.0, y+=(rc.height()-ih)/2.0;
							rc=QRect(x,y,iw,ih);
						}
					}
					ty=AS(idx++);
				 } else if( ccmp(ty, "full") ) {
					resize=true;
					if(!bRect ) rc=getQRectF(var);
					ty=AS(idx++);
				}
				QRect rect=rc.toRect();
				// qDebug() <<FMT("xxxxxx image rc xxxxxxx (%s %s)", ty, resize?"resize":"-") << rect;
				if( ccmp(ty,"blur")) {
					// ex) image("img", "fixed", "blur", radius, extent, rc, type)
					QGraphicsBlurEffect blur;
					double radius=2;
					int extent=toInteger(arrs->get(idx+1));
					sv=arrs->get(idx);
					if( isNumberVar(sv) ) radius=toDouble(sv);
					blur.setBlurRadius(radius);
					applyDrawEffect(p, rect, pixmap, img, &blur, extent, toString(arrs->get(idx+3)), arrs->get(idx+2) );
					return true;
				} else if( ccmp(ty,"colorize")) {
					// ex) image("img", "fixed", "colorize", strength, color, extent, rc, type)
					QGraphicsColorizeEffect effect;
					int extent=toInteger(arrs->get(idx+2));
					int x=toNum(arrs->get(idx), 50);
					effect.setStrength( (qreal)(x/100.0) );
					effect.setColor( getGlobalColor(arrs->get(idx+1)) );
					applyDrawEffect(p, rect, pixmap, img, &effect, extent, toString(arrs->get(idx+4)), arrs->get(idx+3) );
					return true;
				} else if( ccmp(ty,"shadow")) {
					// ex) image("img", "fixed", "shadow", x, y, radius, color, extent, rc, type)
					QGraphicsDropShadowEffect effect;
					int extent=toInteger(arrs->get(idx+5));
					int x=toInteger(arrs->get(idx)), y=toInteger(arrs->get(idx+1));
					int radius=toInteger(arrs->get(idx+3));
					effect.setColor( getGlobalColor(arrs->get(idx+4),"#a0a0a0") );
					effect.setOffset( x, y );
					effect.setBlurRadius(radius);
					applyDrawEffect(p, rect, pixmap, img, &effect, extent, toString(arrs->get(idx+7)), arrs->get(idx+6) );
					return true;
				}
			}
			QRect rect=rc.toRect();
			if( img ) {
                p->drawImage(rect, resize?img->scaled(rect.size(),Qt::KeepAspectRatio):*img);
			} else if( pixmap ) {
				p->drawPixmap(rect, resize?pixmap->scaled(rect.size(),Qt::KeepAspectRatio):*pixmap);
            }
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 3: { // font
		if( cnt==0 ) {
			TreeNode* node=getFontInfo(NULL, NULL);
			rst->setVar('n',0,(LPVOID)node);
			return true;
		}
        int idx=0;
		QFont font=p->font();        
        StrVar* sv=arrs->get(idx++);
        if( SVCHK('n',0) ) {
            TreeNode* node=(TreeNode*)SVO;
            sv=node->gv("@g");
            if( SVCHK('g',0) ) {
                QPainter* painter=(QPainter*)SVO;
                font=painter->font();
            } else {
                sv=arrs->get(idx++);
                if(sv) {
                    int sp=0, ep=0;
                    sv=getStrVarPointer(sv,&sp,&ep);
                    node->setJson(sv,sp,ep);
                }
                fontInfoNode(&font, node );
            }
        } else {
            if( isNumberVar(sv) ) {
                font.setPointSize(toInteger(sv));
            } else if(sv) {
                TreeNode* node=tn->addNode("@style");
                int sp=0, ep=0;
                sv=getStrVarPointer(sv,&sp,&ep);
                node->reuse();
                node->setJson(sv,sp,ep);
                node->setNodeFlag(FLAG_SET);
                fontInfoNode(&font, node, NULL );
            }
        }
		p->setFont(font);
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 4: { // pen
		if( cnt==0 ) {
			p->setPen( QPen(Qt::transparent) );
			return true;
		}
		QPen pen=p->pen();
		int sty=0;
		for( int n=0; n<cnt; n++ ) {
			sv=arrs->get(n);
			if( sv==NULL ) break;
			if( SVCHK('9',0) ) {
				pen.setColor( Qt::transparent );
			} else if( SVCHK('i',3) || sv->charAt(0)=='#' ) {
				pen.setColor( getQColor(sv) );
			} else if( isNumberVar(sv) ) {
				pen.setWidth( toInteger(sv) );
			} else {
				LPCC type=AS(n);
				if( slen(type) ) {
					if( sty==0 ) {
						pen.setStyle(
							slen(type)==0 ? Qt::SolidLine :
							ccmp(type,"dash")? Qt::DashLine :
							ccmp(type,"dot")? Qt::DotLine :
							ccmp(type,"dashDot")? Qt::DashDotLine : Qt::SolidLine );
					} else if( sty==1 ) {
						pen.setCapStyle( ccmp(type,"round") ? Qt::RoundCap : ccmp(type,"flat") ? Qt::FlatCap : ccmp(type,"square") ? Qt::SquareCap : Qt::MPenCapStyle );
					} else if( sty==2 ) {
						pen.setJoinStyle( ccmp(type,"round") ? Qt::RoundJoin : ccmp(type,"bevel") ? Qt::BevelJoin : ccmp(type,"miter") ? Qt::MiterJoin : Qt::MPenJoinStyle );
					}
					sty++;
				}
			}
		}
		p->setPen(pen);
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 5: { // opacity
        if( cnt==0 ) {
			rst->setVar('4',0).addInt(p->opacity());
		} else {
			sv=arrs->get(0);
			if( SVCHK('4',0) ) {
				p->setOpacity(toDouble(sv));
			} else if( isNumberVar(sv) ) {
				int opa = toInteger(sv);
				if( isVarTrue(arrs->get(1)) ) {
					int op=p->opacity();
					op+=opa;
					qreal val=(qreal)(op/100.0);
					p->setOpacity(val);
				} else {
					qreal val=(qreal)(opa/100.0);
					p->setOpacity(val);
				}
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 6:
	case 36: { // arc // pie
		if( cnt==0 ) return false;
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		QPainterPath* path=NULL;
		sv=tn->gv("@path");
		if( SVCHK('d',12) ) {
			path=(QPainterPath*)SVO;
		}
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		bool bMove=false;
		double sa=0.0, se=360.0;
		sv=arrs->get(idx++);
		if( isNumberVar(sv) ) {
			sa=toDouble(sv);
			sv=arrs->get(idx++);
			if( isNumberVar(sv) ) {
				se=toDouble(sv);
			} else if( SVCHK('3',1) ) {
				bMove=true;
			}
		}
		if( bMove && path==NULL ) {
			path=new QPainterPath();
			tn->GetVar("@path")->setVar('d',12,(LPVOID)path);
		}
		if( path ) {
			if( bMove ) {
				path->arcMoveTo(rc, sa);
			} else {
				if( fref==36 ) {
					path->moveTo(rc.center());
					path->arcTo(rc, sa, se);
				} else {
					path->arcTo(rc, sa, se);
				}
				QPointF pt=path->currentPosition();
				tn->GetVar("@linePos")->setVar('i',20).addDouble(pt.x()).addDouble(pt.y());
			}
		} else {
			if( fref==40 ) {
				p->drawPie(rc, sa, se);
			} else {
				p->drawArc(rc, sa, se);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 7: { // line
		if( cnt==0 ) {
			return true;
		}
		QPainterPath* path=NULL;
		sv=tn->gv("@path");
		if( SVCHK('d',12) ) {
			path=(QPainterPath*)SVO;
		}
		sv=arrs->get(0);
		if( SVCHK('i',20) ) {
			double x=sv->getDouble(4), y=sv->getDouble(4+sizeof(double));
			sv=arrs->get(1);
			if( SVCHK('i',20) ) {
				double x1=sv->getDouble(4), y1=sv->getDouble(4+sizeof(double));
				p->drawLine(x, y, x1, y1);
			} else if(path) {
				if( SVCHK('3',1) ) {
					path->moveTo(x,y);
				} else {
					path->lineTo(x,y);
				}
			}
		} else if( SVCHK('a',0) ) {
			XListArr* arr=(XListArr*)SVO;
			QRectF rcDest=getQRectF(var);
			// bool offset=tn->isTrue("@offsetUse");
			for(int n=0; n<arr->size(); n++ ) {
				double x=0, y=0;
				sv=arr->get(n);
				if( SVCHK('i',20) ) {
					x=sv->getDouble(4), y=sv->getInt(4+sizeof(double));
				}
				/*
				if( offset ) {
					x-=rcDest.x(), y-=rcDest.y();
				}
				*/
				if( path ) {
					if( n==0 ) {
						path->moveTo(x,y);
					} else {
						path->lineTo(x,y);
					}
				} else {
					double x1=0, y1=0;
					n++;
					sv=arr->get(n);
					if( SVCHK('i',20) ) {
						x1=sv->getDouble(4), y1=sv->getInt(4+sizeof(double));
						p->drawLine(x, y, x1, y1);
					}
				}
			}
		} else if(isNumberVar(sv) ) {
			double x=toDouble(sv),y=toDouble(arrs->get(1));
			if( path ) {
				sv=arrs->get(2);
				if( SVCHK('3',1) ) {
					path->moveTo(x,y);
				} else {
					path->lineTo(x,y);
				}
			} else if( isNumberVar(arrs->get(2)) ) {
				double x1=toDouble(arrs->get(2)),y1=toDouble(arrs->get(3));
				p->drawLine(x, y, x1, y1);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 8: { // path
		if( cnt==0 ) {
			QPainterPath* path=NULL;
			sv=tn->gv("@path");
			rst->setVar('3',SVCHK('d',12) ? 1:0 );
			return true;
		}
		sv=arrs->get(0);
		if( SVCHK('3',1) || SVCHK('3',0) ) {
			sv=tn->gv("@path");
			if( SVCHK('d',12) ) {
				QPainterPath* path=(QPainterPath*)SVO;
				SAFE_DELETE(path);
				if( SVCHK('3',1)) {
					path=new QPainterPath();
					tn->GetVar("@path")->setVar('d',12,(LPVOID)path);
				}
			} else {
				if( SVCHK('3',1)) {
					QPainterPath* path=new QPainterPath();
					tn->GetVar("@path")->setVar('d',12,(LPVOID)path);
				}
			}
		} else {
			LPCC ty=toString(sv);
			QPainterPath* path=NULL;
			sv=tn->gv("@path");
			if( SVCHK('d',12) ) {
				path=(QPainterPath*)SVO;
			}
			/*
			[ex]
			  d.path('start');
			  draw~~
			  d.path('end', line, close);
			*/
			if( ccmp(ty,"start") || ccmp(ty,"text") ) {
				if( path ) {
					SAFE_DELETE(path);
					sv->reuse();
				}
				p->save();
				path=new QPainterPath();
				tn->GetVar("@path")->setVar('d',12,(LPVOID)path);
				if( ccmp(ty,"text") ) {
					// path('text', 'hello world', 'size:50px, weight:bold, name:fontName');
					LPCC txt=AS(1);
					QFont font=p->font();
					sv=arrs->get(2);
                    if( SVCHK('n',0)) {
                        fontInfoNode(&font, (TreeNode*)SVO);
                        // getFontInfo(&font, toString(sv), p);
					}
					QRect fontBoundingRect = QFontMetrics(font).boundingRect(KR(txt));
					path->addText(-QPointF(fontBoundingRect.center()), font, KR(txt));
				} else {
					LPCC rule=AS(1);
					if( ccmp(rule,"oddEven") )
						path->setFillRule(Qt::OddEvenFill);
					else
						path->setFillRule(Qt::WindingFill);
				}
			} else if( ccmp(ty,"close") ) {
				if(path) {
					path->closeSubpath();
				}
			} else if( ccmp(ty,"end") ) {
				sv=arrs->get(1);
				if( SVCHK('3',1) ) {
					sv=arrs->get(2);
					if( SVCHK('3',1) ) {
						path->closeSubpath();
					}
					p->drawPath(*path);
				}
				if( path ) {
					SAFE_DELETE(path);
					p->restore();
				}
				p->restore();                
			} else if( ccmp(ty,"points") ) {
				if( path ) {
					XListArr* arr=tn->addArray("@pathPoints", true);
					QPolygonF polys=path->toFillPolygon();
					for( int n=0,sz=polys.size(); n<sz; n++ ) {
						QPointF pt=polys.at(n);
						arr->add()->setVar('i',20).addDouble(pt.x()).addDouble(pt.y());
					}
					rst->setVar('a',0,(LPVOID)arr);
				}
		   } else {
				qDebug("#9#draw path type not define(type:%s)", ty);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 9: { // fill
		if( cnt==0 ) {
			sv=tn->gv("@img");
			if( SVCHK('i',9) ) {
				QImage* img=(QImage*)SVO;
				if( img ) img->fill(Qt::transparent);
			}
			rst->setVar('n',0,(LPVOID)tn);
			return true;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		QPainterPath* path=NULL;
		sv=tn->gv("@path");
		if( SVCHK('d',12) ) {
			path=(QPainterPath*)SVO;
		}
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		if( cnt>2 ) {
		   QRect rcCur=rc.toRect();
		   LPCC type=AS(idx++);
		   double a=toDouble(arrs->get(idx++)), b=toDouble(arrs->get(idx++));
		   if( ccmp(type,"gradient") || ccmp(type,"grad") ) {
			  double c=toDouble(arrs->get(idx++)), d=toDouble(arrs->get(idx++));
			  fillGradient(p, rcCur, a, b, c, d,  arrs, idx, path);
		   } else if( ccmp(type,"radial") ) {
			  double c=toDouble(arrs->get(idx++)), d=toDouble(arrs->get(idx++)), e=toDouble(arrs->get(idx++));
			  fillRadialGradient(p, rcCur, a, b, c, d, e, arrs, idx, path);
		   } else if( ccmp(type,"conical") ) {
			  fillConicalGradient(p, rcCur, a, b, arrs, idx, path);
		   }
		} else {
            sv=arrs->get(idx);
			if( path ) {
                p->fillPath(*path, getPainterBrush(sv));
			} else {
                p->fillRect(rc, getPainterBrush(sv) );
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 10: { // translate
		if( cnt==0 ) return true;
		sv=arrs->get(0);
		if( isNumberVar(sv) ) {
			QTransform* form=NULL;
			double dx=toDouble(sv), dy=toDouble(arrs->get(1));
			if( dy==0 ) dy=dx;
			p->translate(dx, dy);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 11: { // scale
		if( cnt==0 ) return true;
		sv=arrs->get(0);
		if( isNumberVar(sv) ) {

			double dx=toDouble(sv), dy=toDouble(arrs->get(1));
			if( dy==0 ) dy=dx;
            if( isVarTrue(arrs->get(2))) {
                QTransform form;
                form.scale(1.5,1.5);
                p->setTransform(form);
            } else {
                p->scale(dx, dy);
            }
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 12: { // rotate
		if( cnt==0 ) return true;
		sv=arrs->get(0);
		if( isNumberVar(sv) ) {
			double angle=toDouble(sv);
			LPCC ty=AS(1);
			if( slen(ty) ) {
				if( ccmp(ty,"x") || ccmp(ty,"y") || ccmp(ty,"z") ) {
					QTransform tf;
					tf.rotateRadians(angle, ccmp(ty,"y")? Qt::YAxis: ccmp(ty,"x")? Qt::XAxis: Qt::ZAxis );
					p->setTransform(tf);
				}
			} else {
				p->rotate(angle);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 13: { // rect
		if( cnt==0 ) {
			rst->add(var);
			return true;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		sv=arrs->get(idx++);
		if( SVCHK('3',1) ) {
			sv=tn->gv("@path");
			if( SVCHK('d',12) ) {
				QPainterPath* path=(QPainterPath*)SVO;
				path->moveTo(rc.topLeft());
				path->lineTo(rc.topRight());
				path->lineTo(rc.bottomRight());
				path->lineTo(rc.bottomLeft());
				path->lineTo(rc.topLeft());
			} else {
				p->drawRect(rc);
			}
		} else {
			p->drawRect(rc);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 14: { // circle
		if( cnt==0 ) {
			return true;
		}
		sv=arrs ? arrs->get(0): NULL;
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		sv=arrs->get(idx++);
		if( SVCHK('3',1) ) {
			sv=tn->gv("@path");
			if( SVCHK('d',12) ) {
				QPainterPath* path=(QPainterPath*)SVO;
				path->addEllipse(rc);
			} else {
				p->drawEllipse(rc);
			}
		} else {
			p->drawEllipse(rc);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 15: { // roundRect
		sv=arrs? arrs->get(0): NULL;
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		double rx=5, ry=5;
		if( arrs ) {
			sv= arrs->get(idx++);
			if( isNumberVar(sv) ) {
				rx=toDouble(sv);
				sv= arrs->get(idx);
				if( isNumberVar(sv) ) {
					ry=toDouble(sv);
				} else {
					ry=rx;
				}
			}
		}
		sv=arrs->get(idx++);
		if( SVCHK('3',1) ) {
			sv=tn->gv("@path");
			if( SVCHK('d',12) ) {
				QPainterPath* path=(QPainterPath*)SVO;
				path->addEllipse(rc);
			} else {
				p->drawRoundedRect(rc, rx, ry, Qt::AbsoluteSize);
			}
		} else {
			p->drawRoundedRect(rc, rx, ry, Qt::AbsoluteSize);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 16: { //

	} break;
	case 17: { // curve
		if( cnt==0 ) {
			return true;
		}
		QPainterPath* path = NULL;
		sv=tn->gv("@path");
		if( SVCHK('d',12) ) {
			path=(QPainterPath*)SVO;
		} else {
			qDebug("#9#draw path not define(function: curve)");
			return true;
		}

		sv=arrs->get(0);
		if( isNumberVar(sv) ) {
			double a=toDouble(arrs->get(0)), b=toDouble(arrs->get(1)), c=toDouble(arrs->get(2)), d=toDouble(arrs->get(3)), e=toDouble(arrs->get(4)), f=toDouble(arrs->get(5));
			path->cubicTo(a,b,c,d,e,f);
		} else if( SVCHK('a',0) ) {
			XListArr* arr=(XListArr*)SVO;
			QList<QPointF> points;
			QPointF p;
			sv=arrs->get(1);
			double factor=isNumberVar(sv)? toDouble(sv): 4;
			// bool offset=tn->isTrue("@offsetUse");
			for( int n=0,sz=arr->size(); n<sz; n++ ) {
				double x=0, y=0;
				sv=arr->get(n);
				if( SVCHK('i',2) ) {
					x=sv->getInt(4), y=sv->getInt(8);
				} else if( SVCHK('i',20) ) {
					x=sv->getDouble(4), y=sv->getDouble(4+sizeof(double));
				} else {
					x=toDouble(sv),y=toDouble(arrs->get(n++));
				}
				/*
				if( offset ) {
					QRectF rcDest=getQRectF(var);
					x-=rcDest.x(), y-=rcDest.y();
				}
				*/
				p = QPointF(x, y);
				if (points.count() > 1 && (n<sz-2) && (distancePoints(points.last(), p) < factor)) {
					continue;
				}
				points.append(p);
			}
			if(points.count() < 3) {
				qDebug("#0#curve point count error (count:%d)\n", points.count());
				return true;
			}
			QPointF pt1, pt2;
			for( int i = 0; i < points.count() - 1; i++) {
				pt1 = getLineStart(points[i], points[i + 1]);
				if (i == 0) {
					path->moveTo(pt1);
				} else {
					path->quadTo(points[i], pt1);
				}
				pt2 = getLineEnd(points[i], points[i + 1]);
				path->lineTo(pt2);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 18: { // html
		if( cnt==0 ) {
			return false;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		LPCC html=AS(idx++);
		if( slen(html) ) {
			QTextDocument doc;
			sv=arrs->get(idx++);
			if( SVCHK('a',0) ) {
				XListArr* arr=(XListArr*)SVO;
				for( int n=0,sz=arr->size(); n<sz; n++ ) {
					sv=arr->get(n);
					if( !SVCHK('x',21) ) continue;
					int pos=sv->find("\f",FUNC_HEADER_POS,64);
					if( pos>FUNC_HEADER_POS  ) {
						LPCC key=gBuf.add(sv->get(FUNC_HEADER_POS),pos-FUNC_HEADER_POS);
						rst->set(sv,pos+1,sv->length());
						if(checkFuncObject(rst,'n',0)) {
							TreeNode* d=(TreeNode*)rst->getObject(FUNC_HEADER_POS);
							sv=d->gv("@img");
							if(SVCHK('i',9)) {
								// qDebug("html add resource: %s",key);
								QImage* img=(QImage*)SVO;
								doc.addResource( QTextDocument::ImageResource, QUrl(KR(key)), QVariant(*img) );
							}
						} else {
							LPCC src=rst->get();
							doc.addResource( QTextDocument::StyleSheetResource, QUrl(KR(key)), KR(src) );
						}
					}
				}
				sv=arrs->get(idx++);
			} else if( SVCHK('n',0) ) {
				TreeNode* resource=(TreeNode*)SVO;
				for( WBoxNode* bn=resource->First(); bn ; bn=resource->Next() ) {
					LPCC name=resource->getCurCode();
					if( name[0]=='@' ) continue;
					sv=&(bn->value);
					if(SVCHK('n',0)) {
						TreeNode* d=(TreeNode*)SVO;
						sv=d->gv("@img");
						if(SVCHK('i',9)) {
							qDebug("html add resource: %s",name );
							QImage* img=(QImage*)SVO;
							doc.addResource( QTextDocument::ImageResource, QUrl(KR(name)), QVariant(*img) );
						}
					} else {
						LPCC src=sv->get();
						doc.addResource( QTextDocument::StyleSheetResource, QUrl(KR(name)), KR(src) );
					}

				}
				sv=arrs->get(idx++);
			}
			bool bPrint=false;
			if( sv ) {
				// ex) html(html, [resource, margin, width], printYN);
				if( isNumberVar(sv) ) {
					doc.setDocumentMargin(toDouble(sv));
					sv=arrs->get(idx++);
					if( isNumberVar(sv) ) {
						doc.setTextWidth(toDouble(sv));
						sv=arrs->get(idx++);
					}
				}
				if( SVCHK('3',1)  ) {
					bPrint=true;
				}
            }
			QPrinter* printer=NULL;
			doc.setHtml(KR(html));
			sv=tn->GetVar("@printer");
			if( SVCHK('p',11) ) {
				printer=(QPrinter*)SVO;
				if( bPrint ) {
					QRect pageRect = printer->pageRect();
					doc.setPageSize(pageRect.size());
				} else if(rc.isValid() ){
					doc.setPageSize(QSizeF(rc.width(),rc.height()));
				}
			} else if(rc.isValid() ){
				doc.setPageSize(QSizeF(rc.width(),rc.height()));
			}
			if( printer && bPrint ) {
				qDebug("#0#html print ... rect:%s, pageRect:%s (%d,%d)\n",
                   toString(bRect?sv:var),
                   toString(setVarRect(rst,printer->pageRect())),
                   doc.documentLayout()->documentSize().width(),
                   doc.documentLayout()->documentSize().height() );
			}
			p->setClipping(true);
			p->setClipRect(rc);
			p->translate(rc.topLeft());
			doc.drawContents(p);
			p->translate(-rc.topLeft());
			p->setClipping(false);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 19: { // bound
		if( cnt==0) {
			setVarRectF(rst, getQRectF(var));
			return false;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		LPCC txt=AS(idx++);
		if( slen(txt) ) {
			LPCC txt=AS(1), ty=AS(2);
			int flag =
				ccmp(ty,"wrap")? Qt::TextWordWrap:
				ccmp(ty,"all")? Qt::TextWrapAnywhere:  Qt::TextWordWrap;
			QRectF rcCur = p->boundingRect(rc,flag,KR(txt));
			setVarRectF(rst, rcCur);
		}
	} break;
	case 20: { // endPos
		if( cnt==0 ) {
			return true;
		}
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		LPCC str = AS(idx++);
		QString txt(KR(str));
		QFontMetrics fm(p->font());
		sv = arrs->get(1);
		int w = fm.width(txt);
		int tw = rc.width();
		if( tw>128 ) tw-=10;
		if( w>tw ) {
			int len = txt.length();
			if( len<2 ) {
				rst->setVar('0',0).addInt(1);
				return true;
			}
			int sp = len/2;
			for(int n=0; n<64; n++) {
				w = fm.width( txt.mid(0,sp) );
				if( w<tw ) break;
				if( sp<2 ) {
					rst->setVar('0',0).addInt(1);
					return true;
				}
				sp/=2;
			}
			int pp = sp;
			for( int n=sp+1; n<len; n++ ) {
				char c = txt.at(n).toLatin1();  // .toAscii();
				if( ISBLANK(c) || IS_OPERATION(c) ) {
					pp = n;
					if( ISBLANK(c) ) pp++;
				}
				w=fm.width(txt.mid(0,n));
				if( w>=tw ) {
					break;
				}
			}
			rst->setVar('0',0).addInt(pp);
		} else {
			rst->setVar('0',0).addInt(1);
		}
	} break;
	case 21: { // textSize
		if( cnt==0 ) {
			return true;
		}
		LPCC str = AS(idx++);
		if( slen(str) ) {
			QString txt(KR(str));
			QFontMetrics fm(p->font());
			rst->setVar('i',2).addInt(fm.width(txt)).addInt(fm.height());
		}
	} break;
	case 22: { // clip
		sv=arrs? arrs->get(0): NULL;
		if( SVCHK('3',0) ) {
			p->setClipping(false);
			return true;
		}
		if( SVCHK('d',13) ) {
			QRegion* reg=(QRegion*)SVO;
			LPCC mode=AS(1);
			if( slen(mode) ) {
				// p->setClipRegion(*reg, ccmp(mode,"intersect")?Qt::IntersectClip: ccmp(mode,"unite")?Qt::UniteClip: Qt::ReplaceClip);
			} else {
				p->setClipRegion(*reg);
			}
			return true;
		}
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		sv=arrs?arrs->get(idx): NULL;
		if( sv==NULL || SVCHK('3',1) ) {
			p->setClipping(true);
			p->setClipRect(rc);
		} else if( SVCHK('3',0) ) {
			p->setClipping(false);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 23: { // mode
		if( cnt==0 ) {
			p->setRenderHints(QPainter::Antialiasing,true);
			return true;
		}
		sv=arrs->get(0);
		if(SVCHK('n',0)) {
			TreeNode* node=(TreeNode*)SVO;
			if( node->xstat==FNSTATE_DRAW ) {
				sv=node->gv("@img");
				if( SVCHK('i',9) ) {
					QImage* destImage=(QImage*)SVO;
					LPCC type = AS(1);
					sv=arrs->get(2);
					if( SVCHK('i',5) ) {
						bRect=true;
						idx=1;
					}
					QRectF rc=getQRectF(bRect?sv:var);
					// qDebug("#0# destImage image type==%s", type);
					setPainterMode(p, type, true);
					p->drawImage(rc, *destImage);
				}
			}
		} else {
			LPCC type = AS(0);
			bool chk = arrs->size()==2 && isVarTrue(arrs->get(1))==false ? false: true;
			setPainterMode(p, type, chk);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 24: { // save
		p->save();
		if(arrs ) {
			if( !fc->isNodeFlag(FFLAG_SAVE) ) {
				int sp=0,ep=0;
				StrVar* sv=getStrVarPointer(arrs->get(0),&sp,&ep);
				if(sp<ep) {
					tn->setJson(sv,sp,ep);
					fc->setNodeFlag(FFLAG_SAVE);
				}
			}
			if( fc->isNodeFlag(FFLAG_SAVE) ) {
				QFont font=p->font();
				fontInfoNode(&font, tn);
				p->setFont(font);
			}
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 25: { // restore
        TreeNode* style=tn->getNode("@style");
        if(style) {
            style->reuseAll(true);
            style->clearNodeFlag(FLAG_SET);
        }
		p->restore();
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 26: { // pathPosition
		QPainterPath* path=NULL;
		sv=tn->gv("@path");
		if( SVCHK('d',12) ) {
			path=(QPainterPath*)SVO;
			QPointF pt=path->currentPosition();
			rst->setVar('i',20).addDouble(pt.x()).addDouble(pt.y());
		}
	} break;
	case 27: { // brush
		if( cnt==0 ) return false;
		if( cnt>2 ) {
		   LPCC type=AS(0);
		   QRect rc=QRect();
		   double a=toDouble(arrs->get(1)), b=toDouble(arrs->get(2));
		   if( ccmp(type,"gradient") || ccmp(type,"grad") ) {
			  double c=toDouble(arrs->get(3)), d=toDouble(arrs->get(4));
			  fillGradient(p, rc, a, b, c, d,  arrs, 5, NULL );
		   } else if( ccmp(type,"radial") ) {
			  double c=toDouble(arrs->get(3)), d=toDouble(arrs->get(4)), e=toDouble(arrs->get(5));
			  fillRadialGradient(p, rc, a, b, c, d, e, arrs, 6, NULL);
		   } else if( ccmp(type,"conical") ) {
			  fillConicalGradient(p, rc, a, b, arrs, 3, NULL);
		   }
		} else {
			p->setBrush(getPainterBrush(arrs->get(0)));
		}
	} break;
	case 28: {		// nextPage
		sv = tn->gv("@printer");
		if( SVCHK('p',11) ) {
			QPrinter* printer = (QPrinter*)SVO;
			// p->end();
			printer->newPage();
			// p->begin(printer);
			tn->incrNum("@pageNum");
		}
	} break;
	case 29: {		// pageNumber
		rst->setVar('0',0).addInt( tn->getInt("@pageNum") );
	} break;
	case 30: {		// clearImage
		sv=tn->gv("@img");
		if( SVCHK('i',9) ) {
			QImage* img=(QImage*)SVO;
			if( img ) img->fill(Qt::transparent);
			tn->GetVar("@noChange")->setVar('3',0);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 31: {		// offset
        tn->GetVar("@offsetUse")->setVar('3', cnt==0||isVarTrue(arrs->get(0)) ? 1 : 0);
	} break;
	case 32: {		// drawAngle
		QRectF rect;
		int w=10, angle=270;
		if( arrs ) {
			sv=arrs->get(0);
			if( SVCHK('i',5) ) {
				rect=getQRectF(sv);
				bRect=true;
				idx=1;
			}
			sv=arrs->get(idx++);
			if( isNumberVar(sv) ) {
				angle=toInteger(sv);
			}
			sv=arrs->get(idx++);
			if( isNumberVar(sv) ) {
				w=toInteger(sv);
			}
		} else {
			rect=getQRectF(var);
		}
		// qDebug("... drawAngle %d, %d", angle, w);

		if( rect.isValid() ) {
			QRect drawingRect;
			/*
			bool offset=tn->isTrue("@offsetUse");
			if( offset ) {
				if( bRect ) {
					QRectF rcDest=getQRectF(var);
					rect.moveTo(rect.x()-rcDest.x(), rect.y()-rcDest.y());
				} else {
					rect.moveTo(0,0);
				}
			}
			*/
			drawingRect.setX(rect.x() + w);
			drawingRect.setY(rect.y() + w);
			drawingRect.setWidth(rect.width() - w * 2);
			drawingRect.setHeight(rect.height() - w * 2);

			QConicalGradient gradient;
			gradient.setCenter(drawingRect.center());
			gradient.setAngle(90);
			gradient.setColorAt(0, QColor(178, 255, 246));
			gradient.setColorAt(1, QColor(5, 44, 50));

			int arcLengthApproximation = w + w / 3;
			QPen pen(QBrush(gradient), w);
			pen.setCapStyle(Qt::RoundCap);
			p->setPen(pen);
			p->drawArc(drawingRect, 90 * 16 - arcLengthApproximation, -angle * 16);
		}
	} break;
	case 33: {  // transform
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 34: {  // region
		XListArr* arr=tn->addArray("@ranges");
        if( cnt==0 ) {
			rst->setVar('a',0,(LPVOID)arr);
			return true;
		}
		LPCC code=AS(0);
		if( ccmp(code,"@delete") ) {
			for(int n=0,sz=arr->size();n<sz; n++ ) {
				getPairValue(arr, n, rst);
				sv=rst;
				if( SVCHK('d',13) ) {
					QRegion* reg=(QRegion*)SVO;
					SAFE_DELETE(reg);
				}
			}
			arr->reuse();
			return true;
		}
		StrVar* var=NULL;
		StrVar* el=arrs->get(1);
		int idx=findPairIndex(arr,code);
		if( idx==-1 ) {
			if( el ) {
				var=arr->add();
			}
		} else {
			getPairValue(arr,idx,rst);
			if( el ) {
				sv=rst;
				if( SVCHK('d',13) ) {
					QRegion* reg=(QRegion*)SVO;
					SAFE_DELETE(reg);
				}
				var=arr->get(idx);
				var->reuse();
			}
		}
		if( el && var ) {
			QRegion* region=NULL;
			sv=el;
			if( SVCHK('a',0) ) {
				XListArr* a=(XListArr*)SVO;
				QPolygon pa;
				int ds=sizeof(double);
				for(int n=0,sz=a->size(); n<sz; n++) {
					sv=a->get(n);
					if( SVCHK('i',20) ) {
						pa<<QPoint((int)sv->getDouble(4), (int)sv->getDouble(4+ds));
					}
				}
				region=new QRegion(pa);
			} else if( SVCHK('i',5) || SVCHK('i',4) ) {
				QRect rc=setQRect(sv);
				LPCC mode=AS(2);
				if( slen(mode) ) mode="rect";
				region=new QRegion(rc, ccmp(mode,"rect")?QRegion::Rectangle: QRegion::Ellipse);
			}
			if( region ) {
				rst->setVar('d',13,(LPVOID)region);
				var->setVar('x',21).add(code).add('\f').add(rst);
			}
		}
	} break;
	case 35: {  // imageSize
		sv=tn->gv("@g");
		if( SVCHK('i',9) ) {
			QImage* img=(QImage*)SVO;
			rst->setVar('i',2).addInt(img->width()).addInt((img->height()));
		}
	} break;
	case 40: { // rectLine
        if( cnt==0 ) return false;
		int cnt=arrs->size();
		int x=0;
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			x=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		/*
		bool offset=tn->isTrue("@offsetUse");
		if( offset ) {
			if( bRect ) {
				QRectF rcDest=getQRectF(var);
				rc.moveTo(rc.x()-rcDest.x(), rc.y()-rcDest.y());
			} else {
				rc.moveTo(0,0);
			}
		}
		*/
		LPCC ty = AS(x++);
		if( slen(ty)==0 )
			ty="0";
		bool bchk = false;
		if( cnt>x ) {
			bchk = true;
			p->save();
			if( cnt==(x+1) ) {
				p->setPen(getQColor(arrs->get(x)) );
			} else {
				QColor clr = getQColor(arrs->get(x++));
				int lsize = toInteger(arrs->get(x++));
				LPCC type = AS(x);
				QPen pen(clr, lsize,
					ccmp(type,"dash")? Qt::DashLine :
					ccmp(type,"dot")? Qt::DotLine :
					ccmp(type,"dashDot")? Qt::DashDotLine : Qt::SolidLine );
				p->setPen(pen);
			}
		}
		QRect r=rc.toRect();
		for(int n=0; n<4; n++, ty++ ) {
			char ch = *ty;
			if( !ch ) break;
			// int num = ch-48;
			switch( ch ) {
			case '1': {
				p->drawLine(r.bottomLeft(), r.topLeft() );
			} break;
			case '2': {
				p->drawLine(r.topLeft(), r.topRight());
			} break;
			case '3': {
				p->drawLine(r.topRight(), r.bottomRight());
			} break;
			case '4': {
				p->drawLine(r.bottomRight(), r.bottomLeft());
			} break;
			case '5': {
				QPoint pt = r.center();
				p->drawLine(QPoint(r.left(),pt.y()), QPoint(r.right(),pt.y()) );
			} break;
			case '6': {
				QPoint pt = r.center();
				p->drawLine(QPoint(pt.x(),r.top()), QPoint(pt.x(),r.bottom()) );
			} break;
			case '7': {
				p->drawLine(r.topLeft(), r.bottomRight());
			} break;
			case '8': {
				p->drawLine(r.topRight(), r.bottomLeft());
			} break;
			case 'x': {
				p->drawLine(r.bottomLeft(), r.topLeft() );
				p->drawLine(r.bottomRight(), r.topRight() );
			} break;
			case 'y': {
				p->drawLine(r.topLeft(), r.topRight() );
				p->drawLine(r.bottomLeft(), r.bottomRight() );
			} break;
			case '0': {
				p->drawLine(r.bottomLeft(), r.topLeft() );
				p->drawLine(r.topLeft(), r.topRight());
				p->drawLine(r.topRight(), r.bottomRight());
				p->drawLine(r.bottomRight(), r.bottomLeft());
			} break;
			default: break;
			}
		}
		if( bchk ) p->restore();
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 41: {  // destroy
		sv=tn->gv("@g");
		if( SVCHK('g',0)) {
			QPainter* painter=(QPainter*)SVO;
			SAFE_DELETE(painter);
			sv->reuse();
		}
		sv=tn->gv("@img");
		if( SVCHK('i',9) ) {
			QImage* img=(QImage*)SVO;
			SAFE_DELETE(img);
			sv->reuse();
		}
		if( arrs ) {
			sv=arrs->get(0);
			if( SVCHK('3',1) && tn->isNodeFlag(FLAG_NEW)) {
				SAFE_DELETE(tn);
			}
		}
	} break;
	case 42: {  // end
		if(p->end() ) {
			bool bDel=false;
			if( arrs ) {
				sv=arrs->get(0);
				if( SVCHK('3',1) ) bDel=true;

			}
			if( bDel) {
				sv=tn->gv("@g");
				if( SVCHK('g',0)) {
					QPainter* painter=(QPainter*)SVO;
					SAFE_DELETE(painter);
					sv->reuse();
				}
			}
			sv=tn->gv("@img");
			if( SVCHK('i',9) ) {
				QImage* img=(QImage*)SVO;
				SAFE_DELETE(img);
				sv->reuse();
			}
		}
	} break;
	case 43: {  // resizeDraw
        if(cnt==0) return true;
		int w=toInteger(arrs->get(0)), h=toInteger(arrs->get(1));
		bool bfill=isVarTrue(arrs->get(2));
		if( w>0 && h>0 ) {
			QImage* img = new QImage(w,h, QImage::Format_ARGB32);
			if( bfill ) {
				img->fill(Qt::transparent);
			}
			// qDebug("#### Draw image create (%d, %d) #########\n\n",w, h);
			QPainter* painter = new QPainter(img);
			sv=tn->gv("@g");
			if( SVCHK('g',0)) {
				QPainter* painter=(QPainter*)SVO;
				SAFE_DELETE(painter);
				sv->reuse();
			}
			sv=tn->gv("@img");
			if( SVCHK('i',9) ) {
				QImage* img=(QImage*)SVO;
				SAFE_DELETE(img);
				sv->reuse();
			}
			tn->xtype=0;
			setVarRectF(tn->GetVar("@rect"), QRectF(0,0,w,h));
			tn->GetVar("@img")->setVar('i',9,(LPVOID)img);
			tn->GetVar("@g")->setVar('g',0,(LPVOID)painter);
			rst->setVar('n',0,(LPVOID)tn);
		}

	} break;
	case 44: { // mapValue
        if(cnt==0) {
			return true;
		}
		int sz=arrs->size();
		double a=toDouble(arrs->get(0));
		double sa=sz>1? toDouble(arrs->get(1)):0;
		double sb=sz>2? toDouble(arrs->get(2)):100;
		double ea=sz>3? toDouble(arrs->get(3)):0;
		double eb=sz>4? toDouble(arrs->get(2)):100;
		rst->setVar('1',0).addUL64(mapPointValue(a,sa,sb,ea,eb));
	} break;
	case 45: { // polygon
		XListArr* points=NULL;
        if(cnt==0) {
			points=tn->addArray("@pathPoints");
		} else {
			sv=arrs->get(0);
			if(SVCHK('a',0)) points=(XListArr*)SVO;
			else points=arrs;
		}
		QPolygonF pa;
		for(int n=0,sz=points->size(); n<sz; n++) {
			int ds=sizeof(long long);
			sv=points->get(n);
			if( SVCHK('i',20) ) {
				pa<<QPointF((float)sv->getDouble(4), (float)sv->getDouble(4+ds));
			}
		}
		if(pa.size()>0) {
			p->drawPolygon(pa);
		}
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 46: { // arcPath
		if( cnt==0 ) return false;
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
		QPainterPath path;
		double sa=0.0, se=360.0;
		sv=arrs->get(idx++);
		if( isNumberVar(sv) ) {
			sa=toDouble(sv);
			sv=arrs->get(idx++);
			if( isNumberVar(sv) ) {
				se=toDouble(sv);
			}
		}
		// path.arcMoveTo(rc, sa);
		path.arcMoveTo(rc, sa);
		path.arcTo(rc, sa, se);
		drawFillPath(p,&path,rc,arrs,idx);
		rst->setVar('n',0,(LPVOID)tn);
	} break;
	case 47: { // polygonPath
		if( cnt==0 ) return false;
		sv=arrs->get(0);
		if( SVCHK('i',5) ) {
			bRect=true;
			idx=1;
		}
		QRectF rc=getQRectF(bRect?sv:var);
        QPainterPath path;
        sv=arrs->get(idx++);
        if(SVCHK('a',0)) {
            XListArr* points=(XListArr*)SVO;
            QPolygonF pa;
            for(int n=0,sz=points->size(); n<sz; n++) {
                int ds=sizeof(long long);
                sv=points->get(n);
                if( SVCHK('i',20) ) {
                    pa<<QPointF((float)sv->getDouble(4), (float)sv->getDouble(4+ds));
                }
            }
            if(pa.size()>0) {
                path.addPolygon(pa);
                drawFillPath(p,&path,rc,arrs,idx);
            } else {
                qDebug("polygonPath path empty !!! %d", idx);
            }
        } else {
            qDebug("polygonPath path not array !!!");
        }

		rst->setVar('n',0,(LPVOID)tn);
	} break;
    case 48: { // drawPath
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( SVCHK('i',5) ) {
            bRect=true;
            idx=1;
        }
        QRectF rc=getQRectF(bRect?sv:var);
        QPainterPath path;
        sv=arrs->get(idx++);
        if(SVCHK('a',0)) {
            XListArr* a=(XListArr*)SVO;
            int n=0;
            bool bfirst=true, bcheck=false;
            qreal sp=0, ep=0;
            while(n<a->size()) {
                sv=a->get(n++);
                if(SVCHK('i',20) || SVCHK('i',2) ) {
                     qreal dx=0, dy=0;
                     if(SVCHK('i',2)) {
                         dx=(double)sv->getInt(4), dy=(double)sv->getInt(8);
                     } else {
                         int sz=sizeof(double);
                         dx=sv->getDouble(4), dy=sv->getDouble(4+sz);
                     }
                     if(bfirst) {
                        path.moveTo(dx,dy);
                     } else {
                        path.lineTo(dx,dy);
                     }
                } else if(SVCHK('i',5) || SVCHK('i',4)) {
                     path.addRect(getQRectF(sv));
                } else {
                    LPCC ty=toString(sv,rst);
                    if(ccmp(ty,"move")) {
                        bfirst=bcheck=true;
                    } else {
                        QRectF rcCur=getQRectF(a->get(n++));
                        if(ccmp(ty,"arc")) {
                            sp=toDouble(a->get(n++)), ep=toDouble(a->get(n++));
                            if(bfirst) {
                                path.arcMoveTo(rcCur,sp);
                            }
                            path.arcTo(rcCur,sp,ep);
                        } else if(ccmp(ty,"round")) {
                            sp=toDouble(a->get(n++)), ep=toDouble(a->get(n++));
                            path.addRoundedRect(rcCur,sp,ep);
                        } else if(ccmp(ty,"rect")) {
                            path.addRect(rcCur);
                        } else if(ccmp(ty,"ellipse")) {
                            path.addEllipse(rcCur);
                        }
                    }
                }
                if(bfirst && !bcheck) bfirst=false;
                if(bcheck) bcheck=false;
            }
            drawFillPath(p,&path,rc,arrs,idx);
        } else {
            qDebug("drawPath path not array !!!");
        }

        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 49: { // imageData
        StrVar* data=NULL;
        if(arrs) {
            data=tn->GetVar("@imageData");
        } else {
            data=rst;
        }
        sv=tn->gv("@img");
        data->reuse();
        if( SVCHK('i',9) ) {
            QImage* img=(QImage*)SVO;
            const int bytesPerLine = img->bytesPerLine();
            const uchar* bits = img->constBits();
            data->addU32(0x8000);
            data->addU32(bytesPerLine);
            data->addU32(img->width());
            data->addU32(img->height());
            for (int y = 0; y < img->height(); ++y) {
                const quint32* scanLine = reinterpret_cast<const quint32*>(bits + (y* bytesPerLine));
                for(int x=0; x<img->width(); x++ ) {
                    data->addU32(scanLine[x]);
                }
            }
        }
        return true;

    } break;
    case 50: { // convert
        if(cnt==0) {
            return true;
        }
        StrVar* data=arrs->get(0);
        if(checkFuncObject(data,'3',1)) {
            data=tn->gv("@imageData");
        }
        if(data==NULL || data->length()<20 ) {
            qDebug("draw convert data size error !!! ");
            return true;
        }
        U32 imageCheck=data->getU32(0);
        int sp=16;
        int w=data->getU32(8), h=data->getU32(12);
        sv=arrs->get(1);
        if(sv==NULL ) {
            XListArr* list = getListArrTemp();
            list->add()->setVar('0',1).addU32(imageCheck);
            list->add()->setVar('0',1).addU32(data->getU32(4));
            list->add()->setVar('0',1).addU32(w);
            list->add()->setVar('0',1).addU32(h);
            rst->setVar('a',0,(LPVOID)list);
            return true;
        }

        if(isNumberVar(sv) && imageCheck==0x8000 ) {
            // convert(data, dx, dy, 'red')
            int dx=toInteger(sv), dy=toInteger(arrs->get(2)), idx=dy*w+dx;
            U32 rgb=data->getU32(sp+(idx*4));
            sv=arrs->get(3);
            if(SVCHK('i',3)) {
                int ar=sv->getInt(4), ag=sv->getInt(8), ab=sv->getInt(12), aa=256;
                if(sv->length()>16) {
                    aa=sv->getInt(16);
                }
                data->addU32( qRgba(ar,ag,ab,aa), sp+(idx*4));
            } else if(sv) {
                LPCC ty=toString(sv);
                if(ccmp(ty,"color") ) {
                    rst->setVar('i',3).addInt(qRed(rgb)).addInt(qGreen(rgb)).addInt(qBlue(rgb)).addInt(qAlpha(rgb));
                } else if(ccmp(ty,"r") || ccmp(ty,"red") ) {
                   rst->setVar('0',0).addInt((rgb >> 16) & 0xff);
                } else if(ccmp(ty,"g") || ccmp(ty,"green") ) {
                    rst->setVar('0',0).addInt((rgb >> 8) & 0xff);
                } else if(ccmp(ty,"b") || ccmp(ty,"blue") ) {
                    rst->setVar('0',0).addInt(rgb & 0xff);
                } else if(ccmp(ty,"a") || ccmp(ty,"alpha") ) {
                    rst->setVar('0',0).addInt(rgb >> 24);
                }
            } else {
                rst->setVar('0',1).addU32(rgb);
            }
        } else {
            // convert(data, 'rgb')
            LPCC ty=toString(sv);
            if(ccmp(ty,"rgb") ) {
                int idx=sp;
                rst->addU32(0x4000);
                rst->addU32(w);
                rst->addU32(w);
                rst->addU32(h);
                for (int y = 0; y < h; ++y) {
                    for(int x=0; x<w; x++ ) {
                        U32 pixel=data->getU32(sp+(4*idx++));
                        int alpha = qAlpha(pixel);
                        // Un-premultiply and convert RGB to BGR.
                        if (alpha == 255)
                            rst->addU32( 0xFF000000 | (qBlue(pixel) << 16) | (qGreen(pixel) << 8) | (qRed(pixel)) );
                        else if (alpha > 0)
                            rst->addU32((alpha << 24) | (((255 * qBlue(pixel)) / alpha) << 16) | (((255 * qGreen(pixel)) / alpha) << 8) | ((255 * qRed(pixel)) / alpha));
                        else
                            rst->addU32(0);
                    }
                }

            } else {
                qDebug("draw convert type not defined!!! (ty:%s)", ty);
            }
        }
    } break;
    case 81: { // desktop
        if(cnt==0) {
            qDebug("QGuiApplication::screens().size() == %d", QGuiApplication::screens().size());
            QScreen* pScreen = QGuiApplication::screens().at(0);
            if(pScreen) {
                var=setVarRectF(rst, pScreen->geometry());
            }
        } else {
            sv=arrs->get(0);
            if( SVCHK('i',5) ) {
                bRect=true;
                idx=1;
            } else {
                QScreen* pScreen = QGuiApplication::screens().at(0);
                if(pScreen) {
                    var=setVarRectF(rst, pScreen->geometry());
                }
            }
        }
        QRectF rc=getQRectF(bRect?sv:var);
        BITMAPINFO dib_define;
        int width=(int)rc.width(), height=(int)rc.height();
        dib_define.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        dib_define.bmiHeader.biWidth = width;
        dib_define.bmiHeader.biHeight = height;
        dib_define.bmiHeader.biPlanes = 1;
        dib_define.bmiHeader.biBitCount = 24;
        dib_define.bmiHeader.biCompression = BI_RGB;
        dib_define.bmiHeader.biSizeImage = (((width * 24 + 31) & ~31) >> 3) * height;
        dib_define.bmiHeader.biXPelsPerMeter = 0;
        dib_define.bmiHeader.biYPelsPerMeter = 0;
        dib_define.bmiHeader.biClrImportant = 0;
        dib_define.bmiHeader.biClrUsed = 0;
        /*
        HBITMAP hbitmap = ::CreateDIBSection(hdc, &dib_define, DIB_RGB_COLORS,
            (void **)&imageData, 0, 0);
        */
        HDC hdc=GetDC(NULL);
        HDC mdc=::CreateCompatibleDC(hdc);
        HBITMAP hbmMem=CreateCompatibleBitmap(hdc,width,height);
        HBITMAP hbmOld=(HBITMAP)::SelectObject(mdc, hbmMem);
        ::BitBlt(mdc, 0, 0, width, height, hdc, (int)rc.left(), (int)rc.top(), SRCCOPY);
        HBITMAP hBitmap=(HBITMAP)::SelectObject(mdc,hbmOld);
        /*
        DWORD bufSize= (width * 3 + 3) / 4 * 4 * height;
        HANDLE hDib  = GlobalAlloc(GHND,bufSize);
        BYTE* pBuffer = (BYTE*)GlobalLock(hDib);
        if(::GetDIBits(mdc, hBitmap, 0, height, pBuffer, &dib_define, DIB_RGB_COLORS) == 0) {
            return false;
        }
        */
        QImage* img=BitmapToImage(hBitmap, rst);
        if(img) {
            QPainter* painter = new QPainter(img);
            sv=tn->GetVar("@img");
            if(SVCHK('i',9)) {
                QImage* prev=(QImage*)SVO;
                SAFE_DELETE(prev);
                sv->reuse();
            }
            setVarRectF(tn->GetVar("@rect"), rc);
            tn->GetVar("@img")->setVar('i',9,(LPVOID)img);
            tn->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        }
        ::DeleteDC(mdc);
        ::DeleteObject(hbmMem);
        ::ReleaseDC(NULL, hdc);
        if(arrs) {
            sv=arrs->get(idx++);
            if(SVCHK('3',1)) {
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
        return true;
    } break;
    case 82: { // encodeAc
        if(cnt==0) {
            qDebug("QGuiApplication::screens().size() == %d", QGuiApplication::screens().size());
            return false;
        }
        sv=arrs->get(0);
        if(SVCHK('i',7)) {
            int inSize=sv->getInt(FUNC_HEADER_POS);
            CArithmeticEncoder ac(sv->get(FUNC_HEADER_POS+4), inSize);
            char* buf=NULL;
            unsigned long bufLen=0;
            ac.EncodeBuffer();
            ac.GetBuffer(buf, bufLen);
            qDebug("encode in size:%d out size: %d", inSize, (int)bufLen);
            rst->setVar('i',7).addInt((int)bufLen).add(buf, (int)bufLen);
        }
        return true;
    } break;
    case 83: { // decodeAc
        if(cnt==0) {
            return true;
        }
        sv=arrs->get(0);
        if(SVCHK('i',7)) {
            int inSize=sv->getInt(FUNC_HEADER_POS);
            CArithmeticEncoder ac(sv->get(FUNC_HEADER_POS+4), inSize);
            char* buf=NULL;
            unsigned long bufLen=0;
            ac.DecodeBuffer();
            ac.GetBuffer(buf, bufLen);
            qDebug("in size:%d out size: %d", inSize, (int)bufLen);
            rst->setVar('i',7).addInt((int)bufLen).add(buf, (int)bufLen);
        }
        return true;
    }

    default: return false;
	}
	return true;
}

bool callDrawFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
	StrVar* sv=tn->gv("@g");
    if( !SVCHK('g',0) ) {
        sv=tn->gv("@rect");
        return callDraw(NULL, sv, arrs, tn, fn, rst, fnm, fc);
    }
	QPainter* painter=(QPainter*)SVO;
	sv=tn->gv("@rect");
	if( SVCHK('i',5) ) {
		return callDraw(painter, sv, arrs, tn, fn, rst, fnm, fc);
	}
	return true;
}

bool fillGradient(QPainter* painter, QRect& rc, double x, double y, double dx, double dy, XListArr* arr, int idx, QPainterPath* path ) {
	if( painter==NULL ) return false;
	QLinearGradient g;
	g.setStart(x, y);
	g.setFinalStop(dx, dy);
	StrVar* sv=NULL;
	for( int n=idx,sz=arr->size();n<sz;n++ ) {
		sv=arr->get(n++);
		if( isNumberVar(sv) ) {
			double sp=toDouble(sv);
			g.setColorAt(sp, getGlobalColor(arr->get(n)) );
		} else break;
	}
	if( path ) {
		painter->fillPath(*path, g);
	} else if( rc.isValid() ) {
		painter->fillRect(rc, g);
	} else {
		painter->setBrush(QBrush(g));
	}
	return true;
}
bool fillRadialGradient(QPainter* painter, QRect& rc, double x, double y, double dx, double dy, double radius, XListArr* arr, int idx, QPainterPath* path ) {
	if( painter==NULL ) return false;
	QRadialGradient g;
	g.setCenter(x, y);
	g.setFocalPoint(dx, dy);
	g.setRadius(radius);
	for( int n=idx,sz=arr->size();n<sz;n++ ) {
		StrVar* sv=arr->get(n++);
		if( isNumberVar(sv) ) {
			double sp=toDouble(sv);
			g.setColorAt(sp, getGlobalColor(arr->get(n)) );
		} else break;
	}
	if( path ) {
		painter->fillPath(*path, g);
	} else if( rc.isValid() ) {
		painter->fillRect(rc, g);
	} else {
		painter->setBrush(QBrush(g));
	}
	return true;
}
bool fillConicalGradient(QPainter* painter, QRect& rc, double x, double y, XListArr* arr, int idx, QPainterPath* path ) {
	if( painter==NULL ) return false;
	QConicalGradient g;
	g.setCenter(x, y);
	for( int n=idx,sz=arr->size();n<sz;n++ ) {
		StrVar* sv=arr->get(n++);
		if( isNumberVar(sv) ) {
			double sp=toDouble(sv);
			g.setColorAt(sp, getGlobalColor(arr->get(n)) );
		} else break;
	}
	if( path ) {
		painter->fillPath(*path, g);
	} else if( rc.isValid() ) {
		painter->fillRect(rc, g);
	} else {
		painter->setBrush(QBrush(g));
	}
	return true;
}
