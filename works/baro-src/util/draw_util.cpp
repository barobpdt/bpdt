#include "draw_util.h"
#include "../baroscript/func_util.h"

inline bool readWord( QIODevice &sdevice, unsigned short *target, bool invert=true ) {
    unsigned short t;
    if (sdevice.read((char*)&t, 2) != 2) return false;
    if (invert)
        *target = ((t&255) << 8) | ((t>>8)&255);
    else
        *target = t;
    return true;
}

inline bool exifScanloop( QIODevice &jpegFile, unsigned int &tnOffset, unsigned int &tnLength ) {
    // LOOP THROUGH TAGS
    while (1) {
        unsigned short tagid, tagLength;
        if (!readWord( jpegFile, &tagid) ) return 0;
        if (tagid == JPEG_EOI || tagid == JPEG_SOS) {
                // Data ends
            break;
        }
        if (!readWord( jpegFile, &tagLength) ) return 0;

        if (tagid == JPEG_APP1) {
            char str[6];
            jpegFile.read(str,6 );

            // Store the current position for offset calculation
            int basepos = jpegFile.pos();

            // read tiff - header
            unsigned short tifhead[2];
            for (int h=0; h<2; h++) {
                if (!readWord(jpegFile, &tifhead[h])) return false;
            }
            if (tifhead[0] != 0x4949) {
                return false;
            }

            while (1) {
                unsigned int offset;
                jpegFile.read( (char*)&offset, 4);
                if (offset==0) break;
                jpegFile.seek( basepos + offset );

                unsigned short fields;
                if (!readWord(jpegFile, &fields, false)) return false;
                while (fields>0) {
                    char ifdentry[12];
                    jpegFile.read( ifdentry, 12 );
                    unsigned short tagnumber = (((unsigned short)ifdentry[0]) | (unsigned short)ifdentry[1]<<8);
                            // Offset of the thumbnaildata
                    if (tagnumber == 0x0201) {
                        memcpy( &tnOffset, ifdentry+8, 4 );
                        tnOffset += basepos;

                    } else  // Length of the thumbnaildata
                        if (tagnumber == 0x0202) {
                            memcpy( &tnLength, ifdentry+8, 4 );
                        };
                    fields--;
                    if (tnOffset != 0 && tnLength!=0) return true;
                }
            }
            return false;
        }
        jpegFile.seek( jpegFile.pos() + tagLength-2 );
    }
    return false;
}
bool fetchThumbnail(LPCC filenm, QImage& image) {
    QFile jpegFile(KR(filenm));
    if(!jpegFile.open( QIODevice::ReadOnly	) ) return false;
    unsigned short jpegId;
    if(!readWord( jpegFile, &jpegId )) return false;
    if(jpegId!= JPEG_SOI) return false;
    // JPEG SOI must be here

    unsigned int tnOffset = 0;
    unsigned int tnLength = 0;
    if( exifScanloop( jpegFile, tnOffset, tnLength)) {
            // Goto the thumbnail offset in the file
        jpegFile.seek( tnOffset );
            // Use image reader to decode jpeg-encoded thumbnail
        QByteArray tnArray = jpegFile.read( tnLength );
        QBuffer buf( &tnArray, 0 );
        QImageReader reader(&buf);
        reader.setAutoDetectImageFormat( false );
        reader.setFormat("jpg");
        reader.read(&image);
        jpegFile.close();
        return true;
    }
    return false;
}


inline double toReal(StrVar* sv, double def) {
    double num=toDouble(sv);
    if( def ) {
        if( num==0 ) num=def;
    }
    return num;
}


