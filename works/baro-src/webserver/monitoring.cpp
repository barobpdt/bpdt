#include "monitoring.h"

/************************************************************************/
/* CArithmeticEncoder                                                            */
/************************************************************************/

// Construct the frame
CDIBFrame::CDIBFrame() : m_x(0), m_y(0), m_nBitCount(32), m_FrameDC(NULL),
    m_hBkgFrame(NULL), m_pBkgBits(NULL), m_hLastBkgFrame(NULL),
    m_pBitmapInfo(NULL), m_dwBkgImageBytes(0), m_pBitmapInfoHdr(NULL)
{
}

// Construct the frame
CDIBFrame::CDIBFrame(int x,int y,int nBitCount) : m_x(x), m_y(y), m_nBitCount(nBitCount), m_FrameDC(NULL),
    m_hBkgFrame(NULL), m_pBkgBits(NULL), m_hLastBkgFrame(NULL),
    m_pBitmapInfo(NULL), m_dwBkgImageBytes(0), m_pBitmapInfoHdr(NULL)
{
    // Create the frame
    CreateFrame();
}

// Construct the frame from the buffer
CDIBFrame::CDIBFrame(BYTE * pBuffer,DWORD dwLen)
{
    // Create the frame
    CreateFrame(pBuffer,dwLen);
}

// Copy/Assignment constructor
CDIBFrame::CDIBFrame(const CDIBFrame & rhs)
{
    *this = rhs;
}

// Deconstruct the frame
CDIBFrame::~CDIBFrame()
{
    // Delete the frame
    DeleteFrame();
}

// Copy/Assignment operator
CDIBFrame & CDIBFrame::operator = (const CDIBFrame & rhs)
{
    if (this != &rhs)
    {
        // Copy the dimensions
        m_x = rhs.m_x;
        m_y = rhs.m_y;
        m_nBitCount = rhs.m_nBitCount;

        // Create this frame from this others buffer
        const std::vector<BYTE> & Buffer = rhs.m_Buffer;
        const BYTE * pBuffer = NULL;
        DWORD dwLen = (DWORD)Buffer.size();
        if (dwLen)
            pBuffer = &Buffer[0];
        CreateFrame(pBuffer,dwLen);
    }
    return *this;
}

// Set the dimensions and create the frame
void CDIBFrame::Init(int x,int y,int nBitCount) {
    // Test the dimensions
    if (x < 1 || y < 1)
        return;

    // Test for already being initialized
    if (m_FrameDC && m_x == x && m_y == y)
        return;

    // Set the new dimensions
    m_x = x;
    m_y = y;
    m_nBitCount = nBitCount;

    // Create the frame
    CreateFrame();
}

