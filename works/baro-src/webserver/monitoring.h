#ifndef MONITORING_H
#define MONITORING_H

#include "../fobject/FSocket.h"
#include "../fobject/FThread.h"
#include "../fobject/FFile.h"
#include "../baroscript/func_util.h"

// CArithmeticEncoder
#define BITMASK 0x80
#define MSB 0x8000
#define NSB 0x4000
#define USB 0x3FFF

class CDIBFrame
{
public:
    CDIBFrame();
    CDIBFrame(int x,int y,int nBitCount);
    CDIBFrame(BYTE * pBuffer,DWORD dwLen);
    CDIBFrame(const CDIBFrame & rhs);
    ~CDIBFrame();

public:
    CDIBFrame & operator = (const CDIBFrame & rhs);
    operator HBITMAP () {return m_hBkgFrame;}
    operator HDC () {return m_FrameDC;}
    operator LPSTR () {return m_pBkgBits;}
    operator LPSTR * () {return &m_pBkgBits;}
    operator BITMAPINFO * () {return m_pBitmapInfo;}
    operator BITMAPINFOHEADER * () {return m_pBitmapInfoHdr;}

public:
    void Init(int x,int y,int nBitCount);
    void CreateFrame(const BYTE * pBuffer = NULL,DWORD dwLen = 0);
    void DeleteFrame();

public:
    int m_x,m_y,m_nBitCount;
    DWORD m_dwBkgImageBytes;

protected:
    HDC m_FrameDC;
    HBITMAP m_hBkgFrame;
    LPSTR m_pBkgBits;
    HBITMAP m_hLastBkgFrame;
    std::vector<BYTE> m_Buffer;
    BITMAPINFO * m_pBitmapInfo;
    BITMAPINFOHEADER * m_pBitmapInfoHdr;
};

class QDCWithImage {
public:
    QDCWithImage( int width, int height ):
        m_Bitmap( 0 ),
        m_Width( width ),
        m_Height( height ),
        m_BitmapData( 0 ),
        m_DCBitmap( 0 )
    {
        BITMAPINFO bitmapInfo;
        memset( &bitmapInfo, 0, sizeof( bitmapInfo ) );
        bitmapInfo.bmiHeader.biSize         = sizeof( BITMAPINFOHEADER );
        bitmapInfo.bmiHeader.biWidth        = width;
        bitmapInfo.bmiHeader.biHeight       = height;
        bitmapInfo.bmiHeader.biPlanes       = 1;
        bitmapInfo.bmiHeader.biSizeImage    = 0;

        bitmapInfo.bmiHeader.biBitCount     = 32;
        bitmapInfo.bmiHeader.biCompression  = BI_RGB;

        HDC screenDC = GetDC( 0 );

        m_Bitmap = CreateDIBSection( screenDC, &bitmapInfo, DIB_RGB_COLORS,
            reinterpret_cast< void** >( &m_BitmapData ), 0, 0 );
        m_nBufferSize = ((((width * 32) + 31) & ~31) >> 3) * height;
        // biSizeImage = ((((biWidth * biBitCount) + 31) & ~31) >> 3) * biHeight
        ReleaseDC( 0, screenDC );
    }

    ~QDCWithImage()
    {
        DeleteObject( m_Bitmap );
    }
public:
    bool isValid() const
    {
        return ( m_Bitmap != 0 &&
                 m_BitmapData != 0 );
    }
    QImage image() const
    {
        if( !isValid() )
            return QImage();
        return QImage( m_BitmapData, m_Width, m_Height, QImage::Format_RGB32 );
    }

    HDC getDC()
    {
        if( !isValid() || m_DCBitmap != 0 )
            return 0;
        HDC screenDC = GetDC( 0 );
        HDC result = CreateCompatibleDC( screenDC );
        ReleaseDC( 0, screenDC );
        m_DCBitmap = reinterpret_cast< HBITMAP >(SelectObject( result, m_Bitmap ) );
        return result;
    }

    void releaseDC( HDC dc ) {
        if( dc == 0 || m_DCBitmap == 0 )
            return;
        SelectObject( dc, m_DCBitmap );
        DeleteDC( dc );
        m_DCBitmap = 0;
    }

    HBITMAP bitmap()
    {
        return m_Bitmap;
    }



private:
    Q_DISABLE_COPY( QDCWithImage )
    int         m_Width;
    int         m_Height;
public:
    int         m_nBufferSize;
    uchar*      m_BitmapData;
    HBITMAP     m_Bitmap;
    HBITMAP     m_DCBitmap;
};

/*
void drawToDC( HDC outputDC, int width, int height )
{
    QDCWithImage im( width, height );
    QImage         image = im.image();

    {
        QPainter p( &image );

        p.setBrush( QBrush( QColor( 0, 0, 255 ), Qt::SolidPattern ) );
        p.drawRect( rect() );
    }

    HDC bitmapDC = im.getDC();

    BitBlt( outputDC, 0, 0, width, height, bitmapDC, 0, 0, SRCCOPY );

    im.releaseDC( bitmapDC );
}
*/

struct CompressionInfo
{
    char * m_pInBuffer;
    DWORD m_dwSrcBytes;
    char ** m_ppOutBuffer;
    DWORD * m_pdwOutBytes;
};

