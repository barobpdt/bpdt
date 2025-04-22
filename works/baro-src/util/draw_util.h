#ifndef DRAW_UTIL_H
#define DRAW_UTIL_H
#include "widget_util.h"
#define JPEG_SOI 0xffd8
#define JPEG_SOS 0xffda
#define JPEG_EOI 0xffd9
#define JPEG_APP1 0xffe1

bool fetchThumbnail(LPCC filenm, QImage& image);
void drawShadow(QPainter* painter, TreeNode* cf);
QPainterPath roundRectPath(const QRect &rect, int radius=5);
#endif // DRAW_UTIL_H