// Create the frame
void CDIBFrame::CreateFrame(const BYTE * pBuffer,DWORD dwLen)
{
    // Cleanup the last frame
    DeleteFrame();

    // Create a DC for the compatible display
    HDC DisplayDC=GetDC(GetDesktopWindow());

    // Create the last frame DC
    m_FrameDC = CreateCompatibleDC(DisplayDC);
    if (m_FrameDC)
    {
        if (!pBuffer)
        {
            // Calculate the size of the bitmap info structure (header + color table)
            DWORD dwLen = (DWORD)((WORD)sizeof(BITMAPINFOHEADER) + (m_nBitCount == 32 ? 0 : (m_nBitCount == 8 ? 256 : 16)) * sizeof(RGBQUAD));

            // Allocate the bitmap structure
            m_Buffer.resize(dwLen,0);
            BYTE * pBuffer = &m_Buffer[0];

            // Set up the bitmap info structure for the DIB section
            m_pBitmapInfo = (BITMAPINFO*)pBuffer;
            m_pBitmapInfoHdr = (BITMAPINFOHEADER*)&(m_pBitmapInfo->bmiHeader);
            m_pBitmapInfoHdr->biSize = sizeof(BITMAPINFOHEADER);
            m_pBitmapInfoHdr->biWidth = m_x;
            m_pBitmapInfoHdr->biHeight = m_y;
            m_pBitmapInfoHdr->biPlanes = 1;
            m_pBitmapInfoHdr->biBitCount = m_nBitCount;
            m_pBitmapInfoHdr->biCompression = BI_RGB;


            if (m_nBitCount == 8)
            {
                // 256 color gray scale color table embedded in the DIB
                RGBQUAD Colors;
                for (int iColor = 0;iColor < 256;iColor++)
                {
                    Colors.rgbBlue = Colors.rgbGreen = Colors.rgbRed = Colors.rgbReserved = iColor;
                    m_pBitmapInfo->bmiColors[iColor] = Colors;
                }
                m_pBitmapInfo->bmiHeader.biClrUsed = 256;
            }
            else if (m_nBitCount == 4)
            {
                // 16 color gray scale color for low bandwidth
                RGBQUAD Colors;
                for (int iColor = 0;iColor < 16;iColor++)
                {
                    Colors.rgbBlue = Colors.rgbGreen = Colors.rgbRed = Colors.rgbReserved = iColor * 16;
                    m_pBitmapInfo->bmiColors[iColor] = Colors;
                }
                m_pBitmapInfo->bmiHeader.biClrUsed = 16;
            }

            // Create the DIB for the frame
            m_hBkgFrame = CreateDIBSection(m_FrameDC,m_pBitmapInfo,DIB_RGB_COLORS,(void**)&m_pBkgBits,NULL,0);

            // Prepare the frame DIB for painting
            m_hLastBkgFrame = (HBITMAP)SelectObject(m_FrameDC, m_hBkgFrame);

            // Initialize the DIB to black
            PatBlt(m_FrameDC,0,0,m_x,m_y,BLACKNESS);
        }
        else
        {
            // Copy the buffer
            m_Buffer.resize(dwLen,0);
            memcpy(&m_Buffer[0],pBuffer,dwLen);

            // Create the DIB from the buffer
            m_pBitmapInfo = (BITMAPINFO *)pBuffer;
            m_pBitmapInfoHdr = &m_pBitmapInfo->bmiHeader;
            m_hBkgFrame = CreateDIBSection(m_FrameDC,m_pBitmapInfo,DIB_RGB_COLORS,(void**)&m_pBkgBits,NULL,0);
            m_hLastBkgFrame = (HBITMAP)SelectObject(m_FrameDC, m_hBkgFrame);
            memcpy(m_pBkgBits,pBuffer + m_pBitmapInfoHdr->biSize,m_pBitmapInfoHdr->biSizeImage);
        }

        // Get the byte storage amount for the DIB bits
        BITMAP bmMask;
        GetObject(m_hBkgFrame,sizeof(BITMAP),&bmMask);
        m_dwBkgImageBytes = bmMask.bmWidthBytes * bmMask.bmHeight;
    }

    // Delete the dc's
    ReleaseDC(GetDesktopWindow(),DisplayDC);
}

// Delete the frame
void CDIBFrame::DeleteFrame()
{
    // Check for a frame to cleanup
    if (m_FrameDC)
    {
        // UnSelect the DIB
        SelectObject(m_FrameDC, m_hLastBkgFrame);

        // Delete the DIB for the frame
        DeleteObject(m_hBkgFrame);
        m_pBkgBits = NULL;

        // Delete the last frame DC
        DeleteDC(m_FrameDC);
    }
}

MultiThreadedCompression::MultiThreadedCompression( HANDLE * phHandle,CArithmeticEncoder * pAC, TreeNode* cf ) : m_phHandle(phHandle), m_pAC(pAC), xcf(cf), FThread() {
    HANDLE & hHandle = *m_phHandle;
    hHandle = CreateEvent(NULL,FALSE,FALSE,NULL);
    exec();
}
MultiThreadedCompression::~MultiThreadedCompression() {
    SAFE_DELETE(m_pAC);
    SAFE_DELETE(xcf);
    Final();
}

