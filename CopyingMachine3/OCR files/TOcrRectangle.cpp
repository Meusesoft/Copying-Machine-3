//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "TOcrRectangle.h"
#include "TOcrCollection.h"

#define M_PI       3.14159265358979323846

//---------------------------------------------------------------------------
TOcrRectangle::TOcrRectangle() {

    x=y=width=height=0;
    eType = eUnknownRect;
    sContent = L"";
    iSentenceNumber = -1;
    fAnglePrevious = fAngleNext = 1000;
    iMergeWithRectangle = -1;
    iGaps = 0;
    iAreaNumber = -1;
    bTouchingCharacters = false;
    hBitmap = NULL;

    oError = TError::GetInstance();
    }

TOcrRectangle::~TOcrRectangle() {

    //at deletion, delete children first
    for (unsigned int i=0; i<oChilds.size(); i++) {
        delete oChilds[i];
        }

    oVectorPoints.clear();

    oCharacterSuggestions.clear();

    if (hBitmap!=NULL) {
        GlobalUnlock(hBitmap);
        GlobalFree(hBitmap);
        }
    }

bool
TOcrRectangle::DeleteChild(int iIndex, bool bDeleteInstance) {

    bool bRetValue;
    TOcrRectangle* oChild;

    bRetValue = false;

    try {

        MakeSure(iIndex < (int)oChilds.size(), ERR_INVALIDPOINTER, LEVEL_WARNING, L"iIndex < oChilds.size()");
        MakeSure(iIndex >= 0, ERR_INVALIDPOINTER, LEVEL_WARNING, L"iIndex >= 0");

        oChild = oChilds[iIndex];
        oChilds.erase(oChilds.begin() + iIndex);

        if (bDeleteInstance) delete oChild;
        bRetValue = true;
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return bRetValue;
    }

bool
TOcrRectangle::AddChild(TOcrRectangle* poChild) {

    bool bRetValue;

    bRetValue = false;

    try {

        MakeSure(oChilds.size() < oChilds.max_size(), ERR_OUTOFMEMORY, LEVEL_CRITICAL, L"");

        MakeSure(poChild->height > 0, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poChild->height > 0");
        MakeSure(poChild->width > 0, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poChild->width > 0");

        oChilds.push_back(poChild);

        bRetValue = true;
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return bRetValue;
    }

//---------------------------------------------------------------------------
int
TOcrRectangle::GetNumberChilds() {

    return oChilds.size();
    }
//---------------------------------------------------------------------------
TOcrRectangle*
TOcrRectangle::GetChild(int iIndex) {

    TOcrRectangle* oRetValue;

    try {

        MakeSure(iIndex < (int)oChilds.size(), ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"iIndex < oChilds.size()");

        oRetValue = oChilds[iIndex];
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return oRetValue;
    }

//---------------------------------------------------------------------------

bool
TOcrRectangle::CoordinateInMe(long px, long py) {

    bool bRetval = true;

    if (px<x) bRetval = false;
    if (px>(x+width)) bRetval = false;
    if (py<y) bRetval = false;
    if (py>(y+height)) bRetval = false;

    return bRetval;
    }

//---------------------------------------------------------------------------

bool
TOcrRectangle::RectangleOverlap(TOcrRectangle* poOther) {

    bool bRetval = false;

    if ((x <= poOther->x && x + width >= poOther->x) || (poOther->x <= x && poOther->x + poOther->width >= x)) {
        if ((y <= poOther->y && y + height >= poOther->y) || (poOther->y <= y && poOther->y + poOther->height >= y)) {

            bRetval = true;

            }
        }

    return bRetval;
}

//---------------------------------------------------------------------------

TOcrPoint
TOcrRectangle::GetCenterPoint() {

    TOcrPoint pRetval;

    pRetval.x = x + (width >> 1);
    pRetval.y = y + (height >> 1);

    return pRetval;
    }

//---------------------------------------------------------------------------
long
TOcrRectangle::DistanceChildren(int iIndex1, int iIndex2) {

    long lRetValue;
    TOcrRectangle* oChild;
    TOcrPoint pPos1;
    TOcrPoint pPos2;

    double root1, root2;

    try {

        MakeSure(oChilds.size()>(unsigned int)iIndex1, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex1");
        MakeSure(oChilds.size()>(unsigned int)iIndex2, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex2");

        oChild = oChilds[iIndex1];
        pPos1 = oChild->GetCenterPoint();

        oChild = oChilds[iIndex2];
        pPos2 = oChild->GetCenterPoint();

        root1 = pow((double)abs(pPos1.x - pPos2.x), 2);
        root2 = pow((double)abs(pPos1.y - pPos2.y), 2);

        lRetValue = (long)sqrt(root1 + root2);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return lRetValue;
    }

//----------------------------------------------------------
long
TOcrRectangle::DistanceBetweenChildren(int iIndex1, int iIndex2) {

    long lRetValue;

    lRetValue = 0;

    TOcrRectangle* oChildFrom;
    TOcrRectangle* oChildTo;

    long xFrom, xTo, yFrom, yTo, xPos, yPos, xOrg, yOrg;
    int xDelta, yDelta;
    int xMoved, yMoved;
    double dDirection;
    TOcrPoint cPointFrom, cPointTo;

    try {

        MakeSure(oChilds.size()>(unsigned int)iIndex1, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex1");
        MakeSure(oChilds.size()>(unsigned int)iIndex2, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex2");

        oChildFrom = oChilds[iIndex1];
        oChildTo = oChilds[iIndex2];

        cPointFrom = oChildFrom->GetCenterPoint();
        cPointTo   = oChildTo->GetCenterPoint();

        xFrom = xOrg = cPointFrom.x;
        yFrom = yOrg = cPointFrom.y;

        xTo = cPointTo.x;
        yTo = cPointTo.y;


        lRetValue = DistanceChildren(iIndex1, iIndex2) + 1;

        if (abs(xTo-xFrom) > abs(yTo-yFrom)) {

            if (xTo!=xFrom) { //just to be sure we don't get a division by zero
                dDirection = (double)(yTo - yFrom) / (double)(xTo - xFrom);
                }

            xPos = xFrom;
            yPos = yFrom;
            if (xFrom>xTo) {
                xDelta = -1;
                }
            else {
                xDelta = 1;
                }
            xMoved = 0;

            while (oChildFrom->CoordinateInMe(xPos, yPos)) {

                lRetValue--;

                xMoved += xDelta;
                xPos += xDelta;

                yPos = (long)(yOrg + xMoved * dDirection);
                }

            xPos = xTo;
            yPos = yTo;
            xMoved = 0;
            xDelta = xDelta * -1; //we walk to the different side

            while (oChildTo->CoordinateInMe(xPos, yPos)) {

                lRetValue--;

                xMoved += xDelta;
                xPos += xDelta;

                yPos = (long)(yOrg + xMoved * dDirection);
                }
            }
        else {
            if (yTo!=yFrom) { //just to be sure we don't get a division by zero
                dDirection = (double)(xTo - xFrom) / (double)(yTo - yFrom) ;
                }

            xPos = xFrom;
            yPos = yFrom;
            if (yFrom>yTo) {
                yDelta = -1;
                }
            else {
                yDelta = 1;
                }
            yMoved = 0;

            while (oChildFrom->CoordinateInMe(xPos, yPos)) {

                lRetValue--;

                yMoved += yDelta;
                yPos += yDelta;

                xPos = (long)(xOrg + yMoved * dDirection);
                }

            xPos = xTo;
            yPos = yTo;
            yMoved = 0;
            yDelta = yDelta * -1; //we walk to the different side

            while (oChildTo->CoordinateInMe(xPos, yPos)) {

                lRetValue--;

                yMoved += yDelta;
                yPos += yDelta;

                xPos = (long)(xOrg + yMoved * dDirection);
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return lRetValue;
    }
//---------------------------------------------------------------------------
double
TOcrRectangle::AverageAreaChildren() {

    //This function calculates the average area of all child rectangles
    //of this rectangle;

    double dReturn = 0;
    TOcrRectangle* oChild;

    if (oChilds.size()>0) {

        for (unsigned int i=0; i<oChilds.size(); i++) {

            oChild = oChilds[i];

            dReturn += oChild->width * oChild->height;
            }

        dReturn = dReturn / oChilds.size();
        }

    return dReturn;
    }
//---------------------------------------------------------------------------
double
__fastcall TOcrRectangle::AngleBetweenChildren(int iIndex1, int iIndex2) {

    TOcrRectangle* oFromRectangle;
    TOcrRectangle* oToRectangle;
    double dx, dy, dInput, dAngle;
    long xTo,xFrom,yTo,yFrom;

    try {

        MakeSure(oChilds.size()>(unsigned int)iIndex1, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex1");
        MakeSure(oChilds.size()>(unsigned int)iIndex2, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex2");

        oFromRectangle = GetChild(iIndex1);
        oToRectangle = GetChild(iIndex2);

        xTo = oToRectangle->x;
        xFrom = oFromRectangle->x;
        yTo = oToRectangle->y;
        yFrom = oFromRectangle->y;

        if (xTo <= xFrom && (xTo+oToRectangle->width) > xFrom) {
            xTo = xFrom;
            }
        if (xFrom <= xTo && (xFrom+oFromRectangle->width) > xTo) {
            xFrom = xTo;
            }
        if (yTo <= yFrom && (yTo+oToRectangle->height) > yFrom) {
            yTo = yFrom;
            }
        if (yFrom <= yTo && (yFrom+oFromRectangle->height) > yTo) {
            yFrom = yTo;
            }


        dx = abs(xTo - xFrom);
        dy = abs(yTo - yFrom);

        if (dx!=0) {
            dInput = dy / dx;

            dAngle = ((atan(dInput) * 360)  / (2 * M_PI));
            }
        else {
            if (yTo!=yFrom) {
                dAngle = 90;
                }
            else {
                dAngle = 0;
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return dAngle;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRectangle::MergeRectangles(long lIndex1, long lIndex2) {

    //this function takes the two given rectangles and merges them
    //into a newly created rectangle and returns the index of the
    //newly created rectangle.

    TOcrRectangle* oRectangle1;
    TOcrRectangle* oRectangle2;
    TOcrRectangle* oNewRectangle;
    TOcrPoint cPoint;

    //init
    long lReturnValue = -1;

    try {

        MakeSure(oChilds.size()>(unsigned int)lIndex1, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex1");
        MakeSure(oChilds.size()>(unsigned int)lIndex2, ERR_INDEXOUTOFBOUNDS, LEVEL_WARNING, L"oChilds.size()>(unsigned int)iIndex2");

        oRectangle1 = GetChild(lIndex1);
        oRectangle2 = GetChild(lIndex2);

        //create instance of TOcrRectangle class
        oNewRectangle = new TOcrRectangle();

        //determine dimensions of new rectangle.
        oNewRectangle->x = (oRectangle1->x < oRectangle2->x) ?  oRectangle1->x : oRectangle2->x;
        oNewRectangle->y = (oRectangle1->y < oRectangle2->y) ?  oRectangle1->y : oRectangle2->y;

        oNewRectangle->width = ((oRectangle1->x + oRectangle1->width) > (oRectangle2->x + oRectangle2->width )) ?  oRectangle1->x + oRectangle1->width - oNewRectangle->x : oRectangle2->x + oRectangle2->width - oNewRectangle->x;
        oNewRectangle->height = ((oRectangle1->y + oRectangle1->height) > (oRectangle2->y + oRectangle2->height )) ?  oRectangle1->y + oRectangle1->height - oNewRectangle->y : oRectangle2->y + oRectangle2->height - oNewRectangle->y;

        //pump over connected component points from rectangle 1
        for (long lIndex=0; lIndex<(long)oRectangle1->oConnectedComponentPoints.size(); lIndex++) {

            cPoint = oRectangle1->oConnectedComponentPoints[lIndex];

            cPoint.x += oRectangle1->x;
            cPoint.y += oRectangle1->y;

            cPoint.x -= oNewRectangle->x;
            cPoint.y -= oNewRectangle->y;

            oNewRectangle->oConnectedComponentPoints.push_back(cPoint);
            }


        //pump over connected component points from rectangle 2
        for (long lIndex=0; lIndex<(long)oRectangle1->oConnectedComponentPoints.size(); lIndex++) {

            cPoint = oRectangle2->oConnectedComponentPoints[lIndex];

            cPoint.x += oRectangle2->x;
            cPoint.y += oRectangle2->y;

            cPoint.x -= oNewRectangle->x;
            cPoint.y -= oNewRectangle->y;

            oNewRectangle->oConnectedComponentPoints.push_back(cPoint);
            }

        //add the newly created rectangle
        if (AddChild(oNewRectangle)) {

            lReturnValue = GetNumberChilds() - 1;
            }

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return lReturnValue;
    }
//---------------------------------------------------------------------------


