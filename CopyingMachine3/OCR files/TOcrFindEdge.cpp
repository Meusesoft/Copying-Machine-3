//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TError.h"
#include "TOcrThread.h"
#include "TOcrFindEdge.h"
//---------------------------------------------------------------------------


 TOcrFindEdge::TOcrFindEdge(HGLOBAL phSourceHandle, HGLOBAL phDestinationHandle,
                                      int piTreshold, bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{
    hSourceHandle = phSourceHandle;
    hDestinationHandle = phDestinationHandle;
    iTreshold = piTreshold;

    oError = TError::GetInstance();
}
//---------------------------------------------------------------------------
void
__fastcall TOcrFindEdge::Execute()
{
    void* lpSourceBits;
    void* lpDestinationBits;
    BYTE* lpWorkRow;
    BYTE* lpSourceRow;
    BYTE* lpPallette;
    HGLOBAL hPallette;

    int iReturnValue;

    iReturnValue = -1; //Error

    try {

        MakeSure(hSourceHandle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hSourceHandle!=NULL");
        MakeSure(hDestinationHandle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hDestinationHandle!=NULL");

        BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hSourceHandle);
        BITMAPINFO* lpDestinationBitmapInfo = (BITMAPINFO*)::GlobalLock(hDestinationHandle);

        MakeSure(lpSourceBitmapInfo!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpSourceBitmapInfo");
        MakeSure(lpDestinationBitmapInfo!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpDestinationBitmapInfo");

        //make a copy of the image characteristics to the destination handle

            int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
            int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

            lpSourceBits = lpSourceBitmapInfo->bmiColors + nColors;// * sizeof(RGBQUAD);
            lpDestinationBits = lpDestinationBitmapInfo->bmiColors + nColors;// * sizeof(RGBQUAD);

            if (nColors>0) {

                hPallette = ::GlobalAlloc(GMEM_MOVEABLE, lpSourceBitmapInfo->bmiHeader.biClrUsed);
                MakeSure(hPallette!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hPallette");

                lpPallette = (BYTE*)::GlobalLock(hPallette);
                MakeSure(lpPallette!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpPallette");

                for (int i=0; i<(int)lpSourceBitmapInfo->bmiHeader.biClrUsed; i++) {

                    lpPallette[i] = (lpSourceBitmapInfo->bmiColors[i].rgbRed + lpSourceBitmapInfo->bmiColors[i].rgbBlue + lpSourceBitmapInfo->bmiColors[i].rgbGreen)/3;
                    }
                }

            long iBytesCopy = (long)lpSourceBits - (long)lpSourceBitmapInfo;
            memcpy(lpDestinationBitmapInfo, lpSourceBitmapInfo, iBytesCopy);

            ::GlobalUnlock(hSourceHandle);
            ::GlobalUnlock(hDestinationHandle);


        //calculate image numbers
            long iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
            long iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
            int  iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
            long iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

         //do the search operation for edges

             for (long y = 1; y<iImageHeight-1; y++) {

                lpWorkRow = (BYTE*)lpDestinationBits + (iImageHeight-y-1)*iRowLength;
                //lpWorkRow++;
                lpSourceRow = (BYTE*)lpSourceBits + (iImageHeight-y-1)*iRowLength;

                for (long x = 1; x<iImageWidth-1; x++) {

                    if (lpSourceRow[x] > iTreshold) {

                        lpWorkRow[x] = 0xFF;
                        }
                    else {
                        lpWorkRow[x] = 0x00;
                        }
                    }
                }

/*       for (long y = 1; y<iImageHeight-1; y++) {

            lpWorkRow = (BYTE*)lpDestinationBits + (iImageHeight-y-1)*iRowLength;
            //lpWorkRow++;
            lpSourceRow = (BYTE*)lpSourceBits;// + (iImageHeight-y-1)*iRowLength;

            for (long x = 1; x<iImageWidth-1; x++) {

                if (!CheckPixel(lpSourceRow, x, iImageHeight-y-1, iBitsPerPixel, iRowLength, 32, lpPallette)) {

                    lpWorkRow[x] = 0xFF;

                    }
                }
            }
  */


        iReturnValue = 0;

        ::GlobalUnlock(hPallette);
        ::GlobalFree(hPallette);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    ReturnValue = iReturnValue;
}
//---------------------------------------------------------------------------
/*
//This function returns true if the given pixel is on the edge of something

bool
__fastcall TOcrFindEdge::CheckPixel(BYTE* lpPixel, int x, int y, int piBitsPerPixel, long piRowLength, int piTreshold, BYTE* lpPallette) {

    bool bRetValue;

    bRetValue = false;

    int iMasker;

    iMasker = 0;

    try {

        //if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 0, piTreshold, lpPallette)) iMasker = iMasker + 2;
        //if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 1, piTreshold)) iMasker = iMasker + 4;
        if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 2, piTreshold, lpPallette)) iMasker = iMasker + 16;
        if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 3, piTreshold, lpPallette)) iMasker = iMasker + 128;
        if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 4, piTreshold, lpPallette)) iMasker = iMasker + 64;
        //if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 5, piTreshold)) iMasker = iMasker + 32;
        //if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 6, piTreshold, lpPallette)) iMasker = iMasker + 8;
        //if (ComparePixels(lpPixel, x, y, piBitsPerPixel, piRowLength, 7, piTreshold)) iMasker = iMasker + 1;

        //if (iMasker==0 || iMasker==255) bRetValue = true;

        //if ((iMasker & 7)==7) bRetValue = true;
        //if ((iMasker && 41)==41) bRetValue = true;
        //if ((iMasker && 204)==204) bRetValue = true;
        //if ((iMasker && 148)==148) bRetValue = true;

        //if (iMasker == 2 || iMasker == 8 || iMasker == 16 || iMasker == 64) bRetValue = true;
        //if (iMasker == 10 || iMasker == 18 || iMasker == 80 || iMasker == 72) bRetValue = true;

        if (iMasker !=0) bRetValue = true;

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, "");
        }

    return bRetValue;
    }

//---------------------------------------------------------------------------

//This Function compares to pixels and return true if the difference is more than the treshold
bool
__fastcall TOcrFindEdge::ComparePixels(BYTE* lpPixel, int x, int y, int piBitsPerPixel, long piRowLength, int piAngle, int piTreshold, BYTE* lpPallette) {

    bool bRetValue;
    BYTE Value1, Value2;

    bRetValue = false;

    try {

        MakeSure(piBitsPerPixel!=8, LEVEL_WARNING, ERR_INVALIDPOINTER, "piBitsPerPixel==8");

        Value1 = lpPallette[lpPixel[x + y*piRowLength]];

        switch (piAngle) {

            case 0: {
                //recht erboven
                Value2 = lpPallette[lpPixel[x + (y-1)*piRowLength]];
                break;
                }
            case 1: {
                //rechtsboven
                Value2 = lpPallette[lpPixel[x + 1 + (y-1)*piRowLength]];
                break;
                }
            case 2: {
                //rechts
                Value2 = lpPallette[lpPixel[x + 1 + y*piRowLength]];
                break;
                }
            case 3: {
                //rechtsonder
                Value2 = lpPallette[lpPixel[x + (y+1)*piRowLength]];
                break;
                }
            case 4: {
                //onder
                Value2 = lpPallette[lpPixel[x + (y+1)*piRowLength]];
                break;
                }
            case 5: {
                //linksonder
                Value2 = lpPallette[lpPixel[x - 1 + (y+1)*piRowLength]];
                break;
                }
            case 6: {
                //links
                Value2 = lpPallette[lpPixel[x - 1 + y*piRowLength]];
                break;
                }
            case 7: {
                //linksboven
                Value2 = lpPallette[lpPixel[x - 1 + (y-1)*piRowLength]];
                break;
                }
            }

        bRetValue = (abs(Value1-Value2) > piTreshold);

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, "");
        }
    return bRetValue;
    }
 */