bool MultiThreadedCompression::exec() {
    if( !IsRunning() )
        Create();
    return true;
}
void MultiThreadedCompression::Run() {
    while( true ) {
        xlock.EnterMutex();
        int flag=toInteger(xarr.get(0));
        int type=flag&0xFF;
        if(type==2) {
            m_pAC->DecodeBuffer();
        } else {
            m_pAC->EncodeBuffer();
        }
        xlock.LeaveMutex();
        FThread::Sleep(10);
        /*
        HANDLE & hHandle = *m_phHandle;
        SetEvent(hHandle);
        */
    }

}
void MultiThreadedCompression::Final() {
    HANDLE & hHandle = *m_phHandle;
    CloseHandle(hHandle);
}

bool MultiThreadedCompression::pushEvent(int flag)
{
    xlock.EnterMutex();
    xarr.add()->setVar('0',0).addInt(flag);
    xlock.LeaveMutex();
}


// Drive the multithreaded arithmetic encoder
CDriveMultiThreadedCompression::CDriveMultiThreadedCompression(int nTotalThreads, TreeNode* cf) : m_dwLastBytes(0), xcf(cf)
{
    // Set the total number of threads to do work
    m_nIndex = 0;
    m_nTotalThreads = nTotalThreads;

    // Construct the multithreaded driver array for the arithmetic encoder
    m_ppMultiThreadedCompression = new MultiThreadedCompression * [m_nTotalThreads];

    // Construct the arithmetic encoders
    for (int iHandle = 0;iHandle < m_nTotalThreads;++iHandle)
    {
        // Construct the arithmetic encoder
        CArithmeticEncoder * pAC = new CArithmeticEncoder;
        TreeNode* cf= new TreeNode(2);
        // Construct the driver
        MultiThreadedCompression * &pMultiThreadedArithmeticEncoder = m_ppMultiThreadedCompression[iHandle];
        pMultiThreadedArithmeticEncoder = new MultiThreadedCompression(&m_arrhAC[iHandle],pAC,cf);
    }
}

// Clean up the multithreaded arithmetic encoder
CDriveMultiThreadedCompression::~CDriveMultiThreadedCompression()
{
    // Clean up the threads of work

    for (int iHandle = 0;iHandle < m_nTotalThreads;iHandle++)
    {
        // End the thread
        MultiThreadedCompression * pMultiThreadedArithmeticEncoder = m_ppMultiThreadedCompression[iHandle];
        // pMultiThreadedArithmeticEncoder->PostThreadMessage(WM_ENDTHREAD,0,0);
        // Wait for the thread to exit before cleaning it up
        /*
        if (WaitForSingleObject(pMultiThreadedArithmeticEncoder->m_hThread,5000) != WAIT_OBJECT_0)
            TerminateThread(pMultiThreadedArithmeticEncoder->m_hThread,0);
        */
    }

    // Delete the driver array
    delete [] m_ppMultiThreadedCompression;
}



// Set the encode buffer
void CDriveMultiThreadedCompression::SetBuffer(char * pBuffer,DWORD dwTotalBytes,BOOL bEncode)
{
    // Get the amount of bytes to process per thread
    if (bEncode)
        m_dwBytesPerThread = dwTotalBytes / m_nTotalThreads;
    else
    {
        // Get the number of threads used to create this buffer
        memcpy(&m_nTotalThreads,pBuffer,sizeof(m_nTotalThreads));
        pBuffer += sizeof(m_nTotalThreads);
    }

    // Construct the arithmetic encoders
    for (int iHandle = 0;iHandle < m_nTotalThreads;++iHandle)
    {
        // Set the bytes to process
        DWORD dwBytes = 0;
        char * pData = NULL;

        // Test for encoding or decoding
        if (bEncode)
        {
            DWORD dwBegBytes = m_dwBytesPerThread * iHandle;
            DWORD dwEndBytes = dwBegBytes + m_dwBytesPerThread - 1;
            if ((iHandle + 1) == m_nTotalThreads)
                dwEndBytes = dwTotalBytes - 1;
            pData = pBuffer + dwBegBytes;
            dwBytes = dwEndBytes - dwBegBytes + 1;
        }
        else
        {
            // Copy the decode bytes
            memcpy(&dwBytes,pBuffer,sizeof(dwBytes));
            pBuffer += sizeof(dwBytes);

            // Get the buffer
            pData = pBuffer;
            if ((iHandle + 1) != m_nTotalThreads)
                pBuffer += dwBytes;
        }

        // Get the driver
        MultiThreadedCompression * pMultiThreadedCompression = m_ppMultiThreadedCompression[iHandle];
        CArithmeticEncoder * pAC = pMultiThreadedCompression->GetAC();

        // Set the buffer
        pAC->SetBuffer(pData,dwBytes);
    }
}

