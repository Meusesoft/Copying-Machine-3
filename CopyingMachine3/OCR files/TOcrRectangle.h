//---------------------------------------------------------------------------

#ifndef TOcrRectangleH
#define TOcrRectangleH

//#include <vector.h>
#include "TError.h"
//---------------------------------------------------------------------------
struct TOcrPoint {

    long x;
    long y;
                                               /*
    bool operator ==(const TOcrPoint pcPoint) {}

    bool operator !=(const TOcrPoint pcPoint);   */

    };

struct TOcrCharacterSuggestion {

    char cCharacter;
    wstring sCharacter;
    long lScore;
    long lShapeIndex;
    };

enum modes { LASTMODE = -1, BW40=0, C40, BW80, C80, MONO = 7 };

enum eRectangleType {eUnknownRect = -1, eCharacter = 0, eImageRect = 1, eInvertedCharacter = 2, eSplitCharacter = 3};

class TOcrRectangle {

    public:
        TOcrRectangle();
        ~TOcrRectangle();

        bool AddChild(TOcrRectangle* poChild);
        bool DeleteChild(int iIndex, bool bDeleteInstance = true);
        int  GetNumberChilds();
        TOcrRectangle* GetChild(int iIndex);

        bool CoordinateInMe(long x, long y);
        bool RectangleOverlap(TOcrRectangle*);
        TOcrPoint GetCenterPoint();
        vector<TOcrPoint> oVectorPoints;
        vector<TOcrCharacterSuggestion> oCharacterSuggestions;

        long DistanceChildren(int iIndex1, int iIndex2);
        long DistanceBetweenChildren(int iIndex1, int iIndex2);
        double AverageAreaChildren();
        double __fastcall AngleBetweenChildren(int iIndex1, int iIndex2);

        long __fastcall MergeRectangles(long lIndex1, long lIndex2);

        long x,y,width,height,numberpixels;

        int  iSentenceNumber;
        int  iAreaNumber;
        int  iMergeWithRectangle;
        int  iGaps;
        bool bTouchingCharacters;

        float fAnglePrevious, fAngleNext;
        vector<TOcrPoint> oConnectedComponentPoints;

        eRectangleType eType;
        wstring sContent;

        HGLOBAL hBitmap;

    private:

        vector<TOcrRectangle*> oChilds;
        TError* oError;

    };

#endif
