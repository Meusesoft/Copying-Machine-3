//---------------------------------------------------------------------------

#ifndef TOcrAnalyseRectanglesH
#define TOcrAnalyseRectanglesH
//---------------------------------------------------------------------------
#include "TOcrRectangle.h"
//---------------------------------------------------------------------------
class TOcrAnalyseRectangles : public TOcrThread
{            
public:
    TOcrAnalyseRectangles(HGLOBAL, TOcrRectangle*, bool CreateSuspended);

	void __fastcall Execute();
private:

    HGLOBAL hImageHandle;
    TOcrRectangle* oRectangles;
    TError* oError;

    long iRowLength;
    long iImageHeight;
    long iImageWidth;
    BYTE* lpBits;

    double __fastcall CalculateRowVariance(TOcrRectangle* poRectangle);
    double __fastcall CalculateColumnVariance(TOcrRectangle* poRectangle);
    
};
//---------------------------------------------------------------------------
#endif
