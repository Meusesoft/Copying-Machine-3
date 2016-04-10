#include "StdAfx.h"
#include "CopyingMachineCore.h"
#include "LibTesseract\tessdll.h"
#include "unichar.h"

COCRCore::COCRCore(sGlobalInstances pInstances):
	CInterfaceBase(pInstances)
{
	//Initialise thread variables
	bEndThread = false;
	hThread = NULL;
	iRequestNumber = 0;
	hMainWindow = NULL;

	oOcr = new TOcr(oGlobalInstances);
	oOcr->bSaveXML		= false;
	oOcr->bSaveImage	= false;
	oOcr->bSaveResults	= false;
	oOcr->bSaveSplits	= false;
	oOcr->iCompareMethod = 0;
}

COCRCore::~COCRCore(void)
{
	//Clean up the thread
	if (hThread!=NULL) {
		
		SetEndThread();

		//Wait for the thread to finish.
		WaitForSingleObject(hThread, INFINITE);
	
		CloseHandle(hThread);

		DeleteCriticalSection(&ThreadCriticalSection);
		DeleteCriticalSection(&EndThreadCriticalSection);
		}

	//delete instances
	delete oOcr;
}

//This function fills the page on the outputdevice with images from
//the document
eActionResult  
COCRCore::OCRPage(sOCRPage* poOCRPage) {

	#ifdef DEBUG
	return OCRPageTOcr(poOCRPage);
	#endif
	#ifndef DEBUG
	return OCRPageTesseract(poOCRPage);
	#endif
	}

//OCR the page with TOCR
eActionResult 
COCRCore::OCRPageTOcr(sOCRPage* poOCRPage) {

	eActionResult cResult = eOcrError;

	CCopyingMachineCore*	oCore;
	CCopyDocumentPage*		oPage;
	CRecognitionLayer*		oLayer;
	CRecognizedText*		oText;
	sCopyDocumentSelection cSelection;
	Rect cRectangle;
	RectF cSelectionRectangle;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oPage = poOCRPage->oPage;

	if (oPage!=NULL) {

		//we got the page, use it.
		//1. Get the selection and create an instance of the TOcr class
		oPage->GetSelection(cSelection);

		cSelectionRectangle.X =		(REAL)cSelection.fLeft;
		cSelectionRectangle.Y =		(REAL)cSelection.fTop;
		cSelectionRectangle.Height = (REAL)(cSelection.fBottom - cSelection.fTop);
		cSelectionRectangle.Width =	(REAL)(cSelection.fRight - cSelection.fLeft);

		//2 Start OCR through TOcr
		oOcr->SetImage(oPage->GetImage(), cSelectionRectangle);

		#ifdef DEBUG
		oOcr->bSaveXML		= poOCRPage->oOcrSettings.bSaveXML;
		oOcr->bSaveImage	= poOCRPage->oOcrSettings.bSaveImage;
		oOcr->bSaveResults	= poOCRPage->oOcrSettings.bSaveResults;
		oOcr->bSaveSplits	= poOCRPage->oOcrSettings.bSaveSplits;
		oOcr->iCompareMethod = poOCRPage->oOcrSettings.iCompareMethod;
		#endif

		oOcr->hParentWnd = hMainWindow;
		oOcr->Execute();

		#ifdef DEBUG
		poOCRPage->oOcrSettings.hIntermediateImage = oOcr->hIntermediateHandle;
		poOCRPage->oOcrSettings.sTrace = oOcr->GetTraceResult();
		#endif

		//3 Process the results
		oLayer = oPage->GetRecognitionLayer();
		
		cRectangle.X = (int)cSelectionRectangle.X;
		cRectangle.Y = (int)cSelectionRectangle.Y;
		cRectangle.Width = (int)cSelectionRectangle.Width;
		cRectangle.Height = (int)cSelectionRectangle.Height;

		oText = oLayer->CreateRecognizedText(cRectangle, poOCRPage->cLanguage);
		std::wstring sOutput;
		sOutput = L"";

		long lRectangleId;
		long lNumberRectangles;
		long lSpaces;
		TOcrRectangle*	oChildRectangle;

		lSpaces = 0;

		for (long lSentenceIndex=0; lSentenceIndex<(long)oOcr->oSentences->oSentences.size(); lSentenceIndex++) {

			lNumberRectangles = oOcr->oSentences->oSentences[lSentenceIndex]->NumberOfRectangles();

			for (long lRectangleIndex=0; lRectangleIndex<lNumberRectangles; lRectangleIndex++) {

				lRectangleId = oOcr->oSentences->oSentences[lSentenceIndex]->GetRectangle(lRectangleIndex);
				
				if (lRectangleId==-1) {

					lSpaces++;
					sOutput += L" ";
					}
				else {
				
					oChildRectangle = oOcr->oRectangles->GetChild(lRectangleId);

					if (oText) {

						sOutput += oChildRectangle->sContent[0];
						
						cRectangle.X		= oChildRectangle->x;
						cRectangle.Y		= oChildRectangle->y;
						cRectangle.Width	= oChildRectangle->width;
						cRectangle.Height	= oChildRectangle->height;

						oText->AddCharacter(oChildRectangle->sContent[0],	//the contents of the character
											cRectangle,						//bounding box of the character
											lSpaces,						//the number of spaces before this character
											lRectangleIndex==0 ? 1 : 0);	// is this the end of this line/sentence?

						lSpaces = 0;
						}
					}
				}
			
			if (lSentenceIndex>0) sOutput += L"\r\n";
			}

		//4. Put the result in the OCR object
		cResult = eOk;

		poOCRPage->sOutput = sOutput;
		poOCRPage->cResult = eOk;

		//5 Clean up
		//delete oOcr;
	}

	return cResult;
}