class CCursor
{
public:
    CCursor() : m_hCursor(NULL) {}
    ~CCursor()
    {
        Destroy();
    }
    operator HCURSOR() {return m_hCursor;}
    void CreateCursor(DWORD dwXHotSpot,DWORD dwYHotSpot,int nWidth,int nHeight,WORD bmMaskPlanes,WORD bmMaskBitsPixel,WORD bmColorPlanes,WORD bmColorBitsPixel,BYTE * pMaskBits,BYTE * pColorBits)
    {
        // Clean up
        Destroy();

        // Create a mask bitmap
        HBITMAP hMask = NULL;
        HBITMAP hColor = NULL;
        /*
        CBitmap Mask;
        if (pMaskBits)
        {
            Mask.CreateBitmap(nWidth,nHeight,bmMaskPlanes,bmMaskBitsPixel,pMaskBits);
            hMask = (HBITMAP)Mask;
        }
        // Create a color bitmap
        CBitmap Color;
        if (pColorBits)
        {
            Color.CreateBitmap(nWidth,nHeight,bmColorPlanes,bmColorBitsPixel,pColorBits);
            hColor = (HBITMAP)Color;
        }
        */

        // Create an Icon from the "color" and "mask" bitmaps
        ICONINFO IconInfo;
        IconInfo.fIcon = FALSE;
        IconInfo.xHotspot = dwXHotSpot;
        IconInfo.yHotspot = dwYHotSpot;
        IconInfo.hbmMask = hMask;
        IconInfo.hbmColor = hColor;

        // Create the cursor
        m_hCursor = CreateIconIndirect(&IconInfo);
    }

protected:
    void Destroy()
    {
        if (m_hCursor)
        {
            DestroyCursor(m_hCursor);
            m_hCursor = NULL;
        }
    }

private:
    HCURSOR m_hCursor;
};

class CArithmeticEncoder
{
public:
    CArithmeticEncoder();
    CArithmeticEncoder(char * pBufferIn,unsigned long ulnSrcCount);
    bool EncodeBuffer(char * pBufferIn,unsigned long ulnSrcCount,char * & pBufferOut,unsigned long & ulnDestCount);
    bool DecodeBuffer(char * pBufferIn,unsigned long ulnSrcCount,char ** ppBufferOut,unsigned long * pulnDestCount,BOOL bAlloc = TRUE);

    void SetBuffer(char * pBufferIn,unsigned long ulnSrcCount);
    void GetBuffer(char * & pBufferOut,unsigned long & ulnDestCount);
    bool EncodeBuffer();
    bool DecodeBuffer();

    ~CArithmeticEncoder();

protected:
    void InitModel();
    void ScaleCounts();
    unsigned int RangeCounts();
    void BuildMap();
    void OutputBit(unsigned short int usiBit,unsigned char & ucOutSymbol,unsigned char & ucBitMask,unsigned long & ulDestCount,char * pBuffer);
    void OutputUnderflowBits(unsigned short int usiBit,unsigned long & ulUnderflowBits,unsigned char & ucOutSymbol,unsigned char & ucBitMask,unsigned long & ulDestCount,char * pBuffer);
    void FlushBitMask(unsigned char & ucBitMask,unsigned char & ucOutSymbol,unsigned long & ulDestCount,char * pBuffer);

protected:
    char * m_pBufferIn;
    unsigned long m_ulnSrcCount;
    char * m_pBufferOut;
    unsigned long m_ulnDestCount;
    unsigned long m_ulnLastBuffer;

private:
    unsigned long m_ac[256];
    unsigned long m_ac2[256];
    unsigned short int m_ar[257];
    unsigned int m_aMap[16384];
};

class MultiThreadedCompression : public FThread {
public:
    MultiThreadedCompression( HANDLE * phHandle,CArithmeticEncoder * pAC, TreeNode* cf );
    ~MultiThreadedCompression();

    TreeNode*		xcf;
    FMutex			xlock;
    HANDLE*         m_phHandle;
    CArithmeticEncoder* m_pAC;
    XListArr		xarr;

public:
    CArithmeticEncoder* GetAC() { return m_pAC; }
    void Run();
    void Final();
    bool exec();
    bool pushEvent(int flag);
};

class CDriveMultiThreadedCompression {
public:
    CDriveMultiThreadedCompression(int nTotalThreads, TreeNode* cf);
    ~CDriveMultiThreadedCompression();
    void SetBuffer(char * pBuffer,DWORD dwTotalBytes,BOOL bEncode);
    void GetBuffer(char ** ppBuffer,DWORD * pdwBytes,BOOL bEncode,BOOL bAlloc = TRUE);
    void GetBufferSize(BOOL bEncode,DWORD & dwBytes);
    bool Encode();
    bool Decode();
public:
    TreeNode* xcf;
protected:
    bool ProcessBuffer(BOOL bEncode);

protected:
    int m_nTotalThreads;
    HANDLE m_arrhAC[8];
    DWORD m_dwBytesPerThread;
    MultiThreadedCompression ** m_ppMultiThreadedCompression;
    DWORD m_dwLastBytes;
    StrVar m_Buffer;
    char * m_pBuffer;
    int m_nIndex;
};

class RefreshThread : public FThread {
public:
    RefreshThread( TreeNode* cf );
    ~RefreshThread();

    TreeNode*		xcf;
    FMutex			xlock;
    XListArr		xarr;

public:
    void Run();
    void Final();
    bool exec();
    bool pushEvent(int flag);
};

#endif // MONITORING_H
