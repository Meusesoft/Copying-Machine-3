//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrThread.h"
#include "tinyxml/tinyxml.h"
#include "TOcrRecogniseCharacter.h"
#include "TError.h"
//---------------------------------------------------------------------------

TOcrRecogniseCharacter::TOcrRecogniseCharacter(HGLOBAL phImageHandle, 
											   TOcrRectangle* poOcrRectangle, 
											   TOcrSentences* poOcrSentences, 
											   TOcrCharacterShapes* poOcrCharacterShapes,
											   TDataLayer* poDataLayer,
											   bool CreateSuspended)
    : TOcrThread(CreateSuspended)
{
    hImageHandle = phImageHandle;
    oOcrRectangles = poOcrRectangle;
    oOcrSentences = poOcrSentences;
    oOcrCharacterShapes = poOcrCharacterShapes;
	oDataLayer = poDataLayer;

    bSaveImage = false;
    bSaveResults = false;
    bSaveXML = false;

    oError = TError::GetInstance();
    }
//---------------------------------------------------------------------------
void __fastcall TOcrRecogniseCharacter::Execute()
{
    int iReturnValue;

    iReturnValue = 0;

    try {

        //create and init data layer. The data layers contains functions for
        //preprocessing bitmaps for recognition.
        lTimerLoadXML = ::GetTickCount();

        //oDataLayer = new TDataLayer();
        MakeSure(oDataLayer!=NULL, ERR_CREATEOBJECT, LEVEL_WARNING, L"oDataLayer");

        //oDataLayer->LoadShapes("D:\\Projects\\CopyingMachine3\\debug\\shapenet.xml");

        lTimerLoadXML = ::GetTickCount() - lTimerLoadXML;

        //create a bitmap and usable pointers out of the memory handle.
        BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hImageHandle);
        MakeSure(lpSourceBitmapInfo!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lpSourceBitmapInfo");

        int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
        int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

        lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

        //calculate image numbers
        iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
        iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
        int  iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
        iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

        //work through all the sentences
        TOcrSentence*  oOcrSentence;

        for (unsigned int i=0; i<oOcrSentences->oSentences.size(); i++) {

            oOcrSentence = oOcrSentences->oSentences[i];

            ProcessSentence(oOcrSentence);
            }

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }

    //terminate this thread
    ReturnValue = iReturnValue;
    }
//---------------------------------------------------------------------------

void
__fastcall TOcrRecogniseCharacter::ProcessSentence(TOcrSentence* poOcrSentence) {

    int iStartIndex, iEndIndex, iNumberRectangles;

    try {
        MakeSure(poOcrSentence!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"poOcrSentence==NULL");

        //walk through all the rectangles of the sentences and
        //find the words (they're seperated by rectangle index -1).
        iStartIndex=0;
        iEndIndex=0;
        iNumberRectangles = poOcrSentence->NumberOfRectangles();

        //first process step, recognise characters;
        while (iStartIndex<iNumberRectangles) {

            iEndIndex++;

            if (iEndIndex>iNumberRectangles-1) {

                //We reached the end of the sentence, process the last word
                ProcessWord(poOcrSentence, iStartIndex, iNumberRectangles-1);

                iStartIndex = iEndIndex+1;
                iEndIndex = iStartIndex-1;
                }
            else {

                if (poOcrSentence->GetRectangle(iEndIndex) == -1) {

                   //We found a space, process the last word
                    ProcessWord(poOcrSentence, iStartIndex, iEndIndex-1);

					while ((poOcrSentence->GetRectangle(iEndIndex) == -1) && 
						(iEndIndex<iNumberRectangles)) {

							iEndIndex++;
						}

                    iStartIndex = iEndIndex;
                    iEndIndex = iStartIndex-1;
                    }
                }
            }

        /*//second step, try to split apart possible touching characters. Indications
        //for touching characters are: 1. no suggested characters and 2. the first
        //recognized character has a recognition score of more than 1000. A score of 0 is
        //a perfect match
        iStartIndex=iNumberRectangles;

        while (iStartIndex>0) {

            iRectNumber = poOcrSentence->GetRectangle(iStartIndex-1);

            if (iRectNumber!=-1) {

                oRectangle = oOcrRectangles->GetChild(iRectNumber);

                if (oRectangle->bTouchingCharacters) {

                    //SplitCharacter(oRectangle, poOcrSentence, iStartIndex-1);
                    }
                }

            iStartIndex--;
            }


        //third step, reconsider the recognition by the dimensions of the characters
        //top line, base line, bottom line and if they correspond to the suggested
        //characters. This is for determining the differences between a 's' and a 'S'
        // for example.
        ReconsiderMethod1(poOcrSentence);*/

        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }
    }

//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::ProcessWord(TOcrSentence* poSentence, int iStartIndex, int iEndIndex) {

    //TOcrRectangle* oOcrRectangle;
    //bool bFound;
    //int iCounter;
    try {

        MakeSure(iStartIndex <= iEndIndex, ERR_ASSERTIONFAILED, LEVEL_WARNING, L"iStartIndex <= iEndIndex");

        for (int i=iStartIndex; i<=iEndIndex; i++) {

            ProcessCharacterBitmap(poSentence, i);
            }

        /*for (int i=iStartIndex; i<=iEndIndex; i++) {

            oOcrRectangle = oOcrRectangles->GetChild(poSentence->GetRectangle(i));
            bFound = false;
            iCounter = 0;
            } */
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }

//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::ProcessCharacterBitmap(TOcrSentence* poOcrSentence, int iIndex) {

    TOcrRectangle* oOcrRectangle;
    BYTE* lTempSpace;
    HGLOBAL hTempSpace;
    int RectX, RectY, RectW, RectH;
    long lSourcePointer, lDestinationPointer;
    int nGaps;
    int iRectNumber;

    try {
        //Get the rectangle from the collection of rectangles
        oOcrRectangle = oOcrRectangles->GetChild(poOcrSentence->GetRectangle(iIndex));

        MakeSure(oOcrRectangle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oOcrRectangle=NULL");

        //first we make a work image of the character. That workimage is surrounded
        //by a rectangle of white pixels. The extra space is needed to be able to
        //create a border of vectors around it.

        oOcrRectangle->eType = eCharacter;

        RectX = oOcrRectangle->x;
        RectY = oOcrRectangle->y;
        RectW = oOcrRectangle->width+1+2;
        RectH = oOcrRectangle->height+1+2;

        //0. create temporary workspace
        hTempSpace = GlobalAlloc(GMEM_MOVEABLE, RectW * RectH);
        MakeSure(hTempSpace!=NULL, ERR_OUTOFMEMORY, LEVEL_RECOVERABLE, L"hTempSpace=NULL");

        lTempSpace = (BYTE*)GlobalLock(hTempSpace);
        MakeSure(lTempSpace!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lTempSpace=NULL");

        FillMemory(lTempSpace, RectW*RectH, 255);

        //1. copy rectangle into temporary workspace
        for (int y=0; y<RectH-2; y++) {

            lSourcePointer = RectX + (iImageHeight - RectY - y  - 1) * iRowLength;
            lDestinationPointer = (RectH - y - 2) * RectW + 1;

            CopyMemory(lTempSpace + lDestinationPointer, lpBits + lSourcePointer, RectW - 2);
            }

        //this is a temporary thing
        oOcrRectangle->width+=2;
        oOcrRectangle->height+=2;

        //2. Isolate connected component
        //2.1 change black pixels of connected components into colorcode 4
        for (unsigned long lIndex=0; lIndex < oOcrRectangle->oConnectedComponentPoints.size(); lIndex++) {
            GrowConnectedComponent(lTempSpace, oOcrRectangle,
                                        oOcrRectangle->oConnectedComponentPoints[lIndex].x + 1,
                                        oOcrRectangle->oConnectedComponentPoints[lIndex].y + 1, 0, 4,8);
            }
        iRectNumber = poOcrSentence->GetRectangle(iIndex);

        //2.2 remove all other black pixels: they belong to other objects
        for (int y=0; y<RectH; y++) {

            lDestinationPointer = (RectH - y - 1) * RectW;

            for (int x=0; x<RectW; x++) {

                if (lTempSpace[lDestinationPointer]==0) {
                    lTempSpace[lDestinationPointer] = 0xFF;
                    }

                lDestinationPointer++;
                }
            }

        //3. find white space around component (if present)
        //3.1 move through outer rectangle, and paint all white space
        //    into other color (0)
        //
            //right
           lDestinationPointer = (RectH-1)*RectW + RectW - 1;

            for (int y=0; y<RectH; y++) {

                if (lTempSpace[lDestinationPointer]==0xFF) {
                    GrowConnectedComponent(lTempSpace, oOcrRectangle, RectW-1, y, 255, 0,8);
                    }
                lDestinationPointer -= RectW;
                }

            //top
            lDestinationPointer = (RectH - 1) * RectW;

            for (int x=0; x<RectW; x++) {

                if (lTempSpace[lDestinationPointer]==0xFF) {
                    GrowConnectedComponent(lTempSpace, oOcrRectangle, x, 0, 255, 0,8);
                    }
                lDestinationPointer++;
                }

            //bottom
            lDestinationPointer = 0;

            for (int x=0; x<RectW; x++) {

                if (lTempSpace[lDestinationPointer]==0xFF) {
                    GrowConnectedComponent(lTempSpace, oOcrRectangle, x, RectH-1, 255, 0,8);
                    }
                lDestinationPointer++;
                }

            //left
           lDestinationPointer = (RectH-1)*RectW ;

            for (int y=0; y<RectH; y++) {

                if (lTempSpace[lDestinationPointer]==0xFF) {
                    GrowConnectedComponent(lTempSpace, oOcrRectangle, 0, y, 255, 0, 8);
                    }
                lDestinationPointer -= RectW;
                }

        //4. search for white connected components, they are inner gaps
        //   and count them and give color 252. more then 2 might be an
        //   indicator for an image.
        nGaps = 0;

        for (int y=0; y<RectH; y++) {

            lDestinationPointer = (RectH - y - 1) * RectW;
            for (int x=0; x<RectW; x++) {

                if (lTempSpace[lDestinationPointer]==255) {
                    if (GrowConnectedComponent(lTempSpace, oOcrRectangle, x, y, 255, 0, 3)) {

                        nGaps++;

                        }
                    }
                lDestinationPointer++;
                }
            }

        oOcrRectangle->iGaps = nGaps;
        if (nGaps>2) {

            oOcrRectangle->eType = eImageRect;
            };

        int iCharacterX;
        int iCharacterY;
        BYTE bPoint;

        BYTE* bCompareGrid;

        //create the compare matrix, meaning translate the original
        //character to a 32x32 grid
        bCompareGrid = (BYTE*)malloc(32*32);
        MakeSure(bCompareGrid!=NULL, ERR_OUTOFMEMORY, LEVEL_RECOVERABLE, L"bCompareGrid=NULL");

        for (int y=0; y<32; y++) {

            iCharacterY = (int)((float)((RectH-2) * y) / 32) ;

            for (int x=0; x<32; x++) {

                iCharacterX = (int)((float)((RectW-2) * x) / 32);

                lDestinationPointer = (RectH - iCharacterY - 2) * RectW + iCharacterX + 1;

                bPoint = lTempSpace[lDestinationPointer];

                lDestinationPointer = x + y*32;

                MakeSure(lDestinationPointer<1024, ERR_INVALIDPOINTER, LEVEL_WARNING, L"lDestinationPointer>=1024");
                bCompareGrid[lDestinationPointer] = bPoint;
                }
            }

        //AddDistanceToCompareGrid(bCompareGrid);

        double dRatio;

        MakeSure(oOcrRectangle->height-2>0, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oOcrRectangle->height-2<=0");
        dRatio = ((oOcrRectangle->width-2) * 10) / (oOcrRectangle->height-2);

        /* ADD SHAPE RECOGNITION HERE */
        //Do a thinning procedure over the bitmap
            oDataLayer->DoThinning(bCompareGrid, 4, 2);
            oDataLayer->DoThinning(bCompareGrid, 0, 2);

        //analyse the stroke direction within the bitmap
            oDataLayer->FindStrokes(bCompareGrid, -1);

        //recognise this bitmap
            cRecognition oRecognition;
            oRecognition.cCompareBitmap = bCompareGrid;

            oDataLayer->RecognizeBitmapOnShapeNet(oRecognition);
//            oDataLayer->RecognizeBitmap(oRecognition);

        //Save for debug the rectangle
         if (bSaveImage || bSaveResults) {

            if (bSaveImage) {
				SaveRectangleImage32(bCompareGrid, oOcrRectangle, iRectNumber);
				SaveRectangleImage(lTempSpace, oOcrRectangle, iRectNumber, RectW, RectH);
				}
			if (bSaveResults)	SaveRecognitionInformation(oRecognition, oOcrRectangle, iRectNumber);
            }

		 //fill the suggestions
            TOcrCharacterSuggestion oSuggestion;

            oOcrRectangle->oCharacterSuggestions.clear();

            for (long lScores=0; lScores<(long)oRecognition.oScores.size(); lScores++) {

                oSuggestion.cCharacter = (char)oRecognition.oScores[lScores].sTitle[1];
                oSuggestion.sCharacter = oRecognition.oScores[lScores].sTitle;
                oSuggestion.lScore = (long)(oRecognition.oScores[lScores].fScore * 1000);
                oSuggestion.lShapeIndex = oRecognition.oScores[lScores].lId;

                MakeSure(oOcrRectangle->oCharacterSuggestions.max_size()>oOcrRectangle->oCharacterSuggestions.size()+1, ERR_OUTOFMEMORY, LEVEL_RECOVERABLE, L"maxsize<=size");
                oOcrRectangle->oCharacterSuggestions.push_back(oSuggestion);
                }

        //free allocated memory
        free(bCompareGrid);
        GlobalUnlock(hTempSpace);
        GlobalFree(hTempSpace);

        oOcrRectangle->width-=2;
        oOcrRectangle->height-=2;

        //process the recognition, see if it is a possible connected character
        if (oOcrRectangle->oCharacterSuggestions.size() > 0) {
            oOcrRectangle->sContent = oOcrRectangle->oCharacterSuggestions[0].sCharacter;

            if (/*oOcrRectangle->oCharacterSuggestions[0].lScore <= 250
                    || */oOcrRectangle->sContent == L"connected") {

                oOcrRectangle->sContent = L"*";
                oOcrRectangle->bTouchingCharacters = true;
                }
            if (oOcrRectangle->sContent == L"garbage") {

                oOcrRectangle->sContent = L"*";
                }
            }
        else{
            oOcrRectangle->sContent = L"*";
            oOcrRectangle->bTouchingCharacters = true;
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(ERR_UNKNOWN, LEVEL_WARNING, L"");
        }
    }
//---------------------------------------------------------------------------

void
__fastcall TOcrRecogniseCharacter::SplitCharacter(TOcrRectangle* poOcrRectangle,
                    TOcrSentence* poOcrSentence, int iPosition) {

   //first try to guess how many characters may be involved here... therefor we take
   //the average width of the recognised characters and divide the width of the current
   //rectangle with that average. This will only work if there are characters recognised...
   long lTotalWidth;
   float fAverageWidth;
   long lRecognisedCharacters;
   long lNumberPropositions;
   TOcrRectangle* oRectangle;
   int  iRectangleIndex;

   lTotalWidth = 0;
   lRecognisedCharacters = 0;

   for (long lIndex=0; lIndex<poOcrSentence->NumberOfRectangles(); lIndex++) {

        iRectangleIndex = poOcrSentence->GetRectangle(lIndex);

        if (iRectangleIndex!=-1) {

            oRectangle = oOcrRectangles->GetChild(iRectangleIndex);

            if (!oRectangle->bTouchingCharacters) {

                lRecognisedCharacters++;
                lTotalWidth += oRectangle->width;
                }
            }
        }

    if (lRecognisedCharacters>0) {

        fAverageWidth = (float)lTotalWidth / (float)lRecognisedCharacters;

        /*lNumberCharacters = poOcrRectangle->width / fAverageWidth;
        if (lNumberCharacters<=1) {
            lNumberCharacters=2;
            }     */
        }
    else {

        //lNumberCharacters = 2;
        fAverageWidth = 1;
        }

    //create temporary image of rectangle, with this image we can determine
    //the sizes of the splitted rectangles
        long lSourcePointer, lDestinationPointer;
        BYTE* hRectangleImage;
        long lRectW, lRectH, lRectX, lRectY;

    hRectangleImage = (BYTE*)malloc((poOcrRectangle->width+1) * (poOcrRectangle->height+1));

    if (hRectangleImage!=NULL) {

        poOcrRectangle->eType = eSplitCharacter;

        lRectW = poOcrRectangle->width+1;
        lRectH = poOcrRectangle->height+1;
        lRectX = poOcrRectangle->x;
        lRectY = poOcrRectangle->y;

        FillMemory(hRectangleImage, lRectW * lRectH, 255);

        //1. copy rectangle into temporary workspace
        for (int y=0; y<lRectH; y++) {

            lSourcePointer = lRectX + (iImageHeight - lRectY - y  - 1) * iRowLength;
            lDestinationPointer = (lRectH - y - 1) * lRectW;

            CopyMemory(hRectangleImage + lDestinationPointer, lpBits + lSourcePointer, lRectW);

            }

        //2. Isolate connected component
        //2.1 change black pixels of connected components into colorcode 4
        for (unsigned long lIndex=0; lIndex < poOcrRectangle->oConnectedComponentPoints.size(); lIndex++) {
            GrowConnectedComponent((BYTE*)hRectangleImage, poOcrRectangle,
                                            poOcrRectangle->oConnectedComponentPoints[lIndex].x + 1,
                                            poOcrRectangle->oConnectedComponentPoints[lIndex].y + 1, 0, 4, 8);
            }

        //2.2 remove all other black pixels: they belong to other objects
        for (int y=0; y<lRectH; y++) {

            lDestinationPointer = (lRectH - y - 1) * lRectW;

                for (int x=0; x<lRectW; x++) {

                    if (hRectangleImage[lDestinationPointer]==0) {
                        hRectangleImage[lDestinationPointer] = 0xFF;
                        }

                    lDestinationPointer++;
                    }
                }

        //try to find a good fit for the seperation lines by moving them a bit to the left and right
        //and choose the combination with the lowest score.
        vector<long> oSeperatorLines;

        long lStartX = 0;
        long lDeltaX;
        long lRectNumber = poOcrSentence->GetRectangle(iPosition);
        long lConnectedPixels;

        sMinimaMaxima* oTopMM;
        sMinimaMaxima* oBottomMM;

        oTopMM = new sMinimaMaxima;
        oBottomMM = new sMinimaMaxima;

        FindMimimaAtTop(hRectangleImage, lRectW, lRectH, oTopMM);
        FindMaximaAtBottom(hRectangleImage, lRectW, lRectH, oBottomMM);

        //find minima and maxima in image
        oSeperatorLines.push_back(0); //add the start of the first rectangle

        for (long x=(long)(fAverageWidth/3); x<(long)(lRectW-fAverageWidth/3); x++) {

            lConnectedPixels = MaxConnectedPixelsOnVerticalLine(hRectangleImage, lRectW, lRectH, x);

            //indication 1: maximum connected pixels on vertical line is 1 or 2
            if (lConnectedPixels == 1 || lConnectedPixels == 2) {

                lStartX = x;

                do {

                    x++;

                    if (x < lRectW) {
                        lConnectedPixels = MaxConnectedPixelsOnVerticalLine(hRectangleImage, lRectW, lRectH, x);
                        }
                        
                    } while (x < lRectW && (lConnectedPixels==1 || lConnectedPixels==2));


                lDeltaX = (x - lStartX)/2;
                x -= lDeltaX;

                oSeperatorLines.push_back(x);

                x+= lDeltaX;

                if (lDeltaX<1) {
                    x++;
                    }
                }
            else {

                //indication 2 there is a local maximum at the same point as a local minimum and the space
                //between them is at least smallen than half of the height of the rectangle
                if (oTopMM->oColumn[x].bMinimum || oBottomMM->oColumn[x].bMaximum) {

                    /*LocalMinimumAtTop(hRectangleImage, lRectW, lRectH, x) &&
                    LocalMaximumAtBottom(hRectangleImage, lRectW, lRectH, x) &&
                    MaxConnectedPixelsOnVerticalLine(hRectangleImage, lRectW, lRectH, x) < (lRectH/2)) {
                    */

                    oSeperatorLines.push_back(x);

                    x+=2; //to prevent blocks of size 0, these are meaningless

                    //x += 2; // a minimum is calculated in a span of 5 columns, so the next 2 are still involvled
                    }
                }
            }

        oSeperatorLines.push_back(lRectW);

        oTopMM->oColumn.clear();
        delete oTopMM;
        oBottomMM->oColumn.clear();
        delete oBottomMM;







        lNumberPropositions = oSeperatorLines.size();

        long lNextPropositionStart;
        long lTempBestScore;
        long lTempNextPropositionStart;
        long lLine;
        long lTestPoint;
        long lNewRectangle;
        bool bConnectedPointFound;
        bool bContinue;
        TOcrSentence* oTempSentence;
        TOcrRectangle* oBestRectangle;
        TOcrPoint oConnectedPoint;
        TOcrRectangle* oBestRectangles;
        TOcrRectangle* oTempRectangle;

        oBestRectangles = new TOcrRectangle();

        if (lNumberPropositions>2) {


            oTempSentence = new TOcrSentence();

            lNextPropositionStart = 0;
            oBestRectangle = NULL;

            while (lNextPropositionStart < lNumberPropositions - 1) {

                lTempBestScore = 10000;
                lTempNextPropositionStart = lNextPropositionStart;

                for (long lStartPoint = lNumberPropositions - 1; lStartPoint > lNextPropositionStart; lStartPoint--) {

                    //configure the next rectangle to test
                    oTempRectangle = new TOcrRectangle();

                    //set the dimensions of the rectangle
                    long lEnd = oSeperatorLines[lStartPoint];
                    long lStart = oSeperatorLines[lNextPropositionStart];

                    oTempRectangle->width = lEnd - lStart;
                    oTempRectangle->height = poOcrRectangle->height;
                    oTempRectangle->x = poOcrRectangle->x + oSeperatorLines[lNextPropositionStart];
                    oTempRectangle->y = poOcrRectangle->y;

                    //change dimensions of the rectangle (empty lines on top and bottom of image)
                    bContinue = true;

                    while (bContinue && oTempRectangle->height > 0) {

                        bContinue = (PixelsOnHorizontalLine(hRectangleImage, oTempRectangle->x - poOcrRectangle->x, oTempRectangle->width, lRectW, lRectH, oTempRectangle->height) == 0);

                        if (bContinue) {
                            oTempRectangle->height--;
                            }
                        }

                    bContinue = true;
                    lLine = 0;

                    while (bContinue && oTempRectangle->height > 0) {

                        bContinue = (PixelsOnHorizontalLine(hRectangleImage, oTempRectangle->x - poOcrRectangle->x, oTempRectangle->width, lRectW, lRectH, lLine) == 0);

                        if (bContinue) {
                            lLine++;
                            oTempRectangle->y++;
                            oTempRectangle->height--;
                            }
                        }

                    //set the connected component point of the new rectangle
                    bConnectedPointFound = false;

                    for (int y=0; y<oTempRectangle->height+1 && !bConnectedPointFound; y++) {

                        lTestPoint = (lRectH - y - (oTempRectangle->y - poOcrRectangle->y) - 1) * lRectW;
                        lTestPoint +=  oTempRectangle->x - poOcrRectangle->x;

                        for (int x=0; x<oTempRectangle->width && !bConnectedPointFound; x++) {

                            if (hRectangleImage[lTestPoint]!=0xFF) {

                                oConnectedPoint.x = x;
                                oConnectedPoint.y = y;

                                oTempRectangle->oConnectedComponentPoints.push_back(oConnectedPoint);

                                bConnectedPointFound = true;
                                }

                            lTestPoint++;
                            }
                        }

                     //test the result
                    oOcrRectangles->AddChild(oTempRectangle);
                    lNewRectangle = oOcrRectangles->GetNumberChilds() - 1;

                    oTempSentence->AddRectangle(lNewRectangle);


                    if (oTempRectangle->height > 0 && oTempRectangle->width > 0) {

                        ProcessCharacterBitmap(oTempSentence, 0);
                        }
                        
                    oTempSentence->DeleteRectangle(0);
                    oOcrRectangles->DeleteChild(lNewRectangle, false);

                    if (oTempRectangle->oCharacterSuggestions.size()>0) {

                        long lCurrentScore = oTempRectangle->oCharacterSuggestions[0].lScore;

                        if (lTempBestScore > lCurrentScore) {

                            if (oBestRectangle!=NULL) delete oBestRectangle;
                            oBestRectangle = oTempRectangle;

                            lTempBestScore = oTempRectangle->oCharacterSuggestions[0].lScore;
                            lTempNextPropositionStart = lStartPoint;
                            }
                        else {

                            delete oTempRectangle;
                            }
                        }
                    }

                //save the best rectangle
                lNextPropositionStart = lTempNextPropositionStart;

                if (lTempBestScore == 10000) {

                    //no characters found, the first part cannot be recognised
                    lNextPropositionStart++;
                    }
                else {
                    lTempBestScore = 10000;

                    //save the found rectangle in the best rectangle vector
                    oBestRectangles->AddChild(oBestRectangle);
                    oBestRectangle = NULL;
                    }
                }

            //BestRectangles now contains the best possible (sub optimal) division of the
            //original rectangle. Use this as a substitute for the original character
            poOcrSentence->DeleteRectangle(iPosition);

            for (long lIndex=0; lIndex<oBestRectangles->GetNumberChilds(); lIndex++) {

                oTempRectangle = new TOcrRectangle();

                oRectangle = oBestRectangles->GetChild(lIndex);

                oTempRectangle->x = oRectangle->x;
                oTempRectangle->y = oRectangle->y;
                oTempRectangle->width = oRectangle->width;
                oTempRectangle->height = oRectangle->height;

                oConnectedPoint.x = oRectangle->oConnectedComponentPoints[0].x;
                oConnectedPoint.y = oRectangle->oConnectedComponentPoints[0].y;

                oTempRectangle->oConnectedComponentPoints.push_back(oConnectedPoint);

                oOcrRectangles->AddChild(oTempRectangle);
                lNewRectangle = oOcrRectangles->GetNumberChilds() - 1;

                poOcrSentence->InsertRectangle(iPosition, lNewRectangle);

                ProcessCharacterBitmap(poOcrSentence, iPosition);

                iPosition++;
                }
            //clean up
            delete oTempSentence;
            }

        //save the  image with split lines if asked for
        if (bSaveSplits)
            {
            long lStartX;

            //place best found seperation lines in the image
            for (long lIndex=0; lIndex < oBestRectangles->GetNumberChilds(); lIndex++) {

                oTempRectangle = oBestRectangles->GetChild(lIndex);

                lStartX = oTempRectangle->x - poOcrRectangle->x;

                for (long lY=0; lY < poOcrRectangle->height+1; lY++) {

                    if (hRectangleImage[lStartX] == 0xFF) {

                        hRectangleImage[lStartX] = 0xFD;
                        }

                    lStartX += (poOcrRectangle->width + 1);
                    }
                }

            //place best found seperation lines in the image
            for (unsigned long lIndex=0; lIndex < oSeperatorLines.size(); lIndex++) {

                lStartX = oSeperatorLines[lIndex];

                for (long lY=0; lY < poOcrRectangle->height+1; lY++) {

                    if (hRectangleImage[lStartX] == 0xFF) {

                        hRectangleImage[lStartX] = 0x02;
                        }

                    lStartX += (poOcrRectangle->width + 1);
                    }
                }

            //save image
            SaveRectangleImage(hRectangleImage, poOcrRectangle, lRectNumber, lRectW, lRectH);
            }

        //delete[] lProposedLines;
        delete oBestRectangles;
        free (hRectangleImage);
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::FindMaximaAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, sMinimaMaxima* poMM) {

    bool bAscending;
    long lColumn;
    long lCursor;
    long lRow;
    long lContinue;

    sMinimumMaximum cMM;

    lColumn = 0;
    lRow = lRectH-1;
    bAscending = false;

    lCursor = 0;
    lContinue = 0;

    //first find the starting point (the first non-white pixel in the first column from bottom to top)
    while (hTotalImage[lCursor] == 0xFF) {

        lCursor += lRectW;
        lRow--;
        }

    cMM.bMinimum = false;
    cMM.bMaximum = false;
    cMM.lPosition = lRow;
    poMM->oColumn.push_back(cMM);

    lColumn++;
    lCursor++;

    //do the math, walk trough the lines
    while (lColumn < lRectW) {

        if (hTotalImage[lCursor] == 0xFF) {

            //the pixel next to the previous one is white, meaning we have to
            //ascend in search of the next black pixel
            if (!bAscending) {

                //look if the pixel below it is black, if so we need to continue
                //descending, if not we found our maximum and can start descending
                if (lRow<lRectH-1) {

                    if (hTotalImage[lCursor-lRectW]==0x04) {

                        lRow++;
                        lCursor -= lRectW;

                        //continue descending to bottom
                        while (lRow < lRectH && hTotalImage[lCursor] == 0x04) {

                            lRow++;
                            lCursor -= lRectW;
                            }

                        //we found the next white pixel, continue searching for a minimum
                        cMM.bMinimum = false;
                        cMM.bMaximum = false;
                        cMM.lPosition = lRow;
                        poMM->oColumn.push_back(cMM);

                        lContinue = 0;
                        }
                    else {

                        //we can start to ascend, the previous pixel was a minimum
                        bAscending = true;

                        poMM->oColumn[lColumn-1 - (lContinue/2)].bMinimum = true;

                        cMM.bMinimum = false;
                        cMM.bMaximum = false;
                        cMM.lPosition = lRow;
                        poMM->oColumn.push_back(cMM);

                        lContinue = 0;
                        }
                    }
                else {

                    //exception, we are on the bottom row, and need to
                    //start ascending. Previous pixel was minimum
                    bAscending = true;

                    poMM->oColumn[lColumn-1 - (lContinue/2)].bMinimum = true;

                    /*cMM.bMinimum = false;
                    cMM.bMaximum = false;
                    cMM.lPosition = lRow;
                    poMM->oColumn.push_back(cMM);*/
                    }
                }

            if (bAscending) {

                while (lRow > 0 && hTotalImage[lCursor] == 0xFF) {

                    lRow--;
                    lCursor += lRectW;
                    }

                //we found the next black pixel, continue searching for a maximum
                cMM.bMinimum = false;
                cMM.bMaximum = false;
                cMM.lPosition = lRow;
                poMM->oColumn.push_back(cMM);

                lContinue=0;
                }
            }
        else {

            //the next pixel is black

            if (lRow<lRectH-1) {

                if (hTotalImage[lCursor-lRectW] == 0xFF) {

                    //still on bottom row, continue search to the right;
                    cMM.bMinimum = false;
                    cMM.bMaximum = false;
                    cMM.lPosition = lRow;
                    poMM->oColumn.push_back(cMM);

                    lContinue++;
                    }
                else {

                    //pixel to the bottom right is also black. Meaning we
                    //have found a maximum = the previous pixel! If we weren't
                    //ascending
                    if (bAscending) {
                        poMM->oColumn[lColumn-1].bMaximum = true;
                        }

                    //now continue the bottom!
                    bAscending = false;

                    while (lRow<lRectH-1 && hTotalImage[lCursor]==0x04) {

                        lCursor-=lRectW;
                        lRow++;
                        }

                    if (hTotalImage[lCursor]==0xFF) {

                        //we have gone one too far
                        lCursor+=lRectW;
                        lRow--;
                        }

                    cMM.bMinimum = false;
                    cMM.bMaximum = false;
                    cMM.lPosition = lRow;
                    poMM->oColumn.push_back(cMM);

                    lContinue=0;
                    }
                }
            else {

                //exception: we are on the bottom row, the next pixel is black
                //so we need to continue to the right.
                cMM.bMinimum = false;
                cMM.bMaximum = false;
                cMM.lPosition = lRow;
                poMM->oColumn.push_back(cMM);

                lContinue++;
                }
            }

        //move one position to the right
        lColumn++;
        lCursor++;
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::FindMimimaAtTop(BYTE* hTotalImage, long lRectW, long lRectH, sMinimaMaxima* poMM) {

    bool bAscending;
    long lColumn;
    long lCursor;
    long lRow;
    long lContinue;

    sMinimumMaximum cMM;


    lColumn = 0;
    lRow = 0;
    bAscending = false;

    lCursor = lRectW * (lRectH - 1);

    //first find the starting point (the first non-white pixel in the first column from top to bottom)
    while (hTotalImage[lCursor] == 0xFF) {

        lCursor -= lRectW;
        lRow++;
        }

    cMM.bMinimum = false;
    cMM.bMaximum = false;
    cMM.lPosition = lRow;
    poMM->oColumn.push_back(cMM);

    lColumn++;
    lCursor++;

    //do the math, walk trough the lines
    while (lColumn < lRectW) {


        if (hTotalImage[lCursor] == 0xFF) {

            //the pixel next to the previous one is white, meaning we have to
            //descend in search of the next black pixel
            if (bAscending) {

                //look if the pixel above it is black, if so we need to continue
                //ascending, if not we found our maximum and can start descending
                if (lRow>0) {

                    if (hTotalImage[lCursor+lRectW]==0x04) {

                        lRow--;
                        lCursor += lRectW;

                        //continue ascending to top
                        while (lRow > 0 && hTotalImage[lCursor] == 0x04) {

                            lRow--;
                            lCursor += lRectW;
                            }

                        //we found the next white pixel, continue searching for a minimum
                        cMM.bMinimum = false;
                        cMM.bMaximum = false;
                        cMM.lPosition = lRow;
                        poMM->oColumn.push_back(cMM);

                        lContinue=0;
                        }
                    else {

                        //we can start to descend, the previous pixel was a maximum
                        bAscending = false;

                        poMM->oColumn[lColumn-1 - (lContinue/2)].bMaximum = true;
                        }
                    }
                else {

                    //exception, we are on the top row, and need to
                    //start descending. Previous pixel was maximum
                    bAscending = false;

                    poMM->oColumn[lColumn-1 - (lContinue/2)].bMaximum = true;

                     }
                }

            if (!bAscending) {

                while (lRow < lRectH && hTotalImage[lCursor] == 0xFF) {

                    lRow++;
                    lCursor -= lRectW;
                    }

                //we found the next black pixel, continue searching for a minimum
                cMM.bMinimum = false;
                cMM.bMaximum = false;
                cMM.lPosition = lRow;
                poMM->oColumn.push_back(cMM);

                lContinue = 0;
                }
            }
        else {

            //the next pixel is black

            if (lRow>0) {

                if (hTotalImage[lCursor+lRectW] == 0xFF) {

                    //still on top row, continue search to the right;
                    cMM.bMinimum = false;
                    cMM.bMaximum = false;
                    cMM.lPosition = lRow;
                    poMM->oColumn.push_back(cMM);

                    lContinue++;
                    }
                else {

                    //pixel to the top right is also black. Meaning we
                    //have found a minimum = the previous pixel! If we weren't
                    //ascending
                    if (!bAscending) {
                        poMM->oColumn[lColumn-1].bMinimum = true;
                        }

                    //now continue the top!
                    bAscending = true;

                    while (lRow>0 && hTotalImage[lCursor]==0x04) {

                        lCursor+=lRectW;
                        lRow--;
                        }

                    if (hTotalImage[lCursor]==0xFF) {

                        //we have gone one too far
                        lCursor-=lRectW;
                        lRow++;
                        }

                    cMM.bMinimum = false;
                    cMM.bMaximum = false;
                    cMM.lPosition = lRow;
                    poMM->oColumn.push_back(cMM);

                    lContinue=0;
                    }
                }
            else {

                //exception: we are on the top row, the next pixel is black
                //so we need to continue to the right.
                cMM.bMinimum = false;
                cMM.bMaximum = false;
                cMM.lPosition = lRow;
                poMM->oColumn.push_back(cMM);

                lContinue++;
                }
            }

        //move one position to the right
        lColumn++;
        lCursor++;
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::PixelsOnHorizontalLine(BYTE* hTotalImage, long lStartX, long lWidth, long lRectW, long lRectH, long lLine) {

    long lTestPoint;
    long lReturnValue = 0;

    lTestPoint = (lRectH - lLine - 1) * lRectW;
    lTestPoint += lStartX;

    for (long lIndex=0; lIndex<lWidth; lIndex++) {

        if (hTotalImage[lTestPoint]!=0xFF) {
            lReturnValue++;
            }
        lTestPoint++;
        }


    return lReturnValue;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::PixelsOnVerticalLine(BYTE* hTotalImage, long lRectW, long lRectH, long lLine) {

    long lTestPoint;
    long lReturnValue = 0;

    lTestPoint = (lRectH - 1) * lRectW;
    lTestPoint += lLine;

    for (long lIndex=0; lIndex<lRectH; lIndex++) {

        if (hTotalImage[lTestPoint]!=0xFF) {
            lReturnValue++;
            }
        lTestPoint-=lRectW;
        }

    return lReturnValue;
    }
//---------------------------------------------------------------------------
bool
__fastcall TOcrRecogniseCharacter::LocalMinimumAtTop(BYTE* hTotalImage, long lRectW, long lRectH, long lLine) {

    //This function returns true if there is a local minimum at the top. There is a minimum when
    //the top profile is descending for two pixel columns and will be ascending for two more
    //pixel columns

    // x    xx            x    x
    // xx  xxx   <-yes   xx   xx  <-no
    // xx xxxx           xx  xxx
    // xxxxxxx           xxxxxxx

    bool bReturnValue;
    long lCurrentLine;
    long lPreviousCount;
    long lCurrentCount;
    long lStep;
    long lMinCount;
    long lMaxCount;

    bReturnValue = false;
    lCurrentLine = lLine - 2;
    lPreviousCount = -1;
    lStep = 0;
    lMinCount = lRectH;
    lMaxCount = 0;

    if (lCurrentLine>=0) {

        bReturnValue = true;

        while (bReturnValue && lStep < 5 && lCurrentLine < lRectW) {

            lCurrentCount = NumberWhitePixelAtTop(hTotalImage, lRectW, lRectH, lCurrentLine);

            if (lMinCount > lCurrentCount) {
                lMinCount = lCurrentCount;
                }
            if (lMaxCount < lCurrentCount) {
                lMaxCount = lCurrentCount;
                }

            if (lStep == 2) {

                if (lRectH != lCurrentCount + NumberWhitePixelAtBottom(hTotalImage, lRectW, lRectH, lCurrentLine) + PixelsOnVerticalLine(hTotalImage, lRectW, lRectH, lCurrentLine)) {

                    bReturnValue = false;
                    }
                }


            /*if (lCurrentCount == lPreviousCount) {

                bReturnValue = false;
                }     */

            //the last two steps should be ascending
            if (lPreviousCount < lCurrentCount && lStep > 2) {

                bReturnValue = false;
                }

            //the first two steps should be descending
            if (lPreviousCount > lCurrentCount && lStep <= 2) {

                bReturnValue = false;
                }

            lCurrentLine++;

            if (lCurrentLine == lRectW) {
                bReturnValue = false;
                }

            lPreviousCount = lCurrentCount;

            lStep++;
            }
        }

    if (lMinCount == lMaxCount) {

        bReturnValue = false;
        }

    return bReturnValue;
    }
//---------------------------------------------------------------------------
bool
__fastcall TOcrRecogniseCharacter::LocalMaximumAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, long lLine) {

    //This function returns true if there is a local maximum at the bottom. There is a maximum when
    //the top profile is ascending for two pixel columns and will be descending for two more
    //pixel columns

    // xxxxxxx           xxxxxxx
    // xx xxxx           xx  xxx
    // xx  xxx   <-yes   xx   xx  <-no
    // x    xx            x    x

    bool bReturnValue;
    long lCurrentLine;
    long lPreviousCount;
    long lCurrentCount;
    long lStep;
    long lMinCount;
    long lMaxCount;

    bReturnValue = false;
    lCurrentLine = lLine - 2;
    lPreviousCount = -1;
    lStep = 0;
    lMinCount = lRectH;
    lMaxCount = 0;

    if (lCurrentLine>=0) {

        bReturnValue = true;

        while (bReturnValue && lStep < 5 && lCurrentLine < lRectW) {

            lCurrentCount = NumberWhitePixelAtBottom(hTotalImage, lRectW, lRectH, lCurrentLine);

            if (lMinCount > lCurrentCount) {
                lMinCount = lCurrentCount;
                }
            if (lMaxCount < lCurrentCount) {
                lMaxCount = lCurrentCount;
                }


            if (lStep == 2) {

                if (lRectH != lCurrentCount + NumberWhitePixelAtTop(hTotalImage, lRectW, lRectH, lCurrentLine) + PixelsOnVerticalLine(hTotalImage, lRectW, lRectH, lCurrentLine)) {

                    bReturnValue = false;
                    }
                }
            //the last two steps should be ascending
            /*if (lPreviousCount == lCurrentCount) {

                bReturnValue = false;
                } */

            if (lPreviousCount < lCurrentCount && lStep > 2) {

                bReturnValue = false;
                }

            //the first two steps should be descending
            if (lPreviousCount > lCurrentCount && lStep <= 2) {

                bReturnValue = false;
                }

            lCurrentLine++;

            if (lCurrentLine == lRectW) {
                bReturnValue = false;
                }

            lPreviousCount = lCurrentCount;

            lStep++;
            }
        }

    if (lMinCount == lMaxCount) {

        bReturnValue = false;
        }

    return bReturnValue;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::NumberWhitePixelAtTop(BYTE* hTotalImage, long lRectW, long lRectH, long lLine) {

    //This function counts the number of white pixels from the top of the given
    //column(line) in the given image
    long lPointer;
    long lCounter;

    lPointer = lLine + (lRectW * (lRectH-1));
    lCounter = 0;

    while (hTotalImage[lPointer]==0xFF && lPointer>0) {

        lCounter++;
        lPointer-=lRectW;
        }

    return lCounter;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::NumberWhitePixelAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, long lLine) {

    //This function counts the number of white pixels from the top of the given
    //column(line) in the given image
    long lPointer;
    long lCounter;

    lPointer = lLine;
    lCounter = 0;

    while (hTotalImage[lPointer]==0xFF && lPointer < lRectW*lRectH) {

        lCounter++;
        lPointer+=lRectW;
        } 

    return lCounter;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::MaxConnectedPixelsOnVerticalLine(BYTE* hTotalImage,
                                                                    long lRectW,
                                                                    long lRectH,
                                                                    long lLine) {
    long lTestPoint;
    long lSequence = 0;
    long lReturnValue = 0;

    lTestPoint = (lRectH - 1) * lRectW;
    lTestPoint += lLine;

    for (long lIndex=0; lIndex<lRectH; lIndex++) {

        if (hTotalImage[lTestPoint]!=0xFF) {
            lSequence++;
            if (lSequence>lReturnValue) {
                lReturnValue = lSequence;
                }
            }
        else {
            lSequence=0;
            }

        lTestPoint-=lRectW;
        }

    return lReturnValue;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::ReconsiderMethod1(TOcrSentence* poOcrSentence) {

    TOcrCollection* oLines[5];
    cOcrCharacterShape oShape;

    int iStartIndex, iNumberRectangles, iRectNumber;
    TOcrRectangle* oRectangle;

    bool bTopFound;
    bool bBottomFound;

    for (long lIndex=0; lIndex<5; lIndex++) {
        oLines[lIndex] = new TOcrCollection();
        }

    iStartIndex=0;
    iNumberRectangles = poOcrSentence->NumberOfRectangles();




    while (iStartIndex<iNumberRectangles) {

        iRectNumber = poOcrSentence->GetRectangle(iStartIndex);

        if (iRectNumber != -1) {

            oRectangle = oOcrRectangles->GetChild(iRectNumber);

            if (oRectangle->oCharacterSuggestions.size()>0) {

                oShape = oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[0].lShapeIndex];

                //find the top of the character
                bTopFound = false;

                if (oShape.bTopLine) {
                    oLines[0]->AddItem(oRectangle->y);
                    bTopFound = true;
                    }

                if (!bTopFound && oShape.bMiddleLine) {
                    oLines[1]->AddItem(oRectangle->y);
                    bTopFound = true;
                    }

                if (!bTopFound && oShape.bCenterLine) {
                    oLines[2]->AddItem(oRectangle->y);
                    bTopFound = true;
                    }

                if (!bTopFound && oShape.bBaseLine) {
                    oLines[3]->AddItem(oRectangle->y);
                    bTopFound = true;
                    }

                //find the bottom of the character
                bBottomFound = false;

                if (oShape.bBottomLine) {
                    oLines[4]->AddItem(oRectangle->y + oRectangle->height);
                    bBottomFound = true;
                    }

                if (!bBottomFound && oShape.bBaseLine) {
                    oLines[3]->AddItem(oRectangle->y + oRectangle->height);
                    bBottomFound = true;
                    }

                if (!bBottomFound && oShape.bCenterLine) {
                    oLines[2]->AddItem(oRectangle->y + oRectangle->height);
                    bBottomFound = true;
                    }

                if (!bBottomFound && oShape.bMiddleLine) {
                    oLines[1]->AddItem(oRectangle->y + oRectangle->height);
                    bBottomFound = true;
                    }
                }
            }

        iStartIndex++;
        }


    //get the two highest (most reliable) measurements
    long lLines[5];
    long lMax1, lMax2;
    long lDeviation1, lDeviation2;
    long lIndexMax1, lIndexMax2, lSwapIndex;

    lMax1 = lMax2 = 0;
    lIndexMax1 = lIndexMax2 = 0;
    lDeviation1 = lDeviation2 = 1000;
    bool bProcessed;

    for (long lIndex = 0; lIndex<5; lIndex++) {

        lLines[lIndex] = oLines[lIndex]->lNumberItems();


        if (lLines[lIndex]>0) {

            bProcessed = false;

            if (lLines[lIndex] > lMax1) {

                lMax2 = lMax1;
                lIndexMax2 = lIndexMax1;
                lDeviation2 = lDeviation1;

                lMax1 = lLines[lIndex];
                lIndexMax1 = lIndex;
                lDeviation1 = oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median());

                bProcessed = true;
                }

            if (!bProcessed && lLines[lIndex] >= lMax1 && lDeviation1 > oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median())) {

                lMax2 = lMax1;
                lIndexMax2 = lIndexMax1;
                lDeviation2 = lDeviation1;

                lMax1 = lLines[lIndex];
                lIndexMax1 = lIndex;
                lDeviation1 = oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median());

                bProcessed = true;
                }

            if (!bProcessed && lLines[lIndex] >= lMax2 && lLines[lIndex] <= lMax1) {

                if (lLines[lIndex] > lMax2) {
                    lMax2 = lLines[lIndex];
                    lIndexMax2 = lIndex;
                    lDeviation2 = oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median());
                    }
                if (lDeviation2 > oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median())) {
                    lMax2 = lLines[lIndex];
                    lIndexMax2 = lIndex;
                    lDeviation2 = oLines[lIndex]->lMeanDeviation(oLines[lIndex]->Median());
                    }
                }
            }
        }

    if (lIndexMax1 > lIndexMax2) {

        lSwapIndex = lIndexMax1;
        lIndexMax1 = lIndexMax2;
        lIndexMax2 = lSwapIndex;
        }



    long lRatios[5];

    lRatios[0] = 0;
    lRatios[1] = 65;
    lRatios[2] = 190;
    lRatios[3] = 240;
    lRatios[4] = 290;


    float fIncrement;
    float  lStartConstant;

    fIncrement = ((float)oLines[lIndexMax2]->Median() - (float)oLines[lIndexMax1]->Median()) / ((float)lRatios[lIndexMax2] - (float)lRatios[lIndexMax1]);
    lStartConstant = (float)oLines[lIndexMax1]->Median() - fIncrement * (float)lRatios[lIndexMax1];


    for (long lIndex=0; lIndex<5; lIndex++) {

        lLines[lIndex] = (long)(lStartConstant + fIncrement * (lRatios[lIndex]));
        }

    ReconsiderRecognition(poOcrSentence, lLines[0], lLines[1], lLines[2], lLines[3], lLines[4]);


    for (long lIndex=0; lIndex<5; lIndex++) {

        delete oLines[lIndex];
        }
    }

//---------------------------------------------------------------------------

void
__fastcall TOcrRecogniseCharacter::ReconsiderMethod2(TOcrSentence* poOcrSentence) {


    TOcrCollection* oHeights;
    TOcrCollection* oTops;
    TOcrCollection* oBottoms;

    int iStartIndex, iNumberRectangles, iRectNumber;
    TOcrRectangle* oRectangle;

    oHeights = new TOcrCollection();
    oTops = new TOcrCollection();
    oBottoms = new TOcrCollection();

    //second process step: determine position different lines
    iStartIndex=0;
    iNumberRectangles = poOcrSentence->NumberOfRectangles();

    while (iStartIndex<iNumberRectangles) {

        iRectNumber = poOcrSentence->GetRectangle(iStartIndex);

        if (iRectNumber != -1) {

            oRectangle = oOcrRectangles->GetChild(iRectNumber);

            oTops->AddItem(oRectangle->y);
            oHeights->AddItem(oRectangle->height);
            oBottoms->AddItem(oRectangle->y + oRectangle->height);
            }

        iStartIndex++;
        }

    //determine top rows.
    oTops->SortGroupsBySize();

    long lTopLine = (oTops->GetGroup(oTops->NumberGroups()-1).lStartLong + oTops->GetGroup(oTops->NumberGroups()-1).lEndLong) / 2;

    if (oTops->GetGroup(oTops->NumberGroups()-1).lSize < poOcrSentence->NumberOfRectangles() / 4) {
        lTopLine = -1;
        }

    long lMiddleLine = -1;
    if (oTops->NumberGroups() > 1) {
        lMiddleLine = (oTops->GetGroup(oTops->NumberGroups()-2).lStartLong + oTops->GetGroup(oTops->NumberGroups()-2).lEndLong) / 2;
        if (oTops->GetGroup(oTops->NumberGroups()-2).lSize < poOcrSentence->NumberOfRectangles() / 4) {
            lMiddleLine = -1;
            }
        }

    if (lTopLine>lMiddleLine) {
        long lTemp;

        lTemp = lMiddleLine;
        lMiddleLine = lTopLine;
        lTopLine = lTemp;
        }


    //determine bottom rows.
    oBottoms->SortGroupsBySize();

    long lBaseLine = (oBottoms->GetGroup(oBottoms->NumberGroups()-1).lStartLong + oBottoms->GetGroup(oBottoms->NumberGroups()-1).lEndLong) / 2;

    if (oBottoms->GetGroup(oBottoms->NumberGroups()-1).lSize < poOcrSentence->NumberOfRectangles() / 4) {
        lBaseLine = 100000;
        }

    long lBottomLine = 100000;
    if (oBottoms->NumberGroups() > 1) {
        lBottomLine = (oBottoms->GetGroup(oBottoms->NumberGroups()-2).lStartLong + oBottoms->GetGroup(oBottoms->NumberGroups()-2).lEndLong) / 2;
        if (oBottoms->GetGroup(oBottoms->NumberGroups()-2).lSize < poOcrSentence->NumberOfRectangles() / 4) {
            lBottomLine = 100000;
            }
        }

    if (lBaseLine>lBottomLine) {
        long lTemp;

        lTemp = lBottomLine;
        lBaseLine = lBottomLine;
        lBottomLine = lTemp;
        }


    long lTempTopLine, lDefTopLine;
    long lTempMiddleLine, lDefMiddleLine;
    long lTempBaseLine, lDefBaseLine;
    long lTempBottomLine, lDefBottomLine;






    if (lBottomLine == 100000 || lTopLine == -1) {

        //we are going to determine what the combination of top/middle and base/bottom
        //line is with the lowest score. Every rectangle has a list of suggested character,
        //and we want to know what the best combination is.
        long lMinScore = 100000;
        long lScore;

        for (int i=0; i<4; i++) {

            lTempTopLine = lTopLine;
            lTempMiddleLine = lMiddleLine;
            lTempBaseLine = lBaseLine;
            lTempBottomLine = lBottomLine;

            if (lTempBottomLine==100000) {

                if (i & 1) {
                    lTempBottomLine = (long)(lTempBaseLine + (oHeights->Median() * 0.25));
                    }
                else {
                    lTempBottomLine = lTempBaseLine;
                    lTempBaseLine = lTempBottomLine - (oHeights->Median() * 0.25);
                    }
                }

            if (lTempTopLine==-1) {

                if (i & 2) {
                    lTempTopLine = lTempMiddleLine - (oHeights->Median() * 0.25);
                    }
                else {
                    lTempTopLine = lTempMiddleLine;
                    lTempMiddleLine = lTempTopLine +  (oHeights->Median() * 0.25);
                    }
                }

            lScore = ReconsiderRecognition(poOcrSentence, lTempTopLine, lTempMiddleLine, (lTempMiddleLine + lTempBaseLine)/2, lTempBaseLine, lTempBottomLine);

            //MessageBox(NULL, poOcrSentence->GetContent(oOcrRectangles).c_str(), "Test", MB_OK);

            if (lScore < lMinScore) {

                lDefTopLine = lTempTopLine;
                lDefMiddleLine = lTempMiddleLine;
                lDefBaseLine = lTempBaseLine;
                lDefBottomLine = lTempBottomLine;

                lMinScore = lScore;
                }
            }
        }
    else {

        lDefTopLine = lTopLine;
        lDefMiddleLine = lMiddleLine;
        lDefBaseLine = lBaseLine;
        lDefBottomLine = lBottomLine;

        }


    ReconsiderRecognition(poOcrSentence, lDefTopLine, lDefMiddleLine, (lDefBaseLine + lDefMiddleLine)/2, lDefBaseLine, lDefBottomLine);

    delete oTops;
    delete oHeights;
    delete oBottoms; 
    }

//---------------------------------------------------------------------------

long
__fastcall TOcrRecogniseCharacter::ReconsiderRecognition(TOcrSentence* poOcrSentence, long lTopLine,
                                                         long lMiddleLine, long lCenterLine, long lBaseLine,
                                                         long lBottomLine) {

    //long lCenterLine = (lBaseLine + lMiddleLine ) / 2;
    long lTopMiddle = (lTopLine + lMiddleLine) / 2;
    long lMiddleCenter = (lMiddleLine + lCenterLine) / 2;
    long lCenterBase = (lCenterLine + lBaseLine) / 2;
    long lBaseBottom = (lBaseLine + lBottomLine) / 2;
    bool bFound;
    unsigned long iCounter;
    bool bTop, bMiddle, bBase, bCenter, bBottom;

    int iStartIndex, iNumberRectangles, iRectNumber;
    long lRetValue;

    TOcrRectangle* oRectangle;

    //reconsider recognition characters
    iStartIndex = 0;
    iNumberRectangles = poOcrSentence->NumberOfRectangles();
    lRetValue = 0;

    while (iStartIndex<iNumberRectangles) {

        iRectNumber = poOcrSentence->GetRectangle(iStartIndex);

        if (iRectNumber != -1) {

            oRectangle = oOcrRectangles->GetChild(iRectNumber);
            bFound = false;
            iCounter = 0;

            if (oRectangle->oCharacterSuggestions.size()>0) {

                oRectangle->sContent = oRectangle->oCharacterSuggestions[0].cCharacter;

                bTop = bMiddle = bBase = bBottom = bCenter = false;

                //top sides
                if (oRectangle->y <= lTopMiddle) {
                    bTop = true;
                    }
                if (oRectangle->y > lTopMiddle && oRectangle->y <= lMiddleCenter) {
                    bMiddle = true;
                    }

                if (oRectangle->y > lMiddleCenter && oRectangle->y <= lCenterBase) {
                    bCenter = true;
                    }

                if (oRectangle->y > lCenterBase && oRectangle->y <= lBaseBottom) {
                    bBase = true;
                    }


                //bottom sides
                if ((oRectangle->y + oRectangle->height) > lTopMiddle && (oRectangle->y + oRectangle->height) <= lMiddleCenter) {
                    bMiddle = true;
                    }

                if ((oRectangle->y + oRectangle->height) > lMiddleCenter && (oRectangle->y + oRectangle->height) <= lCenterBase) {
                    bCenter = true;
                    }

                if ((oRectangle->y + oRectangle->height) > lCenterBase && (oRectangle->y + oRectangle->height) <= lBaseBottom) {
                    bBase = true;
                    }

                if ((oRectangle->y + oRectangle->height) > lBaseBottom) {
                    bBottom = true;
                    }

                while (/*oRectangle->oCharacterSuggestions[iCounter].lScore<=100 && */iCounter<oRectangle->oCharacterSuggestions.size() && !bFound) {

                    //We take the top most suggested shape that falls in the same lines (top, base, middle)
                    //as the to be recognised shape.

                    //if (oRectangle->iGaps == oOcrCharacterShapes->oShapes[oOcrRectangle->oCharacterSuggestions[iCounter].lShapeIndex].lHoles) {

                    if (oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].bTopLine == bTop &&
                        oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].bMiddleLine == bMiddle &&
                        oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].bCenterLine == bCenter &&
                        oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].bBaseLine == bBase &&
                        oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].bBottomLine == bBottom ) {

                        if (oRectangle->iGaps == oOcrCharacterShapes->oShapes[oRectangle->oCharacterSuggestions[iCounter].lShapeIndex].lHoles) {
                            oRectangle->sContent = oRectangle->oCharacterSuggestions[iCounter].cCharacter;
                            bFound = true;
                            lRetValue += oRectangle->oCharacterSuggestions[iCounter].lScore;
                            }
                        }

                    //We take the top most suggested shape that complies with the number of gaps/holes
                    //in the to be recognised shape. For example, a D has 1 gap and a S 0. When the
                    //to be recognised shape has a gap, but the top spot of the suggested doesn't
                    //then we can be positive that the suggestion is false. Therefor we take the next
                    //one.
                    /*if (oRectangle->iGaps == oOcrCharacterShapes->oShapes[oOcrRectangle->oCharacterSuggestions[iCounter].lShapeIndex].lHoles) {

                        oRectangle->sContent = oRectangle->oCharacterSuggestions[iCounter].cCharacter;
                        bFound = true;
                        }   */

                    iCounter++;
                    }

                if (!bFound) {
                    oRectangle->sContent = L"?";
                    lRetValue+= 100;
                    }
                }
            else {
                //no suggestions... what to do??? ***
                oRectangle->sContent = L"?";
                lRetValue+= 100;
                }
            }

        iStartIndex++;
        }

    return lRetValue;
    }