// Get the size of the output buffer
void CDriveMultiThreadedCompression::GetBufferSize(BOOL bEncode,DWORD & dwBytes)
{
    // Track the total output size, encoding adds in the byte counts, decoding removes them
    dwBytes = bEncode ? sizeof(m_nTotalThreads) : 0;

    // Calculate the output storage
    for (int iHandle = 0;iHandle < m_nTotalThreads;++iHandle)
    {
        // Get the driver
        MultiThreadedCompression * pMultiThreadedCompression = m_ppMultiThreadedCompression[iHandle];
        CArithmeticEncoder * pAC = pMultiThreadedCompression->GetAC();
        // Temporary buffer pointers
        char * pBufferOut = NULL;
        DWORD dwBytesOut = 0;

        // Get the buffer size and data pointer
        pAC->GetBuffer(pBufferOut,dwBytesOut);

        // Increment the output storage requirement
        if (bEncode)
            dwBytes += sizeof(dwBytesOut);
        dwBytes += dwBytesOut;
    }
}

// Get the buffer
void CDriveMultiThreadedCompression::GetBuffer(char ** ppBuffer,DWORD * pdwBytes,BOOL bEncode,BOOL bAlloc)
{
    // Get the total output size
    DWORD dwBytes = 0;
    GetBufferSize(bEncode,dwBytes);
    *pdwBytes = dwBytes;

    // Allocate the output buffer
    if (bAlloc) {
        if (dwBytes > m_dwLastBytes)
        {
            // Allocate a new buffer, release the old
            m_pBuffer = m_Buffer.alloc(dwBytes);
            // Set the new buffer size
            m_dwLastBytes = dwBytes;
        }
    }

    // Allocate the output storage
    char * pBuffer = bAlloc ? m_pBuffer : *ppBuffer;
    DWORD dwPos = 0;

    if (bEncode)
    {
        // Copy the number of threads that created this storage
        memcpy(pBuffer + dwPos,&m_nTotalThreads,sizeof(m_nTotalThreads));
        dwPos += sizeof(m_nTotalThreads);
    }

    // Copy to the output storage
    for (int iHandle = 0;iHandle < m_nTotalThreads;++iHandle)
    {
        // Get the driver
        MultiThreadedCompression * pMultiThreadedCompression = m_ppMultiThreadedCompression[iHandle];

        // Get the compression object
        CArithmeticEncoder * pAC = pMultiThreadedCompression->GetAC();
        // Get the buffer size and data pointer
        char * pBufferOut = NULL;
        DWORD dwBytesOut = 0;
        pAC->GetBuffer(pBufferOut,dwBytesOut);

        if (bEncode)
        {
            // For compression, copy the buffer size to the output storage
            memcpy(pBuffer + dwPos,&dwBytesOut,sizeof(dwBytesOut));
            dwPos += sizeof(dwBytesOut);
        }

        // Copy the buffer to the output storage
        memcpy(pBuffer + dwPos,pBufferOut,dwBytesOut);
        if ((iHandle + 1) < m_nTotalThreads)
            dwPos += dwBytesOut;
    }

    // Set the return
    if (bAlloc)
        *ppBuffer = m_pBuffer;
}

// Encode a buffer
bool CDriveMultiThreadedCompression::Encode()
{
    // Encode
    return ProcessBuffer(TRUE);
}

// Decode a buffer
bool CDriveMultiThreadedCompression::Decode()
{
    // Decode
    return ProcessBuffer(FALSE);
}