//OCR the page with Tesseract
eActionResult 
COCRCore::OCRPageTesseract(sOCRPage* poOCRPage) {

	eActionResult cResult;
	
	oTrace->StartTrace(__WFUNCTION__);

	cResult = eOcrError;

	CCopyingMachineCore*	oCore;
	CCopyDocumentPage*		oPage;
	CRecognitionLayer*		oLayer;
	CRecognizedText*		oText;
	sCopyDocumentSelection cSelection;
	Rect cRectangle;
	Bitmap* oPageBitmap;
	long lNumberEndOfLines;

	oCore = (CCopyingMachineCore*)oGlobalInstances.oCopyingMachineCore;
	oPage = poOCRPage->oPage;

	if (oPage!=NULL) {

		//we got the page, use it.
		//1. Make a copy of the selection into a processable memory buffer
		oPage->GetSelection(cSelection);

		cRectangle.X =		(int)cSelection.fLeft;
		cRectangle.Y =		(int)cSelection.fTop;
		cRectangle.Height = (int)(cSelection.fBottom - cSelection.fTop);
		cRectangle.Width =	(int)(cSelection.fRight - cSelection.fLeft);

		oPageBitmap = oPage->GetImage()->GetBitmap();

		if (oPageBitmap) {

			BitmapData* bitmapDataSource = new BitmapData;
			DWORD dMemSize;
			DWORD dRowSize;
			unsigned char* cPageContent;
			unsigned char* cSourcePointer;
			unsigned char* cDestinationPointer;

			//Lock the bits of the image to be able to copy the contents
			oPageBitmap->LockBits(&cRectangle, ImageLockModeRead, oPageBitmap->GetPixelFormat(), bitmapDataSource); 

			//Calculate the necessary size
			int iBPP = GetPixelFormatSize(oPageBitmap->GetPixelFormat());
			dRowSize = (cRectangle.Width * iBPP);
			dRowSize = dRowSize / 8 + (((dRowSize % 8) > 0) ? 1 : 0); //Row size is byte aligned
			dMemSize = cRectangle.Height * dRowSize;

			//Allocate memory and initialise pointers
			cPageContent = (unsigned char*)GlobalAlloc(GMEM_FIXED, dMemSize);
			cDestinationPointer = cPageContent;
			cSourcePointer = (unsigned char*)bitmapDataSource->Scan0;

			//Copy the contents from the source to the destination buffer (cPageContent)
			for (long lIndex=0; lIndex<(long)bitmapDataSource->Height; lIndex++) {

				memcpy_s(cDestinationPointer, dRowSize, cSourcePointer, dRowSize);

				cDestinationPointer += dRowSize;
				cSourcePointer += bitmapDataSource->Stride;
				}

			//Release the source bitmap
			oPageBitmap->UnlockBits(bitmapDataSource);

			//2a. Determine the language
			std::string sLanguage;

			switch (poOCRPage->cLanguage) {

				case LANG_DUTCH:
					sLanguage = "nld";
					break;

				case LANG_GERMAN:
					sLanguage = "deu";
					break;

				default:
					sLanguage = "eng";
					break;
				}

			//2b. Start OCR through Tesseract
			TessDllAPI* oOcrApi;
			
			oOcrApi = new TessDllAPI(sLanguage.c_str()); //to do, now we use Dutch as standard language

			oOcrApi->BeginPageUpright(cRectangle.Width, cRectangle.Height, cPageContent, iBPP);

			ETEXT_DESC* output = oOcrApi->Recognize_all_Words();

			oOcrApi->EndPage();

			GlobalFree(cPageContent);

			//3. Process the results
			oLayer = oPage->GetRecognitionLayer();
			oText = oLayer->CreateRecognizedText(cRectangle, poOCRPage->cLanguage);

			std::wstring sOutput;
			sOutput = L"";

			// It should be noted that the format for char_code for version 2.0 and beyond is UTF8
			// which means that ASCII characters will come out as one structure but other characters
			// will be returned in two or more instances of this structure with a single byte of the
			// UTF8 code in each, but each will have the same bounding box.
			// Programs which want to handle languagues with different characters sets will need to
			// handle extended characters appropriately, but *all* code needs to be prepared to
			// receive UTF8 coded characters for characters such as bullet and fancy quotes.
			int j;
			for (int i = 0; i < output->count; i = j) {
				const EANYCODE_CHAR* ch = &output->text[i];
		
				for (int b = 0; b < ch->blanks; ++b)
					sOutput += L" ";

				for (j = i; j < output->count; j++)
					{
					const EANYCODE_CHAR* unich = &output->text[j];

					if (ch->left != unich->left || ch->right != unich->right ||
						ch->top != unich->top || ch->bottom != unich->bottom)
						break;

					sOutput += unich->char_code;

					lNumberEndOfLines = 0;

					if (ch->formatting & 64) {
						sOutput += L"\r\n";
						lNumberEndOfLines++;
						}
					if (ch->formatting & 128) {
						sOutput += L"\r\n\r\n";
						lNumberEndOfLines+=2;
						}
					if (oText) {
						
						cRectangle.X		= unich->left;
						cRectangle.Y		= unich->top;
						cRectangle.Width	= unich->right - ch->left;
						cRectangle.Height	= unich->bottom - ch->top;

						oText->AddCharacter(unich->char_code, cRectangle, ch->blanks, lNumberEndOfLines);
						}
					}
				}

			delete oOcrApi;


			//4. Put the result in the OCR object
			cResult = eOk;

			poOCRPage->sOutput = sOutput;
			poOCRPage->cResult = eOk;
			}
		}

	oTrace->EndTrace(__WFUNCTION__, cResult);

	return cResult;
}

