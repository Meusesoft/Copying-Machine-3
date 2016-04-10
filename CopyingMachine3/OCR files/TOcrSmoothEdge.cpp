//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrThread.h"
#include "TOcrSmoothEdge.h"
//---------------------------------------------------------------------------

TOcrSmoothEdges::TOcrSmoothEdges(HGLOBAL phImageHandle, bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{

    hImageHandle = phImageHandle;

}
//---------------------------------------------------------------------------
void __fastcall TOcrSmoothEdges::Execute()
{
    BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);

    int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
    int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

    lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

    //calculate image numbers
    iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
    iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
    int  iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
    iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

    int iCounter = 0;
    long iPointer;

    for (long y=0; y<iImageHeight; y++) {

        iCounter=0;

        for (long x=0; x<iImageWidth; x++) {

            iPointer = x + (iImageHeight - y -1) *iRowLength;

            if (lpBits[iPointer+2]==0) {
                iCounter=5;
                }
            if (iCounter>0) {
                lpBits[iPointer]=0;
                }

            iCounter--;            
            }
        }

    ReturnValue = 0;

    ::GlobalUnlock(hImageHandle);
   // Terminate();
}
//---------------------------------------------------------------------------
