//---------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include "TError.h"
#include "TOcr.h"
#include "TOcrFindEdge.h"
#include "TOcrSmoothEdge.h"
#include "TOcrSentences.h"
#include "TOcrFindRectangle.h"
#include "TOcrAnalyseRectangles.h"
#include "TOcrCharacterShapes.h"
#include "TOcrRecogniseCharacter.h"
#include "TOcrFindSentences.h"
#include "TOcrResources.h"
#include "..\Dialog Files\DialogTemplate.h"
#include "DlgTOcrDebug.h"
#include "..\Logic Files\CopyingMachineCore.h"

//---------------------------------------------------------------------------

TOcr::TOcr(sGlobalInstances poGlobalInstances) {

 	memcpy_s(&oGlobalInstances, sizeof(sGlobalInstances), &poGlobalInstances, sizeof(sGlobalInstances));
	oError = TError::GetInstance();

	hImageHandle = NULL;
    oTestOutput = NULL;
	oSentences = NULL;
    oRectangles = NULL;
	hParentWnd = NULL; 
	hIntermediateHandle = NULL;

	oDataLayer = new TDataLayer("D:\\Projects\\CopyingMachine3\\debug\\shapenet.xml");
	oDataLayer->Resume();
    }

TOcr::~TOcr() {

    if (oRectangles!=NULL) {
        delete oRectangles;
        }
    if (oSentences!=NULL) {
        delete oSentences;
        }
	if (oDataLayer!=NULL) {
		delete oDataLayer;
		}		
    }

//Set the image to process
void 
TOcr::SetImage(HGLOBAL phImageHandle) {

	hImageHandle = phImageHandle;
	}

//Set the image to process
void 
TOcr::SetImage(CBitmap* poBitmap, RectF pcRectangle) {

	Bitmap* oProcessBitmap;
	Bitmap* oTempBitmap;

	//Make a copy of the selection of the image
	oTempBitmap = poBitmap->CreatePartialBitmap(pcRectangle);

	//Convert to 8bpp image
	oProcessBitmap = CBitmap::ConvertBitmap(oTempBitmap, PixelFormat8bppIndexed);
	delete oTempBitmap;

	//Get the handle to the image
	hImageHandle = CBitmap::ConvertToHandle(oProcessBitmap);
	}

    
long
TOcr::GetRectangleAtPosition(long pX, long pY) {

    //this functions searches for the rectangle at the given coordinates (pX, pY)

    long lReturnValue;

    TOcrRectangle* oCandidate;

    lReturnValue = -1;

    try {

        MakeSure(oRectangles!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oRectangles!=NULL");

        for (long lIndex=0; lIndex<oRectangles->GetNumberChilds()-1; lIndex++) {

             oCandidate = oRectangles->GetChild(lIndex);

             MakeSure(oCandidate!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"oCandidate!=NULL");

              if (oCandidate->x <= pX && oCandidate->y <= pY) {

                  if ((oCandidate->x + oCandidate->width) >= pX) {

                      if ((oCandidate->y + oCandidate->height) >= pY) {

                          lReturnValue = lIndex;
                          }
                      }
                  }
              }
         }

    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }

     return lReturnValue;
   }

//This function starts the complete OCR process
bool
TOcr::Execute() {

	//CDlgTOcrDebug* oDlgDebug;
	//oDlgDebug = new CDlgTOcrDebug(oGlobalInstances.hInstance, hParentWnd, IDD_OCRDEBUG, oGlobalInstances);
	//oDlgDebug->Create();
	sTrace.clear();
	sTrace = L"Execute\r\n";	

	return DoExecute();
}

