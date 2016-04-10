//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrRectangle.h"
#include "TError.h"
#include "TOcrThread.h"
#include "TOcrFindRectangle.h"
//---------------------------------------------------------------------------

TOcrFindRectangle::TOcrFindRectangle(HGLOBAL phImageHandle, TOcrRectangle* poRectangles, bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{
    hImageHandle = phImageHandle;
    oRectangles = poRectangles;

    lTimeInCheckRectangle = 0;
    lTimeInGrowRectangle = 0;
    lNumberInCheckRectangle = 0;
    lNumberInGrowRectangle = 0;

    oError = TError::GetInstance();
}
//---------------------------------------------------------------------------
void
__fastcall TOcrFindRectangle::Execute() {

    int iReturnValue;

    iReturnValue = -1; //Error

    try {

        //Get a pointer to the bitmap and get the characteristics of the bitmap

            MakeSure(hImageHandle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hImageHandle!=NULL");

            BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);

            MakeSure(lpSourceBitmapInfo!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpSourceBitmapInfo!=NULL");

            int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
            int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

            lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

            iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
            iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
            int iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
            iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

        //Fine the rectangles with a connected component algorithm
            ConnectedComponentGeneration(oRectangles);

        //Unlock the pointer to the bitmap
            ::GlobalUnlock(hImageHandle);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }

    ReturnValue = iReturnValue;
}
//---------------------------------------------------------------------------
void
__fastcall TOcrFindRectangle::ConnectedComponentGeneration(TOcrRectangle* poRectangle) {

    TOcrRectangle* oCurrentRectangle;
    TOcrPoint oConnectedComponentPoint;

    long y = poRectangle->y;
    long x = poRectangle->x;
    long xmax;
    long lPointer = 0;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    while (y<poRectangle->y + poRectangle->height) {

        x = poRectangle->x + ((y & 2) / 2);

        // we are using a mesh, therefor
        // we use y&1 to differentiate the starting point of each row.
        // the mesh looks like:
        // +-+-+-+-+
        // ---------
        // -+-+-+-+-
        // ---------
        // +-+-+-+-+
        // ---------
        // -+-+-+-+-
        // we check every other pixel, that way we can check half of the image
        // and still find all the component we want to find, except for lines
        // at exactly 45 degrees and 1 pixel wide. But at this point we find
        // those irrelevant;

        xmax = poRectangle->x + poRectangle->width;
        lPointer = x + (iImageHeight - y -1) *iRowLength;

        while (x<xmax) {

            if (lpBits[lPointer]==0) {
                //We found a new pixel with the
                oCurrentRectangle = new TOcrRectangle();
                MakeSure(oCurrentRectangle!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"TOcrRectangle");

                oCurrentRectangle->x = x;
                oCurrentRectangle->width = 0;
                oCurrentRectangle->height = 0;
                oCurrentRectangle->y = y;

                if (GenerateConnectedComponent(poRectangle, oCurrentRectangle)) {

                    oConnectedComponentPoint.x = x-oCurrentRectangle->x;
                    oConnectedComponentPoint.y = y-oCurrentRectangle->y;

                    oCurrentRectangle->oConnectedComponentPoints.push_back(oConnectedComponentPoint);

                    if (oCurrentRectangle->height < iImageHeight*0.9 && oCurrentRectangle->width < iImageWidth*0.9) {

                        poRectangle->AddChild(oCurrentRectangle);
                        }
                    }
                else {
                    delete oCurrentRectangle;
                    }
                }

            x += 2;
            lPointer += 2;
            }

        y += 2;
        }
    }

//---------------------------------------------------------------------------
bool
__fastcall TOcrFindRectangle::GenerateConnectedComponent(TOcrRectangle* poRectangle, TOcrRectangle* poChild) {

    //this functies growes the oCurrentRectangle in such manner that it
    //surrounds connected pixels.

    //the function returns true if there are more than 8 connected pixels present.
    //The number of 8 comes from the article bu L.A.Fletcher and R.Kasturi,
    // 'A Robust Algorithm for Text String Separation from Mixed Text/Graphics Images'

    //in 'A word extraction algorithm for machine-printed documents using a
    //    3D neighborhood graph algorithm' a number of 6 pixels was mentioned
    //but for now the number of 8 seems to work allright.

    int iNumberPixels = 0;
    long lPointer;
    bool bError = false;

    long xmin;
    long ymin;
    long xmax;
    long ymax;

    long x,y;

    vector<long> xCache;
    vector<long> yCache;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");
    MakeSure(poChild!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poChild!=NULL");

    //initialise variables
    unsigned long iIndex = 0;

    x = xmin = xmax = poChild->x;
    y = ymin = ymax = poChild->y;

    //place first detected pixel in vector
    xCache.push_back(x);
    yCache.push_back(y);

    lPointer = x + (iImageHeight - y -1) *iRowLength;
    lpBits[lPointer]=cColorReplacementStep1;

    do {
        //get pixel from vector (if there are any)
        x = xCache[iIndex];
        y = yCache[iIndex];

        //determine if we need to increase our rectangle scope.
        if (x<xmin) xmin = x;
        if (x>xmax) xmax = x;
        if (y<ymin) ymin = y;
        if (y>ymax) ymax = y;

        //calculate pointer and change color of pixel to make sure
        //it will not be detected for other connected components
        lPointer = x + (iImageHeight - y -1) *iRowLength;
        iNumberPixels++;

        //look around and add interesting pixels to vector
        if (!bError) bError = GCC_CheckNeighbour(poRectangle, x, -1, y,  0, xCache, yCache);
        //if (!bError) bError = GCC_CheckNeighbour(poRectangle, x, -1, y, -1, xCache, yCache);
        if (!bError) bError = GCC_CheckNeighbour(poRectangle, x,  0, y, -1, xCache, yCache);
        //if (!bError) bError = GCC_CheckNeighbour(poRectangle, x,  1, y, -1, xCache, yCache);
        if (!bError) bError = GCC_CheckNeighbour(poRectangle, x,  1, y,  0, xCache, yCache);
        //if (!bError) bError = GCC_CheckNeighbour(poRectangle, x,  1, y,  1, xCache, yCache);
        if (!bError) bError = GCC_CheckNeighbour(poRectangle, x,  0, y,  1, xCache, yCache);
        //if (!bError) bError = GCC_CheckNeighbour(poRectangle, x, -1, y,  1, xCache, yCache);

        //update iIndex;
        iIndex++;
        } while (iIndex<xCache.size() && !bError);

    //clear the vectors
    xCache.clear();
    yCache.clear();

    //set the child's dimensions according to the min and max variables
    poChild->x = xmin;
    poChild->y = ymin;
    poChild->width = xmax - xmin;
    poChild->height = ymax - ymin;
    poChild->numberpixels = iNumberPixels;

    GenerateBitmap(poChild);

    //return true if there are more than 8 connected pixels
    return (iNumberPixels >= 8 && !bError);
    }
//---------------------------------------------------------------------------
bool
__fastcall TOcrFindRectangle::GCC_CheckNeighbour(TOcrRectangle* poRectangle, long x, int dx, long y, int dy, vector<long>& pxCache, vector<long>& pyCache) {

    bool bError = false;
    long lPointer;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    lPointer = x + (iImageHeight - y - 1) *iRowLength;

    //check if pixel is withing boundaries
    if (poRectangle->CoordinateInMe(x+dx, y+dy)) {

            //check color code of pixel
            if (lpBits[lPointer + dx - (dy * iRowLength)]==0) {

                //add pixel to cache
                if (pxCache.max_size()>pxCache.size() && pyCache.max_size()>pyCache.size()) {
                    pxCache.push_back(x+dx);
                    pyCache.push_back(y+dy);
                    lpBits[lPointer + dx - (dy * iRowLength)]=cColorReplacementStep1;
                   }
                else {
                    bError = true;
                    }
                }
            }

    return bError;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindRectangle::AnalyseRectangle(TOcrRectangle* poRectangle, int iRecursionDepth) {

   TOcrRectangle* oStartRectangle = poRectangle;
   TOcrRectangle* oCurrentRectangle;

   MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

   oStartRectangle->x = 4;
   oStartRectangle->y = 4;
   oStartRectangle->width = iImageWidth - 8;
   oStartRectangle->height = iImageHeight - 8;

   poRectangle->AddChild(oStartRectangle);

   long y = oStartRectangle->y;
   long x = oStartRectangle->x;
   long lPointer = 0;

   //search for the first pixel
   CleanupCacheForNextRow(oStartRectangle, y);

   while (y<oStartRectangle->y + oStartRectangle->height) {

    lPointer = x + (iImageHeight - y -1) *iRowLength;

    if (x<lNextRectangle) {

      if (lpBits[lPointer]==0) {
          //een rand gevonden!
          oCurrentRectangle = new TOcrRectangle();
          MakeSure(oCurrentRectangle!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"TOcrRectangle");

          oCurrentRectangle->x = x;
          oCurrentRectangle->width = 0;
          oCurrentRectangle->height = 0;
          oCurrentRectangle->y = y;

          GrowRectangle(oStartRectangle, oCurrentRectangle);

          x = oCurrentRectangle->x + oCurrentRectangle->width + 1;

          oStartRectangle->AddChild(oCurrentRectangle);

          //rectangle index in tijdelijke cache opslaan
          oRectangleCache.push_back(oStartRectangle->GetNumberChilds()-1);
          }
      else {
          x++;
          }
        }

    if (x>=lNextRectangle) {
        x = FindNextSpace(oStartRectangle ,x, y);
        }

    if (x>oStartRectangle->x + oStartRectangle->width) {
        y++;
        CleanupCacheForNextRow(oStartRectangle, y);
        lNextRectangle = 0;
        x = FindNextSpace(oStartRectangle, oStartRectangle->x, y);
        }
   }

    //Merge the found rectangles
    MergeRectangle(oStartRectangle);
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindRectangle::CleanupCacheForNextRow(TOcrRectangle* poRectangle, long y) {

    TOcrRectangle* oChild;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    //Walk through cache and remove rectangles that do not cross the give
    //row (y).

    for (int i=oRectangleCache.size(); i>0; i--) {

        oChild = poRectangle->GetChild(oRectangleCache[i-1]);

        if (oChild->y + oChild->height < y) {

            //this rectangle does not cross the current row anymore, it can
            //be removed from this cache

            oRectangleCache.erase(oRectangleCache.begin() + i - 1);

            }
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrFindRectangle::FindNextSpace(TOcrRectangle* poRectangle, long x, long y) {

    long lLongestRectangle;
    long lCandidateX;
    TOcrRectangle* oChild;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    lCandidateX = x;

    while (lCandidateX >= lNextRectangle && lCandidateX<(poRectangle->x + poRectangle->width + 1)) {

        //we are now inside a rectangle, search for the longest rectangle
        //where x is part of
        lLongestRectangle = 0; //poRectangle->x + poRectangle->width + 1;
        for (unsigned int i=0; i<oRectangleCache.size(); i++) {

            oChild = poRectangle->GetChild(oRectangleCache[i]);

            if (lCandidateX>=oChild->x && lCandidateX<=(oChild->x+oChild->width)) {

                //we are currently in the given oChild rectangle, but is
                //this child the longest?
                if ((oChild->x + oChild->width) > lLongestRectangle) {
                    lLongestRectangle = oChild->x + oChild->width;
                    }
                }
            }

        lCandidateX = lLongestRectangle + 1;

        if (lCandidateX < poRectangle->x) {
            lCandidateX = poRectangle->x;
            }

        //we have the longest part now, search for the start position
        //of the next rectangle after the longest rectangle
        lNextRectangle = poRectangle->x + poRectangle->width + 1;
        for (unsigned int i=0; i<oRectangleCache.size(); i++) {

            oChild = poRectangle->GetChild(oRectangleCache[i]);

            if (oChild->x > lLongestRectangle && oChild->x < lNextRectangle) {

                lNextRectangle = oChild->x;

                }
            }
        }

    return lCandidateX;
    }
//---------------------------------------------------------------------------

void
__fastcall TOcrFindRectangle::MergeRectangle(TOcrRectangle* poRectangle) {

    //merge rectangles if they overlap eachother
    bool bNoMerge = false;
    int  iAddIndex = 0;
    int  iCompareIndex = 0;
    int  iRectangleIndex;
    TOcrRectangle* oCompareRectangle;
    TOcrRectangle* oCacheRectangle;
    long x2;
    long y2;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    while (!bNoMerge) {

        bNoMerge = true;
        iAddIndex = 0;
        iCompareIndex = 0;

        //clear cache
        while(oRectangleCache.size()>0) {
            oRectangleCache.erase(oRectangleCache.begin());
            }

        for (int i=poRectangle->y; (i<(poRectangle->y + poRectangle->height)) && bNoMerge; i++) {

            //add rectangle to cache which or on give line i;
            while ((iAddIndex < poRectangle->GetNumberChilds()) && poRectangle->GetChild(iAddIndex)->y == i) {

                oRectangleCache.push_back(iAddIndex);
                iAddIndex++;
                }

            //compare rectangles with rectangles in the cache (those are the ones on the same vertical line)
            while (iCompareIndex<iAddIndex && bNoMerge) {

                oCompareRectangle = poRectangle->GetChild(iCompareIndex);

                for (unsigned int j=0; j<oRectangleCache.size() && bNoMerge; j++) {

                    iRectangleIndex = oRectangleCache[j];
                    if (iRectangleIndex != iCompareIndex) {

                        oCacheRectangle = poRectangle->GetChild(iRectangleIndex);

                       if (oCompareRectangle->RectangleOverlap(oCacheRectangle)) {


                            //the rectangles overlap, merge the two together
                            x2 = max(oCompareRectangle->width + oCompareRectangle->x,  oCacheRectangle->width + oCacheRectangle->x);
                            if (oCompareRectangle->x < oCacheRectangle->x) {

                                oCompareRectangle->width = x2 - oCompareRectangle->x;
                                }
                            else {
                                oCompareRectangle->x = oCacheRectangle->x;
                                oCompareRectangle->width = x2 - oCacheRectangle->x;
                                }

                            y2 = max(oCompareRectangle->height + oCompareRectangle->y,  oCacheRectangle->height + oCacheRectangle->y);
                            if (oCompareRectangle->y < oCacheRectangle->y) {

                                oCompareRectangle->height = y2 - oCompareRectangle->y;
                                poRectangle->DeleteChild(iRectangleIndex);
                                }
                            else {
                                oCacheRectangle->y = oCacheRectangle->y;
                                oCacheRectangle->height = y2 - oCacheRectangle->y;
                                oCacheRectangle->x = oCompareRectangle->x;
                                oCacheRectangle->width = oCompareRectangle->width;

                                poRectangle->DeleteChild(iCompareIndex);
                                }

                            bNoMerge = false;
                            }
                        }
                    }

                iCompareIndex++;
                }

            if (bNoMerge) {
                CleanupCacheForNextRow(poRectangle, i+1);
                }
            }
        }
}
//---------------------------------------------------------------------------
bool
__fastcall TOcrFindRectangle::GrowRectangle(TOcrRectangle* poParent, TOcrRectangle* poChild) {

    long lStart = ::GetTickCount();

    bool bRetval = true; // true = klaar
    long lPointer;
    long lNumber;

    bool bDoCheckLeft = true;
    bool bDoCheckBottom = true;
    bool bDoCheckRight = true;

    MakeSure(poParent!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poParent!=NULL");
    MakeSure(poChild!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poChild!=NULL");

    while (bDoCheckLeft || bDoCheckBottom || bDoCheckRight) {

       //check left side
       if (bDoCheckLeft) {

            if (poParent->x <= poChild->x) {

                bDoCheckLeft = false;

                lPointer = poChild->x - 1 + (iImageHeight - poChild->y -1) *iRowLength;

                for (lNumber = 0; lNumber < poChild->height+1 && !bDoCheckLeft; lNumber++) {


                    if (lpBits[lPointer]==0) {
                        //verder naar links groeien
                        poChild->x--;
                        bDoCheckLeft = true;
                        bDoCheckBottom = true;
                        }

                    lPointer -= iRowLength;

                    }
                }
            else {
                bDoCheckLeft = false;
                }
            }

        //check right side
        if (bDoCheckRight) {

            if (poParent->x + poParent->width != poChild->x + poChild->width) {

              bDoCheckRight = false;

                lPointer = poChild->x + poChild->width + 1 + (iImageHeight - poChild->y -1) *iRowLength;

                for (lNumber = 0; lNumber < poChild->height+1 && !bDoCheckRight; lNumber++) {

                    if (lpBits[lPointer]==0) {
                        //verder naar rechts groeien
                        poChild->width++;
                        bDoCheckRight = true;
                        bDoCheckBottom = true;
                        }

                    lPointer -= iRowLength;

                    }
                }
            else {
                bDoCheckRight = false;
                }
            }

        //check bottom
        if (bDoCheckBottom) {
            if (poParent->y + poParent->height != poChild->y + poChild->height) {

                bDoCheckBottom = false;

                lPointer = poChild->x - 1 + (iImageHeight - poChild->y - poChild->height -2) * iRowLength;

                for (lNumber = 0; lNumber < poChild->width + 3 && !bDoCheckBottom; lNumber++) {

                    if (lpBits[lPointer]==0) {
                        //verder naar beneden groeien
                        poChild->height++;
                        bDoCheckRight = true;
                        bDoCheckBottom = true;
                        bDoCheckLeft = true;
                        }

                    lPointer++;
                    }
                }
            else {
                bDoCheckBottom = false;
                }
            }
        }

    lTimeInGrowRectangle += ::GetTickCount() - lStart;
    lNumberInGrowRectangle++;

    return bRetval;
    }

//---------------------------------------------------------------------------
bool
__fastcall TOcrFindRectangle::CheckInRectangle(TOcrRectangle* poRectangle, long& x, long y) {

    //this function checks if a certain coordinate lies with the bounding box of
    //the given rectangle

    long lStart = ::GetTickCount();

    bool bRetval = false;
    TOcrRectangle* oChild;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    for (unsigned int i=0; i<oRectangleCache.size() && !bRetval; i++) {

        oChild = poRectangle->GetChild(oRectangleCache[i]);
        if (oChild->CoordinateInMe(x,y)) {

            x = oChild->x + oChild->width;

            bRetval = true;
            }
        }

    lTimeInCheckRectangle += ::GetTickCount() - lStart;
    lNumberInCheckRectangle++;

    return bRetval;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindRectangle::GenerateBitmap(TOcrRectangle* poRectangle) {

    //Generate a bitmap of the connected component. The bitmap contains only
    //pixels of the connected component and no other ones. The bitmaps has the
    //size of the bounding rectangle of the connected component
    HGLOBAL hBitmap;
    long lWidth, lHeight;
    BYTE* pSourcePointer, *pDestinationPointer;
    BYTE* pBitmap;

    MakeSure(poRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poRectangle!=NULL");

    lWidth = poRectangle->width+1;
    lHeight = poRectangle->height+1;

    pBitmap = NULL;

    hBitmap = GlobalAlloc(GMEM_MOVEABLE, lWidth * lHeight);

    MakeSure(hBitmap!=NULL, ERR_OUTOFMEMORY, LEVEL_WARNING, L"");

    pBitmap = (BYTE*)GlobalLock(hBitmap);

    MakeSure(pBitmap!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"pBitmap!=NULL");

    poRectangle->hBitmap = hBitmap;

    FillMemory(pBitmap, lWidth * lHeight, 255);

    for (long lY=0; lY<lHeight; lY++) {

        pSourcePointer = lpBits + poRectangle->x + (iImageHeight - poRectangle->y - lY  - 1) * iRowLength;
        pDestinationPointer = pBitmap + (lHeight - lY - 1) * lWidth;

        for (long lX=0; lX<lWidth; lX++) {

            if (*pSourcePointer==(BYTE)cColorReplacementStep1) {

                *pDestinationPointer = 0x00;
                *pSourcePointer = cColorReplacementStep2;
                }

            pDestinationPointer++;
            pSourcePointer++;
            }
        }
    }
//---------------------------------------------------------------------------