//This is the main function for printing a document by thread
bool
COCRCore::StartOCRThread() {

	oTrace->StartTrace(__WFUNCTION__);
	
	bEndThread = !InitializeCriticalSectionAndSpinCount(&EndThreadCriticalSection, 0x80000400);
	bEndThread = bEndThread || !InitializeCriticalSectionAndSpinCount(&ThreadCriticalSection, 0x80000400);

	//Create a thread for printing the document. Only if the critical section
	//is initialized
	if (!bEndThread) {

		hThread = CreateThread(
			NULL,									// default security attributes
			0,										// use default stack size  
			&ThreadOCRPage,							// thread function 
			oGlobalInstances.oCopyingMachineCore,	// argument to thread function 
			0,										// use default creation flags 
			NULL);									// returns the thread identifier 
 
			}

	oTrace->EndTrace(__WFUNCTION__,(bool)(hThread!=NULL));

	return (bool)(hThread!=NULL); 
	}

//This is the main function for ocring a page by thread
int
COCRCore::OCRPageThread(CCopyDocumentPage* poPage, cOcrSettings poOcrSettings, LANGID pcLanguage) {

	sOCRPage *oOCRPage;
	
	oTrace->StartTrace(__WFUNCTION__);

	if (bEndThread && hThread!=NULL) {

		//Wait for the thread to finish.
		WaitForSingleObject(hThread, INFINITE);
		
		hThread = NULL;
		}

	if (hThread==NULL) {

		StartOCRThread();
		}

	if (hThread!=NULL) {
	
		EnterCriticalSection(&ThreadCriticalSection); 

		iRequestNumber++;

		oOCRPage = new sOCRPage;
		oOCRPage->oPage = poPage;
		oOCRPage->iRequest = iRequestNumber;
		oOCRPage->cLanguage = pcLanguage;

		#ifdef DEBUG
		oOCRPage->oOcrSettings.bSaveXML		= poOcrSettings.bSaveXML;
		oOCRPage->oOcrSettings.bSaveImage	= poOcrSettings.bSaveImage;
		oOCRPage->oOcrSettings.bSaveResults	= poOcrSettings.bSaveResults;
		oOCRPage->oOcrSettings.bSaveSplits	= poOcrSettings.bSaveSplits;
		oOCRPage->oOcrSettings.iCompareMethod= poOcrSettings.iCompareMethod;
		oOCRPage->oOcrSettings.hIntermediateImage = poOcrSettings.hIntermediateImage;
		oOCRPage->oOcrSettings.sTrace = poOcrSettings.sTrace;
		#endif

		oOCRQueue.push_back(oOCRPage);

		LeaveCriticalSection(&ThreadCriticalSection);
		}

	oTrace->EndTrace(__WFUNCTION__, (oOCRPage==NULL) ? -1 : iRequestNumber);

	return oOCRPage==NULL ? -1 : iRequestNumber;
	}