// Do the buffer encoding or decoding
bool CDriveMultiThreadedCompression::ProcessBuffer(BOOL bEncode)
{
    // Process the number of threads of work
    m_nIndex++;
    for (int iHandle = 0;iHandle < m_nTotalThreads;iHandle++)
    {
        // Perform the multithreaded arithmetic encoding
        MultiThreadedCompression * pMultiThreadedArithmeticEncoder = m_ppMultiThreadedCompression[iHandle];
        int flag= m_nIndex<<8 || (bEncode?1:2);
        pMultiThreadedArithmeticEncoder->pushEvent(flag); //>p(WM_PROCESSBUFFER,bEncode,(LPARAM)m_bAC);
    }

    // Wait for all the work to complete for this set of ranges
    return true;
}

CArithmeticEncoder::CArithmeticEncoder() :
    m_pBufferOut(NULL), m_ulnDestCount(0), m_ulnLastBuffer(0)
{
}

CArithmeticEncoder::CArithmeticEncoder(char * pBufferIn,unsigned long ulnSrcCount) :
    m_pBufferIn(pBufferIn), m_ulnSrcCount(ulnSrcCount), m_pBufferOut(NULL), m_ulnDestCount(0), m_ulnLastBuffer(0)
{
}

// Set the input buffer, clear the output
void CArithmeticEncoder::SetBuffer(char * pBufferIn,unsigned long ulnSrcCount)
{
    // Set the buffer
    m_pBufferIn = pBufferIn;
    m_ulnSrcCount = ulnSrcCount;
    m_ulnDestCount = 0;
}

// Get the output buffer
void CArithmeticEncoder::GetBuffer(char * & pBufferOut,unsigned long & ulnDestCount)
{
    // Set the buffer
    pBufferOut = m_pBufferOut;
    ulnDestCount = m_ulnDestCount;
}

CArithmeticEncoder::~CArithmeticEncoder()
{
    // Cleanup the output
    if (m_pBufferOut)
        delete [] m_pBufferOut;
}

// Initialze the modeling data
void CArithmeticEncoder::InitModel()
{
    memset(&m_ac,0,sizeof(m_ac));
    memset(&m_ac2,0,sizeof(m_ac2));
    memset(&m_ar,0,sizeof(m_ar));
    memset(&m_aMap,0,sizeof(m_aMap));
}

// Scale the counts for AC
__inline void CArithmeticEncoder::ScaleCounts()
{
    // Make a copy for 0 count checking
    for (int i = 0;i < 256;++i)
        m_ac2[i] = m_ac[i];

    // Scale to restrict to 14 bits for precision
    for (;;)
    {
        unsigned int uiTotal = 0;
        for (int i = 0;i < 256;++i)
        {
            uiTotal += m_ac2[i];
            if (uiTotal > 16383)
            {
                for (int j = 0;j < 256;++j)
                {
                    m_ac2[j] >>= 1;
                    if (m_ac2[j] == 0 && m_ac[j] != 0)
                        m_ac2[j] = 1;
                }
                break;
            }
        }
        if (uiTotal > 16383)
            continue;
        break;
    }
}

// Build the scaled range values
__inline unsigned int CArithmeticEncoder::RangeCounts()
{
    unsigned int uiScale = 0;
    for (int i = 0;i < 256;++i)
    {
        m_ar[i] = uiScale;
        uiScale += m_ac2[i];
    }
    m_ar[256] = uiScale;

    // Return the scale
    return uiScale;
}

// Build the map that maps the range values back to a symbol for fast decoding
__inline void CArithmeticEncoder::BuildMap()
{
    for (unsigned int ui = 0;ui < 256;++ui)
        if (m_ac2[ui])
            for (unsigned short int uiRange = m_ar[ui];uiRange < m_ar[ui + 1];++uiRange)
                m_aMap[uiRange] = ui;
}

