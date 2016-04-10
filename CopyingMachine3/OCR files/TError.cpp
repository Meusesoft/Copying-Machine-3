//---------------------------------------------------------------------------
#include "StdAfx.h"
#include "terror.h"

static TError* m_pInstance  = NULL;
static int m_pRefCount = 0;

//---------------------------------------------------------------------------
TExceptionContainer::TExceptionContainer(long plErrorCode, long plErrorLevel,
                                         wstring psFunction, long plLine,
                                         wstring psArgument) : exception() {

    lErrorLevel = plErrorLevel;
    lErrorCode = plErrorCode;
    lLine = plLine;
    sFunction = psFunction;
    sArgument = psArgument;
    }
//---------------------------------------------------------------------------
/*TExceptionContainer::~TExceptionContainer() {

    }  */
//---------------------------------------------------------------------------
TError::TError() {

    bDebug = false;

    hResource = NULL;
    hDialog = NULL;

    m_pInstance = NULL;
    m_pRefCount = 0;
    }
//---------------------------------------------------------------------------
TError::~TError() {

    wstring ErrorMessage;

    }
//---------------------------------------------------------------------------
TError*
TError::CreateInstance() {

    if( !m_pInstance )
        m_pInstance = new TError();

    m_pRefCount++;
    return m_pInstance;
    }

//---------------------------------------------------------------------------
TError*
TError::GetInstance() {

    return m_pInstance;
    }

//---------------------------------------------------------------------------
void
TError::DeleteInstance() {

    if (m_pRefCount>0) {

        m_pRefCount--;

        if(m_pRefCount==0) {

            delete m_pInstance;
            m_pInstance = NULL;
            }
        }
    }

//---------------------------------------------------------------------------
void
TError::TestAndThrow(bool bTest, TExceptionContainer T) {

    if (!bTest) {

        CreateError(T, false);

        throw T;
        }
    }
//---------------------------------------------------------------------------
void
TError::CreateAndThrow(TExceptionContainer T) {

    CreateError(T, false);

    throw T;
    }
//---------------------------------------------------------------------------
void
TError::CreateError(TExceptionContainer T, bool bReport) {

    wstring sError;
    wstring sErrorMessage;
    wstring sErrorCaption;
    wstring sResource;
    unsigned int iFlags;
    LPVOID lpMsgBuf;

    //get the texts for the body and the caption of the error dialog/log
    if (hResource!=NULL) {
        //sError = sErrorMessage.LoadStringA(hResource, T.lErrorCode);

        switch (T.lErrorLevel) {

            case LEVEL_WARNING: {
                //sErrorCaption = sErrorMessage.LoadStringA(hResource, TXT_WARNING);
                iFlags = MB_OK | MB_ICONWARNING;
                break;
                }

            case LEVEL_RECOVERABLE: {
               // sErrorCaption = sErrorMessage.LoadStringA(hResource, TXT_RECOVERABLE);
                iFlags = MB_OK | MB_ICONWARNING;
                break;
                }

            case LEVEL_CRITICAL: {
               // sErrorCaption = sErrorMessage.LoadStringA(hResource, TXT_CRITICALERROR);
                iFlags = MB_OK | MB_ICONSTOP;
                break;
                }
            }
        }
    else {

        sError = L"An exceptional error occurred. %s";

        switch (T.lErrorLevel) {

            case LEVEL_WARNING: {
                sErrorCaption = L"Warning";
                iFlags = MB_OK | MB_ICONWARNING;
                break;
                }

            case LEVEL_RECOVERABLE: {
                sErrorCaption = L"Error";
                iFlags = MB_OK | MB_ICONWARNING;
                break;
                }

            case LEVEL_CRITICAL: {
                sErrorCaption = L"Critical Error";
                iFlags = MB_OK | MB_ICONSTOP;
                break;
                }
            }
        }

    //get the last error from the filesystem
    if (T.lErrorCode == ERR_FILESYSTEMERROR) {

        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            _wtoi(T.sArgument.c_str()),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL
            );

        sError = (wchar_t*)lpMsgBuf;

        // Free the buffer.
        LocalFree( lpMsgBuf );
        }

    //show error to user
	wchar_t cErrorMessage[1024];

	swprintf_s(cErrorMessage, sizeof(cErrorMessage), sError.c_str(), T.sArgument.c_str());
    sErrorMessage = cErrorMessage; 

    if (bDebug) {
        sErrorMessage += L"\nfunction '";
		sErrorMessage += T.sFunction;
		sErrorMessage += L"' \nline ";
		sErrorMessage += T.lLine;
        }

    if (bDebug || bReport) {

        ::MessageBox(hDialog!=NULL ? hDialog : hMainWindow, sErrorMessage.c_str(), sErrorCaption.c_str(), iFlags);
        }

    //log error
    if (bDebug) {
        sError += L"\nfunction '";
		sError += T.sFunction;
		sError += L"' \nline ";
		sError += T.lLine;
        }

    //Try to close application: critical error occurred
    if (T.lErrorLevel==LEVEL_CRITICAL && hMainWindow!=NULL) {

        //::PostMessage(hMainWindow, WM_CLOSE, 0, 0);
        }
    }

//---------------------------------------------------------------------------