//This function returns true if an end thread is signalled
bool 
COCRCore::GetEndThread() {

	bool bResult;

	EnterCriticalSection(&EndThreadCriticalSection); 

	bResult = bEndThread;

    LeaveCriticalSection(&EndThreadCriticalSection);

	return bResult;
	}

//This function set the EndThread signal to true
void 
COCRCore::SetEndThread() {

	EnterCriticalSection(&EndThreadCriticalSection); 

	bEndThread = true;

    LeaveCriticalSection(&EndThreadCriticalSection); 
	}

//This is the main processing function for printing document
DWORD 
WINAPI ThreadOCRPage(LPVOID lpParameter) {

	sOCRPage* oOCRPage;	
	COCRCore* oOCRCore;
	CCopyingMachineCore* oCore;
	eActionResult cResult;

	oCore = (CCopyingMachineCore*)lpParameter;
	oOCRCore = oCore->oOCRCore;

	while (!oOCRCore->GetEndThread()) {

		Sleep(500);

		EnterCriticalSection(&oOCRCore->ThreadCriticalSection); 

		if (oOCRCore->oOCRQueue.size()>0) {

			Sleep(500);

			oOCRPage = oOCRCore->oOCRQueue[0];
			
			LeaveCriticalSection(&oOCRCore->ThreadCriticalSection);

			cResult = oOCRCore->OCRPage(oOCRPage);

			//let the application know we are done.
			oCore->oNotifications->CommunicateEvent(eNotificationPageOCRDone, oOCRPage); 

			EnterCriticalSection(&oOCRCore->ThreadCriticalSection); 

			oOCRCore->oOCRQueue.erase(oOCRCore->oOCRQueue.begin());
			}
		
		LeaveCriticalSection(&oOCRCore->ThreadCriticalSection);
		}

	return 0;
	}