// Output a bit
__inline void CArithmeticEncoder::OutputBit(unsigned short int usiBit,unsigned char & ucOutSymbol,unsigned char & ucBitMask,unsigned long & ulDestCount,char * pBuffer)
{
    // Output the most significant bit
    if (usiBit)
        ucOutSymbol |= ucBitMask;
    ucBitMask >>= 1;

    // Test for output
    if (ucBitMask == 0)
    {
        // Output encoded byte
        pBuffer[ulDestCount++] = ucOutSymbol;
        ucOutSymbol = 0;
        ucBitMask = BITMASK;
    }
}

// Output the underflow bits
__inline void CArithmeticEncoder::OutputUnderflowBits(unsigned short int usiBit,unsigned long & ulUnderflowBits,unsigned char & ucOutSymbol,unsigned char & ucBitMask,unsigned long & ulDestCount,char * pBuffer)
{
    // Account for all underflow bits
    while (ulUnderflowBits)
    {
        // Output the most significant bit
        OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBuffer);

        // Decrement the count
        ulUnderflowBits--;
    }
}

// Flush the bitmask, simulating shifting in bits
__inline void CArithmeticEncoder::FlushBitMask(unsigned char & ucBitMask,unsigned char & ucOutSymbol,unsigned long & ulDestCount,char * pBuffer)
{
    while (ucBitMask)
    {
        ucOutSymbol |= ucBitMask;
        ucBitMask >>= 1;
    }

    // Output remaining encoded byte
    pBuffer[ulDestCount++] = ucOutSymbol;
    ucOutSymbol = 0;
    ucBitMask = BITMASK;
}

// Implementation of Encode
bool CArithmeticEncoder::EncodeBuffer(char * pBufferIn,unsigned long ulnSrcCount,char * & pBufferOut,unsigned long & ulnDestCount)
{
    // Initialize the modeling data arrays
    InitModel();

    // Input buffer
    unsigned char * pBuffer = (unsigned char *)pBufferIn;

    // The symbol and count
    unsigned char ucSymbol = 0;
    unsigned int uinSymbol = 0;

    // Counts and scaled counts
    unsigned long ulByte;
    for (ulByte = 0;ulByte < ulnSrcCount;++ulByte)
        uinSymbol += (m_ac[pBuffer[ulByte]]++ ? 0 : 1);
    if (!uinSymbol)
        return false;

    // Scale the counts
    ScaleCounts();

    // Get the count ranges
    unsigned int uiScale = RangeCounts();

    // Allocate the maximum output buffer (Total bytes encoded + Total symbol count + Symbols and counts + Max Output data)
    unsigned long ulnBuffer = 4 + 1 + 1280 + ulnSrcCount;

    // Test for growing the buffer
    if (ulnBuffer > m_ulnLastBuffer)
    {
        if (pBufferOut)
            delete [] pBufferOut;
        pBufferOut = new char[ulnBuffer];
        m_ulnLastBuffer = ulnBuffer;
    }

    // Starting buffer position is at the total symbol count
    unsigned long ulDestCount = 4;

    // Write the count of symbols modeling data
    pBufferOut[ulDestCount++] = (unsigned char)(uinSymbol - 1);

    // Write the symbol counts modeling data
    ucSymbol = 0;
    do
    {
        if (m_ac[ucSymbol])
        {
            pBufferOut[ulDestCount++] = ucSymbol;
            pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0xFF000000) >> 24);
            pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0x00FF0000) >> 16);
            pBufferOut[ulDestCount++] = (unsigned char)((m_ac[ucSymbol] & 0x0000FF00) >> 8);
            pBufferOut[ulDestCount++] = (unsigned char)(m_ac[ucSymbol] & 0x000000FF);
        }
    } while (++ucSymbol);

    // Encode the data
    unsigned short int usiLow = 0,usiHigh = 0xFFFF;
    unsigned int uiRange = 0;
    unsigned long ulUnderflowBits = 0;

    // Output tracking
    unsigned char ucOutSymbol = 0;
    unsigned char ucBitMask = BITMASK;

    // Output bit
    unsigned short int usiBit = 0;

    // Main loop
    for (ulByte = 0;ulByte < ulnSrcCount;++ulByte)
    {
        // Get the symbol
        ucSymbol = pBuffer[ulByte];

        // Calculate the range, high value, and low value
        uiRange = (unsigned int)(usiHigh - usiLow + 1);
        usiHigh = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol + 1]) / uiScale - 1);
        usiLow = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol]) / uiScale);

        // Build onto the output
        for (;;)
        {
            // Check for output
            usiBit = usiHigh & MSB;
            if (usiBit == (usiLow & MSB))
            {
                // Output the most significant bit
                OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

                // Output previous underflow bits first
                if (ulUnderflowBits > 0)
                {
                    // Get the underflow bit
                    usiBit = ~usiHigh & MSB;

                    // Output the underflow bits
                    OutputUnderflowBits(usiBit,ulUnderflowBits,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);
                }
            }
            else if ((usiLow & NSB) && !(usiHigh & NSB))
            {
                // Underflow prevention
                ulUnderflowBits++;
                usiHigh |= NSB;
                usiLow &= USB;
            }
            else
                break;

            // Shift the inputs
            usiHigh = usiHigh << 1;
            usiHigh |= 1;
            usiLow = usiLow << 1;
        }

        // Update the symbol count
        if (!(--m_ac[ucSymbol]))
        {
            // Scale the counts for the removed symbol
            ScaleCounts();

            // Get the range for the counts
            uiScale = RangeCounts();
        }
    }

    // Flush the encoder of the 2 low MSB's and any remaing underflow
    usiBit = usiLow & NSB;
    OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);
    usiBit = ~usiLow & NSB;
    OutputBit(usiBit,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

    // Output the remaining underflow bits
    if (ulUnderflowBits > 0)
        OutputUnderflowBits(usiBit,ulUnderflowBits,ucOutSymbol,ucBitMask,ulDestCount,pBufferOut);

    // Flush out the bitmask
    if (ucBitMask)
        FlushBitMask(ucBitMask,ucOutSymbol,ulDestCount,pBufferOut);

    // Output the total bytes encoded
    pBufferOut[0] = (unsigned char)((ulnSrcCount & 0xFF000000) >> 24);
    pBufferOut[1] = (unsigned char)((ulnSrcCount & 0x00FF0000) >> 16);
    pBufferOut[2] = (unsigned char)((ulnSrcCount & 0x0000FF00) >> 8);
    pBufferOut[3] = (unsigned char)(ulnSrcCount & 0x000000FF);

    // Set the ouput size
    ulnDestCount = ulDestCount;

    return true;
}

