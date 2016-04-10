//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TError.h"
#include "TOcrThread.h"
#include "TOcrAnalyseRectangles.h"
//---------------------------------------------------------------------------
TOcrAnalyseRectangles::TOcrAnalyseRectangles(HGLOBAL phImageHandle, TOcrRectangle* poRectangles, bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{
    hImageHandle = phImageHandle;
    oRectangles = poRectangles;

    oError = TError::GetInstance();
}
//---------------------------------------------------------------------------
void __fastcall TOcrAnalyseRectangles::Execute()
{
    //Analyse all rectangles and see if they are
    //Characters, Images or Inverted Characters

    int iReturnValue;

    iReturnValue = -1; //Error

    try {

        MakeSure(hImageHandle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hImageHandle!=NULL");

        BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);

        MakeSure(lpSourceBitmapInfo!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpSourceBitmapInfo!=NULL");

        int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
        int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

        lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

        //calculate image numbers
        iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
        iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
        int  iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
        iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);




        //see 'A word extraction algorithm for machine-printed documents using
        //     a 3D neighborhood graph model' by H.Park, S.Ok, Y. Yu and H.CHo, 2001
        //     International Journal on Document Analysis and Recognition

        double dAvgArea;
        TOcrRectangle* oChild;
        const  double cL0 = 0.8;
        const  double cD0 = 0.7;
        const  double cC1 = -1.09;
        const  double cC2 = 2.8;

        dAvgArea = oRectangles->AverageAreaChildren();

        for (int i=0; i<oRectangles->GetNumberChilds(); i++) {

             oChild = oRectangles->GetChild(i);

             oChild->eType = eCharacter;

             if ((oChild->width * oChild->height) > dAvgArea * 25) {
                oChild->eType = eImageRect;
                }

    /*         //step 1, compare the area of the rectangle
             //with the average rectangle size
             if ((oChild->width * oChild->height) <= dAvgArea) {
                oChild->eType = eCharacter;
                }
             else {
                //we have a rather large rectangle, proceed to step 2
                //checking of elongation.
                Elong = (double)min(oChild->width, oChild->height) / (double)max(oChild->width, oChild->height);

                if (Elong <= cL0) {
                    //Step 3, compare density. Pictures normally have larger
                    //density than characters.
                    dDensity = (double)oChild->numberpixels / (double)(oChild->width * oChild->height);

                    if (dDensity >= cD0) {
                        oChild->eType = eImage;
                        }
                    else {
                        if (cC1 * CalculateRowVariance(oChild) + cC2 <= CalculateColumnVariance(oChild)) {
                            oChild->eType = eInvertedCharacter;
                            }
                        else {
                            oChild->eType = eImage;
                            }
                        }
                    }
                else {
                    if (cC1 * CalculateRowVariance(oChild) + cC2 <= CalculateColumnVariance(oChild)) {
                        oChild->eType = eCharacter;
                        }
                    else {
                        oChild->eType = eImage;
                        }
                    }

                } */
            }

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    ReturnValue = iReturnValue;

    ::GlobalUnlock(hImageHandle);
}
//---------------------------------------------------------------------------
double
__fastcall TOcrAnalyseRectangles::CalculateRowVariance(TOcrRectangle* poRectangle) {

    vector<long> iRunsPerRow;
    long iRuns;
    long lPointer;
    BYTE iCurrentRun;
    double dAverage;
    double dVariance;

    //Count number of pixels per row
    for (int i=0; i<poRectangle->height; i++) {

       lPointer = poRectangle->x + (iImageHeight - poRectangle->y - i -1) * iRowLength;

        iCurrentRun = lpBits[lPointer];
        iRuns = 1;

        for (int j=0; j<poRectangle->width; j++) {

            if (abs(lpBits[lPointer]) != abs(iCurrentRun)) {
                iRuns++;
                iCurrentRun = lpBits[lPointer];
                }

            lPointer++;
            }

        iRunsPerRow.push_back(iRuns);
        }

    //calculate average runs
    dAverage = 0;
    for (unsigned int i=0; i<iRunsPerRow.size()-1; i++) {
        dAverage += abs(iRunsPerRow[i+1] - iRunsPerRow[i]);
        }
    dAverage = dAverage / (iRunsPerRow.size()-1);

    //calculate variance;
    dVariance = 0;
    for (unsigned int i=0; i<iRunsPerRow.size()-1; i++) {
        dVariance += pow((abs(iRunsPerRow[i+1] - iRunsPerRow[i]) - dAverage), 2);
        }
    dVariance = dVariance / (iRunsPerRow.size()-1);

    iRunsPerRow.clear();

    return dVariance;
    }
//---------------------------------------------------------------------------
double
__fastcall TOcrAnalyseRectangles::CalculateColumnVariance(TOcrRectangle* poRectangle) {

    vector<long> iRunsPerColumn;
    long iRuns;
    long lPointer;
    BYTE iCurrentRun;
    double dAverage;
    double dVariance;

    //Count number of pixels per row
    for (int i=0; i<poRectangle->width; i++) {

       lPointer = poRectangle->x + i + (iImageHeight - poRectangle->y -1) * iRowLength;

        iCurrentRun = lpBits[lPointer];
        iRuns = 1;

        for (int j=0; j<poRectangle->height; j++) {

            if (abs(lpBits[lPointer]) != abs(iCurrentRun)) {
                iRuns++;
                iCurrentRun = lpBits[lPointer];
                }

            lPointer-=iRowLength;
            }

        iRunsPerColumn.push_back(iRuns);
        }

    //calculate average runs
    dAverage = 0;
    for (unsigned int i=0; i<iRunsPerColumn.size()-1; i++) {
        dAverage += abs(iRunsPerColumn[i+1] - iRunsPerColumn[i]);
        }
    dAverage = dAverage / (iRunsPerColumn.size()-1);

    //calculate variance;
    dVariance = 0;
    for (unsigned int i=0; i<iRunsPerColumn.size()-1; i++) {
        dVariance += pow((abs(iRunsPerColumn[i+1] - iRunsPerColumn[i]) - dAverage), 2);
        }
    dVariance = dVariance / (iRunsPerColumn.size()-1);

    iRunsPerColumn.clear();

    return dVariance;
    }
//---------------------------------------------------------------------------