void drawShadow(QPainter* painter, TreeNode* cf) {
    QRect rc;
    StrVar* sv=cf->gv("rect");
    if( SVCHK('i',4) ) {
        rc.setRect(sv->getInt(4),sv->getInt(8),sv->getInt(12),sv->getInt(16) );
    } else {
        rc.setRect(0,0,100,100);
    }

    qint16 _margin = toNum(cf->gv("margin"), 10);
    qreal _radius=toReal(cf->gv("radius"), 2.0);
    QColor _start=getGlobalColor(cf->gv("startColor"),"#909090");
    QColor _end=getGlobalColor(cf->gv("endColor"),"#EAEAEA");
    qreal _startPosition = (qreal)(toNum(cf->gv("startPos"),0) /100.0);
    qreal _endPosition0 = (qreal)(toNum(cf->gv("endPos"),100) /100.0);
    qreal _endPosition1 = (qreal)(toNum(cf->gv("midPos"),60) /100.0);
    qreal _width =(qreal)rc.width();
    qreal _height=(qreal)rc.height();

    painter->setPen(Qt::NoPen);

    QLinearGradient gradient;
    gradient.setColorAt(_startPosition, _start);
    gradient.setColorAt(_endPosition0, _end);
    // Right
    QPointF right0(_width - _margin, _height / 2);
    QPointF right1(_width, _height / 2);
    gradient.setStart(right0);
    gradient.setFinalStop(right1);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(QPointF(_width - _margin*_radius, _margin), QPointF(_width, _height - _margin)), 0.0, 0.0);
    // Left
    QPointF left0(_margin, _height / 2);
    QPointF left1(0, _height / 2);
    gradient.setStart(left0);
    gradient.setFinalStop(left1);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(QPointF(_margin *_radius, _margin), QPointF(0, _height - _margin)), 0.0, 0.0);
    // Top
    QPointF top0(_width / 2, _margin);
    QPointF top1(_width / 2, 0);
    gradient.setStart(top0);
    gradient.setFinalStop(top1);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(QPointF(_width - _margin, 0), QPointF(_margin, _margin)), 0.0, 0.0);
    // Bottom
    QPointF bottom0(_width / 2, _height - _margin);
    QPointF bottom1(_width / 2, _height);
    gradient.setStart(bottom0);
    gradient.setFinalStop(bottom1);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(QPointF(_margin, _height - _margin), QPointF(_width - _margin, _height)), 0.0, 0.0);
    // BottomRight
    QPointF bottomright0(_width - _margin, _height - _margin);
    QPointF bottomright1(_width, _height);
    gradient.setStart(bottomright0);
    gradient.setFinalStop(bottomright1);
    gradient.setColorAt(_endPosition1, _end);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(bottomright0, bottomright1), 0.0, 0.0);
    // BottomLeft
    QPointF bottomleft0(_margin, _height - _margin);
    QPointF bottomleft1(0, _height);
    gradient.setStart(bottomleft0);
    gradient.setFinalStop(bottomleft1);
    gradient.setColorAt(_endPosition1, _end);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(bottomleft0, bottomleft1), 0.0, 0.0);
    // TopLeft
    QPointF topleft0(_margin, _margin);
    QPointF topleft1(0, 0);
    gradient.setStart(topleft0);
    gradient.setFinalStop(topleft1);
    gradient.setColorAt(_endPosition1, _end);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(topleft0, topleft1), 0.0, 0.0);
    // TopRight
    QPointF topright0(_width - _margin, _margin);
    QPointF topright1(_width, 0);
    gradient.setStart(topright0);
    gradient.setFinalStop(topright1);
    gradient.setColorAt(_endPosition1, _end);
    painter->setBrush(QBrush(gradient));
    painter->drawRoundRect(QRectF(topright0, topright1), 0.0, 0.0);
    // Widget
    painter->setBrush(QBrush("#FFFFFF"));
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundRect(QRectF(QPointF(_margin, _margin), QPointF(_width - _margin, _height - _margin)), _radius, _radius);
}


QPainterPath roundRectPath(const QRect &rect, int radius) {
    // int radius = qMin(rect.width(), rect.height()) / 2;
    int diam = 2 * radius;

    int x1, y1, x2, y2;
    rect.getCoords(&x1, &y1, &x2, &y2);

    QPainterPath path;
    path.moveTo(x2, y1 + radius);
    path.arcTo(QRect(x2 - diam, y1, diam, diam), 0.0, +90.0);
    path.lineTo(x1 + radius, y1);
    path.arcTo(QRect(x1, y1, diam, diam), 90.0, +90.0);
    path.lineTo(x1, y2 - radius);
    path.arcTo(QRect(x1, y2 - diam, diam, diam), 180.0, +90.0);
    path.lineTo(x1 + radius, y2);
    path.arcTo(QRect(x2 - diam, y2 - diam, diam, diam), 270.0, +90.0);
    path.closeSubpath();
    return path;
}

