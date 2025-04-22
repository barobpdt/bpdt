#ifndef BARCODE_H
#define BARCODE_H
#define BARCORD_USE
#include <QColor>
#include <QPainter>
#include <QGraphicsItem>

#ifdef BARCORD_USE
#include "../zint/zint.h"


class QZint
{
private:

public:
     enum BorderType{NO_BORDER=0, BIND=2, BOX=4};
     enum AspectRatioMode{IgnoreAspectRatio=0, KeepAspectRatio=1, CenterBarCode=2};

public:
    QZint();
    ~QZint();

    int  symbol();
    void setSymbol(int symbol);

    QString text();
    void setText(const QString & text);

    QString primaryMessage();
    void setPrimaryMessage(const QString & primaryMessage);

    void setHeight(int height);
    int height();

    void setWidth(int width);
    int width();

    void setOption3(int option);

    QColor fgColor();
    void setFgColor(const QColor & fgColor);

    QColor bgColor();
    void setBgColor(const QColor & bgColor);

    BorderType borderType();
    void setBorderType(BorderType border);

    int borderWidth();
    void setBorderWidth(int boderWidth);

    int pdf417CodeWords();
    void setPdf417CodeWords(int pdf417CodeWords);

    int securityLevel();
    void setSecurityLevel(int securityLevel);

    float scale();
    void setScale(float scale);

    int mode();
    void setMode(int securityLevel);

    void setInputMode(int input_mode);

    void setWhitespace(int whitespace);

    QString error_message();

    void render(QPainter & painter, const QRectF & paintRect, AspectRatioMode mode=IgnoreAspectRatio);

    const QString & lastError();
    bool hasErrors();

    bool save_to_file(QString filename);

    void setHideText(bool hide);

private:
    void encode();
    int module_set(int y_coord, int x_coord);

private:
    int m_symbol;
    QString m_text;
    QString m_primaryMessage;
    int m_height;
    BorderType m_border;
    int m_borderWidth;
    int m_width;
    int m_securityLevel;
    int m_pdf417CodeWords;
    int m_input_mode;
    QColor m_fgColor;
    QColor m_bgColor;
    QString m_lastError;
    int m_error;
    int m_whitespace;
    zint_symbol * m_zintSymbol;
    float m_scale;
    int m_option_3;
    bool m_hidetext;
};
#endif

#endif // BARCODE_H
