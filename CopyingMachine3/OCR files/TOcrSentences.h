//---------------------------------------------------------------------------

#ifndef TOcrSentencesH
#define TOcrSentencesH
#include "TOcrRectangle.h"
//---------------------------------------------------------------------------
class TOcrSentence {

    public:
        TOcrSentence();
        ~TOcrSentence();

        void __fastcall AddRectangle(long plRectangle);
        void __fastcall DeleteRectangle(long plRectangle);
        void __fastcall UpdateRectangle(long plPosition, long plNewRectangle);
        void __fastcall InsertRectangle(long plPosition, long plRectangle);
        long __fastcall GetRectangle(long plPosition);
        long __fastcall NumberOfRectangles();
        long __fastcall AverageHeight(TOcrRectangle* poRectangles);
        long __fastcall AverageWidth(TOcrRectangle* poRectangles);
        double __fastcall AverageAngle(TOcrRectangle* poRectangles);
        wstring __fastcall GetContent(TOcrRectangle* poRectangles);

        int iSentenceNumber;

        int x;
        int y;
        int width;
        int height;

    private:

        TError* oError;

        vector<long> lRectangles;

        int lAverageHeight;
        int lAverageWidth;


        double dAverageAngle;
    };
//---------------------------------------------------------------------------
class TOcrSentences {

    public:
        TOcrSentences();
        ~TOcrSentences();

        vector<TOcrSentence*> oSentences;
    };
//---------------------------------------------------------------------------
#endif
