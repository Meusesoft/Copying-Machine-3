//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TError.h"
#include "TOcrThread.h"
#include "TOcrFindSentences.h"
//---------------------------------------------------------------------------

TOcrFindSentences::TOcrFindSentences(HGLOBAL phImageHandle, TOcrRectangle* poRectangles, TOcrSentences* poSentences, bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{
    hImageHandle = phImageHandle;
    oRectangles = poRectangles;
    oSentences = poSentences;

    BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);

    iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
    iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;

    ::GlobalUnlock(hImageHandle);

    lTimeDrawSentences = 0;
    lTimeCalculateDistances = 0;
    lTimeFindSentences = 0;
    lTimeAddSpaces = 0;
    lTimeCleanUpSentences = 0;
    lTimeCreateAreas = 0;

    oError = TError::GetInstance();
}
//---------------------------------------------------------------------------
void __fastcall TOcrFindSentences::Execute()
{
    int iReturnValue;

    iReturnValue = 0;

    try {

        //Create areas
        CreateAreas();

        //Find sentences with areas
        FindSentences();

        //Uniform the height;
        CalculateBoundingBox();

        //Clean up sentences
        CleanUpSentences();

        //Add spaces to sentences
        AddSpacesToSentences();

        //Draw Sentences in intermediate image (debugging)
        DrawSentences();
        ExportAreas();

        //clear memory in use by Areas
        ClearAreas();

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
void
__fastcall TOcrFindSentences::CreateAreas() {

    //this function divides the image into areas so that each area contains
    //on average 20 rectangles. This helps to speed up the process of finding
    //the closest rectangle.

    TOcrArea* oArea;
    TOcrRectangle* oRectangle;
    TOcrPoint oCenter;

    long      nDividerLinesX, nDividerLinesY;
    long      nAreas;
    long      lAreaWidth, lAreaHeight;
    long      lAreaNumber;

    long lStartTime = ::GetTickCount();

    try {

        MakeSure(iImageWidth>0, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"iImageWidth>0");

        //calculate the number of dividerlines necessary to make up the desired
        //number of 20 rectangles per area
        nAreas = oRectangles->GetNumberChilds() / 20;

        if (nAreas<1) nAreas = 1;

        //determine the desired size of the areas.
        nDividerLinesX = 0;
        nDividerLinesY = (nDividerLinesX * iImageHeight) / iImageWidth;

        while (((nDividerLinesX+1)*(nDividerLinesY+1)) < nAreas) {

            nDividerLinesX++;
            nDividerLinesY = (nDividerLinesX * iImageHeight) / iImageWidth;
            }

        lNumberAreasX = nDividerLinesX+1;
        lNumberAreasY = nDividerLinesY+1;

        lAreaWidth = iImageWidth / lNumberAreasX;
        lAreaHeight = iImageHeight / lNumberAreasY;

        MakeSure(lAreaWidth>0, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"lAreaWidth>0");
        MakeSure(lAreaHeight>0, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"lAreaHeight>0");

        //create the areas
        for (long lY=0; lY<lNumberAreasY; lY++) {

            for (long lX=0; lX<lNumberAreasX; lX++) {

                oArea = new TOcrArea();

                MakeSure(oArea!=NULL, ERR_CREATEOBJECT, LEVEL_RECOVERABLE, L"oArea");

                oArea->lX = lX * lAreaWidth;
                oArea->lY = lY * lAreaHeight;

                if (lX == nDividerLinesX) {

                    oArea->lWidth = iImageWidth - lX * lAreaWidth;
                    }
                else {

                    oArea->lWidth = lAreaWidth;
                    }

                if (lX == nDividerLinesX) {

                    oArea->lHeight = iImageHeight - lY * lAreaHeight;
                    }
                else {

                    oArea->lHeight = lAreaHeight;
                    }

                oAreas.push_back(oArea);
                }
            }

        nAreas = lNumberAreasX*lNumberAreasY;

        //assign an area to every rectangle
        for (long lIndex=0; lIndex<oRectangles->GetNumberChilds(); lIndex++) {

            oRectangle = oRectangles->GetChild(lIndex);

            oCenter = oRectangle->GetCenterPoint();

            lAreaNumber = (oCenter.x / lAreaWidth);
            lAreaNumber = lAreaNumber + (lNumberAreasX * (oCenter.y / lAreaHeight));

            //assign the area number to the rectangle
            oRectangle->iAreaNumber = lAreaNumber;

            //save the rectangle number in the area vector
            MakeSure(lAreaNumber<(long)oAreas.size(), ERR_INDEXOUTOFBOUNDS, LEVEL_RECOVERABLE, L"lAreaNumber");

            oAreas[lAreaNumber]->lRectangleNumbers.push_back(lIndex);
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    lTimeCreateAreas += ::GetTickCount() - lStartTime;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::ClearAreas() {

    //this function deletes all oArea instances in the oAreas vector

    TOcrArea* oArea;

    for (long lIndex=0; lIndex<(long)oAreas.size(); lIndex++) {

        oArea = oAreas[lIndex];
        delete oArea;
        }

    oAreas.clear();
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::FindSentences() {

    //This function starts the search for a sentence. It takes the
    //first free rectangle (not belonging to a sentence) and takes
    //it as the first step in a growth process in which more rectangles
    //are added to it

    long lStartTime = ::GetTickCount();

    long iPointer;
    TOcrRectangle* oCurrentRectangle;
    TOcrSentence*  oNewSentence;

    try {

        iPointer = 0;

        while (iPointer<oRectangles->GetNumberChilds()) {

            oCurrentRectangle = oRectangles->GetChild(iPointer);

            if (oCurrentRectangle->iSentenceNumber == -1  && oCurrentRectangle->eType == eCharacter) {

                //start a new sentence
                oNewSentence = new TOcrSentence;

                MakeSure(oNewSentence!=NULL, ERR_CREATEOBJECT, LEVEL_RECOVERABLE, L"oNewSentence");

                oNewSentence->AddRectangle(iPointer);
                oNewSentence->iSentenceNumber = oSentences->oSentences.size();
                oCurrentRectangle->iSentenceNumber = oNewSentence->iSentenceNumber;

                GrowSentence(oNewSentence);

                oSentences->oSentences.push_back(oNewSentence);
                }

            iPointer++;
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    lTimeFindSentences += ::GetTickCount() - lStartTime;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::GrowSentence(TOcrSentence* poSentence) {

    //This function adds rectangles the the current rectangle. There are
    //two growth paths, to the left and to the right.

    int iLeftPointerInSentence, iRightPointerInSentence;
    long iNextRectangle;
    bool bGrown;
    TOcrRectangle* oTempRectangle;

    iLeftPointerInSentence = 0;
    iRightPointerInSentence = 0;
    bGrown = true;

    try {

        while (bGrown) {

            bGrown = false;

            //Grow right
            iNextRectangle = FindClosestRectangle(poSentence, poSentence->GetRectangle(iRightPointerInSentence), 0, false);
            while (iNextRectangle != -1) {

                iRightPointerInSentence = poSentence->NumberOfRectangles();
                poSentence->AddRectangle((long)iNextRectangle);

                oTempRectangle = oRectangles->GetChild(iNextRectangle);
                oTempRectangle->iSentenceNumber = poSentence->iSentenceNumber;

                bGrown = true;

                iNextRectangle = FindClosestRectangle(poSentence, poSentence->GetRectangle(iRightPointerInSentence), 0, false);
                }

            //Grow left
            iNextRectangle = FindClosestRectangle(poSentence, poSentence->GetRectangle(iLeftPointerInSentence), 0, true);
            while (iNextRectangle != -1) {

                poSentence->InsertRectangle(0, (long)iNextRectangle);
                iRightPointerInSentence++;

                oTempRectangle = oRectangles->GetChild(iNextRectangle);
                oTempRectangle->iSentenceNumber = poSentence->iSentenceNumber;

                bGrown = true;

                iNextRectangle = FindClosestRectangle(poSentence, poSentence->GetRectangle(iLeftPointerInSentence), 0, true);
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }
    }
//---------------------------------------------------------------------------
int
__fastcall TOcrFindSentences::FindClosestRectangle(TOcrSentence* poSentence, long iRectangle, float fAngle, bool bLeft) {

    //this function searches the distance matrix for the closest rectangle
    //to the given rectangle. The closest rectangle must comply to
    //the following rules:
    // * Different from the given rectangle
    // * Rectangle must not be a part of another sentence
    // * Must be on the left side of given rectangle (if bLeft is true)
    // * Must be on the right side of given rectangle (if bLeft is false)
    // * Angle between rectangle and given rectangle must be less than 5 degrees

    int iRetValue;
    long iMinDistance;
    long iMaxAllowedDistance;
    double dAverageAngle;
    TOcrRectangle *oCurrentRectangle, *oCandidateNeighbour;

    long lAreaNumber;
    long lCurrentAreaNumber;
    long lCurrentRectangle;
    long lDistance;
    TOcrArea* oArea;

    iRetValue = -1;
    iMinDistance = 2000;

    try {

        MakeSure(poSentence!=NULL, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poSentence!=NULL");

        oCurrentRectangle = oRectangles->GetChild(iRectangle);

        dAverageAngle = poSentence->AverageAngle(oRectangles);
        iMaxAllowedDistance = (long)(poSentence->AverageHeight(oRectangles)*1.5);

        lAreaNumber = oCurrentRectangle->iAreaNumber;

        for (long lIndex=0; lIndex<6; lIndex++) {

            //the lIndex is converted to an area number according to the
            //matrix below. The start point is the center area (lIndex = 2).
            //lIndex 3 is on the left when bleft = true or on the right when bleft = false

            //  *************
            //  * 1 * 0 * 1 *
            //  *************
            //  * 3 * 2 * 3 *
            //  *************
            //  * 5 * 4 * 5 *
            //  *************

            lCurrentAreaNumber = lAreaNumber;

            //translate the lIndex to the x axis of the areas
            if (lIndex & 1) {

                if (bLeft) {

                    lCurrentAreaNumber--;
                    }
                else {

                    lCurrentAreaNumber++;
                    }
                }

            //translate the lIndex to the y axis of the areas
            lCurrentAreaNumber += ((lIndex/2)-1) * lNumberAreasX;   

            //the current area number must ly within boundaries (0, number areas)
            //no throw here if it is outside the boundaries because when the area
            //lies on the edge of the image this algorithm will try to go further
            if (lCurrentAreaNumber>=0 && lCurrentAreaNumber<(long)oAreas.size()) {

                oArea = oAreas[lCurrentAreaNumber];

                //loop through all the rectangles in the area
                for (long lRectIndex=0; lRectIndex < (long)oArea->lRectangleNumbers.size(); lRectIndex++) {

                    lCurrentRectangle = oArea->lRectangleNumbers[lRectIndex];

                    //CurrentRectangle must be different from rectangle
                    if (lCurrentRectangle != iRectangle) {

                        oCandidateNeighbour = oRectangles->GetChild(lCurrentRectangle);

                        //Candidate neighbour must be free; not part of another sentence
                        if (oCandidateNeighbour->iSentenceNumber == -1 && oCandidateNeighbour->eType == eCharacter) {

                            //Candidate neighbour must be on left/right side
                            if ((bLeft && oCandidateNeighbour->x < oCurrentRectangle->x) ||
                                (!bLeft && oCandidateNeighbour->x > oCurrentRectangle->x)) {

                                lDistance = oRectangles->DistanceBetweenChildren(iRectangle, lCurrentRectangle);

                                //Distance must be between current minimum and Maximum allowed
                                if (iMinDistance > lDistance && iMaxAllowedDistance > lDistance) {

                                    //Angle between rectangle and sentence must be less than 5 degrees
                                    if (oRectangles->AngleBetweenChildren(lCurrentRectangle, iRectangle) < 5) {

                                        iMinDistance = lDistance;
                                        iRetValue = lCurrentRectangle;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return iRetValue;
    }

/*int
__fastcall TOcrFindSentences::FindClosestRectangle(TOcrSentence* poSentence, long iRectangle, float fAngle, bool bLeft) {

    //this function searches the distance matrix for the closest rectangle
    //to the given rectangle. The closest rectangle must comply to
    //the following rules:
    // * Different from the given rectangle
    // * Rectangle must not be a part of another sentence
    // * Must be on the left side of given rectangle (if bLeft is true)
    // * Must be on the right side of given rectangle (if bLeft is false)
    // * Angle between rectangle and given rectangle must be less than 5 degrees

    int iRetValue;
    long iMinDistance;
    long iMaxAllowedDistance;
    long iAverageHeight;
    double dAverageAngle;
    TOcrRectangle *oCurrentRectangle, *oCandidateNeighbour;

    iRetValue = -1;
    iMinDistance = 2000;
    oCurrentRectangle = oRectangles->GetChild(iRectangle);


//    iMaxAllowedDistance = max(oCurrentRectangle->width, oCurrentRectangle->height);
    dAverageAngle = poSentence->AverageAngle(oRectangles);
    iMaxAllowedDistance = poSentence->AverageHeight(oRectangles)*1.5;

    for (long i=0; i<oRectangles->GetNumberChilds(); i++) {

        if (i != iRectangle) {
           if (iMinDistance > oDistanceMatrix[i][iRectangle]) {
                if (iMaxAllowedDistance > oDistanceMatrix[i][iRectangle]) {
                    oCandidateNeighbour = oRectangles->GetChild(i);
                    if (oCandidateNeighbour->iSentenceNumber ==-1) {
                        //if ((oCandidateNeighbour->height < oCurrentRectangle->height*5) {
                            if ((bLeft && oCandidateNeighbour->x < oCurrentRectangle->x) ||
                                (!bLeft && oCandidateNeighbour->x > oCurrentRectangle->x)) {

                                if (oRectangles->AngleBetweenChildren(i, iRectangle) < 5) {
                                    iMinDistance = oDistanceMatrix[i][iRectangle];
                                    iRetValue = i;
                                    }
                                }
                            }
                        //}
                    }
                }
            }
        }

    return iRetValue;
    }
*/
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::AddSpacesToSentences() {

    //this function add spaces to sentences. Space are added between two
    //characters when the space between the two is larger than
    //60 percent of the average width of all the characters in the sentence

    long lStartTime = ::GetTickCount();
    long lPointer;
    long lAverageWidthCharacters;
    long lWidthBetweenCharacters;
    int nSpaces;
    TOcrSentence* oSentence;

    try {

        for (unsigned int i=0; i<oSentences->oSentences.size(); i++) {

            oSentence = oSentences->oSentences[i];
            lPointer = oSentence->NumberOfRectangles() - 1;
            lAverageWidthCharacters = (long)(oSentence->AverageWidth(oRectangles) * 0.60);

            while (lPointer>0) {

                //lWidthBetweenCharacters = oDistanceMatrix[oSentence->GetRectangle(lPointer)][oSentence->GetRectangle(lPointer-1)];

                lWidthBetweenCharacters = oRectangles->DistanceBetweenChildren(oSentence->GetRectangle(lPointer), oSentence->GetRectangle(lPointer-1));

                if (lWidthBetweenCharacters >= lAverageWidthCharacters && lAverageWidthCharacters>0) {

                    nSpaces = lWidthBetweenCharacters / lAverageWidthCharacters;

                    for (int j=0; j<nSpaces; j++) {

                        oSentence->InsertRectangle(lPointer, -1);

                        }
                    }

                lPointer --;
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    lTimeAddSpaces += ::GetTickCount() - lStartTime;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::DrawSentences() {

    //this function is for debugging purposes only. It draws bounding boxes of
    //recognised sentences into the intermediate image

    long lStartTime = ::GetTickCount();

    int  iPointer;
    long xPos, yPos;
    long lIndex;
    int nSpaces;
    TOcrSentence* oSentence;
    TOcrRectangle* oRectangle;
    TOcrPoint cCurrentPoint, cPreviousPoint;

    BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);

    int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
    int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

    lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

    //calculate image numbers
    iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
    iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
    iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
    iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

     for (unsigned int i=0; i<oSentences->oSentences.size(); i++) {

          oSentence = oSentences->oSentences[i];

          for (int j=0; j<oSentence->NumberOfRectangles() ; j++) {

            lIndex = oSentence->GetRectangle(j);
            nSpaces=0;
            while (lIndex==-1 && j<oSentence->NumberOfRectangles()) {
                j++;
                nSpaces++;
                lIndex = oSentence->GetRectangle(j);
                }

            if (j<oSentence->NumberOfRectangles()) {
                oRectangle = oRectangles->GetChild(oSentence->GetRectangle(j));

                cCurrentPoint = oRectangle->GetCenterPoint();
                xPos = cCurrentPoint.x - 1;
                yPos = cCurrentPoint.y - 1;

                //draw center point
                for (int xPointer=xPos; xPointer<xPos+3; xPointer++) {
                    for (int yPointer=yPos; yPointer<yPos+3; yPointer++) {
                        iPointer = xPointer + (iImageHeight - yPointer -1)*iRowLength;
                        lpBits[iPointer]= 0xFE;
                        }
                    }

                if (j>0) {
                    DrawLine(cPreviousPoint, cCurrentPoint, nSpaces);
                    }

                cPreviousPoint = cCurrentPoint;
                }
            }

        //drawbounding box
        cPreviousPoint.x = oSentence->x;
        cPreviousPoint.y = oSentence->y;

        cCurrentPoint = cPreviousPoint;
        cCurrentPoint.x += oSentence->width;

        DrawLine(cPreviousPoint, cCurrentPoint, 0);

        cPreviousPoint = cCurrentPoint;
        cCurrentPoint.y += oSentence->height;

        DrawLine(cPreviousPoint, cCurrentPoint, 0);

        cPreviousPoint = cCurrentPoint;
        cCurrentPoint.x -= oSentence->width;

        DrawLine(cPreviousPoint, cCurrentPoint, 0);

        cPreviousPoint = cCurrentPoint;
        cCurrentPoint.y -= oSentence->height;

        DrawLine(cPreviousPoint, cCurrentPoint, 0);
        }




    ::GlobalUnlock(hImageHandle);

    lTimeDrawSentences += ::GetTickCount() - lStartTime;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::DrawLine(TOcrPoint pcFromPoint, TOcrPoint pcToPoint, int pnSpaces) {

    //This function is for debugging purposes only. It draws a line between to two point and
    //draws rectanlges in the middle. The number of rectangle represents the number of
    //spaces between to rectangles

    int xFrom, xTo;
    int yFrom, yTo;
    int xPos, yPos;
    int xDelta, yDelta;
    int xMoved, yMoved;
    int iPointer;
    double dDirection;

    xFrom = pcFromPoint.x;
    yFrom = pcFromPoint.y;

    xTo = pcToPoint.x;
    yTo = pcToPoint.y;

    dDirection = 0;

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

        while (xPos != xTo) {

            xMoved += xDelta;
            xPos += xDelta;

            yPos = (long)(pcFromPoint.y + xMoved * dDirection);

            iPointer = xPos + (iImageHeight - yPos -1)*iRowLength;
            lpBits[iPointer]= 0xFE;
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

        while (yPos != yTo) {

            yMoved += yDelta;
            yPos += yDelta;

            xPos = (long)(pcFromPoint.x + yMoved * dDirection);

            iPointer = xPos + (iImageHeight - yPos -1)*iRowLength;
            lpBits[iPointer]= 0xFE;
            }
        }

        //spaties erin tekenen
        for (int n=0; n<pnSpaces; n++) {

            xPos = abs(xFrom + xTo) / 2;
            yPos = abs(yFrom + yTo) / 2 - ((pnSpaces * 5)/2) + n*5;

            for (int x=xPos-1; x<xPos+3; x++) {

                for (int y=yPos-1; y<yPos+3; y++) {

                    if (x>0 && y>0 && x<iImageWidth && y<iImageHeight) {

                        iPointer = x + (iImageHeight - y -1)*iRowLength;
                        lpBits[iPointer]= 0xFD;

                        }
                    }

                }
            }
    }

//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::ExportAreas() {

    //this function is for debugging purposes only. It write
    //the distance matrix to disc: 'c:\\areas.txt'

    //char cBuffer[100];
    //char cNumber[10];
    //TFileStream* out;
    //int iNumberChilds;


    //out = new TFileStream("c:\\areas.txt", fmCreate);

    //for (int i=0; i<oAreas.size(); i++) {


    //        itoa(i, cNumber, 10);
    //        strcpy(cBuffer, cNumber);
    //        strcat(cBuffer, "\t");

    //        itoa(oAreas[i]->lRectangleNumbers.size(), cNumber, 10);
    //        strcat(cBuffer, cNumber);
    //        strcat(cBuffer, "\r\n");

    //        out->WriteBuffer(&cBuffer, strlen(cBuffer));
    //        }
    // delete out;

    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::CalculateBoundingBox() {

    //this function calculates the bounding box of the sentence.

    TOcrSentence* oSentence;
    TOcrRectangle* oRectangle;
    long lTop;
    long lBottom;


    for (unsigned long lSentenceIndex=0; lSentenceIndex<oSentences->oSentences.size(); lSentenceIndex++) {

        oSentence = oSentences->oSentences[lSentenceIndex];

        oRectangle = oRectangles->GetChild(oSentence->GetRectangle(0));

        oSentence->x = oRectangle->x;
        oSentence->y = oRectangle->y;

        lTop = oRectangle->y;
        lBottom = oRectangle->y + oRectangle->height;

        //find the top and bottom of the sentence
        for (unsigned long lRectangleIndex=0; lRectangleIndex<(unsigned long)oSentence->NumberOfRectangles(); lRectangleIndex++) {

            oRectangle = oRectangles->GetChild(oSentence->GetRectangle(lRectangleIndex));
            oSentence->width = oRectangle->x + oRectangle->width - oSentence->x;

            if (lTop>oRectangle->y) lTop = oRectangle->y;
            if (lBottom<oRectangle->y + oRectangle->height) lBottom = oRectangle->y + oRectangle->height;
            }

        oSentence->y = lTop;
        oSentence->height = lBottom - lTop;
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::CleanUpSentences() {

    //this function cleans up the collection of sentences. With cleaning up
    //we mean that it removes excessively large sentences/rectangles and that
    //it removes sentences which are 90% bounded by other sentences. Most of
    //the time these bounded sentences are a collection of points belong to
    //characters in the bounding sentence.

    long lStartTime = ::GetTickCount();

    TOcrSentence* oOuterSentence;
    TOcrSentence* oInnerSentence;

    //step 1: remove excessively large sentences

    //step 2: merge sentences
    try {

        for (long lSentenceIndex=0; lSentenceIndex<(long)oSentences->oSentences.size(); lSentenceIndex++) {

            oOuterSentence = oSentences->oSentences[lSentenceIndex];

            for (long lSentenceStart = 0; lSentenceStart<(long)oSentences->oSentences.size(); lSentenceStart++) {

                if (lSentenceStart!=lSentenceIndex) {

                    oInnerSentence = oSentences->oSentences[lSentenceStart];

                    if (SentenceWithinSentence(oOuterSentence, oInnerSentence, 2)) {

                        //clear the sentence, but don't delete it
                        while (oInnerSentence->NumberOfRectangles()>0) {

                            MergeRectangles(oInnerSentence, oOuterSentence);

                            oInnerSentence->DeleteRectangle(0);
                            }

                        oInnerSentence->x = 0;
                        oInnerSentence->y = 0;
                        oInnerSentence->width = 0;
                        oInnerSentence->height = 0;
                        }
                    }
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    lTimeCleanUpSentences += ::GetTickCount() - lStartTime;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrFindSentences::MergeRectangles(TOcrSentence* poInnerSentence, TOcrSentence* poOuterSentence) {

    //this function takes a to be deleted rectangle and sees whether it could
    //be a part of another rectangle in the outer sentence. It could be a
    //dot for example. If so, it will merge both rectangles.

    long lInnerRectangle;
    long lOuterRectangle;

    TOcrRectangle* oNewRectangle;

    long lMergeRectangle = -1;
    long lMinDistance = 1000;
    long lBestAngle = 90;
    long lDistance;
    long lAngle;
    long lAverageAngle;
    long lNewRectangle;

    try {

        MakeSure(poInnerSentence!=NULL, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poInnerSentence!=NULL");
        MakeSure(poOuterSentence!=NULL, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poOuterSentence!=NULL");

        lInnerRectangle = poInnerSentence->GetRectangle(0);
        lAverageAngle = (long)poOuterSentence->AverageAngle(oRectangles);

        //we merge two rectangles if:
        //1. the distance between them is the  minimum in comparing to other pairs in the sentece
        //2. if the angle between them is the best 90 degress angle in comparision to the average angle

        for (long lIndex=0; lIndex<poOuterSentence->NumberOfRectangles(); lIndex++) {

            lOuterRectangle = poOuterSentence->GetRectangle(lIndex);

            lDistance = oRectangles->DistanceBetweenChildren(lInnerRectangle, lOuterRectangle);

            if (lDistance<+lMinDistance) {

                lMinDistance = lDistance;

                lAngle = (long)oRectangles->AngleBetweenChildren(lInnerRectangle, lOuterRectangle);
                lAngle = (lAngle % 180) - 90 + lAverageAngle;

                if (abs(lAngle) <=5) {

                    if (abs(lAngle) <= lBestAngle) {

                        lBestAngle = abs(lAngle);
                        lMergeRectangle = lIndex;
                        }
                    }
                }
            }

        if (lMergeRectangle!=-1) {

            //we found a rectangle with which we can merge
            lNewRectangle = oRectangles->MergeRectangles(poOuterSentence->GetRectangle(lMergeRectangle), lInnerRectangle);

            if (lNewRectangle!=-1) {

                poOuterSentence->UpdateRectangle(lMergeRectangle, lNewRectangle);

                oNewRectangle = oRectangles->GetChild(lNewRectangle);
                oNewRectangle->iSentenceNumber = poOuterSentence->iSentenceNumber;
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }
    }
//---------------------------------------------------------------------------
bool
__fastcall TOcrFindSentences::SentenceWithinSentence(TOcrSentence* poSentenceOuter, TOcrSentence* poSentenceInner, long plPixelMargin) {

    //this function tests if the inner sentence really lies inside the
    //outer sentence by comparing their bounding boxes. A margin can be used
    //so that this function still return true if the box lies with one row or two columns
    //outside the outer sentence
    bool bReturnValue;

    bReturnValue = false;

    try {

        MakeSure(poSentenceOuter!=NULL, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poSentenceOuter!=NULL");
        MakeSure(poSentenceInner!=NULL, ERR_ASSERTIONFAILED, LEVEL_RECOVERABLE, L"poSentenceInner!=NULL");

        if (poSentenceOuter->x - plPixelMargin <= poSentenceInner->x) {

            if (poSentenceOuter->y - plPixelMargin <= poSentenceInner->y) {

                if (poSentenceOuter->x + poSentenceOuter->width + plPixelMargin >= poSentenceInner->x + poSentenceInner->width) {

                    if (poSentenceOuter->y + poSentenceOuter->height + plPixelMargin >= poSentenceInner->y + poSentenceInner->height) {

                        bReturnValue = true;
                        }
                    }
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return bReturnValue;
    }
//---------------------------------------------------------------------------

