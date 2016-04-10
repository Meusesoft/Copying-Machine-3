//---------------------------------------------------------------------------

#ifndef TOcrFindSentencesH
#define TOcrFindSentencesH
//---------------------------------------------------------------------------
#include "TOcrRectangle.h"
#include "TOcrSentences.h"
#include "TOcrArea.h"
//-------------------------------------------------------------------------
class TOcrFindSentences: public TOcrThread
{
	public:
		TOcrFindSentences(HGLOBAL, TOcrRectangle*, TOcrSentences*, bool CreateSuspended);

		void __fastcall Execute();

		long lTimeDrawSentences;
		long lTimeCalculateDistances;
		long lTimeCreateAreas;
		long lTimeFindSentences;
		long lTimeAddSpaces;
		long lTimeCleanUpSentences;

		long nSentences;

	private:

		HGLOBAL hImageHandle;
		TOcrRectangle* oRectangles;
		TOcrSentences* oSentences;

		long iRowLength;
		long iImageHeight;
		long iImageWidth;
		long lNumberAreasX, lNumberAreasY;

		int  iBitsPerPixel;
		BYTE* lpBits;
		vector<TOcrArea*> oAreas;
		TError* oError;

		void __fastcall FindSentences();
		void __fastcall GrowSentence(TOcrSentence* poSentence);
		void __fastcall AddSpacesToSentences();
		void __fastcall CleanUpSentences();
		void __fastcall CreateAreas();
		void __fastcall ClearAreas();
		void __fastcall MergeRectangles(TOcrSentence* poInnerSentence, TOcrSentence* poOuterSentence);
		bool __fastcall SentenceWithinSentence(TOcrSentence* poSentenceOuter, TOcrSentence* poSentenceInner, long plPixelMargin);
		int __fastcall  FindClosestRectangle(TOcrSentence* poSentence, long iRectangle, float fAngle, bool bLeft);
		void __fastcall CalculateBoundingBox();

		//debugging/trace functions
		void __fastcall DrawSentences();
		void __fastcall DrawLine(TOcrPoint pcFromPoint, TOcrPoint pcToPoint, int pnSpaces);
		void __fastcall ExportAreas();

};
//---------------------------------------------------------------------------
#endif