//---------------------------------------------------------------------------

void
__fastcall TOcrRecogniseCharacter::ProcessCharacterShape(TOcrSentence* poOcrSentence, int iIndex) {

    TOcrRectangle* oOcrRectangle;
    BYTE* lTempSpace;
    HGLOBAL hTempSpace;
    int RectX, RectY, RectW, RectH;
    long lSourcePointer, lDestinationPointer;
    int nGaps;
           int iRectNumber;

    //Get the rectangle from the collection of rectangles
    oOcrRectangle = oOcrRectangles->GetChild(poOcrSentence->GetRectangle(iIndex));


    if (oOcrRectangle!=NULL) {

        //first we make a work image of the character. That workimage is surrounded
        //by a rectangle of white pixels. The extra space is needed to be able to
        //create a border of vectors around it.

        oOcrRectangle->eType = eCharacter;

        RectX = oOcrRectangle->x;
        RectY = oOcrRectangle->y;
        RectW = oOcrRectangle->width+1+2;
        RectH = oOcrRectangle->height+1+2;

        //0. create temporary workspace
        hTempSpace = GlobalAlloc(GMEM_MOVEABLE, RectW * RectH);
        if (hTempSpace!=NULL) {

            lTempSpace = (BYTE*)GlobalLock(hTempSpace);
            FillMemory(lTempSpace, RectW*RectH, 255);



            //1. copy rectangle into temporary workspace
            for (int y=0; y<RectH-2; y++) {

                lSourcePointer = RectX + (iImageHeight - RectY - y  - 1) * iRowLength;
                lDestinationPointer = (RectH - y - 2) * RectW + 1;

                CopyMemory(lTempSpace + lDestinationPointer, lpBits + lSourcePointer, RectW - 2);

                }


           //this is a temporary thing
            oOcrRectangle->width+=2;
            oOcrRectangle->height+=2;

            //2. Isolate connected component
            //2.1 change black pixels of connected components into colorcode 254
            for (unsigned long lIndex=0; lIndex < oOcrRectangle->oConnectedComponentPoints.size(); lIndex++) {
                GrowConnectedComponent(lTempSpace, oOcrRectangle,
                                            oOcrRectangle->oConnectedComponentPoints[lIndex].x + 1,
                                            oOcrRectangle->oConnectedComponentPoints[lIndex].y + 1, 0, 254,8);
                }
            iRectNumber = poOcrSentence->GetRectangle(iIndex);

            //if (bSaveImage) SaveRectangleImage(lTempSpace, oOcrRectangle, iRectNumber, RectW, RectH);

            //2.2 remove all other black pixels: they belong to other objects
            for (int y=0; y<RectH; y++) {

                lDestinationPointer = (RectH - y - 1) * RectW;

                for (int x=0; x<RectW; x++) {

                    if (lTempSpace[lDestinationPointer]==0) {
                        lTempSpace[lDestinationPointer] = 0xFF;
                        }

                    lDestinationPointer++;
                    }
                }



            //3. find white space around component (if present)
            //3.1 move through outer rectangle, and paint all white space
            //    into other color (253)
            //
                //right
               lDestinationPointer = (RectH-1)*RectW + RectW - 1;

                for (int y=0; y<RectH; y++) {

                    if (lTempSpace[lDestinationPointer]==0xFF) {
                        GrowConnectedComponent(lTempSpace, oOcrRectangle, RectW-1, y, 255, 253,8);
                        }
                    lDestinationPointer -= RectW;
                    }

                //top
                lDestinationPointer = (RectH - 1) * RectW;

                for (int x=0; x<RectW; x++) {

                    if (lTempSpace[lDestinationPointer]==0xFF) {
                        GrowConnectedComponent(lTempSpace, oOcrRectangle, x, 0, 255, 253,8);
                        }
                    lDestinationPointer++;
                    }

                //bottom
                lDestinationPointer = 0;

                for (int x=0; x<RectW; x++) {

                    if (lTempSpace[lDestinationPointer]==0xFF) {
                        GrowConnectedComponent(lTempSpace, oOcrRectangle, x, RectH-1, 255, 253,8);
                        }
                    lDestinationPointer++;
                    }

                //left
               lDestinationPointer = (RectH-1)*RectW ;

                for (int y=0; y<RectH; y++) {

                    if (lTempSpace[lDestinationPointer]==0xFF) {
                        GrowConnectedComponent(lTempSpace, oOcrRectangle, 0, y, 255, 253,8);
                        }
                    lDestinationPointer -= RectW;
                    }






            //4. search for white connected components, they are inner gaps
            //   and count them and give color 252. more then 2 might be an
            //   indicator for an image.
            nGaps = 0;

            for (int y=0; y<RectH; y++) {

               lDestinationPointer = (RectH - y - 1) * RectW;
                for (int x=0; x<RectW; x++) {

                    if (lTempSpace[lDestinationPointer]==255) {
                        if (GrowConnectedComponent(lTempSpace, oOcrRectangle, x, y, 255, 252, 3)) {

                            nGaps++;

                            }
                        }
                    lDestinationPointer++;
                    }
                }

            oOcrRectangle->iGaps = nGaps;
            if (nGaps>2) {

                oOcrRectangle->eType = eImageRect;

                };

            //5 find outline
            /*bool bOutlineFound;
            BYTE bPixel;
            int  iFirstX, iFirstY;

            iFirstX = iFirstY = -1;
            oPointStack.clear();

            for (int y=0; y<RectH; y++) {

               lDestinationPointer = (RectH - y - 1) * RectW;
                for (int x=0; x<RectW; x++) {

                    if (lTempSpace[lDestinationPointer]==253) {

                        bOutlineFound;

                        bPixel = GetPixel(lTempSpace, oOcrRectangle, x+1, y, 253);
                        bOutlineFound = (bPixel!=253 && bPixel!=251);

                        if (!bOutlineFound) {

                            bPixel = GetPixel(lTempSpace, oOcrRectangle, x, y+1, 253);
                            bOutlineFound = (bPixel!=253 && bPixel!=251);

                            if (!bOutlineFound) {
                                bPixel = GetPixel(lTempSpace, oOcrRectangle, x-1, y, 253);
                                bOutlineFound = (bPixel!=253 && bPixel!=251);

                                if (!bOutlineFound) {

                                    bPixel = GetPixel(lTempSpace, oOcrRectangle, x, y-1, 253);
                                    bOutlineFound = (bPixel!=253 && bPixel!=251);
                                    }
                                }
                            }

                        if (bOutlineFound) {
                            lTempSpace[lDestinationPointer]=251;

                            if (iFirstX==-1) {
                                iFirstX = x;
                                iFirstY = y;
                                }
                            }
                        }
                    lDestinationPointer++;
                    }
                }*/



            iRectNumber = poOcrSentence->GetRectangle(iIndex);

            //Create shape from image
            //CreateShape(lTempSpace, oOcrRectangle);

            //Match known shapes with shape in oOcrRectangle
            //MatchShapes(oOcrRectangle, iRectNumber);

            //Save for debug the rectangle
            if (bSaveImage || bSaveResults)
                {
                SaveRectangleImage(lTempSpace, oOcrRectangle, iRectNumber, RectW, RectH);

                }

            //free allocated memory
            GlobalUnlock(hTempSpace);
            GlobalFree(hTempSpace);

            oOcrRectangle->width-=2;
            oOcrRectangle->height-=2;
            }
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::RecogniseCharacterBitmap(BYTE* pbCharacterSpace, int piShapeIndex, double pdRatio, long &lSpecialPoints, long &lBonus) {

    //this function calculate the matching score of the given charachtershape and
    //the comparison shape given by shapeindex. The score is calculated on pixeldistances,
    //special points (some points define a character in contrast to similar characters),
    //ratio

    long lScore;
    long lDifference;
    long lDeltaRatio;
    long lComparePointer;

    BYTE bPoint;
    BYTE bCompare;

    BYTE* bOriginalGrid;

    lSpecialPoints = 0;
    lBonus = 0;
    lScore = 0;

    bOriginalGrid = oOcrCharacterShapes->oShapes[piShapeIndex].bCompareGrid;

    for (int y=0; y<32; y++) {

        for (int x=0; x<32; x++) {

            lComparePointer = x + y*32;

            bPoint = pbCharacterSpace[lComparePointer];

            bCompare = bOriginalGrid[lComparePointer];

            if (bCompare>4) {
                lSpecialPoints++;
                if (bCompare==5) {

                    bCompare = 4;
                    if (bPoint == 4) {

                        lBonus++;
                        }
                    }

                if (bCompare==6) {

                    bCompare = 0;
                    if (bPoint == 0) {

                        lBonus++;
                        }
                    }
                }

            lDifference = abs(bPoint - bCompare);

            if (lDifference == 4) {

                lScore += lDifference;
                }
            }
        }

     if (lBonus >=  lSpecialPoints * 0.9) {

        lScore = lScore / 2;
        }

     lDeltaRatio = abs((pdRatio - oOcrCharacterShapes->oShapes[piShapeIndex].lRatio)/2);

     lScore = lScore * (1 + lDeltaRatio);

     lScore += lDeltaRatio * 10;

     return lScore;
     }
//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::AddScoreToSuggestions(TOcrRectangle* poRectangle, int piShapeIndex, long plScore) {

    //add the calculated score to the suggestions of the given rectangle
    bool bSuggestionSaved;
    TOcrCharacterSuggestion cCharacterSuggestion;

    bSuggestionSaved = false;

    cCharacterSuggestion.cCharacter = oOcrCharacterShapes->oShapes[piShapeIndex].cCharacter[0];
    cCharacterSuggestion.lShapeIndex = piShapeIndex;
    cCharacterSuggestion.lScore = plScore;

    for (unsigned int j=0; j<poRectangle->oCharacterSuggestions.size() && !bSuggestionSaved; j++) {

        //Place the lMatchIndication in the vector oOcrRectangle->oCharacterSuggestions,
        //so that oOcrRectangle->oCharacterSuggestions is order by ascending lMatchIndication number
        //The lower lMatchIndication is, the better the shapes matched. In the end the top
        //ranked shape will be the suggested character.
        if (plScore < poRectangle->oCharacterSuggestions[j].lScore) {

            poRectangle->oCharacterSuggestions.insert(poRectangle->oCharacterSuggestions.begin() + j, cCharacterSuggestion);

            bSuggestionSaved = true;
            }
        }

    if (!bSuggestionSaved) {

        //Not a suggested spot found, place it at the end of the
        //vector oOcrRectangle->oCharacterSuggestions.
        poRectangle->oCharacterSuggestions.push_back(cCharacterSuggestion);
        }
    }
//---------------------------------------------------------------------------

BYTE
__fastcall TOcrRecogniseCharacter::GetPixel(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int x, int y, BYTE bDefault) {

    BYTE bRetValue;
    long lPointer;

    bRetValue = bDefault;

    if (x<=(oOcrRectangle->width) && y<=(oOcrRectangle->height) && x>=0 && y>=0) {

        lPointer = ((oOcrRectangle->height+1 - y - 1) * (oOcrRectangle->width+1)) + x;

        bRetValue = lTempSpace[lPointer];
        }

    return bRetValue;
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrRecogniseCharacter::SetPixel(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int x, int y, BYTE bColor) {

    long lPointer;

    lPointer = ((oOcrRectangle->height+1 - y - 1) * (oOcrRectangle->width+1)) + x;

    lTempSpace[lPointer] = bColor;
    }
//---------------------------------------------------------------------------
bool
__fastcall TOcrRecogniseCharacter::GrowConnectedComponent(BYTE* lpTempSpace, TOcrRectangle* poOcrRectangle, int iStartX, int iStartY, BYTE iFindColor, BYTE iReplaceColor, int iPixels) {

    int iNumberPixels = 0;
    long lPointer;
    bool bError = false;

    long x,y,w,h;

    vector<long> xCache;
    vector<long> yCache;

    //initialise variables
    unsigned long iIndex = 0;

    x = iStartX;
    y = iStartY;
    w = poOcrRectangle->width+1;
    h = poOcrRectangle->height+1;

    //place first detected pixel in vector
    xCache.push_back(x);
    yCache.push_back(y);

    lPointer = x + (h - y -1) * w;
    lpTempSpace[lPointer]=iReplaceColor;

    do {
        //get pixel from vector (if there are any)
        x = xCache[iIndex];
        y = yCache[iIndex];

        //calculate pointer and change color of pixel to make sure
        //it will not be detected for other connected components
        lPointer = x + (h - y -1) * w;
        iNumberPixels++;

        //look around and add interesting pixels to vector
        if (!bError) bError = GCC_CheckNeighbour(lpTempSpace, x, -1, w, y,  0, h, xCache, yCache, iFindColor, iReplaceColor);
        if (!bError) bError = GCC_CheckNeighbour(lpTempSpace, x,  0, w, y, -1, h, xCache, yCache, iFindColor, iReplaceColor);
        if (!bError) bError = GCC_CheckNeighbour(lpTempSpace, x,  1, w, y,  0, h, xCache, yCache, iFindColor, iReplaceColor);
        if (!bError) bError = GCC_CheckNeighbour(lpTempSpace, x,  0, w, y,  1, h, xCache, yCache, iFindColor, iReplaceColor);

        //update iIndex;
        iIndex++;
        } while (iIndex<xCache.size() && !bError);

    //clear the vectors
    xCache.clear();
    yCache.clear();

    //return true if there are more than 8 connected pixels
    return (iNumberPixels >= iPixels && !bError);
    }

//---------------------------------------------------------------------------
bool
__fastcall TOcrRecogniseCharacter::GCC_CheckNeighbour(BYTE* lpTempSpace, long x, int dx, long w, long y, int dy, long h, vector<long>& pxCache, vector<long>& pyCache, BYTE iFindColor, BYTE iReplaceColor) {

    bool bError = false;
    long lPointer;

    lPointer = x + (h - y - 1) * w;

    //check if pixel is withing boundaries
    if (0<=x+dx && x+dx<w && 0<=y+dy && y+dy<h) {

            //check color code of pixel
            if (lpTempSpace[lPointer + dx - (dy * w)]== iFindColor) {

                //add pixel to cache
                if (pxCache.max_size()>pxCache.size() && pyCache.max_size()>pyCache.size()) {
                    pxCache.push_back(x+dx);
                    pyCache.push_back(y+dy);
                    lpTempSpace[lPointer + dx - (dy * w)]=iReplaceColor;
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
TOcrRecogniseCharacter::SaveRectangleImage32(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int RectNumber) {

    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;
    string Filename;
    long size;
    FILE* hFile;
    BYTE bPallette [1024];
    BYTE* lWriteSpace;
    HANDLE hWriteSpace;
	std::wostringstream oFilenameStream;

    oFilenameStream << "c:\\TOCRImages\\";

    switch (oOcrRectangle->eType) {

        case eImageRect: {
            oFilenameStream <<  "Images\\";
            break;
            }
        case eCharacter: {
            oFilenameStream <<  "Characters\\";
            oFilenameStream <<  oOcrRectangle->iGaps;
            oFilenameStream <<  "\\";

            break;
            }
        case eInvertedCharacter: {
            oFilenameStream <<  "InvertedCharacters\\";
            break;
            }

        case eSplitCharacter: {
            oFilenameStream <<  "Splits\\";
            break;
            }
        }

	oFilenameStream << RectNumber;

	std::wstring sFilename(oFilenameStream.str());

    sFilename += L"_32.bmp";


    int  iImageRowLength = (((32*8+31)&(~31))/8);

    if (bSaveImage) {

        _wfopen_s(&hFile, sFilename.c_str(), L"wb");

        if (hFile!=NULL) {

            size = iImageRowLength * 32 + 256*4 + sizeof(BITMAPINFOHEADER);

            bmf.bfType = 'MB';
            bmf.bfSize = sizeof(bmf) + size;
            bmf.bfReserved1 = 0;
            bmf.bfReserved2 = 0;
            bmf.bfOffBits = sizeof(bmi) + 1024 + sizeof(bmf);

            if (fwrite(&bmf, 1, sizeof(bmf), hFile) == sizeof(bmf)) {

                ZeroMemory(&bmi, sizeof(bmi));

                bmi.biSize = sizeof(bmi);
                bmi.biWidth = 32;
                bmi.biHeight = 32;
                bmi.biPlanes = 1;
                bmi.biBitCount = 8;
                bmi.biClrUsed = 256;
                bmi.biClrImportant = 0;
                bmi.biCompression = BI_RGB;

                if (fwrite(&bmi, 1, sizeof(bmi), hFile) == sizeof(bmi)) {

                    for (int i=0; i<256; i++) {
                        bPallette[i*4] = i;
                        bPallette[i*4+1] = i;
                        bPallette[i*4+2] = i;
                        }

                    bPallette[0*4] = 255;
                    bPallette[0*4+1] = 255;
                    bPallette[0*4+2] = 255;

                    bPallette[1*4] = 192;
                    bPallette[1*4+1] = 192;
                    bPallette[1*4+2] = 192;

                    bPallette[2*4] = 128;
                    bPallette[2*4+1] = 128;
                    bPallette[2*4+2] = 128;

                    bPallette[3*4] = 64;
                    bPallette[3*4+1] = 64;
                    bPallette[3*4+2] = 64;

                    bPallette[4*4] = 0;
                    bPallette[4*4+1] = 0;
                    bPallette[4*4+2] = 0;



                    bPallette[254*4] = 128;
                    bPallette[254*4+1] = 0;
                    bPallette[254*4+2] = 0;
                    bPallette[253*4] = 128;
                    bPallette[253*4+1] = 255;
                    bPallette[253*4+2] = 128;
                    bPallette[250*4] = 128;
                    bPallette[250*4+1] = 0;
                    bPallette[250*4+2] = 0;
                    bPallette[249*4] = 255;
                    bPallette[249*4+1] = 255;
                    bPallette[249*4+2] = 255;
                    bPallette[248*4] = 255;
                    bPallette[248*4+1] = 255;
                    bPallette[248*4+2] = 0;

                    if (fwrite(&bPallette, 1, 1024, hFile)==1024) {

                        hWriteSpace = GlobalAlloc(GMEM_MOVEABLE, iImageRowLength * 32);

                        if (hWriteSpace!=NULL) {

                            lWriteSpace = (BYTE*)GlobalLock(hWriteSpace);

                            for (int y=0; y<32; y++) {

                                memcpy(lWriteSpace + y*iImageRowLength, lTempSpace + 1024 - y*32 - 32, 32);

                                }

                            fwrite(lWriteSpace, 1, 32 * iImageRowLength, hFile);

                            GlobalUnlock(hWriteSpace);
                            GlobalFree(hWriteSpace);
                            }
                        }
                    }
                }

            fclose(hFile);
            }
        }
    }
//---------------------------------------------------------------------------
void 
TOcrRecogniseCharacter::SaveRecognitionInformation(cRecognition &poRecognition, TOcrRectangle* oOcrRectangle, int RectNumber) {

	std::wostringstream oFilenameStream;
	std::ofstream* out;
	long lIndex;
    char cBuffer[100];

    oFilenameStream << "c:\\TOCRImages\\";

    switch (oOcrRectangle->eType) {

        case eImageRect: {
            oFilenameStream <<  "Images\\";
            break;
            }
        case eCharacter: {
            oFilenameStream <<  "Characters\\";
            oFilenameStream <<  oOcrRectangle->iGaps;
            oFilenameStream <<  "\\";

            break;
            }
        case eInvertedCharacter: {
            oFilenameStream <<  "InvertedCharacters\\";
            break;
            }

        case eSplitCharacter: {
            oFilenameStream <<  "Splits\\";
            break;
            }
        }

	oFilenameStream << RectNumber;
	oFilenameStream << L"_score.txt";

	std::wstring sFilename(oFilenameStream.str());

	out = new ofstream(sFilename.c_str(), ios_base::out);

	if (out!=NULL) {

	    for (unsigned int i=0; i<poRecognition.oScores.size(); i++) {

			sprintf_s(cBuffer, sizeof(cBuffer), "%s : %f\r\n", poRecognition.oScores[i].sTitle.c_str(), poRecognition.oScores[i].fScore);
			
			out->write(cBuffer, strlen(cBuffer));
		}

		out->close();
		}
	}

//---------------------------------------------------------------------------
void
TOcrRecogniseCharacter::SaveRectangleImage(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int RectNumber, long RectW, long RectH) {

    BITMAPFILEHEADER bmf;
    BITMAPINFOHEADER bmi;
	std::wstring Filename;
    long size;
    FILE* hFile;
    BYTE bPallette [1024];
    BYTE* lWriteSpace;
    HANDLE hWriteSpace;
	std::wostringstream oFilenameStream;


    oFilenameStream << "c:\\TOCRImages\\";

    switch (oOcrRectangle->eType) {

        case eImageRect: {
            oFilenameStream <<  "Images\\";
            break;
            }
        case eCharacter: {
            oFilenameStream <<  "Characters\\";
            oFilenameStream <<  oOcrRectangle->iGaps;
            oFilenameStream <<  "\\";

            break;
            }
        case eInvertedCharacter: {
            oFilenameStream <<  "InvertedCharacters\\";
            break;
            }

        case eSplitCharacter: {
            oFilenameStream <<  "Splits\\";
            break;
            }
        }

	std:wstring FileNameVector;
	ofstream* out;
    char cBuffer[100];
    char cNumber[10];

    oFilenameStream <<  RectNumber;
    oFilenameStream <<  ".bmp";
			
	std::wstring sFilename(oFilenameStream.str());

    int  iImageRowLength = (((RectW*8+31)&(~31))/8);

    if (bSaveImage || bSaveSplits) {

        errno_t iResult = _wfopen_s(&hFile, sFilename.c_str(), L"wb");

		iResult = 0;

		if (hFile!=NULL) {

            size = iImageRowLength * RectH + 256*4 + sizeof(BITMAPINFOHEADER);

            bmf.bfType = 'MB';
            bmf.bfSize = sizeof(bmf) + size;
            bmf.bfReserved1 = 0;
            bmf.bfReserved2 = 0;
            bmf.bfOffBits = sizeof(bmi) + 1024 + sizeof(bmf);

            if (fwrite(&bmf, 1, sizeof(bmf), hFile) == sizeof(bmf)) {

                ZeroMemory(&bmi, sizeof(bmi));

                bmi.biSize = sizeof(bmi);
                bmi.biWidth = RectW;
                bmi.biHeight = RectH;
                bmi.biPlanes = 1;
                bmi.biBitCount = 8;
                bmi.biClrUsed = 256;
                bmi.biClrImportant = 0;
                bmi.biCompression = BI_RGB;

                if (fwrite(&bmi, 1, sizeof(bmi), hFile) == sizeof(bmi)) {

                    for (int i=0; i<256; i++) {
                        bPallette[i*4] = i;
                        bPallette[i*4+1] = i;
                        bPallette[i*4+2] = i;
                        }

                    bPallette[0*4] = 255;
                    bPallette[0*4+1] = 255;
                    bPallette[0*4+2] = 255;

                    bPallette[1*4] = 192;
                    bPallette[1*4+1] = 192;
                    bPallette[1*4+2] = 192;

                    bPallette[2*4] = 128;
                    bPallette[2*4+1] = 128;
                    bPallette[2*4+2] = 128;

                    bPallette[3*4] = 64;
                    bPallette[3*4+1] = 64;
                    bPallette[3*4+2] = 64;

                    bPallette[4*4] = 0;
                    bPallette[4*4+1] = 0;
                    bPallette[4*4+2] = 0;



                    bPallette[254*4] = 128;
                    bPallette[254*4+1] = 0;
                    bPallette[254*4+2] = 0;
                    bPallette[253*4] = 128;
                    bPallette[253*4+1] = 255;
                    bPallette[253*4+2] = 128;
                    bPallette[250*4] = 128;
                    bPallette[250*4+1] = 0;
                    bPallette[250*4+2] = 0;
                    bPallette[249*4] = 255;
                    bPallette[249*4+1] = 255;
                    bPallette[249*4+2] = 255;
                    bPallette[248*4] = 255;
                    bPallette[248*4+1] = 255;
                    bPallette[248*4+2] = 0;

                    if (fwrite(&bPallette, 1, 1024, hFile)==1024) {

                        hWriteSpace = GlobalAlloc(GMEM_MOVEABLE, iImageRowLength * RectH);

                        if (hWriteSpace!=NULL) {

                            lWriteSpace = (BYTE*)GlobalLock(hWriteSpace);

                            for (int y=0; y<RectH; y++) {

                                memcpy(lWriteSpace + y*iImageRowLength, lTempSpace + y*RectW, RectW);

                                }

                            fwrite(lWriteSpace, 1, RectH * iImageRowLength, hFile);

                            GlobalUnlock(hWriteSpace);
                            GlobalFree(hWriteSpace);
                            }
                        }
                    }
                }

            fclose(hFile);
            }
        }
    }

//-----------------------------------------------------------------------------
void __fastcall TOcrRecogniseCharacter::AddDistanceToCompareGrid(BYTE* bCompareGrid) {

    long lHighestNumber;

    for (int lDistance = 3; lDistance>1; lDistance--) {

        for (int y=0; y<32; y++) {

            for (int x=0; x<32; x++) {

                if (bCompareGrid[x+y*32]==0) {

                    lHighestNumber = GetHighestNeighbour(bCompareGrid, x, y);

                    if (lHighestNumber==lDistance+1) {

                        bCompareGrid[x+y*32] = lDistance;
                        }
                    }
                }
            }
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::GetHighestNeighbour(BYTE* bCompareGrid, int x, int y) {

    long lReturnValue = 0;
    long lNeighbour = 0;

    lNeighbour = GetNeighbour(bCompareGrid, x-1, y);
    if (lNeighbour>lReturnValue) lReturnValue = lNeighbour;
    lNeighbour = GetNeighbour(bCompareGrid, x+1, y);
    if (lNeighbour>lReturnValue) lReturnValue = lNeighbour;
    lNeighbour = GetNeighbour(bCompareGrid, x, y-1);
    if (lNeighbour>lReturnValue) lReturnValue = lNeighbour;
    lNeighbour = GetNeighbour(bCompareGrid, x, y+1);
    if (lNeighbour>lReturnValue) lReturnValue = lNeighbour;

    return lReturnValue;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrRecogniseCharacter::GetNeighbour(BYTE* bCompareGrid, int x, int y) {

    if (x<0) return 0;
    if (x>31) return 0;
    if (y<0) return 0;
    if (y>31) return 0;

    return bCompareGrid[x+y*32];
    }
//---------------------------------------------------------------------------




