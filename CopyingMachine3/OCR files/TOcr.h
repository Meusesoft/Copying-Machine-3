//---------------------------------------------------------------------------

#ifndef TOcrH
#define TOcrH
//---------------------------------------------------------------------------

#include "TOcrRectangle.h"
#include "TOcrSentences.h"
#include "TOcrThread.h"
#include "TDataLayer.h"
#include "../Logic files/Bitmap.h"

struct cOcrSettings {

	bool bSaveXML;
	bool bSaveSplits;
	bool bSaveResults;
	bool bSaveImage;
	int iCompareMethod;
	HGLOBAL hIntermediateImage;
	std::wstring sTrace;
	};

struct cRealCharacter {

    long lRectNumber;
    char cChar[2];
    long lSuccess;
    };

class TOcr {

    public:
        TOcr(sGlobalInstances poGlobalInstances);
        ~TOcr();

		void SetImage(HGLOBAL phImageHandle);
		void SetImage(CBitmap* poBitmap, RectF pcRectangle);

		bool DoExecute();
        bool Execute();
        wstring GetResultFile();
		wstring GetTraceResult();

        //following is only for debug purposes
        void SetTestOutput(Graphics* poTestOutput);

        long GetRectangleAtPosition(long pX, long pY);
        HWND hIndicators;
        HWND hText;
		HWND hParentWnd;
        TOcrRectangle* oRectangles;
        TOcrSentences* oSentences;

        bool bSaveXML;
        bool bSaveImage;
        bool bSaveResults;
        bool bSaveSplits;
        int  iCompareMethod;  // 0=All, 1=Groups, 2=Tree

         HGLOBAL hIntermediateHandle;

	private:

        TError* oError;
		TDataLayer *oDataLayer;

        void OutputImage(HGLOBAL phImageHandle);
        void ProcessEvents();

        HGLOBAL hImageHandle;
        Graphics* oTestOutput;
		std::wstring sTrace;

        vector<cRealCharacter> oRealCharacters;
        void __fastcall LoadRealCharacters();
        void __fastcall SaveRealCharacters();
		 
		sGlobalInstances oGlobalInstances;

    };

#endif