bool
TOcr::DoExecute() {

//    ofstream* out;
    bool bRetval;
    int  iReturnValue;
    TOcrFindEdge* oOcrFindEdge;
    TOcrFindRectangle* oOcrFindRectangle;
//    TOcrSmoothEdges* oOcrSmoothEdge;
    TOcrAnalyseRectangles* oOcrAnalyseRectangles;
    TOcrRecogniseCharacter* oOcrRecogniseCharacter;
    TOcrFindSentences* oOcrFindSentences;
//	wchar_t sTraceTemplate[128];
	std::wostringstream os;

    long lStartTime, lEndTime, lTotalStart, lTotalEnd;
    wstring sTimer;

    bRetval = false;

   try {

		sTrace += L"Creating directories\r\n";

		//Initialize
        CreateDirectory(L"c:\\TOcrImages", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Characters", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Images", NULL);
        CreateDirectory(L"c:\\TOcrImages\\InvertedCharacters", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Characters\\0", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Characters\\1", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Characters\\2", NULL);
        CreateDirectory(L"c:\\TOcrImages\\Splits", NULL);

		if (oSentences!=NULL) {
			delete oSentences;
			}
        if (oRectangles!=NULL) {
            delete oRectangles;
            }

        //Initiate the image handle so we can use it
            MakeSure(hImageHandle!=NULL, ERR_INVALIDPOINTER, LEVEL_WARNING, L"hImageHandle!=NULL");

            long iImageSize = ::GlobalSize(hImageHandle);
            hIntermediateHandle = ::GlobalAlloc(GMEM_MOVEABLE, iImageSize);


        //find the edges in the image by using the given treshold

            //out = new TFileStream("c:\\rectangles.txt", fmCreate);

            int iTreshold = 96;

            lTotalStart = ::GetTickCount();
            lStartTime = ::GetTickCount();

            oOcrFindEdge = new TOcrFindEdge(hImageHandle, hIntermediateHandle, iTreshold, true);
            oOcrFindEdge->Resume();
            iReturnValue = oOcrFindEdge->WaitFor();

            delete oOcrFindEdge;

            MakeSure(iReturnValue==0, ERR_UNKNOWN, LEVEL_RECOVERABLE, L"oOcrFindEdge");

            lEndTime = ::GetTickCount();

			os << L"TOcrFindEdge:";
			os << (lEndTime - lStartTime);
			os << L"ms\r\n";

	 //find the rectangles inside the image

            BITMAPINFO* lpBitmapInfo = (BITMAPINFO*)::GlobalLock(hIntermediateHandle);
            lpBitmapInfo->bmiColors[1].rgbRed = 32;
            lpBitmapInfo->bmiColors[1].rgbBlue = 32;
            lpBitmapInfo->bmiColors[1].rgbGreen = 120;
            lpBitmapInfo->bmiColors[254].rgbRed = 64;
            lpBitmapInfo->bmiColors[254].rgbBlue = 192;
            lpBitmapInfo->bmiColors[254].rgbGreen = 64;
            lpBitmapInfo->bmiColors[253].rgbRed = 192;
            lpBitmapInfo->bmiColors[253].rgbBlue = 64;
            lpBitmapInfo->bmiColors[253].rgbGreen = 64;

            oRectangles = new TOcrRectangle();
            oRectangles->x = 1;
            oRectangles->y = 1;
            oRectangles->width = lpBitmapInfo->bmiHeader.biWidth - 2;
            oRectangles->height = lpBitmapInfo->bmiHeader.biHeight - 2;

            lStartTime = ::GetTickCount();

            ::GlobalUnlock(hIntermediateHandle);

            oOcrFindRectangle = new TOcrFindRectangle(hIntermediateHandle, oRectangles, true);
            oOcrFindRectangle->Resume();
            oOcrFindRectangle->WaitFor();

            delete oOcrFindRectangle;

            lEndTime = ::GetTickCount();

 			os << L"TOcrFindRectangle:";
			os << (lEndTime - lStartTime);
			os << L"ms\r\n";
 			os << L"        Number Rectangles: ";
			os << oRectangles->GetNumberChilds();
			os << L"\r\n";

        //Analyse the rectangles in the image

            lStartTime = ::GetTickCount();

            oOcrAnalyseRectangles = new TOcrAnalyseRectangles(hIntermediateHandle, oRectangles, true);
            oOcrAnalyseRectangles->Resume();
            oOcrAnalyseRectangles->WaitFor();

            delete oOcrAnalyseRectangles;

            lEndTime = ::GetTickCount();

           // oIndicators->Items->Add(sTimer);
 			os << L"TOcrAnalyseRectangles:";
			os << (lEndTime - lStartTime);
			os << L"ms\r\n";


        //Construct sentences out of single rectangles

            lStartTime = ::GetTickCount();

            oSentences = new TOcrSentences;

            oOcrFindSentences = new TOcrFindSentences(hIntermediateHandle, oRectangles, oSentences, true);
            oOcrFindSentences->Resume();
            oOcrFindSentences->WaitFor();

            lEndTime = ::GetTickCount();

  			os << L"TOcrFindSentences:";
			os << (lEndTime - lStartTime);
			os << L"ms\r\n";
  			os << L"        Time Calculate Distances:";
			os << oOcrFindSentences->lTimeCalculateDistances;
			os << L"ms\r\n";
  			os << L"        Time Find Sentences: ";
			os << oOcrFindSentences->lTimeFindSentences;
			os << L"ms\r\n";
  			os << L"        Time Add Spaces: ";
			os << oOcrFindSentences->lTimeAddSpaces;
			os << L"ms\r\n";
  			os << L"        Time Draw Sentences: ";
			os << oOcrFindSentences->lTimeDrawSentences;
			os << L"ms\r\n";
  			os << L"        Number Sentences:";
			os << oSentences->oSentences.size();
			os << L"\r\n";

            delete oOcrFindSentences;



			//Draw rectangles for debugging purposes, can be removed at latter
        //stage in development.

          lStartTime = ::GetTickCount();

//          char cBuffer[100];
//          char cNumber[100];
          long iRowLength;
          long iImageHeight;
          long iImageWidth;
          BYTE* lpBits;

          BITMAPINFO* lpSourceBitmapInfo = (BITMAPINFO*)::GlobalLock(hIntermediateHandle);

          int bpp = lpSourceBitmapInfo->bmiHeader.biBitCount;
          int nColors = lpSourceBitmapInfo->bmiHeader.biClrUsed ? lpSourceBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

          lpBits = (BYTE*)lpSourceBitmapInfo->bmiColors + nColors * sizeof(RGBQUAD);

          //calculate image numbers
          iImageWidth = lpSourceBitmapInfo->bmiHeader.biWidth;
          iImageHeight = lpSourceBitmapInfo->bmiHeader.biHeight;
          int  iBitsPerPixel = lpSourceBitmapInfo->bmiHeader.biBitCount;
          iRowLength = (((iImageWidth*iBitsPerPixel+31)&(~31))/8);

          TOcrRectangle* oChild;

          for (int i=0; i<oRectangles->GetNumberChilds(); i++) {

            oChild = oRectangles->GetChild(i);

            //draw top
            for (int j=0; j<oChild->width; j++) {
                int iPointer = oChild->x + j + (iImageHeight - oChild->y -1)*iRowLength;
                lpBits[iPointer]= 0x80;
                }

            //draw bottom
            for (int j=0; j<oChild->width; j++) {
                int iPointer = oChild->x + j + (iImageHeight - oChild->y - oChild->height -1)*iRowLength;
                lpBits[iPointer]= 0x80;
                }

            //draw left
            for (int j=0; j<oChild->height; j++) {
                int iPointer = oChild->x + (iImageHeight - oChild->y - j -1)*iRowLength;
                lpBits[iPointer]= 0x80;
                }

            //draw right
            for (int j=0; j<oChild->height; j++) {
                int iPointer = oChild->x + oChild->width + (iImageHeight - oChild->y - j -1)*iRowLength;
                lpBits[iPointer]= 0x80;
                }
			}

          ::GlobalUnlock(hIntermediateHandle);

          lEndTime = ::GetTickCount();

          os <<  L"DrawRectangles: ";
          os <<  (lEndTime - lStartTime);
 		  os <<  L"ms\r\n";

        //Do FindEdge again to clean up the intermediate handle, can be
        //removed in later stage of development

			HGLOBAL hImageHandleForRecognition;

			hImageHandleForRecognition = hIntermediateHandle;
            hImageHandleForRecognition = ::GlobalAlloc(GMEM_MOVEABLE, iImageSize);

            oOcrFindEdge = new TOcrFindEdge(hImageHandle, hImageHandleForRecognition, iTreshold, true);
            oOcrFindEdge->Resume();
            oOcrFindEdge->WaitFor();

            delete oOcrFindEdge;

        //Load the xml files used for recognition
            lStartTime = ::GetTickCount();

			TOcrCharacterShapes* oOcrCharacterShapes;

            oOcrCharacterShapes = new TOcrCharacterShapes();
            //oOcrCharacterShapes->LoadData("D:\\Projects\\CopyingMachine3\\debug\\characters_org.xml");

            LoadRealCharacters();
            lEndTime = ::GetTickCount();

			  os <<  L"Load characters XML: ";
			  os <<  (lEndTime - lStartTime);
 			  os <<  L"ms\r\n";

//            float fPercentage;
//            long lNumberSuggestions;
//            long lSuccesNumber;
	
			//Wait for the data layer to finish loading its data
			oDataLayer->WaitFor();
			
			//Do the recognition
			lStartTime = ::GetTickCount();

            oOcrRecogniseCharacter = new TOcrRecogniseCharacter(hImageHandleForRecognition, oRectangles, oSentences, oOcrCharacterShapes, oDataLayer, true);

            oOcrRecogniseCharacter->bSaveXML = bSaveXML;
            oOcrRecogniseCharacter->bSaveImage = bSaveImage;
            oOcrRecogniseCharacter->bSaveResults = bSaveResults;
            oOcrRecogniseCharacter->bSaveSplits = bSaveSplits;

            oOcrRecogniseCharacter->Resume();
            oOcrRecogniseCharacter->WaitFor();

            wstring cSentence;
            cSentence = L"";
//            TOcrRectangle* oWordCharacter;

            for (unsigned long is=0; is<oSentences->oSentences.size(); is++) {

                cSentence = oSentences->oSentences[is]->GetContent(oRectangles);
                }

            os << L"TOcrRecogniseCharacter:";

            lEndTime = ::GetTickCount();

            os << (lEndTime - lStartTime);
			os << L"ms\r\n";

            os << L"        Time Load XML: ";
            os << oOcrRecogniseCharacter->lTimerLoadXML;
			os << L"ms\r\n";

		delete oOcrRecogniseCharacter;
        delete oOcrCharacterShapes;

       //Clean up
       //delete oSentences;

            ::GlobalFree(hImageHandleForRecognition);

            bRetval = true;

            lTotalEnd = ::GetTickCount();

            os << L"Total time: ";
            os << (lTotalEnd - lTotalStart);
			os << L"ms\r\n";
   			
			std::wstring sResult(os.str());

			sTrace += sResult;
        }
    catch (const TExceptionContainer& T) {

        ReportAndThrow(T.lErrorCode, T.lErrorLevel, T.sArgument);
        }
    catch (...) {

        ReportError(LEVEL_WARNING, ERR_UNKNOWN, L"");
        }

    return bRetval;
    }

//This function returns the filename of the file created after completion
//of the OCR process.
wstring 
TOcr::GetResultFile() {


    return L"";
    }

//This function return the trace result string
wstring 
TOcr::GetTraceResult() {

	std::wstring sResult;

	sResult = std::wstring(sTrace);
	sResult += L"\n\r";

	return sResult;
}


//This function sets the PaintBox where in the test images are drawn
void
TOcr::SetTestOutput(Graphics* poTestOutput) {

    oTestOutput = poTestOutput;
    }

//This function write the result image to the test output
void
TOcr::OutputImage(HGLOBAL phImageHandle) {

    if (oTestOutput!=NULL) {
        //void* lpBits;
        //BITMAPINFO* lpBitmapInfo = (BITMAPINFO*)::GlobalLock(phImageHandle);

        //int bpp = lpBitmapInfo->bmiHeader.biBitCount;
        //int nColors = lpBitmapInfo->bmiHeader.biClrUsed ? lpBitmapInfo->bmiHeader.biClrUsed : 1 << bpp;

        //lpBits = lpBitmapInfo->bmiColors + nColors;

        //oTestOutput->Height = lpBitmapInfo->bmiHeader.biHeight;
        //oTestOutput->Width = lpBitmapInfo->bmiHeader.biWidth;
        //oTestOutput->Picture->Bitmap->Width = lpBitmapInfo->bmiHeader.biWidth;
        //oTestOutput->Picture->Bitmap->Height = lpBitmapInfo->bmiHeader.biHeight;

        //HBITMAP hbmp = oTestOutput->Picture->Bitmap->Handle;
        //HDC hdc = oTestOutput->Canvas->Handle;

        //SetDIBits (hdc, hbmp, 0, oTestOutput->Height, lpBits, lpBitmapInfo, DIB_RGB_COLORS);
        //oTestOutput->Repaint();

        //::GlobalUnlock(phImageHandle);
        }
    }

void
TOcr::ProcessEvents() {

    MSG msg;

    if(PeekMessage(&msg, NULL, 0, 0, 1)) {

        if( (msg.message != WM_DESTROY)   &&
            (msg.message != WM_NCDESTROY) &&
            (msg.message != WM_CLOSE)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        }
    }

void
__fastcall TOcr::LoadRealCharacters()
{
//        TiXmlElement* RealCharactersElement;
        TiXmlElement* RealCharacterElement = 0;

        TiXmlNode* RealCharactersNode;
//        TiXmlNode* RealCharacterNode;

        TiXmlDocument doc("D:\\Projects\\CopyingMachine3\\debug\\real.xml");

        cRealCharacter oReal;

        //laadt data
        doc.LoadFile();

        RealCharactersNode = doc.FirstChild("CharacterReals");

        if (RealCharactersNode!=NULL) {

            RealCharacterElement = RealCharactersNode->FirstChildElement("CharacterReal");

            while (RealCharacterElement !=NULL) {

                //oMatch.pX = atoi(RealCharacterElement->Attribute("Px")->c_str());
                //oMatch.pY = atoi(RealCharacterElement->Attribute("Py")->c_str());
                oReal.lRectNumber =  atoi(RealCharacterElement->Attribute("number"));
                strcpy(oReal.cChar, RealCharacterElement->Attribute("char"));
                oReal.lSuccess = 0;

                oRealCharacters.push_back(oReal);

                RealCharacterElement = RealCharacterElement->NextSiblingElement();
                }
        }
 }

void
__fastcall TOcr::SaveRealCharacters()
{
        TiXmlElement* RealsElement;
        TiXmlElement* RealElement = 0;

//        TiXmlNode* RealNode;

        TiXmlDocument doc("D:\\Projects\\CopyingMachine3\\debug\\real_result.xml");

//        cRealCharacter oReal;

        RealsElement = new TiXmlElement("Reals");

        if (RealsElement!=NULL) {

            for (unsigned long i=0; i<oRealCharacters.size(); i++) {

                RealElement = new TiXmlElement("Real");
                RealElement->SetAttribute("number", oRealCharacters[i].lRectNumber);
                RealElement->SetAttribute("char", oRealCharacters[i].cChar);
                RealElement->SetAttribute("success", oRealCharacters[i].lSuccess);

                RealsElement->InsertEndChild(*RealElement);

                delete RealElement;
                }
            }

          doc.SaveFile();

          delete RealsElement;
    }

