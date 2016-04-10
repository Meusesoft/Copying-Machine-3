//---------------------------------------------------------------------------

#ifndef TErrorH
#define TErrorH

#include "TOcrResources.h"

#define ReportError(lErrorCode, lErrorLevel, sArgument) oError->CreateError(TExceptionContainer(lErrorCode, lErrorLevel, __WFUNCTION__, __LINE__, sArgument), true)
#define ThrowError(lErrorCode, lErrorLevel, sArgument) throw TExceptionContainer(TExceptionContainer(lErrorCode, lErrorLevel, __WFUNCTION__, __LINE__, sArgument))
#define MakeSure(bTest, lErrorCode, lErrorLevel, sTest) oError->TestAndThrow(bTest, TExceptionContainer(lErrorCode, lErrorLevel, __WFUNCTION__, __LINE__, sTest))
#define ReportAndThrow(lErrorCode, lErrorLevel, sArgument) oError->CreateAndThrow(TExceptionContainer(lErrorCode, lErrorLevel, __WFUNCTION__, __LINE__, sArgument));

#define LEVEL_WARNING 0
#define LEVEL_RECOVERABLE 10
#define LEVEL_CRITICAL 20




class TExceptionContainer : public exception {

    public:
        TExceptionContainer(long plErrorCode, long plErrorLevel,
                            wstring psFunction, long plLine, wstring psArgument);

//        ~TExceptionContainer();

        long lErrorCode;
        long lErrorLevel;
        wstring sFunction;
        wstring sArgument;
        long lLine;
    };
//---------------------------------------------------------------------------


class TError {

    public:

        TError();
        ~TError();

        static TError* CreateInstance();
        static TError* GetInstance();
        static void DeleteInstance();

        void TestAndThrow(bool bTest, TExceptionContainer T);
        void CreateAndThrow(TExceptionContainer T);
        void CreateError(TExceptionContainer T, bool bReport = false);


        long lArchive;
        long lFolder;
        long lDocument;
        long lItem;
        long lMetaData;
        long lMetaDataField;
        long lMeta;
        long lMetaItem;

        bool bDebug;

        HINSTANCE hResource;
        HWND      hMainWindow;
        HWND      hDialog;

        //static TError* m_pInstance;
        //static int  m_pRefCount;
    };

//---------------------------------------------------------------------------
#endif
 