// Implementation of Encode
bool CArithmeticEncoder::EncodeBuffer()
{
    return EncodeBuffer(m_pBufferIn,m_ulnSrcCount,m_pBufferOut,m_ulnDestCount);
}

// Implementation of decode
bool CArithmeticEncoder::DecodeBuffer(char * pBufferIn,unsigned long ulnSrcCount,char ** ppBufferOut,unsigned long * pulnDestCount,BOOL bAlloc)
{
    // Initialize the modeling data arrays
    InitModel();

    // Input buffer
    unsigned char * pBuffer = (unsigned char *)pBufferIn;
    unsigned long ulSrcCount = 0;

    // Read the total bytes encoded
    unsigned char uc4 = pBuffer[ulSrcCount++];
    unsigned char uc3 = pBuffer[ulSrcCount++];
    unsigned char uc2 = pBuffer[ulSrcCount++];
    unsigned char uc1 = pBuffer[ulSrcCount++];

    // Calculate the total bytes
    unsigned long ulnBuffer = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;

    // Test for growing the buffer
    if (ulnBuffer > m_ulnLastBuffer)
    {
        if (bAlloc)
        {
            if (*ppBufferOut)
                delete [] *ppBufferOut;
            *ppBufferOut = new char[ulnBuffer];
        }
        m_ulnLastBuffer = ulnBuffer;
    }

    unsigned long ulTotal = ulnBuffer;
    unsigned char * pBufferOut = (unsigned char *)*ppBufferOut;
    *pulnDestCount = ulTotal;

    // The symbol
    unsigned char ucSymbol = 0;

    // Read the count of modeling data
    unsigned int uinSymbol = pBuffer[ulSrcCount++] + 1;

    // Read the modeling data
    for (unsigned int uin = 0;uin < uinSymbol;++uin)
    {
        ucSymbol = pBuffer[ulSrcCount++];
        uc4 = pBuffer[ulSrcCount++];
        uc3 = pBuffer[ulSrcCount++];
        uc2 = pBuffer[ulSrcCount++];
        uc1 = pBuffer[ulSrcCount++];
        unsigned long ulSymbolTotal = (uc4 << 24) + (uc3 << 16) + (uc2 << 8) + uc1;
        m_ac[ucSymbol] = ulSymbolTotal;
    }

    // Scale the counts
    ScaleCounts();

    // Get the range of counts
    unsigned int uiScale = RangeCounts();

    // Build the decode map of range to symbol for fast lookup
    BuildMap();

    // Initialize the code variables
    unsigned short int usiCode = 0;
    unsigned char ucCode = 0;

    // Read the first code word
    ucCode = pBuffer[ulSrcCount++];
    usiCode |= ucCode;
    usiCode <<= 8;
    ucCode = pBuffer[ulSrcCount++];
    usiCode |= ucCode;

    // Initialize the count of decoded characters and code
    unsigned long ulDestCount = 0;

    // Initialize the range
    unsigned short int usiLow = 0,usiHigh = 0xFFFF;
    unsigned int uiRange = 0;
    unsigned long ulUnderflowBits = 0;
    unsigned short int usiCount = 0;

    // The bit mask tracks the remaining bits in the input code buffer
    unsigned char ucBitMask = 0;

    // The main decoding loop
    do
    {
        // Get the range and count for the current arithmetic code
        uiRange = (unsigned int)(usiHigh - usiLow + 1);
        usiCount = (unsigned short int)((((usiCode - usiLow) + 1) * uiScale - 1) / uiRange);

        // Look up the symbol in the map
        ucSymbol = m_aMap[usiCount];

        // Output the symbol
        pBufferOut[ulDestCount++] = ucSymbol;
        if (ulDestCount >= ulTotal)
            break;

        // Calculate the high and low value of the symbol
        usiHigh = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol + 1]) / uiScale - 1);
        usiLow = usiLow + (unsigned short int)((uiRange * m_ar[ucSymbol]) / uiScale);

        // Remove the symbol from the stream
        for (;;)
        {
            if ((usiHigh & MSB) == (usiLow & MSB))
            {
                // Fall through to shifting
            }
            else if ((usiLow & NSB) == NSB && (usiHigh & NSB) == 0)
            {
                // Account for underflow
                usiCode ^= NSB;
                usiHigh |= NSB;
                usiLow &= USB;
            }
            else
                break;

            // Shift out a bit from the low and high values
            usiHigh <<= 1;
            usiHigh |= 1;
            usiLow <<= 1;

            // Shift out a bit from the code
            usiCode <<= 1;

            // Test for a needing an input symbol
            if (ucBitMask)
            {
shift:

                // Test for shifting in bits
                if (ucCode & ucBitMask)
                    usiCode |= 1;
                ucBitMask >>= 1;
            }
            else
            {
                // Load up a new code
                if (ulSrcCount < ulnSrcCount)
                {
                    ucCode = pBuffer[ulSrcCount++];
                    ucBitMask = BITMASK;
                    goto shift;
                }
                else if (ulDestCount < ulTotal)
                {
                    // Helper with the last remaining symbols
                    ucBitMask = BITMASK;
                    goto shift;
                }
            }
        }

        // Update the symbol count
        if (!(--m_ac[ucSymbol]))
        {
            // Scale the counts for the removed symbol
            ScaleCounts();

            // Get the range of counts
            uiScale = RangeCounts();

            // Build the lookup map
            BuildMap();
        }
    } while (ulDestCount < ulTotal);

    return true;
}

// Implementation of decode
bool CArithmeticEncoder::DecodeBuffer()
{
    return DecodeBuffer(m_pBufferIn,m_ulnSrcCount,&m_pBufferOut,&m_ulnDestCount);
}
