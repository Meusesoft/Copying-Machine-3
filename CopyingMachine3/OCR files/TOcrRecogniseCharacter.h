//---------------------------------------------------------------------------

#ifndef TOcrRecogniseCharacterH
#define TOcrRecogniseCharacterH
//---------------------------------------------------------------------------
#include "TOcrRectangle.h"
#include "TOcrSentences.h"
#include "TOcrCharacterShapes.h"
#include "TOcrDistancePoints.h"
#include "TOcrCollection.h"
#include "TNeuralNetwork.h"
#include "TDataLayer.h"
//---------------------------------------------------------------------------
struct sMinimumMaximum {

    bool bMinimum;
    bool bMaximum;

    long lPosition;
    };

struct sMinimaMaxima {

    vector<sMinimumMaximum> oColumn;

    };
//---------------------------------------------------------------------------

class TOcrRecogniseCharacter : public TOcrThread
{
     public:
        TOcrRecogniseCharacter(HGLOBAL, TOcrRectangle*, TOcrSentences*, TOcrCharacterShapes*, TDataLayer*, bool CreateSuspended);

        void __fastcall Execute();

		bool bSaveImage;
        bool bSaveResults;
        bool bSaveXML;
        bool bSaveSplits;

        long lTimerLoadXML;

    protected:

        void __fastcall ProcessSentence(TOcrSentence*);
        void __fastcall ProcessWord(TOcrSentence*, int iStartIndex, int iEndIndex);
        void __fastcall ProcessCharacterShape(TOcrSentence*, int iIndex);
        void __fastcall ProcessCharacterBitmap(TOcrSentence*, int iIndex);
        long __fastcall RecogniseCharacterBitmap(BYTE* pbCharacterSpace, int iShapeIndex, double pdRatio, long &lSpecialPoints, long &lBonus);
        void __fastcall AddScoreToSuggestions(TOcrRectangle* poRectangle, int piShapeIndex, long plScore);

        void __fastcall ReconsiderMethod1(TOcrSentence* poOcrSentence);
        void __fastcall ReconsiderMethod2(TOcrSentence* poOcrSentence);
        long __fastcall ReconsiderRecognition(TOcrSentence*, long lTopLine, long lMiddleLine, long lCenterLine, long lBaseLine, long lBottomLine);

        void __fastcall SplitCharacter(TOcrRectangle* poOcrRectangle, TOcrSentence* poOcrSentence, int iPosition);
        long __fastcall PixelsOnHorizontalLine(BYTE* hTotalImage, long lStartX, long lWidth, long lRectW, long lRectH, long lLine);
        long __fastcall PixelsOnVerticalLine(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        long __fastcall MaxConnectedPixelsOnVerticalLine(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        bool __fastcall LocalMinimumAtTop(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        bool __fastcall LocalMaximumAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        long __fastcall NumberWhitePixelAtTop(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        long __fastcall NumberWhitePixelAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, long lLine);
        void __fastcall FindMaximaAtBottom(BYTE* hTotalImage, long lRectW, long lRectH, sMinimaMaxima* poMM);
        void __fastcall FindMimimaAtTop(BYTE* hTotalImage, long lRectW, long lRectH, sMinimaMaxima* poMM);





        bool __fastcall GrowConnectedComponent(BYTE*, TOcrRectangle*, int, int, BYTE, BYTE, int iPixels);
        bool __fastcall GCC_CheckNeighbour(BYTE*, long x, int dx, long w, long y, int dy, long h, vector<long>& pxCache, vector<long>& pyCache, BYTE iFindColor, BYTE iReplaceColor);
        long __fastcall GetHighestNeighbour(BYTE* bCompareGrid, int x, int y);
        long __fastcall GetNeighbour(BYTE* bCompareGrid, int x, int y);


        void SaveRectangleImage(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int RectNumber, long RectW, long RectH);
        void SaveRectangleImage32(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int RectNumber);
		void SaveRecognitionInformation(cRecognition &poRecognition, TOcrRectangle* oOcrRectangle, int RectNumber);

        BYTE __fastcall GetPixel(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int x, int y, BYTE bDefault=0);
        void __fastcall SetPixel(BYTE* lTempSpace, TOcrRectangle* oOcrRectangle, int x, int y, BYTE bColor);
        void __fastcall AddDistanceToCompareGrid(BYTE* bCompareGrid);

	private:

        HGLOBAL hImageHandle;
        TOcrRectangle* oOcrRectangles;
        TOcrSentences* oOcrSentences;
		TDataLayer* oDataLayer;

        vector <TOcrPoint> oPointStack;

        TOcrCharacterShapes* oOcrCharacterShapes;

        long iRowLength;
        long iImageHeight;
        long iImageWidth;
        BYTE* lpBits;

        TOcrPoint oStartPoint;

        TError* oError;

};
//---------------------------------------------------------------------------
#endif
