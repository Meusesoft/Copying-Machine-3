//---------------------------------------------------------------------------
#include "stdafx.h"
#include "TOcrSentences.h"
//---------------------------------------------------------------------------

TOcrSentence::TOcrSentence() {

    oError = TError::GetInstance();

    //nothing more to initialise
    }

TOcrSentence::~TOcrSentence() {

    //clear the vector
    lRectangles.clear();
    }

//---------------------------------------------------------------------------
void
__fastcall TOcrSentence::AddRectangle(long plRectangle) {

    lAverageHeight = lAverageWidth = -1;
	dAverageAngle = -1;

    try {

        MakeSure(lRectangles.size() < lRectangles.max_size(), ERR_OUTOFMEMORY, LEVEL_CRITICAL, L"");

        lRectangles.push_back(plRectangle);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrSentence::UpdateRectangle(long plPosition, long plNewRectangle) {

    lAverageHeight = lAverageWidth = -1;
	dAverageAngle = -1;

    try {

        MakeSure(plPosition < (long)lRectangles.size() , ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"plPosition");

        lRectangles[plPosition] = plNewRectangle;
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrSentence::DeleteRectangle(long plRectangle) {

    lAverageHeight = lAverageWidth = -1;
	dAverageAngle = -1;

    try {

        MakeSure(plRectangle < (long)lRectangles.size() , ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"plPosition");

        lRectangles.erase(lRectangles.begin()+plRectangle);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }
//---------------------------------------------------------------------------
void
__fastcall TOcrSentence::InsertRectangle(long plPosition, long plRectangle) {

    lAverageHeight = lAverageWidth = -1;
	dAverageAngle = -1;

    try {

        MakeSure(plPosition < (long)lRectangles.size() , ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"plPosition");

        lRectangles.insert(lRectangles.begin()+plPosition, plRectangle);
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrSentence::NumberOfRectangles() {

    return lRectangles.size();
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrSentence::GetRectangle(long plPosition) {

    long lReturnValue;

    try {

        MakeSure(plPosition < (long)lRectangles.size() , ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"plPosition");

        lReturnValue = lRectangles[plPosition];
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }

    return lReturnValue;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrSentence::AverageHeight(TOcrRectangle* poRectangles) {

    TOcrRectangle* oTempRectangle;
    long lIndex;
    long lNumberRectangles;

    if (lAverageHeight == -1) {

        try {

            MakeSure(poRectangles!=NULL, ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"poRectangles");

            lAverageHeight = 0;
            lNumberRectangles = 0;

            for (unsigned int i=0; i<lRectangles.size(); i++) {
                lIndex = lRectangles[i];

                if (lIndex!=-1) {
                    oTempRectangle = poRectangles->GetChild(lIndex);
                    lAverageHeight += oTempRectangle->height;
                    lNumberRectangles++;
                    }
                }

            if (lNumberRectangles>0) {
                lAverageHeight = lAverageHeight / lNumberRectangles;
                }
            }
        catch (const TExceptionContainer& T) {

            ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
            }
        catch (...) {

            ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
            }
        }

    return lAverageHeight;
    }
//---------------------------------------------------------------------------
long
__fastcall TOcrSentence::AverageWidth(TOcrRectangle* poRectangles) {

    TOcrRectangle* oTempRectangle;
    long lIndex;
    long lNumberRectangles;


    if (lAverageWidth == -1) {

        try {

            MakeSure(poRectangles!=NULL, ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"poRectangles");

            lAverageWidth = 0;
            lNumberRectangles = 0;

            for (unsigned int i=0; i<lRectangles.size(); i++) {
                lIndex = lRectangles[i];

                if (lIndex!=-1) {
                    oTempRectangle = poRectangles->GetChild(lIndex);
                    lAverageWidth += oTempRectangle->width;
                    lNumberRectangles++;
                    }
                }

            if (lNumberRectangles>0) {

                lAverageWidth = lAverageWidth / lNumberRectangles;
                }
            }
        catch (const TExceptionContainer& T) {

            ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
            }
        catch (...) {

            ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
            }
        }

    return lAverageWidth;
    }
//---------------------------------------------------------------------------
double
__fastcall TOcrSentence::AverageAngle(TOcrRectangle* poRectangles) {

    long lPointerFrom, lPointerTo, lNumberRectangles;

    lNumberRectangles = 0;

    if (dAverageAngle == -1) {

        try {

            MakeSure(poRectangles!=NULL, ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"poRectangles");

            dAverageAngle = 0;

            lPointerFrom = 0;
            lPointerTo = 0;


            while (lRectangles[lPointerTo]==-1 && lPointerTo<(long)lRectangles.size()) {
                lPointerTo++;
                }

            while (lPointerTo<(long)lRectangles.size()) {

                while (lRectangles[lPointerTo]==-1 && lPointerTo<(long)lRectangles.size()) {
                    lPointerTo++;
                    }

                if (lPointerTo<(long)lRectangles.size()) {
                    dAverageAngle += poRectangles->AngleBetweenChildren(lPointerFrom, lPointerTo);
                    lNumberRectangles++;
                    }

                lPointerFrom = lPointerTo;
                lPointerTo++;
                }

            if (lNumberRectangles>0) {
                dAverageAngle = dAverageAngle / lNumberRectangles;
                }
            }
        catch (const TExceptionContainer& T) {

            ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
            }
        catch (...) {

            ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
            }
       }

    return dAverageAngle;
    }

//---------------------------------------------------------------------------

wstring
__fastcall TOcrSentence::GetContent(TOcrRectangle* poRectangles) {

    wstring cRetValue;
    TOcrRectangle* oWordCharacter;
    
    cRetValue = L"";

    try {

        MakeSure(poRectangles!=NULL, ERR_INVALIDPOINTER, LEVEL_RECOVERABLE, L"poRectangles");

        for (long lRectangleIndex=0; lRectangleIndex<NumberOfRectangles(); lRectangleIndex++) {

            if (GetRectangle(lRectangleIndex) == -1) {
                //space
                cRetValue += L" ";
                }
            else {
                //character
                oWordCharacter = poRectangles->GetChild(GetRectangle(lRectangleIndex));
                cRetValue += oWordCharacter->sContent;
                }
            }
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return cRetValue;
    }

//---------------------------------------------------------------------------
TOcrSentences::TOcrSentences() {

    //nothing to initialise
    }

TOcrSentences::~TOcrSentences() {

    //delete all sentences object and clear the vector
    for (unsigned int i=0; i<oSentences.size(); i++) {
        delete oSentences[i];
        }

    oSentences.clear();
    }

//---------------------------------------------------------------------------

